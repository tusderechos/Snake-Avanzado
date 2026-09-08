#ifndef INICIOSESION_H
#define INICIOSESION_H

#include <QGraphicsScene>
#include <QString>

class QLineEdit;
class QLabel;
class QPushButton;

class InicioSesion : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit InicioSesion(
        QObject *parent = nullptr
        );

signals:
    // Regresa a la primera pantalla.
    void volverSolicitado();

    // Se emite cuando el usuario y la contraseña
    // son correctos.
    void sesionIniciada(
        const QString &usuario
        );

private slots:
    // Comprueba los datos escritos.
    void intentarIniciarSesion();

    // Muestra u oculta la contraseña.
    void alternarVisibilidadContrasena();

private:
    QLineEdit *campoUsuario;
    QLineEdit *campoContrasena;

    QLabel *mensajeEstado;

    QPushButton *botonIngresar;
    QPushButton *botonVolver;
    QPushButton *botonMostrarContrasena;

    void construirInterfaz();
};

#endif // INICIOSESION_H
