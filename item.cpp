#include "item.h"

Item::Item(TipoItem tipo) : m_tipo(tipo) {}

TipoItem Item::tipo() const { return m_tipo; }

int Item::tiempoExtraSegundos() const {
    return m_tipo == TipoItem::Reloj ? 10 : 0;
}

int Item::duracionTurnos() const {
    return m_tipo == TipoItem::Hielo || m_tipo == TipoItem::Trampa ? 10 : 0;
}

int Item::puntos() const {
    return m_tipo == TipoItem::Rayo ? 30 : m_tipo == TipoItem::Bomba ? -20 : 0;
}

int Item::segmentosDelta() const {
    return m_tipo == TipoItem::Tijeras ? -2 : m_tipo == TipoItem::Bomba ? -1 : 0;
}
