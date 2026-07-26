#include "input/BoardMapper.hpp"
#include "input/RemoteController.hpp"
#include "network/ClientBoardSync.hpp"
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

void disable_console_quick_edit() {
    const HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    if (input == INVALID_HANDLE_VALUE) {
        return;
    }

    DWORD mode = 0;
    if (!GetConsoleMode(input, &mode)) {
        return;
    }

    mode &= ~ENABLE_QUICK_EDIT_MODE;
    mode &= ~ENABLE_INSERT_MODE;
    SetConsoleMode(input, mode);
}

void pause_console_before_exit() {
    const HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    if (input == INVALID_HANDLE_VALUE) {
        return;
    }

    std::cout << "\nPress Enter to close this window..." << std::endl;
    std::cout.flush();

    std::string line;
    std::getline(std::cin, line);
}

class ClientExitPause {
public:
    explicit ClientExitPause(bool enabled) : enabled_(enabled) {}

    ~ClientExitPause() {
        if (enabled_ && pause_) {
            pause_console_before_exit();
        }
    }

    void dismiss() {
        pause_ = false;
    }

private:
    bool enabled_;
    bool pause_ = true;
};
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
std::string quote_for_cmd(const std::string& value) {
    if (value.find_first_of(" \t\"") == std::string::npos) {
        return value;
    }

    std::string quoted = "\"";
    for (const char character : value) {
        if (character == '"') {
            quoted += "\\\"";
        } else {
            quoted += character;
        }
    }
    quoted += "\"";
    return quoted;
}

std::string resolve_client_working_directory(const std::string& exe_path) {
    namespace fs = std::filesystem;
    std::string working_dir = fs::path(exe_path).parent_path().string();
    fs::path dir = fs::path(exe_path).parent_path();
    for (int depth = 0; depth < 8; ++depth) {
        if (is_project_root(dir)) {
            return dir.string();
        }
        if (!dir.has_parent_path()) {
            break;
        }
        dir = dir.parent_path();
    }
    return working_dir;
}

void launch_client_process(const std::string& exe_path, const std::vector<std::string>& extra_args) {
    std::string command_line = quote_for_cmd(exe_path) + " --client";
    for (const std::string& arg : extra_args) {
        command_line += " " + quote_for_cmd(arg);
    }

    std::vector<char> command_buffer(command_line.begin(), command_line.end());
    command_buffer.push_back('\0');

    STARTUPINFOA startup_info{};
    startup_info.cb = sizeof(startup_info);
    PROCESS_INFORMATION process_info{};

    const BOOL created = CreateProcessA(
        exe_path.c_str(),
        command_buffer.data(),
        nullptr,
        nullptr,
        FALSE,
        CREATE_NEW_CONSOLE,
        nullptr,
        resolve_client_working_directory(exe_path).c_str(),
        &startup_info,
        &process_info
    );
    if (!created) {
        throw std::runtime_error("Failed to launch client window.");
    }

    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);
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

    launch_client_process(exe_path, extra_args);
    launch_client_process(exe_path, extra_args);
    return true;
}
#endif

}  // namespace

int main(int argc, char** argv) {
#ifdef _WIN32
    ClientExitPause exit_pause(is_client_mode(argc, argv));
#endif

    try {
#ifdef _WIN32
        if (launch_client_windows(argc, argv)) {
            return 0;
        }
        disable_console_quick_edit();
#endif

        use_project_root_as_working_directory();
        const std::string assets_root = resolve_assets_root();

        HomeScreen::render(std::cout);

        WebSocketClient client;
        client.connect(server_url(argc, argv));

        RemoteGameSession session(client);
        ClientBoardSync board_sync(session);
        HomeScreen::wait_for_lobby_state(session, std::cout);

        const LoginResult login = HomeScreen::prompt_auth(std::cin, std::cout, &session);
        if (!login.success) {
            std::cout << login.error << std::endl;
            return 1;
        }

        if (login.mode == AuthMode::Register) {
            std::cout << "Creating account for " << login.username << "...\n";
        } else {
            std::cout << "Logging in as " << login.username << "...\n";
        }

        session.send_auth(
            login.username,
            login.password,
            login.mode == AuthMode::Register
        );

        if (!HomeScreen::wait_for_auth(session, std::cout, login)) {
#ifdef _WIN32
            MessageBoxA(
                nullptr,
                session.auth_error().c_str(),
                "Kung Fu Chess - Authentication Failed",
                MB_OK | MB_ICONERROR
            );
#endif
            return 1;
        }

        std::cout << "\nSigned in as " << login.username
                  << " (Rating: " << session.player_rating() << ")\n";
        std::cout << "Finding an online match...\n";

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

        const GameSnapshot initial_snapshot = board_sync.snapshot_with_selection(std::nullopt);
        BoardMapper mapper(
            initial_snapshot.board_height,
            initial_snapshot.board_width,
            initial_snapshot.cell_size,
            initial_snapshot.side_panel_width
        );
        RemoteController controller(session, board_sync, mapper);
        const std::string window_title = "KungFu Chess - " + login.username;
        std::cout << "Click the board window titled \"" << window_title
                  << "\" to play. Avoid clicking this console while playing.\n";
        std::cout.flush();
#ifdef _WIN32
        exit_pause.dismiss();
        FreeConsole();
#endif
        Renderer renderer(session, board_sync, controller, assets_root, window_title);
        renderer.run();
    } catch (const std::exception& e) {
#ifdef _WIN32
        MessageBoxA(nullptr, e.what(), "KungFu Chess", MB_OK | MB_ICONERROR);
#else
        std::cerr << e.what() << std::endl;
#endif
        return 1;
    }

    return 0;
}
