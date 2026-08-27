#include "snake.h"

Snake::Snake()
    : m_cabeza(nullptr), m_longitud(0)
{
}

Snake::~Snake()
{
    limpiar();
}

void Snake::insertarCabeza(int x, int y)
{
    Nodo *nuevoNodo = new Nodo(x, y);
    nuevoNodo->siguiente = m_cabeza;
    m_cabeza = nuevoNodo;
    ++m_longitud;
}

void Snake::avanzar(int x, int y, bool crecer)
{
    if (m_cabeza == nullptr || crecer) {
        insertarCabeza(x, y);
        return;
    }

    if (m_cabeza->siguiente == nullptr) {
        m_cabeza->x = x;
        m_cabeza->y = y;
        return;
    }

    Nodo *antesDeCola = m_cabeza;
    while (antesDeCola->siguiente->siguiente != nullptr) {
        antesDeCola = antesDeCola->siguiente;
    }

    Nodo *cola = antesDeCola->siguiente;
    antesDeCola->siguiente = nullptr;
    cola->x = x;
    cola->y = y;
    cola->siguiente = m_cabeza;
    m_cabeza = cola;
}

void Snake::limpiar()
{
    while (m_cabeza != nullptr) {
        Nodo *nodoAEliminar = m_cabeza;
        m_cabeza = m_cabeza->siguiente;
        delete nodoAEliminar;
    }

    m_longitud = 0;
}

int Snake::cabezaX() const
{
    return m_cabeza == nullptr ? -1 : m_cabeza->x;
}

int Snake::cabezaY() const
{
    return m_cabeza == nullptr ? -1 : m_cabeza->y;
}

int Snake::longitud() const
{
    return m_longitud;
}

bool Snake::ocupa(int x, int y) const
{
    Nodo *actual = m_cabeza;
    while (actual != nullptr) {
        if (actual->x == x && actual->y == y) {
            return true;
        }
        actual = actual->siguiente;
    }

    return false;
}

Nodo *Snake::cabeza() const
{
    return m_cabeza;
}
