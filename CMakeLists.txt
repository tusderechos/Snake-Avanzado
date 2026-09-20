cmake_minimum_required(VERSION 3.16)

project(Snake LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia Network)

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
    animadorserpiente.h
    animadorserpiente.cpp
    audio.h
    audio.cpp
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
    dialogos.h
    manualusuario.h
    manualusuario.cpp
    skins.h
    validarcuenta.h
    validarcuenta.cpp
    supabaseclient.h
    supabaseclient.cpp
    recursos.qrc
)

target_link_libraries(Snake PRIVATE Qt6::Widgets Qt6::Multimedia Qt6::Network)

include(CTest)
if(BUILD_TESTING)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    add_executable(test_validacion tests/test_validacion.cpp validarcuenta.cpp)
    target_include_directories(test_validacion PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(test_validacion PRIVATE Qt6::Core)
    add_test(NAME validacion COMMAND test_validacion)
    foreach(nombre IN ITEMS supabaseclient gestorusuarios)
        add_executable(test_${nombre} tests/test_${nombre}.cpp supabaseclient.cpp)
        target_include_directories(test_${nombre} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
        target_link_libraries(test_${nombre} PRIVATE Qt6::Test Qt6::Network)
        add_test(NAME ${nombre} COMMAND test_${nombre})
    endforeach()
    target_sources(test_gestorusuarios PRIVATE gestorusuarios.cpp gestorconfiguracion.cpp)
endif()

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
    # Optimizaciones de distribución: reducen trabajo del ejecutable sin
    # cambiar la lógica del juego ni sacrificar la depuración en Debug.
    target_compile_options(Snake PRIVATE
        $<$<CONFIG:Release>:-O2;-DNDEBUG>
        $<$<CONFIG:MinSizeRel>:-Os;-DNDEBUG>
    )
elseif (MSVC)
    target_compile_options(Snake PRIVATE /W4)
    target_compile_options(Snake PRIVATE
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:MinSizeRel>:/O1>
        $<$<CONFIG:Release>:/DNDEBUG>
        $<$<CONFIG:MinSizeRel>:/DNDEBUG>
    )
endif()

include(GNUInstallDirs)
install(TARGETS Snake
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
