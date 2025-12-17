#include "protocol.h"
#include <sstream>

bool Protocol::parse(const std::string& input, Command& cmd) {
    std::istringstream iss(input);
    iss >> cmd.name;
    std::string arg;
    while (iss >> arg) cmd.args.push_back(arg);
    return !cmd.name.empty();
}
