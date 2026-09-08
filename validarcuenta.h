#ifndef VALIDARCUENTA_H
#define VALIDARCUENTA_H

#include <QString>

class ValidarCuenta
{
public:
    // Validación del nombre de usuario
    static bool usuarioValido(const QString &usuario);

    // Validaciones individuales de la contraseña
    static bool longitudValida(const QString &contrasena);
    static bool contieneMayuscula(const QString &contrasena);
    static bool contieneMinuscula(const QString &contrasena);
    static bool contieneNumero(const QString &contrasena);
    static bool contieneCaracterEspecial(const QString &contrasena);
    static bool noContieneEspacios(const QString &contrasena);

    // Validación completa
    static bool contrasenaValida(const QString &contrasena);

    // Comparación con el campo de confirmación
    static bool contrasenasCoinciden(
        const QString &contrasena,
        const QString &confirmacion
        );
};

#endif // VALIDARCUENTA_H