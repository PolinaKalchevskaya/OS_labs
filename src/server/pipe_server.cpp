#include "server/pipe_server.h"
#include "common/pipe_constants.h"
#include <iostream>

PipeServer::PipeServer() : m_hPipe(INVALID_HANDLE_VALUE) {}

PipeServer::~PipeServer() {
    close();
}

bool PipeServer::createInstance() {
    
    m_hPipe = CreateNamedPipeW(
        PIPE_NAME,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,  
        PIPE_BUFFER_SIZE,
        PIPE_BUFFER_SIZE,
        0,
        nullptr
    );

    if (m_hPipe == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        if (error == ERROR_PIPE_BUSY) {
            
            std::wcout << L"Pipe is busy, waiting 1 second..." << std::endl;
            Sleep(1000);

            m_hPipe = CreateNamedPipeW(
                PIPE_NAME,
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES,
                PIPE_BUFFER_SIZE,
                PIPE_BUFFER_SIZE,
                0,
                nullptr
            );
        }

        if (m_hPipe == INVALID_HANDLE_VALUE) {
            std::wcerr << L"CreateNamedPipe failed: " << GetLastError() << std::endl;
            return false;
        }
    }
    return true;
}

void PipeServer::disconnect() {
    if (m_hPipe != INVALID_HANDLE_VALUE) {
        FlushFileBuffers(m_hPipe);
        DisconnectNamedPipe(m_hPipe);
    }
}

void PipeServer::close() {
    if (m_hPipe != INVALID_HANDLE_VALUE) {
        CloseHandle(m_hPipe);
        m_hPipe = INVALID_HANDLE_VALUE;
    }
}