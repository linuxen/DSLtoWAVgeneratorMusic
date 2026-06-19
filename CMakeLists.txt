cmake_minimum_required(VERSION 3.24)

project(
    itmoloops
    VERSION 1.0
    DESCRIPTION "C++"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)


add_subdirectory(lib)
add_subdirectory(bin)
