#pragma once
#include <windows.h>

class PipeServer {
public:
    PipeServer();
    ~PipeServer();
    bool createInstance();
    void disconnect();
    HANDLE getHandle() const { return m_hPipe; }
    void close();

private:
    HANDLE m_hPipe;
};