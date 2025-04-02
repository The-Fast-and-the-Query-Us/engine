add_executable(mock main.cpp)
target_link_libraries(mock lib index)
target_compile_options(mock PRIVATE -Wno-deprecated-declarations -g) # allow use of sprintf
