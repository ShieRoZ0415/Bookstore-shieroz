#include "include/log.h"
#include <iostream>
#include <iomanip>

LogManager::LogManager() {}

void LogManager::print_separator() {
    std::cout << "========================================\n";
}

void LogManager::print_header(const std::string &title) {
    print_separator();
    std::cout << "          " << title << "\n";
    print_separator();
}

void LogManager::generate_employee_report() {
    print_header("员工工作情况报告");
    std::cout << "员工ID\t操作次数\n";
    std::cout << "root\t100\n";  // 示例数据
    std::cout << "admin\t50\n";
    print_separator();
}

void LogManager::show_log() {
    print_header("系统日志");
    std::cout << "时间\t\t用户ID\t操作\n";
    std::cout << "2024-01-01 10:00:00\troot\tlogin\n";
    std::cout << "2024-01-01 10:01:00\troot\tbuy book\n";
    print_separator();
}