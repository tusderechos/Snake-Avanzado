#include "tablero.h"

#include <new>

Tablero::Tablero(int columnas, int filas)
    : m_mapa(nullptr),
      m_columnas(columnas > 0 ? columnas : 1),
      m_filas(filas > 0 ? filas : 1)
{
    reservar(m_columnas, m_filas);
    limpiar();
}

Tablero::~Tablero() {
    liberar();
}

void Tablero::reservar(int columnas, int filas) {
    int **nuevoMapa = new int *[filas]();

    try {
        for (int y = 0; y < filas; ++y) {
            nuevoMapa[y] = new int[columnas]();
        }
    } catch (...) {
        for (int y = 0; y < filas; ++y) {
            delete[] nuevoMapa[y];
        }
        delete[] nuevoMapa;
        throw;
    }

    m_mapa = nuevoMapa;
}

void Tablero::liberar() {
    if (m_mapa == nullptr) {
        return;
    }

    for (int y = 0; y < m_filas; ++y) {
        delete[] m_mapa[y];
        m_mapa[y] = nullptr;
    }

    delete[] m_mapa;
    m_mapa = nullptr;
}

void Tablero::redimensionar(int columnas, int filas) {
    const int nuevasColumnas = columnas > 0 ? columnas : 1;
    const int nuevasFilas = filas > 0 ? filas : 1;

    if (nuevasColumnas == m_columnas && nuevasFilas == m_filas) {
        limpiar();
        return;
    }

    int **mapaAnterior = m_mapa;
    const int columnasAnteriores = m_columnas;
    const int filasAnteriores = m_filas;

    m_mapa = nullptr;
    m_columnas = nuevasColumnas;
    m_filas = nuevasFilas;

    try {
        reservar(m_columnas, m_filas);
    } catch (...) {
        m_mapa = mapaAnterior;
        m_columnas = columnasAnteriores;
        m_filas = filasAnteriores;
        throw;
    }

    for (int y = 0; y < filasAnteriores; ++y) {
        delete[] mapaAnterior[y];
    }
    delete[] mapaAnterior;
    limpiar();
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
