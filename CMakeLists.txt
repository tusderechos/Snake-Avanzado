cmake_minimum_required(VERSION 3.16)

project(Snake LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia Network)

add_executable(Snake WIN32
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
)

target_link_libraries(Snake PRIVATE Qt6::Widgets Qt6::Multimedia Qt6::Network)

# Los recursos visuales son numerosos. Qt los transforma directamente en un
# recurso binario, evitando compilar una matriz C++ enorme en cada Release.
qt_add_big_resources(snake_resource_objects recursos.qrc)
target_sources(Snake PRIVATE ${snake_resource_objects})

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

# Generar una instalación portable de Windows con las DLL de Qt y sus
# plugins. Esto evita publicar solo Snake.exe sin sus dependencias de Qt.
if (WIN32 AND COMMAND qt_generate_deploy_app_script)
    qt_generate_deploy_app_script(
        TARGET Snake
        OUTPUT_SCRIPT snake_deploy_script
        NO_TRANSLATIONS
    )
    install(SCRIPT "${snake_deploy_script}")
endif()
