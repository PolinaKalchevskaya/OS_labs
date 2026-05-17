#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include "common/employee.h"

class FileManager {
public:
    FileManager(const std::wstring& filename);
    ~FileManager();
    bool createAndFill(const std::vector<Employee>& employees);
    void display() const;
    bool readRecord(int key, Employee& out) const;
    bool updateRecord(int key, const Employee& data);

private:
    std::wstring m_filename;
    HANDLE m_hFile;
    bool isOpen() const;
};