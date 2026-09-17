#ifndef SKINS_H
#define SKINS_H

#include <QImage>
#include <QPixmap>
#include <QRect>
#include <QString>

namespace Skins
{
inline QString rutaCabeza(const QString &skin)
{
    if (skin == "gato") return ":/assets/skin_gato.png";
    if (skin == "dragon") return ":/assets/skin_dragon.png";
    if (skin == "burro") return ":/assets/skin_burro.png";
    if (skin == "thanos") return ":/assets/skin_thanos.png";
    if (skin == "spiderman") return ":/assets/skin_spiderman.png";
    if (skin == "miles") return ":/assets/skin_miles.png";
    if (skin == "personaje") return ":/assets/skin_personaje.png";
    return ":/assets/cabeza_snake.png";
}

inline QPixmap cabeza(const QString &skin, int tamano)
{
    const QPixmap original(rutaCabeza(skin));
    if (original.isNull()) return {};

    const QImage imagen = original.toImage().convertToFormat(QImage::Format_ARGB32);
    QRect limites;
    for (int y = 0; y < imagen.height(); ++y) {
        for (int x = 0; x < imagen.width(); ++x) {
            if (imagen.pixelColor(x, y).alpha() > 5) {
                limites |= QRect(x, y, 1, 1);
            }
        }
    }

    const QImage recorte = limites.isValid() ? imagen.copy(limites) : imagen;
    return QPixmap::fromImage(recorte).scaled(
        tamano, tamano, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}
}

#endif // SKINS_H
