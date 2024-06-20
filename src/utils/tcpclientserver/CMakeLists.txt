cmake_minimum_required(VERSION 3.17)
project(tcp)

set(CMAKE_CXX_STANDARD 14)
SET(GCC_COVERAGE_LINK_FLAGS "-pthread")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

add_executable(tcp main.cpp Client.cpp Client.h Server.cpp Server.h tcpStructs.h)