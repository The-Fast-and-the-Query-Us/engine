find_package(Threads)
add_executable(server server.cpp)
target_link_libraries(server lib Threads::Threads)
