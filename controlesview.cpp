#include "controlesview.h"
#include "gestorconfiguracion.h"

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

QGraphicsProxyWidget *agregarWidget(
    QGraphicsScene *escena,
    QWidget *widget,
    qreal x,
    qreal y
    )
{
    QGraphicsProxyWidget *proxy = escena->addWidget(widget);
    proxy->setPos(x, y);
    proxy->setZValue(2);
    return proxy;
}
}

ControlesView::ControlesView(QObject *parent)
    : QGraphicsScene(parent)
    , controlActual("FLECHAS")
    , botonWasd(nullptr)
    , botonFlechas(nullptr)
    , estadoWasd(nullptr)
    , estadoFlechas(nullptr)
{
    setSceneRect(0, 0, 1254, 1254);
    construirInterfaz();
}

QString ControlesView::estiloOpcion(bool activa) const
{
    if (activa)
    {
        return
            "QPushButton { background-color: rgba(48, 132, 24, 240);"
            "color: white; border: 3px solid #68dd3e; border-radius: 16px;"
            "font-family: 'Arial'; font-size: 38px; font-weight: bold; }"
            "QPushButton:hover { background-color: rgba(67, 165, 34, 245);"
            "border-color: #d2a93b; }";
    }

    return
        "QPushButton { background-color: rgba(15, 25, 16, 235);"
        "color: #aeb9ab; border: 3px solid #596257; border-radius: 16px;"
        "font-family: 'Arial'; font-size: 38px; font-weight: bold; }"
        "QPushButton:hover { background-color: rgba(28, 48, 27, 240);"
        "color: white; border-color: #8b6f26; }";
}

void ControlesView::construirInterfaz()
{
    QPixmap fondoOriginal(":/imagenes/imagenes/Ajustes.png");
    QPixmap fondoAjustado = fondoOriginal.scaled(
        1254, 1254, Qt::IgnoreAspectRatio, Qt::SmoothTransformation
        );

    QGraphicsPixmapItem *fondo = addPixmap(fondoAjustado);
    fondo->setPos(0, 0);
    fondo->setZValue(0);

    QGraphicsRectItem *velo = addRect(
        0, 0, 1254, 1254,
        QPen(Qt::NoPen), QBrush(QColor(0, 0, 0, 65))
        );
    velo->setZValue(1);

    QGraphicsRectItem *panel = addRect(
        155, 175, 944, 825,
        QPen(QColor(181, 145, 48, 210), 3),
        QBrush(QColor(7, 17, 8, 218))
        );
    panel->setZValue(1);

    QLabel *titulo = new QLabel("CONTROLES");
    titulo->setFixedSize(554, 88);
    titulo->setAlignment(Qt::AlignCenter);
    titulo->setStyleSheet(
        "QLabel { background-color: rgba(8, 24, 8, 225); color: white;"
        "border: 2px solid #d2a93b; border-radius: 12px;"
        "font-family: 'Arial'; font-size: 40px; font-weight: bold; }"
        );
    agregarWidget(this, titulo, 350, 55);

    QLabel *instruccion = new QLabel(
        "SELECCIONA CÓMO QUIERES CONTROLAR LA SERPIENTE"
        );
    instruccion->setFixedSize(820, 55);
    instruccion->setAlignment(Qt::AlignCenter);
    instruccion->setStyleSheet(
        "QLabel { background: transparent; color: #dce6d9; border: none;"
        "font-size: 20px; font-weight: bold; }"
        );
    agregarWidget(this, instruccion, 217, 220);

    QLabel *tituloWasd = new QLabel("WASD");
    QLabel *tituloFlechas = new QLabel("FLECHAS");
    for (QLabel *etiqueta : {tituloWasd, tituloFlechas})
    {
        etiqueta->setFixedSize(220, 72);
        etiqueta->setAlignment(Qt::AlignCenter);
        etiqueta->setStyleSheet(
            "QLabel { background: transparent; color: white; border: none;"
            "font-size: 28px; font-weight: bold; }"
            );
    }
    agregarWidget(this, tituloWasd, 210, 355);
    agregarWidget(this, tituloFlechas, 210, 600);

    botonWasd = new QPushButton("W     A     S     D");
    botonFlechas = new QPushButton("↑     ←     ↓     →");
    botonWasd->setFixedSize(540, 120);
    botonFlechas->setFixedSize(540, 120);
    botonWasd->setCursor(Qt::PointingHandCursor);
    botonFlechas->setCursor(Qt::PointingHandCursor);
    botonWasd->setAccessibleName("Usar controles WASD");
    botonFlechas->setAccessibleName("Usar teclas de dirección");
    agregarWidget(this, botonWasd, 445, 330);
    agregarWidget(this, botonFlechas, 445, 575);

    estadoWasd = new QLabel;
    estadoFlechas = new QLabel;
    for (QLabel *estado : {estadoWasd, estadoFlechas})
    {
        estado->setFixedSize(170, 52);
        estado->setAlignment(Qt::AlignCenter);
    }
    agregarWidget(this, estadoWasd, 815, 465);
    agregarWidget(this, estadoFlechas, 815, 710);

    QPushButton *botonVolver = new QPushButton;
    botonVolver->setFixedSize(94, 82);
    botonVolver->setCursor(Qt::PointingHandCursor);
    botonVolver->setIcon(crearIconoSalida());
    botonVolver->setIconSize(QSize(58, 58));
    botonVolver->setToolTip("Volver a configuración");
    botonVolver->setAccessibleName("Volver a configuración");
    botonVolver->setStyleSheet(
        "QPushButton { background-color: rgba(48, 132, 24, 235);"
        "border: 2px solid #68dd3e; border-radius: 10px; }"
        "QPushButton:hover { background-color: rgba(67, 165, 34, 245);"
        "border-color: #d2a93b; }"
        );
    agregarWidget(this, botonVolver, 960, 855);

    connect(botonWasd, &QPushButton::clicked,
            this, &ControlesView::seleccionarWasd);
    connect(botonFlechas, &QPushButton::clicked,
            this, &ControlesView::seleccionarFlechas);
    connect(botonVolver, &QPushButton::clicked,
            this, &ControlesView::volverSolicitado);

    actualizarSeleccion("FLECHAS");
}

