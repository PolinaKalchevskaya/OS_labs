
#include <windows.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>

 
class ArrayManager {
private:
    std::vector<int> m_array;
    mutable std::mutex m_mutex;

public:
    ArrayManager(int size) : m_array(size, 0) {}

    int GetSize() const {
        return static_cast<int>(m_array.size());
    }

    int GetElement(int index) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (index < 0 || index >= static_cast<int>(m_array.size())) {
            return -1;
        }
        return m_array[index];
    }

    bool MarkElement(int index, int markerNumber) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (index < 0 || index >= static_cast<int>(m_array.size())) {
            return false;
        }
        if (m_array[index] == 0) {
            m_array[index] = markerNumber;
            return true;
        }
        return false;
    }

    int ClearMarker(int markerNumber) {
        std::lock_guard<std::mutex> lock(m_mutex);
        int clearedCount = 0;
        for (auto& val : m_array) {
            if (val == markerNumber) {
                val = 0;
                clearedCount++;
            }
        }
        return clearedCount;
    }

    void Reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::fill(m_array.begin(), m_array.end(), 0);
    }
};


bool TestInitialization() {
    ArrayManager arr(10);
    if (arr.GetSize() != 10) return false;
    for (int i = 0; i < arr.GetSize(); ++i) {
        if (arr.GetElement(i) != 0) return false;
    }
    return true;
}

bool TestMarkElement() {
    ArrayManager arr(5);

    if (!arr.MarkElement(0, 1)) return false;
    if (arr.GetElement(0) != 1) return false;

    if (arr.MarkElement(0, 2)) return false;
    if (arr.GetElement(0) != 1) return false;

    if (!arr.MarkElement(2, 2)) return false;
    if (arr.GetElement(2) != 2) return false;

    if (arr.MarkElement(10, 3)) return false;
    if (arr.MarkElement(-1, 3)) return false;

    return true;
}

bool TestClearMarker() {
    ArrayManager arr(5);

    arr.MarkElement(0, 1);
    arr.MarkElement(1, 1);
    arr.MarkElement(2, 2);
    arr.MarkElement(3, 1);

    int cleared = arr.ClearMarker(1);
    if (cleared != 3) return false;

    if (arr.GetElement(0) != 0) return false;
    if (arr.GetElement(1) != 0) return false;
    if (arr.GetElement(3) != 0) return false;
    if (arr.GetElement(2) != 2) return false;

    cleared = arr.ClearMarker(1);
    if (cleared != 0) return false;

    return true;
}

bool TestMultipleMarkers() {
    ArrayManager arr(10);

    arr.MarkElement(0, 1);
    arr.MarkElement(2, 1);
    arr.MarkElement(4, 1);
    arr.MarkElement(1, 2);
    arr.MarkElement(3, 2);
    arr.MarkElement(5, 2);

    if (arr.GetElement(0) != 1) return false;
    if (arr.GetElement(1) != 2) return false;
    if (arr.GetElement(2) != 1) return false;
    if (arr.GetElement(3) != 2) return false;
    if (arr.GetElement(4) != 1) return false;
    if (arr.GetElement(5) != 2) return false;

    arr.ClearMarker(1);

    if (arr.GetElement(0) != 0) return false;
    if (arr.GetElement(1) != 2) return false;
    if (arr.GetElement(2) != 0) return false;
    if (arr.GetElement(3) != 2) return false;

    arr.ClearMarker(2);

    for (int i = 0; i < arr.GetSize(); ++i) {
        if (arr.GetElement(i) != 0) return false;
    }

    return true;
}

bool TestMultiThreadMarking() {
    const int ARRAY_SIZE = 100;
    const int MARKER_COUNT = 5;
    const int OPERATIONS_PER_MARKER = 1000;

    ArrayManager arr(ARRAY_SIZE);
    std::vector<std::thread> threads;
    std::atomic<int> successCount{ 0 };

    for (int i = 0; i < MARKER_COUNT; ++i) {
        threads.emplace_back([&arr, i, OPERATIONS_PER_MARKER, &successCount]() {
            int markerNumber = i + 1;
            for (int op = 0; op < OPERATIONS_PER_MARKER; ++op) {
                int index = rand() % ARRAY_SIZE;
                if (arr.MarkElement(index, markerNumber)) {
                    successCount++;
                }
            }
            });
    }

    for (auto& t : threads) {
        t.join();
    }

    int totalMarked = 0;
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        if (arr.GetElement(i) != 0) {
            totalMarked++;
        }
    }

    return (successCount == totalMarked) && (totalMarked <= ARRAY_SIZE);
}

