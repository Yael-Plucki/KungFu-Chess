#pragma once

#include <set>
#include "../model/Board.hpp"
#include "../model/Piece.hpp"
#include "../model/Position.hpp"

class PieceRuleStrategy {
public:
    virtual ~PieceRuleStrategy() = default;
    virtual std::set<Position> legal_destinations(const Board& board, const Piece& piece) const = 0;
};

class RookRule : public PieceRuleStrategy {
public:
    std::set<Position> legal_destinations(const Board& board, const Piece& piece) const override;
};

class BishopRule : public PieceRuleStrategy {
public:
    std::set<Position> legal_destinations(const Board& board, const Piece& piece) const override;
};

class QueenRule : public PieceRuleStrategy {
public:
    std::set<Position> legal_destinations(const Board& board, const Piece& piece) const override;
};

class KnightRule : public PieceRuleStrategy {
public:
    std::set<Position> legal_destinations(const Board& board, const Piece& piece) const override;
};

class KingRule : public PieceRuleStrategy {
public:
    std::set<Position> legal_destinations(const Board& board, const Piece& piece) const override;
};

class PawnRule : public PieceRuleStrategy {
public:
    std::set<Position> legal_destinations(const Board& board, const Piece& piece) const override;
};

class PieceRules {
public:
    static std::set<Position> legal_destinations(const Board& board, const Piece& piece);
    static bool is_destination_legal(const Board& board, const Position& from, const Position& to);

private:
    static const PieceRuleStrategy& strategy_for(Kind kind);
};
