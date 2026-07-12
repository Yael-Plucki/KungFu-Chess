#include "BoardParser.hpp"
#include <stdexcept>
bool BoardParser::isValidToken(const std::string& token)
{
    if (token == ".")
        return true;

    if (token.length() != 2)
        return false;

    bool validColor =
        token[0] == 'w' || token[0] == 'b';

    bool validPiece =
        token[1] == 'K' ||
        token[1] == 'Q' ||
        token[1] == 'R' ||
        token[1] == 'B' ||
        token[1] == 'N' ||
        token[1] == 'P';

    return validColor && validPiece;
}
Piece BoardParser::parseToken(const std::string& token, int row, int col) {
    Color color = (token[0] == 'w') ? Color::White : Color::Black;
    Kind kind;

    switch (token[1]) {
        case 'R': kind = Kind::Rook; break;
        case 'B': kind = Kind::Bishop; break;
        case 'Q': kind = Kind::Queen; break;
        case 'K': kind = Kind::King; break;
        case 'N': kind = Kind::Knight; break;
        case 'P': 
        default:  kind = Kind::Pawn; break;
    }
    return Piece(color, kind, Position(row, col));
}
Board BoardParser::parseBoard() {
    std::vector<std::vector<Piece>> grid;
    std::string line;
    int expectedCols = -1; // Initialize with -1 to indicate first row

    while (std::getline(std::cin, line) && line != "Commands:") {
        if (line.find("Board:") != std::string::npos || line.empty()) continue;

        std::stringstream ss(line);
        std::string token;
        std::vector<Piece> row;
        int colIdx = 0;

        while (ss >> token) {
            if (!isValidToken(token)) {
                std::cerr << "ERROR UNKNOWN_TOKEN" << std::endl;
                throw std::runtime_error("ERROR UNKNOWN_TOKEN");
            }
            
            if (token == ".") {
                row.push_back(Piece(Color::White, Kind::Empty, Position(static_cast<int>(grid.size()), colIdx)));
            } else {
                row.push_back(parseToken(token, static_cast<int>(grid.size()), colIdx));
            }
            colIdx++;
        }

        // Check if this is the first row being processed
        if (expectedCols == -1) {
            expectedCols = colIdx;
        } 
        // Compare current row width with the expected width
        else if (colIdx != expectedCols) {
            std::cout << "ERROR ROW_WIDTH_MISMATCH" << std::endl;
            throw std::runtime_error("ERROR ROW_WIDTH_MISMATCH");
        }

        grid.push_back(row);
    }
    return Board(grid);
}