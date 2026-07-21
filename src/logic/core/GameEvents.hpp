#pragma once

#include "../model/GameStats.hpp"
#include "../model/Position.hpp"
#include <string>

struct MoveAcceptedEvent {
    Position src;
    Position dest;
};

struct MoveRejectedEvent {
    Position src;
    Position dest;
    std::string reason;
};

struct JumpStartedEvent {
    Position cell;
};

struct MoveResolvedEvent {
    MoveEvent move;
};

struct GameOverEvent {};

struct TimeAdvancedEvent {
    int delta_ms = 0;
    long long current_time_ms = 0;
};
