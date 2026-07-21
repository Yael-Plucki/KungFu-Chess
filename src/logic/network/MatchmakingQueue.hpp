#pragma once

#include <optional>
#include <string>
#include <vector>

struct Seeker {
    std::string connection_id;
    std::string username;
    int rating = 1200;
};

class MatchmakingQueue {
public:
    static constexpr int kRatingRange = 100;

    void add_seeker(Seeker seeker);
    void remove_seeker(const std::string& connection_id);
    bool is_seeking(const std::string& connection_id) const;

    // Returns matched pair and removes both from the queue.
    std::optional<std::pair<Seeker, Seeker>> try_match();

private:
    static bool ratings_compatible(int rating_a, int rating_b);
    std::vector<Seeker> seekers_;
};
