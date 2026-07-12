#include "Board.hpp"
#include <iostream>

Board::Board(const vector<Piece>& grid)
    : grid(grid)
{
}

int Board::getRows() const
{
    return grid.size();
}

int Board::getCols() const
{
    if (grid.empty())
        return 0;

    return grid[0].size();
}

bool Board::isValidPosition(Position p) const
{
    return p.getRow() >= 0 &&
    p.getRow() < getRows() &&
    p.getCol() >= 0 &&
    p.getCol() < getCols();
}

Piece Board::getPiece(Position p) const
{
    if (!isValidPosition(p))
        return "";

    return grid[p.getRow()][p.getCol()];
}
bool Board::isEmpty(Position p){
    if(grid[p.getRow()][p.getCol()].getKind()==Kind::Empty)return true;
    return false;
}
void Board::movePiece(Position to, const Piece& piece)
{
    if (isValidPosition(to) && isEmpty(to)){
        grid[p.getRow()][p.getCol()] = piece;
        piece.setPosition(to);
    }
}

