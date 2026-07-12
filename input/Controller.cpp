void GameState::handleClick(int x, int y)
{
    if (gameOver)
        return;

    int row = y / 100;
    int col = x / 100;
    
    if (!board.isValidPosition(row, col))
    return;

    std::string piece = board.getPiece(row, col);

    if (isAirborne &&
        piece != "." &&
        piece[0] != airbornePiece[0] &&
        row == airborneRow &&
        col == airborneCol)
        {
            board.setPiece(row, col, ".");
            return;
        }
    
    if (hasPendingMove)
        return;

    if (selectedRow == -1)
    {
        if (piece != ".")
        {
            selectedRow = row;
            selectedCol = col;
        }
        
        return;
    }
    
    std::string selectedPiece = board.getPiece(selectedRow, selectedCol);

    if (piece != "." &&
        piece[0] == selectedPiece[0])
    {
        selectedRow = row;
        selectedCol = col;
    }
    else
    {
        if (!isLegalMove(selectedPiece,
            selectedRow,
            selectedCol,
            row,
            col))
        {
            return;
        }

        hasPendingMove = true;
        moveFinishTime =
        gameClock +
        getMoveDistance(selectedPiece,
                        selectedRow,
                        selectedCol,
                        row,
                        col) * MOVE_DURATION;

        fromRow = selectedRow;
        fromCol = selectedCol;
        toRow = row;
        toCol = col;
        movingPiece = selectedPiece;
        selectedRow = -1;
        selectedCol = -1;
    }
}
void GameState::jump(int x, int y)
{
    if (gameOver)
        return;

    if (hasPendingMove)
        return;

    int row = y / 100;
    int col = x / 100;

    if (!board.isValidPosition(row, col))
        return;

    if (board.getPiece(row, col) == ".")
        return;

    isAirborne = true;

    airborneRow = row;
    airborneCol = col;
    airbornePiece = board.getPiece(row, col);

    hasPendingJump = true;
    jumpFinishTime = gameClock + MOVE_DURATION;

    selectedRow = -1;
    selectedCol = -1;
}
