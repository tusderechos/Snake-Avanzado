#ifndef JUEGOVIEW_H
#define JUEGOVIEW_H

#include <QGraphicsView>
#include <QHash>
#include <QList>
#include <QPointF>
#include <QString>

#include "fruta.h"
#include "gestorpartida.h"
#include "obstaculo.h"
#include "item.h"
#include "modojuego.h"
#include "controles.h"
#include <memory>

class QGraphicsRectItem;
class QGraphicsScene;
class QGraphicsTextItem;
class QGraphicsPixmapItem;
class QKeyEvent;
class QCloseEvent;
class QTimer;
class QPixmap;
class QMediaPlayer;
class QAudioOutput;
class QVideoSink;
class ProgresoNivel;
class Snake;
class Tablero;
class QVariantAnimation;

class JuegoView : public QGraphicsView {
public:
    explicit JuegoView(int nivelInicial = 1,
                       ConfiguracionJuego configuracion = {},
                       const QString &usuario = {});
    ~JuegoView() override;

protected:
    void keyPressEvent(QKeyEvent *evento) override;
    void closeEvent(QCloseEvent *evento) override;

private:
    static constexpr int MAX_COLUMNAS = 20;
    static constexpr int MAX_FILAS = 20;
    static constexpr int MAX_OBSTACULOS_MOVILES = 6;
    static constexpr int MAX_OBJETOS_ACTIVOS = 3;
    static constexpr int TAMANO_CELDA = 40;
    static constexpr int PANEL_ANCHO = 250;
    static constexpr int INTERVALO_NIVEL_1 = 150;
    static constexpr int INTERVALO_NIVEL_2 = 100;
    static constexpr int INTERVALO_NIVEL_3 = 50;
    static constexpr int TIEMPO_INICIAL_SEGUNDOS = 300;
    static constexpr int COOLDOWN_CAJA_TURNOS = 5;
    static constexpr int GARANTIA_CAJA_TURNOS = 20;
    static constexpr int GARANTIA_CAJA_FRUTAS = 4;
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
    static constexpr int MANZANA_DORADA = 4;
    static constexpr int CAJA_MISTERIOSA = 5;
    static constexpr int FRUTA_GRANDE = 6;
    static constexpr int FRUTA_ENERGETICA = 7;

    void crearGrid();
    void ajustarVistaAlMonitor();
    void iniciarAnimacionPuntaje();
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
    int indiceObjetoEn(int x, int y) const;
    TipoFruta frutaAleatoriaParaNivel() const;
    void aplicarItem();
    void actualizarInformacion();
    void avanzarJuego();
    void terminarJuego();
    void reproducirExplosion();
    void mostrarTarjetaDerrota();
    void ganarNivel();
    int intervaloActual() const;
    bool esNivelConBordesMortales() const;
    void guardarPuntajePartida();

    QGraphicsScene *m_escena;
    QGraphicsTextItem *m_informacion;
    QGraphicsRectItem *m_casillas[MAX_FILAS][MAX_COLUMNAS];
    QGraphicsPixmapItem *m_sprites[MAX_FILAS][MAX_COLUMNAS];
    QTimer *m_temporizador;
    QTimer *m_animadorPuntaje;
    std::unique_ptr<Snake> m_serpiente;
    std::unique_ptr<Tablero> m_tablero;
    std::unique_ptr<ProgresoNivel> m_progreso;
    int m_direccionX;
    int m_direccionY;
    int m_manzanaX;
    int m_manzanaY;
    int m_tipoObjeto;
    Fruta m_frutaActual;
    struct ObjetoActivo {
        bool activo = false;
        int x = -1;
        int y = -1;
        int tipo = VACIO;
        Fruta fruta;
    };
    ObjetoActivo m_objetos[MAX_OBJETOS_ACTIVOS];
    int m_nivel;
    int m_columnas;
    int m_filas;
    int m_margenColiseo;
    int m_metaFrutas;
    int m_metaPuntos;
    int m_metaLongitud;
    Obstaculo m_obstaculos[MAX_OBSTACULOS_MOVILES];
    int m_turnoObstaculos;
    int m_turnosObstaculosCongelados;
    int m_turnosHielo;
    int m_turnosEnergia;
    int m_tiempoExtraSegundos;
    int m_tiempoRestanteSegundos;
    int m_milisegundosTiempo;
    int m_turnosTrampa;
    int m_trampaX;
    int m_trampaY;
    int m_turnosDesdeCaja;
    int m_frutasDesdeCaja;
    QString m_ultimoEfecto;
    QString m_usuario;
    int m_puntajePartida;
    int m_puntajeVisual;
    int m_nivelesCompletados;
    int m_puntajeRegistrado;
    int m_bonusMonedasRegistrado;
    bool m_cambioDireccionPendiente;
    EsquemaControles m_esquemaControles;
    bool m_terminado;
    ConfiguracionJuego m_configuracion;
    GestorPartida m_gestorPartida;
    QPixmap *m_spriteCabeza;
    QPixmap *m_spriteCuerpo;
    QPixmap *m_spriteCola;
    QPixmap *m_spriteFrutaNormal;
    QPixmap *m_spriteFrutaDorada;
    QPixmap *m_spriteFrutaGrande;
    QPixmap *m_spriteFrutaEnergetica;
    QPixmap *m_spriteCaja;
    QPixmap *m_fondoNivel;
    QMediaPlayer *m_explosionPlayer;
    QAudioOutput *m_explosionAudio;
    QVideoSink *m_explosionSink;
    QGraphicsPixmapItem *m_explosionItem;
    bool m_tarjetaDerrotaMostrada;
    QHash<int, QPointF> m_posicionesVisuales;
    QList<QVariantAnimation *> m_animacionesMovimiento;
};

#endif // JUEGOVIEW_H
