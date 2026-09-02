#include "tutorial.h"

#include <QBrush>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QFont>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QPen>
#include <QPixmap>
#include <QImage>
#include <QPainter>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoSink>
#include <QVideoFrame>
#include <QUrl>
#include <QGraphicsPixmapItem>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QTimer>

TutorialView::TutorialView()
    : m_escena(new QGraphicsScene(this)),
      m_overlay(nullptr),
      m_timer(new QTimer(this)),
      m_longitud(3),
      m_direccionX(0),
      m_direccionY(0),
      m_fase(0),
      m_turnos(0),
      m_manzana(true),
      m_dorada(true),
      m_grande(false),
      m_energetica(false),
      m_caja(false),
      m_turnosEfecto(0),
      m_puntaje(0),
      m_frutaActual(),
      m_esquemaControles(EsquemaControles::Flechas),
      m_spriteCabeza(new QPixmap()),
      m_spriteCuerpo(new QPixmap()),
      m_spriteCola(new QPixmap()),
      m_explosionPlayer(new QMediaPlayer(this)),
      m_explosionAudio(new QAudioOutput(this)),
      m_explosionSink(new QVideoSink(this)),
      m_explosionItem(nullptr),
      m_colisionando(false),
      m_overlayFrutasIniciales(false),
      m_overlayFrutasEspeciales(false),
      m_overlayCaja(false) {
    QFontDatabase::addApplicationFont(":/assets/Fredoka-Variable.ttf");
    const QPixmap cabeza(":/assets/cabeza_snake.png");
    if (!cabeza.isNull()) {
        *m_spriteCabeza = cabeza.scaled(44, 44, Qt::KeepAspectRatio,
                                        Qt::SmoothTransformation);
    }
    QImage cuerpo(36, 36, QImage::Format_ARGB32_Premultiplied);
    cuerpo.fill(Qt::transparent);
    QPainter pintor(&cuerpo);
    pintor.setRenderHint(QPainter::Antialiasing);
    pintor.setPen(Qt::NoPen);
    pintor.setBrush(QColor(92, 190, 47));
    pintor.drawEllipse(2, 2, 32, 32);
    pintor.setBrush(QColor(46, 125, 38));
    pintor.drawEllipse(11, 8, 6, 6);
    pintor.drawEllipse(21, 19, 6, 6);
    *m_spriteCuerpo = QPixmap::fromImage(cuerpo);
    QImage cola(36, 36, QImage::Format_ARGB32_Premultiplied);
    cola.fill(Qt::transparent);
    QPainter pintorCola(&cola);
    pintorCola.setRenderHint(QPainter::Antialiasing);
    pintorCola.setPen(Qt::NoPen);
    pintorCola.setBrush(QColor(76, 167, 43));
    pintorCola.drawEllipse(5, 5, 26, 26);
    pintorCola.setBrush(QColor(46, 125, 38));
    pintorCola.drawEllipse(13, 10, 5, 5);
    pintorCola.drawEllipse(21, 20, 5, 5);
    *m_spriteCola = QPixmap::fromImage(cola);
    m_explosionPlayer->setAudioOutput(m_explosionAudio);
    m_explosionAudio->setVolume(0.35);
    m_explosionPlayer->setVideoSink(m_explosionSink);
    m_explosionPlayer->setSource(QUrl("qrc:/assets/deltarune_explosion.mp4"));
    connect(m_explosionSink, &QVideoSink::videoFrameChanged, this,
            [this](const QVideoFrame &videoFrame) {
                if (m_explosionItem == nullptr) return;
                QImage imagen = videoFrame.toImage().convertToFormat(QImage::Format_ARGB32);
                for (int y = 0; y < imagen.height(); ++y) {
                    for (int x = 0; x < imagen.width(); ++x) {
                        QColor pixel = imagen.pixelColor(x, y);
                        if (pixel.green() > 80
                            && pixel.green() > pixel.red() * 1.18
                            && pixel.green() > pixel.blue() * 1.18) {
                            pixel.setAlpha(0);
                        } else if (pixel.green() > pixel.red() * 1.08) {
                            pixel.setAlpha(qMax(0, pixel.alpha() - 100));
                        }
                        imagen.setPixelColor(x, y, pixel);
                    }
                }
                m_explosionItem->setPixmap(QPixmap::fromImage(
                    imagen.scaled(170, 170, Qt::KeepAspectRatio,
                                  Qt::SmoothTransformation)));
            });
    connect(m_explosionPlayer, &QMediaPlayer::errorOccurred, this,
            [this](QMediaPlayer::Error) {
                if (!m_colisionando) return;
                if (m_explosionItem != nullptr) {
                    delete m_explosionItem;
                    m_explosionItem = nullptr;
                }
                mostrarOverlay("¡CHOCASTE!\n\nLos obstáculos y tu propio cuerpo bloquean el camino.\n\nPresioná ENTER para reintentar esta etapa.");
            });
    setScene(m_escena);
    setFixedSize(TABLERO * CELDA + 4, TABLERO * CELDA + 4);
    setWindowTitle("Snake - Tutorial");
    setFocusPolicy(Qt::StrongFocus);
    m_escena->setSceneRect(0, 0, TABLERO * CELDA, TABLERO * CELDA);

    m_serpiente[0] = QPoint(3, 5);
    m_serpiente[1] = QPoint(2, 5);
    m_serpiente[2] = QPoint(1, 5);
    connect(m_timer, &QTimer::timeout, this, [this]() { avanzar(); });
    mostrarOverlay("TUTORIAL\n\nMové la serpiente con las flechas o con WASD.\nLa cabeza guía el movimiento y cada segmento la sigue.\n\nPresioná una dirección para comenzar.");
    dibujar();
}

