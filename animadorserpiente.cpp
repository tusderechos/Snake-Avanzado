#include "animadorserpiente.h"

#include <QTimer>

AnimadorSerpiente::AnimadorSerpiente(QObject *parent)
    : QObject(parent),
      m_temporizador(nullptr)
{
}

void AnimadorSerpiente::iniciar(int intervalo)
{
    if (m_temporizador == nullptr) {
        m_temporizador = new QTimer(this);
        connect(m_temporizador, &QTimer::timeout, this,
                &AnimadorSerpiente::tick);
    }
    m_temporizador->start(intervalo);
}

void AnimadorSerpiente::detener()
{
    if (m_temporizador != nullptr) m_temporizador->stop();
}

void AnimadorSerpiente::cambiarIntervalo(int intervalo)
{
    if (m_temporizador != nullptr) m_temporizador->setInterval(intervalo);
}
