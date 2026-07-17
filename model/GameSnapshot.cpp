#include "GameSnapshot.hpp"
#include "Board.hpp"

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
    snap.current_time = current_time;

    for (int row = 0; row < board.getRows(); ++row) {
        for (int col = 0; col < board.getCols(); ++col) {
            Position cell(row, col);
            Piece piece = board.at(cell);
            if (piece.getKind() == Kind::Empty) {
                continue;
            }

            State render_state = piece.getState();
            const ActiveMotionInfo* motion = find_motion_for_cell(motions, cell);
            if (motion != nullptr) {
                render_state = State::Moving;
            }

            const bool is_jump = motion != nullptr && motion->source == motion->destination;

            snap.pieces.push_back(SnapshotPiece{
                piece.getId(),
                piece.getColor(),
                piece.getKind(),
                render_state,
                cell,
                motion != nullptr ? std::optional<ActiveMotionInfo>(*motion) : std::nullopt,
                is_jump
            });
        }
    }

    for (const ActiveMotionInfo& motion : motions) {
        if (!motion.active) {
            continue;
        }

        Position cell = motion.source;
        if (board.at(cell).getKind() != Kind::Empty) {
            continue;
        }

        const bool is_jump = motion.source == motion.destination;
        snap.pieces.push_back(SnapshotPiece{
            motion.piece_id,
            motion.color,
            motion.kind,
            State::Moving,
            cell,
            motion,
            is_jump
        });
    }

    return snap;
}
