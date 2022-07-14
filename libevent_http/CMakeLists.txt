cmake_minimum_required(VERSION 3.20)
project(libevent_http)

set(CMAKE_CXX_STANDARD 11)

add_executable(libevent_http
        main.cpp
        http_lib.h http_lib.cpp)
TARGET_LINK_LIBRARIES(libevent_http -levent)