#ifndef ANIMADORSERPIENTE_H
#define ANIMADORSERPIENTE_H

#include <QObject>

class QTimer;

class AnimadorSerpiente : public QObject
{
    Q_OBJECT

public:
    explicit AnimadorSerpiente(QObject *parent = nullptr);

public slots:
    void iniciar(int intervalo);
    void detener();
    void cambiarIntervalo(int intervalo);

signals:
    void tick();

private:
    QTimer *m_temporizador;
};

#endif // ANIMADORSERPIENTE_H
