#include "tablero.h"

Tablero::Tablero(int columnas, int filas)
    : m_mapa(nullptr),
      m_columnas(columnas > 0 ? columnas : 1),
      m_filas(filas > 0 ? filas : 1)
{
    m_mapa = new int *[m_filas];
    for (int y = 0; y < m_filas; ++y) {
        m_mapa[y] = new int[m_columnas];
    }
    limpiar();
}

Tablero::~Tablero() {
    if (m_mapa != nullptr) {
        for (int y = 0; y < m_filas; ++y) {
            delete[] m_mapa[y];
            m_mapa[y] = nullptr;
        }
    }

    delete[] m_mapa;
    m_mapa = nullptr;
}

void Tablero::limpiar() {
    if (m_mapa == nullptr) {
        return;
    }

    for (int y = 0; y < m_filas; ++y) {
        for (int x = 0; x < m_columnas; ++x) {
            m_mapa[y][x] = 0;
        }
    }
}

bool Tablero::coordenadaValida(int x, int y) const {
    return x >= 0 && x < m_columnas && y >= 0 && y < m_filas;
}

int Tablero::valor(int x, int y) const {
    if (!coordenadaValida(x, y)) {
        return -1;
    }

    return m_mapa[y][x];
}

void Tablero::poner(int x, int y, int valor) {
    if (!coordenadaValida(x, y)) {
        return;
    }

    m_mapa[y][x] = valor;
}

int Tablero::columnas() const {
    return m_columnas;
}

int Tablero::filas() const {
    return m_filas;
}
