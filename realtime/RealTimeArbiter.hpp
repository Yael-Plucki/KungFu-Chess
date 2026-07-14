#pragma once

#include <memory>
#include <set>
#include <vector>
#include "../model/ActiveMotionInfo.hpp"
#include "../model/Board.hpp"
#include "../model/GameConstants.hpp"
#include "../model/Position.hpp"
#include "Motion.hpp"

struct ArrivalEvents {
    bool arrived = false;
    bool king_captured = false;
};

class RealTimeArbiter {
public:
    explicit RealTimeArbiter(std::shared_ptr<Board> board);

    bool start_motion(Position src, Position dest);
    bool start_jump(const Position& cell);
    ArrivalEvents advance_time(int ms);
    bool has_active_motion() const;
    long long get_current_time() const;
    std::vector<ActiveMotionInfo> active_motion_infos() const;

private:
    std::shared_ptr<Board> board;
    std::vector<Motion> active_motions;
    long long current_time = 0;
    int next_motion_sequence = 0;

    bool has_motion_from(const Position& src) const;
    static const Motion* find_opposing_motion(
        const Position& from,
        const Position& to,
        const std::vector<Motion>& motions
    );
    bool resolve_arrival(
        const Motion& motion,
        const std::vector<Motion>& finished,
        std::set<Position>& cancelled_sources
    );
};
