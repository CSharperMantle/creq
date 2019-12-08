cmake_minimum_required(VERSION 3.0.0)
project(creq VERSION 0.1.3)

add_library(creq STATIC creq.c)

add_executable(creq_test creq_test.c creq.c)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

set(CMAKE_C_FLAGS_DEBUG "-Wall -Wextra -Og -g")
set(CMAKE_C_FLAGS_RELEASE "-Wall -Wextra -O3")