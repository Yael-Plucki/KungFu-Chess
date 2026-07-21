#pragma once

#include "model/Piece.hpp"
#include <string>
#include <unordered_map>

struct AnimationConfig {
    std::string name;
    int frames_per_sec = 6;
    bool is_loop = true;
    std::string next_state = "idle";
    int frame_count = 5;
};

std::string piece_code(Color color, Kind kind);

class AnimationConfigRegistry {
public:
    explicit AnimationConfigRegistry(std::string assets_root);

    const AnimationConfig& config(const std::string& piece_code, const std::string& state) const;
    std::string sprite_path(
        const std::string& piece_code,
        const std::string& state,
        int frame_number
    ) const;

private:
    std::string assets_root;
    mutable std::unordered_map<std::string, AnimationConfig> cache;

    static std::string cache_key(const std::string& piece_code, const std::string& state);
    AnimationConfig load_config(const std::string& piece_code, const std::string& state) const;
};
