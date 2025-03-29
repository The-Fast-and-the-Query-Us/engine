add_library(lib INTERFACE)
target_include_directories(lib INTERFACE .)

find_package(OpenSSL REQUIRED)
target_link_libraries(lib INTERFACE OpenSSL::SSL OpenSSL::Crypto)
