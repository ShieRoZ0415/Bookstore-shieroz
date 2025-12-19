#pragma once
#include <string>
#include <vector>

struct Session {
    std::string user_id;
    int privilege;
    std::string selected_isbn;

    Session(const std::string &u = "", int p = 0)
        : user_id(u), privilege(p), selected_isbn("") {}
};

class SessionStack {
public:
    bool empty() const;
    Session &top();
    const Session &top() const;
    void push(const Session &s);
    void pop();
    int current_privilege() const;
    bool is_user_logged_in(const std::string &user_id) const;

private:
    std::vector<Session> stack;
};