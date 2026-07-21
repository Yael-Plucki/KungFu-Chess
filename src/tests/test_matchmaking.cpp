#include "test_framework.hpp"
#include "network/MatchmakingQueue.hpp"

static void test_match_within_rating_range() {
    MatchmakingQueue queue;
    queue.add_seeker(Seeker{"a", "alice", 1200});
    queue.add_seeker(Seeker{"b", "bob", 1250});

    const std::optional<std::pair<Seeker, Seeker>> matched = queue.try_match();
    EXPECT_TRUE(matched.has_value());
    EXPECT_EQ(matched->first.username, std::string("alice"));
    EXPECT_EQ(matched->second.username, std::string("bob"));
    EXPECT_FALSE(queue.is_seeking("a"));
    EXPECT_FALSE(queue.is_seeking("b"));
}

static void test_no_match_outside_rating_range() {
    MatchmakingQueue queue;
    queue.add_seeker(Seeker{"a", "alice", 1200});
    queue.add_seeker(Seeker{"b", "bob", 1400});

    const std::optional<std::pair<Seeker, Seeker>> matched = queue.try_match();
    EXPECT_FALSE(matched.has_value());
    EXPECT_TRUE(queue.is_seeking("a"));
    EXPECT_TRUE(queue.is_seeking("b"));
}

static void test_remove_seeker() {
    MatchmakingQueue queue;
    queue.add_seeker(Seeker{"a", "alice", 1200});
    queue.remove_seeker("a");
    EXPECT_FALSE(queue.is_seeking("a"));
}

int main() {
    RUN_TEST(test_match_within_rating_range);
    RUN_TEST(test_no_match_outside_rating_range);
    RUN_TEST(test_remove_seeker);
    return 0;
}
