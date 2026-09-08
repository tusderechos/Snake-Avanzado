#include "perfil.h"
#include "gestorusuarios.h"
#include "validarcuenta.h"

#include <QApplication>
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QGraphicsRectItem>
#include <QBrush>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPen>
#include <QPixmap>
#include <QPushButton>

Perfil::Perfil(QObject *parent)
    : QGraphicsScene(parent)
    , etiquetaInicial(nullptr)
    , etiquetaUsuario(nullptr)
    , etiquetaPuntos(nullptr)
    , mensajeEstado(nullptr)
    , campoContrasenaActual(nullptr)
    , campoContrasenaNueva(nullptr)
    , campoConfirmacion(nullptr)
    , botonMostrarContrasenas(nullptr)
{
    setSceneRect(0, 0, 1254, 1254);
    construirInterfaz();
}

QLabel *Perfil::crearEtiqueta(
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
        "   background-color: rgba(8, 24, 8, 210);"
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

void Perfil::construirInterfaz()
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

    // Capa oscura para que el contenido sea legible sin ocultar el fondo.
    QGraphicsRectItem *velo = addRect(
        0, 0, 1254, 1254,
        QPen(Qt::NoPen),
        QBrush(QColor(0, 0, 0, 65))
        );
    velo->setZValue(1);

    QGraphicsRectItem *panel = addRect(
        175, 145, 904, 930,
        QPen(QColor(181, 145, 48, 210), 3),
        QBrush(QColor(7, 17, 8, 218))
        );
    panel->setZValue(1);

    QString estiloCampo =
        "QLineEdit {"
        "   background-color: rgba(18, 38, 18, 240);"
        "   color: white;"
        "   border: 2px solid #5fcf37;"
        "   border-radius: 10px;"
        "   font-size: 23px;"
        "   padding: 6px 14px;"
        "}"
        "QLineEdit:focus {"
        "   background-color: rgba(29, 61, 28, 245);"
        "   border-color: #d2a93b;"
        "}";

    QString estiloBoton =
        "QPushButton {"
        "   background-color: rgba(48, 132, 24, 235);"
        "   color: white;"
        "   border: 2px solid #68dd3e;"
        "   border-radius: 10px;"
        "   font-family: 'Arial';"
        "   font-size: 22px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(67, 165, 34, 245);"
        "   border-color: #d2a93b;"
        "}"
        "QPushButton:pressed {"
        "   background-color: rgba(35, 110, 10, 245);"
        "   border-color: white;"
        "}";

    QLabel *titulo = crearEtiqueta("PERFIL", 390, 45, 474, 78, 38);
    titulo->setStyleSheet(
        "QLabel {"
        "   background-color: rgba(8, 24, 8, 225);"
        "   color: white;"
        "   border: 2px solid #d2a93b;"
        "   border-radius: 12px;"
        "   font-family: 'Arial';"
        "   font-size: 38px;"
        "   font-weight: bold;"
        "}"
        );

    etiquetaInicial = crearEtiqueta(
        "?", 235, 205, 155, 155, 64
        );

    etiquetaInicial->setStyleSheet(
        "QLabel {"
        "   background-color: rgba(46, 126, 24, 235);"
        "   color: white;"
        "   border: 4px solid #d2a93b;"
        "   border-radius: 76px;"
        "   font-family: 'Arial';"
        "   font-size: 64px;"
        "   font-weight: bold;"
        "}"
        );

    QLabel *textoUsuario = crearEtiqueta("USUARIO", 430, 205, 170, 45, 18);
    textoUsuario->setStyleSheet(
        "QLabel { background: transparent; color: #b7c5b3; border: none;"
        "font-size: 18px; font-weight: bold; }"
        );
    etiquetaUsuario = crearEtiqueta(
        "---", 430, 248, 555, 58, 29
        );

    QLabel *textoPuntos = crearEtiqueta("PUNTOS", 430, 315, 170, 40, 18);
    textoPuntos->setStyleSheet(
        "QLabel { background: transparent; color: #b7c5b3; border: none;"
        "font-size: 18px; font-weight: bold; }"
        );
    etiquetaPuntos = crearEtiqueta(
        "0", 600, 310, 200, 52, 27
        );

    QLabel *seccion = crearEtiqueta(
        "CAMBIAR CONTRASEÑA", 235, 410, 500, 58, 27
        );
    seccion->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    seccion->setStyleSheet(
        "QLabel { background: transparent; color: white; border: none;"
        "border-bottom: 2px solid #d2a93b; font-size: 27px;"
        "font-weight: bold; padding-left: 4px; }"
        );

    const QString estiloDescripcion =
        "QLabel { background: transparent; color: #c8d2c5; border: none;"
        "font-size: 18px; font-weight: bold; }";

    QLabel *textoActual = crearEtiqueta("Contraseña actual", 265, 490, 300, 35, 18);
    QLabel *textoNueva = crearEtiqueta("Nueva contraseña", 265, 620, 300, 35, 18);
    QLabel *textoConfirmar = crearEtiqueta("Confirmar contraseña", 265, 750, 330, 35, 18);
    textoActual->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    textoNueva->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    textoConfirmar->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    textoActual->setStyleSheet(estiloDescripcion);
    textoNueva->setStyleSheet(estiloDescripcion);
    textoConfirmar->setStyleSheet(estiloDescripcion);

    campoContrasenaActual = new QLineEdit;
    campoContrasenaActual->setFixedSize(725, 68);
    campoContrasenaActual->setMaxLength(8);
    campoContrasenaActual->setEchoMode(QLineEdit::Password);
    campoContrasenaActual->setPlaceholderText("Contraseña actual");
    campoContrasenaActual->setStyleSheet(estiloCampo);

    QGraphicsProxyWidget *proxyActual =
        addWidget(campoContrasenaActual);

    proxyActual->setPos(265, 525);
    proxyActual->setZValue(2);

    campoContrasenaNueva = new QLineEdit;
    campoContrasenaNueva->setFixedSize(725, 68);
    campoContrasenaNueva->setMaxLength(8);
    campoContrasenaNueva->setEchoMode(QLineEdit::Password);
    campoContrasenaNueva->setPlaceholderText("Nueva contraseña");
    campoContrasenaNueva->setStyleSheet(estiloCampo);

    QGraphicsProxyWidget *proxyNueva =
        addWidget(campoContrasenaNueva);

    proxyNueva->setPos(265, 655);
    proxyNueva->setZValue(2);

    campoConfirmacion = new QLineEdit;
    campoConfirmacion->setFixedSize(725, 68);
    campoConfirmacion->setMaxLength(8);
    campoConfirmacion->setEchoMode(QLineEdit::Password);
    campoConfirmacion->setPlaceholderText("Repita la contraseña nueva");
    campoConfirmacion->setStyleSheet(estiloCampo);

    QGraphicsProxyWidget *proxyConfirmacion =
        addWidget(campoConfirmacion);

    proxyConfirmacion->setPos(265, 785);
    proxyConfirmacion->setZValue(2);

    botonMostrarContrasenas =
        new QPushButton("MOSTRAR");

    botonMostrarContrasenas->setFixedSize(190, 52);
    botonMostrarContrasenas->setCursor(Qt::PointingHandCursor);
    botonMostrarContrasenas->setStyleSheet(estiloBoton);

    QGraphicsProxyWidget *proxyMostrar =
        addWidget(botonMostrarContrasenas);

    proxyMostrar->setPos(800, 414);
    proxyMostrar->setZValue(2);

    QPushButton *botonGuardar =
        new QPushButton("GUARDAR CAMBIOS");

    botonGuardar->setFixedSize(420, 72);
    botonGuardar->setCursor(Qt::PointingHandCursor);
    botonGuardar->setStyleSheet(estiloBoton);

    QGraphicsProxyWidget *proxyGuardar =
        addWidget(botonGuardar);

    proxyGuardar->setPos(570, 965);
    proxyGuardar->setZValue(2);

    mensajeEstado = new QLabel(
        "Escriba los datos para cambiar la contraseña"
        );

    mensajeEstado->setFixedSize(725, 52);
    mensajeEstado->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mensajeEstado->setStyleSheet(
        "QLabel {"
        "   background-color: transparent;"
        "   color: #e7c85b;"
        "   border: none;"
        "   font-size: 17px;"
        "   padding-left: 4px;"
        "}"
        );

    QGraphicsProxyWidget *proxyMensaje =
        addWidget(mensajeEstado);

    proxyMensaje->setPos(265, 870);
    proxyMensaje->setZValue(2);

    QPushButton *botonVolver =
        new QPushButton("VOLVER");

    botonVolver->setFixedSize(250, 72);
    botonVolver->setCursor(Qt::PointingHandCursor);
    botonVolver->setStyleSheet(estiloBoton);

    QGraphicsProxyWidget *proxyVolver =
        addWidget(botonVolver);

    proxyVolver->setPos(265, 965);
    proxyVolver->setZValue(2);

    connect(
        botonMostrarContrasenas,
        &QPushButton::clicked,
        this,
        &Perfil::alternarVisibilidadContrasenas
        );

    connect(
        botonGuardar,
        &QPushButton::clicked,
        this,
        &Perfil::intentarCambiarContrasena
        );

    connect(
        botonVolver,
        &QPushButton::clicked,
        this,
        &Perfil::volverSolicitado
        );
}

