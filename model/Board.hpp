#pragma once

#include <vector>
#include "Piece.hpp"
#include "Position.hpp"

class Board {
private:
    std::vector<std::vector<Piece>> grid;
    int rows, cols;

public:
    Board(const std::vector<std::vector<Piece>>& initial_grid);

    int getRows() const;
    int getCols() const;

    Piece at(int row, int col) const;
    void setPiece(int row, int col, const Piece& piece);
    
    bool isValidPosition(int row, int col) const;
};