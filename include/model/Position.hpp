#pragma once

class Position {
    int row, col;
    Position(int r, int c);

    bool operator==(const Position& other) const;

    bool operator!=(const Position& other) const;
    std::string toString() const;
};