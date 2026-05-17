#include <windows.h>
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <atomic>
#include "common/employee.h"
#include "common/pipe_constants.h"
#include "server/file_manager.h"
#include "server/pipe_server.h"
#include "server/request_handler.h"

using namespace std;

atomic<bool> g_shutdownServer(false);
atomic<int> g_connectedClients(0);
mutex g_consoleMutex;

void serverCommandThread(RequestHandler* handler) {
    wstring cmd;
    while (!g_shutdownServer) {
        {
            lock_guard<mutex> lock(g_consoleMutex);
            wcout << L"\nEnter 'exit' to shutdown server: ";
        }
        wcin >> cmd;
        if (cmd == SERVER_EXIT_COMMAND) {
            {
                lock_guard<mutex> lock(g_consoleMutex);
                wcout << L"Shutting down server..." << endl;
            }
            g_shutdownServer = true;
            if (handler) handler->shutdown();
            break;
        }
    }
}

int main() {
    wstring filename;
    wcout << L"Enter filename: ";
    wcin >> filename;

    vector<Employee> employees;
    int n;
    wcout << L"Enter number of employees: ";
    wcin >> n;

    for (int i = 0; i < n; ++i) {
        int id;
        char name[10];
        double hours;
        wcout << L"Employee " << i + 1 << L":\n";
        wcout << L"  ID: "; wcin >> id;
        wcout << L"  Name: ";
        cin >> name;
        wcout << L"  Hours: "; wcin >> hours;
        employees.emplace_back(id, name, hours);
    }

    FileManager fileManager(filename);
    if (!fileManager.createAndFill(employees)) {
        wcerr << L"Failed to create file" << endl;
        return 1;
    }

    fileManager.display();

    int clientCount;
    wcout << L"Enter number of clients: ";
    wcin >> clientCount;

    RequestHandler handler(fileManager);
    handler.setClientCount(clientCount);

  
    vector<PipeServer> pipes(clientCount);
    for (int i = 0; i < clientCount; ++i) {
       
        if (i > 0) Sleep(500);

        if (!pipes[i].createInstance()) {
            wcerr << L"Failed to create pipe for client " << i + 1 << endl;
           
        }
        else {
            wcout << L"Pipe instance " << i + 1 << L" created" << endl;
        }
    }

    wcout << L"\n========================================\n";
    wcout << L"Server is ready!\n";
    wcout << L"Please start " << clientCount << L" client(s) manually.\n";
    wcout << L"Run client.exe from the same folder as server.exe\n";
    wcout << L"========================================\n\n";

    vector<thread> clientThreads;
    for (int i = 0; i < clientCount; ++i) {
        clientThreads.emplace_back([&handler, &pipes, i, clientCount]() {
            {
                lock_guard<mutex> lock(g_consoleMutex);
                wcout << L"Waiting for client " << i + 1 << L" to connect..." << endl;
            }

            
            BOOL connected = ConnectNamedPipe(pipes[i].getHandle(), nullptr);
            if (!connected && GetLastError() != ERROR_PIPE_CONNECTED) {
                {
                    lock_guard<mutex> lock(g_consoleMutex);
                    wcerr << L"ConnectNamedPipe failed for client " << i + 1
                        << L": " << GetLastError() << endl;
                }
                return;
            }

            {
                lock_guard<mutex> lock(g_consoleMutex);
                wcout << L"Client " << i + 1 << L" connected!" << endl;
            }
            g_connectedClients++;

            
            while (!g_shutdownServer) {
                handler.processRequest(pipes[i].getHandle());
                Sleep(10);
            }

            pipes[i].disconnect();
            {
                lock_guard<mutex> lock(g_consoleMutex);
                wcout << L"Client " << i + 1 << L" disconnected" << endl;
            }
            });
    }

    thread cmdThread(serverCommandThread, &handler);

   
    while (g_connectedClients < clientCount && !g_shutdownServer) {
        Sleep(100);
    }

    handler.waitForAllClients();

    g_shutdownServer = true;
    handler.shutdown();
    Sleep(1000);

    if (cmdThread.joinable()) cmdThread.join();
    for (auto& th : clientThreads) {
        if (th.joinable()) th.join();
    }
    for (auto& pipe : pipes) {
        pipe.close();
    }

    wcout << L"\nFile after modification:\n";
    fileManager.display();
    wcout << L"\nServer shutting down. Press Enter...";
    wcin.ignore();
    wcin.get();

    return 0;
}