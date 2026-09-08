#ifndef TUTORIAL_H
#define TUTORIAL_H

#include <QGraphicsView>
#include <QPoint>
#include <QString>

#include "fruta.h"
#include "item.h"
#include "controles.h"

class QGraphicsScene;
class QGraphicsTextItem;
class QKeyEvent;
class QPixmap;
class QMediaPlayer;
class QAudioOutput;
class QVideoSink;
class QGraphicsPixmapItem;
class QTimer;

class TutorialView : public QGraphicsView {
public:
    explicit TutorialView(const QString &usuario = {});
    ~TutorialView() override;

protected:
    void keyPressEvent(QKeyEvent *evento) override;

private:
    static constexpr int CELDA = 50;
    static constexpr int TABLERO = 10;
    static constexpr int PANEL_ANCHO = 220;
    static constexpr int MAX_SERPIENTE = 30;

    void dibujar();
    void mostrarOverlay(const QString &texto);
    void quitarOverlay();
    void actualizarInformacion();
    void avanzar();
    void siguienteDireccion(int x, int y);
    bool come(int x, int y);
    void reproducirExplosion();
    void reiniciarEtapa();

    QGraphicsScene *m_escena;
    QGraphicsTextItem *m_overlay;
    QTimer *m_timer;
    QPoint m_serpiente[MAX_SERPIENTE];
    int m_longitud;
    int m_direccionX;
    int m_direccionY;
    int m_fase;
    int m_turnos;
    bool m_manzana;
    bool m_dorada;
    bool m_grande;
    bool m_energetica;
    bool m_caja;
    int m_turnosEfecto;
    int m_puntaje;
    Fruta m_frutaActual;
    EsquemaControles m_esquemaControles;
    QString m_usuario;
    QPixmap *m_spriteCabeza;
    QPixmap *m_spriteCuerpo;
    QPixmap *m_spriteCola;
    QPixmap *m_spriteFrutaNormal;
    QPixmap *m_spriteFrutaDorada;
    QPixmap *m_spriteFrutaGrande;
    QPixmap *m_spriteFrutaEnergetica;
    QPixmap *m_spriteCaja;
    QMediaPlayer *m_explosionPlayer;
    QAudioOutput *m_explosionAudio;
    QVideoSink *m_explosionSink;
    QGraphicsPixmapItem *m_explosionItem;
    bool m_colisionando;
    bool m_overlayFrutasIniciales;
    bool m_overlayFrutasEspeciales;
    bool m_overlayCaja;
};

#endif
