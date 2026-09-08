#ifndef CONTROLESVIEW_H
#define CONTROLESVIEW_H

#include <QGraphicsScene>
#include <QString>

class QLabel;
class QPushButton;

class ControlesView : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit ControlesView(QObject *parent = nullptr);
    void establecerUsuario(const QString &usuario);

signals:
    void volverSolicitado();

private slots:
    void seleccionarWasd();
    void seleccionarFlechas();

private:
    QString usuarioActual;
    QString controlActual;
    QPushButton *botonWasd;
    QPushButton *botonFlechas;
    QLabel *estadoWasd;
    QLabel *estadoFlechas;

    void construirInterfaz();
    void actualizarSeleccion(const QString &control);
    QString estiloOpcion(bool activa) const;
};

#endif // CONTROLESVIEW_H
