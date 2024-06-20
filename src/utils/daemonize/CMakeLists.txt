cmake_minimum_required(VERSION 3.17)
project(daemonize)

set(CMAKE_CXX_STANDARD 14)

add_executable(daemonize main.cpp daemonize.cpp daemonize.h)