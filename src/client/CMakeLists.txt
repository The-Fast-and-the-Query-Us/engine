find_package(Threads)
add_executable(server server.cpp)
target_link_libraries(server lib Threads::Threads)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/frontend/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
