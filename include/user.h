#pragma once
#include <string>

#include "MemoryRiver.h"
#include "MemoryRiver.hpp"

struct User {
    char user_id[31];
    char password[31];
    char username[31];
    int privilege;

    User();
    User(const std::string &uid, const std::string &pwd,
         const std::string &uname, int priv);
};

class AccountManager {
public:
    AccountManager();
    void initialize();

    bool register_user(const std::string &user_id,
                       const std::string &password,
                       const std::string &user_name);

    bool login(const std::string &user_id,
               const std::string &password,
               bool has_password,
               User &out_user,
               int caller_privilege);

    bool passwd(const std::string &user_id,
                const std::string &old_password,
                const std::string &new_password,
                bool has_old_password,
                int caller_privilege);

    bool useradd(const std::string &user_id,
                 const std::string &password,
                 int privilege,
                 const std::string &user_name,
                 int caller_privilege);

    bool delete_user(const std::string &user_id);

private:
    MemoryRiver<User, 1> user_file;

    bool find_user(const std::string &user_id, User &user, int &index);
    bool validate_string(const std::string &str, bool allow_quotes);
};