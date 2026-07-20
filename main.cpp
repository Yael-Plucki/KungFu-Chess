#include <iostream>
#include "shell/HomeScreen.hpp"
#include "texttests/ScriptRunner.hpp"

int main() {
    try {
        const LoginResult login = HomeScreen::prompt_auth(std::cin, std::cout);
        if (!login.success) {
            std::cout << login.error << std::endl;
            return 1;
        }

        const HomeMenuChoice choice = HomeScreen::show_menu(std::cin, std::cout, login.username);
        if (choice != HomeMenuChoice::Play) {
            return 0;
        }

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
