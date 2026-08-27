#include "reglasmovimiento.h"

bool ReglasMovimiento::dentroDeLimites(int x, int y, int columnas, int filas)
{
    return x >= 0 && x < columnas && y >= 0 && y < filas;
}

int ReglasMovimiento::envolverCoordenada(int valor, int limite)
{
    if (valor < 0) {
        return limite - 1;
    }
    if (valor >= limite) {
        return 0;
    }
    return valor;
}
