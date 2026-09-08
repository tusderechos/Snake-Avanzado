#include "ranking.h"
#include "gestorusuarios.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QWidget>

Ranking::Ranking(QObject *parent)
    : QGraphicsScene(parent)
    , botonVolver(nullptr)
{
    for (int i = 0; i < CANTIDAD_POSICIONES; i++)
    {
        panelesFila[i] = nullptr;
        etiquetasPosicion[i] = nullptr;
        etiquetasUsuario[i] = nullptr;
        etiquetasPuntos[i] = nullptr;
    }

    setSceneRect(0, 0, 1254, 1254);

    construirInterfaz();
    actualizarRanking();
}

void Ranking::construirInterfaz()
{
    // =====================================================
    // FONDO
    // =====================================================

    QPixmap fondoOriginal(
        ":/imagenes/imagenes/Ranking.png"
        );

    QPixmap fondoAjustado =
        fondoOriginal.scaled(
            1254,
            1254,
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
            );

    QGraphicsPixmapItem *fondo =
        addPixmap(fondoAjustado);

    fondo->setPos(0, 0);
    fondo->setZValue(0);

    // =====================================================
    // TÍTULO
    // =====================================================

    QLabel *titulo = new QLabel("RANKING");

    titulo->setFixedSize(760, 100);
    titulo->setAlignment(Qt::AlignCenter);

    titulo->setStyleSheet(
        "QLabel {"
        "   background-color: rgba(25, 15, 8, 225);"
        "   color: white;"
        "   border: 6px solid #d5a52e;"
        "   border-radius: 12px;"
        "   font-family: 'Georgia';"
        "   font-size: 52px;"
        "   font-weight: bold;"
        "}"
        );

    QGraphicsProxyWidget *proxyTitulo =
        addWidget(titulo);

    proxyTitulo->setPos(247, 150);
    proxyTitulo->setZValue(2);

    // =====================================================
    // ENCABEZADOS DE LA TABLA
    // =====================================================

    QWidget *encabezado = new QWidget;

    encabezado->setFixedSize(760, 75);
    encabezado->setAttribute(
        Qt::WA_StyledBackground,
        true
        );

    encabezado->setStyleSheet(
        "QWidget {"
        "   background-color: rgba(35, 20, 10, 235);"
        "   border: 4px solid #d5a52e;"
        "}"
        "QLabel {"
        "   background-color: transparent;"
        "   border: none;"
        "   color: white;"
        "   font-family: 'Georgia';"
        "   font-size: 25px;"
        "   font-weight: bold;"
        "}"
        );

    QLabel *tituloPosicion =
        new QLabel("NO.", encabezado);

    QLabel *tituloUsuario =
        new QLabel("USUARIO", encabezado);

    QLabel *tituloPuntos =
        new QLabel("PTS.", encabezado);

    tituloPosicion->setGeometry(
        10, 5, 130, 65
        );

    tituloUsuario->setGeometry(
        150, 5, 400, 65
        );

    tituloPuntos->setGeometry(
        560, 5, 180, 65
        );

    tituloPosicion->setAlignment(
        Qt::AlignCenter
        );

    tituloUsuario->setAlignment(
        Qt::AlignCenter
        );

    tituloPuntos->setAlignment(
        Qt::AlignCenter
        );

    QGraphicsProxyWidget *proxyEncabezado =
        addWidget(encabezado);

    proxyEncabezado->setPos(247, 275);
    proxyEncabezado->setZValue(2);

    // =====================================================
    // CINCO FILAS DEL RANKING
    // =====================================================

    const qreal posicionInicialY = 360;
    const qreal separacionFilas = 88;

    for (int i = 0; i < CANTIDAD_POSICIONES; i++)
    {
        panelesFila[i] = new QWidget;

        panelesFila[i]->setFixedSize(
            760,
            78
            );

        panelesFila[i]->setAttribute(
            Qt::WA_StyledBackground,
            true
            );

        etiquetasPosicion[i] =
            new QLabel(panelesFila[i]);

        etiquetasUsuario[i] =
            new QLabel(panelesFila[i]);

        etiquetasPuntos[i] =
            new QLabel(panelesFila[i]);

        etiquetasPosicion[i]->setGeometry(
            10, 5, 130, 68
            );

        etiquetasUsuario[i]->setGeometry(
            150, 5, 400, 68
            );

        etiquetasPuntos[i]->setGeometry(
            560, 5, 180, 68
            );

        etiquetasPosicion[i]->setAlignment(
            Qt::AlignCenter
            );

        etiquetasUsuario[i]->setAlignment(
            Qt::AlignCenter
            );

        etiquetasPuntos[i]->setAlignment(
            Qt::AlignCenter
            );

        etiquetasPosicion[i]->setText(
            QString::number(i + 1)
            );

        configurarEstiloFila(i);

        QGraphicsProxyWidget *proxyFila =
            addWidget(panelesFila[i]);

        proxyFila->setPos(
            247,
            posicionInicialY
                + i * separacionFilas
            );

        proxyFila->setZValue(2);
    }

    // =====================================================
    // BOTÓN VOLVER
    // =====================================================

    botonVolver = new QPushButton("Volver");

    botonVolver->setFixedSize(320, 85);

    botonVolver->setCursor(
        Qt::PointingHandCursor
        );

    botonVolver->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(55, 145, 20, 230);"
        "   color: white;"
        "   border: 5px solid #39ff14;"
        "   border-radius: 12px;"
        "   font-size: 30px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(85, 190, 25, 240);"
        "   border-color: #b6ff00;"
        "}"
        "QPushButton:pressed {"
        "   background-color: rgba(35, 110, 10, 240);"
        "}"
        );

    QGraphicsProxyWidget *proxyVolver =
        addWidget(botonVolver);

    proxyVolver->setPos(467, 850);
    proxyVolver->setZValue(2);

    connect(
        botonVolver,
        &QPushButton::clicked,
        this,
        [this]()
        {
            emit volverSolicitado();
        }
        );

    setSceneRect(0, 0, 1254, 1254);
}

