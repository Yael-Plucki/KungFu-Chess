#pragma once

#include <optional>
#include <string>

struct UserRecord {
    std::string username;
    int rating = 1200;
};

enum class AuthStatus {
    Success,
    InvalidCredentials,
    UserAlreadyExists,
    UserNotFound,
    InvalidInput
};

struct AuthResult {
    AuthStatus status = AuthStatus::InvalidInput;
    UserRecord user;
    std::string message;
};

class UserDatabase {
public:
    explicit UserDatabase(std::string db_path = "kungfu.db");

    AuthResult register_user(const std::string& username, const std::string& password);
    AuthResult authenticate(const std::string& username, const std::string& password);
    std::optional<UserRecord> get_user(const std::string& username) const;
    bool update_rating(const std::string& username, int rating);

private:
    std::string db_path_;

    void initialize_schema() const;
};
