#include "PieceAnimator.hpp"
#include <algorithm>

PieceAnimator::PieceAnimator(int /*piece_id*/, std::string piece_code_value)
    : piece_code_value(std::move(piece_code_value)),
      current_state_name("idle") {}

void PieceAnimator::enter_state(const std::string& state) {
    if (current_state_name == state) {
        return;
    }

    current_state_name = state;
    local_time_ms = 0;
}

void PieceAnimator::sync(const SnapshotPiece& piece) {
    if (piece.state == State::Moving) {
        const bool motion_anim =
            current_state_name == "idle" ||
            current_state_name == "long_rest" ||
            current_state_name == "short_rest";
        if (last_game_state != State::Moving || motion_anim) {
            enter_state(piece.is_jump_motion ? "jump" : "move");
        }
    } else if (piece.state == State::Idle) {
        if (current_state_name == "move") {
            enter_state("long_rest");
        } else if (current_state_name == "jump") {
            enter_state("short_rest");
        }
    }

    last_game_state = piece.state;
}

int PieceAnimator::current_frame(const AnimationConfig& config) const {
    if (config.frames_per_sec <= 0 || config.frame_count <= 0) {
        return 1;
    }

    const long long frame = (local_time_ms * config.frames_per_sec) / 1000;
    if (config.is_loop) {
        return static_cast<int>(frame % config.frame_count) + 1;
    }

    return static_cast<int>(std::min<long long>(frame, config.frame_count - 1)) + 1;
}

bool PieceAnimator::is_clip_finished(const AnimationConfig& config) const {
    const bool one_shot_rest =
        current_state_name == "long_rest" || current_state_name == "short_rest";

    if ((config.is_loop && !one_shot_rest) || config.frames_per_sec <= 0 || config.frame_count <= 0) {
        return false;
    }

    const long long clip_duration_ms =
        (config.frame_count * 1000LL) / config.frames_per_sec;
    return local_time_ms >= clip_duration_ms;
}

void PieceAnimator::update(int delta_ms, const AnimationConfigRegistry& configs) {
    local_time_ms += delta_ms;

    const AnimationConfig& config = configs.config(piece_code_value, current_state_name);
    if (is_clip_finished(config)) {
        enter_state(config.next_state);
    }
}

std::string PieceAnimator::sprite_path(const AnimationConfigRegistry& configs) const {
    const AnimationConfig& config = configs.config(piece_code_value, current_state_name);
    return configs.sprite_path(piece_code_value, current_state_name, current_frame(config));
}
