#ifndef NODO_H
#define NODO_H

struct Nodo {
    int x;
    int y;
    Nodo *siguiente;

    Nodo(int nuevaX, int nuevaY)
        : x(nuevaX), y(nuevaY), siguiente(nullptr)
    {
    }
};

#endif // NODO_H
