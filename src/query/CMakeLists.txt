add_executable(server server.cpp)
target_link_libraries(server lib index)
target_compile_options(server PRIVATE -Wno-deprecated-declarations) # allow use of sprintf
