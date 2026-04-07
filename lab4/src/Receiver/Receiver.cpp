#include "../Shared/QueueHeader.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

const wchar_t* READY_EVENT_NAME = L"Global\\ReceiverReadyEvent";
const wchar_t* DATA_AVAILABLE_EVENT_NAME = L"Global\\DataAvailableEvent";
const wchar_t* SENDER_READY_EVENT_PREFIX = L"Global\\SenderReadyEvent_";

int wmain(int argc, wchar_t* argv[]) {
    setlocale(LC_ALL, "Russian");

    try {
       
        wstring fileName;
        int queueCapacity;

        wcout << L"Введите имя бинарного файла: ";
        getline(wcin, fileName);

        wcout << L"Введите максимальное количество сообщений в очереди: ";
        wcin >> queueCapacity;
        wcin.ignore();

        if (queueCapacity <= 0) {
            wcerr << L"Ошибка: емкость очереди должна быть положительной" << endl;
            return 1;
        }

      
        MessageQueue queue(fileName, queueCapacity, true);
        wcout << L"Бинарный файл создан: " << fileName << endl;

       
        int senderCount;
        wcout << L"Введите количество процессов Sender: ";
        wcin >> senderCount;
        wcin.ignore();

        if (senderCount <= 0) {
            wcerr << L"Ошибка: количество Sender должно быть положительным" << endl;
            return 1;
        }

       
        HANDLE hReadyEvent = CreateEventW(nullptr, TRUE, FALSE, READY_EVENT_NAME);
        HANDLE hDataEvent = CreateEventW(nullptr, FALSE, FALSE, DATA_AVAILABLE_EVENT_NAME);

        
        vector<PROCESS_INFORMATION> senderProcesses;
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);

       
        wstring exeDir = exePath;
        size_t pos = exeDir.find_last_of(L"\\");
        if (pos != wstring::npos) {
            exeDir = exeDir.substr(0, pos + 1);
        }
        wstring senderExe = exeDir + L"Sender.exe";

        for (int i = 0; i < senderCount; ++i) {
            wstring cmdLine = L"\"" + senderExe + L"\" \"" + fileName + L"\" " + to_wstring(i);

            STARTUPINFOW si = { sizeof(si) };
            PROCESS_INFORMATION pi = {};

            wchar_t* cmdLineBuffer = new wchar_t[cmdLine.length() + 1];
            wcscpy_s(cmdLineBuffer, cmdLine.length() + 1, cmdLine.c_str());

            if (!CreateProcessW(
                senderExe.c_str(),
                cmdLineBuffer,
                nullptr, nullptr, FALSE,
                CREATE_NEW_CONSOLE,
                nullptr, nullptr,
                &si, &pi
            )) {
                delete[] cmdLineBuffer;
                wcerr << L"Ошибка при запуске Sender " << i << L": " << GetLastError() << endl;
                continue;
            }

            delete[] cmdLineBuffer;
            senderProcesses.push_back(pi);
            wcout << L"Запущен Sender " << i << L" (PID: " << pi.dwProcessId << L")" << endl;
        }

       
        wcout << L"Ожидание готовности всех Sender..." << endl;
        for (int i = 0; i < senderCount; ++i) {
            wstring eventName = SENDER_READY_EVENT_PREFIX + to_wstring(i);
            HANDLE hEvent = OpenEventW(SYNCHRONIZE, FALSE, eventName.c_str());
            if (hEvent) {
                WaitForSingleObject(hEvent, INFINITE);
                CloseHandle(hEvent);
            }
        }
        wcout << L"Все Sender готовы к работе!" << endl;

     
        wstring command;
        wcout << L"\nДоступные команды:" << endl;
        wcout << L"  read  - прочитать сообщение из бинарного файла" << endl;
        wcout << L"  exit  - завершить работу" << endl;

        while (true) {
            wcout << L"\nВведите команду: ";
            getline(wcin, command);

            if (command == L"read") {
                wcout << L"Ожидание сообщения..." << endl;

                if (queue.IsEmpty()) {
                    WaitForSingleObject(hDataEvent, INFINITE);
                }

                string message;
                if (queue.ReadMessage(message)) {
                    wcout << L"Получено сообщение: " << wstring(message.begin(), message.end()) << endl;
                    wcout << L"Осталось сообщений: " << queue.GetCount() << endl;
                }
                else {
                    wcout << L"Не удалось прочитать сообщение" << endl;
                }
            }
            else if (command == L"exit") {
                wcout << L"Завершение работы Receiver..." << endl;
                break;
            }
            else {
                wcout << L"Неизвестная команда. Используйте 'read' или 'exit'" << endl;
            }
        }

      
        for (auto& pi : senderProcesses) {
            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }

        CloseHandle(hReadyEvent);
        CloseHandle(hDataEvent);

        wcout << L"Receiver завершил работу" << endl;

    }
    catch (const exception& e) {
        wcerr << L"Ошибка: " << e.what() << endl;
        return 1;
    }

    return 0;
}