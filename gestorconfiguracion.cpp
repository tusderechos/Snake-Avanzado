#include "gestorconfiguracion.h"

#include <QCoreApplication>
#include <QSaveFile>
#include <QTextStream>
#include <QVector>

#include <fstream>
#include <string>

QString GestorConfiguracion::obtenerRutaArchivo()
{
    return QCoreApplication::applicationDirPath()
           + "/configuracion.txt";
}

void GestorConfiguracion::cargarVolumen(
    const QString &usuario,
    int &volumenMusica,
    int &volumenSonido
    )
{
    volumenMusica = 75;
    volumenSonido = 75;

    if (usuario.isEmpty())
    {
        return;
    }

    std::ifstream archivo(
        obtenerRutaArchivo().toStdString()
        );

    if (!archivo.is_open())
    {
        return;
    }

    std::string lineaArchivo;

    while (std::getline(archivo, lineaArchivo))
    {
        QString linea = QString::fromStdString(
            lineaArchivo
            );

        int primerSeparador = linea.indexOf('|');
        int segundoSeparador = linea.indexOf(
            '|',
            primerSeparador + 1
            );
        int tercerSeparador = linea.indexOf(
            '|',
            segundoSeparador + 1
            );

        if (primerSeparador < 0 || segundoSeparador < 0)
        {
            continue;
        }

        QString usuarioGuardado = linea.left(
            primerSeparador
            );

        if (usuarioGuardado != usuario)
        {
            continue;
        }

        bool musicaCorrecta = false;
        bool sonidoCorrecto = false;

        int musica = linea.mid(
            primerSeparador + 1,
            segundoSeparador - primerSeparador - 1
            ).toInt(&musicaCorrecta);

        int sonido = linea.mid(
            segundoSeparador + 1
            , tercerSeparador < 0
                  ? -1
                  : tercerSeparador - segundoSeparador - 1
            ).toInt(&sonidoCorrecto);

        if (musicaCorrecta
            && sonidoCorrecto
            && musica >= 0
            && musica <= 100
            && sonido >= 0
            && sonido <= 100)
        {
            volumenMusica = musica;
            volumenSonido = sonido;
        }

        return;
    }
}

bool GestorConfiguracion::guardarVolumen(
    const QString &usuario,
    int volumenMusica,
    int volumenSonido
    )
{
    if (usuario.isEmpty()
        || volumenMusica < 0
        || volumenMusica > 100
        || volumenSonido < 0
        || volumenSonido > 100)
    {
        return false;
    }

    QVector<QString> lineas;
    std::ifstream archivoLectura(
        obtenerRutaArchivo().toStdString()
        );

    std::string lineaArchivo;

    while (std::getline(archivoLectura, lineaArchivo))
    {
        lineas.append(
            QString::fromStdString(lineaArchivo)
            );
    }

    archivoLectura.close();

    QString control = cargarControl(usuario);

    QString nuevaLinea = usuario
                         + "|"
                         + QString::number(volumenMusica)
                         + "|"
                         + QString::number(volumenSonido)
                         + "|"
                         + control;

    bool usuarioEncontrado = false;

    for (int posicion = 0;
         posicion < lineas.size();
         posicion++)
    {
        int separador = lineas[posicion].indexOf('|');

        if (separador >= 0
            && lineas[posicion].left(separador) == usuario)
        {
            lineas[posicion] = nuevaLinea;
            usuarioEncontrado = true;
            break;
        }
    }

    if (!usuarioEncontrado)
    {
        lineas.append(nuevaLinea);
    }

    QSaveFile archivoEscritura(obtenerRutaArchivo());
    if (!archivoEscritura.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    QTextStream salida(&archivoEscritura);
    for (const QString &linea : lineas)
    {
        salida << linea << '\n';
    }

    salida.flush();
    return archivoEscritura.commit();
}

QString GestorConfiguracion::cargarControl(
    const QString &usuario
    )
{
    const QString controlPredeterminado = "FLECHAS";

    if (usuario.isEmpty())
    {
        return controlPredeterminado;
    }

    std::ifstream archivo(
        obtenerRutaArchivo().toStdString()
        );

    if (!archivo.is_open())
    {
        return controlPredeterminado;
    }

    std::string lineaArchivo;

    while (std::getline(archivo, lineaArchivo))
    {
        QString linea = QString::fromStdString(lineaArchivo);
        int primerSeparador = linea.indexOf('|');

        if (primerSeparador < 0
            || linea.left(primerSeparador) != usuario)
        {
            continue;
        }

        int segundoSeparador = linea.indexOf(
            '|', primerSeparador + 1
            );
        int tercerSeparador = linea.indexOf(
            '|', segundoSeparador + 1
            );

        if (segundoSeparador < 0 || tercerSeparador < 0)
        {
            return controlPredeterminado;
        }

        QString control = linea.mid(tercerSeparador + 1).trimmed();

        if (control == "WASD" || control == "FLECHAS")
        {
            return control;
        }

        return controlPredeterminado;
    }

    return controlPredeterminado;
}

bool GestorConfiguracion::guardarControl(
    const QString &usuario,
    const QString &control
    )
{
    if (usuario.isEmpty()
        || (control != "WASD" && control != "FLECHAS"))
    {
        return false;
    }

    int volumenMusica = 75;
    int volumenSonido = 75;
    cargarVolumen(usuario, volumenMusica, volumenSonido);

    QVector<QString> lineas;
    std::ifstream archivoLectura(
        obtenerRutaArchivo().toStdString()
        );

    std::string lineaArchivo;

    while (std::getline(archivoLectura, lineaArchivo))
    {
        lineas.append(QString::fromStdString(lineaArchivo));
    }

    archivoLectura.close();

    QString nuevaLinea = usuario
                         + "|"
                         + QString::number(volumenMusica)
                         + "|"
                         + QString::number(volumenSonido)
                         + "|"
                         + control;

    bool usuarioEncontrado = false;

    for (int posicion = 0; posicion < lineas.size(); posicion++)
    {
        int separador = lineas[posicion].indexOf('|');

        if (separador >= 0
            && lineas[posicion].left(separador) == usuario)
        {
            lineas[posicion] = nuevaLinea;
            usuarioEncontrado = true;
            break;
        }
    }

    if (!usuarioEncontrado)
    {
        lineas.append(nuevaLinea);
    }

    QSaveFile archivoEscritura(obtenerRutaArchivo());
    if (!archivoEscritura.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    QTextStream salida(&archivoEscritura);
    for (const QString &linea : lineas)
    {
        salida << linea << '\n';
    }

    salida.flush();
    return archivoEscritura.commit();
}
