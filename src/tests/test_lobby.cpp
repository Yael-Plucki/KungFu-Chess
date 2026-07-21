#include "test_framework.hpp"
#include "network/Lobby.hpp"

static void test_first_login_is_white_second_is_black() {
    Lobby lobby;

    const Lobby::JoinResult white = lobby.try_login("conn-1", "Alice", 1300);
    EXPECT_TRUE(white == Lobby::JoinResult::White);
    EXPECT_EQ(lobby.players_joined(), 1);
    EXPECT_FALSE(lobby.is_ready());
    EXPECT_EQ(lobby.white_player()->rating, 1300);

    const Lobby::JoinResult black = lobby.try_login("conn-2", "Bob", 1100);
    EXPECT_TRUE(black == Lobby::JoinResult::Black);
    EXPECT_EQ(lobby.players_joined(), 2);
    EXPECT_TRUE(lobby.is_ready());
    EXPECT_EQ(lobby.white_player()->username, std::string("Alice"));
    EXPECT_EQ(lobby.black_player()->username, std::string("Bob"));
}

static void test_third_login_is_rejected_when_full() {
    Lobby lobby;
    lobby.try_login("conn-1", "Alice", 1200);
    lobby.try_login("conn-2", "Bob", 1200);

    const Lobby::JoinResult third = lobby.try_login("conn-3", "Carol", 1200);
    EXPECT_TRUE(third == Lobby::JoinResult::Full);
    EXPECT_EQ(lobby.players_joined(), 2);
}

static void test_game_starts_only_after_both_players_join() {
    Lobby lobby;
    lobby.try_login("conn-1", "Alice", 1200);
    EXPECT_FALSE(lobby.is_started());

    lobby.try_login("conn-2", "Bob", 1200);
    EXPECT_FALSE(lobby.is_started());

    lobby.start_game();
    EXPECT_TRUE(lobby.is_started());
}

int main() {
    RUN_TEST(test_first_login_is_white_second_is_black);
    RUN_TEST(test_third_login_is_rejected_when_full);
    RUN_TEST(test_game_starts_only_after_both_players_join);
    return 0;
}
