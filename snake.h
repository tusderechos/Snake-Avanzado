#ifndef SNAKE_H
#define SNAKE_H

#include "Nodo.h"

class Snake
{
public:
    Snake();
    ~Snake();

    void insertarCabeza(int x, int y);
    void avanzar(int x, int y, bool crecer);
    void limpiar();

    int cabezaX() const;
    int cabezaY() const;
    int longitud() const;
    bool ocupa(int x, int y) const;
    Nodo *cabeza() const;

private:
    Nodo *m_cabeza;
    int m_longitud;
};

#endif // SNAKE_H
