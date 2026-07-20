#include "Lobby.hpp"

Lobby::JoinResult Lobby::try_login(const std::string& connection_id, const std::string& username, int rating) {
    if (game_started_) {
        return JoinResult::Full;
    }

    if (white_.has_value() && white_->connection_id == connection_id) {
        return JoinResult::AlreadyJoined;
    }
    if (black_.has_value() && black_->connection_id == connection_id) {
        return JoinResult::AlreadyJoined;
    }

    if (!white_.has_value()) {
        white_ = LobbyPlayer{connection_id, username, Color::White, rating};
        return JoinResult::White;
    }

    if (!black_.has_value()) {
        black_ = LobbyPlayer{connection_id, username, Color::Black, rating};
        return JoinResult::Black;
    }

    return JoinResult::Full;
}

bool Lobby::is_ready() const {
    return white_.has_value() && black_.has_value();
}

bool Lobby::is_started() const {
    return game_started_;
}

void Lobby::start_game() {
    game_started_ = true;
}

void Lobby::set_winner(Color color) {
    winner_ = color;
}

void Lobby::update_ratings(int white_rating, int black_rating) {
    if (white_.has_value()) {
        white_->rating = white_rating;
    }
    if (black_.has_value()) {
        black_->rating = black_rating;
    }
}

int Lobby::players_joined() const {
    return static_cast<int>(white_.has_value()) + static_cast<int>(black_.has_value());
}

const std::optional<LobbyPlayer>& Lobby::white_player() const {
    return white_;
}

const std::optional<LobbyPlayer>& Lobby::black_player() const {
    return black_;
}

std::optional<Color> Lobby::color_for_connection(const std::string& connection_id) const {
    if (white_.has_value() && white_->connection_id == connection_id) {
        return Color::White;
    }
    if (black_.has_value() && black_->connection_id == connection_id) {
        return Color::Black;
    }
    return std::nullopt;
}

std::optional<Color> Lobby::winner_color() const {
    return winner_;
}
