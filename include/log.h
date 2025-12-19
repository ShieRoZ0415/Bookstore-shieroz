#pragma once
#include <string>

class LogManager {
public:
    LogManager();

    void generate_employee_report();
    void show_log();

private:
    void print_header(const std::string &title);
    void print_separator();
};