find_package(Threads)
add_executable(server server.cpp)
target_link_libraries(server crawler_interface lib Threads::Threads)

add_executable(visibility_server visibility_server.cpp)
target_link_libraries(visibility_server crawler_interface lib Threads::Threads)
