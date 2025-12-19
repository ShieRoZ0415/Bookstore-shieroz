#pragma once
#include <string>

#include "MemoryRiver.h"

struct FinanceRecord {
    bool is_income;
    double amount;

    FinanceRecord(bool income = false, double amt = 0.0)
        : is_income(income), amount(amt) {}
};

class FinanceManager {
public:
    FinanceManager();

    void add_income(double amount);
    void add_expense(double amount);
    void show_last_n(int n);
    void show_all();
    void generate_report();

private:
    MemoryRiver<FinanceRecord, 3> finance_file;  // info1: 总收入, info2: 总支出, info3: 记录数
};