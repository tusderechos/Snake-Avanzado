#ifndef PERFIL_H
#define PERFIL_H

#include <QGraphicsScene>
#include <QString>
#include <QStringList>

class QLabel;
class QLineEdit;
class QPushButton;

class Perfil : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit Perfil(QObject *parent = nullptr);

    // Actualiza la información del usuario conectado.
    void establecerUsuario(const QString &usuario);

signals:
    void volverSolicitado();

private slots:
    void intentarCambiarContrasena();
    void alternarVisibilidadContrasenas();
    void cambiarAvatar(int paso);

private:
    QString usuarioActual;

    QLabel *etiquetaInicial;
    QLabel *etiquetaUsuario;
    QLabel *etiquetaPuntos;
    QLabel *etiquetaAvatar;
    QLabel *nombreAvatar;
    QLabel *mensajeEstado;

    QLineEdit *campoContrasenaActual;
    QLineEdit *campoContrasenaNueva;
    QLineEdit *campoConfirmacion;

    QPushButton *botonMostrarContrasenas;
    QPushButton *botonGuardar = nullptr;
    QPushButton *botonVolver = nullptr;
    QPushButton *avatarAnterior = nullptr;
    QPushButton *avatarSiguiente = nullptr;
    QStringList avatares;
    QStringList nombresAvatares;
    int avatarActual = 0;
    bool solicitudPendiente = false;

    void construirInterfaz();

    QLabel *crearEtiqueta(
        const QString &texto,
        qreal x,
        qreal y,
        int ancho,
        int alto,
        int tamanoLetra
        );
    void actualizarAvatar();
};

#endif // PERFIL_H
