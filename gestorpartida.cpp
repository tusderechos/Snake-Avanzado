#include "gestorpartida.h"

GestorPartida::GestorPartida() : m_completada(false) {}
void GestorPartida::reiniciar() { m_completada = false; }
void GestorPartida::marcarCompletada() { m_completada = true; }
bool GestorPartida::estaCompletada() const { return m_completada; }
