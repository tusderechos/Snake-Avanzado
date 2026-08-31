#include "snake.h"

Snake::Snake()
    : m_cabeza(nullptr), m_cola(nullptr), m_longitud(0)
{
}

Snake::~Snake() {
    limpiar();
}

void Snake::insertarCabeza(int x, int y) {
    Nodo *nuevoNodo = new Nodo(x, y);
    nuevoNodo->siguiente = m_cabeza;
    m_cabeza = nuevoNodo;

    if (m_cola == nullptr) {
        m_cola = nuevoNodo;
    }

    m_longitud++;
}

void Snake::avanzar(int x, int y, bool crecer) {
    if (m_cabeza == nullptr || crecer) {
        insertarCabeza(x, y);
        return;
    }

    if (m_cabeza->siguiente == nullptr) {
        m_cabeza->x = x;
        m_cabeza->y = y;
        m_cola = m_cabeza;
        return;
    }

    Nodo *antesDeCola = m_cabeza;

    while (antesDeCola->siguiente->siguiente != nullptr) {
        antesDeCola = antesDeCola->siguiente;
    }

    Nodo *cola = antesDeCola->siguiente;
    antesDeCola->siguiente = nullptr;
    m_cola = antesDeCola;
    cola->x = x;
    cola->y = y;
    cola->siguiente = m_cabeza;
    m_cabeza = cola;
}

void Snake::limpiar() {
    while (m_cabeza != nullptr) {
        Nodo *nodoAEliminar = m_cabeza;
        m_cabeza = m_cabeza->siguiente;
        delete nodoAEliminar;
    }

    m_cola = nullptr;
    m_longitud = 0;
}

int Snake::cabezaX() const {
    return m_cabeza == nullptr ? -1 : m_cabeza->x;
}

int Snake::cabezaY() const {
    return m_cabeza == nullptr ? -1 : m_cabeza->y;
}

int Snake::longitud() const {
    return m_longitud;
}

bool Snake::ocupa(int x, int y) const {
    const Nodo *actual = m_cabeza;
    while (actual != nullptr) {
        if (actual->x == x && actual->y == y) {
            return true;
        }
        actual = actual->siguiente;
    }

    return false;
}

bool Snake::ocupaCola(int x, int y) const {
    return m_cola != nullptr && m_cola->x == x && m_cola->y == y;
}

const Nodo *Snake::cabeza() const {
    return m_cabeza;
}
