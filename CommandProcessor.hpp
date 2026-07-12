#ifndef COMMANDPROCESSOR_H
#define COMMANDPROCESSOR_H

#include "GameState.hpp"

class CommandProcessor
{
private:
    GameState& game;

public:
    CommandProcessor(GameState& game);

    void run();
};

#endif