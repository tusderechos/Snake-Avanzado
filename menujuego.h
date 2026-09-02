#ifndef MENUJUEGO_H
#define MENUJUEGO_H

#include "modojuego.h"

#include <QWidget>

#include <functional>

class MenuJuego : public QWidget {
public:
    explicit MenuJuego(std::function<void(ModoJuego)> alSeleccionar,
                       QWidget *parent = nullptr);

private:
    std::function<void(ModoJuego)> m_alSeleccionar;
};

#endif
