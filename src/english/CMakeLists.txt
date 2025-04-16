add_library(english porter.cpp normalize.cpp strip_url_prefix.cpp)
target_link_libraries(english PRIVATE lib)
target_include_directories(english PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
