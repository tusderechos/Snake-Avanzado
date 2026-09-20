#include "juegoview.h"
#include "gestorconfiguracion.h"
#include "gestorusuarios.h"
#include "animadorserpiente.h"
#include "audio.h"

#include "progreso.h"
#include "reglasmovimiento.h"
#include "snake.h"
#include "tablero.h"
#include "skins.h"
#include "dialogos.h"

#include <QBrush>
#include <QGuiApplication>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QMessageBox>
#include <QPen>
#include <QRandomGenerator>
#include <QScreen>
#include <QTimer>
#include <QDateTime>
#include <QThread>
#include <QMetaObject>
#include <QVariantAnimation>
#include <QFont>
#include <QFontDatabase>
#include <QImage>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QAbstractButton>
#include <QGraphicsProxyWidget>
#include <QPushButton>
#include <QVideoSink>
#include <QVideoFrame>
#include <QPainter>
#include <QPixmap>
#include <QTransform>
#include <utility>

JuegoView::JuegoView(int nivelInicial, ConfiguracionJuego configuracion,
                     const QString &usuario)
    : m_escena(new QGraphicsScene(this)),
      m_informacion(nullptr),
      m_hiloAnimacion(new QThread(this)),
      m_animadorSerpiente(new AnimadorSerpiente()),
      m_temporizadorCuentaRegresiva(new QTimer(this)),
      m_animadorPuntaje(new QTimer(this)),
      m_temporizadorFrutas(new QTimer(this)),
      m_cuentaRegresiva(nullptr),
      m_cuentaRegresivaValor(0),
      m_serpiente(new Snake()),
      m_tablero(new Tablero(10, 10)),
      m_progreso(new ProgresoNivel(META_FRUTAS_NIVEL_1,
                                   META_PUNTOS_NIVEL_1,
                                   META_LONGITUD_NIVEL_1)),
      m_direccionX(1),
      m_direccionY(0),
      m_nivel(NIVEL_1),
      m_columnas(10),
      m_filas(10),
      m_margenColiseo(0),
      m_metaFrutas(META_FRUTAS_NIVEL_1),
      m_metaPuntos(META_PUNTOS_NIVEL_1),
      m_metaLongitud(META_LONGITUD_NIVEL_1),
      m_turnoObstaculos(0),
      m_turnosObstaculosCongelados(0),
      m_turnosHielo(0),
      m_turnosEnergia(0),
      m_tiempoExtraSegundos(0),
      m_tiempoRestanteSegundos(TIEMPO_INICIAL_SEGUNDOS),
      m_milisegundosTiempo(0),
      m_turnosTrampa(0),
      m_trampaX(-1),
      m_trampaY(-1),
      m_turnosDesdeCaja(0),
      m_frutasDesdeCaja(0),
      m_ultimoEfecto(),
      m_usuario(usuario),
      m_puntajePartida(0),
      m_puntajeVisual(0),
      m_nivelesCompletados(0),
      m_puntajeRegistrado(0),
      m_bonusMonedasRegistrado(0),
      m_cambioDireccionPendiente(false),
      m_esquemaControles(EsquemaControles::Flechas),
      m_terminado(false),
      m_configuracion(configuracion),
      m_spriteCabeza(new QPixmap()),
      m_spriteCuerpo(new QPixmap()),
      m_spriteCola(new QPixmap()),
      m_spriteFrutaNormal(new QPixmap()),
      m_spriteFrutaDorada(new QPixmap()),
      m_spriteFrutaGrande(new QPixmap()),
      m_spriteFrutaEnergetica(new QPixmap()),
      m_spriteCaja(new QPixmap()),
      m_fondoNivel(new QPixmap()),
      m_explosionPlayer(new QMediaPlayer(this)),
      m_explosionAudio(new QAudioOutput(this)),
      m_explosionSink(new QVideoSink(this)),
      m_explosionItem(nullptr),
      m_overlayPausa(nullptr),
      m_shrekPausa(nullptr),
      m_textoPausa(nullptr),
      m_proxyReanudar(nullptr),
      m_botonReanudar(nullptr),
      m_proxyMenuPrincipal(nullptr),
      m_botonMenuPrincipal(nullptr),
      m_tarjetaDerrotaMostrada(false)
{
    m_esquemaControles = GestorConfiguracion::cargarControl(m_usuario) == "WASD"
        ? EsquemaControles::WASD : EsquemaControles::Flechas;
    setScene(m_escena);
    // El tablero actualiza muchos elementos con frecuencia y casi nunca hace
    // búsquedas espaciales. Desactivar el índice BSP evita reconstruirlo en
    // cada movimiento y reduce el coste del repintado.
    m_escena->setItemIndexMethod(QGraphicsScene::NoIndex);
    m_escena->setBackgroundBrush(QColor(13, 18, 26));
    setFocusPolicy(Qt::StrongFocus);
    QFontDatabase::addApplicationFont(":/assets/Fredoka-Variable.ttf");

    configurarNivel(nivelInicial);
    cargarSprites();
    crearGrid();
    reiniciar();

    m_explosionPlayer->setAudioOutput(m_explosionAudio);
    // La derrota usa game_over.mp3; el video conserva únicamente su imagen.
    m_explosionAudio->setVolume(0.0);
    m_explosionPlayer->setVideoSink(m_explosionSink);
    m_explosionPlayer->setSource(QUrl("qrc:/assets/deltarune_explosion.mp4"));
    connect(m_explosionSink, &QVideoSink::videoFrameChanged, this,
            [this](const QVideoFrame &videoFrame) {
                if (m_explosionItem == nullptr) return;
                QImage imagen = videoFrame.toImage().convertToFormat(QImage::Format_ARGB32);
                for (int y = 0; y < imagen.height(); ++y) {
                    for (int x = 0; x < imagen.width(); ++x) {
                        QColor pixel = imagen.pixelColor(x, y);
                        const bool verde = pixel.green() > 80
                            && pixel.green() > pixel.red() * 1.18
                            && pixel.green() > pixel.blue() * 1.18;
                        if (verde) {
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
    connect(m_explosionPlayer, &QMediaPlayer::mediaStatusChanged, this,
            [this](QMediaPlayer::MediaStatus estado) {
                if (estado == QMediaPlayer::EndOfMedia) {
                    if (m_explosionItem != nullptr) {
                        delete m_explosionItem;
                        m_explosionItem = nullptr;
                    }
                    mostrarTarjetaDerrota();
                }
            });
    connect(m_explosionPlayer, &QMediaPlayer::errorOccurred, this,
            [this](QMediaPlayer::Error) {
                if (!m_terminado) return;
                if (m_explosionItem != nullptr) {
                    delete m_explosionItem;
                    m_explosionItem = nullptr;
                }
                mostrarTarjetaDerrota();
            });

    m_animadorSerpiente->moveToThread(m_hiloAnimacion);
    connect(m_hiloAnimacion, &QThread::finished,
            m_animadorSerpiente, &QObject::deleteLater);
    connect(m_animadorSerpiente, &AnimadorSerpiente::tick, this,
            [this]() { avanzarJuego(); }, Qt::QueuedConnection);
    m_hiloAnimacion->start();
    connect(m_temporizadorCuentaRegresiva, &QTimer::timeout, this,
            [this]() { actualizarCuentaRegresiva(); });
    connect(m_animadorPuntaje, &QTimer::timeout, this, [this]() {
        if (m_progreso == nullptr) {
            m_animadorPuntaje->stop();
            return;
        }
        const int objetivo = m_progreso->puntaje();
        if (m_puntajeVisual >= objetivo) {
            m_puntajeVisual = objetivo;
            m_animadorPuntaje->stop();
            actualizarInformacion();
            return;
        }
        const int incremento = objetivo - m_puntajeVisual > 40 ? 2 : 1;
        m_puntajeVisual = qMin(objetivo, m_puntajeVisual + incremento);
        actualizarInformacion();
    });
    m_temporizadorFrutas->setInterval(100);
    connect(m_temporizadorFrutas, &QTimer::timeout, this,
            [this]() { actualizarTemporizadoresFrutas(); });
    iniciarCuentaRegresiva();
}

JuegoView::~JuegoView() {
    detenerAnimacionEnHilo();
    m_hiloAnimacion->quit();
    m_hiloAnimacion->wait();
    delete m_spriteCabeza;
    delete m_spriteCuerpo;
    delete m_spriteCola;
    delete m_spriteFrutaNormal;
    delete m_spriteFrutaDorada;
    delete m_spriteFrutaGrande;
    delete m_spriteFrutaEnergetica;
    delete m_spriteCaja;
    delete m_fondoNivel;
}

void JuegoView::keyPressEvent(QKeyEvent *evento) {
    if (evento->key() == Qt::Key_Escape) {
        alternarPausa();
        return;
    }
    if (m_pausado) return;
    if (Controles::esMovimiento(evento->key())
        && !Controles::esTeclaPermitida(evento->key(), m_esquemaControles)) {
        return;
    }

    if ((evento->key() == Qt::Key_Left || evento->key() == Qt::Key_A)
        && !m_cambioDireccionPendiente && m_direccionX != 1) {
        m_direccionX = -1;
        m_direccionY = 0;
        m_cambioDireccionPendiente = true;
    } else if ((evento->key() == Qt::Key_Right || evento->key() == Qt::Key_D)
               && !m_cambioDireccionPendiente && m_direccionX != -1) {
        m_direccionX = 1;
        m_direccionY = 0;
        m_cambioDireccionPendiente = true;
    } else if ((evento->key() == Qt::Key_Up || evento->key() == Qt::Key_W)
               && !m_cambioDireccionPendiente && m_direccionY != 1) {
        m_direccionX = 0;
        m_direccionY = -1;
        m_cambioDireccionPendiente = true;
    } else if ((evento->key() == Qt::Key_Down || evento->key() == Qt::Key_S)
               && !m_cambioDireccionPendiente && m_direccionY != -1) {
        m_direccionX = 0;
        m_direccionY = 1;
        m_cambioDireccionPendiente = true;
    } else {
        QGraphicsView::keyPressEvent(evento);
    }
}

void JuegoView::iniciarCuentaRegresiva() {
    detenerAnimacionEnHilo();
    m_temporizadorCuentaRegresiva->stop();
    m_cuentaRegresivaValor = 3;

    if (m_cuentaRegresiva == nullptr) {
        m_cuentaRegresiva = m_escena->addText("");
        m_cuentaRegresiva->setDefaultTextColor(Qt::white);
        m_cuentaRegresiva->setFont(QFont("Fredoka", 72, QFont::Bold));
        m_cuentaRegresiva->setZValue(20);
    }
    m_cuentaRegresiva->setVisible(true);
    actualizarCuentaRegresiva();
    m_temporizadorCuentaRegresiva->start(1000);
}

void JuegoView::actualizarCuentaRegresiva() {
    if (m_cuentaRegresiva == nullptr) return;

    if (m_cuentaRegresivaValor > 0) {
        m_cuentaRegresiva->setPlainText(QString::number(m_cuentaRegresivaValor));
        const QRectF areaJuego(m_margenColiseo, m_margenColiseo,
                               m_columnas * TAMANO_CELDA,
                               m_filas * TAMANO_CELDA);
        m_cuentaRegresiva->setPos(
            areaJuego.center().x() - m_cuentaRegresiva->boundingRect().width() / 2,
            areaJuego.center().y() - m_cuentaRegresiva->boundingRect().height() / 2);
        --m_cuentaRegresivaValor;
        return;
    }

    m_cuentaRegresiva->setPlainText("0");
    const QRectF areaJuego(m_margenColiseo, m_margenColiseo, m_columnas * TAMANO_CELDA, m_filas * TAMANO_CELDA);
    m_cuentaRegresiva->setPos(
        areaJuego.center().x() - m_cuentaRegresiva->boundingRect().width() / 2,
        areaJuego.center().y() - m_cuentaRegresiva->boundingRect().height() / 2);
    m_temporizadorCuentaRegresiva->stop();
    QTimer::singleShot(450, this, [this]() {
        if (m_cuentaRegresivaValor != 0 || m_terminado) return;
        if (m_cuentaRegresiva != nullptr) m_cuentaRegresiva->setVisible(false);
        const qint64 ahora = QDateTime::currentMSecsSinceEpoch();
        for (ObjetoActivo &objeto : m_objetos) {
            if (objeto.activo && objeto.tipo != CAJA_MISTERIOSA) {
                objeto.creadaEnMs = ahora;
            }
        }
        m_temporizadorFrutas->start();
        iniciarAnimacionEnHilo();
    });
}

void JuegoView::iniciarAnimacionEnHilo() {
    QMetaObject::invokeMethod(m_animadorSerpiente, "iniciar",
                              Qt::QueuedConnection,
                              Q_ARG(int, intervaloActual()));
}

void JuegoView::detenerAnimacionEnHilo() {
    if (m_hiloAnimacion != nullptr && m_hiloAnimacion->isRunning()) {
        QMetaObject::invokeMethod(m_animadorSerpiente, "detener",
                                  Qt::BlockingQueuedConnection);
    }
}

void JuegoView::cambiarIntervaloEnHilo(int intervalo) {
    QMetaObject::invokeMethod(m_animadorSerpiente, "cambiarIntervalo",
                              Qt::QueuedConnection,
                              Q_ARG(int, intervalo));
}

void JuegoView::closeEvent(QCloseEvent *evento) {
    if (!m_omitirGuardado) {
        guardarPuntajePartida();
    }
    evento->accept();
}

void JuegoView::guardarPuntajePartida() {
    if (m_usuario.isEmpty() || m_progreso == nullptr) {
        return;
    }

    const int puntajeTotal = m_puntajePartida + m_progreso->puntaje();
    const int bonusTotal = m_nivelesCompletados * 25
                           + (m_nivelesCompletados >= 3 ? 100 : 0);
    const int puntosNuevos = puntajeTotal - m_puntajeRegistrado;
    const int bonusNuevo = bonusTotal - m_bonusMonedasRegistrado;
    if (puntosNuevos > 0 || bonusNuevo > 0) {
        if (GestorUsuarios::registrarPuntajePartida(m_usuario,
                                                    puntosNuevos,
                                                    bonusNuevo)) {
            m_puntajeRegistrado = puntajeTotal;
            m_bonusMonedasRegistrado = bonusTotal;
        }
    }
}

void JuegoView::crearGrid() {
    // clear() destruye los items de la escena, incluida una explosión activa.
    // Detenemos el reproductor y anulamos el puntero antes de reconstruirla
    // para que las señales y callbacks pendientes no accedan a memoria inválida.
    m_explosionPlayer->stop();
    m_explosionItem = nullptr;
    m_posicionesVisuales.clear();
    for (QVariantAnimation *animacion : std::as_const(m_animacionesMovimiento)) {
        delete animacion;
    }
    m_animacionesMovimiento.clear();
    m_escena->clear();
    m_informacion = nullptr;
    m_cuentaRegresiva = nullptr;

    for (int y = 0; y < MAX_FILAS; ++y) {
        for (int x = 0; x < MAX_COLUMNAS; ++x) {
            m_casillas[y][x] = nullptr;
            m_valoresVisuales[y][x] = -1;
            m_sprites[y][x] = nullptr;
            m_resaltos[y][x] = nullptr;
        }
    }

    const QPen borde(QColor(82, 96, 116), 1);

    if (m_fondoNivel != nullptr && !m_fondoNivel->isNull()) {
        auto *fondo = m_escena->addPixmap(*m_fondoNivel);
        fondo->setZValue(-2);
        fondo->setOpacity(0.92);
    }

    for (int y = 0; y < m_filas; ++y) {
        for (int x = 0; x < m_columnas; ++x) {
            m_casillas[y][x] = m_escena->addRect(
                m_margenColiseo + x * TAMANO_CELDA,
                m_margenColiseo + y * TAMANO_CELDA,
                TAMANO_CELDA,
                TAMANO_CELDA,
                borde,
                QBrush(QColor(21, 28, 38)));
        }
    }

    const QRectF limites(m_margenColiseo - 3, m_margenColiseo - 3,
                         m_columnas * TAMANO_CELDA + 6,
                         m_filas * TAMANO_CELDA + 6);
    auto *bordeLimite = m_escena->addRect(
        limites, QPen(QColor("#f4d06f"), 7), Qt::NoBrush);
    bordeLimite->setZValue(8);

    m_informacion = m_escena->addText("");
    m_informacion->setDefaultTextColor(QColor(235, 240, 245));
    m_informacion->setFont(QFont("Fredoka", 12));
    m_informacion->setTextWidth(PANEL_ANCHO - 30);
    m_informacion->setPos(m_fondoNivel->width() + 15, 24);

    const QString panelRuta = m_nivel == NIVEL_1 ? ":/assets/panel_arena.png"
                              : m_nivel == NIVEL_2 ? ":/assets/panel_hielo.png"
                                                   : ":/assets/panel_madera.png";
    const QPixmap panelOriginal(panelRuta);
    if (!panelOriginal.isNull()) {
        auto *panel = m_escena->addPixmap(panelOriginal.scaled(
            PANEL_ANCHO, m_fondoNivel->height(), Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation));
        panel->setPos(m_fondoNivel->width(), 0);
        panel->setZValue(-1);
        panel->setOpacity(0.78);
    }
    auto *panelSombra = m_escena->addRect(
        m_fondoNivel->width(), 0, PANEL_ANCHO, m_fondoNivel->height(),
        Qt::NoPen, QBrush(QColor(5, 8, 12, 150)));
    panelSombra->setZValue(-1);

    m_escena->setSceneRect(0, 0, m_fondoNivel->width() + PANEL_ANCHO,
                            m_fondoNivel->height());
    ajustarVistaAlMonitor();
}

void JuegoView::ajustarVistaAlMonitor() {
    const QRect areaDisponible = QGuiApplication::primaryScreen()->availableGeometry();
    const int anchoEscena = m_fondoNivel->width() + PANEL_ANCHO;
    const int altoEscena = m_fondoNivel->height();
    const int anchoMaximo = qMax(320, areaDisponible.width() - 24);
    const int altoMaximo = qMax(320, areaDisponible.height() - 64);
    const bool necesitaEscala = anchoEscena + 4 > anchoMaximo
                             || altoEscena + 4 > altoMaximo;

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    resetTransform();
    if (necesitaEscala) {
        setFixedSize(qMin(anchoEscena + 4, anchoMaximo),
                     qMin(altoEscena + 4, altoMaximo));
        // El viewport todavía no tiene su tamaño final durante el constructor
        // o al cambiar de nivel. Recalcular después del resize evita que el
        // Nivel 3 quede renderizado diminuto dentro de una ventana grande.
        QTimer::singleShot(0, this, [this]() {
            if (m_escena != nullptr) {
                fitInView(m_escena->sceneRect(), Qt::KeepAspectRatio);
            }
        });
    } else {
        setFixedSize(anchoEscena + 4, altoEscena + 4);
    }

    move(areaDisponible.center() - QPoint(width() / 2, height() / 2));
}

void JuegoView::cargarSprites() {
    m_margenColiseo = m_nivel == NIVEL_1 ? 50 : m_nivel == NIVEL_2 ? 60 : 80;
    const QString skin = GestorUsuarios::obtenerSkinEquipada(m_usuario);
    QColor colorPrincipal("#5cbe2f");
    QColor colorManchas("#2e7d26");
    if (skin == "gato") {
        colorPrincipal = QColor("#e58a32");
        colorManchas = QColor("#71351e");
    } else if (skin == "dragon") {
        colorPrincipal = QColor("#d83232");
        colorManchas = QColor("#641b24");
    } else if (skin == "burro") {
        colorPrincipal = QColor("#858585");
        colorManchas = QColor("#3f4248");
    } else if (skin == "spiderman") {
        colorPrincipal = QColor("#e52d35");
        colorManchas = QColor("#172e70");
    } else if (skin == "miles") {
        colorPrincipal = QColor("#282832");
        colorManchas = QColor("#d42d55");
    } else if (skin == "personaje") {
        colorPrincipal = QColor("#d18b42");
        colorManchas = QColor("#5b2020");
    } else if (skin == "veneno") {
        colorPrincipal = QColor("#9ccc65");
        colorManchas = QColor("#4a7c2b");
    } else if (skin == "hielo") {
        colorPrincipal = QColor("#65d7e8");
        colorManchas = QColor("#227caa");
    } else if (skin == "fuego") {
        colorPrincipal = QColor("#f06b38");
        colorManchas = QColor("#a62d20");
    } else if (skin == "cosmica") {
        colorPrincipal = QColor("#a875ff");
        colorManchas = QColor("#4d2b96");
    } else if (skin == "thanos") {
        colorPrincipal = QColor("#7048b8");
        colorManchas = QColor("#33205d");
    }
    const QPixmap cabezaCanva = Skins::cabeza(skin, 44);
    if (!cabezaCanva.isNull()) {
        *m_spriteCabeza = cabezaCanva;
    }

    QImage hoja(":/assets/snake_sheet.png");
    if (!hoja.isNull()) {
        for (int y = 0; y < hoja.height(); ++y) {
            for (int x = 0; x < hoja.width(); ++x) {
                const QColor color = hoja.pixelColor(x, y);
                if (color.red() > 245 && color.green() > 245 && color.blue() > 245) {
                    hoja.setPixelColor(x, y, QColor(255, 255, 255, 0));
                }
            }
        }

        // La página 13 del diseño contiene la cabeza y el cuerpo separados.
        // La cabeza proporcionada por el usuario es la fuente principal. El
        // recorte de la hoja queda únicamente como respaldo si falta el asset.
        if (m_spriteCabeza->isNull()) {
            const QImage cabeza = hoja.copy(25, 20, 190, 190);
            *m_spriteCabeza = QPixmap::fromImage(cabeza.scaled(36, 36, Qt::KeepAspectRatio,
                                                               Qt::SmoothTransformation));
        }
    }

    m_spritesGemas.clear();
    if (skin == "thanos") {
        const QColor coloresGemas[] = {QColor("#9b59ff"), QColor("#ffd740"),
                                       QColor("#29b6f6"), QColor("#ef5350"),
                                       QColor("#66bb6a"), QColor("#ff8a65")};
        for (const QColor &color : coloresGemas) {
            QImage gema(36, 36, QImage::Format_ARGB32_Premultiplied);
            gema.fill(Qt::transparent);
            QPainter pintor(&gema);
            pintor.setRenderHint(QPainter::Antialiasing);
            pintor.setPen(QPen(QColor("#f8edb6"), 1.2));
            pintor.setBrush(color);
            pintor.drawEllipse(2, 2, 32, 32);
            pintor.setPen(QPen(color.lighter(155), 1.2));
            pintor.drawArc(6, 5, 22, 22, 35 * 16, 105 * 16);
            pintor.setPen(QPen(color.darker(145), 1.1));
            pintor.drawLine(QPointF(8, 25), QPointF(26, 11));
            pintor.drawLine(QPointF(13, 31), QPointF(29, 18));
            pintor.setPen(Qt::NoPen);
            pintor.setBrush(QColor(255, 255, 255, 175));
            pintor.drawEllipse(QPointF(12, 10), 2.2, 3.2);
            pintor.end();
            m_spritesGemas.append(QPixmap::fromImage(gema));
        }
    }

    // El cuerpo normal se representa con piezas circulares para evitar
    // estirar la ilustración vertical de Canva dentro de cada celda.
    QImage cuerpo(36, 36, QImage::Format_ARGB32_Premultiplied);
    cuerpo.fill(Qt::transparent);
    QPainter pintorCuerpo(&cuerpo);
    pintorCuerpo.setRenderHint(QPainter::Antialiasing);
    pintorCuerpo.setPen(Qt::NoPen);
    pintorCuerpo.setBrush(colorPrincipal);
    pintorCuerpo.drawEllipse(2, 2, 32, 32);
    pintorCuerpo.setBrush(colorManchas);
    pintorCuerpo.drawEllipse(11, 8, 6, 6);
    pintorCuerpo.drawEllipse(21, 19, 6, 6);
    pintorCuerpo.end();
    *m_spriteCuerpo = QPixmap::fromImage(cuerpo);

    QImage cola(36, 36, QImage::Format_ARGB32_Premultiplied);
    cola.fill(Qt::transparent);
    QPainter pintorCola(&cola);
    pintorCola.setRenderHint(QPainter::Antialiasing);
    pintorCola.setPen(Qt::NoPen);
    pintorCola.setBrush(colorPrincipal.darker(110));
    pintorCola.drawEllipse(5, 5, 26, 26);
    pintorCola.setBrush(colorManchas);
    pintorCola.drawEllipse(13, 10, 5, 5);
    pintorCola.drawEllipse(21, 20, 5, 5);
    pintorCola.end();
    *m_spriteCola = QPixmap::fromImage(cola);

    auto cargarObjeto = [](QPixmap *destino, const QString &ruta) {
        const QPixmap original(ruta);
        if (!original.isNull()) {
            *destino = original.scaled(36, 36, Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation);
        }
    };
    cargarObjeto(m_spriteFrutaNormal, ":/assets/fruta_roja.png");
    cargarObjeto(m_spriteFrutaDorada, ":/assets/fruta_dorada.png");
    cargarObjeto(m_spriteFrutaGrande, ":/assets/fruta_grande.png");
    cargarObjeto(m_spriteFrutaEnergetica, ":/assets/fruta_energetica.png");
    cargarObjeto(m_spriteCaja, ":/assets/caja_misteriosa.png");

    const QString fondo = m_nivel == NIVEL_1 ? ":/assets/tablero_arena.png"
                         : m_nivel == NIVEL_2 ? ":/assets/tablero_hielo.png"
                                              : ":/assets/tablero_madera.png";
    const QPixmap originalFondo(fondo);
    if (!originalFondo.isNull()) {
        *m_fondoNivel = originalFondo.scaled(m_columnas * TAMANO_CELDA + 2 * m_margenColiseo,
                                             m_filas * TAMANO_CELDA + 2 * m_margenColiseo,
                                             Qt::IgnoreAspectRatio,
                                             Qt::SmoothTransformation);
    }
}

void JuegoView::configurarNivel(int nivel) {
    m_nivel = nivel == NIVEL_2 ? NIVEL_2 : nivel == NIVEL_3 ? NIVEL_3 : NIVEL_1;
    m_columnas = m_nivel == NIVEL_1 ? 10 : m_nivel == NIVEL_2 ? 15 : 20;
    m_filas = m_nivel == NIVEL_1 ? 10 : m_nivel == NIVEL_2 ? 15 : 20;

    if (m_nivel == NIVEL_1) {
        m_metaFrutas = META_FRUTAS_NIVEL_1;
        m_metaPuntos = META_PUNTOS_NIVEL_1;
        m_metaLongitud = META_LONGITUD_NIVEL_1;
    } else if (m_nivel == NIVEL_2) {
        m_metaFrutas = META_FRUTAS_NIVEL_2;
        m_metaPuntos = META_PUNTOS_NIVEL_2;
        m_metaLongitud = META_LONGITUD_NIVEL_2;
    } else {
        m_metaFrutas = META_FRUTAS_NIVEL_3;
        m_metaPuntos = META_PUNTOS_NIVEL_3;
        m_metaLongitud = META_LONGITUD_NIVEL_3;
    }

    m_tablero->redimensionar(m_columnas, m_filas);
    m_progreso = std::make_unique<ProgresoNivel>(m_metaFrutas,
                                                  m_metaPuntos,
                                                  m_metaLongitud);
    m_turnosObstaculosCongelados = 0;
    m_turnosHielo = 0;
    m_turnosEnergia = 0;
    m_tiempoExtraSegundos = 0;
    m_tiempoRestanteSegundos = TIEMPO_INICIAL_SEGUNDOS;
    m_milisegundosTiempo = 0;
    m_turnosTrampa = 0;
    m_trampaX = -1;
    m_trampaY = -1;
    m_turnosDesdeCaja = 0;
    m_frutasDesdeCaja = 0;
    m_ultimoEfecto.clear();
    m_obstaculosAleatorios.clear();
    inicializarObstaculosMoviles();
    detenerAnimacionEnHilo();
    setWindowTitle(QString("Snake - Nivel %1").arg(m_nivel));
}

void JuegoView::inicializarObstaculosMoviles() {
    m_turnoObstaculos = 0;
    // Cuatro obstáculos móviles: dos horizontales y dos verticales.
    // Los últimos dos forman una diagonal fija y no reciben movimiento.
    const int posicionesIniciales[MAX_OBSTACULOS_MOVILES][4] = {
        {3, 4, 1, 0},  {m_columnas - 4, m_filas - 4, -1, 0},
        {m_columnas / 2, 2, 0, 1},  {m_columnas / 2, m_filas - 3, 0, -1},
        {m_columnas / 3, m_filas / 2, 0, 0},
        {m_columnas - 4, m_filas / 2, 0, 0}
    };

    for (int i = 0; i < MAX_OBSTACULOS_MOVILES; ++i) {
        m_obstaculos[i].configurar(posicionesIniciales[i][0], posicionesIniciales[i][1],
                                   posicionesIniciales[i][2], posicionesIniciales[i][3], i < 4);
    }
}

void JuegoView::construirObstaculos() {
    if (!m_configuracion.obstaculos) {
        return;
    }

    if (m_configuracion.esAleatorio) {
        const int obstaculosObjetivo = m_nivel == NIVEL_1 ? 4
                                     : m_nivel == NIVEL_2 ? 8 : 13;
        const int inicioX = 1;
        const int inicioY = m_filas / 2;
        int colocados = m_obstaculosAleatorios.size();
        for (int intento = 0; intento < m_columnas * m_filas * 3
             && colocados < obstaculosObjetivo; ++intento) {
            const int x = QRandomGenerator::global()->bounded(m_columnas);
            const int y = QRandomGenerator::global()->bounded(m_filas);
            const bool zonaInicial = qAbs(x - inicioX) <= 3 && qAbs(y - inicioY) <= 2;
            if (zonaInicial || m_tablero->valor(x, y) != VACIO
                || !posicionObstaculoDisponible(x, y, -1)) continue;
            m_obstaculosAleatorios.append(QPoint(x, y));
            ++colocados;
        }
        for (const QPoint &posicion : std::as_const(m_obstaculosAleatorios)) {
            m_tablero->poner(posicion.x(), posicion.y(), OBSTACULO);
        }
        for (int i = 0; i < MAX_OBSTACULOS_MOVILES; ++i) {
            const int limite = m_nivel == NIVEL_1 ? 1 : m_nivel == NIVEL_2 ? 2 : 3;
            if (m_configuracion.obstaculosMoviles && i < limite) {
                m_tablero->poner(m_obstaculos[i].x(), m_obstaculos[i].y(), OBSTACULO);
            }
        }
        return;
    }

    if (m_nivel == NIVEL_1) {
        if (!m_configuracion.esAleatorio) {
            return;
        }
        const int centroY = m_filas / 2;
        for (int x = 3; x <= 6; ++x) {
            m_tablero->poner(x, centroY - 2, OBSTACULO);
            m_tablero->poner(x, centroY + 2, OBSTACULO);
        }
        return;
    }

    if (m_nivel == NIVEL_3 && m_configuracion.obstaculosMoviles) {
        for (int i = 0; i < 4; ++i) {
            if (m_tablero->valor(m_obstaculos[i].x(), m_obstaculos[i].y()) == VACIO) {
                m_tablero->poner(m_obstaculos[i].x(), m_obstaculos[i].y(), OBSTACULO);
            }
        }
    }

    static const int obstaculosNivel2[][2] = {
        {2, 4}, {3, 4}, {4, 4}, {10, 4}, {11, 4}, {12, 4},
        {6, 6}, {7, 6}, {8, 6},
        {6, 8}, {7, 8}, {8, 8},
        {2, 10}, {3, 10}, {4, 10}, {10, 10}, {11, 10}, {12, 10}
    };
    static const int obstaculosNivel3[][2] = {
        {4, 4}, {5, 4}, {6, 4}, {13, 4}, {14, 4}, {15, 4},
        {4, 7}, {4, 8}, {15, 7}, {15, 8},
        {8, 6}, {9, 6}, {10, 6}, {11, 6},
        {8, 13}, {9, 13}, {10, 13}, {11, 13},
        {4, 15}, {5, 15}, {6, 15}, {13, 15}, {14, 15}, {15, 15}
    };

    const auto &obstaculos = m_nivel == NIVEL_3
        ? obstaculosNivel3 : obstaculosNivel2;
    const int cantidad = m_nivel == NIVEL_3
        ? static_cast<int>(sizeof(obstaculosNivel3) / sizeof(obstaculosNivel3[0]))
        : static_cast<int>(sizeof(obstaculosNivel2) / sizeof(obstaculosNivel2[0]));
    for (int i = 0; i < cantidad; ++i) {
        const auto &obstaculo = obstaculos[i];
        if (m_tablero->valor(obstaculo[0], obstaculo[1]) == VACIO) {
            m_tablero->poner(obstaculo[0], obstaculo[1], OBSTACULO);
        }
    }
}

void JuegoView::moverObstaculos() {
    const bool usaMoviles = m_configuracion.obstaculosMoviles
                         && (m_nivel == NIVEL_3 || m_configuracion.esAleatorio);
    if (!usaMoviles
        || (++m_turnoObstaculos % (m_configuracion.esAleatorio ? 7 : 4)) != 0) {
        return;
    }

    if (m_turnosObstaculosCongelados > 0) {
        --m_turnosObstaculosCongelados;
        return;
    }

    const int cantidadMoviles = m_configuracion.esAleatorio
        ? (m_nivel == NIVEL_1 ? 1 : m_nivel == NIVEL_2 ? 2 : 3) : 4;
    for (int i = 0; i < cantidadMoviles; ++i) {
        int nuevaX = m_obstaculos[i].x() + m_obstaculos[i].direccionX();
        int nuevaY = m_obstaculos[i].y() + m_obstaculos[i].direccionY();

        const bool fueraDelRecorrido = nuevaX < 2 || nuevaX > m_columnas - 3
                                     || nuevaY < 2 || nuevaY > m_filas - 3;
        if (fueraDelRecorrido) {
            m_obstaculos[i].invertirDireccion();
            nuevaX = m_obstaculos[i].x() + m_obstaculos[i].direccionX();
            nuevaY = m_obstaculos[i].y() + m_obstaculos[i].direccionY();
        }

        if (posicionObstaculoDisponible(nuevaX, nuevaY, i)) {
            m_obstaculos[i].moverA(nuevaX, nuevaY);
        } else {
            // Si la serpiente u otro obstáculo ocupa el destino, espera
            // un turno y cambia el sentido para evitar apariciones injustas.
            m_obstaculos[i].invertirDireccion();
        }
    }
}

bool JuegoView::posicionObstaculoDisponible(int x, int y, int ignorar) const {
    if (x < 0 || x >= m_columnas || y < 0 || y >= m_filas
        || m_serpiente->ocupa(x, y)
        || m_tablero->valor(x, y) == OBSTACULO) {
        return false;
    }

    // Los obstáculos móviles no deben desplazar ni sobrescribir frutas o cajas.
    for (const ObjetoActivo &objeto : m_objetos) {
        if (objeto.activo && objeto.x == x && objeto.y == y) {
            return false;
        }
    }

    for (int i = 0; i < MAX_OBSTACULOS_MOVILES; ++i) {
        if (i != ignorar && m_obstaculos[i].x() == x && m_obstaculos[i].y() == y) {
            return false;
        }
    }

    return true;
}

void JuegoView::reiniciar() {
    m_animadorPuntaje->stop();
    m_puntajeVisual = 0;
    m_posicionesVisuales.clear();
    m_serpiente->limpiar();
    const int inicioY = m_filas / 2;
    m_serpiente->insertarCabeza(1, inicioY);
    m_serpiente->insertarCabeza(2, inicioY);
    m_serpiente->insertarCabeza(3, inicioY);

    m_direccionX = 1;
    m_direccionY = 0;
    m_cambioDireccionPendiente = false;
    m_terminado = false;
    m_progreso->reiniciar();
    m_turnosObstaculosCongelados = 0;
    m_turnosHielo = 0;
    m_turnosEnergia = 0;
    m_tiempoExtraSegundos = 0;
    m_tiempoRestanteSegundos = TIEMPO_INICIAL_SEGUNDOS;
    m_milisegundosTiempo = 0;
    m_turnosTrampa = 0;
    m_trampaX = -1;
    m_trampaY = -1;
    m_turnosDesdeCaja = 0;
    m_frutasDesdeCaja = 0;
    m_ultimoEfecto.clear();
    for (ObjetoActivo &objeto : m_objetos) {
        objeto.activo = false;
        objeto.x = -1;
        objeto.y = -1;
        objeto.tipo = VACIO;
        objeto.creadaEnMs = 0;
    }
    m_obstaculosAleatorios.clear();
    inicializarObstaculosMoviles();

    actualizarMapa();
    generarManzana();
    redibujar();
    actualizarInformacion();
}

void JuegoView::actualizarMapa() {
    m_tablero->limpiar();

    const Nodo *actual = m_serpiente->cabeza();
    while (actual != nullptr) {
        m_tablero->poner(actual->x, actual->y, SERPIENTE);
        actual = actual->siguiente;
    }
    construirObstaculos();
    if (m_turnosTrampa > 0 && m_trampaX >= 0) {
        m_tablero->poner(m_trampaX, m_trampaY, OBSTACULO);
    }
    for (const ObjetoActivo &objeto : m_objetos) {
        if (objeto.activo && m_tablero->valor(objeto.x, objeto.y) == VACIO) {
            m_tablero->poner(objeto.x, objeto.y, objeto.tipo);
        }
    }
}

void JuegoView::redibujar() {
    for (QVariantAnimation *animacion : std::as_const(m_animacionesMovimiento)) {
        animacion->stop();
    }

    const QHash<int, QPointF> posicionesAnteriores = m_posicionesVisuales;
    m_posicionesVisuales.clear();

    // Resolver la posición de cada segmento una sola vez evita recorrer la
    // lista enlazada completa por cada casilla ocupada en cada tick.
    int indicesSegmentos[MAX_FILAS][MAX_COLUMNAS];
    const bool usaGemasThanos = GestorUsuarios::obtenerSkinEquipada(m_usuario)
                              == "thanos" && !m_spritesGemas.isEmpty();
    for (int y = 0; y < m_filas; ++y) {
        for (int x = 0; x < m_columnas; ++x) {
            indicesSegmentos[y][x] = -1;
        }
    }
    int indiceSegmento = 0;
    for (const Nodo *segmento = m_serpiente->cabeza();
         segmento != nullptr;
         segmento = segmento->siguiente) {
        if (segmento->x >= 0 && segmento->x < m_columnas
            && segmento->y >= 0 && segmento->y < m_filas) {
            indicesSegmentos[segmento->y][segmento->x] = indiceSegmento;
        }
        ++indiceSegmento;
    }
    const int longitudActual = m_serpiente->longitud();
    const int cabezaX = m_serpiente->cabezaX();
    const int cabezaY = m_serpiente->cabezaY();

    for (int y = 0; y < m_filas; ++y) {
        for (int x = 0; x < m_columnas; ++x) {
            if (m_sprites[y][x] != nullptr) {
                m_sprites[y][x]->setVisible(false);
            }
            if (m_resaltos[y][x] != nullptr) {
                m_resaltos[y][x]->setVisible(false);
            }

            const int casilla = m_tablero->valor(x, y);
            if (m_valoresVisuales[y][x] != casilla) {
                QColor color(Qt::transparent);
                if (casilla == OBSTACULO) {
                    color = QColor(116, 82, 58);
                }
                m_casillas[y][x]->setBrush(QBrush(color));
                m_valoresVisuales[y][x] = casilla;
            }

            if (casilla == SERPIENTE) {
                const bool esCabeza = x == cabezaX && y == cabezaY;
                const int indice = indicesSegmentos[y][x];
                const bool esCola = !esCabeza && indice == longitudActual - 1;
                const QPixmap *spriteSeleccionado = m_spriteCuerpo;
                if (esCabeza) {
                    spriteSeleccionado = m_spriteCabeza;
                } else if (usaGemasThanos) {
                    const int indiceGema = qMax(0, indice - 1)
                                         % m_spritesGemas.size();
                    spriteSeleccionado = &m_spritesGemas.at(indiceGema);
                } else if (esCola) {
                    spriteSeleccionado = m_spriteCola;
                }
                const QPixmap &sprite = *spriteSeleccionado;
                if (!sprite.isNull()) {
                    QGraphicsPixmapItem *item = m_sprites[y][x];
                    if (item == nullptr) {
                        item = m_escena->addPixmap(sprite);
                        m_sprites[y][x] = item;
                    }
                    item->setPixmap(sprite);
                    item->setVisible(true);
                    item->setOpacity(1.0);

                    const int margen = esCabeza ? -2 : 2;
                    const QPointF destino(m_margenColiseo + x * TAMANO_CELDA + margen,
                                          m_margenColiseo + y * TAMANO_CELDA + margen);
                    const QPointF inicio = posicionesAnteriores.contains(indice)
                        ? (indice == 0 ? posicionesAnteriores.value(0)
                                       : posicionesAnteriores.value(indice - 1))
                        : destino;
                    item->setPos(inicio);
                    if (inicio != destino) {
                        auto animacion = m_animacionesMovimiento.value(item, nullptr);
                        if (animacion == nullptr) {
                            animacion = new QVariantAnimation(this);
                            connect(animacion, &QVariantAnimation::valueChanged,
                                    this, [item](const QVariant &valor) {
                                        item->setPos(valor.toPointF());
                                    });
                            m_animacionesMovimiento.insert(item, animacion);
                        }
                        animacion->stop();
                        animacion->setStartValue(inicio);
                        animacion->setEndValue(destino);
                        animacion->setDuration(qBound(40, intervaloActual() * 8 / 10, 120));
                        animacion->start();
                    }
                    m_posicionesVisuales[indice] = destino;

                    item->setRotation(0);
                    if (esCabeza) {
                        item->setTransformOriginPoint(item->boundingRect().center());
                        if (m_direccionX < 0) {
                            item->setRotation(270);
                        } else if (m_direccionX > 0) {
                            item->setRotation(90);
                        } else if (m_direccionY > 0) {
                            item->setRotation(180);
                        }
                    }

                }
            } else if (casilla == MANZANA || casilla == MANZANA_DORADA
                       || casilla == FRUTA_GRANDE || casilla == FRUTA_ENERGETICA
                       || casilla == CAJA_MISTERIOSA) {
                const QPixmap *sprite = casilla == MANZANA ? m_spriteFrutaNormal
                                      : casilla == MANZANA_DORADA ? m_spriteFrutaDorada
                                      : casilla == FRUTA_GRANDE ? m_spriteFrutaGrande
                                      : casilla == FRUTA_ENERGETICA ? m_spriteFrutaEnergetica
                                                              : m_spriteCaja;
                if (sprite != nullptr && !sprite->isNull()) {
                    qreal opacidad = 1.0;
                    for (const ObjetoActivo &objeto : m_objetos) {
                        if (objeto.activo && objeto.x == x && objeto.y == y
                            && objeto.tipo != CAJA_MISTERIOSA
                            && objeto.creadaEnMs > 0) {
                            const qint64 edad = QDateTime::currentMSecsSinceEpoch()
                                               - objeto.creadaEnMs;
                            if (edad > DURACION_FRUTA_MS - DURACION_DESVANECIMIENTO_MS) {
                                opacidad = qBound(0.0,
                                    static_cast<qreal>(DURACION_FRUTA_MS - edad)
                                    / DURACION_DESVANECIMIENTO_MS, 1.0);
                            }
                            break;
                        }
                    }
                    if (casilla != CAJA_MISTERIOSA) {
                        auto *resalto = m_resaltos[y][x];
                        if (resalto == nullptr) {
                            resalto = m_escena->addEllipse(
                                0, 0, TAMANO_CELDA - 6, TAMANO_CELDA - 6,
                                QPen(QColor("#ffe58a"), 2),
                                QBrush(QColor(255, 224, 104, 55)));
                            resalto->setZValue(3);
                            m_resaltos[y][x] = resalto;
                        }
                        resalto->setPos(m_margenColiseo + x * TAMANO_CELDA + 3,
                                       m_margenColiseo + y * TAMANO_CELDA + 3);
                        resalto->setOpacity(opacidad);
                        resalto->setVisible(true);
                    }
                    auto *item = m_sprites[y][x];
                    if (item == nullptr) {
                        item = m_escena->addPixmap(*sprite);
                        m_sprites[y][x] = item;
                    }
                    item->setPixmap(*sprite);
                    item->setPos(m_margenColiseo + x * TAMANO_CELDA + 2,
                                 m_margenColiseo + y * TAMANO_CELDA + 2);
                    item->setZValue(4);
                    item->setRotation(0);
                    item->setOpacity(opacidad);
                    item->setVisible(true);
                }
            }
        }
    }
}

void JuegoView::generarManzana() {
    const int frutasObjetivo = (m_nivel == NIVEL_3 || m_configuracion.esAleatorio) ? 2 : 1;
    int frutasActivas = 0;
    for (const ObjetoActivo &objeto : m_objetos) {
        if (objeto.activo && objeto.tipo != CAJA_MISTERIOSA) {
            ++frutasActivas;
        }
    }

    while (frutasActivas < frutasObjetivo) {
        int indice = -1;
        for (int i = 0; i < MAX_OBJETOS_ACTIVOS; ++i) {
            if (!m_objetos[i].activo) {
                indice = i;
                break;
            }
        }
        if (indice < 0) break;

        bool generada = false;
        for (int intento = 0; intento < m_columnas * m_filas; ++intento) {
            const int x = QRandomGenerator::global()->bounded(m_columnas);
            const int y = QRandomGenerator::global()->bounded(m_filas);

            if (m_tablero->valor(x, y) == VACIO) {
                const TipoFruta fruta = frutaAleatoriaParaNivel();
                m_objetos[indice].activo = true;
                m_objetos[indice].x = x;
                m_objetos[indice].y = y;
                m_objetos[indice].fruta.configurar(fruta);
                m_objetos[indice].tipo = fruta == TipoFruta::Normal ? MANZANA
                                      : fruta == TipoFruta::Dorada ? MANZANA_DORADA
                                      : fruta == TipoFruta::Grande ? FRUTA_GRANDE
                                                                   : FRUTA_ENERGETICA;
                m_objetos[indice].creadaEnMs = QDateTime::currentMSecsSinceEpoch();
                m_tablero->poner(x, y, m_objetos[indice].tipo);
                ++frutasActivas;
                generada = true;
                break;
            }
        }

        // No hay espacio libre suficiente para completar el objetivo.
        if (!generada) break;
    }

    // La caja se sortea por separado y puede coexistir con las frutas.
    if ((m_nivel == NIVEL_3 || m_configuracion.esAleatorio)
        && m_configuracion.items && m_configuracion.aparicionAleatoria) {
        bool hayCaja = false;
        for (const ObjetoActivo &objeto : m_objetos) {
            hayCaja = hayCaja || (objeto.activo && objeto.tipo == CAJA_MISTERIOSA);
        }
        const bool garantizada = m_turnosDesdeCaja >= GARANTIA_CAJA_TURNOS
                              || m_frutasDesdeCaja >= GARANTIA_CAJA_FRUTAS;
        const bool cooldownCumplido = m_turnosDesdeCaja >= COOLDOWN_CAJA_TURNOS;
        if (!hayCaja && (garantizada || (cooldownCumplido
                                         && QRandomGenerator::global()->bounded(100) < 20))) {
            for (int i = 0; i < MAX_OBJETOS_ACTIVOS; ++i) {
                if (m_objetos[i].activo) continue;
                for (int intento = 0; intento < m_columnas * m_filas; ++intento) {
                    const int x = QRandomGenerator::global()->bounded(m_columnas);
                    const int y = QRandomGenerator::global()->bounded(m_filas);
                    if (m_tablero->valor(x, y) == VACIO) {
                        m_objetos[i].activo = true;
                        m_objetos[i].x = x;
                        m_objetos[i].y = y;
                        m_objetos[i].tipo = CAJA_MISTERIOSA;
                        m_turnosDesdeCaja = 0;
                        m_frutasDesdeCaja = 0;
                        m_tablero->poner(x, y, CAJA_MISTERIOSA);
                        return;
                    }
                }
            }
        }
    }
}

void JuegoView::alternarPausa() {
    if (m_terminado) return;
    m_pausado = !m_pausado;
    if (m_pausado) {
        m_temporizadorCuentaRegresiva->stop();
        m_temporizadorFrutas->stop();
        m_animadorPuntaje->stop();
        detenerAnimacionEnHilo();
        mostrarOverlayPausa();
    } else {
        quitarOverlayPausa();
        if (m_cuentaRegresivaValor > 0) {
            m_temporizadorCuentaRegresiva->start(1000);
        } else {
            if (m_cuentaRegresiva != nullptr) m_cuentaRegresiva->setVisible(false);
            m_temporizadorFrutas->start();
            iniciarAnimacionEnHilo();
        }
    }
}

void JuegoView::mostrarOverlayPausa() {
    const QRectF tablero(m_margenColiseo, m_margenColiseo,
                         m_columnas * TAMANO_CELDA, m_filas * TAMANO_CELDA);
    m_overlayPausa = m_escena->addRect(tablero, QPen(Qt::NoPen),
                                       QBrush(QColor(5, 8, 12, 185)));
    m_overlayPausa->setZValue(50);
    m_textoPausa = m_escena->addText("PAUSA");
    m_textoPausa->setDefaultTextColor(Qt::white);
    m_textoPausa->setFont(QFont("Fredoka", 25, QFont::Bold));
    m_textoPausa->setZValue(52);
    m_textoPausa->setPos(tablero.left() + 42, tablero.top() + 42);

    m_botonReanudar = new QPushButton("CONTINUAR");
    m_botonReanudar->setFixedSize(190, 56);
    m_botonReanudar->setCursor(Qt::PointingHandCursor);
    m_botonReanudar->setStyleSheet(
        "QPushButton { background: #2f8618; color: white; border: 3px solid #68dd3e;"
        " border-radius: 10px; font: bold 18px 'Fredoka'; }"
        "QPushButton:hover { background: #4cae22; border-color: #f4d06f; }");
    m_proxyReanudar = m_escena->addWidget(m_botonReanudar);
    m_proxyReanudar->setZValue(53);
    m_proxyReanudar->setPos(tablero.center().x() - 95, tablero.bottom() - 150);
    connect(m_botonReanudar, &QPushButton::clicked, this, &JuegoView::alternarPausa);

    m_botonMenuPrincipal = new QPushButton("VOLVER");
    m_botonMenuPrincipal->setFixedSize(190, 56);
    m_botonMenuPrincipal->setCursor(Qt::PointingHandCursor);
    m_botonMenuPrincipal->setStyleSheet(
        "QPushButton { background: #2f8618; color: white; border: 3px solid #68dd3e;"
        " border-radius: 10px; font: bold 16px 'Fredoka'; }"
        "QPushButton:hover { background: #4cae22; border-color: #f4d06f; }");
    m_proxyMenuPrincipal = m_escena->addWidget(m_botonMenuPrincipal);
    m_proxyMenuPrincipal->setZValue(53);
    m_proxyMenuPrincipal->setPos(tablero.center().x() - 95, tablero.bottom() - 82);
    connect(m_botonMenuPrincipal, &QPushButton::clicked,
            this, &JuegoView::salirAlMenuPrincipal);

    QPixmap shrek(":/assets/manual/shrek_pausa.png");
    if (!shrek.isNull()) {
        m_shrekPausa = m_escena->addPixmap(shrek.scaled(
            210, 260, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_shrekPausa->setZValue(51);
        m_shrekPausa->setPos(tablero.right() - 225, tablero.top() + 35);
    }
}

bool JuegoView::verificarVictoria() {
    if (m_terminado || m_progreso == nullptr || !m_progreso->gano()) {
        return false;
    }

    redibujar();
    actualizarInformacion();
    ganarNivel();
    return true;
}

void JuegoView::salirAlMenuPrincipal() {
    QMessageBox confirmacion(this);
    confirmacion.setWindowTitle("Salir de la partida");
    confirmacion.setIcon(QMessageBox::Warning);
    confirmacion.setText("¿Querés volver al menú principal?");
    confirmacion.setInformativeText(
        "El progreso, puntaje y monedas obtenidos en esta partida no se guardarán.");
    QAbstractButton *salir = confirmacion.addButton("Salir al menú",
                                                     QMessageBox::AcceptRole);
    confirmacion.addButton("Cancelar", QMessageBox::RejectRole);
    Dialogos::aplicarEstilo(confirmacion);
    confirmacion.exec();
    if (confirmacion.clickedButton() != salir) return;

    m_omitirGuardado = true;
    close();
}

void JuegoView::quitarOverlayPausa() {
    delete m_overlayPausa;
    delete m_shrekPausa;
    delete m_textoPausa;
    delete m_proxyReanudar;
    delete m_proxyMenuPrincipal;
    m_overlayPausa = nullptr;
    m_shrekPausa = nullptr;
    m_textoPausa = nullptr;
    m_proxyReanudar = nullptr;
    m_botonReanudar = nullptr;
    m_proxyMenuPrincipal = nullptr;
    m_botonMenuPrincipal = nullptr;
}

void JuegoView::actualizarTemporizadoresFrutas() {
    if (m_terminado) return;

    const qint64 ahora = QDateTime::currentMSecsSinceEpoch();
    bool cambio = false;
    bool expiro = false;
    for (ObjetoActivo &objeto : m_objetos) {
        if (!objeto.activo || objeto.tipo == CAJA_MISTERIOSA
            || objeto.creadaEnMs <= 0) continue;
        const qint64 edad = ahora - objeto.creadaEnMs;
        if (edad >= DURACION_FRUTA_MS) {
            objeto.activo = false;
            objeto.x = -1;
            objeto.y = -1;
            objeto.tipo = VACIO;
            objeto.creadaEnMs = 0;
            expiro = true;
            cambio = true;
        } else if (edad >= DURACION_FRUTA_MS - DURACION_DESVANECIMIENTO_MS) {
            cambio = true;
        }
    }
    if (expiro) {
        actualizarMapa();
        generarManzana();
    }
    if (cambio) redibujar();
}

int JuegoView::indiceObjetoEn(int x, int y) const {
    for (int i = 0; i < MAX_OBJETOS_ACTIVOS; ++i) {
        if (m_objetos[i].activo && m_objetos[i].x == x && m_objetos[i].y == y) return i;
    }
    return -1;
}

TipoFruta JuegoView::frutaAleatoriaParaNivel() const {
    if (!m_configuracion.frutasEspeciales) return TipoFruta::Normal;
    const int azar = QRandomGenerator::global()->bounded(100);
    if (m_nivel == NIVEL_1) return azar < 45 ? TipoFruta::Dorada : TipoFruta::Normal;
    if (m_nivel == NIVEL_2) {
        if (azar < 45) return TipoFruta::Dorada;
        if (azar < 65) return TipoFruta::Grande;
        return TipoFruta::Normal;
    }
    if (azar < 25) return TipoFruta::Dorada;
    if (azar < 45) return TipoFruta::Grande;
    if (azar < 60) return TipoFruta::Energetica;
    return TipoFruta::Normal;
}

void JuegoView::actualizarInformacion() {
    QString texto =
        QString("NIVEL %1\n\nPUNTAJE\n%2 / %3\n\nFRUTAS\n%4 / %5\n\nLONGITUD\n%6 / %7\n\nVELOCIDAD\n%8 ms\n\nMETA\nCompleta las 3 metas")
            .arg(m_nivel)
            .arg(m_puntajeVisual)
            .arg(m_metaPuntos)
            .arg(m_progreso->frutasComidas())
            .arg(m_metaFrutas)
            .arg(m_serpiente->longitud())
            .arg(m_metaLongitud)
            .arg(intervaloActual());
    if (m_tiempoExtraSegundos > 0) {
        texto += QString("\n\nTIEMPO\n%1 s (+%2 s acumulados)")
                     .arg(m_tiempoRestanteSegundos)
                     .arg(m_tiempoExtraSegundos);
    } else {
        texto += QString("\n\nTIEMPO\n%1 s").arg(m_tiempoRestanteSegundos);
    }
    if (!m_ultimoEfecto.isEmpty()) {
        texto += QString("\n\n%1").arg(m_ultimoEfecto);
    }
    m_informacion->setPlainText(texto);
}

void JuegoView::iniciarAnimacionPuntaje() {
    if (m_progreso == nullptr) return;
    const int objetivo = m_progreso->puntaje();
    if (m_puntajeVisual > objetivo) {
        m_puntajeVisual = objetivo;
    }
    if (m_puntajeVisual < objetivo) {
        m_animadorPuntaje->start(30);
    }
}

void JuegoView::aplicarItem() {
    const TipoItem tipo = static_cast<TipoItem>(
        QRandomGenerator::global()->bounded(6));
    const Item item(tipo);
    m_ultimoEfecto.clear();

    switch (tipo) {
    case TipoItem::Reloj:
        m_tiempoExtraSegundos += item.tiempoExtraSegundos();
        m_tiempoRestanteSegundos += item.tiempoExtraSegundos();
        m_ultimoEfecto = "Reloj: +10 s";
        break;
    case TipoItem::Hielo:
        m_turnosHielo = item.duracionTurnos();
        m_turnosObstaculosCongelados = item.duracionTurnos();
        m_ultimoEfecto = "Hielo: ralentiza y congela 10 turnos";
        break;
    case TipoItem::Rayo:
        m_progreso->registrarItem(item.puntos(), m_serpiente->longitud());
        iniciarAnimacionPuntaje();
        m_turnosEnergia = 100;
        m_ultimoEfecto = "Rayo: +30 puntos";
        break;
    case TipoItem::Tijeras:
        m_serpiente->reducirSegmentos(-item.segmentosDelta());
        m_progreso->registrarItem(item.puntos(), m_serpiente->longitud());
        iniciarAnimacionPuntaje();
        m_ultimoEfecto = "Tijeras: -2 segmentos";
        break;
    case TipoItem::Bomba:
        m_serpiente->reducirSegmentos(-item.segmentosDelta());
        m_progreso->registrarItem(item.puntos(), m_serpiente->longitud());
        iniciarAnimacionPuntaje();
        m_ultimoEfecto = "Bomba: -20 puntos, -1 segmento";
        break;
    case TipoItem::Trampa:
        for (int intento = 0; intento < m_columnas * m_filas; ++intento) {
            const int x = QRandomGenerator::global()->bounded(m_columnas);
            const int y = QRandomGenerator::global()->bounded(m_filas);
            if (m_tablero->valor(x, y) == VACIO) {
                m_trampaX = x;
                m_trampaY = y;
                m_turnosTrampa = item.duracionTurnos();
                m_ultimoEfecto = "Trampa: obstáculo temporal";
                break;
            }
        }
        break;
    }
}

void JuegoView::avanzarJuego() {
    if (m_terminado) {
        return;
    }

    m_milisegundosTiempo += intervaloActual();
    while (m_milisegundosTiempo >= 1000) {
        m_milisegundosTiempo -= 1000;
        --m_tiempoRestanteSegundos;
    }
    if (m_tiempoRestanteSegundos <= 0) {
        terminarJuego();
        return;
    }
    if (m_nivel == NIVEL_3 || m_configuracion.esAleatorio) {
        ++m_turnosDesdeCaja;
    }

    // Solo se permite un giro entre dos actualizaciones del QTimer.
    m_cambioDireccionPendiente = false;
    if (m_turnosHielo > 0) {
        --m_turnosHielo;
    }
    if (m_turnosEnergia > 0) {
        --m_turnosEnergia;
    }
    if (m_turnosTrampa > 0) {
        --m_turnosTrampa;
        if (m_turnosTrampa == 0) {
            m_trampaX = -1;
            m_trampaY = -1;
        }
    }
    cambiarIntervaloEnHilo(intervaloActual());
    moverObstaculos();

    // El tablero debe representar las posiciones nuevas antes de revisar
    // la siguiente casilla de la serpiente.
    actualizarMapa();

    int nuevaX = m_serpiente->cabezaX() + m_direccionX;
    int nuevaY = m_serpiente->cabezaY() + m_direccionY;

    if (esNivelConBordesMortales()
        && !ReglasMovimiento::dentroDeLimites(nuevaX, nuevaY, m_columnas, m_filas)) {
        terminarJuego();
        return;
    }

    if (!esNivelConBordesMortales()) {
        nuevaX = ReglasMovimiento::envolverCoordenada(nuevaX, m_columnas);
        nuevaY = ReglasMovimiento::envolverCoordenada(nuevaY, m_filas);
    }

    const int destino = m_tablero->valor(nuevaX, nuevaY);

    if (destino == OBSTACULO) {
        terminarJuego();
        return;
    }

    if (ReglasMovimiento::colisionaConSerpiente(
            destino == SERPIENTE,
            m_serpiente->ocupaCola(nuevaX, nuevaY),
            m_serpiente->tieneCrecimientoPendiente())) {
        terminarJuego();
        return;
    }

    const int indiceObjeto = indiceObjetoEn(nuevaX, nuevaY);
    const bool comioFruta = destino == MANZANA || destino == MANZANA_DORADA
                          || destino == FRUTA_GRANDE || destino == FRUTA_ENERGETICA;
    const bool tomoCaja = destino == CAJA_MISTERIOSA;
    Fruta frutaComida;
    if (indiceObjeto >= 0) {
        frutaComida = m_objetos[indiceObjeto].fruta;
        if (comioFruta || tomoCaja) {
            m_objetos[indiceObjeto].activo = false;
            m_objetos[indiceObjeto].creadaEnMs = 0;
        }
    }
    const int crecimiento = comioFruta
                                ? frutaComida.crecimiento(m_nivel == NIVEL_3)
                                : 0;
    m_serpiente->avanzar(nuevaX, nuevaY, crecimiento);
    actualizarMapa();
    m_progreso->actualizarLongitud(m_serpiente->longitud());

    if (comioFruta) {
        AudioManager::instancia().reproducirEfecto(AudioManager::Efecto::Comer);
        m_frutaActual = frutaComida;
        m_progreso->registrarFruta(frutaComida.puntos(),
                                   m_serpiente->longitud(),
                                   frutaComida.frutasContadas());
        iniciarAnimacionPuntaje();
        if (m_nivel == NIVEL_3 || m_configuracion.esAleatorio) {
            m_frutasDesdeCaja += frutaComida.frutasContadas();
        }
        if (frutaComida.tipo() == TipoFruta::Energetica) {
            m_turnosEnergia = 100;
            m_ultimoEfecto = "Fruta energética: acelera 5 segundos";
        }
        cambiarIntervaloEnHilo(intervaloActual());
        if (verificarVictoria()) return;

        generarManzana();
    } else if (tomoCaja) {
        AudioManager::instancia().reproducirEfecto(AudioManager::Efecto::Moneda);
        aplicarItem();
        if (verificarVictoria()) return;
        generarManzana();
    }

    if (verificarVictoria()) return;
    redibujar();
    actualizarInformacion();

}

void JuegoView::terminarJuego() {
    m_terminado = true;
    m_temporizadorFrutas->stop();
    detenerAnimacionEnHilo();
    AudioManager::instancia().detenerMusica();
    AudioManager::instancia().reproducirEfecto(AudioManager::Efecto::Perder);
    m_ultimoEfecto = m_tiempoRestanteSegundos <= 0
        ? "Se agotó el tiempo de la partida."
        : "La serpiente chocó con un borde, obstáculo o su propio cuerpo.";
    reproducirExplosion();
}

void JuegoView::reproducirExplosion() {
    m_tarjetaDerrotaMostrada = false;
    if (m_explosionItem != nullptr) {
        delete m_explosionItem;
    }
    m_explosionItem = m_escena->addPixmap(QPixmap());
    m_explosionItem->setZValue(30);
    m_explosionItem->setPos(m_margenColiseo + m_serpiente->cabezaX() * TAMANO_CELDA - 65,
                            m_margenColiseo + m_serpiente->cabezaY() * TAMANO_CELDA - 65);
    m_explosionPlayer->stop();
    m_explosionPlayer->setPosition(0);
    QTimer::singleShot(220, this, [this]() {
        if (m_terminado) m_explosionPlayer->play();
    });
    QTimer::singleShot(2600, this, [this]() {
        if (m_terminado && m_explosionPlayer->mediaStatus() != QMediaPlayer::EndOfMedia) {
            if (m_explosionItem != nullptr) {
                delete m_explosionItem;
                m_explosionItem = nullptr;
            }
            mostrarTarjetaDerrota();
        }
    });
}

void JuegoView::mostrarTarjetaDerrota() {
    if (!m_terminado || m_tarjetaDerrotaMostrada) return;
    m_tarjetaDerrotaMostrada = true;
    m_explosionPlayer->stop();
    QMessageBox tarjeta(this);
    tarjeta.setWindowTitle("Fin de la partida");
    tarjeta.setIcon(QMessageBox::Critical);
    tarjeta.setText(m_tiempoRestanteSegundos <= 0
                        ? "<h2>¡SE ACABÓ EL TIEMPO!</h2>"
                        : "<h2>¡CHOCASTE!</h2>");
    tarjeta.setInformativeText(QString(
        "%1<br><br>"
        "<b>Nivel:</b> %2 &nbsp;&nbsp; <b>Puntaje:</b> %3<br>"
        "<b>Frutas:</b> %4 &nbsp;&nbsp; <b>Longitud:</b> %5")
        .arg(m_ultimoEfecto).arg(m_nivel).arg(m_progreso->puntaje())
        .arg(m_progreso->frutasComidas()).arg(m_serpiente->longitud()));
    QAbstractButton *reintentar = tarjeta.addButton("Reintentar nivel", QMessageBox::AcceptRole);
    tarjeta.addButton("Cerrar", QMessageBox::RejectRole);
    Dialogos::aplicarEstilo(tarjeta);
    tarjeta.exec();
    if (tarjeta.clickedButton() == reintentar) {
        AudioManager::instancia().reproducirJuego();
        reiniciar();
        iniciarCuentaRegresiva();
    } else {
        close();
    }
}

void JuegoView::ganarNivel() {
    m_terminado = true;
    m_temporizadorFrutas->stop();
    detenerAnimacionEnHilo();
    AudioManager::instancia().detenerMusica();
    AudioManager::instancia().reproducirEfecto(AudioManager::Efecto::Ganar);
    ++m_nivelesCompletados;

    if (m_configuracion.progresionAutomatica) {
        GestorUsuarios::marcarNivelHistoriaCompletado(m_usuario, m_nivel);
    }
    // El nivel actual sigue en m_progreso hasta cambiarlo o reiniciarlo.
    // Se guarda una sola vez junto con los niveles anteriores.
    guardarPuntajePartida();

    if (!m_configuracion.progresionAutomatica) {
        QMessageBox tarjeta(this);
        tarjeta.setWindowTitle("Nivel completado");
        tarjeta.setIcon(QMessageBox::Information);
        tarjeta.setText(QString("<h2>¡NIVEL %1 COMPLETADO!</h2>").arg(m_nivel));
        tarjeta.setInformativeText("¿Qué querés hacer ahora?");
        QAbstractButton *reintentar = tarjeta.addButton("Reintentar nivel",
                                                         QMessageBox::AcceptRole);
        tarjeta.addButton("Volver al menú", QMessageBox::RejectRole);
        Dialogos::aplicarEstilo(tarjeta);
        tarjeta.exec();
        if (tarjeta.clickedButton() == reintentar) {
            AudioManager::instancia().reproducirJuego();
            m_puntajePartida += m_progreso->puntaje();
            reiniciar();
            iniciarCuentaRegresiva();
        } else {
            close();
        }
        return;
    }

    if (m_nivel == NIVEL_3) {
        m_gestorPartida.marcarCompletada();
        QMessageBox tarjeta(this);
        tarjeta.setWindowTitle("Juego completado");
        tarjeta.setIcon(QMessageBox::Information);
        tarjeta.setText("<h2>¡JUEGO COMPLETADO!</h2>");
        tarjeta.setInformativeText("¡Completaste los 3 niveles de HISTORIA! "
                                   "Tu premio se encuentra en la Tienda.");
        Dialogos::aplicarEstilo(tarjeta);
        tarjeta.exec();
        close();
        return;
    }

    QMessageBox tarjeta(this);
    tarjeta.setWindowTitle("Nivel completado");
    tarjeta.setIcon(QMessageBox::Information);
    tarjeta.setText(QString("<h2>¡NIVEL %1 COMPLETADO!</h2>").arg(m_nivel));
    tarjeta.setInformativeText(QString(
        "Cumpliste las metas del nivel %1.<br><br>"
        "<b>Puntaje:</b> %2 &nbsp;&nbsp; <b>Frutas:</b> %3<br>"
        "<b>Longitud:</b> %4")
        .arg(m_nivel)
        .arg(m_progreso->puntaje())
        .arg(m_progreso->frutasComidas())
        .arg(m_serpiente->longitud()));
    Dialogos::aplicarEstilo(tarjeta);
    tarjeta.exec();

    AudioManager::instancia().reproducirJuego();

    // Transferir el puntaje solamente cuando se reemplaza el progreso actual.
    // El último nivel no pasa por aquí: al cerrar aún se cuenta desde m_progreso.
    m_puntajePartida += m_progreso->puntaje();
    if (m_nivel == NIVEL_1) {
        configurarNivel(NIVEL_2);
        cargarSprites();
        crearGrid();
    } else if (m_nivel == NIVEL_2) {
        configurarNivel(NIVEL_3);
        cargarSprites();
        crearGrid();
    }

    reiniciar();
    iniciarCuentaRegresiva();
}

int JuegoView::intervaloActual() const {
    int intervalo = m_nivel == NIVEL_3 ? INTERVALO_NIVEL_3 : INTERVALO_NIVEL_1;
    if (m_nivel == NIVEL_2) {
        const int aceleraciones = m_progreso->frutasComidas() / 2;
        intervalo = qMax(50, INTERVALO_NIVEL_2 - aceleraciones * 10);
    }

    if (m_turnosHielo > 0) {
        intervalo += 50;
    }
    if (m_turnosEnergia > 0) {
        intervalo = qMax(50, intervalo - 20);
    }
    return intervalo;
}

bool JuegoView::esNivelConBordesMortales() const {
    return m_nivel == NIVEL_2 || m_nivel == NIVEL_3;
}
