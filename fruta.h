#ifndef FRUTA_H
#define FRUTA_H

enum class TipoFruta { Normal, Dorada, Grande, Energetica };

class Fruta {
public:
    Fruta();
    void configurar(TipoFruta tipo);
    TipoFruta tipo() const;
    int puntos() const;
    int frutasContadas() const;
    int crecimiento() const;
    int crecimiento(bool dobleEnNivel3) const;

private:
    TipoFruta m_tipo;
};

#endif
