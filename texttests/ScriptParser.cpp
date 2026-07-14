#include "ScriptParser.hpp"
#include <sstream>

Command ScriptParser::parse_command(const std::string& line) {
    std::stringstream ss(line);
    std::string type;
    ss >> type;
    
    std::vector<std::string> args;
    std::string arg;
    while (ss >> arg) {
        args.push_back(arg);
    }
    
    return {type, args};
}