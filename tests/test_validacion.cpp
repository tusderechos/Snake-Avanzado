#include "validarcuenta.h"
#include <QCoreApplication>
#include <QDebug>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    int fallos = 0;
    auto comprobar = [&fallos](bool resultado, const char *caso) {
        if (!resultado) { qCritical() << caso; ++fallos; }
    };
    comprobar(ValidarCuenta::correoValido("  jugador+snake@ejemplo.com  "), "Correo recortado válido");
    comprobar(!ValidarCuenta::correoValido("jugador@"), "Dominio ausente");
    comprobar(!ValidarCuenta::correoValido("jugador@ejemplo"), "Dominio sin sufijo");
    comprobar(!ValidarCuenta::correoValido("jugador @ejemplo.com"), "Espacio interno");
    comprobar(!ValidarCuenta::correoValido("jugador@@ejemplo.com"), "Arroba duplicada");
    comprobar(!ValidarCuenta::correoValido("jugador@ejemplo..com"), "Etiqueta de dominio vacía");
    comprobar(ValidarCuenta::contrasenaValida("Ab12!x"), "Límite inferior de contraseña");
    comprobar(ValidarCuenta::contrasenaValida("Abc123!"), "Contraseña de siete caracteres");
    comprobar(!ValidarCuenta::contrasenaValida("Ab1!x"), "Contraseña demasiado corta");
    comprobar(ValidarCuenta::contrasenaValida("Abcd123!"), "Límite superior de contraseña");
    comprobar(!ValidarCuenta::contrasenaValida("Abcde123!"), "Contraseña demasiado larga");
    comprobar(!ValidarCuenta::contrasenaValida("Abcd12! "), "Espacios en contraseña");
    comprobar(!ValidarCuenta::contrasenaValida("Abcd123|"), "Separador prohibido");
    comprobar(!ValidarCuenta::contrasenaValida("abcd123!"), "Mayúscula requerida");
    comprobar(!ValidarCuenta::contrasenaValida("ABCD123!"), "Minúscula requerida");
    comprobar(!ValidarCuenta::contrasenaValida("Abcdefg!"), "Número requerido");
    comprobar(!ValidarCuenta::contrasenaValida("Abcd1234"), "Símbolo requerido");
    comprobar(ValidarCuenta::errorContrasena("Abcd123!").isEmpty(), "Sin error para contraseña válida");
    comprobar(!ValidarCuenta::errorContrasena("abcd123!").isEmpty(), "Error específico para contraseña inválida");
    comprobar(ValidarCuenta::usuarioValido("Snake_123"), "Nombre visible válido");
    comprobar(!ValidarCuenta::usuarioValido("ab"), "Nombre demasiado corto");
    comprobar(!ValidarCuenta::usuarioValido("Snake User"), "Nombre con espacios");
    comprobar(!ValidarCuenta::contrasenasCoinciden("Abcd123!", "abcd123!"), "Confirmación distingue mayúsculas");
    return fallos ? 1 : 0;
}