TutorialView::~TutorialView() {
    m_explosionPlayer->stop();
    if (m_explosionItem != nullptr) delete m_explosionItem;
    delete m_spriteCabeza;
    delete m_spriteCuerpo;
    delete m_spriteCola;
}

void TutorialView::keyPressEvent(QKeyEvent *evento) {
    if (evento->key() == Qt::Key_Return || evento->key() == Qt::Key_Enter) {
        if (m_colisionando) {
            reiniciarEtapa();
            return;
        }
        if (m_overlayFrutasIniciales || m_overlayFrutasEspeciales || m_overlayCaja) {
            m_overlayFrutasIniciales = false;
            m_overlayFrutasEspeciales = false;
            m_overlayCaja = false;
            quitarOverlay();
            m_timer->start(170);
            return;
        }
        if (m_fase == 2) {
            ++m_fase;
            m_grande = true;
            m_energetica = true;
            m_turnos = 0;
            m_overlayFrutasEspeciales = true;
            mostrarOverlay("MÁS FRUTAS\n\nTambién existen frutas especiales.\nProbá sus efectos cuando desaparezca este mensaje.\n\nPresioná ENTER para continuar.");
            dibujar();
        } else if (m_fase == 4) {
            ++m_fase;
            m_caja = true;
            m_overlayCaja = true;
            mostrarOverlay("CAJA DE ÍTEMS\n\nLas cajas entregan efectos especiales para ayudarte o complicarte la partida.\n\nPresioná ENTER para probarla.");
            dibujar();
        } else if (m_fase == 6) {
            ++m_fase;
            quitarOverlay();
            dibujar();
            m_timer->start(250);
        } else if (m_fase == 7 || m_fase == 8) {
            close();
        }
        return;
    }

    int x = 0;
    int y = 0;
    if (evento->key() == Qt::Key_Left || evento->key() == Qt::Key_A) x = -1;
    if (evento->key() == Qt::Key_Right || evento->key() == Qt::Key_D) x = 1;
    if (evento->key() == Qt::Key_Up || evento->key() == Qt::Key_W) y = -1;
    if (evento->key() == Qt::Key_Down || evento->key() == Qt::Key_S) y = 1;
    if (x == 0 && y == 0) return;
    m_esquemaControles = Controles::detectar(evento->key());

    if (m_fase == 0) {
        m_fase = 1;
        m_overlayFrutasIniciales = true;
        mostrarOverlay("MANZANAS\n\nLa manzana roja suma puntos y hace crecer la serpiente.\nLa dorada cuenta como dos frutas y hace crecer dos segmentos.\n\nPresioná ENTER para probarlas.");
        m_timer->start(170);
        m_timer->stop();
        dibujar();
        return;
    }
    if (m_fase == 1 || m_fase == 3 || m_fase == 5 || m_fase == 7) {
        siguienteDireccion(x, y);
    }
}

