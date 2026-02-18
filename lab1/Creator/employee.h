#pragma once

#include <cstdint>

#pragma pack(push, 1)  // ¬ыравнивание по границе 1 байта дл€ совместимости
struct employee {
    int32_t num;        // идентификационный номер сотрудника
    char name[10];      // им€ сотрудника
    double hours;       // количество отработанных часов

    employee() : num(0), hours(0.0) {
        name[0] = '\0';
    }
};
#pragma pack(pop)
