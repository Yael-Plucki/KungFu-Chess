#include "Motion.hpp"

Motion::Motion(Position src, Position dest, long long sTime, long long dur, int seq, Piece moving_piece)
    : source(src),
      destination(dest),
      startTime(sTime),
      duration(dur),
      sequence(seq),
      piece(moving_piece) {}

bool Motion::isFinished(long long currentTime) const {
    return (currentTime >= (startTime + duration));
}
