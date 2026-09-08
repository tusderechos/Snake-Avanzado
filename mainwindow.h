#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class QGraphicsScene;
class EscenaRegistro;
class MenuPrincipal;
class InicioSesion;
class Ranking;
class Ajustes;
class Perfil;
class ControlesView;
class Tienda;
class MenuJuego;
enum class ModoJuego;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(
        QWidget *parent = nullptr
        );

    ~MainWindow();

private slots:
    // Abre la pantalla para crear una cuenta.
    void mostrarRegistro();

    //Abre la pantalla de inicio sesion
    void mostrarInicioSesion();

    // Regresa a la primera pantalla.
    void mostrarMenuInicio();

    // Inicia la sesión y abre el menú del juego.
    void mostrarMenuPrincipal(
        const QString &usuario
        );

    // Abre la pantalla con los cinco mejores jugadores.
    void mostrarRanking();

    // Abre la pantalla de configuración.
    void mostrarAjustes();

    // Abre el perfil del usuario conectado.
    void mostrarPerfil();

    // Abre la selección de controles del usuario conectado.
    void mostrarControles();

    // Abre la pantalla inicial de la tienda.
    void mostrarTienda();

    // Cierra la sesión y regresa al menú de acceso.
    void cerrarSesion();

    // Regresa desde una pantalla secundaria al menú principal.
    void regresarMenuPrincipal();

    // Abre el selector de modos del juego.
    void mostrarMenuJuego();

    // Inicia el modo elegido desde el selector de juego.
    void iniciarModoJuego(ModoJuego modo);

private:
    Ui::MainWindow *ui;

    QGraphicsScene *escenaInicio;
    EscenaRegistro *escenaRegistro;
    InicioSesion *escenaInicioSesion;
    MenuPrincipal *escenaMenuPrincipal;
    Ranking *escenaRanking;
    Ajustes *escenaAjustes;
    Perfil *escenaPerfil;
    ControlesView *escenaControles;
    Tienda *escenaTienda;
    MenuJuego *menuJuego;

    // Nombre del usuario que tiene la sesión abierta.
    QString usuarioActual;

    void construirMenuInicio();
    void ajustarEscenaActual();
};

#endif // MAINWINDOW_H
