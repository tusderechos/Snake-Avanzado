#include "../fruta.h"
#include "../item.h"
#include "../progreso.h"
#include "../snake.h"
#include "../reglasmovimiento.h"

#include <cassert>

int main() {
    Fruta normal;
    normal.configurar(TipoFruta::Normal);
    assert(normal.puntos() == 10);
    assert(normal.frutasContadas() == 1);
    assert(normal.crecimiento() == 1);
    assert(normal.crecimiento(true) == 2);

    Fruta dorada;
    dorada.configurar(TipoFruta::Dorada);
    assert(dorada.puntos() == 30);
    assert(dorada.frutasContadas() == 2);
    assert(dorada.crecimiento() == 2);

    Fruta grande;
    grande.configurar(TipoFruta::Grande);
    assert(grande.puntos() == 20);
    assert(grande.crecimiento() == 2);

    Fruta energetica;
    energetica.configurar(TipoFruta::Energetica);
    assert(energetica.puntos() == 15);
    assert(energetica.crecimiento() == 1);
    assert(energetica.crecimiento(true) == 2);

    assert(Item(TipoItem::Reloj).tiempoExtraSegundos() == 10);
    assert(Item(TipoItem::Hielo).duracionTurnos() == 10);
    assert(Item(TipoItem::Rayo).puntos() == 30);
    assert(Item(TipoItem::Tijeras).segmentosDelta() == -2);
    assert(Item(TipoItem::Bomba).puntos() == -20);
    assert(Item(TipoItem::Trampa).duracionTurnos() == 10);

    Snake snake;
    snake.insertarCabeza(1, 1);
    snake.insertarCabeza(2, 1);
    snake.insertarCabeza(3, 1);
    snake.insertarCabeza(4, 1);
    snake.reducirSegmentos(2);
    assert(snake.longitud() == 3);

    Snake snakeCorta;
    snakeCorta.insertarCabeza(1, 1);
    snakeCorta.avanzar(2, 1, 0);
    assert(snakeCorta.longitud() == 1);

    ProgresoNivel progreso(99, 0, 3);
    progreso.registrarItem(30, 3);
    assert(progreso.puntaje() == 30);

    assert(ReglasMovimiento::colisionaConSerpiente(true, false, false));
    assert(!ReglasMovimiento::colisionaConSerpiente(true, true, false));
    assert(ReglasMovimiento::colisionaConSerpiente(true, true, true));
}
