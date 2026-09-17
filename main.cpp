#include "mainwindow.h"
#include "gestorusuarios.h"

#include <QApplication>
#include <QFontDatabase>

int main(int argc, char *argv[]) {
    QApplication aplicacion(argc, argv);
    aplicacion.setOrganizationName("SnakeAvanzado");
    aplicacion.setApplicationName("Snake");
    QFontDatabase::addApplicationFont(":/assets/Fredoka-Variable.ttf");
    GestorUsuarios::asegurarCuentaAdmin();
    MainWindow ventana;
    ventana.show();
    return aplicacion.exec();
}
