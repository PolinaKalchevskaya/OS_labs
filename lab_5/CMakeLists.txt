cmake_minimum_required(VERSION 3.10)
project(lab5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/src)


add_executable(server
    src/server/server_main.cpp
    src/server/file_manager.cpp
    src/server/pipe_server.cpp
    src/server/request_handler.cpp
)


add_executable(client
    src/client/client_main.cpp
    src/client/pipe_client.cpp
)


add_executable(tests
    tests/test_employee.cpp
)

if(WIN32)
    target_link_libraries(server)
    target_link_libraries(client)
    target_link_libraries(tests)
endif()