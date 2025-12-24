#include "include/application.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <regex>
#include <cctype>
#include <limits>

static bool parse_int_strict(const std::string& s, int& out) {
    if (s.empty()) return false;
    for (unsigned char c : s) if (!std::isdigit(c)) return false;

    // 前导 0 非法
    if (s.size() > 1 && s[0] == '0') return false;

    try {
        std::size_t p = 0;
        long long v = std::stoll(s, &p);
        if (p != s.size()) return false;
        if (v < std::numeric_limits<int>::min() || v > std::numeric_limits<int>::max()) return false;
        out = static_cast<int>(v);
        return true;
    } catch (...) {
        return false;
    }
}

static bool parse_price_strict(const std::string& s, double& out) {
    if (s.empty()) return false;
    if (s.size() > 13) return false;
    if (s[0] == '+' || s[0] == '-') return false;

    std::size_t dot = s.find('.');
    if (dot != std::string::npos && s.find('.', dot + 1) != std::string::npos) return false;

    std::string int_part = (dot == std::string::npos) ? s : s.substr(0, dot);
    std::string frac_part = (dot == std::string::npos) ? "" : s.substr(dot + 1);

    // 必须有整数部分
    if (int_part.empty()) return false;

    for (unsigned char c : int_part) if (!std::isdigit(c)) return false;

    // 前导 0 非法
    if (int_part.size() > 1 && int_part[0] == '0') return false;

    if (dot != std::string::npos) {
        // 小数点后必须有数字
        if (frac_part.empty()) return false;
        // 小数位数超过两位非法
        if (frac_part.size() > 2) return false;
        for (unsigned char c : frac_part) if (!std::isdigit(c)) return false;
    }

    try {
        std::size_t p = 0;
        double v = std::stod(s, &p);
        if (p != s.size()) return false;
        if (v < 0) return false;
        out = v;
        return true;
    } catch (...) {
        return false;
    }
}


Application::Application()
    : account_manager(), book_manager(), finance_manager(), log_manager() {
    account_manager.initialize();
}

void Application::run() {
    std::string line;
    while (std::getline(std::cin, line)) {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

        bool all_space = true;
        for (unsigned char ch : line) {
            if (ch != ' ') {
                all_space = false; break;
            }  // 只认空格
        }
        if (all_space) continue;

        bool bad = false;
        for (unsigned char ch : line) {
            if (ch == ' ') continue;
            if (ch > 127 || ch < 32) { bad = true; break; }
            // 不可见字符非法；任何非空格空白符非法
            if (!std::isprint(ch) || (std::isspace(ch) && ch != ' ')) {
                bad = true;
                break;
            }
        }
        if (bad) {
            std::cout << "Invalid\n";
            continue;
        }

        Command cmd = parser.parse(line);


        if (cmd.type == CommandType::Quit || cmd.type == CommandType::Exit) {
            break;
        }
        
        handle_command(cmd);
    }
}

