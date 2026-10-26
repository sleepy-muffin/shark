
# ========== ---------- ---------- ---------- ==========
#                          NET
# ========== ---------- ---------- ---------- ==========

add_executable(
  http-timeout-test
  http_timeout_test.cpp
)

target_include_directories(
  http-timeout-test
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(
  http-timeout-test
  PRIVATE
    shark::net
    Threads::Threads
)

add_test(NAME http-timeout COMMAND http-timeout-test)

add_executable(
  http-headers-test
  http_headers_test.cpp
)

target_include_directories(
  http-headers-test
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(
  http-headers-test
  PRIVATE
    shark::net
    Threads::Threads
)

add_test(NAME http-headers COMMAND http-headers-test)
