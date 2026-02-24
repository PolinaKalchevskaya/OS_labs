#pragma once

#include <windows.h>
#include <iostream>
#include <string>
#include <limits>


struct ThreadData {
    int* arr;          
    int size;           
    int* min;           
    int* max;         
    double* average;   
    HANDLE hMutex;     
};


DWORD WINAPI MinMaxThread(LPVOID lpParam);
DWORD WINAPI AverageThread(LPVOID lpParam);


int SafeInputInt(const std::string& prompt);