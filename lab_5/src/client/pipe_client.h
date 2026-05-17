#pragma once
#include <windows.h>
#include "common/pipe_constants.h"

class PipeClient {
public:
    PipeClient();
    ~PipeClient();
    bool connect();
    void disconnect();
    bool isConnected() const;
    HANDLE getHandle() const { return m_hPipe; }

private:
    HANDLE m_hPipe;
};