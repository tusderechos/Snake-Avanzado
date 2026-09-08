#ifndef GESTORCONFIGURACION_H
#define GESTORCONFIGURACION_H

#include <QString>

class GestorConfiguracion
{
public:
    static void cargarVolumen(
        const QString &usuario,
        int &volumenMusica,
        int &volumenSonido
        );

    static bool guardarVolumen(
        const QString &usuario,
        int volumenMusica,
        int volumenSonido
        );

    static QString cargarControl(
        const QString &usuario
        );

    static bool guardarControl(
        const QString &usuario,
        const QString &control
        );

private:
    static QString obtenerRutaArchivo();
};

#endif // GESTORCONFIGURACION_H
