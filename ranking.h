#ifndef RANKING_H
#define RANKING_H

#include <QGraphicsScene>

class QLabel;
class QPushButton;
class QWidget;

class Ranking : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit Ranking(
        QObject *parent = nullptr
        );

public slots:
    // Lee nuevamente usuarios.txt y actualiza
    // las cinco posiciones mostradas.
    void actualizarRanking();

signals:
    // Solicita regresar al menú principal.
    void volverSolicitado();

private:
    static const int CANTIDAD_POSICIONES = 5;

    // Panel que contiene cada posición.
    QWidget *panelesFila[CANTIDAD_POSICIONES];

    // Textos mostrados en las cinco filas.
    QLabel *etiquetasPosicion[CANTIDAD_POSICIONES];
    QLabel *etiquetasUsuario[CANTIDAD_POSICIONES];
    QLabel *etiquetasPuntos[CANTIDAD_POSICIONES];

    QPushButton *botonVolver;

    void construirInterfaz();

    // Aplica el color correspondiente a cada puesto.
    void configurarEstiloFila(
        int indice
        );
};

#endif // RANKING_H