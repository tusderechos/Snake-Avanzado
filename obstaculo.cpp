#include "obstaculo.h"

Obstaculo::Obstaculo() : m_x(0), m_y(0), m_direccionX(0), m_direccionY(0), m_movil(false) {}
void Obstaculo::configurar(int x, int y, int direccionX, int direccionY, bool movil) {
    m_x = x; m_y = y; m_direccionX = direccionX; m_direccionY = direccionY; m_movil = movil;
}
int Obstaculo::x() const { return m_x; }
int Obstaculo::y() const { return m_y; }
int Obstaculo::direccionX() const { return m_direccionX; }
int Obstaculo::direccionY() const { return m_direccionY; }
bool Obstaculo::esMovil() const { return m_movil; }
void Obstaculo::moverA(int x, int y) { m_x = x; m_y = y; }
void Obstaculo::invertirDireccion() { m_direccionX *= -1; m_direccionY *= -1; }
