#pragma once
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include <iostream>
#include <algorithm>
#include "MemoryRiver.h"

struct LogEntry {
    char user[31];
    char type[8];
    char action[128];

    LogEntry() {
        std::memset(user, 0, sizeof(user));
        std::memset(type, 0, sizeof(type));
        std::memset(action, 0, sizeof(action));
    }
};

class LogManager {
public:
    LogManager();

    void record_sys(const std::string &user, const std::string &action);
    void record_fin(const std::string &user, const std::string &action);

    void show_log();
    void generate_employee_report();

private:
    MemoryRiver<LogEntry> file;
};
