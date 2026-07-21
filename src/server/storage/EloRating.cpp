#include "EloRating.hpp"

#include <cmath>

namespace EloRating {

double expected_score(int player_rating, int opponent_rating) {
    return 1.0 / (1.0 + std::pow(10.0, (opponent_rating - player_rating) / 400.0));
}

RatingChange apply_match_result(int white_rating, int black_rating, double white_score) {
    const double white_expected = expected_score(white_rating, black_rating);
    const double black_expected = expected_score(black_rating, white_rating);
    const double black_score = 1.0 - white_score;

    RatingChange change;
    change.white_rating = white_rating;
    change.black_rating = black_rating;
    change.white_delta = static_cast<int>(std::lround(kKFactor * (white_score - white_expected)));
    change.black_delta = static_cast<int>(std::lround(kKFactor * (black_score - black_expected)));
    change.white_rating += change.white_delta;
    change.black_rating += change.black_delta;
    return change;
}

}  // namespace EloRating
