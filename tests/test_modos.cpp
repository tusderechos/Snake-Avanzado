#include "controles.h"
#include "reglasmovimiento.h"

#include <iostream>

namespace {
bool comprobar(bool condicion, const char *mensaje) {
    if (!condicion) std::cerr << "Fallo: " << mensaje << '\n';
    return condicion;
}
}

int main() {
    bool ok = true;
    ok &= comprobar(Controles::esTeclaPermitida(Qt::Key_W, EsquemaControles::WASD),
                    "W debe funcionar con WASD");
    ok &= comprobar(!Controles::esTeclaPermitida(Qt::Key_W, EsquemaControles::Flechas),
                    "W no debe funcionar con flechas");
    ok &= comprobar(Controles::esTeclaPermitida(Qt::Key_Left, EsquemaControles::Flechas),
                    "flecha izquierda debe funcionar con flechas");
    ok &= comprobar(ReglasMovimiento::envolverCoordenada(-1, 10) == 9,
                    "envoltura negativa");
    ok &= comprobar(ReglasMovimiento::envolverCoordenada(10, 10) == 0,
                    "envoltura positiva");
    ok &= comprobar(!ReglasMovimiento::colisionaConSerpiente(true, true, false),
                    "la cola puede abandonarse sin crecimiento");
    ok &= comprobar(ReglasMovimiento::colisionaConSerpiente(true, true, true),
                    "la cola colisiona cuando hay crecimiento");
    return ok ? 0 : 1;
}
