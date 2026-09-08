#ifndef TIENDA_H
#define TIENDA_H

#include <QGraphicsScene>
#include <QVector>
#include <QString>

class QLabel;
class QPushButton;

class Tienda : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit Tienda(QObject *parent = nullptr);
    void establecerUsuario(const QString &usuario);

signals:
    void volverSolicitado();

private:
    struct TarjetaSkin {
        QString id;
        int precio;
        QPushButton *boton = nullptr;
        QLabel *estado = nullptr;
    };

    void construirInterfaz();
    void actualizarTienda();
    void actualizarTarjeta(TarjetaSkin &tarjeta);

    QString usuarioActual;
    QLabel *etiquetaMonedas;
    QVector<TarjetaSkin> tarjetas;
};

#endif // TIENDA_H
