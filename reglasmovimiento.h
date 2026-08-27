#ifndef REGLASMOVIMIENTO_H
#define REGLASMOVIMIENTO_H

class ReglasMovimiento
{
public:
    static bool dentroDeLimites(int x, int y, int columnas, int filas);
    static int envolverCoordenada(int valor, int limite);
};

#endif // REGLASMOVIMIENTO_H
