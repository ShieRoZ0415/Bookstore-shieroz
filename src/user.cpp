#include "include/user.h"

#include <cstring>
#include <cctype>

User::User() {
    std::memset(user_id, 0, sizeof(user_id));
    std::memset(password, 0, sizeof(password));
    std::memset(username, 0, sizeof(username));
    privilege = 0;
}

User::User(const std::string &uid, const std::string &pwd,
           const std::string &uname, int priv) {
    std::strncpy(user_id, uid.c_str(), 30);
    user_id[30] = '\0';
    std::strncpy(password, pwd.c_str(), 30);
    password[30] = '\0';
    std::strncpy(username, uname.c_str(), 30);
    username[30] = '\0';
    privilege = priv;
}

AccountManager::AccountManager()
    : user_file("users.dat") {
}

void AccountManager::rebuild_users_file() {
    const char* FN = "users.dat";
    user_file.initialise(FN);
    user_file.write_info(0, 1);

    User root("root", "sjtu", "Super Admin", 7);
    user_file.write(root);
    user_file.write_info(1, 1);
}


void AccountManager::initialize() {
    const char *FN = "users.dat";
    const int HEADER = sizeof(int);

    bool need_init = false;

    // 文件不存在需要初始化
    std::ifstream fin;
    fin.open(FN, std::ios::binary);

    if (!fin.good()) {
        rebuild_users_file();
        return;
    }
    // 文件长度检查
    fin.seekg(0, std::ios::end);
    std::streamoff sz = fin.tellg();
    fin.close();

    if (sz < static_cast<std::streamoff>(HEADER + (int)sizeof(User))) {
        rebuild_users_file();
        return;
    }

    int n = 0;
    user_file.get_info(n, 1);
    if (n == 0) {
        rebuild_users_file();
        return;
    }

    // 验证第一条是否是 root
    User first;
    user_file.read(first, HEADER);

    if (std::strcmp(first.user_id, "root") != 0 ||
        std::strcmp(first.password, "sjtu") != 0 ||
        first.privilege != 7) {
        rebuild_users_file();
        return;
        }
}

bool AccountManager::validate_string(const std::string &str, bool allow_quotes) {
    if (str.empty() || str.length() > 30) return false;

    for (unsigned char uc : str) {
        char c = static_cast<char>(uc);

        if (c == ' ') return false;

        if (uc < 33 || uc > 126) return false;

        if (!allow_quotes && c == '"') return false;

    }
    return true;
}

bool AccountManager::find_user(const std::string &id, User &user, int &index) {
    int n = 0;
    user_file.get_info(n, 1);

    for (int i = 1; i <= n; ++i) {
        const int HEADER = sizeof(int);
        int pos = HEADER + (i - 1) * sizeof(User);

        User tmp;
        user_file.read(tmp, pos);

        if (tmp.user_id[0] == '\0') continue; // 跳过已删除用户

        if (std::strcmp(id.c_str(), tmp.user_id) == 0) {
            user = tmp;
            index = pos;
            return true;
        }
    }
    return false;
}

bool AccountManager::register_user(const std::string &user_id,
                                   const std::string &password,
                                   const std::string &user_name) {
    if (!validate_string(user_id, false) || !validate_string(password, false) ||
        !validate_string(user_name, true)) {
        return false;
    }

    User tmp;
    int idx;
    if (find_user(user_id, tmp, idx)) return false;

    User new_user(user_id, password, user_name, 1);

    int n = 0;
    user_file.get_info(n, 1);
    user_file.write(new_user);
    user_file.write_info(n + 1, 1);

    return true;
}

bool AccountManager::login(const std::string &user_id,
                           const std::string &password,
                           bool has_password,
                           User &out_user,
                           int caller_privilege) {
    int idx = 0;
    if (!find_user(user_id, out_user, idx)) return false;

    if (has_password) {
        if (password != out_user.password) return false;
    } else {
        if (caller_privilege <= out_user.privilege) return false;
    }

    return true;
}

bool AccountManager::passwd(const std::string &user_id,
                            const std::string &old_password,
                            const std::string &new_password,
                            bool has_old_password,
                            int caller_privilege) {
    if (!validate_string(new_password, false)) return false;

    User user;
    int idx = 0;
    if (!find_user(user_id, user, idx)) return false;

    if (has_old_password) {
        if (old_password != user.password) return false;
    } else {
        if (caller_privilege != 7) return false;
    }

    std::strncpy(user.password, new_password.c_str(), 30);
    user.password[30] = '\0';
    user_file.update(user, idx);

    return true;
}

bool AccountManager::useradd(const std::string &user_id,
                             const std::string &password,
                             int privilege,
                             const std::string &user_name,
                             int caller_privilege) {
    if (!validate_string(user_id, false) || !validate_string(password, false) ||
        !validate_string(user_name, true)) {
        return false;
    }

    if (privilege != 1 && privilege != 3 && privilege != 7) return false;
    if (caller_privilege <= privilege) return false;

    User tmp;
    int idx;
    if (find_user(user_id, tmp, idx)) return false;

    User new_user(user_id, password, user_name, privilege);

    int n = 0;
    user_file.get_info(n, 1);
    user_file.write(new_user);
    user_file.write_info(n + 1, 1);

    return true;
}

bool AccountManager::delete_user(const std::string &user_id) {
    User user;
    int idx = 0;
    if (!find_user(user_id, user, idx)) return false;

    // 标记删除
    user.user_id[0] = '\0';
    user_file.update(user, idx);
    
    return true;
}