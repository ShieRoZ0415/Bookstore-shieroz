#include "include/log.h"
#include <fstream>

LogManager::LogManager() : file("log.dat") {
    std::ifstream fin("log.dat", std::ios::binary);
    if (!fin.good()) {
        file.initialise();
        file.write_info(0, 1);
    }
    fin.close();
}

void LogManager::record_sys(const std::string &user, const std::string &action) {
    LogEntry e;
    std::strncpy(e.user, user.c_str(), 30);
    std::strncpy(e.type, "SYS", 7);
    std::strncpy(e.action, action.c_str(), 127);

    file.write(e);

    int cnt = 0;
    file.get_info(cnt, 1);
    file.write_info(cnt + 1, 1);
}

void LogManager::record_fin(const std::string &user, const std::string &action) {
    LogEntry e;
    std::strncpy(e.user, user.c_str(), 30);
    std::strncpy(e.type, "FIN", 7);
    std::strncpy(e.action, action.c_str(), 127);

    file.write(e);

    int cnt = 0;
    file.get_info(cnt, 1);
    file.write_info(cnt + 1, 1);
}

void LogManager::show_log() {
    int cnt = 0;
    file.get_info(cnt, 1);

    std::cout << "LOG\n";
    for (int i = 1; i <= cnt; ++i) {
        int pos = 2 * sizeof(int) + (i - 1) * sizeof(LogEntry);
        LogEntry e;
        file.read(e, pos);
        std::cout << e.user << " " << e.type << " " << e.action << "\n";
    }
    std::cout << "END\n";
}

void LogManager::generate_employee_report() {
    int cnt = 0;
    file.get_info(cnt, 1);

    std::map<std::string, int> sys_cnt;
    std::map<std::string, int> fin_cnt;

    for (int i = 1; i <= cnt; ++i) {
        int pos = 2 * sizeof(int) + (i - 1) * sizeof(LogEntry);
        LogEntry e;
        file.read(e, pos);
        if (std::string(e.type) == "FIN") fin_cnt[e.user]++;
        else sys_cnt[e.user]++;
    }

    std::vector<std::string> users;
    for (auto &p : sys_cnt) users.push_back(p.first);
    for (auto &p : fin_cnt) users.push_back(p.first);
    std::sort(users.begin(), users.end());
    users.erase(std::unique(users.begin(), users.end()), users.end());

    std::cout << "EMPLOYEE REPORT\n";
    for (auto &u : users) {
        int s = sys_cnt[u];
        int f = fin_cnt[u];
        std::cout << u << " " << s << " " << f << " " << (s + f) << "\n";
    }
    std::cout << "END\n";
}
