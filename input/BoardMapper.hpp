#pragma once
#include <optional>
#include "../model/GameConstants.hpp"
#include "../model/Position.hpp"

class BoardMapper {
private:
    int rows;
    int cols;

public:
    BoardMapper(int rows, int cols);
    std::optional<Position> pixel_to_cell(int x, int y) const;
};
