#include "RemoteController.hpp"

RemoteController::RemoteController(RemoteGameSession& session, const BoardMapper& mapper)
    : session_(session), board_mapper_(mapper), selected_cell_(std::nullopt) {}

void RemoteController::click(int x, int y) {
    const std::optional<Position> clicked_cell = board_mapper_.pixel_to_cell(x, y);

    if (!clicked_cell.has_value()) {
        if (selected_cell_.has_value()) {
            selected_cell_ = std::nullopt;
        }
        return;
    }

    const GameSnapshot snap = session_.snapshot_with_selection(selected_cell_);

    if (!selected_cell_.has_value()) {
        if (!snap.is_empty(clicked_cell.value())) {
            selected_cell_ = clicked_cell;
        }
        return;
    }

    const std::optional<SnapshotPiece> selected_piece = snap.piece_at(selected_cell_.value());
    const std::optional<SnapshotPiece> clicked_piece = snap.piece_at(clicked_cell.value());
    if (clicked_piece.has_value() && selected_piece.has_value() &&
        clicked_piece->color == selected_piece->color) {
        selected_cell_ = clicked_cell;
        return;
    }

    session_.send_move(selected_cell_.value(), clicked_cell.value());
    selected_cell_ = std::nullopt;
}

void RemoteController::jump(int x, int y) {
    const std::optional<Position> cell = board_mapper_.pixel_to_cell(x, y);
    if (!cell.has_value()) {
        return;
    }

    const GameSnapshot snap = session_.snapshot_with_selection(selected_cell_);
    if (snap.is_empty(cell.value())) {
        return;
    }

    session_.send_jump(cell.value());
    selected_cell_ = std::nullopt;
}

std::optional<Position> RemoteController::get_selected_cell() const {
    return selected_cell_;
}
