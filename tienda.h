#ifndef TIENDA_H
#define TIENDA_H

#include <QGraphicsScene>

class Tienda : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit Tienda(QObject *parent = nullptr);

signals:
    void volverSolicitado();

private:
    void construirInterfaz();
};

#endif // TIENDA_H
