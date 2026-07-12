#pragma once
#include "../model/Position.hpp"

class Motion {
public:
    Motion(int piece_id, Position source, Position destination);
    int get_piece_id() const;
    Position get_source() const;
    Position get_destination() const;
    float get_progress() const;
    void set_progress(float progress);
private:
    int piece_id;
    Position source;
    Position destination;
    float progress;
};