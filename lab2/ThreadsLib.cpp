#include "ThreadsLab.h"


DWORD WINAPI MinMaxThread(LPVOID lpParam) {
    ThreadData* pData = reinterpret_cast<ThreadData*>(lpParam);

    int* arr = pData->arr;
    int size = pData->size;

    int minVal = arr[0];
    int maxVal = arr[0];

    for (int i = 1; i < size; ++i) {
        if (arr[i] < minVal) minVal = arr[i];
        if (arr[i] > maxVal) maxVal = arr[i];
        Sleep(7);
    }

    if (pData->hMutex) WaitForSingleObject(pData->hMutex, INFINITE);

    *pData->min = minVal;
    *pData->max = maxVal;
    std::cout << "MinMax: Minimum = " << minVal << ", Maximum = " << maxVal << std::endl;

    if (pData->hMutex) ReleaseMutex(pData->hMutex);

    return 0;
}


DWORD WINAPI AverageThread(LPVOID lpParam) {
    ThreadData* pData = reinterpret_cast<ThreadData*>(lpParam);

    int* arr = pData->arr;
    int size = pData->size;
    long long sum = 0;

    for (int i = 0; i < size; ++i) {
        sum += arr[i];
        Sleep(12);
    }

    if (pData->hMutex) WaitForSingleObject(pData->hMutex, INFINITE);

    double avg = static_cast<double>(sum) / size;
    *pData->average = avg;
    std::cout << "Average: Average = " << avg << std::endl;

    if (pData->hMutex) ReleaseMutex(pData->hMutex);

    return 0;
}