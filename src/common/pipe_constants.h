#pragma once

#define PIPE_NAME L"\\\\.\\pipe\\employee_pipe"
#define PIPE_BUFFER_SIZE 4096
#define SERVER_EXIT_COMMAND L"exit"
#define MAX_CLIENTS 10

#ifndef ERROR_LOCK_VIOLATION
#define ERROR_LOCK_VIOLATION 0x0000009FL
#endif