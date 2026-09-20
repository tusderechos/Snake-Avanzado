#include "menuprincipal.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QPixmap>
#include <QPushButton>

MenuPrincipal::MenuPrincipal(QObject *parent)
    : QGraphicsScene(parent)
{
    setSceneRect(0, 0, 1254, 1254);

    construirInterfaz();
}

QPushButton *MenuPrincipal::crearBoton(
    const QString &texto,
    qreal x,
    qreal y,
    qreal ancho,
    qreal alto
    )
{
    QPushButton *boton =
        new QPushButton(texto);

    boton->setFixedSize(
        static_cast<int>(ancho),
        static_cast<int>(alto)
        );

    boton->setCursor(
        Qt::PointingHandCursor
        );

    boton->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(55, 145, 20, 220);"
        "   color: white;"
        "   border: 6px solid #39ff14;"
        "   border-radius: 18px;"
        "   font-family: 'Fredoka';"
        "   font-size: 46px;"
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

    QGraphicsProxyWidget *proxy =
        addWidget(boton);

    proxy->setPos(x, y);
    proxy->setZValue(2);

    return boton;
}

void MenuPrincipal::construirInterfaz()
{
    // =====================================================
    // FONDO
    // =====================================================

    QPixmap fondoOriginal(
        ":/imagenes/imagenes/MenuP.webp"
        );

    QPixmap fondoAjustado =
        fondoOriginal.scaled(
            1254,
            1254,
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
            );

    QGraphicsPixmapItem *fondo =
        addPixmap(fondoAjustado);

    fondo->setPos(0, 0);
    fondo->setZValue(0);

    // =====================================================
    // BOTONES REALES SOBRE LA IMAGEN
    // =====================================================

    QPushButton *botonJugar =
        crearBoton(
            "JUGAR",
            310, 150, 635, 150
            );

    QPushButton *botonRanking =
        crearBoton(
            "RANKING",
            310, 355, 635, 150
            );

    QPushButton *botonTienda =
        crearBoton(
            "TIENDA",
            310, 560, 635, 150
            );

    QPushButton *botonConfiguracion =
        crearBoton(
            "CONFIGURACIÓN",
            310, 760, 635, 150
            );

    QPushButton *botonSalir =
        crearBoton(
            "SALIR",
            310, 965, 635, 150
            );

    m_botonManual = new QPushButton("?", nullptr);
    m_botonManual->setFixedSize(92, 92);
    m_botonManual->setCursor(Qt::PointingHandCursor);
    m_botonManual->setToolTip("Manual de usuario");
    m_botonManual->setStyleSheet(
        "QPushButton { background-color: rgba(48, 132, 24, 235); color: white;"
        " border: 4px solid #68dd3e; border-radius: 10px;"
        " font-family: 'Fredoka'; font-size: 48px; font-weight: bold; }"
        "QPushButton:hover { background-color: #4cae22; border-color: #f4d06f; }"
        );
    auto *proxyManual = addWidget(m_botonManual);
    proxyManual->setPos(1050, 1125);
    proxyManual->setZValue(3);

    // =====================================================
    // CONEXIONES
    // =====================================================

    connect(
        botonJugar,
        &QPushButton::clicked,
        this,
        [this]()
        {
            emit jugarSolicitado();
        }
        );

    connect(
        botonRanking,
        &QPushButton::clicked,
        this,
        [this]()
        {
            emit rankingSolicitado();
        }
        );

    connect(
        botonTienda,
        &QPushButton::clicked,
        this,
        [this]()
        {
            emit tiendaSolicitada();
        }
        );

    connect(
        botonConfiguracion,
        &QPushButton::clicked,
        this,
        [this]()
        {
            emit configuracionSolicitada();
        }
        );

    connect(
        botonSalir,
        &QPushButton::clicked,
        this,
        [this]()
        {
            emit salirSolicitado();
        }
        );

    connect(
        m_botonManual,
        &QPushButton::clicked,
        this,
        [this]()
        {
            emit manualSolicitado();
        }
        );

    // Mantener fijo el tamaño
    setSceneRect(0, 0, 1254, 1254);
}

void MenuPrincipal::establecerManualVisible(bool visible)
{
    if (m_botonManual != nullptr) {
        m_botonManual->setVisible(visible);
    }
}
