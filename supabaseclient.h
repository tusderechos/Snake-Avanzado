#ifndef SUPABASECLIENT_H
#define SUPABASECLIENT_H

#include <QByteArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <functional>
class QObject;

class SupabaseClient {
public:
    struct Resultado {
        bool exito = false;
        int codigo = 0;
        QString mensaje;
        QJsonValue datos;
    };
    using Callback = std::function<void(Resultado)>;
    static void solicitar(const QByteArray &metodo, const QString &ruta,
                          const QJsonObject &cuerpo, const QString &token,
                          QObject *contexto, Callback callback);
    static QString mensajeError(int http, const QJsonObject &body,
                                const QString &networkError = {});
};
#endif
