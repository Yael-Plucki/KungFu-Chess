#pragma once
#include "../model/Board.hpp"
#include "../model/Piece.hpp"
#include <string>
#include <vector>

class BoardParser {
public:
    Board& parseBoard() const;
    Board& parseRows(const std::vector<std::string>& rows) const;
private:
    bool isValidToken(const std::string& token) const;
    Piece parseToken(const std::string& token, int row, int col) const;
    Board& buildGrid(const std::vector<std::vector<Piece>>& grid) const;
};