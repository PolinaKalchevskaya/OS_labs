#pragma once
#include <windows.h>
#include <map>
#include <shared_mutex>
#include <mutex>
#include "common/employee.h"
#include "common/messages.h"
#include "server/file_manager.h"

class RequestHandler {
public:
    RequestHandler(FileManager& fileManager);
    void processRequest(HANDLE hPipe);
    void setClientCount(int count) { m_expectedClients = count; }
    void waitForAllClients();
    void shutdown();

private:
    FileManager& m_fileManager;
    std::map<int, std::shared_mutex> m_recordLocks;
    std::map<int, std::unique_lock<std::shared_mutex>> m_activeLocks;
    std::mutex m_globalMutex;
    std::mutex m_lockMapMutex;
    int m_expectedClients;
    int m_completedClients;
    bool m_shutdown;

    void handleRead(int key, HANDLE hPipe);
    void handleModifyStart(int key, HANDLE hPipe, DWORD lockId);
    void handleModifyComplete(int key, const Employee& newData, HANDLE hPipe, DWORD lockId);
    void handleModifyCancel(int key, HANDLE hPipe, DWORD lockId);
    void sendResponse(HANDLE hPipe, const Response& resp);
    bool receiveRequest(HANDLE hPipe, Request& req);
};