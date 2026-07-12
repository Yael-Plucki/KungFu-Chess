#pragma once

class Position {
    private:
        int row, col;
    public:
        Position(int r, int c);
    
        int getCol()const;
        int getRow()const;

        bool operator==(const Position& other) const;

        bool operator!=(const Position& other) const;
};