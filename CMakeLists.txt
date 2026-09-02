cmake_minimum_required(VERSION 3.16)

project(Snake LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
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
    recursos.qrc
)

target_link_libraries(Snake PRIVATE Qt6::Widgets Qt6::Multimedia)

enable_testing()
add_executable(SnakeModosTests tests/test_modos.cpp controles.cpp)
target_link_libraries(SnakeModosTests PRIVATE Qt6::Widgets)
add_test(NAME SnakeModosTests COMMAND SnakeModosTests)

add_executable(SnakeObjetosTests tests/test_objetos.cpp fruta.cpp item.cpp progreso.cpp snake.cpp reglasmovimiento.cpp)
target_link_libraries(SnakeObjetosTests PRIVATE Qt6::Widgets)
add_test(NAME SnakeObjetosTests COMMAND SnakeObjetosTests)

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
