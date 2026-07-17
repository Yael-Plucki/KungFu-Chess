#include "RealTimeArbiter.hpp"
#include <algorithm>
#include <cmath>

RealTimeArbiter::RealTimeArbiter() = default;

bool RealTimeArbiter::has_motion_from(const Position& src) const {
    for (const Motion& motion : active_motions) {
        if (motion.getSource() == src) {
            return true;
        }
    }
    return false;
}

bool RealTimeArbiter::start_motion(Position src, Position dest) {
    if (!Board::getInstance().isValidPosition(src) || !Board::getInstance().isValidPosition(dest) || has_motion_from(src)) {
        return false;
    }

    Piece piece = Board::getInstance().at(src);
    if (piece.getKind() == Kind::Empty || piece.getState() == State::Moving) {
        return false;
    }

    piece.setState(State::Moving);
    int distance = std::max(
        std::abs(dest.getRow() - src.getRow()),
        std::abs(dest.getCol() - src.getCol())
    );
    long long duration = static_cast<long long>(distance) * GameConstants::MS_PER_CELL;
    active_motions.emplace_back(src, dest, current_time, duration, next_motion_sequence++, piece);
    Board::getInstance().remove_piece(src);
    return true;
}

bool RealTimeArbiter::start_jump(const Position& cell) {
    if (!Board::getInstance().isValidPosition(cell) || has_motion_from(cell)) {
        return false;
    }

    Piece piece = Board::getInstance().at(cell);
    if (piece.getKind() == Kind::Empty || piece.getState() == State::Moving) {
        return false;
    }

    piece.setState(State::Moving);
    Board::getInstance().update_piece(cell, piece);
    active_motions.emplace_back(cell, cell, current_time, GameConstants::MS_PER_CELL, next_motion_sequence++, piece);
    return true;
}

bool RealTimeArbiter::resolve_arrival(const Motion& motion) {
    Position src = motion.getSource();
    Position dest = motion.getDestination();

    if (src == dest) {
        Piece piece = Board::getInstance().at(src);
        if (piece.getKind() != Kind::Empty && piece.getState() == State::Moving) {
            piece.setState(State::Idle);
            Board::getInstance().update_piece(src, piece);
        }
        return false;
    }

    Piece moving = motion.getPiece();
    Piece target = Board::getInstance().at(dest);

    // Only in-place jumps keep the defender on the destination while Moving.
    if (target.getKind() != Kind::Empty &&
        target.getState() == State::Moving &&
        target.getColor() != moving.getColor()) {
        return moving.getKind() == Kind::King;
    }

    bool king_captured = (target.getKind() == Kind::King);

    Piece arrived = moving;
    arrived.setPosition(dest);
    arrived.setState(State::Idle);

    if (arrived.getKind() == Kind::Pawn) {
        int promotion_row = (arrived.getColor() == Color::White) ? 0 : Board::getInstance().getRows() - 1;
        if (dest.getRow() == promotion_row) {
            arrived.setKind(Kind::Queen);
        }
    }

    if (target.getKind() != Kind::Empty) {
        Board::getInstance().remove_piece(dest);
    }
    Board::getInstance().add_piece(arrived);

    return king_captured;
}

ArrivalEvents RealTimeArbiter::advance_time(int ms) {
    current_time += ms;
    ArrivalEvents events;

    std::vector<Motion> finished;
    finished.reserve(active_motions.size());
    for (auto it = active_motions.begin(); it != active_motions.end(); ) {
        if (it->isFinished(current_time)) {
            finished.push_back(*it);
            it = active_motions.erase(it);
        } else {
            ++it;
        }
    }

    std::sort(finished.begin(), finished.end(), [](const Motion& a, const Motion& b) {
        if (a.getStartTime() != b.getStartTime()) {
            return a.getStartTime() < b.getStartTime();
        }
        return a.getSequence() < b.getSequence();
    });

    auto resolve_finished = [&](bool in_place) {
        for (const Motion& motion : finished) {
            bool motion_in_place = motion.getSource() == motion.getDestination();
            if (motion_in_place != in_place) {
                continue;
            }

            events.arrived = true;
            if (resolve_arrival(motion)) {
                events.king_captured = true;
            }
        }
    };

    resolve_finished(false);
    resolve_finished(true);

    return events;
}

bool RealTimeArbiter::has_active_motion() const {
    return !active_motions.empty();
}

long long RealTimeArbiter::get_current_time() const {
    return current_time;
}

std::vector<ActiveMotionInfo> RealTimeArbiter::active_motion_infos() const {
    std::vector<ActiveMotionInfo> infos;
    infos.reserve(active_motions.size());

    for (const Motion& motion : active_motions) {
        const Piece& piece = motion.getPiece();
        infos.push_back(ActiveMotionInfo{
            true,
            motion.getSource(),
            motion.getDestination(),
            motion.getStartTime(),
            motion.getDuration(),
            piece.getId(),
            piece.getColor(),
            piece.getKind()
        });
    }

    return infos;
}
