#ifndef PROGRESO_H
#define PROGRESO_H

class ProgresoNivel
{
public:
    ProgresoNivel(int metaFrutas, int metaPuntos, int metaLongitud);

    void reiniciar();
    void registrarFruta(int puntosObtenidos, int longitudActual);
    bool gano() const;
    int frutasComidas() const;
    int puntaje() const;

private:
    int m_metaFrutas;
    int m_metaPuntos;
    int m_metaLongitud;
    int m_frutasComidas;
    int m_puntaje;
    int m_longitudActual;
};

#endif // PROGRESO_H
