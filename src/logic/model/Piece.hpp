#pragma once
#include "Position.hpp"

enum class Color { White, Black };
enum class Kind { Empty, King, Queen, Rook, Bishop, Knight, Pawn };
enum class State { Idle, Moving, Captured };

class Piece {
    private:
        static int global_id;
        int id;
        Color color;
        Kind kind;
        Position cell;
        State state;
    public:
        Piece(Color clr, Kind k, Position cll)
            : id(global_id++), color(clr), kind(k), cell(cll), state(State::Idle) {}

        static Piece empty(const Position& pos) {
            return Piece(Color::White, Kind::Empty, pos);
        }

        // Getters
        int getId() const { return id; }
        Color getColor() const { return color; }
        Kind getKind() const { return kind; }
        Position getPosition() const { return cell; }
        State getState() const { return state; }

        // Setters
        void setPosition(Position newPos) { cell = newPos; }
        void setState(State newState) { state = newState; }
        void setKind(Kind newKind) { kind = newKind; }
};