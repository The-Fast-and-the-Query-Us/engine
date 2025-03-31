add_executable(server server.cpp query.cpp)
target_link_libraries(server lib index)
