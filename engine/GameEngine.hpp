#pragma once
#include "../rules/RuleEngine.hpp"
#include "../realtime/RealTimeArbiter.hpp"

struct MoveResult {
    bool is_accepted;
    std::string reason;
};

class GameEngine {
public:
    MoveResult request_move(Position src, Position dest);
    void wait(int ms);
};