#include "gestorusuarios.h"

#include <QCoreApplication>

#include <fstream>
#include <string>


QString GestorUsuarios::obtenerRutaArchivo()
{
    // usuarios.txt se guardará junto al ejecutable.
    return QCoreApplication::applicationDirPath()
           + "/usuarios.txt";
}

bool GestorUsuarios::usuarioExiste(
    const QString &usuario
    )
{
    std::ifstream archivo(
        obtenerRutaArchivo().toStdString()
        );

    // Si todavía no existe el archivo,
    // significa que tampoco existen usuarios.
    if (!archivo.is_open())
    {
        return false;
    }

    std::string linea;

    while (std::getline(archivo, linea))
    {
        QString lineaQt =
            QString::fromStdString(linea);

        int posicionSeparador =
            lineaQt.indexOf('|');

        if (posicionSeparador == -1)
        {
            continue;
        }

        QString usuarioGuardado =
            lineaQt.left(posicionSeparador);

        if (usuarioGuardado == usuario)
        {
            return true;
        }
    }

    return false;
}

bool GestorUsuarios::credencialesValidas(
    const QString &usuario,
    const QString &contrasena
    )
{
    std::ifstream archivo(
        obtenerRutaArchivo().toStdString()
        );

    if (!archivo.is_open())
    {
        return false;
    }

    std::string linea;

    while (std::getline(archivo, linea))
    {
        QString lineaQt =
            QString::fromStdString(linea);

        int primerSeparador =
            lineaQt.indexOf('|');

        if (primerSeparador == -1)
        {
            continue;
        }

        int segundoSeparador =
            lineaQt.indexOf(
                '|',
                primerSeparador + 1
                );

        QString usuarioGuardado =
            lineaQt.left(primerSeparador);

        QString contrasenaGuardada;

        // Cuenta sin puntos: usuario|contrasena
        if (segundoSeparador == -1)
        {
            contrasenaGuardada =
                lineaQt.mid(primerSeparador + 1);
        }
        // Cuenta completa: usuario|contrasena|puntos
        else
        {
            contrasenaGuardada =
                lineaQt.mid(
                    primerSeparador + 1,
                    segundoSeparador
                        - primerSeparador
                        - 1
                    );
        }

        if (usuarioGuardado == usuario)
        {
            return contrasenaGuardada == contrasena;
        }
    }

    return false;
}

bool GestorUsuarios::sumarPuntos(
    const QString &usuario,
    int puntosGanados
    )
{
    if (puntosGanados < 0)
    {
        return false;
    }

    QString ruta =
        obtenerRutaArchivo();

    std::ifstream archivoEntrada(
        ruta.toStdString()
        );

    if (!archivoEntrada.is_open())
    {
        return false;
    }

    QVector<QString> lineas;
    std::string linea;
    bool usuarioEncontrado = false;

    while (std::getline(archivoEntrada, linea))
    {
        QString lineaQt =
            QString::fromStdString(linea);

        int primerSeparador =
            lineaQt.indexOf('|');

        if (primerSeparador == -1)
        {
            lineas.append(lineaQt);
            continue;
        }

        int segundoSeparador =
            lineaQt.indexOf(
                '|',
                primerSeparador + 1
                );

        QString usuarioGuardado =
            lineaQt.left(primerSeparador);

        if (usuarioGuardado != usuario)
        {
            lineas.append(lineaQt);
            continue;
        }

        QString contrasenaGuardada;
        int puntosActuales = 0;

        // Cuenta sin puntos: usuario|contrasena
        if (segundoSeparador == -1)
        {
            contrasenaGuardada =
                lineaQt.mid(primerSeparador + 1);
        }
        // Cuenta completa: usuario|contrasena|puntos
        else
        {
            contrasenaGuardada =
                lineaQt.mid(
                    primerSeparador + 1,
                    segundoSeparador
                        - primerSeparador
                        - 1
                    );

            bool conversionCorrecta = false;

            puntosActuales =
                lineaQt.mid(segundoSeparador + 1)
                    .toInt(&conversionCorrecta);

            if (!conversionCorrecta)
            {
                puntosActuales = 0;
            }
        }

        int nuevoTotal =
            puntosActuales + puntosGanados;

        QString lineaActualizada =
            usuarioGuardado
            + "|"
            + contrasenaGuardada
            + "|"
            + QString::number(nuevoTotal);

        lineas.append(lineaActualizada);
        usuarioEncontrado = true;
    }

    archivoEntrada.close();

    if (!usuarioEncontrado)
    {
        return false;
    }

    std::ofstream archivoSalida(
        ruta.toStdString(),
        std::ios::trunc
        );

    if (!archivoSalida.is_open())
    {
        return false;
    }

    for (const QString &lineaActualizada : lineas)
    {
        archivoSalida
            << lineaActualizada.toStdString()
            << "\n";
    }

    return archivoSalida.good();
}

