#pragma once
#include <string>

#include "book.h"
#include "command.h"
#include "finance.h"
#include "log.h"
#include "session.h"
#include "user.h"

class Application {
public:
    Application();
    void run();

private:
    CommandParser parser;
    SessionStack sessions;
    AccountManager account_manager;
    BookManager book_manager;
    FinanceManager finance_manager;
    LogManager log_manager;

    void handle_command(const Command &cmd);
    void handle_show_finance(const std::vector<std::string> &args);
    void handle_report_finance();
    void handle_report_employee();
    void handle_log();
    void show_books_with_criteria(const std::string &criteria);
};
