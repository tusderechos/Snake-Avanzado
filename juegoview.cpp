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
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QTransform>

JuegoView::JuegoView()
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
      m_nivel(NIVEL_1),
      m_columnas(10),
      m_filas(10),
      m_metaFrutas(META_FRUTAS_NIVEL_1),
      m_metaPuntos(META_PUNTOS_NIVEL_1),
      m_metaLongitud(META_LONGITUD_NIVEL_1),
      m_obstaculoX{},
      m_obstaculoY{},
      m_obstaculoDireccionX{},
      m_obstaculoDireccionY{},
      m_turnoObstaculos(0),
      m_cambioDireccionPendiente(false),
      m_terminado(false),
      m_spriteCabeza(new QPixmap()),
      m_spriteCuerpo(new QPixmap()),
      m_spriteCola(new QPixmap())
{
    setScene(m_escena);
    m_escena->setBackgroundBrush(QColor(13, 18, 26));
    setFocusPolicy(Qt::StrongFocus);

    configurarNivel(NIVEL_1);
    crearGrid();
    cargarSprites();
    reiniciar();

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

    if (evento->key() == Qt::Key_Left && !m_cambioDireccionPendiente && m_direccionX != 1) {
        m_direccionX = -1;
        m_direccionY = 0;
        m_cambioDireccionPendiente = true;
    } else if (evento->key() == Qt::Key_Right && !m_cambioDireccionPendiente && m_direccionX != -1) {
        m_direccionX = 1;
        m_direccionY = 0;
        m_cambioDireccionPendiente = true;
    } else if (evento->key() == Qt::Key_Up && !m_cambioDireccionPendiente && m_direccionY != 1) {
        m_direccionX = 0;
        m_direccionY = -1;
        m_cambioDireccionPendiente = true;
    } else if (evento->key() == Qt::Key_Down && !m_cambioDireccionPendiente && m_direccionY != -1) {
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
    m_informacion->setFont(QFont("Arial", 12));
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
        m_obstaculoX[i] = posicionesIniciales[i][0];
        m_obstaculoY[i] = posicionesIniciales[i][1];
        m_obstaculoDireccionX[i] = posicionesIniciales[i][2];
        m_obstaculoDireccionY[i] = posicionesIniciales[i][3];
    }
}

