#include "GameSnapshot.hpp"
#include "Board.hpp"
#include "GameConstants.hpp"

void GameSnapshot::cell_center(const Position& pos, int& pixel_x, int& pixel_y) {
    pixel_x = pos.getCol() * GameConstants::CELL_SIZE + GameConstants::CELL_SIZE / 2;
    pixel_y = pos.getRow() * GameConstants::CELL_SIZE + GameConstants::CELL_SIZE / 2;
}

void GameSnapshot::motion_pixel(
    const ActiveMotionInfo& motion,
    long long current_time,
    int& pixel_x,
    int& pixel_y
) {
    int src_x = 0;
    int src_y = 0;
    int dest_x = 0;
    int dest_y = 0;
    cell_center(motion.source, src_x, src_y);
    cell_center(motion.destination, dest_x, dest_y);

    if (motion.duration <= 0 || current_time >= motion.start_time + motion.duration) {
        pixel_x = dest_x;
        pixel_y = dest_y;
        return;
    }

    double progress = (current_time - motion.start_time) / static_cast<double>(motion.duration);
    if (progress < 0.0) {
        progress = 0.0;
    }

    pixel_x = src_x + static_cast<int>((dest_x - src_x) * progress);
    pixel_y = src_y + static_cast<int>((dest_y - src_y) * progress);
}

const ActiveMotionInfo* GameSnapshot::find_motion_for_cell(
    const std::vector<ActiveMotionInfo>& motions,
    const Position& cell
) {
    for (const ActiveMotionInfo& motion : motions) {
        if (motion.active && motion.source == cell) {
            return &motion;
        }
    }
    return nullptr;
}

bool GameSnapshot::is_empty(const Position& pos) const {
    return !piece_at(pos).has_value();
}

std::optional<SnapshotPiece> GameSnapshot::piece_at(const Position& pos) const {
    for (const SnapshotPiece& piece : pieces) {
        if (piece.cell == pos) {
            return piece;
        }
    }
    return std::nullopt;
}

GameSnapshot GameSnapshot::create(
    const Board& board,
    bool is_game_over,
    std::optional<Position> selected_cell,
    const std::vector<ActiveMotionInfo>& motions,
    long long current_time
) {
    GameSnapshot snap;
    snap.board_width = board.getCols();
    snap.board_height = board.getRows();
    snap.selected_cell = selected_cell;
    snap.game_over = is_game_over;

    for (int row = 0; row < board.getRows(); ++row) {
        for (int col = 0; col < board.getCols(); ++col) {
            Position cell(row, col);
            Piece piece = board.at(cell);
            if (piece.getKind() == Kind::Empty) {
                continue;
            }

            int pixel_x = 0;
            int pixel_y = 0;
            State render_state = piece.getState();

            const ActiveMotionInfo* motion = find_motion_for_cell(motions, cell);
            if (motion != nullptr) {
                motion_pixel(*motion, current_time, pixel_x, pixel_y);
                render_state = State::Moving;
            } else {
                cell_center(cell, pixel_x, pixel_y);
            }

            const bool is_jump = motion != nullptr && motion->source == motion->destination;

            snap.pieces.push_back(SnapshotPiece{
                piece.getId(),
                piece.getColor(),
                piece.getKind(),
                render_state,
                cell,
                pixel_x,
                pixel_y,
                is_jump
            });
        }
    }

    return snap;
}
