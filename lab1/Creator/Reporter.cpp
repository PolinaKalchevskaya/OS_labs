#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include "employee.h"

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: Reporter.exe <binary_file> <report_file> <hourly_rate>" << std::endl;
        return 1;
    }

    std::string binFilename = argv[1];
    std::string reportFilename = argv[2];
    double hourlyRate = 0.0;

    try {
        hourlyRate = std::stod(argv[3]);
        if (hourlyRate <= 0) {
            std::cerr << "Error: hourly rate must be positive!" << std::endl;
            return 1;
        }
    }
    catch (const std::exception&) {
        std::cerr << "Error: invalid hourly rate!" << std::endl;
        return 1;
    }

    std::cout << "Reading binary file: " << binFilename << std::endl;
    std::cout << "Creating report: " << reportFilename << std::endl;
    std::cout << "Hourly rate: " << hourlyRate << std::endl;

    std::ifstream inFile(binFilename, std::ios::binary);
    if (!inFile.is_open()) {
        std::cerr << "Error: cannot open binary file '" << binFilename << "'!" << std::endl;
        return 1;
    }

    std::vector<employee> employees;
    employee emp;

    while (inFile.read(reinterpret_cast<char*>(&emp), sizeof(employee))) {
        employees.push_back(emp);
    }

    inFile.close();

    if (employees.empty()) {
        std::cerr << "Error: file contains no records!" << std::endl;
        return 1;
    }

    std::sort(employees.begin(), employees.end(),
        [](const employee& a, const employee& b) {
            return a.num < b.num;
        });

    std::ofstream reportFile(reportFilename);
    if (!reportFile.is_open()) {
        std::cerr << "Error: cannot create report file '" << reportFilename << "'!" << std::endl;
        return 1;
    }

    reportFile << "Report on file \"" << binFilename << "\"\n";
    reportFile << "Employee ID, Name, Hours, Salary\n";

    for (const auto& e : employees) {
        double salary = e.hours * hourlyRate;
        reportFile << e.num << ", "
            << e.name << ", "
            << std::fixed << std::setprecision(2) << e.hours << ", "
            << std::fixed << std::setprecision(2) << salary << "\n";
    }

    reportFile.close();
    std::cout << "Report created successfully!" << std::endl;

    return 0;
}