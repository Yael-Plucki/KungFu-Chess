#pragma once

#include "model/GameSnapshot.hpp"
#include "AnimationConfig.hpp"
#include <string>

class PieceAnimator {
public:
    PieceAnimator(int piece_id, std::string piece_code_value);

    void sync(const SnapshotPiece& piece);
    void update(int delta_ms, const AnimationConfigRegistry& configs);
    std::string current_state() const { return current_state_name; }
    std::string sprite_path(const AnimationConfigRegistry& configs) const;

private:
    std::string piece_code_value;
    std::string current_state_name;
    long long local_time_ms = 0;
    State last_game_state = State::Idle;

    void enter_state(const std::string& state);
    int current_frame(const AnimationConfig& config) const;
    bool is_clip_finished(const AnimationConfig& config) const;
};
