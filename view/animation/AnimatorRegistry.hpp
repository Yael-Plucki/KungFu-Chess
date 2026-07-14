#pragma once

#include "../../model/GameSnapshot.hpp"
#include "AnimationConfig.hpp"
#include "PieceAnimator.hpp"
#include <unordered_map>

class AnimatorRegistry {
public:
    explicit AnimatorRegistry(std::string assets_root);

    void sync(const GameSnapshot& snapshot);
    void update(int delta_ms);
    std::string sprite_path_for(const SnapshotPiece& piece) const;

private:
    AnimationConfigRegistry configs;
    std::unordered_map<int, PieceAnimator> animators;

    PieceAnimator& get_or_create(const SnapshotPiece& piece);
};
