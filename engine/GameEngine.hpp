#pragma once
#include "../rules/RuleEngine.hpp"
#include "../realtime/RealTimeArbiter.hpp"

struct MoveResult {
    bool is_accepted;
    std::string reason;
};

class GameEngine {
    GameState gameState;
public:
    MoveResult request_move(Position src, Position dest);
    void wait(int ms);
};

//     private:
//     Board board;
    
//     bool gameOver = false;
//     int selectedRow = -1;
//     int selectedCol = -1;
    
//     long long gameClock = 0;
    
//     bool isAirborne = false;
//     std::string airbornePiece;

//     std::string movingPiece;

//     int airborneRow = -1;
//     int airborneCol = -1;

//     static const int MOVE_DURATION = 1000;

//     bool hasPendingMove = false;
//     bool hasPendingJump = false;
    
//     long long moveFinishTime = 0;
//     long long jumpFinishTime = 0;

//     int fromRow = -1;
//     int fromCol = -1;
    
//     int toRow = -1;
//     int toCol = -1;
//     bool isLegalMove(const std::string& piece,
//         int fromRow,
//         int fromCol,
//         int toRow,
//         int toCol) const;
    
//     bool isPathClear(int fromRow,
//             int fromCol,
//             int toRow,
//             int toCol) const;
    
//     bool isPawnMove(const std::string& piece,
//             int fromRow,
//             int fromCol,
//             int toRow,
//             int toCol) const;
    
//     int getMoveDistance(const std::string& piece,
//             int fromRow,
//             int fromCol,
//             int toRow,
//             int toCol) const;

// public:
//     GameState(const Board& b);

//     void handleClick(int x, int y);

//     void wait(int ms);

//     void printBoard() const;

//     void jump(int x, int y);
