# Controles por usuario y cierre de sesión - Plan de implementación

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Crear una pantalla para elegir WASD o flechas por cuenta y hacer que SALIR cierre la sesión sin cerrar la aplicación.

**Architecture:** `GestorConfiguracion` conservará en una sola línea el volumen y el esquema de control de cada cuenta, aceptando el formato antiguo. Una nueva escena `Controles` administrará dos opciones mutuamente exclusivas, mientras `MainWindow` será el único responsable de cambiar escenas y limpiar la sesión.

**Tech Stack:** C++17, Qt 6 Widgets, QGraphicsScene/QGraphicsView, señales y slots, archivos de texto.

**Spec:** `docs/superpowers/specs/2026-09-04-controles-y-cierre-sesion-design.md`

## Global Constraints

- El esquema predeterminado es `FLECHAS`.
- Los únicos valores persistidos son `WASD` y `FLECHAS`.
- Los registros antiguos `usuario|musica|sonido` deben seguir cargando.
- Cambiar volumen debe conservar el control y cambiar control debe conservar ambos volúmenes.
- Solo puede existir una opción de control activa.
- SALIR vacía la sesión y vuelve al menú de acceso; nunca cierra la ventana.
- No se usarán contenedores automáticos para el cuerpo de la serpiente ni el mapa principal.

---

### Task 1: Persistencia conjunta de volumen y controles

**Files:**
- Modify: `gestorconfiguracion.h`
- Modify: `gestorconfiguracion.cpp`

**Interfaces:**
- Produces: `static QString cargarControl(const QString &usuario)`
- Produces: `static bool guardarControl(const QString &usuario, const QString &control)`
- Preserves: `cargarVolumen(...)` y `guardarVolumen(...)`

- [ ] **Step 1: Ejecutar una comprobación que falle con la interfaz ausente**

```powershell
$h = Get-Content -Raw .\gestorconfiguracion.h
if (-not ($h.Contains('cargarControl') -and $h.Contains('guardarControl'))) { exit 1 }
```

Resultado esperado: código de salida `1` porque todavía no existen ambos métodos.

- [ ] **Step 2: Añadir la interfaz mínima**

```cpp
static QString cargarControl(const QString &usuario);
static bool guardarControl(const QString &usuario, const QString &control);
```

- [ ] **Step 3: Actualizar la lectura compatible**

Separar cada línea por posiciones de `|`. Leer música y sonido de los campos segundo y tercero. Si existe un cuarto campo y es `WASD` o `FLECHAS`, devolverlo; en cualquier otro caso devolver `FLECHAS`.

- [ ] **Step 4: Actualizar las escrituras sin perder datos**

`guardarVolumen()` debe leer primero el control existente y escribir:

```text
usuario|nuevoVolumenMusica|nuevoVolumenSonido|controlExistente
```

`guardarControl()` debe rechazar valores distintos de `WASD` y `FLECHAS`, cargar los volúmenes existentes y escribir:

```text
usuario|volumenExistenteMusica|volumenExistenteSonido|nuevoControl
```

- [ ] **Step 5: Repetir la comprobación de interfaz y compilar**

```powershell
$h = Get-Content -Raw .\gestorconfiguracion.h
if (-not ($h.Contains('cargarControl') -and $h.Contains('guardarControl'))) { exit 1 }
$env:Path = 'C:\Qt\Tools\mingw1310_64\bin;' + $env:Path
& 'C:\Qt\Tools\CMake_64\bin\cmake.exe' --build .\build\Desktop_Qt_6_11_1_MinGW_64_bit_Debug --parallel
```

Resultado esperado: comprobación y compilación con código `0`.

### Task 2: Escena Controles con selección exclusiva

**Files:**
- Create: `controles.h`
- Create: `controles.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes: `GestorConfiguracion::cargarControl(const QString &)`
- Consumes: `GestorConfiguracion::guardarControl(const QString &, const QString &)`
- Produces: `void Controles::establecerUsuario(const QString &usuario)`
- Produces signal: `void Controles::volverSolicitado()`

- [ ] **Step 1: Ejecutar una comprobación que falle antes de crear la escena**

```powershell
$cmake = Get-Content -Raw .\CMakeLists.txt
if (-not ((Test-Path .\controles.h) -and (Test-Path .\controles.cpp) -and $cmake.Contains('controles.cpp'))) { exit 1 }
```

Resultado esperado: código `1`.

- [ ] **Step 2: Crear `controles.h`**

Definir `Controles : public QGraphicsScene` con `Q_OBJECT`, `establecerUsuario`, `volverSolicitado`, dos botones de opción, dos indicadores visuales, `usuarioActual`, `seleccionarWasd()` y `seleccionarFlechas()`.

- [ ] **Step 3: Construir la interfaz en `controles.cpp`**

Usar `Ajustes.png` escalado a `1254 x 1254`, velo oscuro, panel central, título `CONTROLES`, una fila para `WASD`, otra para `↑ ← ↓ →`, interruptores verde/oscuro y botón de puerta blanca. Los botones completos de cada fila deben ser pulsables.

- [ ] **Step 4: Implementar exclusividad y persistencia**

```cpp
void Controles::seleccionarWasd()
{
    actualizarSeleccion("WASD");
    GestorConfiguracion::guardarControl(usuarioActual, "WASD");
}

