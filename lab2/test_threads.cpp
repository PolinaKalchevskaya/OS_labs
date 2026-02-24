#include <windows.h>
#include <iostream>
#include <cassert>


DWORD WINAPI MinMaxThread(LPVOID lpParam);
DWORD WINAPI AverageThread(LPVOID lpParam);


struct ThreadData {
    int* arr;
    int size;
    int* min;
    int* max;
    double* average;
    HANDLE hMutex;
};


void PrintArray(const std::string& prefix, int* arr, int size) {
    std::cout << prefix << " [";
    for (int i = 0; i < size; ++i) {
        std::cout << arr[i];
        if (i < size - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}


void TestMinMax() {
    std::cout << "\nTest 1: Testing MinMaxThread..." << std::endl;

    int testArray[] = { 5, 2, 8, 1, 9, 3 };
    int size = sizeof(testArray) / sizeof(testArray[0]);

    PrintArray("Array", testArray, size);

    int minVal = 0, maxVal = 0;
    double avgVal = 0.0;

    HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);
    assert(hMutex != NULL && "Failed to create mutex");

    ThreadData data;
    data.arr = testArray;
    data.size = size;
    data.min = &minVal;
    data.max = &maxVal;
    data.average = &avgVal;
    data.hMutex = hMutex;

    DWORD threadId;
    HANDLE hThread = CreateThread(NULL, 0, MinMaxThread, &data, 0, &threadId);
    assert(hThread != NULL && "Failed to create thread");

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    CloseHandle(hMutex);

    assert(minVal == 1 && "Min value should be 1");
    assert(maxVal == 9 && "Max value should be 9");

    std::cout << "  MinMax test PASSED (min=" << minVal << ", max=" << maxVal << ")" << std::endl;
}


void TestAverage() {
    std::cout << "\nTest 2: Testing AverageThread..." << std::endl;

    int testArray[] = { 2, 4, 6, 8, 10 };
    int size = sizeof(testArray) / sizeof(testArray[0]);

    PrintArray("Array", testArray, size);

    int minVal = 0, maxVal = 0;
    double avgVal = 0.0;

    HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);
    assert(hMutex != NULL && "Failed to create mutex");

    ThreadData data;
    data.arr = testArray;
    data.size = size;
    data.min = &minVal;
    data.max = &maxVal;
    data.average = &avgVal;
    data.hMutex = hMutex;

    DWORD threadId;
    HANDLE hThread = CreateThread(NULL, 0, AverageThread, &data, 0, &threadId);
    assert(hThread != NULL && "Failed to create thread");

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    CloseHandle(hMutex);


    assert(avgVal == 6.0 && "Average should be 6.0");

    std::cout << "  Average test PASSED (average=" << avgVal << ")" << std::endl;
}


void TestSynchronization() {
    std::cout << "\nTest 3: Testing synchronization with mutex..." << std::endl;

    int testArray[] = { 1, 2, 3, 4, 5 };
    int size = sizeof(testArray) / sizeof(testArray[0]);

    PrintArray("Array", testArray, size);

    int minVal = 0, maxVal = 0;
    double avgVal = 0.0;

    HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);
    assert(hMutex != NULL && "Failed to create mutex");

    ThreadData data;
    data.arr = testArray;
    data.size = size;
    data.min = &minVal;
    data.max = &maxVal;
    data.average = &avgVal;
    data.hMutex = hMutex;

    DWORD threadId1, threadId2;
    HANDLE hThreads[2];

    hThreads[0] = CreateThread(NULL, 0, MinMaxThread, &data, 0, &threadId1);
    hThreads[1] = CreateThread(NULL, 0, AverageThread, &data, 0, &threadId2);

    assert(hThreads[0] != NULL && hThreads[1] != NULL && "Failed to create threads");

    WaitForMultipleObjects(2, hThreads, TRUE, INFINITE);

    CloseHandle(hThreads[0]);
    CloseHandle(hThreads[1]);
    CloseHandle(hMutex);

    assert(minVal == 1 && "Min should be 1");
    assert(maxVal == 5 && "Max should be 5");
    assert(avgVal == 3.0 && "Average should be 3.0");

    std::cout << "  Synchronization test PASSED" << std::endl;
}


int main() {
  
    std::cout << "  Running Unit Tests for Threads Lab" << std::endl;
  

    TestMinMax();
    TestAverage();
    TestSynchronization();

    std::cout << "\nAll tests PASSED successfully!" << std::endl;
    return 0;
}