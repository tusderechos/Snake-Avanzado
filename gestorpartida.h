#ifndef GESTORPARTIDA_H
#define GESTORPARTIDA_H

class GestorPartida {
public:
    GestorPartida();
    void reiniciar();
    void marcarCompletada();
    bool estaCompletada() const;

private:
    bool m_completada;
};

#endif
