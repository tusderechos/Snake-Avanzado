#ifndef OBSTACULO_H
#define OBSTACULO_H

class Obstaculo {
public:
    Obstaculo();
    void configurar(int x, int y, int direccionX, int direccionY, bool movil);
    int x() const;
    int y() const;
    int direccionX() const;
    int direccionY() const;
    bool esMovil() const;
    void moverA(int x, int y);
    void invertirDireccion();

private:
    int m_x;
    int m_y;
    int m_direccionX;
    int m_direccionY;
    bool m_movil;
};

#endif
