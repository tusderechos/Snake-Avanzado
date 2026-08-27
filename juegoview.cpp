#include "juegoview.h"

#include "progreso.h"
#include "reglasmovimiento.h"
#include "snake.h"
#include "tablero.h"

#include <QBrush>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPen>
#include <QRandomGenerator>
#include <QTimer>
#include <QFont>

JuegoView::JuegoView()
    : m_escena(new QGraphicsScene(this)),
      m_informacion(nullptr),
      m_temporizador(new QTimer(this)),
      m_serpiente(new Snake()),
      m_tablero(new Tablero(COLUMNAS, FILAS)),
      m_progreso(new ProgresoNivel(META_FRUTAS, META_PUNTOS, META_LONGITUD)),
      m_direccionX(1),
      m_direccionY(0),
      m_manzanaX(0),
      m_manzanaY(0),
      m_terminado(false)
{
    setScene(m_escena);
    m_escena->setBackgroundBrush(QColor(13, 18, 26));
    setWindowTitle("Snake - Fase 1");
    setFixedSize(COLUMNAS * TAMANO_CELDA + 4, FILAS * TAMANO_CELDA + 4);
    setFocusPolicy(Qt::StrongFocus);

    crearGrid();
    reiniciar();

    connect(m_temporizador, &QTimer::timeout, this, [this]() { avanzarJuego(); });
    m_temporizador->start(INTERVALO_MOVIMIENTO);
}

JuegoView::~JuegoView() {
    m_temporizador->stop();

    delete m_serpiente;
    m_serpiente = nullptr;

    delete m_tablero;
    m_tablero = nullptr;

    delete m_progreso;
    m_progreso = nullptr;
}

void JuegoView::keyPressEvent(QKeyEvent *evento) {
    if (evento->key() == Qt::Key_Left && m_direccionX != 1) {
        m_direccionX = -1;
        m_direccionY = 0;
    } else if (evento->key() == Qt::Key_Right && m_direccionX != -1) {
        m_direccionX = 1;
        m_direccionY = 0;
    } else if (evento->key() == Qt::Key_Up && m_direccionY != 1) {
        m_direccionX = 0;
        m_direccionY = -1;
    } else if (evento->key() == Qt::Key_Down && m_direccionY != -1) {
        m_direccionX = 0;
        m_direccionY = 1;
    } else {
        QGraphicsView::keyPressEvent(evento);
    }
}

void JuegoView::crearGrid() {
    const QPen borde(QColor(48, 58, 74));

    for (int y = 0; y < FILAS; ++y) {
        for (int x = 0; x < COLUMNAS; ++x) {
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
    m_informacion->setPos(COLUMNAS * TAMANO_CELDA + 18, 24);

    m_escena->setSceneRect(0, 0, COLUMNAS * TAMANO_CELDA + 220, FILAS * TAMANO_CELDA);
    setFixedSize(COLUMNAS * TAMANO_CELDA + 224, FILAS * TAMANO_CELDA + 4);
}

void JuegoView::reiniciar() {
    m_serpiente->limpiar();
    m_serpiente->insertarCabeza(1, 3);
    m_serpiente->insertarCabeza(2, 3);
    m_serpiente->insertarCabeza(3, 3);

    m_direccionX = 1;
    m_direccionY = 0;
    m_terminado = false;
    m_progreso->reiniciar();

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
}

void JuegoView::redibujar() {
    for (int y = 0; y < FILAS; ++y) {
        for (int x = 0; x < COLUMNAS; ++x) {
            const int casilla = m_tablero->valor(x, y);
            QColor color(21, 28, 38);

            if (casilla == SERPIENTE) {
                color = (x == m_serpiente->cabezaX() && y == m_serpiente->cabezaY())
                            ? QColor(100, 222, 125)
                            : QColor(52, 168, 83);
            } else if (casilla == MANZANA) {
                color = QColor(231, 76, 60);
            }

            m_casillas[y][x]->setBrush(QBrush(color));
        }
    }
}

void JuegoView::generarManzana() {
    for (int intento = 0; intento < COLUMNAS * FILAS; ++intento) {
        const int x = QRandomGenerator::global()->bounded(COLUMNAS);
        const int y = QRandomGenerator::global()->bounded(FILAS);

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
        QString("FASE 1\n\nPUNTAJE\n%1\n\nFRUTAS\n%2 / %3\n\nLONGITUD\n%4 / %5\n\nMETA\nCompleta las 3 metas")
            .arg(m_progreso->puntaje())
            .arg(m_progreso->frutasComidas())
            .arg(META_FRUTAS)
            .arg(m_serpiente->longitud())
            .arg(META_LONGITUD));
}

void JuegoView::avanzarJuego() {
    if (m_terminado) {
        return;
    }

    int nuevaX = m_serpiente->cabezaX() + m_direccionX;
    int nuevaY = m_serpiente->cabezaY() + m_direccionY;

    nuevaX = ReglasMovimiento::envolverCoordenada(nuevaX, COLUMNAS);
    nuevaY = ReglasMovimiento::envolverCoordenada(nuevaY, FILAS);

    const int destino = m_tablero->valor(nuevaX, nuevaY);

    if (destino == SERPIENTE && !m_serpiente->ocupaCola(nuevaX, nuevaY)) {
        terminarJuego();
        return;
    }

    const bool comioManzana = destino == MANZANA;
    m_serpiente->avanzar(nuevaX, nuevaY, comioManzana);
    actualizarMapa();

    if (comioManzana) {
        m_progreso->registrarFruta(PUNTOS_MANZANA, m_serpiente->longitud());
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
    QMessageBox::information(this, "Fin de la partida", "La serpiente choco con su cuerpo.");
    reiniciar();
    m_temporizador->start(INTERVALO_MOVIMIENTO);
}

void JuegoView::ganarNivel() {
    m_terminado = true;
    m_temporizador->stop();
    QMessageBox::information(this, "Nivel completado", "Cumpliste las metas del nivel 1.");
    reiniciar();
    m_temporizador->start(INTERVALO_MOVIMIENTO);
}
