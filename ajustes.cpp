#include "ajustes.h"
#include "gestorconfiguracion.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QGraphicsRectItem>
#include <QBrush>
#include <QIcon>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>

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

    // Marco y puerta abierta.
    pintor.drawRect(8, 8, 30, 48);
    QPolygonF puerta;
    puerta << QPointF(12, 12)
           << QPointF(33, 17)
           << QPointF(33, 49)
           << QPointF(12, 54);
    pintor.drawPolygon(puerta);
    pintor.drawEllipse(QPointF(27, 33), 1.8, 1.8);

    // Flecha de salida.
    pintor.drawLine(QPointF(35, 32), QPointF(57, 32));
    pintor.drawLine(QPointF(49, 24), QPointF(57, 32));
    pintor.drawLine(QPointF(57, 32), QPointF(49, 40));

    return QIcon(imagen);
}
}

Ajustes::Ajustes(QObject *parent)
    : QGraphicsScene(parent)
    , barraMusica(nullptr)
    , barraSonido(nullptr)
    , etiquetaVolumenMusica(nullptr)
    , etiquetaVolumenSonido(nullptr)
    , cargandoConfiguracion(false)
{
    setSceneRect(0, 0, 1254, 1254);
    construirInterfaz();
}

QLabel *Ajustes::crearEtiqueta(
    const QString &texto,
    qreal x,
    qreal y,
    int ancho,
    int alto,
    int tamanoLetra
    )
{
    QLabel *etiqueta = new QLabel(texto);
    etiqueta->setFixedSize(ancho, alto);
    etiqueta->setAlignment(Qt::AlignCenter);
    etiqueta->setStyleSheet(
        "QLabel {"
        "   background-color: rgba(8, 24, 8, 220);"
        "   color: white;"
        "   border: 2px solid rgba(99, 220, 55, 210);"
        "   border-radius: 12px;"
        "   font-family: 'Arial';"
        "   font-size: " + QString::number(tamanoLetra) + "px;"
        "   font-weight: bold;"
        "}"
        );

    QGraphicsProxyWidget *proxy = addWidget(etiqueta);
    proxy->setPos(x, y);
    proxy->setZValue(2);

    return etiqueta;
}

QPushButton *Ajustes::crearBoton(
    const QString &texto,
    qreal x,
    qreal y,
    int ancho,
    int alto
    )
{
    QPushButton *boton = new QPushButton(texto);
    boton->setFixedSize(ancho, alto);
    boton->setCursor(Qt::PointingHandCursor);
    boton->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(48, 132, 24, 235);"
        "   color: white;"
        "   border: 2px solid #68dd3e;"
        "   border-radius: 10px;"
        "   font-family: 'Arial';"
        "   font-size: 28px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(67, 165, 34, 245);"
        "   border-color: #d2a93b;"
        "}"
        "QPushButton:pressed {"
        "   background-color: rgba(35, 110, 10, 245);"
        "   border-color: white;"
        "}"
        );

    QGraphicsProxyWidget *proxy = addWidget(boton);
    proxy->setPos(x, y);
    proxy->setZValue(2);

    return boton;
}

QSlider *Ajustes::crearBarraVolumen(
    qreal x,
    qreal y,
    int valorInicial
    )
{
    QSlider *barra = new QSlider(Qt::Horizontal);
    barra->setFixedSize(570, 72);
    barra->setRange(0, 100);
    barra->setValue(valorInicial);
    barra->setCursor(Qt::PointingHandCursor);
    barra->setStyleSheet(
        "QSlider::groove:horizontal {"
        "   height: 20px;"
        "   background-color: rgba(20, 25, 14, 235);"
        "   border: 3px solid #8b6f26;"
        "   border-radius: 13px;"
        "}"
        "QSlider::sub-page:horizontal {"
        "   background-color: #4f991d;"
        "   border: 2px solid #d2a93b;"
        "   border-radius: 10px;"
        "}"
        "QSlider::add-page:horizontal {"
        "   background-color: rgba(15, 20, 12, 235);"
        "   border-radius: 10px;"
        "}"
        "QSlider::handle:horizontal {"
        "   width: 42px;"
        "   margin: -13px 0;"
        "   border: 4px solid #d2a93b;"
        "   border-radius: 21px;"
        "   background-color: #39ff14;"
        "}"
        "QSlider::handle:horizontal:hover {"
        "   background-color: #b6ff00;"
        "   border-color: #fff176;"
        "}"
        );

    QGraphicsProxyWidget *proxy = addWidget(barra);
    proxy->setPos(x, y);
    proxy->setZValue(2);

    return barra;
}

void Ajustes::crearIcono(
    const QString &simbolo,
    qreal x,
    qreal y
    )
{
    QLabel *icono = new QLabel(simbolo);
    icono->setFixedSize(64, 64);
    icono->setAlignment(Qt::AlignCenter);
    icono->setStyleSheet(
        "QLabel {"
        "   background-color: #2f7d15;"
        "   color: #ffe34f;"
        "   border: 3px solid #d2a93b;"
        "   border-radius: 32px;"
        "   font-size: 34px;"
        "   font-weight: bold;"
        "}"
        );

    QGraphicsProxyWidget *proxy = addWidget(icono);
    proxy->setPos(x, y);
    proxy->setZValue(3);
}

