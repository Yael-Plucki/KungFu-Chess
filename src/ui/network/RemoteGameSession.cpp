#include "RemoteGameSession.hpp"

#include "network/Protocol.hpp"
#include "network/WebSocketClient.hpp"

RemoteGameSession::RemoteGameSession(WebSocketClient& client) : client_(client) {}

void RemoteGameSession::send_auth(const std::string& username, const std::string& password, bool register_account) {
    client_.send(JsonCodec::encode_auth(username, password, register_account));
}

void RemoteGameSession::send_seek() {
    client_.send(JsonCodec::encode_seek());
}

void RemoteGameSession::send_cancel_seek() {
    client_.send(JsonCodec::encode_cancel_seek());
}

void RemoteGameSession::send_resign() {
    client_.send(JsonCodec::encode_resign());
}

void RemoteGameSession::send_move(const Position& src, const Position& dest) {
    client_.send(
        std::string("{\"type\":\"") + Protocol::kMove + "\",\"src\":{\"row\":" +
        std::to_string(src.getRow()) + ",\"col\":" + std::to_string(src.getCol()) +
        "},\"dest\":{\"row\":" + std::to_string(dest.getRow()) + ",\"col\":" +
        std::to_string(dest.getCol()) + "}}"
    );
}

void RemoteGameSession::send_jump(const Position& cell) {
    client_.send(
        std::string("{\"type\":\"") + Protocol::kJump + "\",\"cell\":{\"row\":" +
        std::to_string(cell.getRow()) + ",\"col\":" + std::to_string(cell.getCol()) + "}}"
    );
}

void RemoteGameSession::process_messages() {
    std::string message;
    while (client_.pop_message(message)) {
        handle_message(message);
    }
}

bool RemoteGameSession::has_lobby_state() const {
    return has_lobby_state_;
}

bool RemoteGameSession::can_join_as_black() const {
    if (!has_lobby_state_ || lobby_state_.game_started) {
        return false;
    }
    return lobby_state_.players_joined == 1 &&
           lobby_state_.white_username.has_value() &&
           !lobby_state_.black_username.has_value();
}

bool RemoteGameSession::auth_completed() const {
    return auth_completed_;
}

bool RemoteGameSession::auth_succeeded() const {
    return auth_succeeded_;
}

std::string RemoteGameSession::auth_error() const {
    return auth_error_;
}

int RemoteGameSession::player_rating() const {
    return player_rating_;
}

bool RemoteGameSession::game_started() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return game_started_;
}

bool RemoteGameSession::has_snapshot() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return has_snapshot_;
}

bool RemoteGameSession::opponent_disconnected() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return opponent_disconnected_ && disconnect_deadline_.has_value();
}

int RemoteGameSession::disconnect_countdown_seconds() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (!opponent_disconnected_ || !disconnect_deadline_.has_value()) {
        return 0;
    }

    const auto remaining = *disconnect_deadline_ - std::chrono::steady_clock::now();
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(remaining).count();
    return static_cast<int>(seconds > 0 ? seconds : 0);
}

GameSnapshot RemoteGameSession::snapshot_with_selection(std::optional<Position> selected_cell) const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    GameSnapshot snapshot = latest_snapshot_;
    snapshot.selected_cell = selected_cell;
    return snapshot;
}

const LobbyStateMessage& RemoteGameSession::lobby_state() const {
    return lobby_state_;
}

void RemoteGameSession::handle_message(const std::string& message) {
    if (const std::optional<AuthResultMessage> auth = JsonCodec::decode_auth_result(message)) {
        auth_completed_ = true;
        auth_succeeded_ = auth->success;
        auth_error_ = auth->reason;
        if (auth->success) {
            player_rating_ = auth->rating;
            username_ = auth->username;
        }
        return;
    }

    if (const std::optional<PlayerDisconnectedMessage> disconnected =
            JsonCodec::decode_player_disconnected(message)) {
        std::lock_guard<std::mutex> lock(state_mutex_);
        opponent_disconnected_ = true;
        disconnect_deadline_ = std::chrono::steady_clock::now() +
            std::chrono::seconds(disconnected->seconds_remaining);
        return;
    }

    if (const std::optional<GameStartedMessage> started = JsonCodec::decode_game_started(message)) {
        std::lock_guard<std::mutex> lock(state_mutex_);
        game_started_ = true;
        lobby_state_.game_started = true;
        lobby_state_.white_username = started->white_username;
        lobby_state_.black_username = started->black_username;
        lobby_state_.white_rating = started->white_rating;
        lobby_state_.black_rating = started->black_rating;
        lobby_state_.players_joined = 2;
        return;
    }

    if (const std::optional<LobbyStateMessage> lobby = JsonCodec::decode_lobby_state(message)) {
        lobby_state_ = *lobby;
        has_lobby_state_ = true;
        if (lobby->game_started) {
            std::lock_guard<std::mutex> lock(state_mutex_);
            game_started_ = true;
        }
        return;
    }

    if (const std::optional<GameSnapshot> snapshot = JsonCodec::decode_snapshot(message)) {
        std::lock_guard<std::mutex> lock(state_mutex_);
        latest_snapshot_ = *snapshot;
        has_snapshot_ = true;
        game_started_ = true;
        return;
    }

    if (const std::optional<GameOverMessage> game_over = JsonCodec::decode_game_over(message)) {
        if (username_ == game_over->white_username) {
            player_rating_ = game_over->white_rating;
        } else if (username_ == game_over->black_username) {
            player_rating_ = game_over->black_rating;
        }

        std::lock_guard<std::mutex> lock(state_mutex_);
        latest_snapshot_.game_over = true;
    }
}