void Perfil::establecerUsuario(
    const QString &usuario
    )
{
    usuarioActual = usuario;

    QString inicial = "?";

    if (!usuarioActual.isEmpty())
    {
        inicial = usuarioActual.left(1).toUpper();
    }

    etiquetaInicial->setText(inicial);
    etiquetaUsuario->setText(usuarioActual);
    etiquetaPuntos->setText(
        QString::number(
            GestorUsuarios::obtenerPuntosUsuario(usuarioActual)
            )
        );

    campoContrasenaActual->clear();
    campoContrasenaNueva->clear();
    campoConfirmacion->clear();

    campoContrasenaActual->setEchoMode(QLineEdit::Password);
    campoContrasenaNueva->setEchoMode(QLineEdit::Password);
    campoConfirmacion->setEchoMode(QLineEdit::Password);
    botonMostrarContrasenas->setText("MOSTRAR");

    mensajeEstado->setText(
        "Escriba los datos para cambiar la contraseña"
        );
}

void Perfil::alternarVisibilidadContrasenas()
{
    bool estanOcultas =
        campoContrasenaActual->echoMode()
        == QLineEdit::Password;

    QLineEdit::EchoMode modoNuevo =
        estanOcultas
            ? QLineEdit::Normal
            : QLineEdit::Password;

    campoContrasenaActual->setEchoMode(modoNuevo);
    campoContrasenaNueva->setEchoMode(modoNuevo);
    campoConfirmacion->setEchoMode(modoNuevo);

    botonMostrarContrasenas->setText(
        estanOcultas ? "OCULTAR" : "MOSTRAR"
        );
}

