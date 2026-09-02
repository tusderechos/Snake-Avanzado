#include "juegoview.h"

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
#include <QMessageBox>
#include <QPen>
#include <QRandomGenerator>
#include <QScreen>
#include <QTimer>
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

JuegoView::JuegoView(int nivelInicial, ConfiguracionJuego configuracion)
    : m_escena(new QGraphicsScene(this)),
      m_informacion(nullptr),
      m_temporizador(new QTimer(this)),
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
      m_cambioDireccionPendiente(false),
      m_esquemaControles(EsquemaControles::Flechas),
      m_terminado(false),
      m_configuracion(configuracion),
      m_spriteCabeza(new QPixmap()),
      m_spriteCuerpo(new QPixmap()),
      m_spriteCola(new QPixmap()),
      m_explosionPlayer(new QMediaPlayer(this)),
      m_explosionAudio(new QAudioOutput(this)),
      m_explosionSink(new QVideoSink(this)),
      m_explosionItem(nullptr),
      m_tarjetaDerrotaMostrada(false)
{
    setScene(m_escena);
    m_escena->setBackgroundBrush(QColor(13, 18, 26));
    setFocusPolicy(Qt::StrongFocus);
    QFontDatabase::addApplicationFont(":/assets/Fredoka-Variable.ttf");

    configurarNivel(nivelInicial);
    crearGrid();
    cargarSprites();
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
    m_temporizador->start(intervaloActual());
}

JuegoView::~JuegoView() {
    m_temporizador->stop();
    delete m_spriteCabeza;
    delete m_spriteCuerpo;
    delete m_spriteCola;
}

