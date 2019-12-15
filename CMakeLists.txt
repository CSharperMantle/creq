cmake_minimum_required(VERSION 3.0.0)
project(creq VERSION 0.1.5.1)

add_library(creq STATIC creq.c)

add_executable(creq_test creq_test.c creq.c)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

set(CMAKE_C_FLAGS_DEBUG "-std=c11 -Wall -Wextra -Og -g")
set(CMAKE_C_FLAGS_RELEASE "-std=c11 -Wall -Wextra -O3")