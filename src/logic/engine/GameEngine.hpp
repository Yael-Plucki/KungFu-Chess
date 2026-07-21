#pragma once

#include "../core/EventBus.hpp"
#include "../model/Board.hpp"
#include "../model/GameSnapshot.hpp"
#include "../model/GameStats.hpp"
#include "../rules/RuleEngine.hpp"
#include "../realtime/RealTimeArbiter.hpp"
#include <string>



struct MoveResult {

    bool is_accepted;

    std::string reason;

};



class GameEngine {

private:

    RuleEngine ruleEngine;

    RealTimeArbiter arbiter;

    EventBus eventBus;

    bool game_over;

    GameStats stats;



public:

    GameEngine();

    EventBus& event_bus();

    MoveResult request_move(const Position& src, const Position& dest);

    void jump(const Position& cell);

    void wait(int ms);

    bool is_game_over() const;

    void force_game_over(bool publish_event = true);

    GameSnapshot snapshot(std::optional<Position> selected_cell = std::nullopt) const;

    long long current_time_ms() const;

};

