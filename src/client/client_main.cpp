#include <windows.h>
#include <iostream>
#include <string>
#include <random>
#include <ctime>
#include "common/employee.h"
#include "common/messages.h"
#include "client/pipe_client.h"

using namespace std;

static mt19937 rng(static_cast<unsigned>(time(nullptr)));
static uniform_int_distribution<DWORD> dist(1, 0xFFFFFFFF);

void sendRequestAndWait(HANDLE hPipe, const Request& req, Response& resp) {
    DWORD written;
    WriteFile(hPipe, &req, sizeof(Request), &written, nullptr);
    DWORD read;
    ReadFile(hPipe, &resp, sizeof(Response), &read, nullptr);
}

int main() {
    PipeClient client;
    wcout << L"Connecting to server..." << endl;

    if (!client.connect()) {
        wcerr << L"Failed to connect to server. Make sure server is running." << endl;
        wcout << L"Press Enter to exit...";
        cin.get();
        return 1;
    }

    wcout << L"Connected to server!\n";

    while (true) {
        wcout << L"\n--- Menu ---\n";
        wcout << L"1. Read record\n";
        wcout << L"2. Modify record\n";
        wcout << L"3. Exit\n";
        wcout << L"Choose: ";

        int choice;
        wcin >> choice;

        if (choice == 3) {
            Request exitReq;
            exitReq.op = OperationType::Exit;
            exitReq.key = 0;
            exitReq.lockId = 0;
            Response resp;
            sendRequestAndWait(client.getHandle(), exitReq, resp);
            wcout << L"Goodbye!\n";
            break;
        }

        int key;
        wcout << L"Enter employee ID: ";
        wcin >> key;

        if (choice == 1) {
            Request req;
            req.op = OperationType::Read;
            req.key = key;
            req.lockId = 0;

            Response resp;
            sendRequestAndWait(client.getHandle(), req, resp);

            if (resp.success) {
                wcout << L"Record found:\n";
                wcout << L"  ID: " << resp.record.num << L"\n";
                wcout << L"  Name: " << resp.record.name << L"\n";
                wcout << L"  Hours: " << resp.record.hours << L"\n";
            }
            else {
                wcout << L"Record not found\n";
            }

            wcout << L"Press Enter to continue...";
            wcin.ignore();
            wcin.get();

        }
        else if (choice == 2) {
            DWORD lockId = dist(rng);

            Request req;
            req.op = OperationType::Modify;
            req.key = key;
            req.lockId = lockId;

            Response resp;
            sendRequestAndWait(client.getHandle(), req, resp);

            if (!resp.success) {
                if (resp.errorCode == ERROR_LOCK_VIOLATION) {
                    wcout << L"Record is being modified by another client. Try again later.\n";
                }
                else {
                    wcout << L"Record not found\n";
                }
                wcout << L"Press Enter to continue...";
                wcin.ignore();
                wcin.get();
                continue;
            }

            wcout << L"Current record:\n";
            wcout << L"  ID: " << resp.record.num << L"\n";
            wcout << L"  Name: " << resp.record.name << L"\n";
            wcout << L"  Hours: " << resp.record.hours << L"\n";

            Employee newData = resp.record;
            wcout << L"\nEnter new data:\n";
            wcout << L"  ID (" << newData.num << L"): ";
            wcin >> newData.num;
            wcout << L"  Name (" << newData.name << L"): ";
            cin >> newData.name;
            wcout << L"  Hours (" << newData.hours << L"): ";
            wcin >> newData.hours;

            wcout << L"Confirm modification? (y/n): ";
            char confirm;
            cin >> confirm;

            if (confirm == 'y' || confirm == 'Y') {
                Request completeReq;
                completeReq.op = OperationType::CompleteModify;
                completeReq.key = key;
                completeReq.data = newData;
                completeReq.lockId = lockId;
                sendRequestAndWait(client.getHandle(), completeReq, resp);
                wcout << L"Modification completed!\n";
            }
            else {
                Request cancelReq;
                cancelReq.op = OperationType::CancelModify;
                cancelReq.key = key;
                cancelReq.lockId = lockId;
                sendRequestAndWait(client.getHandle(), cancelReq, resp);
                wcout << L"Modification cancelled.\n";
            }

            wcout << L"Press Enter to continue...";
            wcin.ignore();
            wcin.get();
        }
    }

    client.disconnect();
    return 0;
}