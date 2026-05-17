#include "client/pipe_client.h"
#include <iostream>

PipeClient::PipeClient() : m_hPipe(INVALID_HANDLE_VALUE) {}

PipeClient::~PipeClient() {
    disconnect();
}

bool PipeClient::connect() {
    if (WaitNamedPipeW(PIPE_NAME, NMPWAIT_WAIT_FOREVER) == 0) {
        std::wcerr << L"WaitNamedPipe failed: " << GetLastError() << std::endl;
        return false;
    }

    m_hPipe = CreateFileW(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0,
        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (m_hPipe == INVALID_HANDLE_VALUE) {
        std::wcerr << L"CreateFile failed: " << GetLastError() << std::endl;
        return false;
    }

    DWORD mode = PIPE_READMODE_MESSAGE;
    SetNamedPipeHandleState(m_hPipe, &mode, nullptr, nullptr);
    return true;
}

void PipeClient::disconnect() {
    if (m_hPipe != INVALID_HANDLE_VALUE) {
        CloseHandle(m_hPipe);
        m_hPipe = INVALID_HANDLE_VALUE;
    }
}

bool PipeClient::isConnected() const {
    return m_hPipe != INVALID_HANDLE_VALUE;
}