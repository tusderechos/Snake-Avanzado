cmake_minimum_required(VERSION 3.16)

project(Snake LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia)

add_executable(Snake
    main.cpp
    Nodo.h
    progreso.h
    progreso.cpp
    reglasmovimiento.h
    reglasmovimiento.cpp
    snake.h
    snake.cpp
    tablero.h
    tablero.cpp
    juegoview.h
    juegoview.cpp
    fruta.h
    fruta.cpp
    item.h
    item.cpp
    obstaculo.h
    obstaculo.cpp
    gestorpartida.h
    gestorpartida.cpp
    modojuego.h
    controles.h
    controles.cpp
    menujuego.h
    menujuego.cpp
    tutorial.h
    tutorial.cpp
    mainwindow.h
    mainwindow.cpp
    mainwindow.ui
    ajustes.h
    ajustes.cpp
    controlesview.h
    controlesview.cpp
    escenaregistro.h
    escenaregistro.cpp
    gestorconfiguracion.h
    gestorconfiguracion.cpp
    gestorusuarios.h
    gestorusuarios.cpp
    iniciosesion.h
    iniciosesion.cpp
    menuprincipal.h
    menuprincipal.cpp
    perfil.h
    perfil.cpp
    ranking.h
    ranking.cpp
    tienda.h
    tienda.cpp
    validarcuenta.h
    validarcuenta.cpp
    recursos.qrc
)

target_link_libraries(Snake PRIVATE Qt6::Widgets Qt6::Multimedia)

# Algunos assets exportados desde Canva son WebP aunque conserven una
# extensión .png. Copiar el plugin oficial evita que desaparezcan al ejecutar
# el binario fuera del entorno completo de Qt.
if (WIN32)
    set(QT_WEBP_PLUGIN "${Qt6Core_DIR}/../../../plugins/imageformats/qwebp.dll")
    if (EXISTS "${QT_WEBP_PLUGIN}")
        add_custom_command(TARGET Snake POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                    "$<TARGET_FILE_DIR:Snake>/imageformats"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${QT_WEBP_PLUGIN}"
                    "$<TARGET_FILE_DIR:Snake>/imageformats/qwebp.dll"
            VERBATIM)
    endif()
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(Snake PRIVATE -Wall -Wextra -Wpedantic)
elseif (MSVC)
    target_compile_options(Snake PRIVATE /W4)
endif()

include(GNUInstallDirs)
install(TARGETS Snake
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
