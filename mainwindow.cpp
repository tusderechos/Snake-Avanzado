#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "juegoview.h"
#include "menujuego.h"
#include "modojuego.h"
#include "tutorial.h"
#include "escenaregistro.h"
#include "menuprincipal.h"
#include "iniciosesion.h"
#include "ranking.h"
#include "ajustes.h"
#include "perfil.h"
#include "controlesview.h"
#include "tienda.h"
#include "gestorusuarios.h"
#include "gestorconfiguracion.h"
#include "audio.h"
#include "dialogos.h"
#include "manualusuario.h"

#include <QCursor>
#include <QCloseEvent>
#include <QApplication>
#include <QFrame>
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QMessageBox>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QInputDialog>
#include <QPixmap>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>
#include <QGraphicsSimpleTextItem>
#include <QLinearGradient>
#include <QPen>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , escenaInicio(nullptr)
    , escenaRegistro(nullptr)
    , escenaInicioSesion(nullptr)
    , escenaMenuPrincipal(nullptr)
    , escenaRanking(nullptr)
    , escenaAjustes(nullptr)
    , escenaPerfil(nullptr)
    , escenaControles(nullptr)
    , escenaTienda(nullptr)
    , menuJuego(nullptr)
    , manualUsuario(nullptr)
{
    ui->setupUi(this);
    AudioManager::instancia().inicializar();

    // Ocultar elementos innecesarios
    ui->menubar->hide();
    ui->statusbar->hide();

    // El QGraphicsView ocupa toda la ventana
    QVBoxLayout *layoutPrincipal =
        new QVBoxLayout(ui->centralwidget);

    layoutPrincipal->setContentsMargins(0, 0, 0, 0);
    layoutPrincipal->setSpacing(0);
    layoutPrincipal->addWidget(ui->graphicsView);

    // Configuración de la ventana
    setWindowTitle("Snake");
    setFixedSize(800, 800);

    // Configuración del área gráfica
    ui->graphicsView->setFrameShape(
        QFrame::NoFrame
        );

    ui->graphicsView->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff
        );

    ui->graphicsView->setVerticalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff
        );

    // Construir las dos escenas
    construirMenuInicio();

    escenaRegistro =
        new EscenaRegistro(this);

    escenaMenuPrincipal =
        new MenuPrincipal(this);

    escenaRanking =
        new Ranking(this);

    escenaAjustes =
        new Ajustes(this);

    escenaPerfil =
        new Perfil(this);

    escenaControles =
        new ControlesView(this);

    escenaTienda =
        new Tienda(this);

    escenaInicioSesion=
        new InicioSesion(this);

    // Cuando Registro emita esta señal,
    // MainWindow regresará al menú.
    connect(
        escenaRegistro,
        &EscenaRegistro::volverSolicitado,
        this,
        &MainWindow::mostrarMenuInicio
        );

    connect(
        escenaInicioSesion,
        &InicioSesion::volverSolicitado,
        this,
        &MainWindow::mostrarMenuInicio
        );

    connect(
        escenaInicioSesion,
        &InicioSesion::sesionIniciada,
        this,
        &MainWindow::mostrarMenuPrincipal
        );

    connect(
        escenaRegistro,
        &EscenaRegistro::cuentaCreada,
        this,
        &MainWindow::mostrarMenuPrincipal
        );

    connect(
        escenaRanking,
        &Ranking::volverSolicitado,
        this,
        &MainWindow::regresarMenuPrincipal
        );

    connect(
        escenaAjustes,
        &Ajustes::volverSolicitado,
        this,
        &MainWindow::regresarMenuPrincipal
        );

    connect(
        escenaAjustes,
        &Ajustes::perfilSolicitado,
        this,
        &MainWindow::mostrarPerfil
        );

    connect(
        escenaAjustes,
        &Ajustes::controlesSolicitados,
        this,
        &MainWindow::mostrarControles
        );

    connect(
        escenaPerfil,
        &Perfil::volverSolicitado,
        this,
        &MainWindow::mostrarAjustes
        );

    connect(
        escenaControles,
        &ControlesView::volverSolicitado,
        this,
        &MainWindow::mostrarAjustes
        );

    connect(
        escenaTienda,
        &Tienda::volverSolicitado,
        this,
        &MainWindow::regresarMenuPrincipal
        );

    connect(
        escenaMenuPrincipal,
        &MenuPrincipal::jugarSolicitado,
        this,
        &MainWindow::mostrarMenuJuego
        );

    connect(
        escenaMenuPrincipal,
        &MenuPrincipal::rankingSolicitado,
        this,
        &MainWindow::mostrarRanking
        );

    connect(
        escenaMenuPrincipal,
        &MenuPrincipal::tiendaSolicitada,
        this,
        &MainWindow::mostrarTienda
        );

    connect(
        escenaMenuPrincipal,
        &MenuPrincipal::configuracionSolicitada,
        this,
        &MainWindow::mostrarAjustes
        );

    connect(
        escenaMenuPrincipal,
        &MenuPrincipal::salirSolicitado,
        this,
        &MainWindow::cerrarSesion
        );

    connect(
        escenaMenuPrincipal,
        &MenuPrincipal::manualSolicitado,
        this,
        &MainWindow::mostrarManual
        );

    connect(&GestorUsuarios::instancia(), &GestorUsuarios::errorGuardado,
        this, &MainWindow::mostrarErrorGuardado);
    connect(&GestorUsuarios::instancia(), &GestorUsuarios::perfilActualizado, this, [this]() {
        if (menuJuego) menuJuego->establecerTutorialDisponible(
            !GestorUsuarios::tutorialCompletado(usuarioActual));
    });

    // Mostrar inicialmente el menú
    mostrarMenuInicio();
}