void TutorialView::siguienteDireccion(int x, int y) {
    if (x != 0 && m_direccionX == -x) return;
    if (y != 0 && m_direccionY == -y) return;
    m_direccionX = x;
    m_direccionY = y;
}

bool TutorialView::come(int x, int y) {
    for (int i = 0; i < m_longitud; ++i) {
        if (m_serpiente[i] == QPoint(x, y)) return true;
    }
    return false;
}

void TutorialView::avanzar() {
    if (m_turnosEfecto > 0) {
        --m_turnosEfecto;
        if (m_turnosEfecto == 0) m_timer->setInterval(170);
    }
    if (m_fase == 7 && m_turnosEfecto == 0) {
        m_timer->stop();
        m_fase = 8;
        mostrarOverlay("TUTORIAL COMPLETADO\n\nYa conocés las frutas, obstáculos e ítems.\n\nPresioná ENTER para volver al Menú de Juego.");
        dibujar();
        return;
    }
    if (m_fase == 3 && !m_grande && !m_energetica && ++m_turnos >= 25) {
        m_timer->stop();
        m_fase = 4;
        mostrarOverlay("OBSTÁCULOS\n\nLos obstáculos bloquean casillas y pueden hacerte perder.\n\nPresioná ENTER para continuar.");
        dibujar();
        return;
    }

    if (m_direccionX == 0 && m_direccionY == 0) return;
    QPoint nueva = m_serpiente[0] + QPoint(m_direccionX, m_direccionY);
    if (nueva.x() < 0) nueva.setX(TABLERO - 1);
    if (nueva.x() >= TABLERO) nueva.setX(0);
    if (nueva.y() < 0) nueva.setY(TABLERO - 1);
    if (nueva.y() >= TABLERO) nueva.setY(0);
    const bool chocaConObstaculo = (m_fase == 5)
        && ((nueva == QPoint(4, 3)) || (nueva == QPoint(5, 3)));
    bool chocaConCuerpo = false;
    for (int i = 1; i < m_longitud; ++i) {
        if (m_serpiente[i] == nueva) {
            chocaConCuerpo = true;
            break;
        }
    }
    if (chocaConObstaculo || chocaConCuerpo) {
        reproducirExplosion();
        return;
    }
    for (int i = m_longitud - 1; i > 0; --i) {
        m_serpiente[i] = m_serpiente[i - 1];
    }
    m_serpiente[0] = nueva;

    if (m_fase == 1 && ((m_manzana && come(6, 5)) || (m_dorada && come(8, 5)))) {
        if (m_manzana && come(6, 5)) {
            m_manzana = false;
            m_frutaActual.configurar(TipoFruta::Normal);
            m_puntaje += m_frutaActual.puntos();
            if (m_longitud < MAX_SERPIENTE) ++m_longitud;
        }
        if (m_dorada && come(8, 5)) {
            m_dorada = false;
            m_frutaActual.configurar(TipoFruta::Dorada);
            m_puntaje += m_frutaActual.puntos();
            for (int i = 0; i < m_frutaActual.crecimiento(true)
                            && m_longitud < MAX_SERPIENTE; ++i) ++m_longitud;
        }
        if (!m_manzana && !m_dorada) {
            m_timer->stop();
            m_fase = 2;
            mostrarOverlay("SIGUIENTE ETAPA\n\nPresioná ENTER para conocer las frutas especiales.");
        }
    } else if (m_fase == 3 && ((m_grande && come(7, 2)) || (m_energetica && come(9, 2)))) {
        if (m_grande && come(7, 2)) {
            m_grande = false;
            m_frutaActual.configurar(TipoFruta::Grande);
            m_puntaje += m_frutaActual.puntos();
            for (int i = 0; i < m_frutaActual.crecimiento(true)
                            && m_longitud < MAX_SERPIENTE; ++i) ++m_longitud;
        }
        if (m_energetica && come(9, 2)) {
            m_energetica = false;
            m_frutaActual.configurar(TipoFruta::Energetica);
            m_puntaje += m_frutaActual.puntos();
            for (int i = 0; i < m_frutaActual.crecimiento(true)
                            && m_longitud < MAX_SERPIENTE; ++i) ++m_longitud;
            m_turnosEfecto = 30;
            m_timer->setInterval(130);
        }
        if (!m_grande && !m_energetica) {
            m_timer->stop();
        m_fase = 4;
        mostrarOverlay("OBSTÁCULOS\n\nLos obstáculos bloquean casillas y pueden hacerte perder.\n\nPresioná ENTER para continuar.");
        }
    } else if (m_fase == 5 && m_caja && come(7, 7)) {
        m_caja = false;
        m_timer->stop();
        m_fase = 6;
        m_turnosEfecto = 10;
        m_timer->setInterval(250);
        mostrarOverlay("EFECTO OBTENIDO\n\nObtuviste Hielo: la serpiente va más lento durante 10 turnos.\nAdemás de Hielo, existen Reloj, Rayo, Tijeras, Bomba y Trampa.\n\nPresioná ENTER para continuar.");
    }
    dibujar();
}

