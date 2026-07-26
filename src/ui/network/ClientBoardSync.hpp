#pragma once

#include "core/EventBus.hpp"
#include "core/GameEvents.hpp"
#include "model/GameSnapshot.hpp"
#include "model/Position.hpp"
#include <chrono>
#include <mutex>
#include <optional>
#include <unordered_map>

class RemoteGameSession;

// Mirrors server-side NetworkBridge: WebSocket events arrive via RemoteGameSession's
// EventBus and this class keeps the local board view synchronized for rendering.
class ClientBoardSync {
public:
    explicit ClientBoardSync(RemoteGameSession& session);

    bool has_snapshot() const;
    GameSnapshot snapshot_with_selection(std::optional<Position> selected_cell) const;
    EventBus& event_bus();

private:
    struct VisualMotion {
        ActiveMotionInfo motion;
        std::chrono::steady_clock::time_point started_at;
        bool is_jump = false;
    };

    RemoteGameSession& session_;
    EventBus event_bus_;
    GameSnapshot snapshot_;
    bool has_snapshot_ = false;
    mutable std::mutex state_mutex_;
    mutable std::unordered_map<int, VisualMotion> visual_motions_;

    void wire_event_handlers();
    void apply_snapshot(GameSnapshot snapshot);
    void apply_move_accepted(const MoveAcceptedEvent& event);
    void apply_jump_started(const JumpStartedEvent& event);
    void sync_clock(long long server_time_ms);
    long long extrapolated_time() const;
    long long extrapolated_time_at(std::chrono::steady_clock::time_point now) const;

    void start_visual_motion(
        int piece_id,
        const ActiveMotionInfo& motion,
        bool is_jump,
        std::chrono::steady_clock::time_point started_at
    );
    void apply_visual_motions_to_snapshot(GameSnapshot& snapshot) const;
    void prune_finished_visual_motions() const;

    std::chrono::steady_clock::time_point snapshot_received_at_{};
    long long snapshot_base_time_ = 0;
};