bool RunAllTests() {
    std::cout << "========================================" << std::endl;
    std::cout << "Unit Tests for Laboratory Work #3" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;

    int passed = 0;
    int total = 0;

    struct TestCase {
        const char* name;
        bool (*func)();
    };

    TestCase tests[] = {
        {"Initialization", TestInitialization},
        {"MarkElement", TestMarkElement},
        {"ClearMarker", TestClearMarker},
        {"MultipleMarkers", TestMultipleMarkers},
        {"MultiThreadMarking", TestMultiThreadMarking}
    };

    total = sizeof(tests) / sizeof(tests[0]);

    for (int i = 0; i < total; ++i) {
        std::cout << "Running test: " << tests[i].name << "... ";
        if (tests[i].func()) {
            std::cout << "PASSED" << std::endl;
            passed++;
        }
        else {
            std::cout << "FAILED" << std::endl;
        }
    }

    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Results: " << passed << "/" << total << " tests passed" << std::endl;
    std::cout << "========================================" << std::endl;

    return passed == total;
}

CRITICAL_SECTION g_csArray;
HANDLE g_hStartEvent;
HANDLE g_hResumeEvent;

std::vector<HANDLE> g_vTerminateEvents;
std::vector<HANDLE> g_vBlockedEvents;
std::vector<HANDLE> g_vMarkerThreads;

int* g_pArray = nullptr;
int g_nArraySize = 0;
int g_nMarkerCount = 0;
std::vector<int> g_vMarkedCount;
std::vector<bool> g_vIsTerminated;

volatile bool g_bAllTerminated = false;
volatile LONG g_nActiveMarkers = 0;

void PrintErrorMessage(const std::string& context) {
    LPVOID lpMsgBuf;
    DWORD dwError = GetLastError();

    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&lpMsgBuf,
        0,
        NULL
    );

    std::cerr << "ERROR in " << context << ": "
        << (char*)lpMsgBuf << " (code: " << dwError << ")" << std::endl;

    LocalFree(lpMsgBuf);
}

bool SafeInputInt(const std::string& prompt, int& value, int minValue = 1, int maxValue = INT_MAX) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);

    try {
        value = std::stoi(input);
        if (value >= minValue && value <= maxValue) {
            return true;
        }
        std::cerr << "Error: value must be in range ["
            << minValue << ", " << maxValue << "]" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Input error: " << e.what() << std::endl;
    }
    return false;
}

