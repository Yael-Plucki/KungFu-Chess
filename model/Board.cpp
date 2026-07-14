#include "Board.hpp"

Board::Board(const std::vector<std::vector<Piece>>& initial_grid)
    : grid(initial_grid) {
    rows = static_cast<int>(grid.size());
    cols = (rows > 0) ? static_cast<int>(grid[0].size()) : 0;
    validate_no_duplicates();
}

void Board::validate_no_duplicates() const {
    std::set<int> seen_ids;
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            const Piece& piece = grid[row][col];
            if (piece.getKind() == Kind::Empty) {
                continue;
            }
            if (seen_ids.count(piece.getId()) > 0) {
                throw std::invalid_argument("duplicate piece id");
            }
            seen_ids.insert(piece.getId());
        }
    }
}

int Board::getRows() const { return rows; }
int Board::getCols() const { return cols; }

Piece Board::at(int row, int col) const {
    if (!isValidPosition(row, col)) {
        throw std::out_of_range("Board::at: position out of bounds");
    }
    return grid[row][col];
}

Piece Board::at(const Position& pos) const {
    return at(pos.getRow(), pos.getCol());
}

void Board::add_piece(const Piece& piece) {
    if (piece.getKind() == Kind::Empty) {
        throw std::invalid_argument("Board::add_piece: cannot add empty piece");
    }

    Position pos = piece.getPosition();
    if (!isValidPosition(pos)) {
        throw std::out_of_range("Board::add_piece: position out of bounds");
    }
    if (at(pos).getKind() != Kind::Empty) {
        throw std::invalid_argument("Board::add_piece: duplicate occupancy");
    }

    grid[pos.getRow()][pos.getCol()] = piece;
}

void Board::remove_piece(const Position& pos) {
    if (!isValidPosition(pos)) {
        throw std::out_of_range("Board::remove_piece: position out of bounds");
    }
    grid[pos.getRow()][pos.getCol()] = Piece::empty(pos);
}

void Board::move_piece(const Position& from, const Position& to) {
    if (!isValidPosition(from) || !isValidPosition(to)) {
        throw std::out_of_range("Board::move_piece: position out of bounds");
    }

    Piece piece = at(from);
    if (piece.getKind() == Kind::Empty) {
        throw std::invalid_argument("Board::move_piece: no piece at source");
    }

    remove_piece(from);
    piece.setPosition(to);
    if (at(to).getKind() != Kind::Empty) {
        update_piece(to, piece);
    } else {
        add_piece(piece);
    }
}

void Board::update_piece(const Position& pos, const Piece& piece) {
    if (!isValidPosition(pos)) {
        throw std::out_of_range("Board::update_piece: position out of bounds");
    }
    grid[pos.getRow()][pos.getCol()] = piece;
}

bool Board::isValidPosition(int row, int col) const {
    return (row >= 0 && row < rows && col >= 0 && col < cols);
}

bool Board::isValidPosition(const Position& pos) const {
    return isValidPosition(pos.getRow(), pos.getCol());
}
