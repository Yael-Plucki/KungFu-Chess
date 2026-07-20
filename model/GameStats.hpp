#pragma once

#include "Piece.hpp"
#include "Position.hpp"
#include <optional>
#include <string>
#include <vector>

struct MoveEvent {
    int piece_id = 0;
    Color color = Color::White;
    Kind kind = Kind::Empty;
    Position from;
    Position to;
    std::optional<Kind> captured;
    bool is_jump = false;
    std::optional<Kind> promoted_to;
};

struct PieceMoveHistory {
    int piece_id = 0;
    Color color = Color::White;
    Kind kind = Kind::Empty;
    std::vector<std::string> moves;
};

struct ColorSideStats {
    int score = 0;
    std::vector<PieceMoveHistory> pieces;
};

struct GameStats {
    ColorSideStats white;
    ColorSideStats black;

    ColorSideStats& for_color(Color color);
    const ColorSideStats& for_color(Color color) const;
    void record_move(const MoveEvent& event, int board_rows, int board_cols);
};

int piece_material_value(Kind kind);
std::string format_board_position(const Position& pos, int board_rows, int board_cols);
std::string format_piece_label(Color color, Kind kind);
std::string format_move_event(const MoveEvent& event, int board_rows, int board_cols);
