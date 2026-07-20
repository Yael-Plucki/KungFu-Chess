#pragma once

namespace EloRating {

constexpr int kStartingRating = 1200;
constexpr int kKFactor = 32;

struct RatingChange {
    int white_rating = kStartingRating;
    int black_rating = kStartingRating;
    int white_delta = 0;
    int black_delta = 0;
};

double expected_score(int player_rating, int opponent_rating);
RatingChange apply_match_result(int white_rating, int black_rating, double white_score);

}  // namespace EloRating
