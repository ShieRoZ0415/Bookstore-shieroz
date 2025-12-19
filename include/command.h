#pragma once
#include <string>
#include <vector>

enum class CommandType {
    Unknown,
    Quit,
    Exit,
    Register,
    Su,
    Logout,
    Passwd,
    UserAdd,
    DeleteUser,
    Show,
    Buy,
    Select,
    Modify,
    Import,
    ShowFinance,
    Log,
    ReportFinance,
    ReportEmployee
};

struct Command {
    CommandType type;
    std::vector<std::string> args;
};

class CommandParser {
public:
    Command parse(const std::string &line);

private:
    std::vector<std::string> split(const std::string &line);
    std::string trim(const std::string &str);
};