#pragma once
#include "../model/Board.hpp"
#include "../model/Position.hpp"
#include <string>

struct MoveValidation {
    bool is_valid;
    std::string reason;
};

class RuleEngine {
public:
    // Read-only legality validation for a requested move
    MoveValidation validate_move(const Board& board, const Position& src, const Position& dest) const;
};