void MainWindow::construirMenuInicio()
{
    escenaInicio =
        new QGraphicsScene(this);

    // =====================================================
    // FONDO
    // =====================================================

    QPixmap fondo(
        ":/imagenes/imagenes/MenuI.png"
        );

    QGraphicsPixmapItem *imagenFondo =
        escenaInicio->addPixmap(fondo);

    imagenFondo->setZValue(0);

    escenaInicio->setSceneRect(
        imagenFondo->boundingRect()
        );

    // =====================================================
    // TÍTULO DEL JUEGO CON EFECTO 3D
    // =====================================================

    QString nombreJuego =
        "Serpent Society";

    QFont fuenteTitulo(
        "Fredoka",
        82,
        QFont::Black
        );

    fuenteTitulo.setItalic(true);
    fuenteTitulo.setLetterSpacing(
        QFont::AbsoluteSpacing,
        3
        );

    // Capa oscura trasera
    QGraphicsSimpleTextItem *sombraTitulo =
        escenaInicio->addSimpleText(
            nombreJuego,
            fuenteTitulo
            );

    sombraTitulo->setBrush(
        QColor(5, 25, 0, 230)
        );

    sombraTitulo->setPen(
        QPen(QColor(0, 0, 0, 240), 8)
        );

    qreal posicionTituloX =
        (
            escenaInicio->sceneRect().width()
            - sombraTitulo->boundingRect().width()
            ) / 2.0;

    const qreal posicionTituloY = 65;

    sombraTitulo->setPos(
        posicionTituloX + 14,
        posicionTituloY + 16
        );

    sombraTitulo->setZValue(1);

    // Capa intermedia que produce profundidad
    QGraphicsSimpleTextItem *profundidadTitulo =
        escenaInicio->addSimpleText(
            nombreJuego,
            fuenteTitulo
            );

    profundidadTitulo->setBrush(
        QColor(22, 77, 5)
        );

    profundidadTitulo->setPen(
        QPen(QColor(7, 29, 0), 7)
        );

    profundidadTitulo->setPos(
        posicionTituloX + 7,
        posicionTituloY + 8
        );

    profundidadTitulo->setZValue(2);

    // Degradado de la capa frontal
    QLinearGradient degradadoTitulo(
        0,
        0,
        0,
        120
        );

    degradadoTitulo.setColorAt(
        0.0,
        QColor(239, 255, 138)
        );

    degradadoTitulo.setColorAt(
        0.35,
        QColor(141, 255, 34)
        );

    degradadoTitulo.setColorAt(
        0.7,
        QColor(50, 184, 13)
        );

    degradadoTitulo.setColorAt(
        1.0,
        QColor(11, 92, 0)
        );

    // Capa frontal
    QGraphicsSimpleTextItem *tituloFrontal =
        escenaInicio->addSimpleText(
            nombreJuego,
            fuenteTitulo
            );

    tituloFrontal->setBrush(
        QBrush(degradadoTitulo)
        );

    tituloFrontal->setPen(
        QPen(QColor(223, 255, 69), 4)
        );

    tituloFrontal->setPos(
        posicionTituloX,
        posicionTituloY
        );

    tituloFrontal->setZValue(3);


    // =====================================================
    // LOGO PRINCIPAL
    // =====================================================

    QPixmap logoOriginal(
        ":/imagenes/imagenes/Logo2.png"
        );

    QPixmap logoEscalado =
        logoOriginal.scaled(
            560,
            560,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            );

    QGraphicsPixmapItem *logo =
        escenaInicio->addPixmap(
            logoEscalado
            );

    logo->setZValue(2);

    // Centrado horizontal exacto
    qreal posicionLogoX =
        escenaInicio->sceneRect().center().x()
        - logoEscalado.width() / 2.0;

    logo->setPos(
        posicionLogoX,
        145
        );

    // =====================================================
    // ESTILO DE LOS BOTONES
    // =====================================================

    QString estiloBoton =
        "QPushButton {"
        "   background-color: rgba(55, 145, 20, 220);"
        "   color: white;"
        "   border: 6px solid #39ff14;"
        "   border-radius: 14px;"
        "   font-size: 36px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(85, 190, 25, 235);"
        "   border-color: #b6ff00;"
        "}"
        "QPushButton:pressed {"
        "   background-color: rgba(35, 110, 10, 240);"
        "}";

    // =====================================================
    // BOTÓN CREAR CUENTA
    // =====================================================

    QPushButton *botonCrearCuenta =
        new QPushButton("Crear Cuenta");

    botonCrearCuenta->setFixedSize(
        520,
        90
        );

    botonCrearCuenta->setStyleSheet(
        estiloBoton
        );

    botonCrearCuenta->setCursor(
        Qt::PointingHandCursor
        );

    QGraphicsProxyWidget *proxyCrear =
        escenaInicio->addWidget(
            botonCrearCuenta
            );

    proxyCrear->setZValue(2);

    // =====================================================
    // BOTÓN INICIAR SESIÓN
    // =====================================================

    QPushButton *botonIniciarSesion =
        new QPushButton("Iniciar Sesión");

    botonIniciarSesion->setFixedSize(
        520,
        90
        );

    botonIniciarSesion->setStyleSheet(
        estiloBoton
        );

    botonIniciarSesion->setCursor(
        Qt::PointingHandCursor
        );

    QGraphicsProxyWidget *proxyIniciar =
        escenaInicio->addWidget(
            botonIniciarSesion
            );

    proxyIniciar->setZValue(2);

    // =====================================================
    // POSICIÓN DE LOS BOTONES
    // =====================================================

    const qreal botonCrearY = 660;
    const qreal separacionBotones = 125;

    proxyCrear->setPos(
        (
            escenaInicio->sceneRect().width()
            - botonCrearCuenta->width()
            ) / 2.0,
        botonCrearY
        );

    proxyIniciar->setPos(
        (
            escenaInicio->sceneRect().width()
            - botonIniciarSesion->width()
            ) / 2.0,
        botonCrearY + separacionBotones
        );

    // =====================================================
    // CONEXIONES DE LOS BOTONES
    // =====================================================

    connect(
        botonCrearCuenta,
        &QPushButton::clicked,
        this,
        &MainWindow::mostrarRegistro
        );

    connect(
        botonIniciarSesion,
        &QPushButton::clicked,
        this,
        &MainWindow::mostrarInicioSesion
        );

    botonManualInicio = new QPushButton("?", nullptr);
    botonManualInicio->setFixedSize(82, 82);
    botonManualInicio->setCursor(Qt::PointingHandCursor);
    botonManualInicio->setToolTip("Manual de usuario");
    botonManualInicio->setStyleSheet("QPushButton { background: rgba(48,132,24,235); color: white; border: 4px solid #68dd3e; border-radius: 10px; font: bold 42px 'Fredoka'; } QPushButton:hover { background: #4cae22; border-color: #f4d06f; }");
    auto *proxyManual = escenaInicio->addWidget(botonManualInicio);
    proxyManual->setZValue(4);
    const QRectF limitesInicio = escenaInicio->sceneRect();
    proxyManual->setPos(limitesInicio.right() - botonManualInicio->width() - 28,
                        limitesInicio.bottom() - botonManualInicio->height() - 28);
    connect(botonManualInicio, &QPushButton::clicked, this, &MainWindow::mostrarManual);
}

