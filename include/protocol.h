#pragma once
#include <string>
#include <vector>

struct Command {
    std::string name;
    std::vector<std::string> args;
};

class Protocol {
public:
    static bool parse(const std::string& input, Command& cmd);
};
