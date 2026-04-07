cmake_minimum_required(VERSION 3.15)
project(lab4_os)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /EHsc")
endif()

add_executable(Receiver 
    src/Receiver/Receiver.cpp
    src/Shared/QueueHeader.h
)


add_executable(Sender 
    src/Sender/Sender.cpp
    src/Shared/QueueHeader.h
)


add_executable(UnitTest 
    tests/QueueTest.cpp
    src/Shared/QueueHeader.h
)


add_custom_command(TARGET Receiver POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Sender>
        $<TARGET_FILE_DIR:Receiver>
)

add_custom_command(TARGET UnitTest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Receiver>
        $<TARGET_FILE_DIR:UnitTest>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Sender>
        $<TARGET_FILE_DIR:UnitTest>
)