void Ranking::configurarEstiloFila(int indice)
{
    QString colorFondo;
    QString colorBorde;
    QString colorTexto;

    if (indice == 0)
    {
        // Primer lugar: oro
        colorFondo =
            "rgba(212, 175, 55, 235)";

        colorBorde = "#fff176";
        colorTexto = "#241600";
    }
    else if (indice == 1)
    {
        // Segundo lugar: plata
        colorFondo =
            "rgba(192, 192, 192, 235)";

        colorBorde = "#f5f5f5";
        colorTexto = "#202020";
    }
    else if (indice == 2)
    {
        // Tercer lugar: bronce
        colorFondo =
            "rgba(205, 127, 50, 235)";

        colorBorde = "#efb06a";
        colorTexto = "#271204";
    }
    else
    {
        // Cuarto y quinto lugar
        colorFondo =
            "rgba(25, 18, 12, 220)";

        colorBorde = "#806040";
        colorTexto = "white";
    }

    QString estilo =
        "QWidget {"
        "   background-color: "
        + colorFondo
        + ";"
          "   border: 3px solid "
        + colorBorde
        + ";"
          "}"
          "QLabel {"
          "   background-color: transparent;"
          "   border: none;"
          "   color: "
        + colorTexto
        + ";"
          "   font-family: 'Georgia';"
          "   font-size: 27px;"
          "   font-weight: bold;"
          "}";

    panelesFila[indice]->setStyleSheet(
        estilo
        );
}

void Ranking::actualizarRanking()
{
    QVector<GestorUsuarios::DatoRanking> datos =
        GestorUsuarios::obtenerRanking(5);

    for (int i = 0; i < CANTIDAD_POSICIONES; i++)
    {
        etiquetasPosicion[i]->setText(
            QString::number(i + 1)
            );

        if (i < datos.size())
        {
            etiquetasUsuario[i]->setText(
                datos[i].usuario
                );

            etiquetasPuntos[i]->setText(
                QString::number(datos[i].puntos)
                );
        }
        else
        {
            etiquetasUsuario[i]->setText("---");
            etiquetasPuntos[i]->setText("0");
        }
    }
}