void Application::handle_command(const Command &cmd) {
    int privilege = sessions.current_privilege();
    
    switch (cmd.type) {
    case CommandType::Empty:
        break;
    case CommandType::Register:
        if (cmd.args.size() == 3) {
            bool ok = account_manager.register_user(cmd.args[0], cmd.args[1], cmd.args[2]);
            if (!ok) std::cout << "Invalid\n";
        } else {
            std::cout << "Invalid\n";
        }
        break;
        
    case CommandType::Su: {
        if (cmd.args.size() == 1 || cmd.args.size() == 2) {
            std::string user_id = cmd.args[0];
            std::string password;
            bool has_password = false;
            if (cmd.args.size() == 2) {
                password = cmd.args[1];
                has_password = true;
            }
            User user;
            bool ok = account_manager.login(user_id, password, has_password, user, privilege);
            if (ok) {
                Session s(user.user_id, user.privilege);
                sessions.push(s);
            } else {
                std::cout << "Invalid\n";
            }
        } else {
            std::cout << "Invalid\n";
        }
        break;
    }
        
    case CommandType::Logout:
        if (privilege < 1 || sessions.empty()) {
            std::cout << "Invalid\n";
        } else {
            sessions.pop();
        }
        break;

        
    case CommandType::Passwd:
        if (privilege < 1) {
            std::cout << "Invalid\n";
            break;
        }
        if (cmd.args.size() == 2 || cmd.args.size() == 3) {
            std::string user_id = cmd.args[0];
            std::string current_password, new_password;
            bool has_current_password = false;
            
            if (cmd.args.size() == 3) {
                current_password = cmd.args[1];
                new_password = cmd.args[2];
                has_current_password = true;
            } else {
                new_password = cmd.args[1];
            }
            
            bool ok = account_manager.passwd(user_id, current_password, new_password,
                                            has_current_password, privilege);
            if (!ok) std::cout << "Invalid\n";
        } else {
            std::cout << "Invalid\n";
        }
        break;
        
    case CommandType::UserAdd:
        if (cmd.args.size() == 4 && privilege >= 3) {
            std::string user_id = cmd.args[0];
            std::string password = cmd.args[1];
            int user_privilege = 0;
            if (!parse_int_strict(cmd.args[2], user_privilege)) {
                std::cout << "Invalid\n";
                break;
            }

            std::string username = cmd.args[3];
            
            if (user_privilege != 0 && user_privilege != 1 && user_privilege != 3 && user_privilege != 7) {
                std::cout << "Invalid\n";
                break;
            }
            
            if (privilege <= user_privilege) {
                std::cout << "Invalid\n";
                break;
            }
            
            bool ok = account_manager.useradd(user_id, password, user_privilege, username, privilege);
            if (!ok) std::cout << "Invalid\n";
        } else {
            std::cout << "Invalid\n";
        }
        break;
        
    case CommandType::DeleteUser:
        if (cmd.args.size() == 1 && privilege == 7) {
            std::string user_id = cmd.args[0];
            
            // 检查用户是否已登录
            if (sessions.is_user_logged_in(user_id)) {
                std::cout << "Invalid\n";
                break;
            }
            
            bool ok = account_manager.delete_user(user_id);
            if (!ok) std::cout << "Invalid\n";
        } else {
            std::cout << "Invalid\n";
        }
        break;
        
    case CommandType::Show:
        if (privilege < 1) {
            std::cout << "Invalid\n";
            break;
        }
        // show 指令只允许 0 或 1 个附加参数
        if (cmd.args.size() > 1) {
            std::cout << "Invalid\n";
            break;
        }
        if (cmd.args.empty()) {
            book_manager.show_all();
        } else {
            show_books_with_criteria(cmd.args[0]);
        }
        break;

        
    case CommandType::Buy:
        if (cmd.args.size() == 2 && privilege >= 1) {
            std::string isbn = cmd.args[0];
            int quantity = 0;
            if (!parse_int_strict(cmd.args[1], quantity)) {
                std::cout << "Invalid\n";
                break;
            }


            double total_cost = 0.0;
            
            if (quantity <= 0) {
                std::cout << "Invalid\n";
                break;
            }
            
            bool ok = book_manager.buy(isbn, quantity, total_cost);
            if (ok) {
                finance_manager.add_income(total_cost);
                std::cout << std::fixed << std::setprecision(2) << total_cost << '\n';
            } else {
                std::cout << "Invalid\n";
            }
        } else {
            std::cout << "Invalid\n";
        }
        break;
        
    case CommandType::Select:
        if (cmd.args.size() == 1 && privilege >= 3) {
            std::string isbn = cmd.args[0];
            bool ok = book_manager.select(isbn, sessions.top());
            if (!ok) std::cout << "Invalid\n";
        } else {
            std::cout << "Invalid\n";
        }
        break;
        
    case CommandType::Modify:
        if (!cmd.args.empty() && !sessions.empty() && privilege >= 3) {
            if (sessions.top().selected_pos == -1) {
                std::cout << "Invalid\n";
                break;
            }
            
            std::vector<std::pair<std::string, std::string>> modifications;
            bool bad = false;

            for (const auto& arg : cmd.args) {
                if (arg.empty() || arg[0] != '-') {
                    bad = true;
                    break;
                }
                size_t eq_pos = arg.find('=');
                if (eq_pos == std::string::npos || eq_pos <= 1) {
                    bad = true;
                    break;
                }

                std::string key = arg.substr(1, eq_pos - 1);
                std::string value = arg.substr(eq_pos + 1);

                // 附加参数内容为空则非法
                if (value.empty()) {
                    bad = true;
                    break;
                }

                // 允许的 key
                if (!(key == "ISBN" || key == "name" || key == "author" || key == "keyword" || key == "price")) {
                    bad = true;
                    break;
                }

                // name/author/keyword 必须带双引号；ISBN/price 禁止带双引号
                if (key == "name" || key == "author" || key == "keyword") {
                    if (value.size() < 2 || value.front() != '"' || value.back() != '"') {
                        bad = true;
                        break;
                    }
                    value = value.substr(1, value.size() - 2);
                    // 引号内不允许再出现双引号
                    if (value.find('"') != std::string::npos) {
                        bad = true;
                        break;
                    }
                } else {
                    if (value.find('"') != std::string::npos) {
                        bad = true;
                        break;
                    }
                }

                if (key == "price") {
                    double tmp = 0.0;
                    if (!parse_price_strict(value, tmp)) { // 这里会同时保证格式合法 + >0
                        bad = true;
                        break;
                    }
                }
                modifications.emplace_back(key, value);
            }


            if (bad) {
                std::cout << "Invalid\n";
                break;
            }
            
            bool ok = book_manager.modify(sessions.top().selected_pos, modifications);
            if (!ok) {
                std::cout << "Invalid\n";
            }
        } else {
            std::cout << "Invalid\n";
        }
        break;
        
    case CommandType::Import:
        if (cmd.args.size() == 2 && !sessions.empty() && privilege >= 3) {
            if (sessions.top().selected_pos == -1) {
                std::cout << "Invalid\n";
                break;
            }
            
            int quantity = 0;
            double total_cost = 0.0;

            if (!parse_int_strict(cmd.args[0], quantity)) {
                std::cout << "Invalid\n";
                break;
            }
            if (!parse_price_strict(cmd.args[1], total_cost)) {
                std::cout << "Invalid\n";
                break;
            }


            if (quantity <= 0 || total_cost <= 0) {
                std::cout << "Invalid\n";
                break;
            }

            
            bool ok = book_manager.import(sessions.top().selected_pos, quantity, total_cost);
            if (ok) {
                finance_manager.add_expense(total_cost);
            } else {
                std::cout << "Invalid\n";
            }
        } else {
            std::cout << "Invalid\n";
        }
        break;
        
    case CommandType::ShowFinance:
        handle_show_finance(cmd.args);
        break;
        
    case CommandType::ReportFinance:
        handle_report_finance();
        break;
        
    case CommandType::ReportEmployee:
        handle_report_employee();
        break;
        
    case CommandType::Log:
        handle_log();
        break;
        
    default:
        std::cout << "Invalid\n";
        break;
    }
}

