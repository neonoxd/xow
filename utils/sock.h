#pragma once
#include <sstream>
#include <string>

namespace Socks
{
    extern int act_sock;
    int createConnection();
    int sendMessage(std::string message);

    template <typename... T>
    std::string concat_string(T&&... ts) {
        std::stringstream s;
        int dummy[] = { 0, ((s << std::forward<T>(ts)), 0)... };
        static_cast<void>(dummy);
        return s.str();
    }
}