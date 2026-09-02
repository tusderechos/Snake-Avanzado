#include "snake.h"

Snake::Snake()
    : m_cabeza(nullptr), m_cola(nullptr), m_longitud(0), m_crecimientoPendiente(0)
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

void Snake::avanzar(int x, int y, int segmentosCrecimiento) {
    if (m_cabeza == nullptr) {
        insertarCabeza(x, y);
        return;
    }

    m_crecimientoPendiente += segmentosCrecimiento > 0 ? segmentosCrecimiento : 0;
    Nodo *nuevaCabeza = new Nodo(x, y);
    nuevaCabeza->siguiente = m_cabeza;
    m_cabeza = nuevaCabeza;
    ++m_longitud;

    if (m_crecimientoPendiente > 0) {
        --m_crecimientoPendiente;
        return;
    }

    Nodo *antesDeCola = m_cabeza;
    while (antesDeCola->siguiente != m_cola) {
        antesDeCola = antesDeCola->siguiente;
    }

    delete m_cola;
    m_cola = antesDeCola;
    m_cola->siguiente = nullptr;
    --m_longitud;
}

void Snake::reducirSegmentos(int cantidad) {
    while (cantidad > 0 && m_longitud > 3) {
        Nodo *antesDeCola = m_cabeza;
        while (antesDeCola->siguiente != m_cola) {
            antesDeCola = antesDeCola->siguiente;
        }
        delete m_cola;
        m_cola = antesDeCola;
        m_cola->siguiente = nullptr;
        --m_longitud;
        --cantidad;
    }
}

void Snake::limpiar() {
    while (m_cabeza != nullptr) {
        Nodo *nodoAEliminar = m_cabeza;
        m_cabeza = m_cabeza->siguiente;
        delete nodoAEliminar;
    }

    m_cola = nullptr;
    m_longitud = 0;
    m_crecimientoPendiente = 0;
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

bool Snake::tieneCrecimientoPendiente() const {
    return m_crecimientoPendiente > 0;
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
