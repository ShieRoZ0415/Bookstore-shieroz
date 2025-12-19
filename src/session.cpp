#include "include/session.h"

bool SessionStack::empty() const {
    return stack.empty();
}

Session &SessionStack::top() {
    return stack.back();
}

const Session &SessionStack::top() const {
    return stack.back();
}

void SessionStack::push(const Session &s) {
    stack.push_back(s);
}

void SessionStack::pop() {
    if (stack.empty()) return;
    stack.pop_back();
}

int SessionStack::current_privilege() const {
    if (stack.empty()) return 0;
    return stack.back().privilege;
}

bool SessionStack::is_user_logged_in(const std::string &user_id) const {
    for (const auto &session : stack) {
        if (session.user_id == user_id) {
            return true;
        }
    }
    return false;
}