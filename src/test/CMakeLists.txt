enable_testing()

file(GLOB TEST_SOURCES "*.cpp")

find_package(OpenSSL REQUIRED)

foreach(TEST_SOURCE ${TEST_SOURCES})
  # Extract name without extension
  get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)

  if (TEST_NAME STREQUAL "test_crawler") # This doesnt finish
    continue()
  endif()

  add_executable(${TEST_NAME} ${TEST_SOURCE})

  target_link_libraries(${TEST_NAME} crawler_interface index lib english OpenSSL::SSL OpenSSL::Crypto)

  if (CMAKE_BUILD_TYPE MATCHES "DebugGit")
    target_compile_options(${TEST_NAME} PRIVATE -fsanitize=address -fsanitize=leak -fno-omit-frame-pointer -Wall -Wextra -Werror -pedantic)
    target_link_options(${TEST_NAME} PRIVATE -fsanitize=address -fsanitize=leak)
  else()
    target_compile_options(${TEST_NAME} PRIVATE -fno-omit-frame-pointer -Wall -Wextra -Werror -pedantic -g -O0)
  endif()

  if (TEST_NAME STREQUAL "test_url_send")
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/test_data/loopback_ip.txt)
  else()
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})  # Register as a test
  endif()
endforeach()

find_package(Threads REQUIRED) # for thread
target_link_libraries(test_network Threads::Threads)
target_link_libraries(test_url_send Threads::Threads)
