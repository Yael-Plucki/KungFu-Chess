#include "AnimatorRegistry.hpp"

AnimatorRegistry::AnimatorRegistry(std::string assets_root)
    : configs(std::move(assets_root)) {}

PieceAnimator& AnimatorRegistry::get_or_create(const SnapshotPiece& piece) {
    const auto existing = animators.find(piece.id);
    if (existing != animators.end()) {
        return existing->second;
    }

    const auto inserted = animators.emplace(
        piece.id,
        PieceAnimator(piece.id, piece_code(piece.color, piece.kind))
    );
    return inserted.first->second;
}

void AnimatorRegistry::sync(const GameSnapshot& snapshot) {
    std::unordered_map<int, bool> seen;
    seen.reserve(snapshot.pieces.size());

    for (const SnapshotPiece& piece : snapshot.pieces) {
        seen[piece.id] = true;
        PieceAnimator& animator = get_or_create(piece);
        animator.sync(piece);
    }

    for (auto it = animators.begin(); it != animators.end(); ) {
        if (seen.count(it->first) == 0) {
            it = animators.erase(it);
        } else {
            ++it;
        }
    }
}

void AnimatorRegistry::update(int delta_ms) {
    for (auto& entry : animators) {
        entry.second.update(delta_ms, configs);
    }
}

std::string AnimatorRegistry::sprite_path_for(const SnapshotPiece& piece) const {
    const auto found = animators.find(piece.id);
    if (found == animators.end()) {
        return configs.sprite_path(piece_code(piece.color, piece.kind), "idle", 1);
    }
    return found->second.sprite_path(configs);
}
