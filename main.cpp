#include "juegoview.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication aplicacion(argc, argv);

    JuegoView juego;
    juego.show();

    return aplicacion.exec();
}
