#include "include/application.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <regex>

Application::Application()
    : account_manager(), book_manager(), finance_manager(), log_manager() {
    account_manager.initialize();
}

void Application::run() {
    std::string line;
    while (std::getline(std::cin, line)) {
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
        if (!sessions.empty()) {
            sessions.pop();
        } else {
            std::cout << "Invalid\n";
        }
        break;
        
    case CommandType::Passwd:
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
            int user_privilege = std::stoi(cmd.args[2]);
            std::string username = cmd.args[3];
            
            if (user_privilege != 1 && user_privilege != 3 && user_privilege != 7) {
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
        if (cmd.args.empty() && privilege >= 1) {
            book_manager.show_all();
        } else if (cmd.args[0] == "finance") {
            handle_show_finance(cmd.args);
        } else if (privilege >= 1) {
            show_books_with_criteria(cmd.args[0]);
        } else {
            std::cout << "Invalid\n";
        }
        break;
        
    case CommandType::Buy:
        if (cmd.args.size() == 2 && privilege >= 1) {
            std::string isbn = cmd.args[0];
            int quantity = std::stoi(cmd.args[1]);
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
            if (sessions.top().selected_isbn.empty()) {
                std::cout << "Invalid\n";
                break;
            }
            
            std::vector<std::pair<std::string, std::string>> modifications;
            for (const auto& arg : cmd.args) {
                size_t eq_pos = arg.find('=');
                if (eq_pos == std::string::npos) continue;
                
                std::string key = arg.substr(1, eq_pos - 1);
                std::string value = arg.substr(eq_pos + 1);
                
                if (key == "name" || key == "author" || key == "keyword") {
                    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
                        value = value.substr(1, value.size() - 2);
                    }
                }
                
                modifications.emplace_back(key, value);
            }
            
            bool ok = book_manager.modify(sessions.top().selected_isbn, modifications);
            if (ok) {
                for (const auto& mod : modifications) {
                    if (mod.first == "ISBN") {
                        sessions.top().selected_isbn = mod.second;
                        break;
                    }
                }
            } else {
                std::cout << "Invalid\n";
            }
        } else {
            std::cout << "Invalid\n";
        }
        break;
        
    case CommandType::Import:
        if (cmd.args.size() == 2 && !sessions.empty() && privilege >= 3) {
            if (sessions.top().selected_isbn.empty()) {
                std::cout << "Invalid\n";
                break;
            }
            
            int quantity = std::stoi(cmd.args[0]);
            double total_cost = std::stod(cmd.args[1]);
            
            if (quantity <= 0 || total_cost <= 0) {
                std::cout << "Invalid\n";
                break;
            }
            
            bool ok = book_manager.import(sessions.top().selected_isbn, quantity, total_cost);
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
        int count = std::stoi(args[0]);
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
    std::regex pattern("-([^=]+)=(.+)");
    
    if (!std::regex_match(criteria, match, pattern)) {
        std::cout << "Invalid\n";
        return;
    }
    
    std::string type = match[1];
    std::string value = match[2];
    
    if (type == "name" || type == "author" || type == "keyword") {
        if (value.size() < 2 || value.front() != '"' || value.back() != '"') {
            std::cout << "Invalid\n";
            return;
        }
        value = value.substr(1, value.size() - 2);
    }
    
    if (value.empty()) {
        std::cout << "Invalid\n";
        return;
    }
    
    if (type == "ISBN") {
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