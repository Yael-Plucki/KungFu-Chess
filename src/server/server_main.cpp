#include "engine/GameEngine.hpp"
#include "io/BoardParser.hpp"
#include "network/Lobby.hpp"
#include "network/NetworkBridge.hpp"
#include "network/WebSocketServer.hpp"
#include "storage/UserDatabase.hpp"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {

constexpr int kFrameMs = 16;
constexpr int kDefaultPort = 9002;
constexpr int kLobbyPollMs = 100;
constexpr int kSnapshotSyncMs = 500;

void parse_default_board() {
    BoardParser parser;
    parser.parseRows({
        "bR bN bB bQ bK bB bN bR",
        "bP bP bP bP bP bP bP bP",
        ". . . . . . . .",
        ". . . . . . . .",
        ". . . . . . . .",
        ". . . . . . . .",
        "wP wP wP wP wP wP wP wP",
        "wR wN wB wQ wK wB wN wR"
    });
}

int parse_port(int argc, char** argv) {
    if (argc < 2) {
        return kDefaultPort;
    }

    return std::stoi(argv[1]);
}

bool is_project_root(const std::filesystem::path& dir) {
    return std::filesystem::exists(dir / "CMakeLists.txt") &&
           std::filesystem::exists(dir / "src");
}

std::string resolve_database_path() {
    namespace fs = std::filesystem;
    std::vector<fs::path> starts;
    starts.push_back(fs::current_path());

#ifdef _WIN32
    char buffer[MAX_PATH];
    const DWORD length = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    if (length > 0 && length < MAX_PATH) {
        starts.push_back(fs::path(std::string(buffer, length)).parent_path());
    }
#endif

    for (const fs::path& start : starts) {
        fs::path dir = start;
        for (int depth = 0; depth < 8; ++depth) {
            if (is_project_root(dir)) {
                return (dir / "kungfu.db").string();
            }
            if (!dir.has_parent_path()) {
                break;
            }
            dir = dir.parent_path();
        }
    }

    return "kungfu.db";
}

}  // namespace

int main(int argc, char** argv) {
    try {
        const int port = parse_port(argc, argv);
        parse_default_board();

        const std::string database_path = resolve_database_path();
        UserDatabase user_database(database_path);
        GameEngine engine;
        WebSocketServer server(port);
        Lobby lobby;
        NetworkBridge bridge(engine, server, lobby, user_database);

        server.start();
        std::cout << "KungFu Chess WebSocket server listening on ws://0.0.0.0:" << port << std::endl;
        std::cout << "User database: " << database_path << std::endl;
        std::cout << "Waiting for players to seek a match..." << std::endl;
        bridge.broadcast_lobby_state();

        while (server.is_running() && !lobby.is_started()) {
            bridge.process_inbound();
            bridge.process_disconnects();
            std::this_thread::sleep_for(std::chrono::milliseconds(kLobbyPollMs));
        }

        if (!lobby.is_started()) {
            return 0;
        }

        std::cout << "Both players joined. Starting game." << std::endl;

        long long last_snapshot_sync_ms = 0;
        while (server.is_running()) {
            bridge.process_inbound();
            bridge.process_disconnects();
            bridge.process_disconnect_timers();
            engine.wait(kFrameMs);
            const long long current_time_ms = engine.current_time_ms();
            if (current_time_ms - last_snapshot_sync_ms >= kSnapshotSyncMs) {
                bridge.broadcast_snapshot();
                last_snapshot_sync_ms = current_time_ms;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
