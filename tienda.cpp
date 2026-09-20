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
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFont>
#include "gestorusuarios.h"
#include "skins.h"
#include "dialogos.h"
#include <QMessageBox>

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
    , etiquetaMonedas(nullptr)
{
    setSceneRect(0, 0, 1254, 1254);
    construirInterfaz();
    connect(&GestorUsuarios::instancia(), &GestorUsuarios::perfilActualizado, this, &Tienda::actualizarTienda);
    connect(&GestorUsuarios::instancia(), &GestorUsuarios::pendientesCambiaron, this, &Tienda::actualizarTienda);
}

void Tienda::establecerUsuario(const QString &usuario)
{
    usuarioActual = usuario;
    actualizarTienda();
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
        "font-family: 'Fredoka'; font-size: 42px; font-weight: bold; }"
        );

    QGraphicsProxyWidget *proxyTitulo = addWidget(titulo);
    proxyTitulo->setPos(350, 145);
    proxyTitulo->setZValue(2);

    etiquetaMonedas = new QLabel;
    etiquetaMonedas->setFixedSize(360, 58);
    etiquetaMonedas->setAlignment(Qt::AlignCenter);
    etiquetaMonedas->setStyleSheet(
        "QLabel { background-color: rgba(8, 24, 8, 220); color: #ffe27a;"
        "border: 2px solid #d2a93b; border-radius: 10px; font-size: 24px; font-weight: bold; }"
        );
    auto *proxyMonedas = addWidget(etiquetaMonedas);
    proxyMonedas->setPos(462, 245);
    proxyMonedas->setZValue(2);

    const QList<QString> nombres = {"clasica", "gato", "dragon", "burro", "spiderman", "miles", "personaje", "thanos"};
    const QList<QString> etiquetas = {"Shrek", "Gato", "Dragón", "Burro", "Spider-Man", "Miles", "Farquaad", "Thanos"};
    const QList<int> precios = {0, 150, 250, 350, 500, 650, 850, 1400};
    const QList<QString> colores = {"#78c850", "#e58a32", "#d83232", "#626262", "#e52d35", "#4b4b78", "#d18b42", "#7048b8"};
    const QList<QString> descripciones = {"Shrek original", "Gato naranja", "Dragón rojo", "Burro", "Spider-Man", "Spider-Man negro", "Farquaad dorado", "Gemas del infinito"};

    for (int i = 0; i < nombres.size(); ++i) {
        QWidget *tarjeta = new QWidget;
        tarjeta->setFixedSize(235, 220);
        tarjeta->setAttribute(Qt::WA_StyledBackground, true);
        tarjeta->setStyleSheet(QString("QWidget { background: rgba(8,24,8,220); border: 3px solid %1; border-radius: 10px; } QLabel { border: none; color: white; font-family: 'Fredoka'; font-size: 20px; font-weight: bold; } QPushButton { background: %1; color: white; border: 2px solid white; border-radius: 7px; padding: 6px; font-family: 'Fredoka'; font-weight: bold; } QPushButton:disabled { background: #555555; color: #cccccc; }").arg(colores[i]));
        auto *layout = new QVBoxLayout(tarjeta);
        layout->setContentsMargins(8, 7, 8, 7);
        layout->setSpacing(3);
        auto *nombre = new QLabel(etiquetas[i].toUpper(), tarjeta);
        nombre->setAlignment(Qt::AlignCenter);
        nombre->setFixedHeight(27);
        auto *cabeza = new QLabel(tarjeta);
        cabeza->setFixedSize(70, 66);
        cabeza->setAlignment(Qt::AlignCenter);
        cabeza->setPixmap(Skins::cabeza(nombres[i], 62));
        cabeza->setToolTip("Vista previa de la cabeza");
        auto *descripcion = new QLabel(descripciones[i], tarjeta);
        descripcion->setAlignment(Qt::AlignCenter);
        descripcion->setWordWrap(true);
        descripcion->setFixedHeight(28);
        auto *boton = new QPushButton(tarjeta);
        boton->setFixedHeight(30);
        auto *estado = new QLabel(tarjeta);
        estado->setAlignment(Qt::AlignCenter);
        layout->addWidget(nombre);
        layout->addWidget(cabeza, 0, Qt::AlignCenter);
        layout->addWidget(descripcion);
        layout->addWidget(estado);
        layout->addWidget(boton);

        auto *proxy = addWidget(tarjeta);
        // El panel ocupa x=235..1019. Las tres tarjetas caben en 725 px
        // con 10 px entre ellas, dejando el mismo margen a ambos lados.
        proxy->setPos(265 + (i % 3) * 245, 330 + (i / 3) * 245);
        proxy->setZValue(2);

        tarjetas.append({nombres[i], precios[i], boton, estado, proxy});
        connect(boton, &QPushButton::clicked, this, [this, id = nombres[i], precio = precios[i]]() {
            if (GestorUsuarios::pendientes()) return;
            if (id == "thanos" && GestorUsuarios::obtenerNivelHistoria(usuarioActual) < 3) {
                Dialogos::mostrar(nullptr, QMessageBox::Information, "Skin bloqueada",
                                  "Completá los 3 niveles de HISTORIA para desbloquear a Thanos.");
                return;
            }
            if (GestorUsuarios::tieneSkin(usuarioActual, id)) {
                GestorUsuarios::equiparSkin(usuarioActual, id);
            } else {
                GestorUsuarios::comprarSkin(usuarioActual, id, precio);
            }
            actualizarTienda();
        });
    }

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

    actualizarTienda();
}

void Tienda::actualizarTienda()
{
    if (etiquetaMonedas == nullptr) return;
    etiquetaMonedas->setText(QString("MONEDAS: %1").arg(
        GestorUsuarios::obtenerMonedasUsuario(usuarioActual)));
    for (TarjetaSkin &tarjeta : tarjetas) actualizarTarjeta(tarjeta);
}

void Tienda::actualizarTarjeta(TarjetaSkin &tarjeta)
{
    const bool esThanos = tarjeta.id == "thanos";
    const bool desbloqueadaPorHistoria = GestorUsuarios::obtenerNivelHistoria(usuarioActual) >= 3;
    if (tarjeta.proxy != nullptr) {
        tarjeta.proxy->setVisible(!esThanos || desbloqueadaPorHistoria);
        if (tarjeta.id == "personaje") {
            tarjeta.proxy->setPos(desbloqueadaPorHistoria ? 265 : 510, 820);
        } else if (esThanos) {
            tarjeta.proxy->setPos(510, 820);
        }
    }
    const bool comprada = GestorUsuarios::tieneSkin(usuarioActual, tarjeta.id);
    const bool equipada = GestorUsuarios::obtenerSkinEquipada(usuarioActual) == tarjeta.id;
    const bool bloqueada = tarjeta.id == "thanos"
                        && GestorUsuarios::obtenerNivelHistoria(usuarioActual) < 3;
    tarjeta.estado->setText(bloqueada ? "BLOQUEADA: HISTORIA 3"
                       : equipada ? "EQUIPADA" : comprada ? "DESBLOQUEADA" : QString("%1 monedas").arg(tarjeta.precio));
    tarjeta.boton->setEnabled(!bloqueada && !GestorUsuarios::pendientes() && !usuarioActual.isEmpty() && !equipada && (comprada
                                            || GestorUsuarios::obtenerMonedasUsuario(usuarioActual) >= tarjeta.precio));
    tarjeta.boton->setText(bloqueada ? "BLOQUEADA" : GestorUsuarios::pendientes() ? "GUARDANDO..." : equipada ? "EQUIPADA" : comprada ? "EQUIPAR" : "COMPRAR");
}
