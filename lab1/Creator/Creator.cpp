#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include "employee.h"

int SafeInputInt(const std::string& prompt) {
    int value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cerr << "Error: enter a valid integer!" << std::endl;
    }
}

void SafeInputString(const std::string& prompt, char* buffer, size_t bufferSize) {
    while (true) {
        std::cout << prompt;
        std::string input;
        std::getline(std::cin, input);

        if (input.empty()) {
            std::cerr << "Error: string cannot be empty!" << std::endl;
            continue;
        }

        if (input.length() >= bufferSize) {
            std::cerr << "Error: string too long! Max " << (bufferSize - 1) << " chars." << std::endl;
            continue;
        }

        strcpy_s(buffer, bufferSize, input.c_str());
        break;
    }
}

double SafeInputDouble(const std::string& prompt) {
    double value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (value >= 0) return value;
            std::cerr << "Error: value cannot be negative!" << std::endl;
        }
        else {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cerr << "Error: enter a valid number!" << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: Creator.exe <binary_file_name> <number_of_records>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    int recordCount = 0;

    try {
        recordCount = std::stoi(argv[2]);
        if (recordCount <= 0) {
            std::cerr << "Error: number of records must be positive!" << std::endl;
            return 1;
        }
    }
    catch (const std::exception&) {
        std::cerr << "Error: invalid number of records!" << std::endl;
        return 1;
    }

    std::cout << "Creating file: " << filename << std::endl;
    std::cout << "Number of records: " << recordCount << std::endl;

    std::ofstream outFile(filename, std::ios::binary | std::ios::trunc);
    if (!outFile.is_open()) {
        std::cerr << "Error: cannot create file '" << filename << "'!" << std::endl;
        return 1;
    }

    employee emp;
    for (int i = 0; i < recordCount; ++i) {
        std::cout << "\ Record  " << (i + 1) << " of " << recordCount << "   " << std::endl;

        emp.num = SafeInputInt("Enter employee ID (integer): ");
        SafeInputString("Enter employee name (max 9 chars): ", emp.name, sizeof(emp.name));
        emp.hours = SafeInputDouble("Enter hours worked: ");

        outFile.write(reinterpret_cast<const char*>(&emp), sizeof(employee));
        if (!outFile.good()) {
            std::cerr << "Error: cannot write to file!" << std::endl;
            return 1;
        }
    }

    outFile.close();
    std::cout << "\nFile created successfully!" << std::endl;

    return 0;
}