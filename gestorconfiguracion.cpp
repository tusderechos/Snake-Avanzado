#include "gestorconfiguracion.h"
#include "gestorusuarios.h"
#include <QtMath>

void GestorConfiguracion::cargarVolumen(const QString &usuario, int &musica, int &sonido)
{
    const auto datos = GestorUsuarios::perfil(usuario);
    musica = qRound(datos.value("volumen_musica").toDouble(0.75) * 100);
    sonido = qRound(datos.value("volumen_sonido").toDouble(0.75) * 100);
}
bool GestorConfiguracion::guardarVolumen(const QString &usuario, int musica, int sonido)
{
    return GestorUsuarios::guardarPreferencias(usuario, {
        {"volumen_musica", qBound(0, musica, 100) / 100.0},
        {"volumen_sonido", qBound(0, sonido, 100) / 100.0}});
}
QString GestorConfiguracion::cargarControl(const QString &usuario)
{
    return GestorUsuarios::perfil(usuario).value("control").toString("FLECHAS");
}
bool GestorConfiguracion::guardarControl(const QString &usuario, const QString &control)
{
    return GestorUsuarios::guardarPreferencias(usuario, {{"control", control}});
}
