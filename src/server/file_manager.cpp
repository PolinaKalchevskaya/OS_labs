#include "server/file_manager.h"
#include <iostream>

FileManager::FileManager(const std::wstring& filename)
    : m_filename(filename), m_hFile(INVALID_HANDLE_VALUE) {
}

FileManager::~FileManager() {
    if (m_hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(m_hFile);
    }
}

bool FileManager::isOpen() const {
    return m_hFile != INVALID_HANDLE_VALUE;
}

bool FileManager::createAndFill(const std::vector<Employee>& employees) {
    if (m_hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(m_hFile);
    }

    m_hFile = CreateFileW(m_filename.c_str(), GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ, nullptr, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, nullptr);

    if (m_hFile == INVALID_HANDLE_VALUE) {
        std::wcerr << L"Error creating file: " << GetLastError() << std::endl;
        return false;
    }

    DWORD written;
    for (const auto& emp : employees) {
        if (!WriteFile(m_hFile, &emp, sizeof(Employee), &written, nullptr) ||
            written != sizeof(Employee)) {
            return false;
        }
    }
    FlushFileBuffers(m_hFile);
    return true;
}

void FileManager::display() const {
    if (m_hFile == INVALID_HANDLE_VALUE) {
        std::wcout << L"File is not open" << std::endl;
        return;
    }

    DWORD oldPos = SetFilePointer(m_hFile, 0, nullptr, FILE_CURRENT);
    SetFilePointer(m_hFile, 0, nullptr, FILE_BEGIN);

    Employee emp;
    DWORD read;
    std::wcout << L"\n--- File contents ---\n";
    while (ReadFile(m_hFile, &emp, sizeof(Employee), &read, nullptr) && read == sizeof(Employee)) {
        std::wcout << L"ID: " << emp.num << L", Name: " << emp.name << L", Hours: " << emp.hours << std::endl;
    }
    std::wcout << L"---------------------\n";

    SetFilePointer(m_hFile, oldPos, nullptr, FILE_BEGIN);
}

bool FileManager::readRecord(int key, Employee& out) const {
    if (m_hFile == INVALID_HANDLE_VALUE) return false;

    DWORD oldPos = SetFilePointer(m_hFile, 0, nullptr, FILE_CURRENT);
    SetFilePointer(m_hFile, 0, nullptr, FILE_BEGIN);

    Employee emp;
    DWORD read;
    while (ReadFile(m_hFile, &emp, sizeof(Employee), &read, nullptr) && read == sizeof(Employee)) {
        if (emp.num == key) {
            out = emp;
            SetFilePointer(m_hFile, oldPos, nullptr, FILE_BEGIN);
            return true;
        }
    }

    SetFilePointer(m_hFile, oldPos, nullptr, FILE_BEGIN);
    return false;
}

bool FileManager::updateRecord(int key, const Employee& data) {
    if (m_hFile == INVALID_HANDLE_VALUE) return false;

    DWORD oldPos = SetFilePointer(m_hFile, 0, nullptr, FILE_CURRENT);
    SetFilePointer(m_hFile, 0, nullptr, FILE_BEGIN);

    Employee emp;
    DWORD read;
    LONG offset = 0;
    while (ReadFile(m_hFile, &emp, sizeof(Employee), &read, nullptr) && read == sizeof(Employee)) {
        if (emp.num == key) {
            SetFilePointer(m_hFile, offset, nullptr, FILE_BEGIN);
            DWORD written;
            WriteFile(m_hFile, &data, sizeof(Employee), &written, nullptr);
            FlushFileBuffers(m_hFile);
            SetFilePointer(m_hFile, oldPos, nullptr, FILE_BEGIN);
            return true;
        }
        offset += sizeof(Employee);
    }

    SetFilePointer(m_hFile, oldPos, nullptr, FILE_BEGIN);
    return false;
}