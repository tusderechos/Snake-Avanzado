#include "ajustes.h"
#include "audio.h"
#include "gestorconfiguracion.h"
#include "gestorusuarios.h"
#include <QTimer>

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
    temporizadorGuardado = new QTimer(this);
    temporizadorGuardado->setSingleShot(true);
    temporizadorGuardado->setInterval(400);
    connect(temporizadorGuardado, &QTimer::timeout, this, &Ajustes::confirmarCambios);
    connect(&GestorUsuarios::instancia(), &GestorUsuarios::errorGuardado, this,
        [this](const QString &) { establecerUsuario(usuarioActual); });
    connect(&GestorUsuarios::instancia(), &GestorUsuarios::perfilActualizado, this, [this]() {
        if (!temporizadorGuardado->isActive() && !barraMusica->isSliderDown()
            && !barraSonido->isSliderDown()) establecerUsuario(usuarioActual);
    });
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
        "   font-family: 'Fredoka';"
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
        "   font-family: 'Fredoka';"
        "   font-size: 46px;"
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
    int valorInicial,
    const QString &imagenPerilla,
    int ancho,
    int anchoPerilla
    )
{
    QSlider *barra = new QSlider(Qt::Horizontal);
    // El ancho incluye la mitad de la perilla a cada lado. Así, el centro
    // visual del handle recorre exactamente el canal oscuro de cada asset.
    barra->setFixedSize(ancho, 120);
    barra->setRange(0, 100);
    // El audio responde mientras se arrastra; el guardado permanece protegido
    // por el debounce de 400 ms.
    barra->setTracking(true);
    barra->setValue(valorInicial);
    barra->setCursor(Qt::PointingHandCursor);
    barra->setAttribute(Qt::WA_TranslucentBackground);
    barra->setAutoFillBackground(false);
    barra->setStyleSheet(
        "QSlider { background: transparent; }"
        "QSlider::groove:horizontal {"
        "   height: 72px; background: transparent; border: none;"
        "}"
        "QSlider::sub-page:horizontal, QSlider::add-page:horizontal {"
        "   height: 72px; background: transparent; border: none;"
        "}"
        "QSlider::handle:horizontal {"
        "   width: " + QString::number(anchoPerilla) + "px;"
        "   height: 112px; margin: -20px 0; border: none;"
        "   image: url(" + imagenPerilla + ");"
        "}"
        "QSlider::handle:horizontal:hover {"
        "   image: url(" + imagenPerilla + ");"
        "}"
        );

    QGraphicsProxyWidget *proxy = addWidget(barra);
    proxy->setPos(x, y);
    proxy->setZValue(3);

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

    QLabel *titulo = crearEtiqueta(
        "CONFIGURACIÓN",
        312, 8, 630, 150, 64
        );
    titulo->setStyleSheet(
        "QLabel { background-color: rgba(8, 24, 8, 225); color: white;"
        "border: 5px solid #39ff14; border-radius: 14px;"
        "font-family: 'Fredoka'; font-size: 64px; font-weight: bold; }"
        );

    QLabel *textoMusica = crearEtiqueta("MÚSICA", 75, 225, 375, 110, 44);
    QLabel *textoSonido = crearEtiqueta("SONIDO", 75, 410, 375, 110, 44);
    const QString estiloEtiquetaCanva =
        "QLabel { background-color: rgba(48, 132, 24, 235); color: white;"
        "border: 5px solid #39ff14; border-radius: 12px;"
        "font-family: 'Fredoka'; font-size: 44px; font-weight: bold; }";
    textoMusica->setStyleSheet(estiloEtiquetaCanva);
    textoSonido->setStyleSheet(estiloEtiquetaCanva);

    barraMusica = crearBarraVolumen(
        570, 241, 75, ":/imagenes/imagenes/ajustes_perilla_musica.png", 620, 125
        );

    barraSonido = crearBarraVolumen(
        563, 426, 75, ":/imagenes/imagenes/ajustes_perilla_sonido.png", 634, 138
        );

    auto agregarBarraDecorativa = [this](const QString &ruta, qreal x, qreal y,
                                        int ancho, int alto) {
        const QPixmap original(ruta);
        if (original.isNull()) return;
        auto *barra = addPixmap(original.scaled(ancho, alto, Qt::IgnoreAspectRatio,
                                                 Qt::SmoothTransformation));
        barra->setPos(x, y);
        barra->setZValue(2);
    };
    agregarBarraDecorativa(":/imagenes/imagenes/ajustes_barra_musica.png", 495, 205, 700, 177);
    // El archivo de sonido tiene más margen transparente a la izquierda y
    // menos píxeles visibles de ancho. Estas medidas igualan los bordes
    // visibles de ambas barras sin alterar sus imágenes.
    agregarBarraDecorativa(":/imagenes/imagenes/ajustes_barra_sonido.png", 473, 394, 717, 173);

    etiquetaVolumenMusica = crearEtiqueta("75%", 985, 307, 82, 58, 22);
    etiquetaVolumenSonido = crearEtiqueta("75%", 985, 492, 82, 58, 22);
    etiquetaVolumenMusica->setVisible(false);
    etiquetaVolumenSonido->setVisible(false);
    etiquetaVolumenMusica->setStyleSheet(
        "QLabel { background-color: rgba(46, 126, 24, 235); color: white;"
        "border: 2px solid #d2a93b; border-radius: 10px;"
        "font-size: 22px; font-weight: bold; }"
        );
    etiquetaVolumenSonido->setStyleSheet(etiquetaVolumenMusica->styleSheet());

    QPushButton *botonPerfil = crearBoton(
        "PERFIL",
        438, 668, 375, 110
        );

    QPushButton *botonControles = crearBoton(
        "CONTROLES",
        438, 850, 375, 110
        );

    QPushButton *botonVolver = crearBoton(
        "",
        1090, 1050, 105, 105
        );
    botonVolver->setIcon(crearIconoSalida());
    botonVolver->setIconSize(QSize(70, 70));
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
            AudioManager::instancia().establecerVolumenMusica(valor);
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
            AudioManager::instancia().establecerVolumenSonido(valor);
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
    temporizadorGuardado->stop();
    usuarioPendiente.clear();
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
    AudioManager::instancia().establecerVolumenMusica(volumenMusica);
    AudioManager::instancia().establecerVolumenSonido(volumenSonido);
    cargandoConfiguracion = false;
}

void Ajustes::guardarVolumenActual()
{
    if (cargandoConfiguracion || usuarioActual.isEmpty())
    {
        return;
    }

    usuarioPendiente = usuarioActual;
    temporizadorGuardado->start();
}

void Ajustes::confirmarCambios()
{
    temporizadorGuardado->stop();
    const QString propietario = usuarioPendiente;
    usuarioPendiente.clear();
    if (propietario.isEmpty() || propietario != GestorUsuarios::nombreActual()) return;
    GestorConfiguracion::guardarVolumen(
        propietario,
        barraMusica->value(),
        barraSonido->value()
        );
}
