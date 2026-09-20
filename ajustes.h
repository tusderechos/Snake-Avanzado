#ifndef AJUSTES_H
#define AJUSTES_H

#include <QGraphicsScene>
#include <QString>

class QLabel;
class QPushButton;
class QSlider;
class QTimer;

class Ajustes : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit Ajustes(QObject *parent = nullptr);
    void establecerUsuario(const QString &usuario);
    void confirmarCambios();

signals:
    void volverSolicitado();
    void perfilSolicitado();
    void controlesSolicitados();

private slots:
    void guardarVolumenActual();

private:
    QSlider *barraMusica;
    QSlider *barraSonido;
    QLabel *etiquetaVolumenMusica;
    QLabel *etiquetaVolumenSonido;
    QString usuarioActual;
    bool cargandoConfiguracion;
    QTimer *temporizadorGuardado = nullptr;
    QString usuarioPendiente;

    void construirInterfaz();

    QLabel *crearEtiqueta(
        const QString &texto,
        qreal x,
        qreal y,
        int ancho,
        int alto,
        int tamanoLetra
        );

    QPushButton *crearBoton(
        const QString &texto,
        qreal x,
        qreal y,
        int ancho,
        int alto
        );

    QSlider *crearBarraVolumen(
        qreal x,
        qreal y,
        int valorInicial,
        const QString &imagenPerilla,
        int ancho,
        int anchoPerilla
        );

    void crearIcono(
        const QString &simbolo,
        qreal x,
        qreal y
        );
};

#endif // AJUSTES_H
