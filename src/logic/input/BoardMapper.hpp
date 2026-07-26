#pragma once
#include <optional>
#include "../model/ActiveMotionInfo.hpp"
#include "../model/GameConstants.hpp"
#include "../model/Position.hpp"

class BoardMapper {
private:
    int rows;
    int cols;
    int cell_size_;
    int side_panel_width_;

public:
    BoardMapper(
        int rows,
        int cols,
        int cell_size = GameConstants::CELL_SIZE,
        int side_panel_width = GameConstants::SIDE_PANEL_WIDTH
    );
    std::optional<Position> pixel_to_cell(int x, int y) const;
    int display_width() const;
    int display_height() const;
    int cell_display_size() const;
    int side_panel_width() const;
    int board_offset_x() const;
    void cell_origin(const Position& cell, int& x, int& y) const;
    void cell_center(const Position& cell, int& x, int& y) const;
    void motion_center(
        const ActiveMotionInfo& motion,
        long long current_time,
        int& x,
        int& y
    ) const;
};
