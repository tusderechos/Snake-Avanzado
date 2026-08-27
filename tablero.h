#ifndef TABLERO_H
#define TABLERO_H

class Tablero {
public:
    Tablero(int columnas, int filas);
    ~Tablero();
    Tablero(const Tablero &otro) = delete;
    Tablero &operator=(const Tablero &otro) = delete;

    void limpiar();
    bool coordenadaValida(int x, int y) const;
    int valor(int x, int y) const;
    void poner(int x, int y, int valor);
    int columnas() const;
    int filas() const;

private:
    int **m_mapa;
    int m_columnas;
    int m_filas;
};

#endif // TABLERO_H
