# Минимальная версия CMake
cmake_minimum_required(VERSION 3.10)

# Название проекта
project(graph_lab2)

# Установим стандарт C++
set(CMAKE_CXX_STANDARD 17)

# Добавление исполняемого файла из исходников
add_executable(graph_lab2 main.cpp)