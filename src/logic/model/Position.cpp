#include "Position.hpp"

Position::Position(int r, int c) : row(r), col(c) {}

int Position::getRow() const { return row; }
int Position::getCol() const { return col; }

bool Position::operator==(const Position& other) const {
    return row == other.row && col == other.col;
}

bool Position::operator!=(const Position& other) const {
    return !(*this == other);
}

bool Position::operator<(const Position& other) const {
    if (row != other.row) {
        return row < other.row;
    }
    return col < other.col;
}