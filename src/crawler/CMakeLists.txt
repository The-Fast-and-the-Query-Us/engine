find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

add_library(crawler_interface INTERFACE)
target_include_directories(crawler_interface INTERFACE .)
target_link_libraries(crawler_interface INTERFACE lib index OpenSSL::SSL OpenSSL::Crypto english)

set(SAN "-fsanitize=address,leak")

add_executable(crawler crawler.cpp)
target_link_libraries(crawler PUBLIC crawler_interface Threads::Threads ${SAN})
target_compile_options(crawler PUBLIC -Wall -Wextra -Werror -pedantic -g -O3 ${SAN})
