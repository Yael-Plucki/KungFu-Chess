// class RuleEngine{
//     // bool moveValidation(Position from, Position to, Board board){
//     //     if(!board.isValidPosition(to))return false;
//     //     if(board.isEmpty(from))return false;
//     //     if(board.getPiece(from).getColor()==board.getPiece(to).getColor())return false;
//     //     Rook rook;
//     //     Bishop bishop;
//     //     Queen queen;
//     //     King king;
//     //     Knight knight;
//     //     Pawn pawn;
//     //     switch(p.getKind()) {
//     //         case Kind::Rook:
//     //             return rook.isValidMove(board, from, to);
//     //         case Kind::Bishop:
//     //             return bishop.isValidMove(board, from, to);
//     //         case Kind::Queen:
//     //             return queen.isValidMove(board, from, to);
//     //         case Kind::King:
//     //             return king.isValidMove(board, from, to);
//     //         case Kind::Knight:
//     //             return knight.isValidMove(board, from, to);
//     //         case Kind::Pawn:
//     //             return pawn.isValidMove(board, from, to);
//     //         default:
//     //             return false;
//     //     }    
//     // }
// }
#pragma once
#include "../model/Board.hpp"
#include <string>

struct MoveValidation {
    bool is_valid;
    std::string reason;
};

class RuleEngine {
public:
    MoveValidation validate_move(const Board& board, Position src, Position dest);
};