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
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {

constexpr const char* kDefaultServerUrl = "ws://127.0.0.1:9002";
constexpr const char* kDefaultAssetsRoot = "lib/CTD26/assets (2)/assets/images/pieces";

bool is_project_root(const std::filesystem::path& dir) {
    return std::filesystem::exists(dir / "CMakeLists.txt") &&
           std::filesystem::exists(dir / "src");
}

#ifdef _WIN32
std::string get_executable_path() {
    char buffer[MAX_PATH];
    const DWORD length = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) {
        throw std::runtime_error("Failed to resolve executable path.");
    }
    return std::string(buffer, length);
}
#endif

void use_project_root_as_working_directory() {
    namespace fs = std::filesystem;
    std::vector<fs::path> starts;
    starts.push_back(fs::current_path());
#ifdef _WIN32
    starts.push_back(fs::path(get_executable_path()).parent_path());
#endif

    for (const fs::path& start : starts) {
        fs::path dir = start;
        for (int depth = 0; depth < 8; ++depth) {
            if (is_project_root(dir)) {
                fs::current_path(dir);
                return;
            }
            if (!dir.has_parent_path()) {
                break;
            }
            dir = dir.parent_path();
        }
    }
}

std::string resolve_assets_root() {
    namespace fs = std::filesystem;
    const fs::path relative = kDefaultAssetsRoot;

    if (fs::exists(relative) && fs::is_directory(relative)) {
        return fs::absolute(relative).string();
    }

    std::vector<fs::path> starts;
    starts.push_back(fs::current_path());
#ifdef _WIN32
    starts.push_back(fs::path(get_executable_path()).parent_path());
#endif

    for (const fs::path& start : starts) {
        fs::path dir = start;
        for (int depth = 0; depth < 8; ++depth) {
            const fs::path candidate = dir / relative;
            if (fs::exists(candidate) && fs::is_directory(candidate)) {
                if (is_project_root(dir)) {
                    fs::current_path(dir);
                }
                return candidate.string();
            }
            if (!dir.has_parent_path()) {
                break;
            }
            dir = dir.parent_path();
        }
    }

    std::cerr << "Warning: could not find piece assets at " << relative.string()
              << ". Board and pieces may not render correctly.\n";
    return relative.string();
}

bool is_client_mode(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--client") {
            return true;
        }
    }
    return false;
}

std::string server_url(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--client") {
            continue;
        }
        return argv[i];
    }
    return kDefaultServerUrl;
}

#ifdef _WIN32
std::string quote_for_powershell(const std::string& value) {
    std::string quoted = "'";
    for (const char character : value) {
        if (character == '\'') {
            quoted += "''";
        } else {
            quoted += character;
        }
    }
    quoted += "'";
    return quoted;
}

void launch_powershell_client(const std::string& exe_path, const std::vector<std::string>& extra_args) {
    std::string parameters = "-NoExit -Command \"& " + quote_for_powershell(exe_path) + " --client";
    for (const std::string& arg : extra_args) {
        parameters += " " + quote_for_powershell(arg);
    }
    parameters += "\"";

    namespace fs = std::filesystem;
    std::string working_dir = fs::path(exe_path).parent_path().string();
    fs::path dir = fs::path(exe_path).parent_path();
    for (int depth = 0; depth < 8; ++depth) {
        if (is_project_root(dir)) {
            working_dir = dir.string();
            break;
        }
        if (!dir.has_parent_path()) {
            break;
        }
        dir = dir.parent_path();
    }

    const HINSTANCE result = ShellExecuteA(
        nullptr,
        "open",
        "powershell.exe",
        parameters.c_str(),
        working_dir.c_str(),
        SW_SHOW
    );
    if (reinterpret_cast<intptr_t>(result) <= 32) {
        throw std::runtime_error("Failed to launch PowerShell client window.");
    }
}

bool launch_client_windows(int argc, char** argv) {
    if (is_client_mode(argc, argv)) {
        return false;
    }

    const std::string exe_path = get_executable_path();
    std::vector<std::string> extra_args;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) != "--client") {
            extra_args.emplace_back(argv[i]);
        }
    }

    launch_powershell_client(exe_path, extra_args);
    launch_powershell_client(exe_path, extra_args);
    return true;
}
#endif

}  // namespace

int main(int argc, char** argv) {
    try {
#ifdef _WIN32
        if (launch_client_windows(argc, argv)) {
            return 0;
        }
#endif

        use_project_root_as_working_directory();
        const std::string assets_root = resolve_assets_root();

        HomeScreen::render(std::cout);

        WebSocketClient client;
        client.connect(server_url(argc, argv));

        RemoteGameSession session(client);
        HomeScreen::wait_for_lobby_state(session, std::cout);

        const LoginResult login = HomeScreen::prompt_auth(std::cin, std::cout, &session);
        if (!login.success) {
            std::cout << login.error << std::endl;
            return 1;
        }

        session.send_auth(
            login.username,
            login.password,
            login.mode == AuthMode::Register
        );

        if (!HomeScreen::wait_for_auth(session, std::cout, login)) {
            return 1;
        }

        const HomeMenuChoice menu_choice = HomeScreen::show_gui_menu(
            std::cin,
            std::cout,
            login.username,
            session.player_rating()
        );
        if (menu_choice != HomeMenuChoice::Play) {
            return 0;
        }

        const SeekResult seek_result = HomeScreen::seek_and_wait_for_match(session, std::cout, login.username);
        if (seek_result != SeekResult::Matched) {
            if (seek_result == SeekResult::Timeout) {
#ifdef _WIN32
                MessageBoxA(
                    nullptr,
                    "Could not find an opponent within 1 minute.",
                    "Kung Fu Chess",
                    MB_OK | MB_ICONINFORMATION
                );
#endif
                return 0;
            }
            return 1;
        }

        const GameSnapshot initial_snapshot = session.snapshot_with_selection(std::nullopt);
        BoardMapper mapper(initial_snapshot.board_height, initial_snapshot.board_width);
        RemoteController controller(session, mapper);
        Renderer renderer(session, controller, assets_root);
        renderer.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
