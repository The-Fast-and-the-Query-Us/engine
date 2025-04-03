cmake_minimum_required(VERSION 3.28)
project(MockCrawler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

execute_process(
  COMMAND pybind11-config --cmake
  OUTPUT_VARIABLE config_dir
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(pybind11_DIR "${config_dir}")
find_package(pybind11 REQUIRED)

add_library(pybind MODULE pybind.cpp)
target_link_libraries(pybind PUBLIC pybind11::module)
