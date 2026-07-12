#include "Position.hpp"

Position::Position(int r, int c) : row(r), col(c) {}

int Position::getRow() const { return row; }
int Position::getCol() const { return col; }

std::string Position::toString() const {
    return "(" + std::to_string(row) + ", " + std::to_string(col) + ")";
}

bool Position::operator==(const Position& other) const {
    return row == other.row && col == other.col;
}

bool Position::operator!=(const Position& other) const {
    return !(*this == other);
}