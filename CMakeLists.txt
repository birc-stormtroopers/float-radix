cmake_minimum_required(VERSION 3.21)
project(rsort)
enable_testing()

add_library(rsort radix.c)

add_executable(rsort_test radix_test.c)
target_link_libraries(rsort_test rsort)
add_test(rsort_test rsort_test)
