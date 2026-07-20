#pragma once

#include "../model/Piece.hpp"
#include <optional>
#include <string>

struct LobbyPlayer {
    std::string connection_id;
    std::string username;
    Color color = Color::White;
    int rating = 1200;
};

class Lobby {
public:
    enum class JoinResult { White, Black, Full, AlreadyJoined };

    JoinResult try_login(const std::string& connection_id, const std::string& username, int rating);
    bool is_ready() const;
    bool is_started() const;
    void start_game();
    void set_winner(Color color);
    void update_ratings(int white_rating, int black_rating);

    int players_joined() const;
    const std::optional<LobbyPlayer>& white_player() const;
    const std::optional<LobbyPlayer>& black_player() const;
    std::optional<Color> color_for_connection(const std::string& connection_id) const;
    std::optional<Color> winner_color() const;

private:
    std::optional<LobbyPlayer> white_;
    std::optional<LobbyPlayer> black_;
    bool game_started_ = false;
    std::optional<Color> winner_;
};
