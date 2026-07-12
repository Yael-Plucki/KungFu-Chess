#include <iostream>
#include <vector>
#include <sstream>

#include "model/Board.hpp"
#include "model/GameState.hpp"
#include "model/CommandProcessor.hpp"

int main()
{
    Board board=BoardParser::parseBoard();

    GameState game(board);

    CommandProcessor processor(game);

    processor.run();

    return 0;
}
