#include "validarcuenta.h"

/*Valida que el usuario tenga entre 3 y 15 caracteres
 *  y que solamente use letras, números o guion bajo.
*/
bool ValidarCuenta::usuarioValido(const QString &usuario)
{
    if (usuario.length() < 3 || usuario.length() > 15)
    {
        return false;
    }

    for (const QChar &caracter : usuario)
    {
        bool permitido =
            caracter.isLetterOrNumber() || caracter == '_';

        if (!permitido)
        {
            return false;
        }
    }

    return true;
}

// La contraseña debe tener entre 5 y 8 caracteres.
bool ValidarCuenta::longitudValida(const QString &contrasena)
{
    return contrasena.length() >= 5
           && contrasena.length() <= 8;
}

// Busca al menos una letra mayúscula.
bool ValidarCuenta::contieneMayuscula(
    const QString &contrasena
    )
{
    for (const QChar &caracter : contrasena)
    {
        if (caracter.isUpper())
        {
            return true;
        }
    }

    return false;
}

// Busca al menos una letra minúscula.
bool ValidarCuenta::contieneMinuscula(
    const QString &contrasena
    )
{
    for (const QChar &caracter : contrasena)
    {
        if (caracter.isLower())
        {
            return true;
        }
    }

    return false;
}

// Busca al menos un número.
bool ValidarCuenta::contieneNumero(
    const QString &contrasena
    )
{
    for (const QChar &caracter : contrasena)
    {
        if (caracter.isDigit())
        {
            return true;
        }
    }

    return false;
}

// Busca un símbolo que no sea letra, número ni espacio.
bool ValidarCuenta::contieneCaracterEspecial(
    const QString &contrasena
    )
{
    for (const QChar &caracter : contrasena)
    {
        bool esEspecial = !caracter.isLetterOrNumber() && !caracter.isSpace();

        if (esEspecial)
        {
            return true;
        }
    }

    return false;
}

// Comprueba que la contraseña no tenga espacios.
bool ValidarCuenta::noContieneEspacios(
    const QString &contrasena
    )
{
    for (const QChar &caracter : contrasena)
    {
        if (caracter.isSpace())
        {
            return false;
        }
    }

    return true;
}

// Reúne todos los requisitos anteriores.
bool ValidarCuenta::contrasenaValida(
    const QString &contrasena
    )
{
    return longitudValida(contrasena)
    && contieneMayuscula(contrasena)
        && contieneMinuscula(contrasena)
        && contieneNumero(contrasena)
        && contieneCaracterEspecial(contrasena)
        && noContieneEspacios(contrasena)
        && !contrasena.contains('|');
}

// Comprueba que la confirmación sea exactamente igual.
bool ValidarCuenta::contrasenasCoinciden(
    const QString &contrasena,
    const QString &confirmacion
    )
{
    return contrasena == confirmacion;
}
