#include "reglasmovimiento.h"

bool ReglasMovimiento::dentroDeLimites(int x, int y, int columnas, int filas) {
    return x >= 0 && x < columnas && y >= 0 && y < filas;
}

int ReglasMovimiento::envolverCoordenada(int valor, int limite) {
    if (limite <= 0) {
        return 0;
    }

    if (valor < 0) {
        return limite - 1;
    }
    if (valor >= limite) {
        return 0;
    }
    return valor;
}

bool ReglasMovimiento::colisionaConSerpiente(bool destinoEsSerpiente,
                                             bool destinoEsCola,
                                             bool crecimientoPendiente) {
    if (!destinoEsSerpiente) {
        return false;
    }

    // La cola puede abandonarse en este turno, excepto si el crecimiento
    // pendiente obliga a conservarla.
    return !destinoEsCola || crecimientoPendiente;
}
