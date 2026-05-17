#pragma once
#include <cstring>

struct Employee {
    int num;
    char name[10];
    double hours;

    Employee() : num(0), hours(0.0) {
        name[0] = '\0';
    }

    Employee(int n, const char* nm, double h) : num(n), hours(h) {
       
        int len = strlen(nm);
        int copyLen = (len < 9) ? len : 9;
        for (int i = 0; i < copyLen; i++) {
            name[i] = nm[i];
        }
        name[copyLen] = '\0';
    }
};