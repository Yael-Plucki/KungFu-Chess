#include "GameState.hpp"

GameState::GameState(std::shared_ptr<Board> board) 
    : board(board), game_over(false) {}

std::shared_ptr<Board> GameState::get_board() const {
    return board;
}

bool GameState::is_game_over() const {
    return game_over;
}

void GameState::set_game_over(bool over) {
    game_over = over;
}