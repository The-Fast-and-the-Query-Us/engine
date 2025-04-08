cmake_minimum_required(VERSION 3.30)
set(CMAKE_CXX_COMPILER /usr/bin/g++-10)
project(FastAndQueryUs)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# export lsp shit
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(lib)
add_subdirectory(index)
add_subdirectory(crawler)
add_subdirectory(test)
add_subdirectory(query)
add_subdirectory(mock_crawl)
add_subdirectory(english)
