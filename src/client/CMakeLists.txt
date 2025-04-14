find_package(Threads)
add_executable(server server.cpp)
target_link_libraries(server crawler_interface lib Threads::Threads)
