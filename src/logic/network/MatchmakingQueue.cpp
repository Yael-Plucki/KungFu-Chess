#include "MatchmakingQueue.hpp"

#include <algorithm>

void MatchmakingQueue::add_seeker(Seeker seeker) {
    remove_seeker(seeker.connection_id);
    seekers_.push_back(std::move(seeker));
}

void MatchmakingQueue::remove_seeker(const std::string& connection_id) {
    seekers_.erase(
        std::remove_if(
            seekers_.begin(),
            seekers_.end(),
            [&connection_id](const Seeker& seeker) {
                return seeker.connection_id == connection_id;
            }),
        seekers_.end());
}

bool MatchmakingQueue::is_seeking(const std::string& connection_id) const {
    return std::any_of(
        seekers_.begin(),
        seekers_.end(),
        [&connection_id](const Seeker& seeker) {
            return seeker.connection_id == connection_id;
        });
}

std::optional<std::pair<Seeker, Seeker>> MatchmakingQueue::try_match() {
    for (auto i = seekers_.begin(); i != seekers_.end(); ++i) {
        for (auto j = std::next(i); j != seekers_.end(); ++j) {
            if (!ratings_compatible(i->rating, j->rating)) {
                continue;
            }

            const std::pair<Seeker, Seeker> matched = {*i, *j};
            seekers_.erase(j);
            seekers_.erase(i);
            return matched;
        }
    }
    return std::nullopt;
}

bool MatchmakingQueue::ratings_compatible(int rating_a, int rating_b) {
    const int diff = rating_a > rating_b ? rating_a - rating_b : rating_b - rating_a;
    return diff <= kRatingRange;
}
