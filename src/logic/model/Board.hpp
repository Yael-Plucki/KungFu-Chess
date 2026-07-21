#pragma once

#include <set>
#include <stdexcept>
#include <vector>
#include "Piece.hpp"
#include "Position.hpp"

class Board {
private:
    std::vector<std::vector<Piece>> grid;
    int rows, cols;

    Board() = default;
    Board(const Board&) = delete;
    Board& operator=(const Board&) = delete;

    void validate_no_duplicates() const;

public:
    static Board& getInstance();

    void initialize(const std::vector<std::vector<Piece>>& initial_grid);

    int getRows() const;
    int getCols() const;

    Piece at(const Position& pos) const;

    void add_piece(const Piece& piece);
    void remove_piece(const Position& pos);
    void move_piece(const Position& from, const Position& to);
    void update_piece(const Position& pos, const Piece& piece);

    bool isValidPosition(const Position& pos) const;
};
