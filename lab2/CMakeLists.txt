cmake_minimum_required(VERSION 3.10)
project(ThreadsLab)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(ThreadsLab 
    ThreadsLab.cpp
    ThreadsLib.cpp  
    ThreadsLab.h
)


add_executable(ThreadTests 
    test_threads.cpp
    ThreadsLib.cpp   
    ThreadsLab.h
)

if(WIN32)
    target_compile_definitions(ThreadsLab PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
    target_compile_definitions(ThreadTests PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
endif()