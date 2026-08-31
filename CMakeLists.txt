cmake_minimum_required(VERSION 3.16)

project(Snake LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

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
    recursos.qrc
)

target_link_libraries(Snake PRIVATE Qt6::Widgets)

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
