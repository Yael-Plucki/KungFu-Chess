#pragma once
#include "../model/Position.hpp"
#include "..model/Board.hpp"

class PieceRules {
    virtual ~PieceRules() = default;
    virtual std::set<Position> legal_destinations(const Board& board, const Piece& piece) = 0;
};