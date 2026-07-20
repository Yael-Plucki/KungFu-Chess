#include "input/BoardMapper.hpp"
#include "input/RemoteController.hpp"
#include "network/RemoteGameSession.hpp"
#include "network/WebSocketClient.hpp"
#include "shell/HomeScreen.hpp"
#include "view/Renderer.hpp"
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

constexpr const char* kDefaultServerUrl = "ws://127.0.0.1:9002";

void use_project_root_as_working_directory() {
    namespace fs = std::filesystem;
    fs::path dir = fs::current_path();
    for (int depth = 0; depth < 6; ++depth) {
        if (fs::exists(dir / "CMakeLists.txt") && fs::exists(dir / "main.cpp")) {
            fs::current_path(dir);
            return;
        }
        if (!dir.has_parent_path()) {
            break;
        }
        dir = dir.parent_path();
    }
}

std::string server_url(int argc, char** argv) {
    if (argc > 1) {
        return argv[1];
    }
    return kDefaultServerUrl;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        use_project_root_as_working_directory();

        const LoginResult login = HomeScreen::prompt_auth(std::cin, std::cout);
        if (!login.success) {
            std::cout << login.error << std::endl;
            return 1;
        }

        WebSocketClient client;
        client.connect(server_url(argc, argv));

        RemoteGameSession session(client);
        session.send_auth(
            login.username,
            login.password,
            login.mode == AuthMode::Register
        );

        if (!HomeScreen::wait_for_auth(session, std::cout, login)) {
            return 1;
        }

        if (!HomeScreen::wait_for_match(session, std::cout, login.username)) {
            std::cerr << "Failed to start game." << std::endl;
            return 1;
        }

        const GameSnapshot initial_snapshot = session.snapshot_with_selection(std::nullopt);
        BoardMapper mapper(initial_snapshot.board_height, initial_snapshot.board_width);
        RemoteController controller(session, mapper);
        Renderer renderer(session, controller);
        renderer.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
