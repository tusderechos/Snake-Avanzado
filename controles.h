#ifndef CONTROLES_H
#define CONTROLES_H

#include <QtCore/Qt>

enum class EsquemaControles { Flechas, WASD };

class Controles {
public:
    static EsquemaControles detectar(int tecla);
    static bool esMovimiento(int tecla);
};

#endif
