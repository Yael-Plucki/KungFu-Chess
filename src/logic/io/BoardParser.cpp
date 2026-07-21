#include "BoardParser.hpp"

#include <iostream>

#include <sstream>

#include <stdexcept>



bool BoardParser::isValidToken(const std::string& token) const {

    if (token == ".") return true;

    if (token.length() != 2) return false;

    bool validColor = (token[0] == 'w' || token[0] == 'b');

    bool validPiece = (token[1] == 'K' || token[1] == 'Q' || token[1] == 'R' || 

                       token[1] == 'B' || token[1] == 'N' || token[1] == 'P');

    return validColor && validPiece;

}



Piece BoardParser::parseToken(const std::string& token, int row, int col) const {

    Color color = (token[0] == 'w') ? Color::White : Color::Black;

    Kind kind;

    switch (token[1]) {

        case 'R': kind = Kind::Rook; break;

        case 'B': kind = Kind::Bishop; break;

        case 'Q': kind = Kind::Queen; break;

        case 'K': kind = Kind::King; break;

        case 'N': kind = Kind::Knight; break;

        case 'P': default: kind = Kind::Pawn; break;

    }

    return Piece(color, kind, Position(row, col));

}



Board& BoardParser::buildGrid(const std::vector<std::vector<Piece>>& grid) const {

    if (grid.empty()) {

        throw std::runtime_error("ERROR EMPTY_BOARD");

    }

    Board::getInstance().initialize(grid);
    return Board::getInstance();

}



Board& BoardParser::parseRows(const std::vector<std::string>& rows) const {

    std::vector<std::vector<Piece>> grid;

    int expectedCols = -1;



    for (const std::string& line : rows) {

        if (line.empty()) continue;



        std::stringstream ss(line);

        std::string token;

        std::vector<Piece> row;

        int colIdx = 0;



        while (ss >> token) {

            if (!isValidToken(token)) {

                throw std::runtime_error("ERROR UNKNOWN_TOKEN");

            }



            if (token == ".") {

                row.push_back(Piece::empty(Position(static_cast<int>(grid.size()), colIdx)));

            } else {

                row.push_back(parseToken(token, static_cast<int>(grid.size()), colIdx));

            }

            colIdx++;

        }



        if (colIdx == 0) continue;



        if (expectedCols == -1) {

            expectedCols = colIdx;

        } else if (colIdx != expectedCols) {

            throw std::runtime_error("ERROR ROW_WIDTH_MISMATCH");

        }



        grid.push_back(row);

    }



    return buildGrid(grid);

}



Board& BoardParser::parseBoard() const {

    std::vector<std::string> rows;

    std::string line;



    while (std::getline(std::cin, line) && line != "Commands:") {

        if (line.find("Board") != std::string::npos || line.empty()) continue;

        rows.push_back(line);

    }



    return parseRows(rows);

}