void MainWindow::mostrarManual()
{
    if (manualUsuario != nullptr) return;
    manualUsuario = new ManualUsuario();
    manualUsuario->setAttribute(Qt::WA_DeleteOnClose);
    connect(manualUsuario, &QObject::destroyed, this, [this]() {
        manualUsuario = nullptr;
        show();
    });
    hide();
    manualUsuario->show();
    manualUsuario->raise();
    manualUsuario->activateWindow();
}

void MainWindow::mostrarRegistro()
{
    botonManualInicio->setVisible(false);
    escenaMenuPrincipal->establecerManualVisible(false);
    AudioManager::instancia().reproducirMenu();
    ui->graphicsView->setScene(
        escenaRegistro
        );

    ajustarEscenaActual();
}

void MainWindow::mostrarInicioSesion()
{
    botonManualInicio->setVisible(false);
    escenaMenuPrincipal->establecerManualVisible(false);
    AudioManager::instancia().reproducirMenu();
    ui->graphicsView->setScene(
        escenaInicioSesion
        );
    ajustarEscenaActual();
}

void MainWindow::mostrarMenuInicio()
{
    botonManualInicio->setVisible(true);
    escenaMenuPrincipal->establecerManualVisible(false);
    AudioManager::instancia().reproducirMenu();
    ui->graphicsView->setScene(
        escenaInicio
        );

    ajustarEscenaActual();
}

