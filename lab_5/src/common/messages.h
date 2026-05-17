#pragma once
#include <windows.h>
#include "employee.h"

enum class OperationType : DWORD {
    Read = 1,
    Modify = 2,
    Exit = 3,
    CompleteModify = 4,
    CancelModify = 5
};

struct Request {
    OperationType op;
    int key;
    Employee data;
    DWORD lockId;
};

struct Response {
    bool success;
    DWORD errorCode;
    Employee record;
    DWORD lockId;
};