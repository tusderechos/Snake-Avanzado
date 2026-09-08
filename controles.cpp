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

bool Controles::esTeclaPermitida(int tecla, EsquemaControles esquema) {
    const bool esWASD = tecla == Qt::Key_W || tecla == Qt::Key_A
                     || tecla == Qt::Key_S || tecla == Qt::Key_D;
    const bool esFlecha = tecla == Qt::Key_Left || tecla == Qt::Key_Right
                       || tecla == Qt::Key_Up || tecla == Qt::Key_Down;
    return esquema == EsquemaControles::WASD ? esWASD : esFlecha;
}
