#include "include/finance.h"

#include <iostream>
#include <iomanip>
#include <fstream>

FinanceManager::FinanceManager()
    : finance_file("finance.dat") {
    std::ifstream fin("finance.dat", std::ios::binary);
    bool need_init = false;
    if (!fin.good()) {
        need_init = true;
    } else {
        fin.seekg(0, std::ios::end);
        if (fin.tellg() < static_cast<std::streamoff>(3 * sizeof(int))) need_init = true;
    }
    fin.close();
    if (need_init) finance_file.initialise();
}

void FinanceManager::add_income(double amount) {
    FinanceRecord record(true, amount);
    finance_file.write(record);

    // 读取当前总收入（以分为单位，避免浮点误差）
    int total_income_cents = 0;
    finance_file.get_info(total_income_cents, 1);
    // 增加收入（转换为分）
    total_income_cents += static_cast<int>(amount * 100 + 0.5); // 四舍五入
    finance_file.write_info(total_income_cents, 1);

    // 更新记录数
    int count = 0;
    finance_file.get_info(count, 3);
    finance_file.write_info(count + 1, 3);
}

void FinanceManager::add_expense(double amount) {
    FinanceRecord record(false, amount);
    finance_file.write(record);

    // 读取当前总支出（以分为单位）
    int total_expense_cents = 0;
    finance_file.get_info(total_expense_cents, 2);
    total_expense_cents += static_cast<int>(amount * 100 + 0.5);
    finance_file.write_info(total_expense_cents, 2);

    // 更新记录数
    int count = 0;
    finance_file.get_info(count, 3);
    finance_file.write_info(count + 1, 3);
}

void FinanceManager::show_last_n(int n) {
    int total_count = 0;
    finance_file.get_info(total_count, 3);

    if (n > total_count) {
        std::cout << "Invalid\n";
        return;
    }

    if (n == 0) {
        std::cout << "\n";
        return;
    }

    double income = 0, expense = 0;
    int start = total_count - n + 1;

    for (int i = start; i <= total_count; ++i) {
        int pos = sizeof(int) * 3 + (i - 1) * sizeof(FinanceRecord);
        FinanceRecord record;
        finance_file.read(record, pos);

        if (record.is_income) {
            income += record.amount;
        } else {
            expense += record.amount;
        }
    }

    std::cout << std::fixed << std::setprecision(2)
              << "+ " << income << " - " << expense << '\n';
}

void FinanceManager::show_all() {
    // 读取总收入和总支出（以分为单位）
    int total_income_cents = 0, total_expense_cents = 0;
    finance_file.get_info(total_income_cents, 1);
    finance_file.get_info(total_expense_cents, 2);

    // 转换为元
    double income = total_income_cents / 100.0;
    double expense = total_expense_cents / 100.0;

    std::cout << std::fixed << std::setprecision(2)
              << "+ " << income << " - " << expense << '\n';
}

void FinanceManager::generate_report() {
    // 读取总收入和总支出（以分为单位）
    int total_income_cents = 0, total_expense_cents = 0;
    finance_file.get_info(total_income_cents, 1);
    finance_file.get_info(total_expense_cents, 2);

    // 转换为元
    double total_income = total_income_cents / 100.0;
    double total_expense = total_expense_cents / 100.0;

    int total_count = 0;
    finance_file.get_info(total_count, 3);

    std::cout << "========================================\n";
    std::cout << "          财务报表报告\n";
    std::cout << "========================================\n";
    std::cout << "总交易笔数: " << total_count << "\n";
    std::cout << "总收入: " << std::fixed << std::setprecision(2) 
              << total_income << "\n";
    std::cout << "总支出: " << total_expense << "\n";
    std::cout << "净利润: " << (total_income - total_expense) << "\n";
    std::cout << "========================================\n";
}