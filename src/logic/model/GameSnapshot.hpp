#pragma once

#include <optional>
#include <vector>
#include "ActiveMotionInfo.hpp"
#include "Board.hpp"
#include "GameStats.hpp"
#include "Piece.hpp"
#include "Position.hpp"

struct SnapshotPiece {
    int id;
    Color color;
    Kind kind;
    State state;
    Position cell;
    std::optional<ActiveMotionInfo> motion;
    bool is_jump_motion = false;
};

// Read-only view DTO for the renderer, controller, and printer.
// GameEngine owns mutable state; GameSnapshot never exposes live Board/Piece objects.
struct GameSnapshot {
    int board_width;
    int board_height;
    std::vector<SnapshotPiece> pieces;
    std::optional<Position> selected_cell;
    bool game_over;
    long long current_time = 0;
    GameStats stats;
    std::string white_player_name;
    std::string black_player_name;
    int white_player_rating = 1200;
    int black_player_rating = 1200;

    bool is_empty(const Position& pos) const;
    std::optional<SnapshotPiece> piece_at(const Position& pos) const;

    static GameSnapshot create(
        const Board& board,
        bool game_over,
        std::optional<Position> selected_cell,
        const std::vector<ActiveMotionInfo>& motions,
        long long current_time
    );

private:
    static const ActiveMotionInfo* find_motion_for_cell(
        const std::vector<ActiveMotionInfo>& motions,
        const Position& cell
    );
};
