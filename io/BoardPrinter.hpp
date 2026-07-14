#pragma once
#include "../model/Board.hpp"
#include "../model/GameSnapshot.hpp"
#include <string>
#include <vector>

class BoardPrinter {
public:
    void print(const GameSnapshot& snapshot) const;
    std::vector<std::string> format(const GameSnapshot& snapshot) const;

    void print(const Board& board) const;
    std::vector<std::string> format(const Board& board) const;

private:
    std::string format_piece_token(const Piece& piece) const;
    std::string format_snapshot_piece(const SnapshotPiece& piece) const;
};
