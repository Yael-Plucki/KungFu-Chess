#include "../tests/test_framework.hpp"
#include "../storage/EloRating.hpp"

static void test_equal_ratings_change_by_same_magnitude_opposite_sign() {
    const EloRating::RatingChange change = EloRating::apply_match_result(1200, 1200, 1.0);
    EXPECT_EQ(change.white_rating, 1216);
    EXPECT_EQ(change.black_rating, 1184);
    EXPECT_EQ(change.white_delta, 16);
    EXPECT_EQ(change.black_delta, -16);
}

static void test_underdog_gain_more_points_for_upset() {
    const EloRating::RatingChange change = EloRating::apply_match_result(1000, 1400, 1.0);
    EXPECT_TRUE(change.white_delta > 16);
    EXPECT_TRUE(change.black_delta < -16);
}

static void test_draw_keeps_ratings_close_for_equal_players() {
    const EloRating::RatingChange change = EloRating::apply_match_result(1200, 1200, 0.5);
    EXPECT_EQ(change.white_delta, 0);
    EXPECT_EQ(change.black_delta, 0);
    EXPECT_EQ(change.white_rating, 1200);
    EXPECT_EQ(change.black_rating, 1200);
}

int main() {
    RUN_TEST(test_equal_ratings_change_by_same_magnitude_opposite_sign);
    RUN_TEST(test_underdog_gain_more_points_for_upset);
    RUN_TEST(test_draw_keeps_ratings_close_for_equal_players);
    return 0;
}
