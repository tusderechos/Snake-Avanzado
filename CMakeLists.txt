cmake_minimum_required(VERSION 3.16)

project(Snake LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_executable(ProgresoTests
    progreso_test.cpp
    progreso.h
    progreso.cpp
)
add_test(NAME ProgresoTests COMMAND ProgresoTests)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

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
)

target_link_libraries(Snake PRIVATE Qt6::Widgets)

include(GNUInstallDirs)
install(TARGETS Snake
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
