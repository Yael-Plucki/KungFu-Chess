#include <iostream>
#include "texttests/ScriptRunner.hpp"

int main() {
    try {
        ScriptRunner runner;
        ScriptResult result = runner.run_interactive();
        if (!result.passed) {
            std::cout << result.error << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }

    return 0;
}
