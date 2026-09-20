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

    void establecerManualVisible(bool visible);

signals:
    void jugarSolicitado();
    void rankingSolicitado();
    void tiendaSolicitada();
    void configuracionSolicitada();
    void salirSolicitado();
    void manualSolicitado();

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
    QPushButton *m_botonManual = nullptr;
};

#endif // MENUPRINCIPAL_H
