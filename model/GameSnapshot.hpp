#pragma once

#include <optional>
#include <vector>
#include "ActiveMotionInfo.hpp"
#include "Board.hpp"
#include "Piece.hpp"
#include "Position.hpp"

struct SnapshotPiece {
    int id;
    Color color;
    Kind kind;
    State state;
    Position cell;
    int pixel_x;
    int pixel_y;
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
    static void cell_center(const Position& pos, int& pixel_x, int& pixel_y);
    static void motion_pixel(
        const ActiveMotionInfo& motion,
        long long current_time,
        int& pixel_x,
        int& pixel_y
    );
    static const ActiveMotionInfo* find_motion_for_cell(
        const std::vector<ActiveMotionInfo>& motions,
        const Position& cell
    );
};
