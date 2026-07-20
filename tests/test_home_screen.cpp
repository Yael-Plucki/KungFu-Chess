#include "../tests/test_framework.hpp"
#include "../shell/HomeScreen.hpp"
#include <sstream>
#include <string>

static void test_login_accepts_username_and_password() {
    std::istringstream in("1\nalice\nsecret\n");
    std::ostringstream out;

    const LoginResult result = HomeScreen::prompt_auth(in, out);

    EXPECT_TRUE(result.success);
    EXPECT_TRUE(result.mode == AuthMode::Login);
    EXPECT_EQ(result.username, std::string("alice"));
    EXPECT_EQ(result.password, std::string("secret"));
    EXPECT_TRUE(out.str().find("Welcome, alice!") != std::string::npos);
}

static void test_register_mode_is_selected() {
    std::istringstream in("2\nbob\nsecret\n");
    std::ostringstream out;

    const LoginResult result = HomeScreen::prompt_auth(in, out);

    EXPECT_TRUE(result.success);
    EXPECT_TRUE(result.mode == AuthMode::Register);
    EXPECT_EQ(result.username, std::string("bob"));
}

static void test_login_rejects_empty_username() {
    std::istringstream in("1\n\n   \nbob\npass\n");
    std::ostringstream out;

    const LoginResult result = HomeScreen::prompt_auth(in, out);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.username, std::string("bob"));
    EXPECT_TRUE(out.str().find("Please enter a username.") != std::string::npos);
}

static void test_menu_play_and_exit() {
    std::istringstream play_in("1\n");
    std::ostringstream play_out;
    EXPECT_TRUE(HomeScreen::show_menu(play_in, play_out, "alice") == HomeMenuChoice::Play);

    std::istringstream exit_in("2\n");
    std::ostringstream exit_out;
    EXPECT_TRUE(HomeScreen::show_menu(exit_in, exit_out, "alice") == HomeMenuChoice::Exit);
    EXPECT_TRUE(exit_out.str().find("Goodbye, alice!") != std::string::npos);
}

int main() {
    RUN_TEST(test_login_accepts_username_and_password);
    RUN_TEST(test_register_mode_is_selected);
    RUN_TEST(test_login_rejects_empty_username);
    RUN_TEST(test_menu_play_and_exit);
    return 0;
}
