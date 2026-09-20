#include "manualusuario.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

ManualUsuario::ManualUsuario(QWidget *parent)
    : QWidget(parent),
      paginaImagen(new QLabel(this)),
      textoIntro(new QLabel(this)),
      shrekIntro(new QLabel(this)),
      numeroPagina(new QLabel(this)),
      anterior(new QPushButton("‹ ANTERIOR", this)),
      siguiente(new QPushButton("SIGUIENTE ›", this)),
      cerrarManual(new QPushButton("CERRAR", this)),
      escrituraTimer(new QTimer(this)),
      textoIntroduccion(
          "En los confines del pantano, donde la niebla se enreda entre los árboles, "
          "comienza tu aventura en Serpent Society.\n\n"
          "Soy Shrek, y te acompañaré a lo largo de estas páginas. Aquí descubrirás "
          "cómo dar tus primeros pasos, elegir un camino y guiar a tu serpiente. "
          "Aprenderás a reconocer los frutos que crecen en el pantano, a visitar la "
          "tienda y a tener cuidado con los caprichos de la caja misteriosa.\n\n"
          "Abre el libro cuando necesites recordar algo. El sendero te espera.") {
    setWindowTitle("Manual de usuario - Serpent Society");
    setFixedSize(980, 760);
    setStyleSheet(
        "QWidget { background: #0b160b; color: #fff9df; }"
        "QLabel#numero { color: #d2a93b; font-size: 16px; font-weight: bold; }"
        "QPushButton { background: #2f8618; color: white; border: 2px solid #68dd3e;"
        " border-radius: 9px; padding: 10px 18px; font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { background: #4cae22; border-color: #f4d06f; }");

    paginas = {
        ":/assets/manual/00_portada_fondo.webp",
        ":/assets/manual/00_introduccion_fondo.webp",
        ":/assets/manual/02_cuenta.png",
        ":/assets/manual/03_menu_niveles.png",
        ":/assets/manual/04_tienda.png",
        ":/assets/manual/05_ajustes_perfil.png",
        ":/assets/manual/06_controles.png",
        ":/assets/manual/07_frutas.png",
        ":/assets/manual/08_caja_misteriosa.png"
    };

    paginaImagen->setFixedSize(920, 680);
    paginaImagen->setAlignment(Qt::AlignCenter);
    paginaImagen->setStyleSheet("QLabel { background: #172817; border: 3px solid #d2a93b; border-radius: 12px; }");

    auto *capa = new QWidget(this);
    capa->setAttribute(Qt::WA_TransparentForMouseEvents);
    capa->setStyleSheet("background: transparent;");
    capa->setFixedSize(paginaImagen->size());

    textoIntro->setParent(capa);
    textoIntro->setGeometry(255, 75, 410, 350);
    textoIntro->setWordWrap(true);
    textoIntro->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    textoIntro->setStyleSheet("color: #293d25; font: 16px 'Georgia'; background: rgba(249, 239, 210, 205); padding: 14px; border-radius: 8px;");

    shrekIntro->setParent(capa);
    shrekIntro->setGeometry(80, 465, 145, 185);
    shrekIntro->setAlignment(Qt::AlignCenter);
    shrekIntro->setPixmap(QPixmap(":/assets/manual/shrek_manual.png").scaled(
        shrekIntro->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    auto *paginaLayout = new QGridLayout;
    paginaLayout->setContentsMargins(0, 0, 0, 0);
    paginaLayout->addWidget(paginaImagen, 0, 0);
    paginaLayout->addWidget(capa, 0, 0);

    numeroPagina->setObjectName("numero");
    numeroPagina->setAlignment(Qt::AlignCenter);

    auto *navegacion = new QHBoxLayout;
    navegacion->setContentsMargins(20, 0, 20, 0);
    navegacion->addWidget(cerrarManual);
    navegacion->addWidget(anterior);
    navegacion->addWidget(numeroPagina, 1);
    navegacion->addWidget(siguiente);

    auto *principal = new QVBoxLayout(this);
    principal->setContentsMargins(20, 16, 20, 16);
    principal->setSpacing(10);
    principal->addLayout(paginaLayout, 1);
    principal->addLayout(navegacion);

    connect(anterior, &QPushButton::clicked, this, [this]() {
        mostrarPagina(paginaActual - 1);
    });
    connect(cerrarManual, &QPushButton::clicked, this, &ManualUsuario::close);
    connect(siguiente, &QPushButton::clicked, this, [this]() {
        if (paginaActual + 1 < paginas.size()) {
            mostrarPagina(paginaActual + 1);
        } else {
            close();
        }
    });

    escrituraTimer->setInterval(32);
    connect(escrituraTimer, &QTimer::timeout, this, [this]() {
        if (caracteresIntroduccion >= textoIntroduccion.size()) {
            escrituraTimer->stop();
            textoIntro->setText(textoIntroduccion);
            return;
        }
        ++caracteresIntroduccion;
        textoIntro->setText(textoIntroduccion.left(caracteresIntroduccion) + "▌");
    });

    mostrarPagina(0);
}

void ManualUsuario::mostrarPagina(int pagina) {
    paginaActual = qBound(0, pagina, paginas.size() - 1);

    const QPixmap original(paginas.at(paginaActual));
    if (!original.isNull()) {
        paginaImagen->setPixmap(original.scaled(
            paginaImagen->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        paginaImagen->setText("No se pudo cargar esta página del manual.");
        paginaImagen->setPixmap(QPixmap());
    }

    const bool introduccion = paginaActual == 1;
    textoIntro->setVisible(introduccion);
    shrekIntro->setVisible(introduccion);

    if (introduccion) {
        caracteresIntroduccion = 0;
        textoIntro->clear();
        escrituraTimer->start();
    } else {
        escrituraTimer->stop();
    }

    numeroPagina->setText(QString("PÁGINA %1 DE %2")
                              .arg(paginaActual + 1)
                              .arg(paginas.size()));
    anterior->setVisible(paginaActual > 0);
    siguiente->setText("SIGUIENTE ›");
    siguiente->setVisible(paginaActual + 1 < paginas.size());
}
