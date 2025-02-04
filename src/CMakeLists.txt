cmake_minimum_required(VERSION 3.30)
project(MyProject)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# export lsp shit
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(lib)
include_directories(index)
