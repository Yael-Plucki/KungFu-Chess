#include "RuleEngine.hpp"

bool GameState::isLegalMove(const std::string& piece,
    int fromRow,
    int fromCol,
    int toRow,
    int toCol) const 
{

    std::string destination = board.getPiece(toRow, toCol);

    if (destination != "." &&
        destination[0] == piece[0])
    {
        return false;
    }

    int dr = std::abs(toRow - fromRow);
    int dc = std::abs(toCol - fromCol);

    switch (piece[1])
    {
        case 'K':
            return dr <= 1 && dc <= 1 && (dr != 0 || dc != 0);

        case 'R':
            return ((dr == 0 && dc > 0) ||
            (dc == 0 && dr > 0))
            && isPathClear(fromRow, fromCol,
                        toRow, toCol);

        case 'B':
            return dr == dc &&
            dr > 0 &&
            isPathClear(fromRow, fromCol,
                    toRow, toCol);

        case 'Q':
            return (
                (dr == dc && dr > 0) ||
                (dr == 0 && dc > 0) ||
                (dc == 0 && dr > 0)
                )
                &&
                isPathClear(fromRow, fromCol,
                            toRow, toCol);

        case 'N':
            return (dr == 2 && dc == 1) ||
            (dr == 1 && dc == 2);

        case 'P':
            return isPawnMove(piece,
                fromRow,
                fromCol,
                toRow,
                toCol);

        default:
        return false;
    }
}
bool GameState::isPathClear(int fromRow,
    int fromCol,
    int toRow,
    int toCol) const
    {
        int rowStep = 0;
        int colStep = 0;
        
        if (toRow > fromRow)
            rowStep = 1;
        else if (toRow < fromRow)
            rowStep = -1;

        if (toCol > fromCol)
            colStep = 1;
        else if (toCol < fromCol)
            colStep = -1;

        int row = fromRow + rowStep;
        int col = fromCol + colStep;

        while (row != toRow || col != toCol)
        {
            if (board.getPiece(row, col) != ".")
            return false;

            row += rowStep;
            col += colStep;
        }

        return true;
}
int GameState::getMoveDistance(const std::string& piece,
    int fromRow,
    int fromCol,
    int toRow,
    int toCol) const
{
    int dr = std::abs(toRow - fromRow);
    int dc = std::abs(toCol - fromCol);

    switch (piece[1])
    {
        case 'R':
            return dr + dc;

        case 'B':
            return dr;

        case 'Q':
            if (dr == dc)
                return dr;

            return dr + dc;

        case 'K':
            return 1;

        case 'N':
            return 1;

        case 'P':
            return 1;

        default:
            return 1;
    }
}
