#include "progreso.h"

#include <cassert>

int main()
{
    ProgresoNivel progreso(5, 50, 8);

    assert(!progreso.gano());

    progreso.registrarFruta(10, 4);
    progreso.registrarFruta(10, 5);
    progreso.registrarFruta(10, 6);
    progreso.registrarFruta(10, 7);

    assert(!progreso.gano());
    assert(progreso.frutasComidas() == 4);
    assert(progreso.puntaje() == 40);

    progreso.registrarFruta(10, 8);

    assert(progreso.gano());
    assert(progreso.frutasComidas() == 5);
    assert(progreso.puntaje() == 50);

    return 0;
}
