cmake_minimum_required(VERSION 3.10)
project(Lab3_ThreadSync)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(Lab3 main.cpp)


if(WIN32)
    target_link_libraries(Lab3)
endif()

message(STATUS "========================================")
message(STATUS "Lab3 - Program with test mode")
message(STATUS "  - Run normally: Lab3.exe")
message(STATUS "  - Run tests:    Lab3.exe --test")
message(STATUS "========================================")