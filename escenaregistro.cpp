#include "escenaregistro.h"
#include "gestorusuarios.h"
#include "validarcuenta.h"

#include <QApplication>
#include <QBrush>
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QGraphicsRectItem>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPixmap>
#include <QPen>
#include <QPushButton>
#include <QWidget>

EscenaRegistro::EscenaRegistro(QObject *parent)
    : QGraphicsScene(parent)
    , campoUsuario(nullptr)
    , campoContrasena(nullptr)
    , campoConfirmacion(nullptr)
    , requisitoLongitud(nullptr)
    , requisitoMayuscula(nullptr)
    , requisitoMinuscula(nullptr)
    , requisitoNumero(nullptr)
    , requisitoEspecial(nullptr)
    , requisitoCoincidencia(nullptr)
    , mensajeEstado(nullptr)
    , botonCrearCuenta(nullptr)
    , botonVolver(nullptr)
    , botonMostrarContrasenas(nullptr)
{
    // La escena utiliza las dimensiones originales del fondo.
    setSceneRect(0, 0, 1254, 1254);

    construirInterfaz();
    actualizarValidaciones();
}

void EscenaRegistro::construirInterfaz()
{
    // =====================================================
    // FONDO
    // =====================================================

    // Cargar el fondo original
    QPixmap fondoOriginal(":/imagenes/imagenes/MenuI.png");

    // Ajustarlo exactamente al tamaño de la escena
    QPixmap fondoAjustado = fondoOriginal.scaled(1254,
                                                 1254,
                                                 Qt::IgnoreAspectRatio,
                                                 Qt::SmoothTransformation);

    // Agregar el fondo ya ajustado
    QGraphicsPixmapItem *imagenFondo = addPixmap(fondoAjustado);

    imagenFondo->setPos(0, 0);
    imagenFondo->setZValue(0);

    QGraphicsRectItem *velo = addRect(
        0, 0, 1254, 1254,
        QPen(Qt::NoPen),
        QBrush(QColor(0, 0, 0, 70))
        );
    velo->setZValue(1);

    QGraphicsRectItem *panel = addRect(
        135, 155, 984, 925,
        QPen(QColor(181, 145, 48, 210), 3),
        QBrush(QColor(7, 17, 8, 220))
        );
    panel->setZValue(1);

    // =====================================================
    // ESTILOS
    // =====================================================

    QString estiloTitulo = "QLabel {"
                           "   background-color: transparent;"
                           "   color: #dce6d9;"
                           "   border: none;"
                           "   font-size: 19px;"
                           "   font-weight: bold;"
                           "}";

    QString estiloCampo = "QLineEdit {"
                          "   background-color: rgba(18, 38, 18, 240);"
                          "   color: white;"
                          "   border: 2px solid #5fcf37;"
                          "   border-radius: 10px;"
                          "   font-size: 22px;"
                          "   padding: 6px 14px;"
                          "   selection-background-color: #5fcf37;"
                          "   selection-color: #123500;"
                          "}"
                          "QLineEdit:focus {"
                          "   background-color: rgba(29, 61, 28, 245);"
                          "   border-color: #d2a93b;"
                          "}";

    QString estiloBoton = "QPushButton {"
                          "   background-color: rgba(48, 132, 24, 235);"
                          "   color: white;"
                          "   border: 2px solid #68dd3e;"
                          "   border-radius: 10px;"
                          "   font-size: 22px;"
                          "   font-weight: bold;"
                          "}"
                          "QPushButton:hover {"
                          "   background-color: rgba(67, 165, 34, 245);"
                          "   border-color: #d2a93b;"
                          "}"
                          "QPushButton:pressed {"
                          "   background-color: rgba(35, 110, 10, 240);"
                          "}"
                          "QPushButton:disabled {"
                          "   background-color: rgba(70, 70, 70, 190);"
                          "   color: #b0b0b0;"
                          "   border-color: #596257;"
                          "}";

    QLabel *tituloPrincipal = new QLabel("CREAR CUENTA");
    tituloPrincipal->setFixedSize(554, 88);
    tituloPrincipal->setAlignment(Qt::AlignCenter);
    tituloPrincipal->setStyleSheet(
        "QLabel { background-color: rgba(8, 24, 8, 225); color: white;"
        "border: 2px solid #d2a93b; border-radius: 12px;"
        "font-family: 'Arial'; font-size: 40px; font-weight: bold; }"
        );
    QGraphicsProxyWidget *proxyTituloPrincipal = addWidget(tituloPrincipal);
    proxyTituloPrincipal->setPos(350, 55);
    proxyTituloPrincipal->setZValue(2);

    // =====================================================
    // CAMPO USUARIO
    // =====================================================

    crearTitulo("USUARIO", 205, 225, 300, estiloTitulo);

    campoUsuario = new QLineEdit;
    campoUsuario->setFixedSize(520, 68);
    campoUsuario->setMaxLength(15);
    campoUsuario->setPlaceholderText("Entre 3 y 15 caracteres");
    campoUsuario->setStyleSheet(estiloCampo);

    QGraphicsProxyWidget *proxyUsuario = addWidget(campoUsuario);

    proxyUsuario->setPos(205, 270);
    proxyUsuario->setZValue(2);

    // =====================================================
    // CAMPO CONTRASEÑA
    // =====================================================

    crearTitulo("CONTRASEÑA", 205, 370, 300, estiloTitulo);

    campoContrasena = new QLineEdit;
    campoContrasena->setFixedSize(520, 68);
    campoContrasena->setMaxLength(8);
    campoContrasena->setEchoMode(QLineEdit::Password);
    campoContrasena->setPlaceholderText("Entre 5 y 8 caracteres");
    campoContrasena->setStyleSheet(estiloCampo);

    QGraphicsProxyWidget *proxyContrasena = addWidget(campoContrasena);

    proxyContrasena->setPos(205, 415);
    proxyContrasena->setZValue(2);

    // =====================================================
    // CAMPO CONFIRMACIÓN
    // =====================================================

    crearTitulo("CONFIRMAR CONTRASEÑA", 205, 515, 360, estiloTitulo);

    campoConfirmacion = new QLineEdit;
    campoConfirmacion->setFixedSize(520, 68);
    campoConfirmacion->setMaxLength(8);
    campoConfirmacion->setEchoMode(QLineEdit::Password);
    campoConfirmacion->setPlaceholderText("Escriba nuevamente la contraseña");
    campoConfirmacion->setStyleSheet(estiloCampo);

    QGraphicsProxyWidget *proxyConfirmacion = addWidget(campoConfirmacion);

    proxyConfirmacion->setPos(205, 560);
    proxyConfirmacion->setZValue(2);

    botonMostrarContrasenas =
        new QPushButton("MOSTRAR");

    botonMostrarContrasenas->setFixedSize(185, 44);
    botonMostrarContrasenas->setStyleSheet(estiloBoton);
    botonMostrarContrasenas->setCursor(
        Qt::PointingHandCursor
        );

    QGraphicsProxyWidget *proxyMostrar =
        addWidget(botonMostrarContrasenas);

    proxyMostrar->setPos(540, 365);
    proxyMostrar->setZValue(2);

    // =====================================================
    // PANEL DE REQUISITOS
    // =====================================================

    QWidget *panelRequisitos = new QWidget;
    panelRequisitos->setFixedSize(320, 405);
    panelRequisitos->setStyleSheet("QWidget {"
                                   "   background-color: rgba(14, 30, 14, 235);"
                                   "   border: 2px solid #5fcf37;"
                                   "   border-radius: 12px;"
                                   "}"
                                   "QLabel {"
                                   "   background-color: transparent;"
                                   "   border: none;"
                                   "   color: white;"
                                   "   font-size: 17px;"
                                   "   font-weight: bold;"
                                   "}");

    QLabel *tituloRequisitos = new QLabel("REQUISITOS", panelRequisitos);
    tituloRequisitos->setGeometry(20, 15, 280, 45);
    tituloRequisitos->setAlignment(Qt::AlignCenter);
    tituloRequisitos->setStyleSheet(
        "color: #e7c85b; background: transparent; border: none;"
        "font-size: 21px; font-weight: bold;"
        );

    requisitoLongitud = new QLabel(panelRequisitos);

    requisitoMayuscula = new QLabel(panelRequisitos);

    requisitoMinuscula = new QLabel(panelRequisitos);

    requisitoNumero = new QLabel(panelRequisitos);

    requisitoEspecial = new QLabel(panelRequisitos);

    requisitoCoincidencia = new QLabel(panelRequisitos);

    requisitoLongitud->setGeometry(20, 72, 280, 42);

    requisitoMayuscula->setGeometry(20, 124, 280, 42);

    requisitoMinuscula->setGeometry(20, 176, 280, 42);

    requisitoNumero->setGeometry(20, 228, 280, 42);

    requisitoEspecial->setGeometry(20, 280, 280, 42);

    requisitoCoincidencia->setGeometry(20, 332, 280, 42);

    QGraphicsProxyWidget *proxyRequisitos = addWidget(panelRequisitos);

    proxyRequisitos->setPos(765, 225);
    proxyRequisitos->setZValue(2);

    // =====================================================
    // MENSAJE DE ESTADO
    // =====================================================

    mensajeEstado = new QLabel;
    mensajeEstado->setFixedSize(860, 55);
    mensajeEstado->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mensajeEstado->setStyleSheet("QLabel {"
                                 "   background-color: transparent;"
                                 "   color: #e7c85b;"
                                 "   border: none;"
                                 "   font-size: 18px;"
                                 "   padding-left: 4px;"
                                 "}");

    QGraphicsProxyWidget *proxyMensaje = addWidget(mensajeEstado);

    proxyMensaje->setPos(205, 705);
    proxyMensaje->setZValue(2);

    // =====================================================
    // BOTONES
    // =====================================================

    botonVolver = new QPushButton("Volver");
    botonVolver->setFixedSize(285, 72);
    botonVolver->setStyleSheet(estiloBoton);
    botonVolver->setCursor(Qt::PointingHandCursor);

    QGraphicsProxyWidget *proxyVolver = addWidget(botonVolver);

    proxyVolver->setPos(285, 900);
    proxyVolver->setZValue(2);

    botonCrearCuenta = new QPushButton("Crear Cuenta");

    botonCrearCuenta->setFixedSize(350, 72);
    botonCrearCuenta->setStyleSheet(estiloBoton);
    botonCrearCuenta->setCursor(Qt::PointingHandCursor);
    botonCrearCuenta->setEnabled(false);

    QGraphicsProxyWidget *proxyCrear = addWidget(botonCrearCuenta);

    proxyCrear->setPos(650, 900);
    proxyCrear->setZValue(2);

    // =====================================================
    // CONEXIONES
    // =====================================================

    connect(campoUsuario, &QLineEdit::textChanged, this, &EscenaRegistro::actualizarValidaciones);

    connect(campoContrasena, &QLineEdit::textChanged, this, &EscenaRegistro::actualizarValidaciones);

    connect(campoConfirmacion,
            &QLineEdit::textChanged,
            this,
            &EscenaRegistro::actualizarValidaciones);

    connect(botonCrearCuenta, &QPushButton::clicked, this, &EscenaRegistro::intentarCrearCuenta);

    connect(
        botonMostrarContrasenas,
        &QPushButton::clicked,
        this,
        &EscenaRegistro::alternarVisibilidadContrasenas
        );

    connect(botonVolver, &QPushButton::clicked, this, [this]() { emit volverSolicitado(); });

    // Mantener el lienzo exactamente igual al tamaño del fondo.
    // Los widgets no podrán ampliar automáticamente la escena.
    setSceneRect(0, 0, 1254, 1254);
}

