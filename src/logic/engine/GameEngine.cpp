#include "GameEngine.hpp"
#include "../core/GameEvents.hpp"

GameEngine::GameEngine()
    : arbiter(), game_over(false) {}

EventBus& GameEngine::event_bus() {
    return eventBus;
}

MoveResult GameEngine::request_move(const Position& src, const Position& dest) {
    if (game_over) {
        MoveRejectedEvent rejected{src, dest, "game_over"};
        eventBus.publish(rejected);
        return {false, rejected.reason};
    }

    MoveValidation validation = ruleEngine.validate_move(Board::getInstance(), src, dest);
    if (!validation.is_valid) {
        MoveRejectedEvent rejected{src, dest, validation.reason};
        eventBus.publish(rejected);
        return {false, rejected.reason};
    }

    if (!arbiter.start_motion(src, dest)) {
        MoveRejectedEvent rejected{src, dest, "motion_start_failed"};
        eventBus.publish(rejected);
        return {false, rejected.reason};
    }

    eventBus.publish(MoveAcceptedEvent{src, dest});
    return {true, "ok"};
}

void GameEngine::jump(const Position& cell) {
    if (game_over) {
        return;
    }

    arbiter.start_jump(cell);
    eventBus.publish(JumpStartedEvent{cell});
}

void GameEngine::wait(int ms) {
    ArrivalEvents events = arbiter.advance_time(ms);
    const Board& board = Board::getInstance();
    for (const MoveEvent& move : events.moves) {
        stats.record_move(move, board.getRows(), board.getCols());
        eventBus.publish(MoveResolvedEvent{move});
    }
    if (events.arrived && events.king_captured) {
        game_over = true;
        eventBus.publish(GameOverEvent{});
    }
    eventBus.publish(TimeAdvancedEvent{ms, arbiter.get_current_time()});
}

bool GameEngine::is_game_over() const {
    return game_over;
}

void GameEngine::force_game_over(bool publish_event) {
    if (game_over) {
        return;
    }
    game_over = true;
    if (publish_event) {
        eventBus.publish(GameOverEvent{});
    }
}

GameSnapshot GameEngine::snapshot(std::optional<Position> selected_cell) const {
    GameSnapshot snap = GameSnapshot::create(
        Board::getInstance(),
        game_over,
        selected_cell,
        arbiter.active_motion_infos(),
        arbiter.get_current_time()
    );
    snap.stats = stats;
    return snap;
}

long long GameEngine::current_time_ms() const {
    return arbiter.get_current_time();
}
