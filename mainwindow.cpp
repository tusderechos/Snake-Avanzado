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

#include <QCursor>
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
{
    ui->setupUi(this);

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

    // Una cuenta nueva entra automáticamente al juego.
    connect(
        escenaRegistro,
        &EscenaRegistro::cuentaCreada,
        this,
        &MainWindow::mostrarMenuPrincipal
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
        "Georgia",
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
}

void MainWindow::mostrarRegistro()
{
    ui->graphicsView->setScene(
        escenaRegistro
        );

    ajustarEscenaActual();
}

void MainWindow::mostrarInicioSesion()
{
    ui->graphicsView->setScene(
        escenaInicioSesion
        );
    ajustarEscenaActual();
}

void MainWindow::mostrarMenuInicio()
{
    ui->graphicsView->setScene(
        escenaInicio
        );

    ajustarEscenaActual();
}

void MainWindow::mostrarMenuPrincipal(
    const QString &usuario
    )
{
    usuarioActual = usuario;

    ui->graphicsView->setScene(
        escenaMenuPrincipal
        );

    ajustarEscenaActual();
}

void MainWindow::mostrarRanking()
{
    escenaRanking->actualizarRanking();

    ui->graphicsView->setScene(
        escenaRanking
        );

    ajustarEscenaActual();
}

void MainWindow::mostrarAjustes()
{
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
    escenaTienda->establecerUsuario(usuarioActual);
    ui->graphicsView->setScene(
        escenaTienda
        );

    ajustarEscenaActual();
}

void MainWindow::cerrarSesion()
{
    usuarioActual.clear();
    mostrarMenuInicio();
}

void MainWindow::regresarMenuPrincipal()
{
    ui->graphicsView->setScene(
        escenaMenuPrincipal
        );

    ajustarEscenaActual();
}

void MainWindow::mostrarMenuJuego()
{
    if (menuJuego != nullptr)
    {
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
            show();
        }
        );

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
        auto *tutorial = new TutorialView(usuarioActual);
        tutorial->setAttribute(Qt::WA_DeleteOnClose);
        menuJuego->hide();
        connect(
            tutorial,
            &QObject::destroyed,
            menuJuego,
            &QWidget::show
            );
        tutorial->show();
        return;
    }

    if (modo == ModoJuego::Normal
        && !GestorUsuarios::tutorialCompletado(usuarioActual)) {
        QMessageBox::information(
            menuJuego,
            "Tutorial requerido",
            "Antes de jugar HISTORIA tenés que completar el TUTORIAL."
            );
        auto *tutorial = new TutorialView(usuarioActual);
        tutorial->setAttribute(Qt::WA_DeleteOnClose);
        menuJuego->hide();
        connect(tutorial, &QObject::destroyed, menuJuego, &QWidget::show);
        tutorial->show();
        return;
    }

    int nivelInicial = 1;
    ConfiguracionJuego configuracion;

    if (modo == ModoJuego::Libre)
    {
        const int nivelMaximo = GestorUsuarios::obtenerNivelHistoria(usuarioActual);
        if (nivelMaximo < 1) {
            QMessageBox::information(
                menuJuego,
                "Modo Libre bloqueado",
                "Primero completá al menos el Nivel 1 de HISTORIA."
                );
            return;
        }

        bool aceptado = false;
        nivelInicial = QInputDialog::getInt(
            menuJuego,
            "Modo Libre",
            "Elegí el nivel inicial:",
            nivelMaximo,
            1,
            nivelMaximo,
            1,
            &aceptado
            );

        if (!aceptado)
        {
            return;
        }

        configuracion.progresionAutomatica = false;
    }
    else if (modo == ModoJuego::Aleatorio)
    {
        QDialog dialogo(menuJuego);
        dialogo.setWindowTitle("Configuración aleatoria");
        dialogo.setStyleSheet(
            "QDialog { background: #0d121a; color: #ebf0f5; }"
            );

        auto *formulario = new QFormLayout(&dialogo);
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

        auto *botones = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
            &dialogo
            );
        formulario->addRow(botones);

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

    auto *juego = new JuegoView(nivelInicial, configuracion, usuarioActual);
    juego->setAttribute(Qt::WA_DeleteOnClose);
    menuJuego->hide();
    connect(
        juego,
        &QObject::destroyed,
        menuJuego,
        &QWidget::show
        );
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
