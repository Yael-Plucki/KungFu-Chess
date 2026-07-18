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

int BoardMapper::display_width() const {
    return cols * GameConstants::CELL_SIZE;
}

int BoardMapper::display_height() const {
    return rows * GameConstants::CELL_SIZE;
}

int BoardMapper::cell_display_size() const {
    return GameConstants::CELL_SIZE;
}

void BoardMapper::cell_origin(const Position& cell, int& x, int& y) const {
    x = cell.getCol() * GameConstants::CELL_SIZE;
    y = cell.getRow() * GameConstants::CELL_SIZE;
}

void BoardMapper::cell_center(const Position& cell, int& x, int& y) const {
    x = cell.getCol() * GameConstants::CELL_SIZE + GameConstants::CELL_SIZE / 2;
    y = cell.getRow() * GameConstants::CELL_SIZE + GameConstants::CELL_SIZE / 2;
}

void BoardMapper::motion_center(
    const ActiveMotionInfo& motion,
    long long current_time,
    int& x,
    int& y
) const {
    int src_x = 0;
    int src_y = 0;
    int dest_x = 0;
    int dest_y = 0;
    cell_center(motion.source, src_x, src_y);
    cell_center(motion.destination, dest_x, dest_y);

    if (motion.duration <= 0 || current_time >= motion.start_time + motion.duration) {
        x = dest_x;
        y = dest_y;
        return;
    }

    double progress = (current_time - motion.start_time) / static_cast<double>(motion.duration);
    if (progress < 0.0) {
        progress = 0.0;
    }

    x = src_x + static_cast<int>((dest_x - src_x) * progress);
    y = src_y + static_cast<int>((dest_y - src_y) * progress);
}
