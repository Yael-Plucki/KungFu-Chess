#pragma once
#include <optional>
#include "../model/GameSnapshot.hpp"
#include "../model/Position.hpp"

class GameEngine;
class BoardMapper;

class Controller {
public:
    Controller(GameEngine& engine, const BoardMapper& mapper);
    void click(int x, int y);
    void jump(int x, int y);
    std::optional<Position> get_selected_cell() const;

private:
    GameEngine& gameEngine;
    const BoardMapper& boardMapper;
    std::optional<Position> selected_cell;
};
