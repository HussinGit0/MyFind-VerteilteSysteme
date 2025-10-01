# Prüfe, ob unter Linux gebaut wird
if(NOT (UNIX AND NOT APPLE))
    message(FATAL_ERROR "Dieses Projekt kann nur unter Linux (WSL) gebaut werden.")
endif()

cmake_minimum_required(VERSION 3.5.0)
project(MyFind)

set(CMAKE_CXX_STANDARD 20)

add_executable(MyFind src/main.cpp
<<<<<<< HEAD
    src/Options.cpp)
=======
    src/Options.cpp
    src/SearchResult.cpp)
>>>>>>> 0699c470143cb0d0a9d894d26974f9b0bae27a54