void TutorialView::reproducirExplosion() {
    m_colisionando = true;
    m_timer->stop();
    QObject::disconnect(m_explosionPlayer, &QMediaPlayer::mediaStatusChanged,
                        this, nullptr);
    if (m_explosionItem != nullptr) delete m_explosionItem;
    m_explosionItem = m_escena->addPixmap(QPixmap());
    m_explosionItem->setZValue(20);
    m_explosionItem->setPos(m_serpiente[0].x() * CELDA - 60,
                            m_serpiente[0].y() * CELDA - 60);
    m_explosionPlayer->stop();
    m_explosionPlayer->setPosition(0);
    connect(m_explosionPlayer, &QMediaPlayer::mediaStatusChanged, this,
            [this](QMediaPlayer::MediaStatus estado) {
                if (estado == QMediaPlayer::EndOfMedia && m_colisionando) {
                    if (m_explosionItem != nullptr) {
                        delete m_explosionItem;
                        m_explosionItem = nullptr;
                    }
                    mostrarOverlay("¡CHOCASTE!\n\nLos obstáculos y tu propio cuerpo bloquean el camino.\n\nPresioná ENTER para reintentar esta etapa.");
                }
            });
    QTimer::singleShot(220, this, [this]() {
        if (m_colisionando) m_explosionPlayer->play();
    });
    QTimer::singleShot(2600, this, [this]() {
        if (m_colisionando && m_explosionPlayer->mediaStatus() != QMediaPlayer::EndOfMedia) {
            if (m_explosionItem != nullptr) {
                delete m_explosionItem;
                m_explosionItem = nullptr;
            }
            mostrarOverlay("¡CHOCASTE!\n\nLos obstáculos y tu propio cuerpo bloquean el camino.\n\nPresioná ENTER para reintentar esta etapa.");
        }
    });
}

void TutorialView::reiniciarEtapa() {
    m_explosionPlayer->stop();
    if (m_explosionItem != nullptr) {
        delete m_explosionItem;
        m_explosionItem = nullptr;
    }
    m_serpiente[0] = QPoint(3, 5);
    m_serpiente[1] = QPoint(2, 5);
    m_serpiente[2] = QPoint(1, 5);
    m_longitud = 3;
    m_direccionX = 0;
    m_direccionY = 0;
    m_turnos = 0;
    if (m_fase == 1) {
        m_manzana = true;
        m_dorada = true;
    } else if (m_fase == 3) {
        m_grande = true;
        m_energetica = true;
    } else if (m_fase == 5) {
        m_caja = true;
    }
    m_colisionando = false;
    quitarOverlay();
    dibujar();
    m_timer->start(170);
}

