#ifndef GESTORUSUARIOS_H
#define GESTORUSUARIOS_H

#include <QString>
#include <QVector>

class GestorUsuarios
{
public:
    // Posibles resultados al intentar guardar una cuenta.
    enum class ResultadoRegistro
    {
        Exito,
        UsuarioDuplicado,
        ErrorArchivo
    };

    // Información necesaria para una fila del ranking.
    struct DatoRanking
    {
        QString usuario;
        int puntos;
    };

    // Comprueba si el nombre de usuario ya está registrado.
    static bool usuarioExiste(const QString &usuario);

    // Comprueba que el usuario y la contraseña sean correctos.
    static bool credencialesValidas(
        const QString &usuario,
        const QString &contrasena
        );

    // Suma los puntos obtenidos en una partida.
    static bool sumarPuntos(
        const QString &usuario,
        int puntosGanados
        );

    // Obtiene los puntos acumulados de un usuario.
    static int obtenerPuntosUsuario(
        const QString &usuario
        );

    // Reemplaza la contraseña y conserva los puntos.
    static bool cambiarContrasena(
        const QString &usuario,
        const QString &contrasenaNueva
        );

    // Devuelve los usuarios con mayor puntuación.
    static QVector<DatoRanking> obtenerRanking(
        int limite = 5
        );

    // Guarda una nueva cuenta.
    static ResultadoRegistro registrarUsuario(
        const QString &usuario,
        const QString &contrasena
        );

private:
    // Devuelve la ubicación del archivo usuarios.txt.
    static QString obtenerRutaArchivo();
};

#endif // GESTORUSUARIOS_H