void EscenaRegistro::crearTitulo(
    const QString &texto,
    qreal x,
    qreal y,
    int ancho,
    const QString &estilo
    )
{
    QLabel *titulo = new QLabel(texto);
    titulo->setFixedSize(ancho, 38);
    titulo->setStyleSheet(estilo);

    QGraphicsProxyWidget *proxy = addWidget(titulo);
    proxy->setPos(x, y);
    proxy->setZValue(2);
}

void EscenaRegistro::actualizarIndicador(QLabel *indicador, bool cumplido, const QString &texto)
{
    if (cumplido) {
        indicador->setText("✓ " + texto);
        indicador->setStyleSheet("color: #b6ff00;"
                                 "background: transparent;"
                                 "border: none;"
                                 "font-size: 17px;"
                                 "font-weight: bold;");
    } else {
        indicador->setText("☐ " + texto);
        indicador->setStyleSheet("color: white;"
                                 "background: transparent;"
                                 "border: none;"
                                 "font-size: 17px;"
                                 "font-weight: bold;");
    }
}

void EscenaRegistro::alternarVisibilidadContrasenas()
{
    bool estanOcultas =
        campoContrasena->echoMode()
        == QLineEdit::Password;

    QLineEdit::EchoMode modoNuevo =
        estanOcultas
            ? QLineEdit::Normal
            : QLineEdit::Password;

    campoContrasena->setEchoMode(modoNuevo);
    campoConfirmacion->setEchoMode(modoNuevo);

    botonMostrarContrasenas->setText(
        estanOcultas ? "OCULTAR" : "MOSTRAR"
        );
}