void Perfil::intentarCambiarContrasena()
{
    QString contrasenaActual =
        campoContrasenaActual->text();

    QString contrasenaNueva =
        campoContrasenaNueva->text();

    QString confirmacion =
        campoConfirmacion->text();

    if (contrasenaActual.isEmpty()
        || contrasenaNueva.isEmpty()
        || confirmacion.isEmpty())
    {
        mensajeEstado->setText(
            "Debe completar los tres campos"
            );
        return;
    }

    if (!GestorUsuarios::credencialesValidas(
            usuarioActual,
            contrasenaActual
            ))
    {
        mensajeEstado->setText(
            "La contraseña actual es incorrecta"
            );
        campoContrasenaActual->selectAll();
        campoContrasenaActual->setFocus();
        return;
    }

    if (!ValidarCuenta::contrasenaValida(
            contrasenaNueva
            ))
    {
        mensajeEstado->setText(
            "La contraseña nueva no cumple los requisitos"
            );

        QMessageBox::warning(
            QApplication::activeWindow(),
            "Contraseña inválida",
            "Use entre 5 y 8 caracteres, una mayúscula, "
            "una minúscula, un número y un símbolo."
            );
        return;
    }

    if (!ValidarCuenta::contrasenasCoinciden(
            contrasenaNueva,
            confirmacion
            ))
    {
        mensajeEstado->setText(
            "Las contraseñas nuevas no coinciden"
            );
        campoConfirmacion->selectAll();
        campoConfirmacion->setFocus();
        return;
    }

    if (!GestorUsuarios::cambiarContrasena(
            usuarioActual,
            contrasenaNueva
            ))
    {
        mensajeEstado->setText(
            "No se pudo actualizar el archivo de usuarios"
            );
        return;
    }

    campoContrasenaActual->clear();
    campoContrasenaNueva->clear();
    campoConfirmacion->clear();
    mensajeEstado->setText(
        "Contraseña actualizada correctamente"
        );

    QMessageBox::information(
        QApplication::activeWindow(),
        "Cambio realizado",
        "La contraseña fue actualizada correctamente."
        );
}
