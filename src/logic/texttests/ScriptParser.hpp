#pragma once
#include <string>
#include <vector>

struct Command {
    std::string type; // e.g., "move", "wait", "board"
    std::vector<std::string> args;
};

class ScriptParser {
public:
    Command parse_command(const std::string& line);
};