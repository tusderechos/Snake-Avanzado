#ifndef PERFIL_H
#define PERFIL_H

#include <QGraphicsScene>
#include <QString>

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

private:
    QString usuarioActual;

    QLabel *etiquetaInicial;
    QLabel *etiquetaUsuario;
    QLabel *etiquetaPuntos;
    QLabel *mensajeEstado;

    QLineEdit *campoContrasenaActual;
    QLineEdit *campoContrasenaNueva;
    QLineEdit *campoConfirmacion;

    QPushButton *botonMostrarContrasenas;

    void construirInterfaz();

    QLabel *crearEtiqueta(
        const QString &texto,
        qreal x,
        qreal y,
        int ancho,
        int alto,
        int tamanoLetra
        );
};

#endif // PERFIL_H
