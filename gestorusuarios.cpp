#include "gestorusuarios.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QSaveFile>
#include <QStringList>
#include <QTextStream>

#include <fstream>
#include <string>

namespace {
QString hashPassword(const QString &password) {
    return "sha256$" + QString::fromLatin1(
        QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
}

bool passwordMatches(const QString &stored, const QString &provided) {
    // Se aceptan registros antiguos una sola vez para no invalidar cuentas existentes.
    return stored == provided || stored == hashPassword(provided);
}

struct RegistroUsuario {
    QString usuario;
    QString contrasena;
    int puntos = 0;
    int monedas = 0;
    QStringList skins = {"clasica"};
    QString skinEquipada = "clasica";
};

RegistroUsuario leerRegistro(const QString &linea) {
    RegistroUsuario registro;
    const QStringList campos = linea.split('|');
    if (campos.size() < 2) return registro;

    registro.usuario = campos[0];
    registro.contrasena = campos[1];
    if (campos.size() >= 3) registro.puntos = qMax(0, campos[2].toInt());
    if (campos.size() >= 4) registro.monedas = qMax(0, campos[3].toInt());
    if (campos.size() >= 5 && !campos[4].isEmpty()) {
        registro.skins = campos[4].split(',', Qt::SkipEmptyParts);
        if (!registro.skins.contains("clasica")) registro.skins.prepend("clasica");
    }
    if (campos.size() >= 6 && !campos[5].isEmpty()) registro.skinEquipada = campos[5];
    if (!registro.skins.contains(registro.skinEquipada)) registro.skinEquipada = "clasica";
    return registro;
}

QString serializarRegistro(const RegistroUsuario &registro) {
    return registro.usuario + "|" + registro.contrasena + "|"
           + QString::number(registro.puntos) + "|"
           + QString::number(registro.monedas) + "|"
           + registro.skins.join(',') + "|" + registro.skinEquipada;
}

bool cargarRegistros(const QString &ruta, QVector<RegistroUsuario> &registros) {
    std::ifstream archivo(ruta.toStdString());
    if (!archivo.is_open()) return false;
    std::string linea;
    while (std::getline(archivo, linea)) {
        RegistroUsuario registro = leerRegistro(QString::fromStdString(linea));
        if (!registro.usuario.isEmpty()) registros.append(registro);
    }
    return true;
}

bool guardarRegistros(const QString &ruta, const QVector<RegistroUsuario> &registros) {
    QSaveFile archivo(ruta);
    if (!archivo.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream salida(&archivo);
    for (const RegistroUsuario &registro : registros) salida << serializarRegistro(registro) << '\n';
    salida.flush();
    return archivo.commit();
}
}


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
            return passwordMatches(contrasenaGuardada, contrasena);
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
                lineaQt.mid(segundoSeparador + 1).section('|', 0, 0)
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

    QSaveFile archivoSalida(ruta);
    if (!archivoSalida.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    QTextStream salida(&archivoSalida);
    for (const QString &lineaActualizada : lineas)
    {
        salida << lineaActualizada << '\n';
    }

    salida.flush();
    return archivoSalida.commit();
}

bool GestorUsuarios::registrarPuntajePartida(
    const QString &usuario,
    int puntosGanados,
    int monedasBonus
    )
{
    if (usuario.isEmpty() || puntosGanados <= 0) return false;

    QVector<RegistroUsuario> registros;
    const QString ruta = obtenerRutaArchivo();
    if (!cargarRegistros(ruta, registros)) return false;

    for (RegistroUsuario &registro : registros) {
        if (registro.usuario != usuario) continue;
        registro.puntos += puntosGanados;
        registro.monedas += puntosGanados / 2 + qMax(0, monedasBonus);
        return guardarRegistros(ruta, registros);
    }
    return false;
}

int GestorUsuarios::obtenerMonedasUsuario(const QString &usuario)
{
    QVector<RegistroUsuario> registros;
    if (!cargarRegistros(obtenerRutaArchivo(), registros)) return 0;
    for (const RegistroUsuario &registro : registros) {
        if (registro.usuario == usuario) return registro.monedas;
    }
    return 0;
}

bool GestorUsuarios::tieneSkin(const QString &usuario, const QString &skin)
{
    QVector<RegistroUsuario> registros;
    if (!cargarRegistros(obtenerRutaArchivo(), registros)) return false;
    for (const RegistroUsuario &registro : registros) {
        if (registro.usuario == usuario) return registro.skins.contains(skin);
    }
    return false;
}

QString GestorUsuarios::obtenerSkinEquipada(const QString &usuario)
{
    QVector<RegistroUsuario> registros;
    if (!cargarRegistros(obtenerRutaArchivo(), registros)) return "clasica";
    for (const RegistroUsuario &registro : registros) {
        if (registro.usuario == usuario) return registro.skinEquipada;
    }
    return "clasica";
}

bool GestorUsuarios::comprarSkin(
    const QString &usuario,
    const QString &skin,
    int precio
    )
{
    if (usuario.isEmpty() || skin.isEmpty() || precio < 0) return false;

    QVector<RegistroUsuario> registros;
    const QString ruta = obtenerRutaArchivo();
    if (!cargarRegistros(ruta, registros)) return false;
    for (RegistroUsuario &registro : registros) {
        if (registro.usuario != usuario) continue;
        if (registro.skins.contains(skin) || registro.monedas < precio) return false;
        registro.monedas -= precio;
        registro.skins.append(skin);
        return guardarRegistros(ruta, registros);
    }
    return false;
}

bool GestorUsuarios::equiparSkin(
    const QString &usuario,
    const QString &skin
    )
{
    QVector<RegistroUsuario> registros;
    const QString ruta = obtenerRutaArchivo();
    if (!cargarRegistros(ruta, registros)) return false;
    for (RegistroUsuario &registro : registros) {
        if (registro.usuario != usuario || !registro.skins.contains(skin)) return false;
        registro.skinEquipada = skin;
        return guardarRegistros(ruta, registros);
    }
    return false;
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
            lineaQt.mid(segundoSeparador + 1).section('|', 0, 0)
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
            + hashPassword(contrasenaNueva)
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

    QSaveFile archivoSalida(ruta);
    if (!archivoSalida.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    QTextStream salida(&archivoSalida);
    for (const QString &lineaActualizada : lineas)
    {
        salida << lineaActualizada << '\n';
    }

    salida.flush();
    return archivoSalida.commit();
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
                lineaQt.mid(segundoSeparador + 1).section('|', 0, 0)
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
        << hashPassword(contrasena).toStdString()
        << "|"
        << 0
        << "\n";

    if (!archivo.good())
    {
        return ResultadoRegistro::ErrorArchivo;
    }

    return ResultadoRegistro::Exito;
}