int GestorUsuarios::obtenerPuntosUsuario(
    const QString &usuario
    )
{
    std::ifstream archivo(
        obtenerRutaArchivo().toStdString()
        );

    if (!archivo.is_open())
    {
        return 0;
    }

    std::string linea;

    while (std::getline(archivo, linea))
    {
        QString lineaQt =
            QString::fromStdString(linea);

        int primerSeparador =
            lineaQt.indexOf('|');

        if (primerSeparador == -1)
        {
            continue;
        }

        QString usuarioGuardado =
            lineaQt.left(primerSeparador);

        if (usuarioGuardado != usuario)
        {
            continue;
        }

        int segundoSeparador =
            lineaQt.indexOf(
                '|',
                primerSeparador + 1
                );

        if (segundoSeparador == -1)
        {
            return 0;
        }

        bool conversionCorrecta = false;

        int puntos =
            lineaQt.mid(segundoSeparador + 1)
                .toInt(&conversionCorrecta);

        if (conversionCorrecta)
        {
            return puntos;
        }

        return 0;
    }

    return 0;
}

bool GestorUsuarios::cambiarContrasena(
    const QString &usuario,
    const QString &contrasenaNueva
    )
{
    QString ruta = obtenerRutaArchivo();

    std::ifstream archivoEntrada(
        ruta.toStdString()
        );

    if (!archivoEntrada.is_open())
    {
        return false;
    }

    QVector<QString> lineas;
    std::string linea;
    bool usuarioEncontrado = false;

    while (std::getline(archivoEntrada, linea))
    {
        QString lineaQt =
            QString::fromStdString(linea);

        int primerSeparador =
            lineaQt.indexOf('|');

        if (primerSeparador == -1)
        {
            lineas.append(lineaQt);
            continue;
        }

        QString usuarioGuardado =
            lineaQt.left(primerSeparador);

        if (usuarioGuardado != usuario)
        {
            lineas.append(lineaQt);
            continue;
        }

        int segundoSeparador =
            lineaQt.indexOf(
                '|',
                primerSeparador + 1
                );

        QString puntosGuardados = "0";

        if (segundoSeparador != -1)
        {
            puntosGuardados =
                lineaQt.mid(segundoSeparador + 1);
        }

        QString lineaActualizada =
            usuarioGuardado
            + "|"
            + contrasenaNueva
            + "|"
            + puntosGuardados;

        lineas.append(lineaActualizada);
        usuarioEncontrado = true;
    }

    archivoEntrada.close();

    if (!usuarioEncontrado)
    {
        return false;
    }

    std::ofstream archivoSalida(
        ruta.toStdString(),
        std::ios::trunc
        );

    if (!archivoSalida.is_open())
    {
        return false;
    }

    for (const QString &lineaActualizada : lineas)
    {
        archivoSalida
            << lineaActualizada.toStdString()
            << "\n";
    }

    return archivoSalida.good();
}

QVector<GestorUsuarios::DatoRanking>
GestorUsuarios::obtenerRanking(int limite)
{
    QVector<DatoRanking> ranking;

    if (limite <= 0)
    {
        return ranking;
    }

    std::ifstream archivo(
        obtenerRutaArchivo().toStdString()
        );

    if (!archivo.is_open())
    {
        return ranking;
    }

    std::string linea;

    while (std::getline(archivo, linea))
    {
        QString lineaQt =
            QString::fromStdString(linea);

        int primerSeparador =
            lineaQt.indexOf('|');

        if (primerSeparador == -1)
        {
            continue;
        }

        int segundoSeparador =
            lineaQt.indexOf(
                '|',
                primerSeparador + 1
                );

        QString usuario =
            lineaQt.left(primerSeparador);

        int puntos = 0;

        if (segundoSeparador != -1)
        {
            bool conversionCorrecta = false;

            puntos =
                lineaQt.mid(segundoSeparador + 1)
                    .toInt(&conversionCorrecta);

            if (!conversionCorrecta)
            {
                puntos = 0;
            }
        }

        DatoRanking dato;

        dato.usuario = usuario;
        dato.puntos = puntos;

        ranking.append(dato);
    }

    // Ordenamiento burbuja de mayor a menor puntaje.
    for (int pasada = 0;
         pasada < ranking.size() - 1;
         pasada++)
    {
        for (int posicion = 0;
             posicion < ranking.size() - pasada - 1;
             posicion++)
        {
            if (ranking[posicion].puntos
                < ranking[posicion + 1].puntos)
            {
                DatoRanking temporal =
                    ranking[posicion];

                ranking[posicion] =
                    ranking[posicion + 1];

                ranking[posicion + 1] =
                    temporal;
            }
        }
    }

    while (ranking.size() > limite)
    {
        ranking.removeLast();
    }

    return ranking;
}

GestorUsuarios::ResultadoRegistro
GestorUsuarios::registrarUsuario(
    const QString &usuario,
    const QString &contrasena
    )
{
    if (usuarioExiste(usuario))
    {
        return ResultadoRegistro::UsuarioDuplicado;
    }

    std::ofstream archivo(
        obtenerRutaArchivo().toStdString(),
        std::ios::app
        );

    if (!archivo.is_open())
    {
        return ResultadoRegistro::ErrorArchivo;
    }

    archivo
        << usuario.toStdString()
        << "|"
        << contrasena.toStdString()
        << "|"
        << 0
        << "\n";

    if (!archivo.good())
    {
        return ResultadoRegistro::ErrorArchivo;
    }

    return ResultadoRegistro::Exito;
}
