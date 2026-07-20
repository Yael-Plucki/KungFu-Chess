#pragma once

#include "../model/Position.hpp"
#include <optional>
#include <string>

namespace Protocol {

constexpr const char* kAuth = "auth";
constexpr const char* kAuthResult = "auth_result";
constexpr const char* kMove = "move";
constexpr const char* kJump = "jump";
constexpr const char* kPing = "ping";
constexpr const char* kPong = "pong";

constexpr const char* kSnapshot = "snapshot";
constexpr const char* kLobbyState = "lobby_state";
constexpr const char* kGameStarted = "game_started";
constexpr const char* kMoveAccepted = "move_accepted";
constexpr const char* kMoveRejected = "move_rejected";
constexpr const char* kJumpStarted = "jump_started";
constexpr const char* kMoveResolved = "move_resolved";
constexpr const char* kGameOver = "game_over";

struct InboundMessage {
    enum class Kind { Auth, Move, Jump, Ping, Unknown };

    Kind kind = Kind::Unknown;
    std::string username;
    std::string password;
    bool register_account = false;
    Position src;
    Position dest;
    Position cell;
};

struct InboundFrame {
    std::string connection_id;
    std::string payload;
};

}  // namespace Protocol