void JuegoView::construirObstaculos() {
    if (m_nivel == NIVEL_1) {
        return;
    }

    if (m_nivel == NIVEL_3) {
        for (int i = 0; i < MAX_OBSTACULOS_MOVILES; ++i) {
            m_tablero->poner(m_obstaculoX[i], m_obstaculoY[i], OBSTACULO);
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
    if (m_nivel != NIVEL_3 || (++m_turnoObstaculos % 4) != 0) {
        return;
    }

    for (int i = 0; i < 4; ++i) {
        int nuevaX = m_obstaculoX[i] + m_obstaculoDireccionX[i];
        int nuevaY = m_obstaculoY[i] + m_obstaculoDireccionY[i];

        const bool fueraDelRecorrido = nuevaX < 2 || nuevaX > m_columnas - 3
                                     || nuevaY < 2 || nuevaY > m_filas - 3;
        if (fueraDelRecorrido) {
            m_obstaculoDireccionX[i] *= -1;
            m_obstaculoDireccionY[i] *= -1;
            nuevaX = m_obstaculoX[i] + m_obstaculoDireccionX[i];
            nuevaY = m_obstaculoY[i] + m_obstaculoDireccionY[i];
        }

        if (posicionObstaculoDisponible(nuevaX, nuevaY, i)) {
            m_obstaculoX[i] = nuevaX;
            m_obstaculoY[i] = nuevaY;
        } else {
            // Si la serpiente u otro obstáculo ocupa el destino, espera
            // un turno y cambia el sentido para evitar apariciones injustas.
            m_obstaculoDireccionX[i] *= -1;
            m_obstaculoDireccionY[i] *= -1;
        }
    }
}

bool JuegoView::posicionObstaculoDisponible(int x, int y, int ignorar) const {
    if (x < 0 || x >= m_columnas || y < 0 || y >= m_filas
        || m_serpiente->ocupa(x, y)) {
        return false;
    }

    for (int i = 0; i < MAX_OBSTACULOS_MOVILES; ++i) {
        if (i != ignorar && m_obstaculoX[i] == x && m_obstaculoY[i] == y) {
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
            m_tablero->poner(x, y, MANZANA);
            return;
        }
    }
}

void JuegoView::actualizarInformacion() {
    m_informacion->setPlainText(
        QString("NIVEL %1\n\nPUNTAJE\n%2\n\nFRUTAS\n%3 / %4\n\nLONGITUD\n%5 / %6\n\nVELOCIDAD\n%7 ms\n\nMETA\nCompleta las 3 metas")
            .arg(m_nivel)
            .arg(m_progreso->puntaje())
            .arg(m_progreso->frutasComidas())
            .arg(m_metaFrutas)
            .arg(m_serpiente->longitud())
            .arg(m_metaLongitud)
            .arg(intervaloActual()));
}

void JuegoView::avanzarJuego() {
    if (m_terminado) {
        return;
    }

    // Solo se permite un giro entre dos actualizaciones del QTimer.
    m_cambioDireccionPendiente = false;
    moverObstaculos();

    // El tablero debe representar las posiciones nuevas antes de revisar
    // la siguiente casilla de la serpiente.
    actualizarMapa();
    construirObstaculos();
    if (m_tablero->valor(m_manzanaX, m_manzanaY) == VACIO) {
        m_tablero->poner(m_manzanaX, m_manzanaY, MANZANA);
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

    if (destino == SERPIENTE && !m_serpiente->ocupaCola(nuevaX, nuevaY)) {
        terminarJuego();
        return;
    }

    const bool comioManzana = destino == MANZANA;
    m_serpiente->avanzar(nuevaX, nuevaY, comioManzana);
    actualizarMapa();
    construirObstaculos();

    if (comioManzana) {
        m_progreso->registrarFruta(PUNTOS_MANZANA, m_serpiente->longitud());
        m_temporizador->setInterval(intervaloActual());
        if (m_progreso->gano()) {
            redibujar();
            actualizarInformacion();
            ganarNivel();
            return;
        }

        generarManzana();
    } else {
        m_tablero->poner(m_manzanaX, m_manzanaY, MANZANA);
    }

    redibujar();
    actualizarInformacion();

}

void JuegoView::terminarJuego() {
    m_terminado = true;
    m_temporizador->stop();
    QMessageBox::information(this, "Fin de la partida",
                             esNivelConBordesMortales()
                                 ? "La serpiente choco con un borde, obstaculo o su cuerpo."
                                 : "La serpiente choco con su cuerpo.");
    reiniciar();
    m_temporizador->start(intervaloActual());
}

void JuegoView::ganarNivel() {
    m_terminado = true;
    m_temporizador->stop();

    if (m_nivel == NIVEL_3) {
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
        crearGrid();
    } else if (m_nivel == NIVEL_2) {
        configurarNivel(NIVEL_3);
        crearGrid();
    }

    reiniciar();
    m_temporizador->start(intervaloActual());
}

int JuegoView::intervaloActual() const {
    if (m_nivel == NIVEL_2) {
        const int aceleraciones = m_progreso->frutasComidas() / 2;
        return qMax(50, INTERVALO_NIVEL_2 - aceleraciones * 10);
    }

    return m_nivel == NIVEL_3 ? INTERVALO_NIVEL_3 : INTERVALO_NIVEL_1;
}

bool JuegoView::esNivelConBordesMortales() const {
    return m_nivel == NIVEL_2 || m_nivel == NIVEL_3;
}
