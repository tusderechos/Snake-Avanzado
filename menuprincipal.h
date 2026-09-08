#ifndef MENUPRINCIPAL_H
#define MENUPRINCIPAL_H

#include <QGraphicsScene>

class QPushButton;

class MenuPrincipal : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit MenuPrincipal(
        QObject *parent = nullptr
        );

signals:
    void jugarSolicitado();
    void rankingSolicitado();
    void tiendaSolicitada();
    void configuracionSolicitada();
    void salirSolicitado();

private:
    // Construye el fondo y los cinco botones.
    void construirInterfaz();

    // Crea un botón invisible encima del botón dibujado.
    QPushButton *crearBoton(
        const QString &texto,
        qreal x,
        qreal y,
        qreal ancho,
        qreal alto
        );
};

#endif // MENUPRINCIPAL_H