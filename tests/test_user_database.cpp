#include "../tests/test_framework.hpp"
#include "../storage/UserDatabase.hpp"
#include <filesystem>

static void test_register_and_authenticate_user() {
    const std::filesystem::path db_path = std::filesystem::temp_directory_path() / "kungfu_test_users.db";
    std::filesystem::remove(db_path);

    UserDatabase database(db_path.string());
    const AuthResult registered = database.register_user("alice", "secret");
    EXPECT_TRUE(registered.status == AuthStatus::Success);
    EXPECT_EQ(registered.user.rating, 1200);

    const AuthResult login = database.authenticate("alice", "secret");
    EXPECT_TRUE(login.status == AuthStatus::Success);
    EXPECT_EQ(login.user.username, std::string("alice"));
}

static void test_rejects_wrong_password() {
    const std::filesystem::path db_path = std::filesystem::temp_directory_path() / "kungfu_test_users_badpass.db";
    std::filesystem::remove(db_path);

    UserDatabase database(db_path.string());
    database.register_user("bob", "secret");

    const AuthResult login = database.authenticate("bob", "wrong");
    EXPECT_TRUE(login.status == AuthStatus::InvalidCredentials);
}

static void test_update_rating_persists() {
    const std::filesystem::path db_path = std::filesystem::temp_directory_path() / "kungfu_test_users_rating.db";
    std::filesystem::remove(db_path);

    UserDatabase database(db_path.string());
    database.register_user("carol", "secret");
    EXPECT_TRUE(database.update_rating("carol", 1250));

    const std::optional<UserRecord> user = database.get_user("carol");
    EXPECT_TRUE(user.has_value());
    EXPECT_EQ(user->rating, 1250);
}

int main() {
    RUN_TEST(test_register_and_authenticate_user);
    RUN_TEST(test_rejects_wrong_password);
    RUN_TEST(test_update_rating_persists);
    return 0;
}
