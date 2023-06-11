cmake_minimum_required(VERSION 3.22)
project(bmp24)

set(CMAKE_CXX_STANDARD 23)

set(SOURCES include/bmp_header.h include/bmp_image.h src/bmp_image.cpp)

add_executable(task1 ${SOURCES} src/main1.cpp)
add_executable(task2 ${SOURCES} src/main2.cpp)
add_executable(task3 ${SOURCES} src/main3.cpp)

target_include_directories(task1 PRIVATE include)
target_include_directories(task2 PRIVATE include)
target_include_directories(task3 PRIVATE include)