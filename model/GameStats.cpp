#include "GameStats.hpp"
#include "GameConstants.hpp"
#include <sstream>

int piece_material_value(Kind kind) {
    switch (kind) {
        case Kind::Pawn:
            return GameConstants::SCORE_PAWN;
        case Kind::Knight:
            return GameConstants::SCORE_KNIGHT;
        case Kind::Bishop:
            return GameConstants::SCORE_BISHOP;
        case Kind::Rook:
            return GameConstants::SCORE_ROOK;
        case Kind::Queen:
            return GameConstants::SCORE_QUEEN;
        default:
            return 0;
    }
}

std::string format_board_position(const Position& pos, int board_rows, int board_cols) {
    if (board_rows == 8 && board_cols == 8) {
        const char file = static_cast<char>('a' + pos.getCol());
        const int rank = board_rows - pos.getRow();
        return std::string(1, file) + std::to_string(rank);
    }

    std::ostringstream out;
    out << "(" << pos.getRow() << "," << pos.getCol() << ")";
    return out.str();
}

std::string format_piece_label(Color color, Kind kind) {
    std::string label = (color == Color::White) ? "w" : "b";
    switch (kind) {
        case Kind::Rook:
            label += 'R';
            break;
        case Kind::Bishop:
            label += 'B';
            break;
        case Kind::Queen:
            label += 'Q';
            break;
        case Kind::King:
            label += 'K';
            break;
        case Kind::Knight:
            label += 'N';
            break;
        default:
            label += 'P';
            break;
    }
    return label;
}

std::string format_move_event(const MoveEvent& event, int board_rows, int board_cols) {
    if (event.is_jump) {
        std::string move = format_board_position(event.from, board_rows, board_cols) + " jump";
        if (event.captured.has_value()) {
            move += " x";
        }
        return move;
    }

    std::string move = format_board_position(event.from, board_rows, board_cols) + "-" +
                       format_board_position(event.to, board_rows, board_cols);
    if (event.captured.has_value()) {
        move += " x";
    }
    if (event.promoted_to.has_value()) {
        move += "=" + format_piece_label(event.color, event.promoted_to.value()).substr(1);
    }
    return move;
}

ColorSideStats& GameStats::for_color(Color color) {
    return color == Color::White ? white : black;
}

const ColorSideStats& GameStats::for_color(Color color) const {
    return color == Color::White ? white : black;
}

void GameStats::record_move(const MoveEvent& event, int board_rows, int board_cols) {
    ColorSideStats& side = for_color(event.color);
    if (event.captured.has_value()) {
        side.score += piece_material_value(event.captured.value());
    }

    for (PieceMoveHistory& piece : side.pieces) {
        if (piece.piece_id == event.piece_id) {
            piece.moves.push_back(format_move_event(event, board_rows, board_cols));
            if (event.promoted_to.has_value()) {
                piece.kind = event.promoted_to.value();
            }
            return;
        }
    }

    side.pieces.push_back(PieceMoveHistory{
        event.piece_id,
        event.color,
        event.kind,
        {format_move_event(event, board_rows, board_cols)}
    });
    if (event.promoted_to.has_value()) {
        side.pieces.back().kind = event.promoted_to.value();
    }
}
