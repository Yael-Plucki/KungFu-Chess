#include "Board.hpp"

Board::Board(const std::vector<std::vector<Piece>>& initial_grid) 
    : grid(initial_grid) {
    rows = grid.size();
    cols = (rows > 0) ? grid[0].size() : 0;
}

int Board::getRows() const { return rows; }
int Board::getCols() const { return cols; }

Piece Board::at(int row, int col) const {
    return grid[row][col];
}

void Board::setPiece(int row, int col, const Piece& piece) {
    grid[row][col] = piece;
}

bool Board::isValidPosition(int row, int col) const {
    return (row >= 0 && row < rows && col >= 0 && col < cols);
}