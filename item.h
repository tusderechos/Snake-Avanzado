#ifndef ITEM_H
#define ITEM_H

enum class TipoItem { Reloj, Hielo, Rayo, Tijeras, Bomba, Trampa };

class Item {
public:
    explicit Item(TipoItem tipo);

    TipoItem tipo() const;
    int tiempoExtraSegundos() const;
    int duracionTurnos() const;
    int puntos() const;
    int segmentosDelta() const;

private:
    TipoItem m_tipo;
};

#endif