void Ajustes::construirInterfaz()
{
    QPixmap fondoOriginal(
        ":/imagenes/imagenes/Ajustes.png"
        );

    QPixmap fondoAjustado = fondoOriginal.scaled(
        1254,
        1254,
        Qt::IgnoreAspectRatio,
        Qt::SmoothTransformation
        );

    QGraphicsPixmapItem *fondo = addPixmap(fondoAjustado);
    fondo->setPos(0, 0);
    fondo->setZValue(0);

    QGraphicsRectItem *velo = addRect(
        0, 0, 1254, 1254,
        QPen(Qt::NoPen),
        QBrush(QColor(0, 0, 0, 60))
        );
    velo->setZValue(1);

    QGraphicsRectItem *panel = addRect(
        145, 175, 964, 780,
        QPen(QColor(181, 145, 48, 210), 3),
        QBrush(QColor(7, 17, 8, 218))
        );
    panel->setZValue(1);

    QLabel *titulo = crearEtiqueta(
        "CONFIGURACIÓN",
        350, 55, 554, 88, 40
        );
    titulo->setStyleSheet(
        "QLabel { background-color: rgba(8, 24, 8, 225); color: white;"
        "border: 2px solid #d2a93b; border-radius: 12px;"
        "font-family: 'Arial'; font-size: 40px; font-weight: bold; }"
        );

    QLabel *textoMusica = crearEtiqueta("MÚSICA", 255, 245, 210, 60, 27);
    QLabel *textoSonido = crearEtiqueta("SONIDO", 255, 430, 210, 60, 27);
    textoMusica->setStyleSheet(
        "QLabel { background: transparent; color: white; border: none;"
        "font-size: 27px; font-weight: bold; }"
        );
    textoSonido->setStyleSheet(textoMusica->styleSheet());

    barraMusica = crearBarraVolumen(
        410, 300, 75
        );

    barraSonido = crearBarraVolumen(
        410, 485, 75
        );

    crearIcono("♫", 190, 240);
    crearIcono("🔊", 190, 425);

    etiquetaVolumenMusica = crearEtiqueta("75%", 985, 307, 82, 58, 22);
    etiquetaVolumenSonido = crearEtiqueta("75%", 985, 492, 82, 58, 22);
    etiquetaVolumenMusica->setStyleSheet(
        "QLabel { background-color: rgba(46, 126, 24, 235); color: white;"
        "border: 2px solid #d2a93b; border-radius: 10px;"
        "font-size: 22px; font-weight: bold; }"
        );
    etiquetaVolumenSonido->setStyleSheet(etiquetaVolumenMusica->styleSheet());

    QPushButton *botonPerfil = crearBoton(
        "PERFIL",
        255, 675, 340, 82
        );

    QPushButton *botonControles = crearBoton(
        "CONTROLES",
        659, 675, 340, 82
        );

    QPushButton *botonVolver = crearBoton(
        "",
        1015, 985, 94, 82
        );
    botonVolver->setIcon(crearIconoSalida());
    botonVolver->setIconSize(QSize(58, 58));
    botonVolver->setToolTip("Volver al menú");
    botonVolver->setAccessibleName("Volver al menú");

    connect(
        botonPerfil,
        &QPushButton::clicked,
        this,
        &Ajustes::perfilSolicitado
        );

    connect(
        botonControles,
        &QPushButton::clicked,
        this,
        &Ajustes::controlesSolicitados
        );

    connect(
        botonVolver,
        &QPushButton::clicked,
        this,
        &Ajustes::volverSolicitado
        );

    connect(
        barraMusica,
        &QSlider::valueChanged,
        this,
        [this](int valor)
        {
            etiquetaVolumenMusica->setText(
                QString::number(valor) + "%"
                );
        }
        );

    connect(
        barraSonido,
        &QSlider::valueChanged,
        this,
        [this](int valor)
        {
            etiquetaVolumenSonido->setText(
                QString::number(valor) + "%"
                );
        }
        );

    connect(
        barraMusica,
        &QSlider::valueChanged,
        this,
        &Ajustes::guardarVolumenActual
        );

    connect(
        barraSonido,
        &QSlider::valueChanged,
        this,
        &Ajustes::guardarVolumenActual
        );
}

void Ajustes::establecerUsuario(const QString &usuario)
{
    usuarioActual = usuario;

    int volumenMusica = 75;
    int volumenSonido = 75;

    GestorConfiguracion::cargarVolumen(
        usuarioActual,
        volumenMusica,
        volumenSonido
        );

    cargandoConfiguracion = true;
    barraMusica->setValue(volumenMusica);
    barraSonido->setValue(volumenSonido);
    cargandoConfiguracion = false;
}

void Ajustes::guardarVolumenActual()
{
    if (cargandoConfiguracion || usuarioActual.isEmpty())
    {
        return;
    }

    GestorConfiguracion::guardarVolumen(
        usuarioActual,
        barraMusica->value(),
        barraSonido->value()
        );
}
