cmake_minimum_required(VERSION 3.29)
project(ProxyLab C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(ProxyLab
    cache.c
    cache.h
    error.c
    error.h
    http.c
    http.h
    io.c
    io.h
    proxy.c
    proxy.h)
