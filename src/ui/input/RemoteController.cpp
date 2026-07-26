#include "RemoteController.hpp"

#include "core/GameEvents.hpp"

RemoteController::RemoteController(
    RemoteGameSession& session,
    ClientBoardSync& board_sync,
    const BoardMapper& mapper
)
    : session_(session), board_sync_(board_sync), board_mapper_(mapper), selected_cell_(std::nullopt) {
    move_rejected_subscription_ = board_sync_.event_bus().subscribe<MoveRejectedEvent>(
        [this](const MoveRejectedEvent&) {
            selected_cell_ = std::nullopt;
        }
    );
}

void RemoteController::click(int x, int y) {
    const std::optional<Position> clicked_cell = board_mapper_.pixel_to_cell(x, y);

    if (!clicked_cell.has_value()) {
        if (selected_cell_.has_value()) {
            selected_cell_ = std::nullopt;
        }
        return;
    }

    const GameSnapshot snap = board_sync_.snapshot_with_selection(selected_cell_);
    const std::optional<Color> my_color = session_.player_color();

    if (!selected_cell_.has_value()) {
        const std::optional<SnapshotPiece> clicked_piece = snap.piece_at(clicked_cell.value());
        if (!clicked_piece.has_value()) {
            return;
        }
        if (my_color.has_value() && clicked_piece->color != my_color.value()) {
            return;
        }
        selected_cell_ = clicked_cell;
        return;
    }

    const std::optional<SnapshotPiece> selected_piece = snap.piece_at(selected_cell_.value());
    const std::optional<SnapshotPiece> clicked_piece = snap.piece_at(clicked_cell.value());
    if (clicked_piece.has_value() && selected_piece.has_value() &&
        clicked_piece->color == selected_piece->color) {
        if (my_color.has_value() && clicked_piece->color != my_color.value()) {
            selected_cell_ = std::nullopt;
            return;
        }
        selected_cell_ = clicked_cell;
        return;
    }

    if (selected_piece.has_value() && my_color.has_value() &&
        selected_piece->color != my_color.value()) {
        selected_cell_ = std::nullopt;
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

    const GameSnapshot snap = board_sync_.snapshot_with_selection(selected_cell_);
    const std::optional<SnapshotPiece> piece = snap.piece_at(cell.value());
    if (!piece.has_value()) {
        return;
    }

    const std::optional<Color> my_color = session_.player_color();
    if (my_color.has_value() && piece->color != my_color.value()) {
        return;
    }

    session_.send_jump(cell.value());
    selected_cell_ = std::nullopt;
}

std::optional<Position> RemoteController::get_selected_cell() const {
    return selected_cell_;
}
