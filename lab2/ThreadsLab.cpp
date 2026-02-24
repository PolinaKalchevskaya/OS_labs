#define NOMINMAX
#include "ThreadsLab.h"


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


int main() {
   
    std::cout << "  Lab Work #2: Thread Creation" << std::endl;
  

    int size = 0;
    do {
        size = SafeInputInt("Enter array size (positive integer): ");
    } while (size <= 0);

    int* arr = new (std::nothrow) int[size];
    if (!arr) {
        std::cerr << "Error: failed to allocate memory!" << std::endl;
        return 1;
    }

    std::cout << "Enter " << size << " integer elements:" << std::endl;
    for (int i = 0; i < size; ++i) {
        arr[i] = SafeInputInt("  Element " + std::to_string(i + 1) + ": ");
    }

    int minVal = 0, maxVal = 0;
    double avgVal = 0.0;

    HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);
    if (hMutex == NULL) {
        std::cerr << "Error: Failed to create mutex!" << std::endl;
        delete[] arr;
        return 1;
    }

    ThreadData data = { arr, size, &minVal, &maxVal, &avgVal, hMutex };

    HANDLE hMinMax = CreateThread(NULL, 0, MinMaxThread, &data, 0, NULL);
    HANDLE hAverage = CreateThread(NULL, 0, AverageThread, &data, 0, NULL);

    HANDLE threads[2] = { hMinMax, hAverage };
    WaitForMultipleObjects(2, threads, TRUE, INFINITE);

    std::cout << std::endl;
    std::cout << "Original array: ";
    for (int i = 0; i < size; ++i) std::cout << arr[i] << " ";
    std::cout << std::endl;

    for (int i = 0; i < size; ++i) {
        if (arr[i] == minVal || arr[i] == maxVal) {
            arr[i] = static_cast<int>(avgVal);
        }
    }

    std::cout << "Modified array: ";
    for (int i = 0; i < size; ++i) std::cout << arr[i] << " ";
    std::cout << std::endl;

    CloseHandle(hAverage);
    CloseHandle(hMinMax);
    CloseHandle(hMutex);
    delete[] arr;

    return 0;
}