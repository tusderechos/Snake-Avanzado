#include "tienda.h"

#include <QBrush>
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QGraphicsRectItem>
#include <QIcon>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QPushButton>

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

Tienda::Tienda(QObject *parent)
    : QGraphicsScene(parent)
{
    setSceneRect(0, 0, 1254, 1254);
    construirInterfaz();
}

void Tienda::construirInterfaz()
{
    QPixmap fondoOriginal(":/imagenes/imagenes/Tienda.png");
    QPixmap fondoAjustado = fondoOriginal.scaled(
        1254, 1254, Qt::IgnoreAspectRatio, Qt::SmoothTransformation
        );

    QGraphicsPixmapItem *fondo = addPixmap(fondoAjustado);
    fondo->setPos(0, 0);
    fondo->setZValue(0);

    QGraphicsRectItem *velo = addRect(
        0, 0, 1254, 1254,
        QPen(Qt::NoPen), QBrush(QColor(0, 0, 0, 35))
        );
    velo->setZValue(1);

    // Este panel queda preparado para agregar las skins posteriormente.
    QGraphicsRectItem *panel = addRect(
        235, 110, 784, 1010,
        QPen(QColor(76, 190, 54, 220), 3),
        QBrush(QColor(32, 92, 17, 155))
        );
    panel->setZValue(1);

    QLabel *titulo = new QLabel("TIENDA");
    titulo->setFixedSize(554, 88);
    titulo->setAlignment(Qt::AlignCenter);
    titulo->setStyleSheet(
        "QLabel { background-color: rgba(8, 24, 8, 210); color: white;"
        "border: 2px solid #d2a93b; border-radius: 12px;"
        "font-family: 'Arial'; font-size: 42px; font-weight: bold; }"
        );

    QGraphicsProxyWidget *proxyTitulo = addWidget(titulo);
    proxyTitulo->setPos(350, 145);
    proxyTitulo->setZValue(2);

    QPushButton *botonVolver = new QPushButton;
    botonVolver->setFixedSize(94, 82);
    botonVolver->setCursor(Qt::PointingHandCursor);
    botonVolver->setIcon(crearIconoSalida());
    botonVolver->setIconSize(QSize(58, 58));
    botonVolver->setToolTip("Volver al menú principal");
    botonVolver->setAccessibleName("Volver al menú principal");
    botonVolver->setStyleSheet(
        "QPushButton { background-color: rgba(48, 132, 24, 235);"
        "border: 2px solid #68dd3e; border-radius: 10px; }"
        "QPushButton:hover { background-color: rgba(67, 165, 34, 245);"
        "border-color: #d2a93b; }"
        );

    QGraphicsProxyWidget *proxyVolver = addWidget(botonVolver);
    proxyVolver->setPos(885, 1000);
    proxyVolver->setZValue(2);

    connect(botonVolver, &QPushButton::clicked,
            this, &Tienda::volverSolicitado);
}
