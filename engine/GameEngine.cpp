#include "GameEngine.hpp"

GameEngine::GameEngine()
    : arbiter(), game_over(false) {}

MoveResult GameEngine::request_move(const Position& src, const Position& dest) {
    if (game_over) {
        return {false, "game_over"};
    }

    MoveValidation validation = ruleEngine.validate_move(Board::getInstance(), src, dest);
    if (!validation.is_valid) {
        return {false, validation.reason};
    }

    if (!arbiter.start_motion(src, dest)) {
        return {false, "motion_start_failed"};
    }

    return {true, "ok"};
}

void GameEngine::jump(const Position& cell) {
    if (game_over) {
        return;
    }

    arbiter.start_jump(cell);
}

void GameEngine::wait(int ms) {
    ArrivalEvents events = arbiter.advance_time(ms);
    const Board& board = Board::getInstance();
    for (const MoveEvent& move : events.moves) {
        stats.record_move(move, board.getRows(), board.getCols());
    }
    if (events.arrived && events.king_captured) {
        game_over = true;
    }
}

bool GameEngine::is_game_over() const {
    return game_over;
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
