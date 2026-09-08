#include "juegoview.h"
#include "gestorconfiguracion.h"
#include "gestorusuarios.h"

#include "progreso.h"
#include "reglasmovimiento.h"
#include "snake.h"
#include "tablero.h"

#include <QBrush>
#include <QGuiApplication>
#include <QGraphicsRectItem>
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
#include <QVariantAnimation>
#include <QFont>
#include <QFontDatabase>
#include <QImage>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QAbstractButton>
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
      m_temporizador(new QTimer(this)),
      m_animadorPuntaje(new QTimer(this)),
      m_serpiente(new Snake()),
      m_tablero(new Tablero(10, 10)),
      m_progreso(new ProgresoNivel(META_FRUTAS_NIVEL_1,
                                   META_PUNTOS_NIVEL_1,
                                   META_LONGITUD_NIVEL_1)),
      m_direccionX(1),
      m_direccionY(0),
      m_manzanaX(0),
      m_manzanaY(0),
      m_tipoObjeto(MANZANA),
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
      m_tarjetaDerrotaMostrada(false)
{
    m_esquemaControles = GestorConfiguracion::cargarControl(m_usuario) == "WASD"
        ? EsquemaControles::WASD : EsquemaControles::Flechas;
    setScene(m_escena);
    m_escena->setBackgroundBrush(QColor(13, 18, 26));
    setFocusPolicy(Qt::StrongFocus);
    QFontDatabase::addApplicationFont(":/assets/Fredoka-Variable.ttf");

    configurarNivel(nivelInicial);
    cargarSprites();
    crearGrid();
    reiniciar();

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

    connect(m_temporizador, &QTimer::timeout, this, [this]() { avanzarJuego(); });
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
    m_temporizador->start(intervaloActual());
}

