# Base de datos de Snake

## Acceso con usuario y contraseña

El registro y el inicio de sesión ya no solicitan correo. El cliente convierte el nombre exacto del usuario (sin espacios exteriores) en SHA-256 hexadecimal y agrega `@snake.invalid` como identificador interno de Supabase Auth. Los nombres distinguen mayúsculas de minúsculas. El hash es del nombre, nunca de la contraseña; Supabase continúa almacenando y comprobando las contraseñas.

Se desactivó Confirm email en el proyecto remoto y se verificó `mailer_autoconfirm=true`. No hay recuperación por correo para estas cuentas. Para adaptar cuentas creadas antes del cambio, el propietario debe ejecutar `usuario_sin_correo.sql` en SQL Editor. Ese archivo conserva UUID, contraseñas y datos del juego. Se detectó una cuenta anterior; su migración queda pendiente de ejecución manual.

La compilación y las tres suites de pruebas pasaron después del cambio, incluida una prueba que comprueba el mismo identificador interno en registro e inicio de sesión.

En el proyecto Supabase correcto, abra SQL Editor, pegue todo `schema.sql` y ejecútelo como `postgres`. La migración usa una transacción y se puede repetir. Agrega las columnas faltantes a `perfiles` sin borrar cuentas ni progreso. Sustituye todas las políticas de esa tabla, incluidas las antiguas que permitían consultar perfiles ajenos o modificar monedas y puntos directamente.

Después ejecute `tests.sql` en SQL Editor. Las pruebas usan cuentas temporales dentro de una transacción y terminan con `ROLLBACK`. El 17 de septiembre de 2026 se aplicó la migración al proyecto `cnjuzrtseqcefiebwexf` y estas pruebas finalizaron correctamente en su editor SQL. La compilación local y las tres suites de pruebas del cliente también pasaron. Esto no sustituye una prueba manual de registro con confirmación de correo y una partida completa.

La aplicación debe usar la URL del proyecto y su clave pública `publishable` (o la clave heredada `anon`). Nunca incluya claves `service_role`, claves secretas ni la contraseña de PostgreSQL en el ejecutable o en archivos distribuidos. Las llamadas privadas necesitan además el token de acceso del usuario autenticado. El registro envía `nombre_usuario` en los metadatos de Auth; se exige un nombre único de 3 a 15 letras, números o guiones bajos. La clasificación Unicode de PostgreSQL depende de la configuración regional; compruebe los alfabetos que utilizará la aplicación.

`snake_perfil()` obtiene o recupera el perfil faltante a partir de los metadatos de Auth. Los RPC `snake_comprar_skin(p_skin)`, `snake_equipar_skin(p_skin)`, `snake_tutorial()`, `snake_nivel(p_nivel)` y `snake_partida(p_operacion,p_puntos,p_bonus)` devuelven el perfil actualizado como objeto JSON. El cliente puede modificar directamente solo `control`, `volumen_musica` y `volumen_sonido` de su perfil. Los volúmenes van de 0 a 1; el control admite `WASD` y `FLECHAS`. El ranking público expone únicamente `nombre_usuario` y `puntos`.

Los precios del servidor son los de `tienda.cpp`: clasica 0, gato 150, dragon 250, burro 350, spiderman 500, miles 650, personaje 850 y thanos 1400. Comprar equipa la skin; una compra repetida no descuenta monedas. Equipar requiere tenerla. El tutorial y los niveles nunca retroceden; los niveles van de 0 a 3 y no permiten saltarse el siguiente pendiente.

Cada envío de partida debe conservar el mismo UUID durante todos sus reintentos. El registro se identifica por usuario y UUID; repetirlo no vuelve a otorgar puntos ni monedas y cambiar sus valores genera error. Se suman `p_puntos` y `p_puntos / 2 + p_bonus` monedas, con división entera. El perfil se bloquea durante la operación para serializar compras y resultados concurrentes. Se aceptan hasta 1 000 000 puntos por envío y bonus de 0 a 175 en múltiplos de 25, compatibles con los tres niveles y el bonus final del juego. Un desbordamiento del entero provoca rollback, sin registrar parcialmente la operación.

Estos límites y las funciones protegen la integridad de las operaciones, pero no demuestran que la partida se haya jugado: un cliente modificado puede declarar resultados o usar nuevos UUID. Un ranking resistente a trampas necesitaría validar las partidas en un servidor. No borre el registro de operaciones mientras puedan llegar reintentos antiguos.
