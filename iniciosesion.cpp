#include "iniciosesion.h"
#include "gestorusuarios.h"

#include <QApplication>
#include <QBrush>
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QGraphicsRectItem>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPen>
#include <QPixmap>
#include <QPushButton>

InicioSesion::InicioSesion(QObject *parent)
    : QGraphicsScene(parent)
    , campoUsuario(nullptr)
    , campoContrasena(nullptr)
    , mensajeEstado(nullptr)
    , botonIngresar(nullptr)
    , botonVolver(nullptr)
    , botonMostrarContrasena(nullptr)
{
    setSceneRect(0, 0, 1254, 1254);

    construirInterfaz();
}

void InicioSesion::construirInterfaz()
{
    // =====================================================
    // FONDO
    // =====================================================

    QPixmap fondoOriginal(
        ":/imagenes/imagenes/MenuI.png"
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

    // Panel único para separar el formulario del fondo sin ocultarlo.
    QGraphicsRectItem *panel = addRect(
        170, 80, 914, 880,
        QPen(QColor(181, 145, 48, 210), 3),
        QBrush(QColor(7, 17, 8, 190))
        );
    panel->setZValue(1);

    // =====================================================
    // ESTILOS
    // =====================================================

    QString estiloTitulo =
        "QLabel {"
        "   background-color: rgba(55, 145, 20, 225);"
        "   color: white;"
        "   border: 5px solid #39ff14;"
        "   border-radius: 12px;"
        "   font-size: 34px;"
        "   font-weight: bold;"
        "   padding-left: 15px;"
        "}";

    QString estiloCampo =
        "QLineEdit {"
        "   background-color: rgba(65, 145, 15, 210);"
        "   color: white;"
        "   border: 5px solid #39ff14;"
        "   border-radius: 12px;"
        "   font-size: 30px;"
        "   font-weight: bold;"
        "   padding: 8px 18px;"
        "   selection-background-color: #39ff14;"
        "   selection-color: #123500;"
        "}"
        "QLineEdit:focus {"
        "   background-color: rgba(75, 165, 20, 230);"
        "   border-color: #b6ff00;"
        "}";

    QString estiloBoton =
        "QPushButton {"
        "   background-color: rgba(55, 145, 20, 230);"
        "   color: white;"
        "   border: 5px solid #39ff14;"
        "   border-radius: 12px;"
        "   font-size: 30px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(85, 190, 25, 240);"
        "   border-color: #b6ff00;"
        "}"
        "QPushButton:pressed {"
        "   background-color: rgba(35, 110, 10, 240);"
        "}";

    // =====================================================
    // TÍTULO PRINCIPAL
    // =====================================================

    QLabel *tituloPrincipal =
        new QLabel("INICIAR SESIÓN");

    tituloPrincipal->setFixedSize(814, 90);
    tituloPrincipal->setAlignment(Qt::AlignCenter);
    tituloPrincipal->setStyleSheet(
        estiloTitulo
        );

    QGraphicsProxyWidget *proxyTitulo =
        addWidget(tituloPrincipal);

    proxyTitulo->setPos(220, 120);
    proxyTitulo->setZValue(2);

    // =====================================================
    // USUARIO
    // =====================================================

    QLabel *tituloUsuario =
        new QLabel("Usuario:");

    tituloUsuario->setFixedSize(380, 75);
    tituloUsuario->setStyleSheet(estiloTitulo);

    QGraphicsProxyWidget *proxyTituloUsuario =
        addWidget(tituloUsuario);

    proxyTituloUsuario->setPos(220, 250);
    proxyTituloUsuario->setZValue(2);

    campoUsuario = new QLineEdit;

    campoUsuario->setFixedSize(814, 90);
    campoUsuario->setMaxLength(15);
    campoUsuario->setPlaceholderText(
        "Escriba su nombre de usuario"
        );
    campoUsuario->setStyleSheet(estiloCampo);

    QGraphicsProxyWidget *proxyUsuario =
        addWidget(campoUsuario);

    proxyUsuario->setPos(220, 340);
    proxyUsuario->setZValue(2);

    // =====================================================
    // CONTRASEÑA
    // =====================================================

    QLabel *tituloContrasena =
        new QLabel("Contraseña:");

    tituloContrasena->setFixedSize(380, 75);
    tituloContrasena->setStyleSheet(
        estiloTitulo
        );

    QGraphicsProxyWidget *proxyTituloContrasena =
        addWidget(tituloContrasena);

    proxyTituloContrasena->setPos(220, 480);
    proxyTituloContrasena->setZValue(2);

    campoContrasena = new QLineEdit;

    campoContrasena->setFixedSize(560, 90);
    campoContrasena->setMaxLength(8);
    campoContrasena->setEchoMode(
        QLineEdit::Password
        );
    campoContrasena->setPlaceholderText(
        "Escriba su contraseña"
        );
    campoContrasena->setStyleSheet(estiloCampo);

    QGraphicsProxyWidget *proxyContrasena =
        addWidget(campoContrasena);

    proxyContrasena->setPos(220, 570);
    proxyContrasena->setZValue(2);

    botonMostrarContrasena =
        new QPushButton("MOSTRAR");

    botonMostrarContrasena->setFixedSize(234, 90);
    botonMostrarContrasena->setStyleSheet(estiloBoton);
    botonMostrarContrasena->setCursor(
        Qt::PointingHandCursor
        );

    QGraphicsProxyWidget *proxyMostrar =
        addWidget(botonMostrarContrasena);

    proxyMostrar->setPos(800, 570);
    proxyMostrar->setZValue(2);

    // =====================================================
    // MENSAJE DE ESTADO
    // =====================================================

    mensajeEstado = new QLabel(
        "Ingrese los datos de su cuenta"
        );

    mensajeEstado->setFixedSize(814, 70);
    mensajeEstado->setAlignment(Qt::AlignCenter);

    mensajeEstado->setStyleSheet(
        "QLabel {"
        "   background-color: rgba(0, 0, 0, 190);"
        "   color: #ffeb3b;"
        "   border: 3px solid #ffeb3b;"
        "   border-radius: 10px;"
        "   font-size: 22px;"
        "   font-weight: bold;"
        "}"
        );

    QGraphicsProxyWidget *proxyMensaje =
        addWidget(mensajeEstado);

    proxyMensaje->setPos(220, 700);
    proxyMensaje->setZValue(2);

    // =====================================================
    // BOTÓN VOLVER
    // =====================================================

    botonVolver = new QPushButton("Volver");

    botonVolver->setFixedSize(350, 90);
    botonVolver->setStyleSheet(estiloBoton);
    botonVolver->setCursor(
        Qt::PointingHandCursor
        );

    QGraphicsProxyWidget *proxyVolver =
        addWidget(botonVolver);

    proxyVolver->setPos(220, 810);
    proxyVolver->setZValue(2);

    // =====================================================
    // BOTÓN INGRESAR
    // =====================================================

    botonIngresar =
        new QPushButton("Iniciar Sesión");

    botonIngresar->setFixedSize(350, 90);
    botonIngresar->setStyleSheet(estiloBoton);
    botonIngresar->setCursor(
        Qt::PointingHandCursor
        );

    QGraphicsProxyWidget *proxyIngresar =
        addWidget(botonIngresar);

    proxyIngresar->setPos(684, 810);
    proxyIngresar->setZValue(2);

    // =====================================================
    // CONEXIONES
    // =====================================================

    connect(
        botonVolver,
        &QPushButton::clicked,
        this,
        [this]()
        {
            emit volverSolicitado();
        }
        );

    connect(
        botonIngresar,
        &QPushButton::clicked,
        this,
        &InicioSesion::intentarIniciarSesion
        );

    connect(
        botonMostrarContrasena,
        &QPushButton::clicked,
        this,
        &InicioSesion::alternarVisibilidadContrasena
        );

    // También permite iniciar sesión presionando Enter.
    connect(
        campoContrasena,
        &QLineEdit::returnPressed,
        this,
        &InicioSesion::intentarIniciarSesion
        );

    setSceneRect(0, 0, 1254, 1254);
}

void InicioSesion::alternarVisibilidadContrasena()
{
    bool estaOculta =
        campoContrasena->echoMode()
        == QLineEdit::Password;

    campoContrasena->setEchoMode(
        estaOculta
            ? QLineEdit::Normal
            : QLineEdit::Password
        );

    botonMostrarContrasena->setText(
        estaOculta ? "OCULTAR" : "MOSTRAR"
        );
}

void InicioSesion::intentarIniciarSesion()
{
    QString usuario =
        campoUsuario->text();

    QString contrasena =
        campoContrasena->text();

    if (usuario.isEmpty() || contrasena.isEmpty())
    {
        mensajeEstado->setText(
            "Debe completar el usuario y la contraseña"
            );

        QMessageBox::warning(
            QApplication::activeWindow(),
            "Datos incompletos",
            "Escriba el usuario y la contraseña."
            );

        return;
    }

    if (
        !GestorUsuarios::credencialesValidas(
            usuario,
            contrasena
            )
        )
    {
        mensajeEstado->setText(
            "Usuario o contraseña incorrectos"
            );

        QMessageBox::warning(
            QApplication::activeWindow(),
            "Acceso rechazado",
            "El usuario o la contraseña "
            "son incorrectos."
            );

        campoContrasena->clear();
        campoContrasena->setFocus();

        return;
    }

    QMessageBox::information(
        QApplication::activeWindow(),
        "Bienvenido",
        "Sesión iniciada correctamente. "
        "Bienvenido, " + usuario + "."
        );

    campoUsuario->clear();
    campoContrasena->clear();
    campoContrasena->setEchoMode(QLineEdit::Password);
    botonMostrarContrasena->setText("MOSTRAR");

    emit sesionIniciada(usuario);
}
