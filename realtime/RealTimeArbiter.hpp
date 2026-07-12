#pragma once

class RealTimeArbiter{
    bool has_active_motion() const;

    void start_motion(Piece piece, Position source, Position destination);

    void advance_time(int ms);
}