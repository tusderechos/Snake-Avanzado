#ifndef JUEGOVIEW_H
#define JUEGOVIEW_H

#include <QGraphicsView>

#include <memory>

class QGraphicsRectItem;
class QGraphicsScene;
class QGraphicsTextItem;
class QGraphicsPixmapItem;
class QKeyEvent;
class QTimer;
class QPixmap;
class ProgresoNivel;
class Snake;
class Tablero;

class JuegoView : public QGraphicsView {
public:
    JuegoView();
    ~JuegoView() override;

protected:
    void keyPressEvent(QKeyEvent *evento) override;

private:
    static constexpr int MAX_COLUMNAS = 20;
    static constexpr int MAX_FILAS = 20;
    static constexpr int MAX_OBSTACULOS_MOVILES = 6;
    static constexpr int TAMANO_CELDA = 40;
    static constexpr int INTERVALO_NIVEL_1 = 150;
    static constexpr int INTERVALO_NIVEL_2 = 100;
    static constexpr int INTERVALO_NIVEL_3 = 50;
    static constexpr int NIVEL_1 = 1;
    static constexpr int NIVEL_2 = 2;
    static constexpr int NIVEL_3 = 3;
    static constexpr int META_FRUTAS_NIVEL_1 = 7;
    static constexpr int META_PUNTOS_NIVEL_1 = 70;
    static constexpr int META_LONGITUD_NIVEL_1 = 10;
    static constexpr int META_FRUTAS_NIVEL_2 = 10;
    static constexpr int META_PUNTOS_NIVEL_2 = 100;
    static constexpr int META_LONGITUD_NIVEL_2 = 13;
    static constexpr int META_FRUTAS_NIVEL_3 = 12;
    static constexpr int META_PUNTOS_NIVEL_3 = 120;
    static constexpr int META_LONGITUD_NIVEL_3 = 15;
    static constexpr int PUNTOS_MANZANA = 10;
    static constexpr int VACIO = 0;
    static constexpr int SERPIENTE = 1;
    static constexpr int MANZANA = 2;
    static constexpr int OBSTACULO = 3;

    void crearGrid();
    void cargarSprites();
    void configurarNivel(int nivel);
    void inicializarObstaculosMoviles();
    void construirObstaculos();
    void moverObstaculos();
    bool posicionObstaculoDisponible(int x, int y, int ignorar) const;
    void reiniciar();
    void actualizarMapa();
    void redibujar();
    void generarManzana();
    void actualizarInformacion();
    void avanzarJuego();
    void terminarJuego();
    void ganarNivel();
    int intervaloActual() const;
    bool esNivelConBordesMortales() const;

    QGraphicsScene *m_escena;
    QGraphicsTextItem *m_informacion;
    QGraphicsRectItem *m_casillas[MAX_FILAS][MAX_COLUMNAS];
    QGraphicsPixmapItem *m_sprites[MAX_FILAS][MAX_COLUMNAS];
    QTimer *m_temporizador;
    std::unique_ptr<Snake> m_serpiente;
    std::unique_ptr<Tablero> m_tablero;
    std::unique_ptr<ProgresoNivel> m_progreso;
    int m_direccionX;
    int m_direccionY;
    int m_manzanaX;
    int m_manzanaY;
    int m_nivel;
    int m_columnas;
    int m_filas;
    int m_metaFrutas;
    int m_metaPuntos;
    int m_metaLongitud;
    int m_obstaculoX[MAX_OBSTACULOS_MOVILES];
    int m_obstaculoY[MAX_OBSTACULOS_MOVILES];
    int m_obstaculoDireccionX[MAX_OBSTACULOS_MOVILES];
    int m_obstaculoDireccionY[MAX_OBSTACULOS_MOVILES];
    int m_turnoObstaculos;
    bool m_cambioDireccionPendiente;
    bool m_terminado;
    QPixmap *m_spriteCabeza;
    QPixmap *m_spriteCuerpo;
    QPixmap *m_spriteCola;
};

#endif // JUEGOVIEW_H
