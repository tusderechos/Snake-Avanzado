#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication aplicacion(argc, argv);
    MainWindow ventana;
    ventana.show();
    return aplicacion.exec();
}
