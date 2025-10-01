# Prüfe, ob unter Linux gebaut wird
if(NOT (UNIX AND NOT APPLE))
    message(FATAL_ERROR "Dieses Projekt kann nur unter Linux (WSL) gebaut werden.")
endif()

cmake_minimum_required(VERSION 3.5.0)
project(MyFind)

set(CMAKE_CXX_STANDARD 20)

add_executable(MyFind src/main.cpp
    src/Options.cpp
    src/SearchResult.cpp)


