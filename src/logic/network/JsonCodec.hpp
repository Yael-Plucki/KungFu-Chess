#pragma once

#include "../core/GameEvents.hpp"
#include "../model/GameSnapshot.hpp"
#include "Protocol.hpp"
#include <optional>
#include <string>

struct LobbyStateMessage {
    std::optional<std::string> white_username;
    std::optional<std::string> black_username;
    std::optional<int> white_rating;
    std::optional<int> black_rating;
    int players_joined = 0;
    bool game_started = false;
};

struct GameStartedMessage {
    std::string white_username;
    std::string black_username;
    int white_rating = 1200;
    int black_rating = 1200;
};

struct AuthResultMessage {
    bool success = false;
    std::string username;
    int rating = 1200;
    std::string reason;
};

struct GameOverMessage {
    std::string winner_color;
    std::string white_username;
    std::string black_username;
    int white_rating = 1200;
    int black_rating = 1200;
    int white_rating_delta = 0;
    int black_rating_delta = 0;
};

struct PlayerDisconnectedMessage {
    int seconds_remaining = 20;
};

class JsonCodec {
public:
    static std::optional<Protocol::InboundMessage> parse_inbound(const std::string& json_text);

    static std::string encode_pong();
    static std::string encode_auth(const std::string& username, const std::string& password, bool register_account);
    static std::string encode_seek();
    static std::string encode_cancel_seek();
    static std::string encode_resign();
    static std::string encode_auth_result(const AuthResultMessage& result);
    static std::string encode_snapshot(const GameSnapshot& snapshot);
    static std::string encode_lobby_state(const LobbyStateMessage& lobby);
    static std::string encode_game_started(const GameStartedMessage& message);
    static std::string encode_move_accepted(const MoveAcceptedEvent& event);
    static std::string encode_move_rejected(const MoveRejectedEvent& event);
    static std::string encode_jump_started(const JumpStartedEvent& event);
    static std::string encode_move_resolved(const MoveResolvedEvent& event, int board_rows, int board_cols);
    static std::string encode_game_over(const GameOverMessage& message);
    static std::string encode_player_disconnected(const PlayerDisconnectedMessage& message);

    static std::optional<GameSnapshot> decode_snapshot(const std::string& json_text);
    static std::optional<LobbyStateMessage> decode_lobby_state(const std::string& json_text);
    static std::optional<GameStartedMessage> decode_game_started(const std::string& json_text);
    static std::optional<AuthResultMessage> decode_auth_result(const std::string& json_text);
    static std::optional<GameOverMessage> decode_game_over(const std::string& json_text);
    static std::optional<PlayerDisconnectedMessage> decode_player_disconnected(const std::string& json_text);
};
