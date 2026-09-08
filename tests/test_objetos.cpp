#include "fruta.h"
#include "item.h"
#include "progreso.h"
#include "snake.h"

#include <iostream>

namespace {
bool comprobar(bool condicion, const char *mensaje) {
    if (!condicion) std::cerr << "Fallo: " << mensaje << '\n';
    return condicion;
}
}

int main() {
    bool ok = true;
    Fruta dorada;
    dorada.configurar(TipoFruta::Dorada);
    ok &= comprobar(dorada.puntos() == 30, "puntos de fruta dorada");
    ok &= comprobar(dorada.crecimiento() == 2, "crecimiento de fruta dorada");

    Item bomba(TipoItem::Bomba);
    ok &= comprobar(bomba.puntos() == -20, "puntos de bomba");
    ok &= comprobar(bomba.segmentosDelta() == -1, "delta de bomba");

    Snake serpiente;
    serpiente.insertarCabeza(0, 0);
    serpiente.avanzar(1, 0, 0);
    ok &= comprobar(serpiente.longitud() == 1, "serpiente de longitud uno");

    ProgresoNivel progreso(1, 10, 2);
    progreso.registrarFruta(10, 2, 1);
    ok &= comprobar(progreso.gano(), "metas de progreso");
    return ok ? 0 : 1;
}
