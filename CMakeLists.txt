cmake_minimum_required(VERSION 3.16)

project(MiniProjekt3HashTable LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(hash_table_benchmark
    src/main.cpp
)

target_include_directories(hash_table_benchmark PRIVATE include)
