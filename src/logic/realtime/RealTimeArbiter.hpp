#pragma once

#include <vector>
#include "../model/ActiveMotionInfo.hpp"
#include "../model/Board.hpp"
#include "../model/GameConstants.hpp"
#include "../model/GameStats.hpp"
#include "../model/Position.hpp"
#include "Motion.hpp"

struct ArrivalEvents {
    bool arrived = false;
    bool king_captured = false;
    std::vector<MoveEvent> moves;
};

struct ArrivalResult {
    bool king_captured = false;
    std::optional<MoveEvent> move;
};

class RealTimeArbiter {
public:
    RealTimeArbiter();

    bool start_motion(Position src, Position dest);
    bool start_jump(const Position& cell);
    ArrivalEvents advance_time(int ms);
    bool has_active_motion() const;
    long long get_current_time() const;
    std::vector<ActiveMotionInfo> active_motion_infos() const;

private:
    std::vector<Motion> active_motions;
    long long current_time = 0;
    int next_motion_sequence = 0;

    bool has_motion_from(const Position& src) const;
    ArrivalResult resolve_arrival(const Motion& motion);
};