JuegoView::~JuegoView() {
    m_temporizador->stop();
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
    if (evento->key() == Qt::Key_1 || evento->key() == Qt::Key_2 || evento->key() == Qt::Key_3) {
        const int nivel = evento->key() == Qt::Key_1 ? NIVEL_1
                          : evento->key() == Qt::Key_2 ? NIVEL_2 : NIVEL_3;
        configurarNivel(nivel);
        cargarSprites();
        crearGrid();
        reiniciar();
        m_temporizador->start(intervaloActual());
        return;
    }

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

void JuegoView::closeEvent(QCloseEvent *evento) {
    guardarPuntajePartida();
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

    for (int y = 0; y < MAX_FILAS; ++y) {
        for (int x = 0; x < MAX_COLUMNAS; ++x) {
            m_sprites[y][x] = nullptr;
        }
    }

    const QPen borde(QColor(48, 58, 74));

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
    const QString rutaCabeza = skin == "gato" ? ":/assets/skin_gato.png"
                              : skin == "dragon" ? ":/assets/skin_dragon.png"
                              : skin == "burro" ? ":/assets/skin_burro.png"
                              : skin == "thanos" ? ":/assets/skin_thanos.png"
                              : skin == "spiderman" ? ":/assets/skin_spiderman.png"
                              : skin == "miles" ? ":/assets/skin_miles.png"
                              : skin == "personaje" ? ":/assets/skin_personaje.png"
                              : ":/assets/cabeza_snake.png";
    const QPixmap cabezaCanva(rutaCabeza);
    if (!cabezaCanva.isNull()) {
        *m_spriteCabeza = cabezaCanva.scaled(44, 44, Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation);
        if (skin == "veneno" || skin == "hielo" || skin == "fuego" || skin == "cosmica") {
            QImage cabeza = m_spriteCabeza->toImage();
            QPainter pintor(&cabeza);
            pintor.setCompositionMode(QPainter::CompositionMode_SourceAtop);
            pintor.fillRect(cabeza.rect(), QColor(colorPrincipal.red(),
                                                  colorPrincipal.green(),
                                                  colorPrincipal.blue(), 65));
            pintor.end();
            *m_spriteCabeza = QPixmap::fromImage(cabeza);
        }
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

    // El cuerpo se representa con piezas circulares para evitar estirar
    // la ilustración vertical de Canva dentro de cada celda.
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
    if (skin == "thanos") {
        const QColor gemas[] = {QColor("#9b59ff"), QColor("#ffd740"),
                                QColor("#29b6f6"), QColor("#ef5350"),
                                QColor("#66bb6a"), QColor("#ff8a65")};
        for (int i = 0; i < 6; ++i) {
            pintorCuerpo.setBrush(gemas[i]);
            pintorCuerpo.drawEllipse(5 + (i % 3) * 10, 4 + (i / 3) * 17, 5, 5);
        }
    }
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
    inicializarObstaculosMoviles();
    m_temporizador->stop();
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

    if (m_nivel == NIVEL_3 || (m_configuracion.esAleatorio
                               && m_configuracion.obstaculosMoviles)) {
        for (int i = 0; i < MAX_OBSTACULOS_MOVILES; ++i) {
            m_tablero->poner(m_obstaculos[i].x(), m_obstaculos[i].y(), OBSTACULO);
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

    static const int obstaculos[][2] = {
        {2, 4}, {3, 4}, {4, 4}, {10, 4}, {11, 4}, {12, 4},
        {6, 6}, {7, 6}, {8, 6},
        {6, 8}, {7, 8}, {8, 8},
        {2, 10}, {3, 10}, {4, 10}, {10, 10}, {11, 10}, {12, 10}
    };

    for (const auto &obstaculo : obstaculos) {
        m_tablero->poner(obstaculo[0], obstaculo[1], OBSTACULO);
    }
}

void JuegoView::moverObstaculos() {
    const bool usaMoviles = m_configuracion.obstaculosMoviles
                         && (m_nivel == NIVEL_3 || m_configuracion.esAleatorio);
    if (!usaMoviles
        || (++m_turnoObstaculos % 4) != 0) {
        return;
    }

    if (m_turnosObstaculosCongelados > 0) {
        --m_turnosObstaculosCongelados;
        return;
    }

    for (int i = 0; i < 4; ++i) {
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
        || m_serpiente->ocupa(x, y)) {
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
    }
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
        delete animacion;
    }
    m_animacionesMovimiento.clear();
    const QHash<int, QPointF> posicionesAnteriores = m_posicionesVisuales;

    for (int y = 0; y < m_filas; ++y) {
        for (int x = 0; x < m_columnas; ++x) {
            if (m_sprites[y][x] != nullptr) {
                delete m_sprites[y][x];
                m_sprites[y][x] = nullptr;
            }

            const int casilla = m_tablero->valor(x, y);
            QColor color(Qt::transparent);

            if (casilla == SERPIENTE) {
                color = Qt::transparent;
            } else if (casilla == MANZANA) {
                color = Qt::transparent;
            } else if (casilla == MANZANA_DORADA) {
                color = Qt::transparent;
            } else if (casilla == FRUTA_GRANDE) {
                color = Qt::transparent;
            } else if (casilla == FRUTA_ENERGETICA) {
                color = Qt::transparent;
            } else if (casilla == CAJA_MISTERIOSA) {
                color = Qt::transparent;
            } else if (casilla == OBSTACULO) {
                color = QColor(116, 82, 58);
            }

            m_casillas[y][x]->setBrush(QBrush(color));

            if (casilla == SERPIENTE) {
                const bool esCabeza = x == m_serpiente->cabezaX()
                                   && y == m_serpiente->cabezaY();
                const bool esCola = !esCabeza && m_serpiente->ocupaCola(x, y);
                const QPixmap &sprite = esCabeza ? *m_spriteCabeza
                                      : esCola ? *m_spriteCola : *m_spriteCuerpo;
                if (!sprite.isNull()) {
                    QGraphicsPixmapItem *item = m_escena->addPixmap(sprite);
                    int indiceSegmento = 0;
                    const Nodo *segmento = m_serpiente->cabeza();
                    while (segmento != nullptr
                           && (segmento->x != x || segmento->y != y)) {
                        ++indiceSegmento;
                        segmento = segmento->siguiente;
                    }
                    const int margen = esCabeza ? -2 : 2;
                    const QPointF destino(m_margenColiseo + x * TAMANO_CELDA + margen,
                                          m_margenColiseo + y * TAMANO_CELDA + margen);
                    const QPointF inicio = posicionesAnteriores.contains(indiceSegmento)
                        ? (indiceSegmento == 0 ? posicionesAnteriores.value(0)
                                               : posicionesAnteriores.value(indiceSegmento - 1))
                        : destino;
                    item->setPos(inicio);
                    if (inicio != destino) {
                        auto *animacion = new QVariantAnimation(this);
                        animacion->setStartValue(inicio);
                        animacion->setEndValue(destino);
                        animacion->setDuration(qBound(40, intervaloActual() * 8 / 10, 120));
                        connect(animacion, &QVariantAnimation::valueChanged,
                                [item](const QVariant &valor) {
                                    item->setPos(valor.toPointF());
                                });
                        connect(animacion, &QVariantAnimation::finished, this,
                                [this, animacion]() {
                                    m_animacionesMovimiento.removeOne(animacion);
                                    animacion->deleteLater();
                                });
                        m_animacionesMovimiento.append(animacion);
                        animacion->start();
                    }
                    m_posicionesVisuales[indiceSegmento] = destino;

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

                    m_sprites[y][x] = item;
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
                    auto *item = m_escena->addPixmap(*sprite);
                    item->setPos(m_margenColiseo + x * TAMANO_CELDA + 2,
                                 m_margenColiseo + y * TAMANO_CELDA + 2);
                    m_sprites[y][x] = item;
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
    m_temporizador->setInterval(intervaloActual());
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
        if (comioFruta || tomoCaja) m_objetos[indiceObjeto].activo = false;
    }
    const int crecimiento = comioFruta
                                ? frutaComida.crecimiento(m_nivel == NIVEL_3)
                                : 0;
    m_serpiente->avanzar(nuevaX, nuevaY, crecimiento);
    actualizarMapa();

    if (comioFruta) {
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
        m_temporizador->setInterval(intervaloActual());
        if (m_progreso->gano()) {
            redibujar();
            actualizarInformacion();
            ganarNivel();
            return;
        }

        generarManzana();
    } else if (tomoCaja) {
        aplicarItem();
        generarManzana();
    }

    redibujar();
    actualizarInformacion();

}

void JuegoView::terminarJuego() {
    m_terminado = true;
    m_temporizador->stop();
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
    tarjeta.setStyleSheet(
        "QMessageBox { background:#0d121a; color:#ebf0f5; }"
        "QLabel { color:#ebf0f5; font-family:'Fredoka'; font-size:14px; }"
        "QPushButton { background:#26384a; color:#ffffff; border:1px solid #5cb6e6;"
        " border-radius:8px; padding:8px 16px; min-width:120px; }"
        "QPushButton:hover { background:#34536a; }");
    tarjeta.exec();
    if (tarjeta.clickedButton() == reintentar) {
        reiniciar();
        m_temporizador->start(intervaloActual());
    } else {
        close();
    }
}

void JuegoView::ganarNivel() {
    m_terminado = true;
    m_temporizador->stop();
    ++m_nivelesCompletados;

    if (m_configuracion.progresionAutomatica) {
        m_puntajePartida += m_progreso->puntaje();
        guardarPuntajePartida();
    }

    if (!m_configuracion.progresionAutomatica) {
        QMessageBox tarjeta(this);
        tarjeta.setWindowTitle("Nivel completado");
        tarjeta.setIcon(QMessageBox::Information);
        tarjeta.setText(QString("<h2>¡NIVEL %1 COMPLETADO!</h2>").arg(m_nivel));
        tarjeta.setInformativeText("¿Qué querés hacer ahora?");
        QAbstractButton *reintentar = tarjeta.addButton("Reintentar nivel",
                                                         QMessageBox::AcceptRole);
        tarjeta.addButton("Volver al menú", QMessageBox::RejectRole);
        tarjeta.setStyleSheet(
            "QMessageBox { background:#0d121a; color:#ebf0f5; }"
            "QLabel { color:#ebf0f5; font-family:'Fredoka'; font-size:14px; }"
            "QPushButton { background:#26384a; color:#ffffff; border:1px solid #5cb6e6;"
            " border-radius:8px; padding:8px 16px; min-width:120px; }"
            "QPushButton:hover { background:#34536a; }");
        tarjeta.exec();
        if (tarjeta.clickedButton() == reintentar) {
            reiniciar();
            m_temporizador->start(intervaloActual());
        } else {
            close();
        }
        return;
    }

    if (m_nivel == NIVEL_3) {
        m_gestorPartida.marcarCompletada();
        QMessageBox::information(this, "Juego completado",
                                 "Completaste todos los niveles de Snake.");
        setWindowTitle("Snake - Juego completado");
        actualizarInformacion();
        return;
    }

    QMessageBox::information(this, "Nivel completado",
                             QString("Cumpliste las metas del nivel %1.").arg(m_nivel));

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
    m_temporizador->start(intervaloActual());
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
        intervalo = qMax(30, intervalo - 20);
    }
    return intervalo;
}

bool JuegoView::esNivelConBordesMortales() const {
    return m_nivel == NIVEL_2 || m_nivel == NIVEL_3;
}
