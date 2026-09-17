#ifndef MENUJUEGO_H
#define MENUJUEGO_H

#include "modojuego.h"

#include <QWidget>

class QPushButton;

#include <functional>

class MenuJuego : public QWidget {
public:
    explicit MenuJuego(std::function<void(ModoJuego)> alSeleccionar,
                       QWidget *parent = nullptr);

    void establecerTutorialDisponible(bool disponible);

private:
    std::function<void(ModoJuego)> m_alSeleccionar;
    QPushButton *m_botonTutorial;
};

#endif
