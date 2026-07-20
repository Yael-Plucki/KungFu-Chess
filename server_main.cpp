#include "engine/GameEngine.hpp"
#include "io/BoardParser.hpp"
#include "network/Lobby.hpp"
#include "network/NetworkBridge.hpp"
#include "network/WebSocketServer.hpp"
#include "storage/UserDatabase.hpp"
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

namespace {

constexpr int kFrameMs = 16;
constexpr int kDefaultPort = 9002;
constexpr int kLobbyPollMs = 100;

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

}  // namespace

int main(int argc, char** argv) {
    try {
        const int port = parse_port(argc, argv);
        parse_default_board();

        UserDatabase user_database("kungfu.db");
        GameEngine engine;
        WebSocketServer server(port);
        Lobby lobby;
        NetworkBridge bridge(engine, server, lobby, user_database);

        server.start();
        std::cout << "KungFu Chess WebSocket server listening on ws://0.0.0.0:" << port << std::endl;
        std::cout << "User database: kungfu.db" << std::endl;
        std::cout << "Waiting for 2 players to authenticate from the shell..." << std::endl;
        bridge.broadcast_lobby_state();

        while (server.is_running() && !lobby.is_started()) {
            bridge.process_inbound();
            bridge.try_start_game();
            std::this_thread::sleep_for(std::chrono::milliseconds(kLobbyPollMs));
        }

        if (!lobby.is_started()) {
            return 0;
        }

        std::cout << "Both players joined. Starting game." << std::endl;

        while (server.is_running()) {
            bridge.process_inbound();
            engine.wait(kFrameMs);
            bridge.broadcast_snapshot();
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