void PrintArray() {
    EnterCriticalSection(&g_csArray);
    std::cout << "Array: [";
    for (int i = 0; i < g_nArraySize; ++i) {
        std::cout << g_pArray[i];
        if (i < g_nArraySize - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
    LeaveCriticalSection(&g_csArray);
}

DWORD WINAPI MarkerThread(LPVOID lpParam) {
    int index = static_cast<int>(reinterpret_cast<INT_PTR>(lpParam));
    int markerNumber = index + 1;

    srand(static_cast<unsigned int>(index));
    int markedCount = 0;

    DWORD waitResult = WaitForSingleObject(g_hStartEvent, INFINITE);
    if (waitResult == WAIT_FAILED) {
        PrintErrorMessage("Marker " + std::to_string(markerNumber) + " waiting for start");
        return 1;
    }

    std::cout << "Marker " << markerNumber << " started" << std::endl;

    while (!g_bAllTerminated) {
        int randomIndex = rand() % g_nArraySize;
        bool success = false;

        EnterCriticalSection(&g_csArray);
        if (g_pArray[randomIndex] == 0) {
            g_pArray[randomIndex] = markerNumber;
            markedCount++;
            success = true;
        }
        LeaveCriticalSection(&g_csArray);

        if (success) {
            Sleep(5);
            continue;
        }

        std::cout << "Marker " << markerNumber
            << ": marked=" << markedCount
            << ", cannot mark index=" << randomIndex << std::endl;

        g_vMarkedCount[index] = markedCount;

        if (!SetEvent(g_vBlockedEvents[index])) {
            PrintErrorMessage("Marker " + std::to_string(markerNumber) + " setting blocked event");
        }

        HANDLE waitHandles[2] = { g_hResumeEvent, g_vTerminateEvents[index] };
        waitResult = WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);

        if (waitResult == WAIT_FAILED) {
            PrintErrorMessage("Marker " + std::to_string(markerNumber) + " waiting for signal");
            break;
        }

        if (waitResult == WAIT_OBJECT_0) {
            continue;
        }
        else if (waitResult == WAIT_OBJECT_0 + 1) {
            EnterCriticalSection(&g_csArray);
            int clearedCount = 0;
            for (int i = 0; i < g_nArraySize; ++i) {
                if (g_pArray[i] == markerNumber) {
                    g_pArray[i] = 0;
                    clearedCount++;
                }
            }
            LeaveCriticalSection(&g_csArray);

            std::cout << "Marker " << markerNumber
                << " terminated. Cleared: " << clearedCount << " elements" << std::endl;

            g_vIsTerminated[index] = true;
            InterlockedDecrement(&g_nActiveMarkers);

            return 0;
        }
    }

    return 0;
}

void ResetAllBlockedEvents() {
    for (int i = 0; i < g_nMarkerCount; ++i) {
        if (!g_vIsTerminated[i]) {
            ResetEvent(g_vBlockedEvents[i]);
        }
    }
}

bool WaitForAllMarkersBlocked() {
    std::vector<HANDLE> activeBlockedEvents;
    for (int i = 0; i < g_nMarkerCount; ++i) {
        if (!g_vIsTerminated[i]) {
            activeBlockedEvents.push_back(g_vBlockedEvents[i]);
        }
    }

    if (activeBlockedEvents.empty()) {
        return true;
    }

    DWORD waitResult = WaitForMultipleObjects(
        static_cast<DWORD>(activeBlockedEvents.size()),
        activeBlockedEvents.data(),
        TRUE,
        INFINITE
    );

    if (waitResult == WAIT_FAILED) {
        PrintErrorMessage("Waiting for all markers blocked");
        return false;
    }

    return true;
}

void CleanupResources() {
    if (g_hStartEvent) CloseHandle(g_hStartEvent);
    if (g_hResumeEvent) CloseHandle(g_hResumeEvent);

    for (HANDLE hEvent : g_vTerminateEvents) {
        if (hEvent) CloseHandle(hEvent);
    }
    for (HANDLE hEvent : g_vBlockedEvents) {
        if (hEvent) CloseHandle(hEvent);
    }
    for (HANDLE hThread : g_vMarkerThreads) {
        if (hThread) CloseHandle(hThread);
    }

    DeleteCriticalSection(&g_csArray);
    delete[] g_pArray;
    g_pArray = nullptr;

    std::cout << "All resources released" << std::endl;
}

int RunMainProgram() {
    std::cout << "========================================" << std::endl;
    std::cout << "Laboratory Work #3" << std::endl;
    std::cout << "Topic: Thread Synchronization" << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        int arraySize = 0;
        if (!SafeInputInt("Enter array size (1-100): ", arraySize, 1, 100)) {
            std::cerr << "Invalid array size. Program terminated." << std::endl;
            return 1;
        }
        g_nArraySize = arraySize;

        g_pArray = new (std::nothrow) int[g_nArraySize];
        if (!g_pArray) {
            std::cerr << "Error: failed to allocate memory for array" << std::endl;
            return 1;
        }

        std::fill(g_pArray, g_pArray + g_nArraySize, 0);
        std::cout << "Array initialized with zeros" << std::endl;

        int markerCount = 0;
        if (!SafeInputInt("Enter number of marker threads (1-10): ", markerCount, 1, 10)) {
            std::cerr << "Invalid marker count. Program terminated." << std::endl;
            CleanupResources();
            return 1;
        }
        g_nMarkerCount = markerCount;
        g_nActiveMarkers = markerCount;

        InitializeCriticalSection(&g_csArray);

        g_hStartEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
        g_hResumeEvent = CreateEventA(NULL, TRUE, FALSE, NULL);

        if (!g_hStartEvent || !g_hResumeEvent) {
            PrintErrorMessage("Creating start/resume events");
            CleanupResources();
            return 1;
        }

        g_vTerminateEvents.resize(g_nMarkerCount, NULL);
        g_vBlockedEvents.resize(g_nMarkerCount, NULL);
        g_vMarkerThreads.resize(g_nMarkerCount, NULL);
        g_vMarkedCount.resize(g_nMarkerCount, 0);
        g_vIsTerminated.resize(g_nMarkerCount, false);

        for (int i = 0; i < g_nMarkerCount; ++i) {
            g_vTerminateEvents[i] = CreateEventA(NULL, FALSE, FALSE, NULL);
            g_vBlockedEvents[i] = CreateEventA(NULL, FALSE, FALSE, NULL);

            if (!g_vTerminateEvents[i] || !g_vBlockedEvents[i]) {
                PrintErrorMessage("Creating events for marker " + std::to_string(i + 1));
                CleanupResources();
                return 1;
            }
        }

        std::cout << "\nStarting " << g_nMarkerCount << " marker threads..." << std::endl;
        for (int i = 0; i < g_nMarkerCount; ++i) {
            g_vMarkerThreads[i] = CreateThread(
                NULL,
                0,
                MarkerThread,
                reinterpret_cast<LPVOID>(static_cast<INT_PTR>(i)),
                0,
                NULL
            );

            if (!g_vMarkerThreads[i]) {
                PrintErrorMessage("Creating marker thread " + std::to_string(i + 1));
                CleanupResources();
                return 1;
            }
        }

        std::cout << "Sending start signal..." << std::endl;
        if (!SetEvent(g_hStartEvent)) {
            PrintErrorMessage("Setting start event");
            CleanupResources();
            return 1;
        }

        Sleep(100);

        std::cout << "\n--- Starting marker management ---" << std::endl;

        while (g_nActiveMarkers > 0) {
            std::cout << "\nWaiting for all active markers to block..." << std::endl;
            if (!WaitForAllMarkersBlocked()) {
                PrintErrorMessage("Waiting for markers blocked");
                break;
            }

            std::cout << "\n--- Array state (all markers blocked) ---" << std::endl;
            PrintArray();

            std::cout << "Marker statistics:" << std::endl;
            for (int i = 0; i < g_nMarkerCount; ++i) {
                if (!g_vIsTerminated[i]) {
                    std::cout << "  Marker " << (i + 1)
                        << ": marked elements = " << g_vMarkedCount[i] << std::endl;
                }
                else {
                    std::cout << "  Marker " << (i + 1) << ": terminated" << std::endl;
                }
            }

            int markerToTerminate = 0;
            std::string prompt = "Enter marker number to terminate (1-"
                + std::to_string(g_nMarkerCount) + "): ";

            std::vector<int> activeMarkers;
            for (int i = 0; i < g_nMarkerCount; ++i) {
                if (!g_vIsTerminated[i]) {
                    activeMarkers.push_back(i + 1);
                }
            }

            if (activeMarkers.empty()) {
                break;
            }

            std::cout << "Active markers: ";
            for (int m : activeMarkers) {
                std::cout << m << " ";
            }
            std::cout << std::endl;

            if (!SafeInputInt(prompt, markerToTerminate, 1, g_nMarkerCount)) {
                std::cout << "Try again." << std::endl;
                ResetAllBlockedEvents();
                SetEvent(g_hResumeEvent);
                Sleep(100);
                ResetEvent(g_hResumeEvent);
                continue;
            }

            int indexToTerminate = markerToTerminate - 1;

            if (g_vIsTerminated[indexToTerminate]) {
                std::cout << "Marker " << markerToTerminate << " is already terminated. Choose another." << std::endl;
                ResetAllBlockedEvents();
                SetEvent(g_hResumeEvent);
                Sleep(100);
                ResetEvent(g_hResumeEvent);
                continue;
            }

            std::cout << "Sending terminate signal to marker " << markerToTerminate << "..." << std::endl;
            if (!SetEvent(g_vTerminateEvents[indexToTerminate])) {
                PrintErrorMessage("Setting terminate event for marker " + std::to_string(markerToTerminate));
                break;
            }

            DWORD waitResult = WaitForSingleObject(g_vMarkerThreads[indexToTerminate], INFINITE);
            if (waitResult == WAIT_FAILED) {
                PrintErrorMessage("Waiting for marker " + std::to_string(markerToTerminate) + " to terminate");
                break;
            }

            std::cout << "Marker " << markerToTerminate << " terminated." << std::endl;

            std::cout << "\n--- Array state after terminating marker "
                << markerToTerminate << " ---" << std::endl;
            PrintArray();

            if (g_nActiveMarkers > 0) {
                std::cout << "Sending resume signal to remaining markers..." << std::endl;
                ResetAllBlockedEvents();
                if (!SetEvent(g_hResumeEvent)) {
                    PrintErrorMessage("Setting resume event");
                    break;
                }
                Sleep(100);
                ResetEvent(g_hResumeEvent);
            }
        }

        std::cout << "\n--- All markers terminated ---" << std::endl;
        PrintArray();

    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        CleanupResources();
        return 1;
    }

    CleanupResources();

    std::cout << "\nProgram finished." << std::endl;
    system("pause");
    return 0;
}


int main(int argc, char* argv[]) {
   
    if (argc >= 2 && (strcmp(argv[1], "--test") == 0 || strcmp(argv[1], "-t") == 0)) {
      
        bool allPassed = RunAllTests();
        std::cout << std::endl;
        system("pause");
        return allPassed ? 0 : 1;
    }

    
    return RunMainProgram();
}