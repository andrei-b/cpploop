cmake_minimum_required(VERSION 3.20)
project(cpploop)

set(CMAKE_CXX_STANDARD 14)

find_package(Threads REQUIRED)

include_directories(.)

add_executable(cpploop
        main.cpp
        messageloop.cpp
        messageloop.h
        sleeper.cpp
        sleeper.h
        timer.cpp
        timer.h)

target_link_libraries(cpploop PRIVATE Threads::Threads)
