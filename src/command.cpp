#include "include/command.h"
#include <sstream>
#include <algorithm>
#include <cctype>

std::string CommandParser::trim(const std::string &str) {
    auto start = str.begin();
    while (start != str.end() && std::isspace(*start)) {
        ++start;
    }
    
    auto end = str.end();
    do {
        --end;
    } while (std::distance(start, end) > 0 && std::isspace(*end));
    
    return std::string(start, end + 1);
}

std::vector<std::string> CommandParser::split(const std::string &line) {
    std::vector<std::string> result;
    std::istringstream iss(line);
    std::string token;
    
    bool in_quotes = false;
    std::string current_token;
    
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        
        if (c == '"') {
            in_quotes = !in_quotes;
            current_token += c;
        } else if (c == ' ' && !in_quotes) {
            if (!current_token.empty()) {
                result.push_back(trim(current_token));
                current_token.clear();
            }
        } else {
            current_token += c;
        }
    }
    
    if (!current_token.empty()) {
        result.push_back(trim(current_token));
    }
    
    return result;
}

Command CommandParser::parse(const std::string &line) {
    Command cmd;
    cmd.type = CommandType::Unknown;
    
    std::vector<std::string> tokens = split(line);
    if (tokens.empty()) return cmd;
    
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
    
    if (cmd.type != CommandType::ShowFinance) {
        for (size_t i = 1; i < tokens.size(); ++i) {
            if (!(cmd.type == CommandType::Show && i == 2 && tokens[1] == "finance")) {
                cmd.args.push_back(tokens[i]);
            }
        }
    }
    
    return cmd;
}