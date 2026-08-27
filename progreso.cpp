#include "progreso.h"

ProgresoNivel::ProgresoNivel(int metaFrutas, int metaPuntos, int metaLongitud)
    : m_metaFrutas(metaFrutas),
      m_metaPuntos(metaPuntos),
      m_metaLongitud(metaLongitud),
      m_frutasComidas(0),
      m_puntaje(0),
      m_longitudActual(0)
{
}

void ProgresoNivel::reiniciar()
{
    m_frutasComidas = 0;
    m_puntaje = 0;
    m_longitudActual = 0;
}

void ProgresoNivel::registrarFruta(int puntosObtenidos, int longitudActual)
{
    ++m_frutasComidas;
    m_puntaje += puntosObtenidos;
    m_longitudActual = longitudActual;
}

bool ProgresoNivel::gano() const
{
    return m_frutasComidas >= m_metaFrutas
           && m_puntaje >= m_metaPuntos
           && m_longitudActual >= m_metaLongitud;
}

int ProgresoNivel::frutasComidas() const
{
    return m_frutasComidas;
}

int ProgresoNivel::puntaje() const
{
    return m_puntaje;
}