void MainWindow::mostrarMenuPrincipal(
    const QString &usuario
    )
{
    botonManualInicio->setVisible(false);
    escenaMenuPrincipal->establecerManualVisible(true);
    usuarioActual = usuario;
    int volumenMusica = 75;
    int volumenSonido = 75;
    GestorConfiguracion::cargarVolumen(
        usuarioActual, volumenMusica, volumenSonido);
    AudioManager::instancia().establecerVolumenMusica(volumenMusica);
    AudioManager::instancia().establecerVolumenSonido(volumenSonido);
    AudioManager::instancia().reproducirMenu();

    ui->graphicsView->setScene(
        escenaMenuPrincipal
        );

    ajustarEscenaActual();
}

void MainWindow::mostrarRanking()
{
    escenaMenuPrincipal->establecerManualVisible(false);
    AudioManager::instancia().reproducirMenu();
    escenaRanking->actualizarRanking();

    ui->graphicsView->setScene(
        escenaRanking
        );

    ajustarEscenaActual();
}

void MainWindow::mostrarAjustes()
{
    escenaMenuPrincipal->establecerManualVisible(false);
    AudioManager::instancia().reproducirMenu();
    escenaAjustes->establecerUsuario(
        usuarioActual
        );

    ui->graphicsView->setScene(
        escenaAjustes
        );

    ajustarEscenaActual();
}

void MainWindow::mostrarPerfil()
{
    escenaMenuPrincipal->establecerManualVisible(false);
    escenaAjustes->confirmarCambios();
    AudioManager::instancia().reproducirMenu();
    escenaPerfil->establecerUsuario(
        usuarioActual
        );

    ui->graphicsView->setScene(
        escenaPerfil
        );

    ajustarEscenaActual();
}

void MainWindow::mostrarControles()
{
    escenaMenuPrincipal->establecerManualVisible(false);
    escenaAjustes->confirmarCambios();
    AudioManager::instancia().reproducirMenu();
    escenaControles->establecerUsuario(
        usuarioActual
        );

    ui->graphicsView->setScene(
        escenaControles
        );

    ajustarEscenaActual();
}

void MainWindow::mostrarTienda()
{
    escenaMenuPrincipal->establecerManualVisible(false);
    AudioManager::instancia().reproducirTienda();
    escenaTienda->establecerUsuario(usuarioActual);
    ui->graphicsView->setScene(
        escenaTienda
        );

    ajustarEscenaActual();
}

