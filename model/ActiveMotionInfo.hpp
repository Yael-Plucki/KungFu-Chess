#pragma once

#include "Position.hpp"

struct ActiveMotionInfo {
    bool active = false;
    Position source;
    Position destination;
    long long start_time = 0;
    long long duration = 0;
};
