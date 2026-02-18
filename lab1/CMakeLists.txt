cmake_minimum_required(VERSION 3.10)
project(ProcessLab)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(Creator Creator/Creator.cpp)
add_executable(Reporter Creator/Reporter.cpp)
add_executable(Main Creator/Main.cpp)

# подключаю заголовочный файл
include_directories(Creator)

# отключаю предупреждения
if(WIN32)
    target_compile_definitions(Creator PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
    target_compile_definitions(Reporter PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
    target_compile_definitions(Main PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
endif()

# копирую тут  exe файлы
add_custom_command(TARGET Main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        $<TARGET_FILE:Creator> 
        ${CMAKE_BINARY_DIR}/Creator.exe
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        $<TARGET_FILE:Reporter> 
        ${CMAKE_BINARY_DIR}/Reporter.exe
)