void MainWindow::cerrarSesion()
{
    escenaAjustes->confirmarCambios();
    if (!GestorUsuarios::cerrarSesion()) {
        mostrarErrorGuardado("Hay cambios pendientes de guardar. Espere a que termine el guardado o reintente antes de cerrar sesión.");
        return;
    }
    escenaAjustes->establecerUsuario({});
    escenaControles->establecerUsuario({});
    escenaTienda->establecerUsuario({});
    usuarioActual.clear();
    mostrarMenuInicio();
}

void MainWindow::regresarMenuPrincipal()
{
    botonManualInicio->setVisible(false);
    escenaMenuPrincipal->establecerManualVisible(true);
    escenaAjustes->confirmarCambios();
    AudioManager::instancia().reproducirMenu();
    ui->graphicsView->setScene(
        escenaMenuPrincipal
        );

    ajustarEscenaActual();
}

void MainWindow::mostrarMenuJuego()
{
    botonManualInicio->setVisible(false);
    escenaMenuPrincipal->establecerManualVisible(false);
    AudioManager::instancia().reproducirMenu();
    if (menuJuego != nullptr)
    {
        if (!GestorUsuarios::tutorialCompletado(usuarioActual)) {
            iniciarModoJuego(ModoJuego::Tutorial);
            return;
        }
        menuJuego->establecerTutorialDisponible(false);
        menuJuego->show();
        menuJuego->raise();
        menuJuego->activateWindow();
        hide();
        return;
    }

    menuJuego = new MenuJuego(
        [this](ModoJuego modo)
        {
            iniciarModoJuego(modo);
        }
        );

    menuJuego->setAttribute(Qt::WA_DeleteOnClose);
    connect(
        menuJuego,
        &QObject::destroyed,
        this,
        [this]()
        {
            menuJuego = nullptr;
            escenaMenuPrincipal->establecerManualVisible(true);
            show();
        }
        );

    if (!GestorUsuarios::tutorialCompletado(usuarioActual)) {
        hide();
        iniciarModoJuego(ModoJuego::Tutorial);
        return;
    }

    menuJuego->establecerTutorialDisponible(false);
    hide();
    menuJuego->show();
}

