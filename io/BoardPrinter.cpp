#include "BoardPrinter.hpp"
#include <iostream>
#include <sstream>

std::string BoardPrinter::format_piece_token(const Piece& piece) const {
    if (piece.getKind() == Kind::Empty) {
        return ".";
    }

    std::string color = (piece.getColor() == Color::White) ? "w" : "b";
    char kind_char;
    switch (piece.getKind()) {
        case Kind::Rook: kind_char = 'R'; break;
        case Kind::Bishop: kind_char = 'B'; break;
        case Kind::Queen: kind_char = 'Q'; break;
        case Kind::King: kind_char = 'K'; break;
        case Kind::Knight: kind_char = 'N'; break;
        default: kind_char = 'P'; break;
    }
    return color + kind_char;
}

std::string BoardPrinter::format_snapshot_piece(const SnapshotPiece& piece) const {
    std::string color = (piece.color == Color::White) ? "w" : "b";
    char kind_char;
    switch (piece.kind) {
        case Kind::Rook: kind_char = 'R'; break;
        case Kind::Bishop: kind_char = 'B'; break;
        case Kind::Queen: kind_char = 'Q'; break;
        case Kind::King: kind_char = 'K'; break;
        case Kind::Knight: kind_char = 'N'; break;
        default: kind_char = 'P'; break;
    }
    return color + kind_char;
}

std::vector<std::string> BoardPrinter::format(const GameSnapshot& snapshot) const {
    std::vector<std::string> lines(snapshot.board_height, std::string());
    for (int row = 0; row < snapshot.board_height; ++row) {
        std::ostringstream line;
        for (int col = 0; col < snapshot.board_width; ++col) {
            Position cell(row, col);
            std::optional<SnapshotPiece> piece = snapshot.piece_at(cell);
            line << (piece.has_value() ? format_snapshot_piece(piece.value()) : ".");
            if (col + 1 < snapshot.board_width) {
                line << " ";
            }
        }
        lines[row] = line.str();
    }
    return lines;
}

void BoardPrinter::print(const GameSnapshot& snapshot) const {
    for (const std::string& line : format(snapshot)) {
        std::cout << line << std::endl;
    }
}

std::vector<std::string> BoardPrinter::format(const Board& board) const {
    std::vector<std::string> lines;
    for (int row = 0; row < board.getRows(); ++row) {
        std::ostringstream line;
        for (int col = 0; col < board.getCols(); ++col) {
            line << format_piece_token(board.at(row, col));
            if (col + 1 < board.getCols()) {
                line << " ";
            }
        }
        lines.push_back(line.str());
    }
    return lines;
}

void BoardPrinter::print(const Board& board) const {
    for (const std::string& line : format(board)) {
        std::cout << line << std::endl;
    }
}