void ControlesView::establecerUsuario(const QString &usuario)
{
    usuarioActual = usuario;
    actualizarSeleccion(
        GestorConfiguracion::cargarControl(usuarioActual)
        );
}

void ControlesView::actualizarSeleccion(const QString &control)
{
    controlActual = control == "WASD" ? "WASD" : "FLECHAS";
    bool usaWasd = controlActual == "WASD";

    botonWasd->setStyleSheet(estiloOpcion(usaWasd));
    botonFlechas->setStyleSheet(estiloOpcion(!usaWasd));

    const QString estiloActivo =
        "QLabel { background-color: #2f8a1a; color: white;"
        "border: 2px solid #68dd3e; border-radius: 10px;"
        "font-size: 18px; font-weight: bold; }";
    const QString estiloInactivo =
        "QLabel { background-color: rgba(18, 25, 18, 235); color: #929c90;"
        "border: 2px solid #596257; border-radius: 10px;"
        "font-size: 18px; font-weight: bold; }";

    estadoWasd->setText(usaWasd ? "ACTIVO" : "INACTIVO");
    estadoFlechas->setText(usaWasd ? "INACTIVO" : "ACTIVO");
    estadoWasd->setStyleSheet(usaWasd ? estiloActivo : estiloInactivo);
    estadoFlechas->setStyleSheet(usaWasd ? estiloInactivo : estiloActivo);
}

void ControlesView::seleccionarWasd()
{
    actualizarSeleccion("WASD");

    if (!usuarioActual.isEmpty())
    {
        GestorConfiguracion::guardarControl(usuarioActual, "WASD");
    }
}

void ControlesView::seleccionarFlechas()
{
    actualizarSeleccion("FLECHAS");

    if (!usuarioActual.isEmpty())
    {
        GestorConfiguracion::guardarControl(usuarioActual, "FLECHAS");
    }
}
