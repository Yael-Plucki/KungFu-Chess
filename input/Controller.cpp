#include "Controller.hpp"
#include "../engine/GameEngine.hpp"
#include "BoardMapper.hpp"

Controller::Controller(GameEngine& engine, const BoardMapper& mapper)
    : gameEngine(engine), boardMapper(mapper), selected_cell(std::nullopt) {}

void Controller::click(int x, int y) {
    std::optional<Position> clicked_cell = boardMapper.pixel_to_cell(x, y);

    if (!clicked_cell.has_value()) {
        if (selected_cell.has_value()) {
            selected_cell = std::nullopt;
        }
        return;
    }

    GameSnapshot snap = gameEngine.snapshot(selected_cell);

    if (!selected_cell.has_value()) {
        if (!snap.is_empty(clicked_cell.value())) {
            selected_cell = clicked_cell;
        }
        return;
    }

    std::optional<SnapshotPiece> selected_piece = snap.piece_at(selected_cell.value());
    std::optional<SnapshotPiece> clicked_piece = snap.piece_at(clicked_cell.value());
    if (clicked_piece.has_value() && selected_piece.has_value() &&
        clicked_piece->color == selected_piece->color) {
        selected_cell = clicked_cell;
        return;
    }

    gameEngine.request_move(selected_cell.value(), clicked_cell.value());
    selected_cell = std::nullopt;
}

void Controller::jump(int x, int y) {
    std::optional<Position> cell = boardMapper.pixel_to_cell(x, y);
    if (!cell.has_value()) {
        return;
    }

    GameSnapshot snap = gameEngine.snapshot(selected_cell);
    if (snap.is_empty(cell.value())) {
        return;
    }

    gameEngine.jump(cell.value());
    selected_cell = std::nullopt;
}

std::optional<Position> Controller::get_selected_cell() const {
    return selected_cell;
}
