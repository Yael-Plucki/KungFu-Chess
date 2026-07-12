#pragma once
#include "../model/Position.hpp"
#include "../model/Board.hpp"

// The Interface
class IMovementRule {
public:
    virtual ~IMovementRule() = default;
    virtual bool isValidMove(const Board& board, Position from, Position to) const = 0;
};

// Concrete Classes
class Rook : public IMovementRule {
public:
    bool isValidMove(const Board& board, Position from, Position to) const override { 
        int dr = abs(to.getRow() - from.getRow());
        int dc = abs(to.getCol() - from.getCol());
        return (dr == 0 && dc > 0) || (dr > 0 && dc == 0);
    }
};

class Bishop : public IMovementRule {
public:
    bool isValidMove(const Board& board, Position from, Position to) const override { 
        int dr = abs(to.getRow() - from.getRow());
int dc = abs(to.getCol() - from.getCol());
return (dr == dc) && (dr > 0);
     }
};

class Queen : public IMovementRule {
public:
    bool isValidMove(const Board& board, Position from, Position to) const override { 
        int dr = abs(to.getRow() - from.getRow());
int dc = abs(to.getCol() - from.getCol());
return (dr == 0 && dc > 0) || (dr > 0 && dc == 0) || (dr == dc && dr > 0);
     }
};

class King : public IMovementRule {
public:
    bool isValidMove(const Board& board, Position from, Position to) const override { 
        int dr = abs(to.getRow() - from.getRow());
        int dc = abs(to.getCol() - from.getCol());
        return (dr <= 1 && dc <= 1) && (dr + dc > 0);
     }
};

class Knight : public IMovementRule {
public:
    bool isValidMove(const Board& board, Position from, Position to) const override { 
        int dr = abs(to.getRow() - from.getRow());
        int dc = abs(to.getCol() - from.getCol());
        return (dr == 2 && dc == 1) || (dr == 1 && dc == 2);
     }
};

class Pawn : public IMovementRule {
public:
bool isForward(Color color, Position from, Position to) {
    int dr = to.getRow() - from.getRow();
    
    if (color == Color::White) {
        return dr == -1; // Moving up
    } else {
        return dr == 1;  // Moving down
    }
}

bool isOneStep(Position from, Position to) {
    int dr = std::abs(to.getRow() - from.getRow());
    int dc = std::abs(to.getCol() - from.getCol());
    
    return (dr == 1 && dc == 0);
}
// Inside Pawn class
bool Pawn::isValidMove(const Board& board, Position from, Position to) const override {
    // 1. Get the color from the board at the 'from' position
    Color color = board.getPieceAt(from).getColor(); 
    
    // 2. Now use your logic
    if (!isForward(color, from, to)) return false;
    return isOneStep(from, to);
}};