#include "ClientBoardSync.hpp"

#include "RemoteGameSession.hpp"
#include "core/GameEvents.hpp"
#include "model/GameConstants.hpp"

#include <algorithm>
#include <cmath>

namespace {

SnapshotPiece* find_piece_at(std::vector<SnapshotPiece>& pieces, const Position& cell) {
    for (SnapshotPiece& piece : pieces) {
        if (piece.cell == cell) {
            return &piece;
        }
    }
    return nullptr;
}

SnapshotPiece* find_piece_by_id(std::vector<SnapshotPiece>& pieces, int piece_id) {
    for (SnapshotPiece& piece : pieces) {
        if (piece.id == piece_id) {
            return &piece;
        }
    }
    return nullptr;
}

int move_distance(const Position& src, const Position& dest) {
    return std::max(
        std::abs(dest.getRow() - src.getRow()),
        std::abs(dest.getCol() - src.getCol())
    );
}

long long motion_duration_ms(
    const Position& source,
    const Position& destination,
    long long event_duration_ms,
    bool is_jump
) {
    if (event_duration_ms > 0) {
        return event_duration_ms;
    }
    if (is_jump) {
        return GameConstants::MS_PER_CELL;
    }
    return static_cast<long long>(move_distance(source, destination)) * GameConstants::MS_PER_CELL;
}

ActiveMotionInfo make_visual_motion(
    const SnapshotPiece& piece,
    const Position& source,
    const Position& destination,
    long long duration_ms
) {
    ActiveMotionInfo motion;
    motion.active = true;
    motion.source = source;
    motion.destination = destination;
    motion.start_time = 0;
    motion.duration = duration_ms;
    motion.piece_id = piece.id;
    motion.color = piece.color;
    motion.kind = piece.kind;
    return motion;
}

}  // namespace

ClientBoardSync::ClientBoardSync(RemoteGameSession& session) : session_(session) {
    wire_event_handlers();
}

bool ClientBoardSync::has_snapshot() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return has_snapshot_;
}

long long ClientBoardSync::extrapolated_time_at(
    std::chrono::steady_clock::time_point now
) const {
    const auto elapsed = now - snapshot_received_at_;
    const auto elapsed_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    return snapshot_base_time_ + elapsed_ms;
}

long long ClientBoardSync::extrapolated_time() const {
    return extrapolated_time_at(std::chrono::steady_clock::now());
}

void ClientBoardSync::sync_clock(long long server_time_ms) {
    snapshot_base_time_ = server_time_ms;
    snapshot_received_at_ = std::chrono::steady_clock::now();
}

void ClientBoardSync::start_visual_motion(
    int piece_id,
    const ActiveMotionInfo& motion,
    bool is_jump,
    std::chrono::steady_clock::time_point started_at
) {
    visual_motions_[piece_id] = VisualMotion{motion, started_at, is_jump};
}

void ClientBoardSync::prune_finished_visual_motions() const {
    const auto now = std::chrono::steady_clock::now();
    for (auto it = visual_motions_.begin(); it != visual_motions_.end(); ) {
        const long long elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - it->second.started_at
        ).count();
        if (elapsed_ms >= it->second.motion.duration) {
            it = visual_motions_.erase(it);
        } else {
            ++it;
        }
    }
}

void ClientBoardSync::apply_visual_motions_to_snapshot(GameSnapshot& snapshot) const {
    const auto now = std::chrono::steady_clock::now();

    for (auto& [piece_id, visual_motion] : visual_motions_) {
        const long long elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - visual_motion.started_at
        ).count();
        if (elapsed_ms >= visual_motion.motion.duration) {
            continue;
        }

        SnapshotPiece* piece = find_piece_by_id(snapshot.pieces, piece_id);
        if (piece == nullptr) {
            continue;
        }

        piece->state = State::Moving;
        piece->motion = visual_motion.motion;
        piece->motion_elapsed_ms = elapsed_ms;
        piece->is_jump_motion = visual_motion.is_jump;
    }
}

GameSnapshot ClientBoardSync::snapshot_with_selection(std::optional<Position> selected_cell) const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    GameSnapshot snapshot = snapshot_;
    snapshot.selected_cell = selected_cell;
    if (has_snapshot_) {
        snapshot.current_time = extrapolated_time();
    }

    prune_finished_visual_motions();
    apply_visual_motions_to_snapshot(snapshot);
    return snapshot;
}

EventBus& ClientBoardSync::event_bus() {
    return event_bus_;
}

void ClientBoardSync::wire_event_handlers() {
    session_.set_event_bus(&event_bus_);

    event_bus_.subscribe<SnapshotUpdatedEvent>([this](const SnapshotUpdatedEvent& event) {
        apply_snapshot(event.snapshot);
    });

    event_bus_.subscribe<MoveAcceptedEvent>([this](const MoveAcceptedEvent& event) {
        apply_move_accepted(event);
    });

    event_bus_.subscribe<JumpStartedEvent>([this](const JumpStartedEvent& event) {
        apply_jump_started(event);
    });

    event_bus_.subscribe<MoveResolvedEvent>([this](const MoveResolvedEvent& event) {
        std::lock_guard<std::mutex> lock(state_mutex_);
        if (!has_snapshot_) {
            return;
        }
        snapshot_.stats.record_move(
            event.move,
            snapshot_.board_height,
            snapshot_.board_width
        );
    });

    event_bus_.subscribe<GameOverEvent>([this](const GameOverEvent&) {
        std::lock_guard<std::mutex> lock(state_mutex_);
        snapshot_.game_over = true;
    });
}

void ClientBoardSync::apply_snapshot(GameSnapshot snapshot) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    const auto now = std::chrono::steady_clock::now();

    for (const SnapshotPiece& piece : snapshot.pieces) {
        if (!piece.motion.has_value() || visual_motions_.count(piece.id) > 0) {
            continue;
        }

        ActiveMotionInfo motion = piece.motion.value();
        motion.start_time = 0;
        start_visual_motion(piece.id, motion, piece.is_jump_motion, now);
    }

    sync_clock(snapshot.current_time);
    snapshot_ = std::move(snapshot);
    has_snapshot_ = true;
}

void ClientBoardSync::apply_move_accepted(const MoveAcceptedEvent& event) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (!has_snapshot_) {
        return;
    }

    SnapshotPiece* piece = find_piece_at(snapshot_.pieces, event.src);
    if (piece == nullptr) {
        return;
    }

    const auto started_at = std::chrono::steady_clock::now();
    const long long duration_ms = motion_duration_ms(
        event.src,
        event.dest,
        event.duration_ms,
        false
    );
    const ActiveMotionInfo motion = make_visual_motion(*piece, event.src, event.dest, duration_ms);
    start_visual_motion(piece->id, motion, false, started_at);
}

void ClientBoardSync::apply_jump_started(const JumpStartedEvent& event) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (!has_snapshot_) {
        return;
    }

    SnapshotPiece* piece = find_piece_at(snapshot_.pieces, event.cell);
    if (piece == nullptr) {
        return;
    }

    const auto started_at = std::chrono::steady_clock::now();
    const long long duration_ms = motion_duration_ms(
        event.cell,
        event.cell,
        event.duration_ms,
        true
    );
    const ActiveMotionInfo motion = make_visual_motion(*piece, event.cell, event.cell, duration_ms);
    start_visual_motion(piece->id, motion, true, started_at);
}
