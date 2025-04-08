find_package(OpenSSL REQUIRED)

add_library(crawler_interface INTERFACE)
target_include_directories(crawler_interface INTERFACE .)
target_link_libraries(crawler_interface INTERFACE lib index OpenSSL::SSL OpenSSL::Crypto)

add_executable(crawler crawler.cpp)
target_link_libraries(crawler PUBLIC crawler_interface)
target_compile_options(crawler PUBLIC -Wall -Wextra -Werror -pedantic -g -O3)
