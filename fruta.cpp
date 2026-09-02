#include "fruta.h"

Fruta::Fruta() : m_tipo(TipoFruta::Normal) {}

void Fruta::configurar(TipoFruta tipo) { m_tipo = tipo; }
TipoFruta Fruta::tipo() const { return m_tipo; }
int Fruta::puntos() const {
    switch (m_tipo) {
    case TipoFruta::Dorada: return 30;
    case TipoFruta::Grande: return 20;
    case TipoFruta::Energetica: return 15;
    case TipoFruta::Normal: return 10;
    }
    return 10;
}

int Fruta::frutasContadas() const { return m_tipo == TipoFruta::Dorada ? 2 : 1; }

int Fruta::crecimiento() const {
    return m_tipo == TipoFruta::Dorada || m_tipo == TipoFruta::Grande ? 2 : 1;
}

int Fruta::crecimiento(bool dobleEnNivel3) const {
    return dobleEnNivel3 ? 2 : crecimiento();
}
