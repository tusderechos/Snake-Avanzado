#include "menujuego.h"

#include <QFont>
#include <QLabel>
#include <QFontDatabase>
#include <QIcon>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <utility>

namespace
{
QIcon crearIconoSalida()
{
    QPixmap imagen(64, 64);
    imagen.fill(Qt::transparent);

    QPainter pintor(&imagen);
    pintor.setRenderHint(QPainter::Antialiasing);
    pintor.setPen(QPen(Qt::white, 4, Qt::SolidLine,
                       Qt::RoundCap, Qt::RoundJoin));

    pintor.drawRect(8, 8, 30, 48);
    QPolygonF puerta;
    puerta << QPointF(12, 12)
           << QPointF(33, 17)
           << QPointF(33, 49)
           << QPointF(12, 54);
    pintor.drawPolygon(puerta);
    pintor.drawEllipse(QPointF(27, 33), 1.8, 1.8);
    pintor.drawLine(QPointF(35, 32), QPointF(57, 32));
    pintor.drawLine(QPointF(49, 24), QPointF(57, 32));
    pintor.drawLine(QPointF(57, 32), QPointF(49, 40));

    return QIcon(imagen);
}
}

MenuJuego::MenuJuego(std::function<void(ModoJuego)> alSeleccionar,
                     QWidget *parent)
    : QWidget(parent), m_alSeleccionar(std::move(alSeleccionar)) {
    setWindowTitle("Snake - Menú de Juego");
    setFixedSize(500, 560);
    setStyleSheet(
        "QWidget { background: #0d121a; color: #ebf0f5; }"
        "QPushButton { color: white; font-family: 'Arial Black';"
        " font-weight: bold; }");
    QFontDatabase::addApplicationFont(":/assets/Fredoka-Variable.ttf");
    setFont(QFont("Fredoka", 14));

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(55, 35, 55, 28);
    layout->setSpacing(12);

    auto *titulo = new QLabel("SNAKE AVANZADO", this);
    titulo->setAlignment(Qt::AlignCenter);
    titulo->setFont(QFont("Fredoka", 26, QFont::Bold));
    layout->addWidget(titulo);

    auto *subtitulo = new QLabel("MENÚ DE JUEGO", this);
    subtitulo->setAlignment(Qt::AlignCenter);
    subtitulo->setFont(QFont("Fredoka", 18, QFont::DemiBold));
    subtitulo->setStyleSheet("color: #7bdc66;");
    layout->addWidget(subtitulo);
    layout->addSpacing(20);

    const struct Opcion { const char *texto; ModoJuego modo; } opciones[] = {
        {"TUTORIAL", ModoJuego::Tutorial},
        {"HISTORIA", ModoJuego::Normal},
        {"LIBRE", ModoJuego::Libre},
        {"ALEATORIO", ModoJuego::Aleatorio}
    };

    for (const Opcion &opcion : opciones) {
        auto *boton = new QPushButton(opcion.texto, this);
        boton->setFixedHeight(52);
        boton->setCursor(Qt::PointingHandCursor);
        boton->setStyleSheet(
            "QPushButton {"
            "   background-color: rgba(55, 145, 20, 220);"
            "   color: white;"
            "   border: 3px solid #39ff14;"
            "   border-radius: 12px;"
            "   font-size: 22px;"
            "   font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "   background-color: rgba(85, 190, 25, 235);"
            "   border-color: #b6ff00;"
            "}"
            "QPushButton:pressed {"
            "   background-color: rgba(35, 110, 10, 240);"
            "   border-color: white;"
            "}"
            );
        layout->addWidget(boton);
        connect(boton, &QPushButton::clicked, this, [this, modo = opcion.modo]() {
            if (m_alSeleccionar) {
                m_alSeleccionar(modo);
            }
        });
    }

    auto *nota = new QLabel("Elegí un modo para comenzar", this);
    nota->setAlignment(Qt::AlignCenter);
    nota->setStyleSheet("color: #9aa9b8; margin-top: 18px;");

    auto *filaInferior = new QHBoxLayout;
    filaInferior->setContentsMargins(0, 8, 0, 0);
    filaInferior->addWidget(nota, 1);

    auto *botonVolver = new QPushButton(this);
    botonVolver->setFixedSize(64, 56);
    botonVolver->setCursor(Qt::PointingHandCursor);
    botonVolver->setIcon(crearIconoSalida());
    botonVolver->setIconSize(QSize(42, 42));
    botonVolver->setToolTip("Volver al menú principal");
    botonVolver->setAccessibleName("Volver al menú principal");
    botonVolver->setStyleSheet(
        "QPushButton { background-color: rgba(48, 132, 24, 235);"
        "border: 2px solid #68dd3e; border-radius: 10px; }"
        "QPushButton:hover { background-color: rgba(67, 165, 34, 245);"
        "border-color: #d2a93b; }"
        );

    filaInferior->addWidget(botonVolver);
    layout->addLayout(filaInferior);

    connect(
        botonVolver,
        &QPushButton::clicked,
        this,
        &QWidget::close
        );
}
