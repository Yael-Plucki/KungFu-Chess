#include "RuleEngine.hpp"
#include "PieceRules.hpp"

//all supposed to get snapshot?
MoveValidation RuleEngine::validate_move(const Board& board, const Position& src, const Position& dest) const {
    if (!board.isValidPosition(src) || !board.isValidPosition(dest)) {
        return {false, "outside_board"};
    }

    Piece source_piece = board.at(src);
    if (source_piece.getKind() == Kind::Empty) {
        return {false, "empty_source"};
    }

    Piece dest_piece = board.at(dest);
    if (dest_piece.getKind() != Kind::Empty && dest_piece.getColor() == source_piece.getColor()) {
        return {false, "friendly_destination"};
    }

    if (!PieceRules::is_destination_legal(board, src, dest)) {
        return {false, "illegal_piece_move"};
    }

    return {true, "ok"};
}
