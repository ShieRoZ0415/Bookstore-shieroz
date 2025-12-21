#include "include/command.h"
#include <sstream>
#include <algorithm>
#include <cctype>

std::string CommandParser::trim(const std::string &str) {
    size_t l = 0;
    while (l < str.size() && std::isspace(static_cast<unsigned char>(str[l]))) ++l;

    if (l == str.size()) return "";

    size_t r = str.size() - 1;
    while (r > l && std::isspace(static_cast<unsigned char>(str[r]))) --r;

    return str.substr(l, r - l + 1);
}

std::vector<std::string> CommandParser::split(const std::string &line) {
    std::vector<std::string> result;

    bool in_quotes = false;
    std::string current_token;

    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];

        if (c == '"') {
            in_quotes = !in_quotes;
            current_token += c;
            continue;
        }

        // 空格/tab 等在非引号内都作为分隔符
        if (!in_quotes && std::isspace(static_cast<unsigned char>(c))) {
            std::string t = trim(current_token);
            if (!t.empty()) result.push_back(t);
            current_token.clear();
        } else {
            current_token += c;
        }
    }

    std::string t = trim(current_token);
    if (!t.empty()) result.push_back(t);

    return result;
}


Command CommandParser::parse(const std::string &line) {
    Command cmd;
    cmd.type = CommandType::Unknown;
    
    std::vector<std::string> tokens = split(line);
    if (tokens.empty()) {
        cmd.type = CommandType::Empty;
        return cmd;
    }
    
    std::string op = tokens[0];
    
    if (op == "quit") cmd.type = CommandType::Quit;
    else if (op == "exit") cmd.type = CommandType::Exit;
    else if (op == "register") cmd.type = CommandType::Register;
    else if (op == "su") cmd.type = CommandType::Su;
    else if (op == "logout") cmd.type = CommandType::Logout;
    else if (op == "passwd") cmd.type = CommandType::Passwd;
    else if (op == "useradd") cmd.type = CommandType::UserAdd;
    else if (op == "delete") cmd.type = CommandType::DeleteUser;
    else if (op == "show") {
        if (tokens.size() > 1 && tokens[1] == "finance") {
            cmd.type = CommandType::ShowFinance;
            for (size_t i = 2; i < tokens.size(); ++i) {
                cmd.args.push_back(tokens[i]);
            }
        } else {
            cmd.type = CommandType::Show;
            for (size_t i = 1; i < tokens.size(); ++i) {
                cmd.args.push_back(tokens[i]);
            }
        }
    }
    else if (op == "buy") cmd.type = CommandType::Buy;
    else if (op == "select") cmd.type = CommandType::Select;
    else if (op == "modify") cmd.type = CommandType::Modify;
    else if (op == "import") cmd.type = CommandType::Import;
    else if (op == "log") cmd.type = CommandType::Log;
    else if (op == "report") {
        if (tokens.size() > 1) {
            if (tokens[1] == "finance") cmd.type = CommandType::ReportFinance;
            else if (tokens[1] == "employee") cmd.type = CommandType::ReportEmployee;
        }
    }
    
    // 统一填充参数
    if (cmd.type == CommandType::ShowFinance) {

    } else if (cmd.type == CommandType::Show) {

    } else if (cmd.type == CommandType::ReportFinance || cmd.type == CommandType::ReportEmployee) {
        // report finance / report employee：无额外参数
    } else if (cmd.type != CommandType::Quit && cmd.type != CommandType::Exit
               && cmd.type != CommandType::Empty && cmd.type != CommandType::Unknown) {
        for (size_t i = 1; i < tokens.size(); ++i) {
            cmd.args.push_back(tokens[i]);
        }
               }

    return cmd;
}