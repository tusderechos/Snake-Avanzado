#ifndef GESTORUSUARIOS_H
#define GESTORUSUARIOS_H
#include "supabaseclient.h"
#include <QObject>
#include <QPointer>
#include <QVector>
#include <deque>
#include <functional>
// Lecturas del perfil confirmado. Una escritura aceptada se encola y confirma por señal.
class GestorUsuarios : public QObject {
    Q_OBJECT
public:
    using Respuesta = std::function<void(bool, const QString &)>;
    struct DatoRanking { QString usuario; int puntos; };
    static GestorUsuarios &instancia();
    static QString nombreActual();
    static bool sesionActiva();
    static bool pendientes();
    static void reintentar();
    static bool cerrarSesion();
    static void registrarUsuario(const QString &, const QString &, QObject *, Respuesta);
    static void iniciarSesion(const QString &, const QString &, QObject *, Respuesta);
    static void cambiarContrasena(const QString &, const QString &, const QString &, QObject *, Respuesta);
    static void obtenerRanking(int, QObject *, std::function<void(bool, const QString &, QVector<DatoRanking>)>);
    static int obtenerPuntosUsuario(const QString &);
    static int obtenerMonedasUsuario(const QString &);
    static bool tieneSkin(const QString &, const QString &);
    static QString obtenerSkinEquipada(const QString &);
    static QString obtenerAvatar(const QString &);
    static bool tutorialCompletado(const QString &);
    static int obtenerNivelHistoria(const QString &);
    static QJsonObject perfil(const QString &);
    static bool registrarPuntajePartida(const QString &, int, int = 0);
    static bool comprarSkin(const QString &, const QString &, int);
    static bool equiparSkin(const QString &, const QString &);
    static bool marcarTutorialCompletado(const QString &);
    static bool marcarNivelHistoriaCompletado(const QString &, int);
    static bool guardarPreferencias(const QString &, const QJsonObject &);
    static bool guardarAvatar(const QString &, const QString &);
signals:
    void perfilActualizado();
    void pendientesCambiaron();
    void errorGuardado(const QString &mensaje);
private:
    explicit GestorUsuarios(QObject *parent);
    struct Trabajo { QByteArray metodo; QString ruta; QJsonObject cuerpo; };
    static bool encolar(const QString &, const QString &, const QJsonObject &, const QByteArray & = "POST");
    void procesar();
    void conToken(std::function<void(bool, QString)>);
    bool adoptarSesion(const QJsonObject &);
    QString m_token, m_refresh, m_id, m_correo;
    qint64 m_expira = 0;
    QJsonObject m_perfil;
    std::deque<Trabajo> m_cola;
    bool m_procesando = false, m_fallido = false, m_autenticando = false;
    quint64 m_generacion = 0;
};
#endif
