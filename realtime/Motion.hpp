#pragma once
#include "../model/Position.hpp"

class Motion {
public:
    Motion(Position src, Position dest, long long startTime, long long duration, int sequence);

    Position getSource() const { return source; }
    Position getDestination() const { return destination; }
    long long getStartTime() const { return startTime; }
    long long getDuration() const { return duration; }
    int getSequence() const { return sequence; }
    bool isFinished(long long currentTime) const;

private:
    Position source;
    Position destination;
    long long startTime;
    long long duration;
    int sequence;
};