void TutorialView::mostrarOverlay(const QString &texto) {
    quitarOverlay();
    QPainterPath mascara;
    mascara.addRect(0, 0, TABLERO * CELDA, TABLERO * CELDA);
    const bool explicarFrutasIniciales = texto.startsWith("MANZANAS");
    const bool explicarFrutasEspeciales = texto.startsWith("MÁS FRUTAS");
    const bool explicarFrutas = explicarFrutasIniciales || explicarFrutasEspeciales;
    const bool explicarObstaculos = texto.startsWith("OBSTÁCULOS");
    const bool explicarCaja = texto.startsWith("CAJA DE ÍTEMS");
    if (explicarFrutas) {
        const int xPrimera = explicarFrutasEspeciales ? 7 : 6;
        const int yPrimera = explicarFrutasEspeciales ? 2 : 5;
        const int xSegunda = explicarFrutasEspeciales ? 9 : 8;
        const int ySegunda = explicarFrutasEspeciales ? 2 : 5;
        QPainterPath huecoRojo;
        huecoRojo.addEllipse(xPrimera * CELDA + 3, yPrimera * CELDA + 3, 44, 44);
        mascara -= huecoRojo;
        QPainterPath huecoDorado;
        huecoDorado.addEllipse(xSegunda * CELDA + 3, ySegunda * CELDA + 3, 44, 44);
        mascara -= huecoDorado;
    } else if (explicarObstaculos) {
        QPainterPath hueco;
        hueco.addRect(4 * CELDA, 3 * CELDA, 2 * CELDA, CELDA);
        mascara -= hueco;
    } else if (explicarCaja) {
        QPainterPath hueco;
        hueco.addEllipse(7 * CELDA + 2, 7 * CELDA + 2, 46, 46);
        mascara -= hueco;
    }
    auto *fondo = m_escena->addPath(mascara, Qt::NoPen,
                                    QBrush(QColor(5, 8, 12, 115)));
    fondo->setZValue(10);
    m_overlay = m_escena->addText(texto);
    m_overlay->setDefaultTextColor(Qt::white);
    m_overlay->setFont(QFont("Fredoka", 14));
    m_overlay->setTextWidth(300);
    m_overlay->setPos(30, 35);
    m_overlay->setZValue(11);

    if (explicarFrutas) {
        const int xPrimera = explicarFrutasEspeciales ? 7 : 6;
        const int yPrimera = explicarFrutasEspeciales ? 2 : 5;
        const int xSegunda = explicarFrutasEspeciales ? 9 : 8;
        const int ySegunda = explicarFrutasEspeciales ? 2 : 5;
        const QString nombrePrimero = explicarFrutasEspeciales ? "GRANDE" : "ROJA";
        const QString nombreSegundo = explicarFrutasEspeciales ? "ENERGÉTICA" : "DORADA";
        const QColor colorPrimero = explicarFrutasEspeciales
            ? QColor(255, 180, 100) : QColor(255, 130, 130);
        const QColor colorSegundo = explicarFrutasEspeciales
            ? QColor(220, 140, 255) : QColor(255, 230, 90);
        auto *marcaRoja = m_escena->addEllipse(xPrimera * CELDA + 3, yPrimera * CELDA + 3,
                                               44, 44, QPen(colorPrimero, 3));
        marcaRoja->setZValue(12);
        auto *marcaDorada = m_escena->addEllipse(xSegunda * CELDA + 3, ySegunda * CELDA + 3,
                                                 44, 44, QPen(colorSegundo, 3));
        marcaDorada->setZValue(12);
        auto *etiquetaRoja = m_escena->addText(nombrePrimero);
        etiquetaRoja->setDefaultTextColor(colorPrimero);
        etiquetaRoja->setPos(xPrimera * CELDA + 50, yPrimera * CELDA);
        etiquetaRoja->setZValue(12);
        m_escena->addLine(xPrimera * CELDA + 40, yPrimera * CELDA + 25,
                          xPrimera * CELDA + 50, yPrimera * CELDA + 15,
                          QPen(colorPrimero, 2))->setZValue(12);
        auto *etiquetaDorada = m_escena->addText(nombreSegundo);
        etiquetaDorada->setDefaultTextColor(colorSegundo);
        etiquetaDorada->setPos(xSegunda * CELDA - 65, ySegunda * CELDA + 50);
        etiquetaDorada->setZValue(12);
        m_escena->addLine(xSegunda * CELDA + 20, ySegunda * CELDA + 40,
                          xSegunda * CELDA, ySegunda * CELDA + 50,
                          QPen(colorSegundo, 2))->setZValue(12);
    }
    if (explicarObstaculos) {
        auto *marca = m_escena->addEllipse(4 * CELDA + 2, 3 * CELDA + 2,
                                           2 * CELDA - 4, CELDA - 4,
                                           QPen(QColor(255, 180, 80), 3));
        marca->setZValue(12);
        auto *etiqueta = m_escena->addText("OBSTÁCULO");
        etiqueta->setDefaultTextColor(QColor(255, 190, 100));
        etiqueta->setPos(4 * CELDA + 110, 3 * CELDA);
        etiqueta->setZValue(12);
        m_escena->addLine(4 * CELDA + 95, 3 * CELDA + 25,
                          4 * CELDA + 110, 3 * CELDA + 15,
                          QPen(QColor(255, 190, 100), 2))->setZValue(12);
    }
    if (explicarCaja) {
        auto *marca = m_escena->addEllipse(7 * CELDA + 2, 7 * CELDA + 2,
                                           46, 46, QPen(QColor(120, 210, 255), 3));
        marca->setZValue(12);
        auto *etiqueta = m_escena->addText("CAJA");
        etiqueta->setDefaultTextColor(QColor(120, 210, 255));
        etiqueta->setPos(7 * CELDA + 60, 7 * CELDA);
        etiqueta->setZValue(12);
        m_escena->addLine(7 * CELDA + 48, 7 * CELDA + 25,
                          7 * CELDA + 60, 7 * CELDA + 15,
                          QPen(QColor(120, 210, 255), 2))->setZValue(12);
    }
}

