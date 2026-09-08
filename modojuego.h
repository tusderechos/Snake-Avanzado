#ifndef MODOJUEGO_H
#define MODOJUEGO_H

enum class ModoJuego { Tutorial, Normal, Libre, Aleatorio };

struct ConfiguracionJuego {
    bool frutasEspeciales = true;
    bool items = true;
    bool obstaculos = true;
    bool obstaculosMoviles = true;
    bool aparicionAleatoria = true;
    bool progresionAutomatica = true;
    bool esAleatorio = false;
};

#endif
