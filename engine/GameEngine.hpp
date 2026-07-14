#pragma once



#include "../model/Board.hpp"

#include "../model/GameSnapshot.hpp"

#include "../rules/RuleEngine.hpp"

#include "../realtime/RealTimeArbiter.hpp"

#include <memory>

#include <string>



struct MoveResult {

    bool is_accepted;

    std::string reason;

};



class GameEngine {

private:

    std::shared_ptr<Board> board;

    RuleEngine ruleEngine;

    RealTimeArbiter arbiter;

    bool game_over;



public:

    explicit GameEngine(std::shared_ptr<Board> b);



    MoveResult request_move(const Position& src, const Position& dest);

    void jump(const Position& cell);

    void wait(int ms);

    bool is_game_over() const;

    GameSnapshot snapshot(std::optional<Position> selected_cell = std::nullopt) const;

    long long current_time_ms() const;

};