void JuegoView::keyPressEvent(QKeyEvent *evento) {
    if (evento->key() == Qt::Key_1 || evento->key() == Qt::Key_2 || evento->key() == Qt::Key_3) {
        const int nivel = evento->key() == Qt::Key_1 ? NIVEL_1
                          : evento->key() == Qt::Key_2 ? NIVEL_2 : NIVEL_3;
        configurarNivel(nivel);
        crearGrid();
        reiniciar();
        m_temporizador->start(intervaloActual());
        return;
    }

    if (Controles::esMovimiento(evento->key())) {
        m_esquemaControles = Controles::detectar(evento->key());
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

void JuegoView::crearGrid() {
    m_escena->clear();
    m_informacion = nullptr;

    for (int y = 0; y < MAX_FILAS; ++y) {
        for (int x = 0; x < MAX_COLUMNAS; ++x) {
            m_sprites[y][x] = nullptr;
        }
    }

    const QPen borde(QColor(48, 58, 74));

    for (int y = 0; y < m_filas; ++y) {
        for (int x = 0; x < m_columnas; ++x) {
            m_casillas[y][x] = m_escena->addRect(
                x * TAMANO_CELDA,
                y * TAMANO_CELDA,
                TAMANO_CELDA,
                TAMANO_CELDA,
                borde,
                QBrush(QColor(21, 28, 38)));
        }
    }

    m_informacion = m_escena->addText("");
    m_informacion->setDefaultTextColor(QColor(235, 240, 245));
    m_informacion->setFont(QFont("Fredoka", 12));
    m_informacion->setTextWidth(190);
    m_informacion->setPos(m_columnas * TAMANO_CELDA + 18, 24);

    m_escena->setSceneRect(0, 0, m_columnas * TAMANO_CELDA + 220, m_filas * TAMANO_CELDA);
    setFixedSize(m_columnas * TAMANO_CELDA + 224, m_filas * TAMANO_CELDA + 4);

    const QRect areaDisponible = QGuiApplication::primaryScreen()->availableGeometry();
    move(areaDisponible.center() - QPoint(width() / 2, height() / 2));
}

void JuegoView::cargarSprites() {
    const QPixmap cabezaCanva(":/assets/cabeza_snake.png");
    if (!cabezaCanva.isNull()) {
        *m_spriteCabeza = cabezaCanva.scaled(44, 44, Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation);
    }

    QImage hoja(":/assets/snake_sheet.png");
    if (hoja.isNull()) {
        return;
    }

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

    // El cuerpo se representa con piezas circulares para evitar estirar
    // la ilustración vertical de Canva dentro de cada celda.
    QImage cuerpo(36, 36, QImage::Format_ARGB32_Premultiplied);
    cuerpo.fill(Qt::transparent);
    QPainter pintorCuerpo(&cuerpo);
    pintorCuerpo.setRenderHint(QPainter::Antialiasing);
    pintorCuerpo.setPen(Qt::NoPen);
    pintorCuerpo.setBrush(QColor(92, 190, 47));
    pintorCuerpo.drawEllipse(2, 2, 32, 32);
    pintorCuerpo.setBrush(QColor(46, 125, 38));
    pintorCuerpo.drawEllipse(11, 8, 6, 6);
    pintorCuerpo.drawEllipse(21, 19, 6, 6);
    pintorCuerpo.end();
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
    pintorCola.end();
    *m_spriteCola = QPixmap::fromImage(cola);
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
        {3, 4, 1, 0},  {16, 15, -1, 0},
        {7, 3, 0, 1},  {12, 16, 0, -1},
        {5, 5, 0, 0},  {14, 14, 0, 0}
    };

    for (int i = 0; i < MAX_OBSTACULOS_MOVILES; ++i) {
        m_obstaculos[i].configurar(posicionesIniciales[i][0], posicionesIniciales[i][1],
                                   posicionesIniciales[i][2], posicionesIniciales[i][3], i < 4);
    }
}

void JuegoView::construirObstaculos() {
    if (m_nivel == NIVEL_1 || !m_configuracion.obstaculos) {
        return;
    }

    if (m_nivel == NIVEL_3) {
        for (int i = 0; i < MAX_OBSTACULOS_MOVILES; ++i) {
            m_tablero->poner(m_obstaculos[i].x(), m_obstaculos[i].y(), OBSTACULO);
        }
        return;
    }

    static const int obstaculos[][2] = {
        {5, 4}, {6, 4}, {7, 4}, {9, 4}, {10, 4}, {11, 4},
        {5, 10}, {6, 10}, {7, 10}, {9, 10}, {10, 10}, {11, 10},
        {4, 5}, {4, 6}, {4, 8}, {4, 9},
        {11, 5}, {11, 6}, {11, 8}, {11, 9}
    };

    for (const auto &obstaculo : obstaculos) {
        m_tablero->poner(obstaculo[0], obstaculo[1], OBSTACULO);
    }
}

void JuegoView::moverObstaculos() {
    if (m_nivel != NIVEL_3 || !m_configuracion.obstaculosMoviles
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

    for (int i = 0; i < MAX_OBSTACULOS_MOVILES; ++i) {
        if (i != ignorar && m_obstaculos[i].x() == x && m_obstaculos[i].y() == y) {
            return false;
        }
    }

    return true;
}

void JuegoView::reiniciar() {
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
    inicializarObstaculosMoviles();

    actualizarMapa();
    construirObstaculos();
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
}

void JuegoView::redibujar() {
    for (int y = 0; y < m_filas; ++y) {
        for (int x = 0; x < m_columnas; ++x) {
            if (m_sprites[y][x] != nullptr) {
                delete m_sprites[y][x];
                m_sprites[y][x] = nullptr;
            }

            const int casilla = m_tablero->valor(x, y);
            QColor color(21, 28, 38);

            if (casilla == SERPIENTE) {
                color = QColor(21, 28, 38);
            } else if (casilla == MANZANA) {
                color = QColor(231, 76, 60);
            } else if (casilla == MANZANA_DORADA) {
                color = QColor(241, 196, 15);
            } else if (casilla == FRUTA_GRANDE) {
                color = QColor(239, 139, 61);
            } else if (casilla == FRUTA_ENERGETICA) {
                color = QColor(174, 91, 214);
            } else if (casilla == CAJA_MISTERIOSA) {
                color = QColor(92, 180, 230);
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
                    const int margen = esCabeza ? -2 : 2;
                    item->setPos(x * TAMANO_CELDA + margen, y * TAMANO_CELDA + margen);

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
            }
        }
    }
}

void JuegoView::generarManzana() {
    for (int intento = 0; intento < m_columnas * m_filas; ++intento) {
        const int x = QRandomGenerator::global()->bounded(m_columnas);
        const int y = QRandomGenerator::global()->bounded(m_filas);

        if (m_tablero->valor(x, y) == VACIO) {
            m_manzanaX = x;
            m_manzanaY = y;
            m_tipoObjeto = MANZANA;
            m_frutaActual.configurar(TipoFruta::Normal);
            if (m_nivel == NIVEL_3) {
                const bool cajaGarantizada = m_turnosDesdeCaja >= GARANTIA_CAJA_TURNOS
                                           || m_frutasDesdeCaja >= GARANTIA_CAJA_FRUTAS;
                const bool cooldownCumplido = m_turnosDesdeCaja >= COOLDOWN_CAJA_TURNOS;
                const int azarCaja = QRandomGenerator::global()->bounded(100);
                if (m_configuracion.items && m_configuracion.aparicionAleatoria
                    && (cajaGarantizada || (cooldownCumplido && azarCaja < 15))) {
                    m_tipoObjeto = CAJA_MISTERIOSA;
                    m_turnosDesdeCaja = 0;
                    m_frutasDesdeCaja = 0;
                } else {
                    // Entre las frutas (sin contar cajas), la normal ocupa
                    // el 75%; el 25% restante se reparte entre especiales.
                    const int azarFruta = QRandomGenerator::global()->bounded(100);
                    if (m_configuracion.frutasEspeciales
                        && azarFruta >= 75 && azarFruta < 85) {
                        m_tipoObjeto = MANZANA_DORADA;
                        m_frutaActual.configurar(TipoFruta::Dorada);
                    } else if (m_configuracion.frutasEspeciales
                               && azarFruta >= 85 && azarFruta < 95) {
                        m_tipoObjeto = FRUTA_GRANDE;
                        m_frutaActual.configurar(TipoFruta::Grande);
                    } else if (m_configuracion.frutasEspeciales && azarFruta >= 95) {
                        m_tipoObjeto = FRUTA_ENERGETICA;
                        m_frutaActual.configurar(TipoFruta::Energetica);
                    }
                }
            }
            m_tablero->poner(x, y, m_tipoObjeto);
            return;
        }
    }
}

void JuegoView::actualizarInformacion() {
    QString texto =
        QString("NIVEL %1\n\nPUNTAJE\n%2 / %3\n\nFRUTAS\n%4 / %5\n\nLONGITUD\n%6 / %7\n\nVELOCIDAD\n%8 ms\n\nMETA\nCompleta las 3 metas")
            .arg(m_nivel)
            .arg(m_progreso->puntaje())
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
        m_turnosEnergia = 100;
        m_ultimoEfecto = "Rayo: +30 puntos";
        break;
    case TipoItem::Tijeras:
        m_serpiente->reducirSegmentos(-item.segmentosDelta());
        m_progreso->registrarItem(item.puntos(), m_serpiente->longitud());
        m_ultimoEfecto = "Tijeras: -2 segmentos";
        break;
    case TipoItem::Bomba:
        m_serpiente->reducirSegmentos(-item.segmentosDelta());
        m_progreso->registrarItem(item.puntos(), m_serpiente->longitud());
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
    if (m_nivel == NIVEL_3) {
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
    construirObstaculos();
    if (m_turnosTrampa > 0 && m_trampaX >= 0) {
        m_tablero->poner(m_trampaX, m_trampaY, OBSTACULO);
    }
    if (m_tablero->valor(m_manzanaX, m_manzanaY) == VACIO) {
        m_tablero->poner(m_manzanaX, m_manzanaY, m_tipoObjeto);
    } else {
        generarManzana();
    }

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

    const bool comioFruta = destino == MANZANA || destino == MANZANA_DORADA
                          || destino == FRUTA_GRANDE || destino == FRUTA_ENERGETICA;
    const bool tomoCaja = destino == CAJA_MISTERIOSA;
    const int crecimiento = comioFruta
                                ? m_frutaActual.crecimiento(m_nivel == NIVEL_3)
                                : 0;
    m_serpiente->avanzar(nuevaX, nuevaY, crecimiento);
    actualizarMapa();
    construirObstaculos();
    if (m_turnosTrampa > 0 && m_trampaX >= 0) {
        m_tablero->poner(m_trampaX, m_trampaY, OBSTACULO);
    }

    if (comioFruta) {
        m_progreso->registrarFruta(m_frutaActual.puntos(),
                                   m_serpiente->longitud(),
                                   m_frutaActual.frutasContadas());
        if (m_nivel == NIVEL_3) {
            m_frutasDesdeCaja += m_frutaActual.frutasContadas();
        }
        if (m_frutaActual.tipo() == TipoFruta::Energetica) {
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
    } else {
        m_tablero->poner(m_manzanaX, m_manzanaY, m_tipoObjeto);
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
    m_explosionItem->setPos(m_serpiente->cabezaX() * TAMANO_CELDA - 65,
                            m_serpiente->cabezaY() * TAMANO_CELDA - 65);
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

    if (!m_configuracion.progresionAutomatica) {
        setWindowTitle(QString("Snake - Nivel %1 completado").arg(m_nivel));
        actualizarInformacion();
        return;
    }

    if (m_nivel == NIVEL_1) {
        configurarNivel(NIVEL_2);
        crearGrid();
    } else if (m_nivel == NIVEL_2) {
        configurarNivel(NIVEL_3);
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
