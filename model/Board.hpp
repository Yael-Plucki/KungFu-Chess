#pragma once
#include <vector>
#include <string> 
#include "Piece.hpp"
using namespace std;
class Board
{
    private:
        vector<vector<Piece>> grid;
    public:
        Board(const vestor<vestor<Piece> g);
        int getWidth()const;
        int getHeight() const;
        void addPiece(Piece p);
        void removePiece(Position p);
        std::optional<Piece> get_piece(Position p) const;
        bool is_inside(Position p) const;
        void move_piece(Position source, Position destination);
};