void EscenaRegistro::actualizarValidaciones()
{
    QString usuario = campoUsuario->text();

    QString contrasena = campoContrasena->text();

    QString confirmacion = campoConfirmacion->text();

    actualizarIndicador(requisitoLongitud,
                        ValidarCuenta::longitudValida(contrasena),
                        "Entre 5 y 8 caracteres");

    actualizarIndicador(requisitoMayuscula,
                        ValidarCuenta::contieneMayuscula(contrasena),
                        "Al menos 1 letra mayúscula");

    actualizarIndicador(requisitoMinuscula,
                        ValidarCuenta::contieneMinuscula(contrasena),
                        "Al menos 1 letra minúscula");

    actualizarIndicador(requisitoNumero,
                        ValidarCuenta::contieneNumero(contrasena),
                        "Al menos 1 número");

    actualizarIndicador(requisitoEspecial,
                        ValidarCuenta::contieneCaracterEspecial(contrasena),
                        "Al menos 1 carácter especial");

    bool coinciden = !confirmacion.isEmpty()
                     && ValidarCuenta::contrasenasCoinciden(contrasena, confirmacion);

    actualizarIndicador(requisitoCoincidencia, coinciden, "Las contraseñas coinciden");

    botonCrearCuenta->setEnabled(formularioCompleto());

    if (!usuario.isEmpty() && !ValidarCuenta::usuarioValido(usuario)) {
        mensajeEstado->setText("Usuario inválido: use entre 3 y 15 "
                               "letras, números o _");
    } else if (!contrasena.isEmpty() && !ValidarCuenta::noContieneEspacios(contrasena)) {
        mensajeEstado->setText("La contraseña no puede contener espacios");
    } else if (contrasena.contains('|')) {
        mensajeEstado->setText("La contraseña no puede contener el símbolo |");
    } else if (formularioCompleto()) {
        mensajeEstado->setText("Todos los datos son válidos");
    } else {
        mensajeEstado->setText("Complete todos los requisitos");
    }
}

