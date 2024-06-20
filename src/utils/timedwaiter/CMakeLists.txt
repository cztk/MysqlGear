cmake_minimum_required(VERSION 3.17)
project(TimedWaiter)

set(CMAKE_CXX_STANDARD 14)

SET(GCC_COVERAGE_LINK_FLAGS "-pthread")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

add_executable(TimedWaiter main.cpp timed_waiter.h)