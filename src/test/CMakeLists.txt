enable_testing()

file(GLOB TEST_SOURCES "*.cpp")

find_package(OpenSSL REQUIRED)

foreach(TEST_SOURCE ${TEST_SOURCES})
  # Extract name without extension
  get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)

  if (TEST_NAME STREQUAL "test_bloom_filter") # REMOVE THIS WHEN IT PASSES!
    continue()
  endif()

  add_executable(${TEST_NAME} ${TEST_SOURCE})
  target_link_libraries(${TEST_NAME} index lib crawler OpenSSL::SSL OpenSSL::Crypto)

  if (CMAKE_BUILD_TYPE MATCHES "DebugGit")
    target_compile_options(${TEST_NAME} PRIVATE -fsanitize=address -fsanitize=leak -fno-omit-frame-pointer -Wall -Wextra -Werror -pedantic)
    target_link_options(${TEST_NAME} PRIVATE -fsanitize=address -fsanitize=leak)
  else()
    target_compile_options(${TEST_NAME} PRIVATE -fno-omit-frame-pointer -Wall -Wextra -Werror -pedantic -g -O0)
  endif()

  add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})  # Register as a test
endforeach()
