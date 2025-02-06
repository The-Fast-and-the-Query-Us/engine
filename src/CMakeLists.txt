cmake_minimum_required(VERSION 3.30)
project(MyProject)

enable_testing()

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# export lsp shit
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(lib)
include_directories(index)

# tests
file(GLOB TEST_SOURCES "test/*.cpp")
foreach(TEST_SOURCE ${TEST_SOURCES})
  get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE) # Extract name without extension
  add_executable(${TEST_NAME} ${TEST_SOURCE})

  if (CMAKE_BUILD_TYPE MATCHES "DebugGit")
    target_compile_options(${TEST_NAME} PRIVATE -fsanitize=address -fsanitize=leak -fno-omit-frame-pointer -Wall -Wextra -Werror -pedantic)
    target_link_options(${TEST_NAME} PRIVATE -fsanitize=address -fsanitize=leak)
  else()
    target_compile_options(${TEST_NAME} PRIVATE -fno-omit-frame-pointer -Wall -Wextra -Werror -pedantic -g)
  endif()

  add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})  # Register as a test
endforeach()
