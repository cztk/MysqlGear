cmake_minimum_required(VERSION 3.17)
project(lockedqueue)

set(CMAKE_CXX_STANDARD 14)

add_executable(lockedqueue main.cpp LockedQueue.h)