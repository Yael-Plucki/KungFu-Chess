#include "PieceRules.hpp"
#include <cmath>

namespace {
int pawnStartingRow(Color color, int rows) {
    if (color == Color::Black) {
        return 1;
    }
    return (rows > 3) ? rows - 2 : rows - 1;
}

void collect_sliding_destinations(
    const Board& board,
    const Position& from,
    Color color,
    int row_step,
    int col_step,
    std::set<Position>& destinations
) {
    int row = from.getRow() + row_step;
    int col = from.getCol() + col_step;

    while (board.isValidPosition(Position(row, col))) {
        Piece occupant = board.at(Position(row, col));
        if (occupant.getKind() == Kind::Empty) {
            destinations.insert(Position(row, col));
        } else {
            if (occupant.getColor() != color) {
                destinations.insert(Position(row, col));
            }
            break;
        }
        row += row_step;
        col += col_step;
    }
}
}  // namespace

std::set<Position> RookRule::legal_destinations(const Board& board, const Piece& piece) const {
    std::set<Position> destinations;
    Position from = piece.getPosition();
    static const int directions[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    for (const auto& direction : directions) {
        collect_sliding_destinations(
            board, from, piece.getColor(), direction[0], direction[1], destinations
        );
    }

    return destinations;
}

std::set<Position> BishopRule::legal_destinations(const Board& board, const Piece& piece) const {
    std::set<Position> destinations;
    Position from = piece.getPosition();
    static const int directions[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

    for (const auto& direction : directions) {
        collect_sliding_destinations(
            board, from, piece.getColor(), direction[0], direction[1], destinations
        );
    }

    return destinations;
}

std::set<Position> QueenRule::legal_destinations(const Board& board, const Piece& piece) const {
    std::set<Position> destinations = RookRule().legal_destinations(board, piece);
    std::set<Position> bishop = BishopRule().legal_destinations(board, piece);
    destinations.insert(bishop.begin(), bishop.end());
    return destinations;
}

std::set<Position> KnightRule::legal_destinations(const Board& board, const Piece& piece) const {
    std::set<Position> destinations;
    Position from = piece.getPosition();
    static const int jumps[8][2] = {
        {2, 1}, {2, -1}, {-2, 1}, {-2, -1},
        {1, 2}, {1, -2}, {-1, 2}, {-1, -2}
    };

    for (const auto& jump : jumps) {
        int row = from.getRow() + jump[0];
        int col = from.getCol() + jump[1];
        if (!board.isValidPosition(Position(row, col))) {
            continue;
        }

        Piece occupant = board.at(Position(row, col));
        if (occupant.getKind() == Kind::Empty || occupant.getColor() != piece.getColor()) {
            destinations.insert(Position(row, col));
        }
    }

    return destinations;
}

std::set<Position> KingRule::legal_destinations(const Board& board, const Piece& piece) const {
    std::set<Position> destinations;
    Position from = piece.getPosition();

    for (int row = from.getRow() - 1; row <= from.getRow() + 1; ++row) {
        for (int col = from.getCol() - 1; col <= from.getCol() + 1; ++col) {
            if (row == from.getRow() && col == from.getCol()) {
                continue;
            }
            if (!board.isValidPosition(Position(row, col))) {
                continue;
            }

            Piece occupant = board.at(Position(row, col));
            if (occupant.getKind() == Kind::Empty || occupant.getColor() != piece.getColor()) {
                destinations.insert(Position(row, col));
            }
        }
    }

    return destinations;
}

std::set<Position> PawnRule::legal_destinations(const Board& board, const Piece& piece) const {
    std::set<Position> destinations;
    Position from = piece.getPosition();
    int direction = (piece.getColor() == Color::White) ? -1 : 1;
    int target_row = from.getRow() + direction;

    if (board.isValidPosition(Position(target_row, from.getCol())) &&
        board.at(Position(target_row, from.getCol())).getKind() == Kind::Empty) {
        destinations.insert(Position(target_row, from.getCol()));
    }

    int double_target_row = from.getRow() + 2 * direction;
    if (from.getRow() == pawnStartingRow(piece.getColor(), board.getRows()) &&
        board.isValidPosition(Position(double_target_row, from.getCol()))) {
        int middle_row = from.getRow() + direction;
        if (board.at(Position(middle_row, from.getCol())).getKind() == Kind::Empty &&
            board.at(Position(double_target_row, from.getCol())).getKind() == Kind::Empty) {
            destinations.insert(Position(double_target_row, from.getCol()));
        }
    }

    for (int col_offset : {-1, 1}) {
        int capture_col = from.getCol() + col_offset;
        if (!board.isValidPosition(Position(target_row, capture_col))) {
            continue;
        }
        Piece target = board.at(Position(target_row, capture_col));
        if (target.getKind() != Kind::Empty && target.getColor() != piece.getColor()) {
            destinations.insert(Position(target_row, capture_col));
        }
    }

    return destinations;
}

const PieceRuleStrategy& PieceRules::strategy_for(Kind kind) {
    static RookRule rook;
    static BishopRule bishop;
    static QueenRule queen;
    static KnightRule knight;
    static KingRule king;
    static PawnRule pawn;

    switch (kind) {
        case Kind::Rook: return rook;
        case Kind::Bishop: return bishop;
        case Kind::Queen: return queen;
        case Kind::Knight: return knight;
        case Kind::King: return king;
        case Kind::Pawn: return pawn;
        default: throw std::invalid_argument("unsupported piece kind");
    }
}

std::set<Position> PieceRules::legal_destinations(const Board& board, const Piece& piece) {
    if (piece.getKind() == Kind::Empty) {
        return {};
    }
    return strategy_for(piece.getKind()).legal_destinations(board, piece);
}

bool PieceRules::is_destination_legal(const Board& board, const Position& from, const Position& to) {
    if (from == to || !board.isValidPosition(to)) {
        return false;
    }

    Piece piece = board.at(from);
    if (piece.getKind() == Kind::Empty) {
        return false;
    }

    std::set<Position> destinations = legal_destinations(board, piece);
    return destinations.count(to) > 0;
}
