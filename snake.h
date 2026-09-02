#ifndef SNAKE_H
#define SNAKE_H

#include "Nodo.h"

class Snake {
public:
    Snake();
    ~Snake();
    Snake(const Snake &otro) = delete;
    Snake &operator=(const Snake &otro) = delete;

    void insertarCabeza(int x, int y);
    void avanzar(int x, int y, int segmentosCrecimiento);
    void reducirSegmentos(int cantidad);
    void limpiar();

    int cabezaX() const;
    int cabezaY() const;
    int longitud() const;
    bool tieneCrecimientoPendiente() const;
    bool ocupa(int x, int y) const;
    bool ocupaCola(int x, int y) const;
    const Nodo *cabeza() const;

private:
    Nodo *m_cabeza;
    Nodo *m_cola;
    int m_longitud;
    int m_crecimientoPendiente;
};

#endif // SNAKE_H