void MainWindow::iniciarModoJuego(ModoJuego modo)
{
    if (menuJuego == nullptr)
    {
        return;
    }

    if (modo == ModoJuego::Tutorial)
    {
        AudioManager::instancia().reproducirJuego();
        auto *tutorial = new TutorialView(usuarioActual);
        tutorial->setAttribute(Qt::WA_DeleteOnClose);
        menuJuego->hide();
        connect(tutorial, &QObject::destroyed, menuJuego, [this]() {
            AudioManager::instancia().reproducirMenu();
            menuJuego->establecerTutorialDisponible(
                !GestorUsuarios::tutorialCompletado(usuarioActual));
            menuJuego->show();
        });
        tutorial->show();
        return;
    }

    if (modo == ModoJuego::Normal
        && !GestorUsuarios::tutorialCompletado(usuarioActual)) {
        Dialogos::mostrar(
            menuJuego,
            QMessageBox::Information,
            "Tutorial requerido",
            "Antes de jugar HISTORIA tenés que completar el TUTORIAL."
            );
        auto *tutorial = new TutorialView(usuarioActual);
        tutorial->setAttribute(Qt::WA_DeleteOnClose);
        menuJuego->hide();
        connect(tutorial, &QObject::destroyed, menuJuego, [this]() {
            AudioManager::instancia().reproducirMenu();
            menuJuego->establecerTutorialDisponible(
                !GestorUsuarios::tutorialCompletado(usuarioActual));
            menuJuego->show();
        });
        tutorial->show();
        return;
    }

    int nivelInicial = 1;
    ConfiguracionJuego configuracion;

    if (modo == ModoJuego::Libre)
    {
        const int nivelMaximo = GestorUsuarios::obtenerNivelHistoria(usuarioActual);
        if (nivelMaximo < 1) {
            Dialogos::mostrar(
                menuJuego,
                QMessageBox::Information,
                "Modo Libre bloqueado",
                "Primero completá al menos el Nivel 1 de HISTORIA."
                );
            return;
        }

        QDialog dialogo(menuJuego);
        dialogo.setWindowTitle("Modo Libre");
        dialogo.setFixedSize(430, 270);
        dialogo.setStyleSheet(
            "QDialog { background: #102214; color: #fff9df; }"
            "QLabel#titulo { color: #f4d06f; font: bold 25px 'Fredoka'; }"
            "QLabel#descripcion { color: #d7e5cd; font-size: 15px; }"
            "QSpinBox { background: #071307; color: white; border: 2px solid #d2a93b;"
            " border-radius: 8px; padding: 8px; font-size: 22px; }"
            "QDialogButtonBox QPushButton { background: #2f8618; color: white; border: 2px solid #68dd3e;"
            " border-radius: 8px; padding: 8px 24px; font-weight: bold; }"
            "QDialogButtonBox QPushButton:hover { border-color: #f4d06f; }");
        auto *layout = new QVBoxLayout(&dialogo);
        layout->setContentsMargins(28, 22, 28, 22);
        auto *titulo = new QLabel("ELEGÍ TU NIVEL", &dialogo);
        titulo->setObjectName("titulo");
        titulo->setAlignment(Qt::AlignCenter);
        layout->addWidget(titulo);
        auto *descripcion = new QLabel(
            QString("Podés comenzar en cualquiera de tus niveles desbloqueados.\n\nDesbloqueados: 1 - %1").arg(nivelMaximo),
            &dialogo);
        descripcion->setObjectName("descripcion");
        descripcion->setAlignment(Qt::AlignCenter);
        descripcion->setWordWrap(true);
        layout->addWidget(descripcion);
        auto *nivel = new QSpinBox(&dialogo);
        nivel->setRange(1, nivelMaximo);
        nivel->setValue(nivelMaximo);
        layout->addWidget(nivel);
        auto *botones = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialogo);
        botones->button(QDialogButtonBox::Ok)->setText("JUGAR");
        botones->button(QDialogButtonBox::Cancel)->setText("VOLVER");
        layout->addWidget(botones);
        connect(botones, &QDialogButtonBox::accepted, &dialogo, &QDialog::accept);
        connect(botones, &QDialogButtonBox::rejected, &dialogo, &QDialog::reject);
        if (dialogo.exec() != QDialog::Accepted) {
            return;
        }
        nivelInicial = nivel->value();

        configuracion.progresionAutomatica = false;
    }
    else if (modo == ModoJuego::Aleatorio)
    {
        QDialog dialogo(menuJuego);
        dialogo.setWindowTitle("Modo Aleatorio");
        dialogo.setFixedSize(480, 540);
        dialogo.setStyleSheet(
            "QDialog { background: #102214; color: #fff9df; }"
            "QLabel#titulo { color: #f4d06f; font: bold 25px 'Fredoka'; }"
            "QLabel#descripcion { color: #d7e5cd; font-size: 14px; }"
            "QFrame#panel { background: rgba(7, 19, 7, 220); border: 2px solid #806c36; border-radius: 12px; }"
            "QLabel { color: #fff9df; }"
            "QSpinBox { background: #071307; color: white; border: 2px solid #d2a93b; border-radius: 7px; padding: 5px; }"
            "QCheckBox { spacing: 10px; padding: 6px; font-size: 15px; }"
            "QCheckBox::indicator { width: 20px; height: 20px; border: 2px solid #68dd3e; border-radius: 5px; background: #071307; }"
            "QCheckBox::indicator:checked { background: #68dd3e; }"
            "QDialogButtonBox QPushButton { background: #2f8618; color: white; border: 2px solid #68dd3e; border-radius: 8px; padding: 8px 24px; font-weight: bold; }");

        auto *contenedor = new QVBoxLayout(&dialogo);
        contenedor->setContentsMargins(24, 20, 24, 20);
        auto *titulo = new QLabel("MODO ALEATORIO", &dialogo);
        titulo->setObjectName("titulo");
        titulo->setAlignment(Qt::AlignCenter);
        contenedor->addWidget(titulo);
        auto *descripcion = new QLabel("Personalizá los elementos y el caos de esta partida.", &dialogo);
        descripcion->setObjectName("descripcion");
        descripcion->setAlignment(Qt::AlignCenter);
        contenedor->addWidget(descripcion);

        auto *panel = new QFrame(&dialogo);
        panel->setObjectName("panel");
        auto *formulario = new QFormLayout(panel);
        formulario->setContentsMargins(22, 18, 22, 18);
        formulario->setVerticalSpacing(8);
        auto *nivel = new QSpinBox(&dialogo);
        nivel->setRange(1, 3);
        nivel->setValue(3);
        auto *frutas = new QCheckBox("Frutas especiales", &dialogo);
        auto *items = new QCheckBox("Cajas e ítems", &dialogo);
        auto *obstaculos = new QCheckBox("Obstáculos", &dialogo);
        auto *moviles = new QCheckBox("Obstáculos móviles", &dialogo);
        auto *azar = new QCheckBox("Aparición aleatoria", &dialogo);

        frutas->setChecked(true);
        items->setChecked(true);
        obstaculos->setChecked(true);
        moviles->setChecked(true);
        azar->setChecked(true);

        formulario->addRow("Nivel inicial:", nivel);
        formulario->addRow(frutas);
        formulario->addRow(items);
        formulario->addRow(obstaculos);
        formulario->addRow(moviles);
        formulario->addRow(azar);

        contenedor->addWidget(panel);
        auto *botones = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
            &dialogo
            );
        botones->button(QDialogButtonBox::Ok)->setText("COMENZAR");
        botones->button(QDialogButtonBox::Cancel)->setText("VOLVER");
        contenedor->addWidget(botones);

        connect(
            botones,
            &QDialogButtonBox::accepted,
            &dialogo,
            &QDialog::accept
            );
        connect(
            botones,
            &QDialogButtonBox::rejected,
            &dialogo,
            &QDialog::reject
            );

        if (dialogo.exec() != QDialog::Accepted)
        {
            return;
        }

        nivelInicial = nivel->value();
        configuracion.frutasEspeciales = frutas->isChecked();
        configuracion.items = items->isChecked();
        configuracion.obstaculos = obstaculos->isChecked();
        configuracion.obstaculosMoviles = moviles->isChecked();
        configuracion.aparicionAleatoria = azar->isChecked();
        configuracion.progresionAutomatica = false;
        configuracion.esAleatorio = true;
    }

    AudioManager::instancia().reproducirJuego();
    auto *juego = new JuegoView(nivelInicial, configuracion, usuarioActual);
    juego->setAttribute(Qt::WA_DeleteOnClose);
    menuJuego->hide();
    connect(juego, &QObject::destroyed, menuJuego, [this]() {
        AudioManager::instancia().reproducirMenu();
        menuJuego->show();
    });
    juego->show();
}



