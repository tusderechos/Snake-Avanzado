#ifndef ESCENAREGISTRO_H
#define ESCENAREGISTRO_H

#include <QGraphicsScene>

class QLineEdit;
class QLabel;
class QPushButton;

class EscenaRegistro : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit EscenaRegistro(QObject *parent = nullptr);

signals:
    void volverSolicitado();

    // Envía el nombre del usuario que creó la cuenta.
    void cuentaCreada(const QString &usuario);
private slots:
    // Se ejecutará cada vez que cambie la contraseña.
    void actualizarValidaciones();

    // Se ejecutará al presionar Crear cuenta.
    void intentarCrearCuenta();

    // Muestra u oculta los dos campos de contraseña.
    void alternarVisibilidadContrasenas();

private:
    // Campos de entrada
    QLineEdit *campoUsuario;
    QLineEdit *campoContrasena;
    QLineEdit *campoConfirmacion;

    // Indicadores de requisitos
    QLabel *requisitoLongitud;
    QLabel *requisitoMayuscula;
    QLabel *requisitoMinuscula;
    QLabel *requisitoNumero;
    QLabel *requisitoEspecial;
    QLabel *requisitoCoincidencia;

    // Mensaje visible dentro de la pantalla
    QLabel *mensajeEstado;

    // Botones
    QPushButton *botonCrearCuenta;
    QPushButton *botonVolver;
    QPushButton *botonMostrarContrasenas;

    // Métodos internos
    void construirInterfaz();
    void crearTitulo(
        const QString &texto,
        qreal x,
        qreal y,
        int ancho,
        const QString &estilo
        );
    void actualizarIndicador(QLabel *indicador, bool cumplido, const QString &texto);

    bool formularioCompleto() const;
};

#endif // ESCENAREGISTRO_H
