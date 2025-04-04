add_executable(server server.cpp)
target_link_libraries(server lib index)
target_compile_options(server PRIVATE -Wno-deprecated-declarations) # allow use of sprintf

add_executable(ranker_test ranker_test.cpp)
target_link_libraries(ranker_test lib index)
target_compile_options(ranker_test PRIVATE -Wno-deprecated-declarations) # allow use of sprintf
