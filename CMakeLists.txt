cmake_minimum_required(VERSION 3.0.0)
project(CCOT VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(fibo_test src/test/fibo_test.c include/coroutine.h)

target_include_directories(fibo_test PUBLIC include)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
