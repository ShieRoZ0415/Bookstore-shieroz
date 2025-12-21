#pragma once
#include <string>
#include <vector>

struct Session {
    std::string user_id;
    int privilege;
    int selected_pos;

    Session(const std::string &u = "", int p = 0)
        : user_id(u), privilege(p), selected_pos(-1) {}
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