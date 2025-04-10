add_executable(ranker_test ranker_test.cpp)
target_link_libraries(ranker_test index lib)

add_executable(optimizer optimizer.cpp)
target_link_libraries(optimizer index lib)

find_package(Threads REQUIRED)
add_executable(query_server query_server.cpp)
target_link_libraries(query_server index lib Threads::Threads)
