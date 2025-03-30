add_library(index INTERFACE)
target_include_directories(index INTERFACE .)

# For LSP support
add_executable(dummy dummy.cpp)
target_link_libraries(dummy lib)
