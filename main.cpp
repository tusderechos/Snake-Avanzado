#include "mainwindow.h"
#include "gestorusuarios.h"

#include <QApplication>
#include <QFontDatabase>

int main(int argc, char *argv[]) {
    QApplication aplicacion(argc, argv);
    // Los plugins de imagen se distribuyen junto al ejecutable, por lo que
    // deben resolverse desde la carpeta de la aplicación y no desde el
    // directorio de trabajo elegido por Windows o por el acceso directo.
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());
    aplicacion.setOrganizationName("SnakeAvanzado");
    aplicacion.setApplicationName("Snake");
    QFontDatabase::addApplicationFont(":/assets/Fredoka-Variable.ttf");
    MainWindow ventana;
    ventana.show();
    return aplicacion.exec();
}
