#pragma once

#include "model/Position.hpp"
#include "network/JsonCodec.hpp"
#include <chrono>
#include <mutex>
#include <optional>
#include <string>

class WebSocketClient;

class RemoteGameSession {
public:
    static constexpr int kSeekTimeoutSeconds = 60;
    static constexpr int kDisconnectForfeitSeconds = 20;

    explicit RemoteGameSession(WebSocketClient& client);

    void send_auth(const std::string& username, const std::string& password, bool register_account);
    void send_seek();
    void send_cancel_seek();
    void send_resign();
    void send_move(const Position& src, const Position& dest);
    void send_jump(const Position& cell);

    void process_messages();
    bool has_lobby_state() const;
    bool can_join_as_black() const;
    bool auth_completed() const;
    bool auth_succeeded() const;
    std::string auth_error() const;
    int player_rating() const;
    bool game_started() const;
    bool has_snapshot() const;
    bool opponent_disconnected() const;
    int disconnect_countdown_seconds() const;
    GameSnapshot snapshot_with_selection(std::optional<Position> selected_cell) const;
    const LobbyStateMessage& lobby_state() const;

private:
    WebSocketClient& client_;
    bool auth_completed_ = false;
    bool auth_succeeded_ = false;
    std::string auth_error_;
    int player_rating_ = 1200;
    std::string username_;
    bool game_started_ = false;
    bool has_snapshot_ = false;
    bool has_lobby_state_ = false;
    bool opponent_disconnected_ = false;
    std::optional<std::chrono::steady_clock::time_point> disconnect_deadline_;
    GameSnapshot latest_snapshot_;
    LobbyStateMessage lobby_state_;
    mutable std::mutex state_mutex_;

    void handle_message(const std::string& message);
};