bool EscenaRegistro::formularioCompleto() const
{
    return ValidarCuenta::usuarioValido(campoUsuario->text())
           && ValidarCuenta::contrasenaValida(campoContrasena->text())
           && ValidarCuenta::contrasenasCoinciden(campoContrasena->text(),
                                                  campoConfirmacion->text());
}

void EscenaRegistro::intentarCrearCuenta()
{
    if (!formularioCompleto()) {
        QMessageBox::warning(QApplication::activeWindow(),
                             "Aviso",
                             "Revise los datos y cumpla todos "
                             "los requisitos.");

        return;
    }

    GestorUsuarios::ResultadoRegistro resultado
        = GestorUsuarios::registrarUsuario(campoUsuario->text(), campoContrasena->text());

    switch (resultado) {
    case GestorUsuarios::ResultadoRegistro::Exito: {
        // Guardar temporalmente el nombre antes
        // de limpiar los campos.
        QString usuarioCreado = campoUsuario->text();

        QMessageBox::information(QApplication::activeWindow(),
                                 "Cuenta creada",
                                 "La cuenta fue creada correctamente. "
                                 "Bienvenido, "
                                     + usuarioCreado + ".");

        campoUsuario->clear();
        campoContrasena->clear();
        campoConfirmacion->clear();
        campoContrasena->setEchoMode(QLineEdit::Password);
        campoConfirmacion->setEchoMode(QLineEdit::Password);
        botonMostrarContrasenas->setText("MOSTRAR");

        // MainWindow recibirá esta señal y abrirá
        // directamente el menú principal.
        emit cuentaCreada(usuarioCreado);

        break;
    }

    case GestorUsuarios::ResultadoRegistro::UsuarioDuplicado:

        mensajeEstado->setText("Ese nombre de usuario ya está registrado");

        QMessageBox::warning(QApplication::activeWindow(),
                             "Usuario duplicado",
                             "Ese nombre de usuario ya existe. "
                             "Escriba uno diferente.");

        campoUsuario->setFocus();
        campoUsuario->selectAll();
        break;

    case GestorUsuarios::ResultadoRegistro::ErrorArchivo:

        mensajeEstado->setText("No se pudo guardar la cuenta");

        QMessageBox::critical(QApplication::activeWindow(),
                              "Error",
                              "No se pudo abrir o escribir "
                              "el archivo de usuarios.");
        break;
    }
}
