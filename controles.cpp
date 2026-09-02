#include "controles.h"

EsquemaControles Controles::detectar(int tecla) {
    return tecla == Qt::Key_W || tecla == Qt::Key_A
                   || tecla == Qt::Key_S || tecla == Qt::Key_D
               ? EsquemaControles::WASD
               : EsquemaControles::Flechas;
}

bool Controles::esMovimiento(int tecla) {
    return tecla == Qt::Key_W || tecla == Qt::Key_A
        || tecla == Qt::Key_S || tecla == Qt::Key_D
        || tecla == Qt::Key_Left || tecla == Qt::Key_Right
        || tecla == Qt::Key_Up || tecla == Qt::Key_Down;
}
