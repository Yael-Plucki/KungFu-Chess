#pragma once

#include <string>
#include <vector>

struct ScriptResult {
    bool passed;
    std::string error;
};

enum class ScriptMode {
    Test,
    Interactive
};

class ScriptRunner {
public:
    ScriptResult run(const std::vector<std::string>& lines, ScriptMode mode = ScriptMode::Test);
    ScriptResult run_script(const std::vector<std::string>& lines);
    ScriptResult run_script_file(const std::string& filename);
    ScriptResult run_interactive();

private:
    static bool is_command_line(const std::string& line);
};
