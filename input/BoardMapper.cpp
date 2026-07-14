#include "BoardMapper.hpp"
#include "../model/GameConstants.hpp"

BoardMapper::BoardMapper(int boardRows, int boardCols)
    : rows(boardRows), cols(boardCols) {}

std::optional<Position> BoardMapper::pixel_to_cell(int x, int y) const {
    int row = y / GameConstants::CELL_SIZE;
    int col = x / GameConstants::CELL_SIZE;

    if (row >= 0 && row < rows && col >= 0 && col < cols) {
        return Position(row, col);
    }

    return std::nullopt;
}
