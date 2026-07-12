#pragma once

#include <memory>
#include "Board.hpp"

class GameState {
public:
    GameState(std::shared_ptr<Board> board);

    std::shared_ptr<Board> get_board() const;
    bool is_game_over() const;
    void set_game_over(bool over);

private:
    std::shared_ptr<Board> board;
    bool game_over;
};