#ifndef JUEGOVIEW_H
#define JUEGOVIEW_H

#include <QGraphicsView>

class QGraphicsRectItem;
class QGraphicsScene;
class QGraphicsTextItem;
class QKeyEvent;
class QTimer;
class ProgresoNivel;
class Snake;
class Tablero;

class JuegoView : public QGraphicsView
{
public:
    JuegoView();
    ~JuegoView() override;

protected:
    void keyPressEvent(QKeyEvent *evento) override;

private:
    static constexpr int COLUMNAS = 6;
    static constexpr int FILAS = 6;
    static constexpr int TAMANO_CELDA = 80;
    static constexpr int INTERVALO_MOVIMIENTO = 300;
    static constexpr int META_FRUTAS = 5;
    static constexpr int META_PUNTOS = 50;
    static constexpr int META_LONGITUD = 8;
    static constexpr int PUNTOS_MANZANA = 10;
    static constexpr int VACIO = 0;
    static constexpr int SERPIENTE = 1;
    static constexpr int MANZANA = 2;

    void crearGrid();
    void reiniciar();
    void actualizarMapa();
    void redibujar();
    void generarManzana();
    void actualizarInformacion();
    void avanzarJuego();
    void terminarJuego();
    void ganarNivel();

    QGraphicsScene *m_escena;
    QGraphicsTextItem *m_informacion;
    QGraphicsRectItem *m_casillas[FILAS][COLUMNAS];
    QTimer *m_temporizador;
    Snake *m_serpiente;
    Tablero *m_tablero;
    ProgresoNivel *m_progreso;
    int m_direccionX;
    int m_direccionY;
    int m_manzanaX;
    int m_manzanaY;
    bool m_terminado;
};

#endif // JUEGOVIEW_H
