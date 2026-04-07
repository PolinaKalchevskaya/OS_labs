#pragma once
#include <windows.h>
#include <string>
#include <cstring>
#include <stdexcept>

const int MAX_MESSAGE_LEN = 20;   


#pragma pack(push, 1)
struct QueueHeader {
    int head;         
    int tail;        
    int count;         
    int capacity;     
    DWORD magic;       
};
#pragma pack(pop)


#pragma pack(push, 1)
struct Message {
    char data[MAX_MESSAGE_LEN + 1];  
    bool isValid;                     
};
#pragma pack(pop)


class MessageQueue {
private:
    HANDLE m_hFile;          
    HANDLE m_hFileMapping;  
    LPVOID m_pView;         
    QueueHeader* m_pHeader;  
    Message* m_pMessages;    
    std::wstring m_filePath;

public:
    MessageQueue(const std::wstring& filePath, int capacity, bool createNew = true)
        : m_hFile(nullptr), m_hFileMapping(nullptr), m_pView(nullptr), m_filePath(filePath) {

        DWORD dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
        DWORD dwCreationDisposition = createNew ? CREATE_ALWAYS : OPEN_EXISTING;

        m_hFile = CreateFileW(filePath.c_str(), dwDesiredAccess,
            FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
            dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (m_hFile == INVALID_HANDLE_VALUE)
            throw std::runtime_error("Cannot create/open file");

        DWORD fileSize = sizeof(QueueHeader) + capacity * sizeof(Message);

        if (createNew) {
            LARGE_INTEGER li;
            li.QuadPart = fileSize;
            SetFilePointerEx(m_hFile, li, nullptr, FILE_BEGIN);
            SetEndOfFile(m_hFile);
        }

        m_hFileMapping = CreateFileMappingW(m_hFile, nullptr, PAGE_READWRITE, 0, fileSize, nullptr);
        if (!m_hFileMapping)
            throw std::runtime_error("Cannot create file mapping");

        m_pView = MapViewOfFile(m_hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, fileSize);
        if (!m_pView)
            throw std::runtime_error("Cannot map view");

        m_pHeader = reinterpret_cast<QueueHeader*>(m_pView);
        m_pMessages = reinterpret_cast<Message*>(reinterpret_cast<char*>(m_pView) + sizeof(QueueHeader));

        if (createNew) {
            m_pHeader->head = 0;
            m_pHeader->tail = 0;
            m_pHeader->count = 0;
            m_pHeader->capacity = capacity;
            m_pHeader->magic = 0xDEADBEEF;
            for (int i = 0; i < capacity; ++i)
                m_pMessages[i].isValid = false;
        }
        else {
         
            if (m_pHeader->magic != 0xDEADBEEF)
                throw std::runtime_error("Invalid queue file format");
        }
    }

    ~MessageQueue() {
        if (m_pView) UnmapViewOfFile(m_pView);
        if (m_hFileMapping) CloseHandle(m_hFileMapping);
        if (m_hFile && m_hFile != INVALID_HANDLE_VALUE) CloseHandle(m_hFile);
    }

    bool WriteMessage(const std::string& message) {
        if (message.length() > MAX_MESSAGE_LEN) return false;
        if (m_pHeader->count >= m_pHeader->capacity) return false;

        Message* pMsg = &m_pMessages[m_pHeader->tail];
        strncpy_s(pMsg->data, MAX_MESSAGE_LEN + 1, message.c_str(), _TRUNCATE);
        pMsg->isValid = true;

        m_pHeader->tail = (m_pHeader->tail + 1) % m_pHeader->capacity;
        m_pHeader->count++;

        FlushViewOfFile(pMsg, sizeof(Message));
        FlushViewOfFile(m_pHeader, sizeof(QueueHeader));
        return true;
    }

    bool ReadMessage(std::string& message) {
        if (m_pHeader->count == 0) return false;

        Message* pMsg = &m_pMessages[m_pHeader->head];
        if (!pMsg->isValid) return false;

        message = std::string(pMsg->data);
        pMsg->isValid = false;

        m_pHeader->head = (m_pHeader->head + 1) % m_pHeader->capacity;
        m_pHeader->count--;

        FlushViewOfFile(pMsg, sizeof(Message));
        FlushViewOfFile(m_pHeader, sizeof(QueueHeader));
        return true;
    }

    bool IsEmpty() const { return m_pHeader->count == 0; }
    bool IsFull() const { return m_pHeader->count >= m_pHeader->capacity; }
    int GetCount() const { return m_pHeader->count; }
    int GetCapacity() const { return m_pHeader->capacity; }
};
#pragma once