void Application::handle_show_finance(const std::vector<std::string> &args) {
    if (sessions.current_privilege() < 7) {
        std::cout << "Invalid\n";
        return;
    }
    
    if (args.empty()) {
        finance_manager.show_all();
    } else if (args.size() == 1) {
        int count = 0;
        if (!parse_int_strict(args[0], count)) {
            std::cout << "Invalid\n";
            return;
        }
        if (count < 0) {
            std::cout << "Invalid\n";
            return;
        }
        finance_manager.show_last_n(count);

    } else {
        std::cout << "Invalid\n";
    }
}

void Application::handle_report_finance() {
    if (sessions.current_privilege() < 7) {
        std::cout << "Invalid\n";
        return;
    }
    
    finance_manager.generate_report();
}

void Application::handle_report_employee() {
    if (sessions.current_privilege() < 7) {
        std::cout << "Invalid\n";
        return;
    }
    
    log_manager.generate_employee_report();
}

void Application::handle_log() {
    if (sessions.current_privilege() < 7) {
        std::cout << "Invalid\n";
        return;
    }
    
    log_manager.show_log();
}

void Application::show_books_with_criteria(const std::string &criteria) {
    std::smatch match;
    std::regex pattern("-(ISBN|name|author|keyword)=(.+)");

    if (!std::regex_match(criteria, match, pattern)) {
        std::cout << "Invalid\n";
        return;
    }

    std::string type = match[1];
    std::string value = match[2];

    auto is_ascii_visible = [](const std::string &s) -> bool {
        for (unsigned char ch : s) {
            if (ch > 127) return false;
            if (!std::isprint(ch)) return false;
        }
        return true;
    };

    if (type == "name" || type == "author" || type == "keyword") {
        // 必须有外层引号
        if (value.size() < 2 || value.front() != '"' || value.back() != '"') {
            std::cout << "Invalid\n";
            return;
        }
        value = value.substr(1, value.size() - 2);

        // 引号内不能再出现双引号
        if (value.find('"') != std::string::npos) {
            std::cout << "Invalid\n";
            return;
        }

        // 最大长度 60（不含引号）
        if (value.size() > 60) {
            std::cout << "Invalid\n";
            return;
        }

        // 标准 ASCII 可见字符
        if (!is_ascii_visible(value)) {
            std::cout << "Invalid\n";
            return;
        }

        // show -keyword 只允许单个关键词（出现 '|' 表示多个关键词，操作失败）
        if (type == "keyword" && value.find('|') != std::string::npos) {
            std::cout << "Invalid\n";
            return;
        }
    }

    if (value.empty()) {
        std::cout << "Invalid\n";
        return;
    }

    if (type == "ISBN") {
        // 最大长度 20（允许可见 ASCII）
        if (value.size() > 20) {
            std::cout << "Invalid\n";
            return;
        }
        if (!is_ascii_visible(value)) {
            std::cout << "Invalid\n";
            return;
        }
        book_manager.show_by_isbn(value);
    } else if (type == "name") {
        book_manager.show_by_name(value);
    } else if (type == "author") {
        book_manager.show_by_author(value);
    } else if (type == "keyword") {
        book_manager.show_by_keyword(value);
    } else {
        std::cout << "Invalid\n";
    }
}
