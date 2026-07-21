#pragma once

#include "Piece.hpp"
#include "Position.hpp"

struct ActiveMotionInfo {
    bool active = false;
    Position source;
    Position destination;
    long long start_time = 0;
    long long duration = 0;
    int piece_id = 0;
    Color color = Color::White;
    Kind kind = Kind::Empty;
};
