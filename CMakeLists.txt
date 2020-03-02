cmake_minimum_required(VERSION 3.15)
project(crdts)

set(CMAKE_CXX_STANDARD 14)

add_executable(
        crdts
        sb/lwwregister.cc
        sb/lwwregister.hh
        main.cpp
)