void MainWindow::ajustarEscenaActual()
{
    QTimer::singleShot(
        0,
        this,
        [this]()
        {
            if (ui->graphicsView->scene() != nullptr)
            {
                ui->graphicsView->resetTransform();

                ui->graphicsView->fitInView(
                    QRectF(0, 0, 1254, 1254),
                    Qt::KeepAspectRatio
                    );

                ui->graphicsView->centerOn(
                    627,
                    627
                    );
            }
        }
        );
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::mostrarErrorGuardado(const QString &mensaje)
{
    const QString mensajeNormalizado = mensaje.toLower();
    if (mensajeNormalizado.contains("tablas")
        && mensajeNormalizado.contains("columnas")) {
        // Es un detalle de configuración del backend, no un error accionable
        // para quien está navegando por el juego. Se registra sin interrumpir.
        qWarning() << "Configuración de Supabase incompleta:" << mensaje;
        return;
    }
    if (avisoGuardado) {
        avisoGuardado->setText(mensaje);
        Dialogos::aplicarEstilo(*avisoGuardado);
        avisoGuardado->raise();
        return;
    }
    auto *aviso = new QMessageBox(QMessageBox::Warning, "Guardado pendiente",
        mensaje, QMessageBox::Ok, QApplication::activeWindow());
    avisoGuardado = aviso;
    aviso->setAttribute(Qt::WA_DeleteOnClose);
    if (GestorUsuarios::pendientes()) {
        auto *reintentar = aviso->addButton("Reintentar", QMessageBox::ActionRole);
        connect(reintentar, &QPushButton::clicked, this, []() { GestorUsuarios::reintentar(); });
    }
    Dialogos::aplicarEstilo(*aviso);
    aviso->open();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    escenaAjustes->confirmarCambios();
    if (GestorUsuarios::pendientes()) {
        event->ignore();
        mostrarErrorGuardado("Hay cambios pendientes de guardar. Mantenga el juego abierto y reintente el guardado antes de salir.");
        return;
    }
    QMainWindow::closeEvent(event);
}
