#ifndef RANKING_H
#define RANKING_H

#include <QGraphicsScene>

class QLabel;
class QPushButton;
class QScrollArea;
class QWidget;

class Ranking : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit Ranking(
        QObject *parent = nullptr
        );

public slots:
    // Consulta el ranking remoto y actualiza las posiciones mostradas.
    void actualizarRanking();

signals:
    // Solicita regresar al menú principal.
    void volverSolicitado();

private:
    static const int CANTIDAD_POSICIONES = 100;

    // Paneles que contienen las posiciones del ranking desplazable.
    QWidget *panelesFila[CANTIDAD_POSICIONES];

    // Textos mostrados en las filas del ranking.
    QLabel *etiquetasPosicion[CANTIDAD_POSICIONES];
    QLabel *etiquetasUsuario[CANTIDAD_POSICIONES];
    QLabel *etiquetasAvatar[CANTIDAD_POSICIONES];
    QLabel *etiquetasPuntos[CANTIDAD_POSICIONES];

    QScrollArea *areaScroll;
    QPushButton *botonVolver;
    bool cargando = false;

    void construirInterfaz();

    // Aplica el color correspondiente a cada puesto.
    void configurarEstiloFila(
        int indice
        );
};

#endif // RANKING_H
