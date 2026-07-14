#include "AnimationConfig.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {
std::string read_file(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Cannot open animation config: " + path.string());
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

std::string read_json_string(const std::string& json, const std::string& key) {
    const std::string quoted_key = "\"" + key + "\": \"";
    const std::size_t start = json.find(quoted_key);
    if (start == std::string::npos) {
        return "";
    }

    const std::size_t value_start = start + quoted_key.size();
    const std::size_t value_end = json.find('"', value_start);
    if (value_end == std::string::npos) {
        return "";
    }

    return json.substr(value_start, value_end - value_start);
}

int read_json_int(const std::string& json, const std::string& key, int fallback) {
    const std::string numeric_key = "\"" + key + "\": ";
    const std::size_t start = json.find(numeric_key);
    if (start == std::string::npos) {
        return fallback;
    }

    const std::size_t value_start = start + numeric_key.size();
    return std::stoi(json.substr(value_start));
}

bool read_json_bool(const std::string& json, const std::string& key, bool fallback) {
    const std::string bool_key = "\"" + key + "\": ";
    const std::size_t start = json.find(bool_key);
    if (start == std::string::npos) {
        return fallback;
    }

    const std::size_t value_start = start + bool_key.size();
    return json.compare(value_start, 4, "true") == 0;
}

int count_sprite_frames(const std::filesystem::path& sprites_dir) {
    if (!std::filesystem::exists(sprites_dir)) {
        return 1;
    }

    int count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(sprites_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") {
            ++count;
        }
    }
    return count > 0 ? count : 1;
}
}

std::string piece_code(Color color, Kind kind) {
    const char team = (color == Color::White) ? 'W' : 'B';
    char piece = '?';
    switch (kind) {
        case Kind::King: piece = 'K'; break;
        case Kind::Queen: piece = 'Q'; break;
        case Kind::Rook: piece = 'R'; break;
        case Kind::Bishop: piece = 'B'; break;
        case Kind::Knight: piece = 'N'; break;
        case Kind::Pawn: piece = 'P'; break;
        default: break;
    }
    return std::string{piece, team};
}

AnimationConfigRegistry::AnimationConfigRegistry(std::string assets_root)
    : assets_root(std::move(assets_root)) {}

std::string AnimationConfigRegistry::cache_key(
    const std::string& piece_code_value,
    const std::string& state
) {
    return piece_code_value + ":" + state;
}

AnimationConfig AnimationConfigRegistry::load_config(
    const std::string& piece_code_value,
    const std::string& state
) const {
    const std::filesystem::path state_dir =
        std::filesystem::path(assets_root) / piece_code_value / "states" / state;
    const std::filesystem::path config_path = state_dir / "config.json";
    const std::string json = read_file(config_path);

    AnimationConfig config;
    config.name = state;
    config.frames_per_sec = read_json_int(json, "frames_per_sec", 6);
    config.is_loop = read_json_bool(json, "is_loop", true);
    config.next_state = read_json_string(json, "next_state_when_finished");
    if (config.next_state.empty()) {
        config.next_state = "idle";
    }
    config.frame_count = count_sprite_frames(state_dir / "sprites");
    return config;
}

const AnimationConfig& AnimationConfigRegistry::config(
    const std::string& piece_code_value,
    const std::string& state
) const {
    const std::string key = cache_key(piece_code_value, state);
    const auto existing = cache.find(key);
    if (existing != cache.end()) {
        return existing->second;
    }

    const AnimationConfig loaded = load_config(piece_code_value, state);
    const auto inserted = cache.emplace(key, loaded);
    return inserted.first->second;
}

std::string AnimationConfigRegistry::sprite_path(
    const std::string& piece_code_value,
    const std::string& state,
    int frame_number
) const {
    return (std::filesystem::path(assets_root) / piece_code_value / "states" / state / "sprites" /
            (std::to_string(frame_number) + ".png"))
        .string();
}
