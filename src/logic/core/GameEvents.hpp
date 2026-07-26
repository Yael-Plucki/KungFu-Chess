#pragma once

#include "../model/GameSnapshot.hpp"
#include "../model/GameStats.hpp"
#include "../model/Position.hpp"
#include <string>

struct MoveAcceptedEvent {
    Position src;
    Position dest;
    long long start_time_ms = 0;
    long long duration_ms = 0;
    long long current_time_ms = 0;
};

struct MoveRejectedEvent {
    Position src;
    Position dest;
    std::string reason;
};

struct JumpStartedEvent {
    Position cell;
    long long start_time_ms = 0;
    long long duration_ms = 0;
    long long current_time_ms = 0;
};

struct MoveResolvedEvent {
    MoveEvent move;
};

struct GameOverEvent {};

struct TimeAdvancedEvent {
    int delta_ms = 0;
    long long current_time_ms = 0;
};

struct SnapshotUpdatedEvent {
    GameSnapshot snapshot;
};
