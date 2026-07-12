#include "GameEngine.hpp"
#include <string>
#include <cmath>

void GameEngine::wait(int ms)
{
    gameClock += ms;

    if (hasPendingMove && gameClock >= moveFinishTime)
    {
        std::string piece = movingPiece;
        std::string destination = board.getPiece(toRow, toCol);

        // אם יש כלי קופץ עדיין באוויר על משבצת היעד,
        // והוא אויב - הכלי המגיע נתפס.
        if (isAirborne &&
            toRow == airborneRow &&
            toCol == airborneCol &&
            piece[0] != airbornePiece[0])
        {
            board.setPiece(fromRow, fromCol, ".");
        }
        else
        {
            board.setPiece(toRow, toCol, piece);
            board.setPiece(fromRow, fromCol, ".");

            if (piece == "wP" && toRow == 0)
            {
                board.setPiece(toRow, toCol, "wQ");
            }

            if (piece == "bP" && toRow == board.getRows() - 1)
            {
                board.setPiece(toRow, toCol, "bQ");
            }

            if (destination == "wK" || destination == "bK")
            {
                gameOver = true;
            }
        }

        hasPendingMove = false;
        moveFinishTime = 0;

        movingPiece = "";

        fromRow = -1;
        fromCol = -1;
        toRow = -1;
        toCol = -1;

        selectedRow = -1;
        selectedCol = -1;
    }

        // סיום קפיצה
    if (hasPendingJump &&
        gameClock >= jumpFinishTime)
    {
        hasPendingJump = false;
        jumpFinishTime = 0;

        isAirborne = false;
        airborneRow = -1;
        airborneCol = -1;
        airbornePiece = "";
    }

}
MoveResult request_move(Position src, Position dest){
    if(gameState.is_game_over())return MoveResult(false, "game_over");
    if()
}
