#include "../Shared/QueueHeader.h"
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

const wchar_t* DATA_AVAILABLE_EVENT_NAME = L"Global\\DataAvailableEvent";
const wchar_t* SENDER_READY_EVENT_PREFIX = L"Global\\SenderReadyEvent_";

int wmain(int argc, wchar_t* argv[]) {
    setlocale(LC_ALL, "Russian");

    try {
        if (argc < 3) {
            wcerr << L"Использование: Sender.exe <filename> <sender_id>" << endl;
            return 1;
        }

        wstring fileName = argv[1];
        int senderId = _wtoi(argv[2]);

        wcout << L"Sender " << senderId << L" запущен" << endl;

     
        MessageQueue queue(fileName, 10, false);
        wcout << L"Бинарный файл открыт: " << fileName << endl;

       
        wstring readyEventName = SENDER_READY_EVENT_PREFIX + to_wstring(senderId);
        HANDLE hReadyEvent = CreateEventW(nullptr, TRUE, FALSE, readyEventName.c_str());
        if (hReadyEvent) {
            SetEvent(hReadyEvent);
            CloseHandle(hReadyEvent);
        }

        HANDLE hDataEvent = OpenEventW(EVENT_MODIFY_STATE, FALSE, DATA_AVAILABLE_EVENT_NAME);

        wcout << L"\nДоступные команды:" << endl;
        wcout << L"  send <текст> - отправить сообщение (макс. 20 символов)" << endl;
        wcout << L"  exit          - завершить работу" << endl;

        wstring command;
        while (true) {
            wcout << L"\nВведите команду: ";
            getline(wcin, command);

            if (command.substr(0, 4) == L"send") {
                if (command.length() <= 5) {
                    wcout << L"Ошибка: не указано сообщение" << endl;
                    continue;
                }

                wstring wmessage = command.substr(5);
                if (wmessage.length() > MAX_MESSAGE_LEN) {
                    wcout << L"Ошибка: сообщение слишком длинное (макс. "
                        << MAX_MESSAGE_LEN << L" символов)" << endl;
                    continue;
                }

                string message(wmessage.begin(), wmessage.end());

             
                while (queue.IsFull()) {
                    wcout << L"Очередь заполнена, ожидание освобождения..." << endl;
                    Sleep(500);
                }

                if (queue.WriteMessage(message)) {
                    wcout << L"Сообщение отправлено: " << wmessage << endl;
                    if (hDataEvent) {
                        SetEvent(hDataEvent);
                    }
                }
                else {
                    wcout << L"Ошибка при отправке сообщения" << endl;
                }
            }
            else if (command == L"exit") {
                wcout << L"Sender " << senderId << L" завершает работу" << endl;
                break;
            }
            else {
                wcout << L"Неизвестная команда. Используйте 'send <текст>' или 'exit'" << endl;
            }
        }

        if (hDataEvent) CloseHandle(hDataEvent);

    }
    catch (const exception& e) {
        wcerr << L"Ошибка в Sender: " << e.what() << endl;
        return 1;
    }

    return 0;
}