void TutorialView::quitarOverlay() {
    if (m_overlay != nullptr) {
        delete m_overlay;
        m_overlay = nullptr;
    }
    const QList<QGraphicsItem *> elementos = m_escena->items();
    for (QGraphicsItem *elemento : elementos) {
        if (elemento->zValue() >= 10) delete elemento;
    }
}

void TutorialView::dibujar() {
    const QList<QGraphicsItem *> elementos = m_escena->items();
    for (QGraphicsItem *elemento : elementos) {
        if (elemento->zValue() < 10) delete elemento;
    }
    for (int y = 0; y < TABLERO; ++y) {
        for (int x = 0; x < TABLERO; ++x) {
            m_escena->addRect(x * CELDA, y * CELDA, CELDA, CELDA,
                              QPen(QColor(48, 58, 74)), QBrush(QColor(21, 28, 38)));
        }
    }
    if (m_manzana) m_escena->addEllipse(6 * CELDA + 8, 5 * CELDA + 8, 34, 34,
                                         QPen(Qt::NoPen), QBrush(QColor(231, 76, 60)));
    if (m_dorada) m_escena->addEllipse(8 * CELDA + 8, 5 * CELDA + 8, 34, 34,
                                        QPen(Qt::NoPen), QBrush(QColor(241, 196, 15)));
    if (m_grande) m_escena->addEllipse(7 * CELDA + 8, 2 * CELDA + 8, 34, 34,
                                       QPen(Qt::NoPen), QBrush(QColor(239, 139, 61)));
    if (m_energetica) m_escena->addEllipse(9 * CELDA + 8, 2 * CELDA + 8, 34, 34,
                                           QPen(Qt::NoPen), QBrush(QColor(174, 91, 214)));
    if (m_fase == 4 || m_fase == 5) {
        m_escena->addRect(4 * CELDA, 3 * CELDA, CELDA, CELDA, Qt::NoPen,
                          QBrush(QColor(116, 82, 58)));
        m_escena->addRect(5 * CELDA, 3 * CELDA, CELDA, CELDA, Qt::NoPen,
                          QBrush(QColor(116, 82, 58)));
    }
    if (m_caja) m_escena->addRect(7 * CELDA + 8, 7 * CELDA + 8, 34, 34,
                                   QPen(Qt::NoPen), QBrush(QColor(92, 180, 230)));
    for (int i = m_longitud - 1; i >= 0; --i) {
        const QPixmap *sprite = i == 0 ? m_spriteCabeza
                              : i == m_longitud - 1 ? m_spriteCola : m_spriteCuerpo;
        if (sprite == nullptr || sprite->isNull()) continue;
        auto *pieza = m_escena->addPixmap(*sprite);
        pieza->setPos(m_serpiente[i].x() * CELDA + (i == 0 ? -2 : 2),
                      m_serpiente[i].y() * CELDA + (i == 0 ? -2 : 2));
        if (i == 0) {
            pieza->setTransformOriginPoint(pieza->boundingRect().center());
            if (m_direccionX < 0) pieza->setRotation(270);
            else if (m_direccionX > 0) pieza->setRotation(90);
            else if (m_direccionY > 0) pieza->setRotation(180);
        }
    }
}
