#include "supabaseclient.h"
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QTimer>
#include <QUrl>
#include <utility>

namespace {
const QString urlPredeterminada = QStringLiteral("https://cnjuzrtseqcefiebwexf.supabase.co");
const QByteArray clavePredeterminada = "sb_publishable_UcgxGHxjUiYdy1LT8aQxpg_5LgWlp8I";
bool esLocal(const QUrl &url) {
    return url.host() == "127.0.0.1" || url.host() == "localhost" || url.host() == "::1";
}
QUrl urlBase() {
    const QUrl alternativa(qEnvironmentVariable("SNAKE_SUPABASE_URL"));
    if (alternativa.isValid() && !alternativa.host().isEmpty()
        && alternativa.userInfo().isEmpty()
        && (alternativa.scheme() == "https" || (alternativa.scheme() == "http" && esLocal(alternativa))))
        return alternativa;
    return QUrl(urlPredeterminada);
}
QNetworkAccessManager *administrador() {
    static QPointer<QNetworkAccessManager> instancia;
    if (!instancia) instancia = new QNetworkAccessManager(QCoreApplication::instance());
    return instancia;
}
}

QString SupabaseClient::mensajeError(int http, const QJsonObject &body,
                                     const QString &networkError) {
    QString code = body.value("error_code").toString();
    if (code.isEmpty()) code = body.value("code").toString();
    if (code == "email_address_invalid") return QStringLiteral("El identificador interno de la cuenta no es válido.");
    if (code == "email_not_confirmed") return QStringLiteral("La cuenta requiere activación. Contacte al administrador del juego.");
    if (code == "invalid_credentials") return QStringLiteral("El usuario o la contraseña son incorrectos.");
    if (code == "user_already_exists" || code == "email_exists") return QStringLiteral("Ya existe una cuenta con este usuario.");
    if (code == "weak_password") return QStringLiteral("La contraseña es demasiado débil. Usa una contraseña más segura.");
    if (http == 429 || code == "over_request_rate_limit" || code == "over_email_send_rate_limit"
        || code == "over_sms_send_rate_limit")
        return QStringLiteral("Se realizaron demasiados intentos. Espera un momento y vuelve a intentarlo.");
    if (code == "PGRST205" || code == "PGRST204" || code == "42P01" || code == "42703")
        return QStringLiteral("La base de datos no tiene las tablas o columnas necesarias. Ejecuta el script de configuración de Supabase.");
    for (const QString &field : {QStringLiteral("message"), QStringLiteral("msg"), QStringLiteral("error_description"), QStringLiteral("error")}) {
        const QString message = body.value(field).toString();
        if (!message.isEmpty()) return message;
    }
    if (!networkError.isEmpty()) return networkError;
    return QStringLiteral("La solicitud falló (HTTP %1).").arg(http);
}

void SupabaseClient::solicitar(const QByteArray &metodo, const QString &ruta,
                               const QJsonObject &cuerpo, const QString &token,
                               QObject *contexto, Callback callback) {
    const QPointer<QObject> guardia(contexto);
    const QUrl base = urlBase();
    // The route is appended, never resolved as another origin.
    QString baseTexto = base.toString(QUrl::RemoveQuery | QUrl::RemoveFragment);
    while (baseTexto.endsWith('/')) baseTexto.chop(1);
    const QUrl destino(baseTexto + (ruta.startsWith('/') ? ruta : '/' + ruta));
    QNetworkRequest request(destino);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json");
    const QByteArray clave = qgetenv("SNAKE_SUPABASE_KEY");
    request.setRawHeader("apikey", clave.isEmpty() ? clavePredeterminada : clave);
    if (!token.isEmpty()) request.setRawHeader("Authorization", "Bearer " + token.toUtf8());
    if (metodo == "PATCH" || metodo == "POST") request.setRawHeader("Prefer", "return=representation");
    const QByteArray payload = (metodo == "GET" || metodo == "DELETE")
        ? QByteArray() : QJsonDocument(cuerpo).toJson(QJsonDocument::Compact);
    auto *reply = administrador()->sendCustomRequest(request, metodo, payload);
    auto *timer = new QTimer(reply);
    timer->setSingleShot(true);
    int timeout = 20000;
    if (esLocal(base)) {
        bool valido = false;
        const int overrideMs = qEnvironmentVariableIntValue("SNAKE_SUPABASE_TIMEOUT_MS", &valido);
        if (valido && overrideMs > 0 && overrideMs <= 20000) timeout = overrideMs;
    }
    QObject::connect(timer, &QTimer::timeout, reply, [reply] {
        reply->setProperty("snakeTimeout", true);
        reply->abort();
    });
    QObject::connect(reply, &QNetworkReply::finished, reply,
                     [reply, timer, guardia, token, cuerpo, callback = std::move(callback)]() mutable {
        timer->stop();
        Resultado result;
        result.codigo = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray contenido = reply->isOpen() ? reply->readAll() : QByteArray();
        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(contenido, &parseError);
        if (document.isObject()) result.datos = document.object();
        else if (document.isArray()) result.datos = document.array();
        const bool httpOk = result.codigo >= 200 && result.codigo < 300;
        const bool jsonOk = parseError.error == QJsonParseError::NoError
            && (document.isObject() || document.isArray());
        result.exito = reply->error() == QNetworkReply::NoError && httpOk
            && (result.codigo == 204 || jsonOk);
        if (result.exito) result.mensaje = QStringLiteral("OK");
        else if (reply->property("snakeTimeout").toBool()) result.mensaje = QStringLiteral("Se agotó el tiempo de espera. Revisa tu conexión y vuelve a intentarlo.");
        else if (httpOk && !jsonOk) result.mensaje = QStringLiteral("El servidor devolvió una respuesta JSON inválida.");
        else result.mensaje = mensajeError(result.codigo, document.object(),
            reply->error() == QNetworkReply::NoError ? QString() : reply->errorString());
        // Servers may echo submitted values in errors; never show credentials.
        for (const QString &secreto : {token, cuerpo.value("password").toString(),
                                      cuerpo.value("refresh_token").toString()}) {
            if (!secreto.isEmpty()) result.mensaje.replace(secreto, QStringLiteral("[oculto]"));
        }
        reply->deleteLater();
        if (guardia && callback) callback(std::move(result));
    });
    timer->start(timeout);
}


