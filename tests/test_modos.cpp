#include "../modojuego.h"
#include "../controles.h"

#include <cassert>

int main() {
    assert(static_cast<int>(ModoJuego::Tutorial) == 0);
    assert(static_cast<int>(ModoJuego::Normal) == 1);
    assert(static_cast<int>(ModoJuego::Libre) == 2);
    assert(static_cast<int>(ModoJuego::Aleatorio) == 3);
    assert(Controles::detectar(Qt::Key_W) == EsquemaControles::WASD);
    assert(Controles::detectar(Qt::Key_Left) == EsquemaControles::Flechas);
    assert(Controles::esMovimiento(Qt::Key_Down));
}
