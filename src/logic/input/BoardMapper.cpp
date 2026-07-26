#include "BoardMapper.hpp"
#include "../model/GameConstants.hpp"
#include <cmath>

BoardMapper::BoardMapper(int boardRows, int boardCols, int cellSize, int sidePanelWidth)
    : rows(boardRows), cols(boardCols), cell_size_(cellSize), side_panel_width_(sidePanelWidth) {}

std::optional<Position> BoardMapper::pixel_to_cell(int x, int y) const {
    x -= board_offset_x();
    int row = y / cell_size_;
    int col = x / cell_size_;

    if (row >= 0 && row < rows && col >= 0 && col < cols) {
        return Position(row, col);
    }

    return std::nullopt;
}

int BoardMapper::display_width() const {
    return cols * cell_size_ + 2 * side_panel_width_;
}

int BoardMapper::display_height() const {
    return rows * cell_size_;
}

int BoardMapper::cell_display_size() const {
    return cell_size_;
}

int BoardMapper::side_panel_width() const {
    return side_panel_width_;
}

int BoardMapper::board_offset_x() const {
    return side_panel_width_;
}

void BoardMapper::cell_origin(const Position& cell, int& x, int& y) const {
    x = cell.getCol() * cell_size_ + board_offset_x();
    y = cell.getRow() * cell_size_;
}

void BoardMapper::cell_center(const Position& cell, int& x, int& y) const {
    x = cell.getCol() * cell_size_ + cell_size_ / 2 + board_offset_x();
    y = cell.getRow() * cell_size_ + cell_size_ / 2;
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
    } else if (progress > 1.0) {
        progress = 1.0;
    }

    // Smoothstep easing reduces the clunky linear slide.
    progress = progress * progress * (3.0 - 2.0 * progress);

    x = src_x + static_cast<int>((dest_x - src_x) * progress);
    y = src_y + static_cast<int>((dest_y - src_y) * progress);
}