void Controles::seleccionarFlechas()
{
    actualizarSeleccion("FLECHAS");
    GestorConfiguracion::guardarControl(usuarioActual, "FLECHAS");
}
```

`actualizarSeleccion()` aplicará estilo activo a una opción e inactivo a la otra.

- [ ] **Step 5: Registrar la clase en CMake y compilar**

Añadir `controles.cpp` y `controles.h` a `PROJECT_SOURCES`, ejecutar CMake y compilar. Resultado esperado: `Built target SnakeProject`.

### Task 3: Navegación a Controles y cierre de sesión

**Files:**
- Modify: `ajustes.h`
- Modify: `ajustes.cpp`
- Modify: `mainwindow.h`
- Modify: `mainwindow.cpp`

**Interfaces:**
- Consumes: `Controles::establecerUsuario(const QString &)`
- Consumes signal: `Controles::volverSolicitado()`
- Produces signal: `Ajustes::controlesSolicitados()`
- Produces: `MainWindow::mostrarControles()`
- Produces: `MainWindow::cerrarSesion()`

- [ ] **Step 1: Ejecutar una comprobación que falle con el flujo antiguo**

```powershell
$cpp = Get-Content -Raw .\mainwindow.cpp
$ajustes = Get-Content -Raw .\ajustes.cpp
if ($cpp.Contains('close();') -or $ajustes.Contains('mostrarControlesPendiente')) { exit 1 }
```

Resultado esperado: código `1`.

- [ ] **Step 2: Conectar Ajustes con la nueva escena**

Reemplazar `mostrarControlesPendiente()` por la señal `controlesSolicitados()` y conectar el botón CONTROLES directamente a esa señal.

- [ ] **Step 3: Incorporar Controles en MainWindow**

Declarar y crear `Controles *escenaControles`. Antes de mostrarla, llamar:

```cpp
escenaControles->establecerUsuario(usuarioActual);
ui->graphicsView->setScene(escenaControles);
ajustarEscenaActual();
```

Conectar `Ajustes::controlesSolicitados` con `mostrarControles` y `Controles::volverSolicitado` con `mostrarAjustes`.

- [ ] **Step 4: Reemplazar el cierre de ventana por cierre de sesión**

```cpp
void MainWindow::cerrarSesion()
{
    usuarioActual.clear();
    mostrarMenuInicio();
}
```

Conectar `MenuPrincipal::salirSolicitado` con `MainWindow::cerrarSesion` y eliminar la llamada `close()`.

- [ ] **Step 5: Verificar contrato y compilación completa**

```powershell
$cpp = Get-Content -Raw .\mainwindow.cpp
$ajustes = Get-Content -Raw .\ajustes.cpp
if ($cpp.Contains('close();') -or $ajustes.Contains('mostrarControlesPendiente')) { exit 1 }
$env:Path = 'C:\Qt\Tools\mingw1310_64\bin;' + $env:Path
& 'C:\Qt\Tools\CMake_64\bin\cmake.exe' --build .\build\Desktop_Qt_6_11_1_MinGW_64_bit_Debug --parallel
```

Resultado esperado: código `0` y `Built target SnakeProject`.

- [ ] **Step 6: Prueba manual integrada**

Abrir dos cuentas. Confirmar que una comienza con flechas, cambiarla a WASD, cerrar sesión y entrar nuevamente para comprobar que conserva WASD. Confirmar que la segunda cuenta continúa con flechas. Desde SALIR, comprobar que aparece el menú con Crear Cuenta e Iniciar Sesión y que la ventana sigue abierta.

## Control de versiones

No ejecutar commits desde esta carpeta mientras `git rev-parse --show-toplevel` devuelva `C:/Users/Nathan`, porque ese repositorio abarca archivos ajenos al proyecto. Los cambios se conservarán directamente en `SnakeProject` hasta que el proyecto tenga su propio repositorio Git.
