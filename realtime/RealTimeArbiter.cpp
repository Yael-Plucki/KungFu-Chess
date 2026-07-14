#include "RealTimeArbiter.hpp"
#include <algorithm>
#include <cmath>

RealTimeArbiter::RealTimeArbiter(std::shared_ptr<Board> board_ref)
    : board(std::move(board_ref)) {}

bool RealTimeArbiter::has_motion_from(const Position& src) const {
    for (const Motion& motion : active_motions) {
        if (motion.getSource() == src) {
            return true;
        }
    }
    return false;
}

bool RealTimeArbiter::start_motion(Position src, Position dest) {
    if (!board->isValidPosition(src) || !board->isValidPosition(dest) || has_motion_from(src)) {
        return false;
    }

    Piece piece = board->at(src);
    if (piece.getKind() == Kind::Empty || piece.getState() == State::Moving) {
        return false;
    }

    piece.setState(State::Moving);
    board->update_piece(src, piece);

    int distance = std::max(
        std::abs(dest.getRow() - src.getRow()),
        std::abs(dest.getCol() - src.getCol())
    );
    long long duration = static_cast<long long>(distance) * GameConstants::MS_PER_CELL;
    active_motions.emplace_back(src, dest, current_time, duration, next_motion_sequence++);
    return true;
}

bool RealTimeArbiter::start_jump(const Position& cell) {
    if (!board->isValidPosition(cell) || has_motion_from(cell)) {
        return false;
    }

    Piece piece = board->at(cell);
    if (piece.getKind() == Kind::Empty || piece.getState() == State::Moving) {
        return false;
    }

    piece.setState(State::Moving);
    board->update_piece(cell, piece);
    active_motions.emplace_back(cell, cell, current_time, GameConstants::MS_PER_CELL, next_motion_sequence++);
    return true;
}

const Motion* RealTimeArbiter::find_opposing_motion(
    const Position& from,
    const Position& to,
    const std::vector<Motion>& motions
) {
    for (const Motion& motion : motions) {
        if (motion.getSource() == from &&
            motion.getDestination() == to &&
            from != to) {
            return &motion;
        }
    }
    return nullptr;
}

bool RealTimeArbiter::resolve_arrival(
    const Motion& motion,
    const std::vector<Motion>& finished,
    std::set<Position>& cancelled_sources
) {
    Position src = motion.getSource();
    Position dest = motion.getDestination();

    if (src == dest) {
        Piece piece = board->at(src);
        if (piece.getKind() != Kind::Empty && piece.getState() == State::Moving) {
            piece.setState(State::Idle);
            board->update_piece(src, piece);
        }
        return false;
    }

    Piece moving = board->at(src);
    if (moving.getKind() == Kind::Empty) {
        return false;
    }

    Piece target = board->at(dest);

    if (target.getKind() != Kind::Empty &&
        target.getState() == State::Moving &&
        target.getColor() != moving.getColor()) {
        const Motion* opposing = find_opposing_motion(dest, src, finished);

        if (opposing != nullptr) {
            if (motion.getStartTime() > opposing->getStartTime() ||
                (motion.getStartTime() == opposing->getStartTime() &&
                 motion.getSequence() > opposing->getSequence())) {
                board->remove_piece(src);
                return false;
            }

            cancelled_sources.insert(opposing->getSource());
        } else {
            Piece defender = target;
            defender.setState(State::Idle);
            board->update_piece(dest, defender);
            board->remove_piece(src);
            return false;
        }
    }

    bool king_captured = (target.getKind() == Kind::King);

    board->remove_piece(src);

    Piece arrived = moving;
    arrived.setPosition(dest);
    arrived.setState(State::Idle);

    if (arrived.getKind() == Kind::Pawn) {
        int promotion_row = (arrived.getColor() == Color::White) ? 0 : board->getRows() - 1;
        if (dest.getRow() == promotion_row) {
            arrived.setKind(Kind::Queen);
        }
    }

    if (target.getKind() != Kind::Empty) {
        board->remove_piece(dest);
    }
    board->add_piece(arrived);

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

    std::set<Position> cancelled_sources;

    auto resolve_finished = [&](bool in_place) {
        for (const Motion& motion : finished) {
            if (cancelled_sources.count(motion.getSource()) > 0) {
                continue;
            }

            bool motion_in_place = motion.getSource() == motion.getDestination();
            if (motion_in_place != in_place) {
                continue;
            }

            events.arrived = true;
            if (resolve_arrival(motion, finished, cancelled_sources)) {
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
        infos.push_back(ActiveMotionInfo{
            true,
            motion.getSource(),
            motion.getDestination(),
            motion.getStartTime(),
            motion.getDuration()
        });
    }

    return infos;
}
