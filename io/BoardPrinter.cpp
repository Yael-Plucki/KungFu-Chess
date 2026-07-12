#include "../model/Board.hpp"
#include "../io/BoardPrinter.hpp"
#include <iostream>
std::string BoardPrinter::parsePiece(Piece p) {
    if (p.getKind() == Kind::Empty) {
        return ".";
    }

    std::string color = (p.getColor() == Color::White) ? "w" : "b";
    
    char kindChar;
    switch (p.getKind()) {
        case Kind::Rook:   kindChar = 'R'; break;
        case Kind::Bishop: kindChar = 'B'; break;
        case Kind::Queen:  kindChar = 'Q'; break;
        case Kind::King:   kindChar = 'K'; break;
        case Kind::Knight: kindChar = 'N'; break;
        case Kind::Pawn:
        default:           kindChar = 'P'; break;
    }

    return color + kindChar;
}
void BoardPrinter::print(Board grid) const
{
    for (int i = 0; i < grid.getRows(); i++)
    {
        for (int j = 0; j < grid.getCols(); j++)
        {
            std::cout << parsePiece(grid.at(i, j));

            if (j + 1 < grid.getCols())
                std::cout << " ";
        }

        std::cout << std::endl;
    }
}