# Diseño: controles por usuario y cierre de sesión

## Objetivo

Agregar una pantalla de Controles coherente con el diseño visual de Ajustes. El usuario podrá seleccionar WASD o flechas como esquema de movimiento. La selección será exclusiva, se guardará por cuenta y usará flechas cuando la cuenta aún no tenga una preferencia guardada.

Cambiar el botón SALIR del menú principal para cerrar únicamente la sesión actual y regresar al menú de acceso, sin cerrar la aplicación.

## Pantalla Controles

Se creará la clase `Controles`, derivada de `QGraphicsScene`, en `controles.h` y `controles.cpp`. Usará el fondo `Ajustes.png`, una capa oscura y un panel central con el mismo lenguaje visual que Perfil y Ajustes.

La pantalla mostrará:

- El título `CONTROLES`.
- Una opción `WASD`, acompañada por las cuatro letras.
- Una opción `FLECHAS`, acompañada por los símbolos de dirección.
- Un interruptor visual por opción. Verde significa activo y oscuro significa inactivo.
- Un botón de puerta blanca que emite `volverSolicitado()` para regresar a Ajustes.

Al pulsar cualquiera de las dos opciones, esa opción se activará y la otra se desactivará. Nunca habrá dos opciones activas ni una pantalla sin selección.

## Persistencia

`GestorConfiguracion` administrará música, sonido y control en una misma línea:

```text
usuario|volumenMusica|volumenSonido|control
```

Los valores válidos de control serán `WASD` y `FLECHAS`. La carga aceptará líneas antiguas de tres campos y asignará `FLECHAS` como valor predeterminado.

Guardar el volumen conservará el control existente. Guardar el control conservará los dos volúmenes existentes. De esta forma, cambiar una preferencia no eliminará las demás.

## Navegación

`Ajustes` reemplazará el mensaje provisional de Controles por la señal `controlesSolicitados()`. `MainWindow` creará una única escena `Controles`, le pasará `usuarioActual` antes de mostrarla y conectará su botón de regreso con `mostrarAjustes()`.

El botón `SALIR` del menú principal llamará a un nuevo método `cerrarSesion()`. Este método vaciará `usuarioActual`, mostrará `escenaInicio` y reajustará la vista. No cerrará la ventana ni eliminará cuentas o preferencias.

## Archivos

- Nuevos: `controles.h`, `controles.cpp`.
- Modificados: `ajustes.h`, `ajustes.cpp`, `gestorconfiguracion.h`, `gestorconfiguracion.cpp`, `mainwindow.h`, `mainwindow.cpp`, `CMakeLists.txt`.

## Verificación

- Compilar el ejecutable completo con la configuración Qt existente.
- Confirmar que solo un esquema aparece activo.
- Confirmar que una cuenta sin configuración inicia con flechas.
- Confirmar que la selección se conserva al cambiar de pantalla y volver a iniciar sesión.
- Confirmar que cambiar controles no modifica los volúmenes.
- Confirmar que cambiar volumen no modifica el control.
- Confirmar que SALIR vuelve al menú de acceso y deja abierta la aplicación.
