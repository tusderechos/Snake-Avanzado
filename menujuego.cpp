#include "menujuego.h"

#include <QFont>
#include <QLabel>
#include <QFontDatabase>
#include <QPushButton>
#include <QVBoxLayout>

#include <utility>

MenuJuego::MenuJuego(std::function<void(ModoJuego)> alSeleccionar,
                     QWidget *parent)
    : QWidget(parent), m_alSeleccionar(std::move(alSeleccionar)) {
    setWindowTitle("Snake - Menú de Juego");
    setFixedSize(520, 560);
    setStyleSheet(
        "QWidget { background: #0d121a; color: #ebf0f5; }"
        "QPushButton { background: #182536; border: 1px solid #405875;"
        " border-radius: 10px; padding: 14px; color: #ebf0f5;"
        " font-size: 16px; }"
        "QPushButton:hover { background: #24405d; }"
        "QPushButton:pressed { background: #315b7e; }");
    QFontDatabase::addApplicationFont(":/assets/Fredoka-Variable.ttf");
    setFont(QFont("Fredoka", 14));

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(70, 55, 70, 55);
    layout->setSpacing(16);

    auto *titulo = new QLabel("SNAKE AVANZADO", this);
    titulo->setAlignment(Qt::AlignCenter);
    titulo->setFont(QFont("Fredoka", 26, QFont::Bold));
    layout->addWidget(titulo);

    auto *subtitulo = new QLabel("MENÚ DE JUEGO", this);
    subtitulo->setAlignment(Qt::AlignCenter);
    subtitulo->setFont(QFont("Fredoka", 18, QFont::DemiBold));
    subtitulo->setStyleSheet("color: #7bdc66;");
    layout->addWidget(subtitulo);
    layout->addSpacing(20);

    const struct Opcion { const char *texto; ModoJuego modo; } opciones[] = {
        {"TUTORIAL", ModoJuego::Tutorial},
        {"NORMAL", ModoJuego::Normal},
        {"LIBRE", ModoJuego::Libre},
        {"ALEATORIO", ModoJuego::Aleatorio}
    };

    for (const Opcion &opcion : opciones) {
        auto *boton = new QPushButton(opcion.texto, this);
        layout->addWidget(boton);
        connect(boton, &QPushButton::clicked, this, [this, modo = opcion.modo]() {
            if (m_alSeleccionar) {
                m_alSeleccionar(modo);
            }
        });
    }

    auto *nota = new QLabel("Elegí un modo para comenzar", this);
    nota->setAlignment(Qt::AlignCenter);
    nota->setStyleSheet("color: #9aa9b8; margin-top: 18px;");
    layout->addWidget(nota);
}
