cmake_minimum_required(VERSION 3.16)
project(MyProject)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# export lsp shit
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(lib)
