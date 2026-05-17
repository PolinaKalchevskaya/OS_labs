#include "server/request_handler.h"
#include <iostream>
#include <chrono>
#include <thread>

RequestHandler::RequestHandler(FileManager& fm)
    : m_fileManager(fm), m_expectedClients(0), m_completedClients(0), m_shutdown(false) {
}

void RequestHandler::sendResponse(HANDLE hPipe, const Response& resp) {
    DWORD written;
    WriteFile(hPipe, &resp, sizeof(Response), &written, nullptr);
}

bool RequestHandler::receiveRequest(HANDLE hPipe, Request& req) {
    DWORD read;
    return ReadFile(hPipe, &req, sizeof(Request), &read, nullptr) && read == sizeof(Request);
}

void RequestHandler::handleRead(int key, HANDLE hPipe) {
    std::shared_lock<std::shared_mutex> lock(m_recordLocks[key]);
    Employee emp;
    Response resp;
    resp.success = m_fileManager.readRecord(key, emp);
    resp.lockId = 0;
    if (resp.success) {
        resp.record = emp;
    }
    sendResponse(hPipe, resp);
}

void RequestHandler::handleModifyStart(int key, HANDLE hPipe, DWORD lockId) {
    {
        std::lock_guard<std::mutex> lock(m_lockMapMutex);
        if (m_activeLocks.find(key) != m_activeLocks.end()) {
            Response resp;
            resp.success = false;
            resp.errorCode = ERROR_LOCK_VIOLATION;
            resp.lockId = lockId;
            sendResponse(hPipe, resp);
            return;
        }
    }

    std::unique_lock<std::shared_mutex> lock(m_recordLocks[key]);
    Employee emp;
    Response resp;
    resp.success = m_fileManager.readRecord(key, emp);
    resp.lockId = lockId;

    if (resp.success) {
        resp.record = emp;
        {
            std::lock_guard<std::mutex> mapLock(m_lockMapMutex);
            m_activeLocks[key] = std::move(lock);
        }
    }
    sendResponse(hPipe, resp);
}

void RequestHandler::handleModifyComplete(int key, const Employee& newData, HANDLE hPipe, DWORD lockId) {
    m_fileManager.updateRecord(key, newData);
    {
        std::lock_guard<std::mutex> mapLock(m_lockMapMutex);
        auto it = m_activeLocks.find(key);
        if (it != m_activeLocks.end()) {
            it->second.unlock();
            m_activeLocks.erase(it);
        }
    }
    Response resp;
    resp.success = true;
    resp.lockId = lockId;
    sendResponse(hPipe, resp);
}

void RequestHandler::handleModifyCancel(int key, HANDLE hPipe, DWORD lockId) {
    {
        std::lock_guard<std::mutex> mapLock(m_lockMapMutex);
        auto it = m_activeLocks.find(key);
        if (it != m_activeLocks.end()) {
            it->second.unlock();
            m_activeLocks.erase(it);
        }
    }
    Response resp;
    resp.success = true;
    resp.lockId = lockId;
    sendResponse(hPipe, resp);
}

void RequestHandler::processRequest(HANDLE hPipe) {
    Request req;
    if (!receiveRequest(hPipe, req)) return;

    switch (req.op) {
    case OperationType::Read:
        handleRead(req.key, hPipe);
        break;
    case OperationType::Modify:
        handleModifyStart(req.key, hPipe, req.lockId);
        break;
    case OperationType::CompleteModify:
        handleModifyComplete(req.key, req.data, hPipe, req.lockId);
        break;
    case OperationType::CancelModify:
        handleModifyCancel(req.key, hPipe, req.lockId);
        break;
    case OperationType::Exit:
    {
        std::lock_guard<std::mutex> lock(m_globalMutex);
        m_completedClients++;
        Response resp;
        resp.success = true;
        sendResponse(hPipe, resp);
        break;
    }
    default: break;
    }
}

void RequestHandler::waitForAllClients() {
    while (m_completedClients < m_expectedClients && !m_shutdown) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void RequestHandler::shutdown() {
    m_shutdown = true;
}