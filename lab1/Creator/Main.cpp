#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <limits>
#include "employee.h"

template<typename T>
T SafeInput(const std::string& prompt) {
    T value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cerr << "Error: enter a valid number!" << std::endl;
    }
}

std::string SafeInputString(const std::string& prompt) {
    std::string input;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, input);
        if (!input.empty()) {
            return input;
        }
        std::cerr << "Error: string cannot be empty!" << std::endl;
    }
}

bool RunProcessAndWait(const std::string& commandLine) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    char* cmdLine = _strdup(commandLine.c_str());

    if (!CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE,
        CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
        std::cerr << "Error starting process: " << GetLastError() << std::endl;
        free(cmdLine);
        return false;
    }

    free(cmdLine);

    std::cout << "Waiting for process to complete..." << std::endl;
    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    std::cout << "Process completed." << std::endl;
    return true;
}

void DisplayBinaryFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open file for viewing!" << std::endl;
        return;
    }

    std::cout << "\n  Binary file contents " << std::endl;

    employee emp;
    int recordNum = 1;

    while (file.read(reinterpret_cast<char*>(&emp), sizeof(employee))) {
        std::cout << "Record " << recordNum++ << ": ID=" << emp.num
            << ", Name=" << emp.name
            << ", Hours=" << std::fixed << std::setprecision(2) << emp.hours << std::endl;
    }

    file.close();
    std::cout << "-----------------------------\n" << std::endl;
}

void DisplayReport(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open report file!" << std::endl;
        return;
    }

    std::cout << "\n Report contents " << std::endl;

    std::string line;
    while (std::getline(file, line)) {
        std::cout << line << std::endl;
    }

    file.close();
    std::cout << "------------------------\n" << std::endl;
}

int main() {
    std::cout << "Lab Work #1: Process Creation " << std::endl;

    std::string binFilename = SafeInputString("Enter binary file name: ");
    int recordCount = SafeInput<int>("Enter number of records: ");

    if (recordCount <= 0) {
        std::cerr << "Error: number of records must be positive!" << std::endl;
        return 1;
    }

    std::cout << "\nStarting Creator" << std::endl;
    std::string creatorCmd = "Creator.exe \"" + binFilename + "\" " + std::to_string(recordCount);

    if (!RunProcessAndWait(creatorCmd)) {
        std::cerr << "Error executing Creator" << std::endl;
        return 1;
    }

    DisplayBinaryFile(binFilename);

    std::string reportFilename = SafeInputString("Enter report file name: ");
    double hourlyRate = SafeInput<double>("Enter hourly rate: ");

    if (hourlyRate <= 0) {
        std::cerr << "Error: hourly rate must be positive!" << std::endl;
        return 1;
    }

    std::cout << "\nStarting Reporter" << std::endl;
    std::string reporterCmd = "Reporter.exe \"" + binFilename + "\" \"" +
        reportFilename + "\" " + std::to_string(hourlyRate);

    if (!RunProcessAndWait(reporterCmd)) {
        std::cerr << "Error executing Reporter" << std::endl;
        return 1;
    }

    DisplayReport(reportFilename);

    std::cout << "Main program finished." << std::endl;
    return 0;
}