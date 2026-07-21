#include "UserDatabase.hpp"

#include "EloRating.hpp"
#include "PasswordHasher.hpp"
#include <sqlite3.h>
#include <stdexcept>

namespace {

AuthResult failure(AuthStatus status, std::string message) {
    return AuthResult{status, {}, std::move(message)};
}

AuthResult success(UserRecord user) {
    return AuthResult{AuthStatus::Success, std::move(user), "ok"};
}

}  // namespace

UserDatabase::UserDatabase(std::string db_path) : db_path_(std::move(db_path)) {
    initialize_schema();
}

void UserDatabase::initialize_schema() const {
    sqlite3* db = nullptr;
    if (sqlite3_open(db_path_.c_str(), &db) != SQLITE_OK) {
        throw std::runtime_error("Failed to open user database");
    }

    const char* sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "username TEXT PRIMARY KEY,"
        "password_hash TEXT NOT NULL,"
        "salt TEXT NOT NULL,"
        "rating INTEGER NOT NULL DEFAULT 1200"
        ");";

    char* error_message = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &error_message) != SQLITE_OK) {
        std::string error = error_message != nullptr ? error_message : "schema error";
        sqlite3_free(error_message);
        sqlite3_close(db);
        throw std::runtime_error(error);
    }

    sqlite3_close(db);
}

AuthResult UserDatabase::register_user(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        return failure(AuthStatus::InvalidInput, "Username and password are required.");
    }

    if (get_user(username).has_value()) {
        return failure(AuthStatus::UserAlreadyExists, "Username already exists.");
    }

    sqlite3* db = nullptr;
    if (sqlite3_open(db_path_.c_str(), &db) != SQLITE_OK) {
        return failure(AuthStatus::InvalidInput, "Failed to open user database.");
    }

    const std::string salt = PasswordHasher::generate_salt();
    const std::string hash = PasswordHasher::hash_password(password, salt);

    sqlite3_stmt* statement = nullptr;
    const char* sql = "INSERT INTO users (username, password_hash, salt, rating) VALUES (?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return failure(AuthStatus::InvalidInput, "Failed to prepare registration.");
    }

    sqlite3_bind_text(statement, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, hash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, salt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(statement, 4, EloRating::kStartingRating);

    const int step_result = sqlite3_step(statement);
    sqlite3_finalize(statement);
    sqlite3_close(db);

    if (step_result != SQLITE_DONE) {
        return failure(AuthStatus::InvalidInput, "Failed to save user.");
    }

    return success(UserRecord{username, EloRating::kStartingRating});
}

AuthResult UserDatabase::authenticate(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        return failure(AuthStatus::InvalidInput, "Username and password are required.");
    }

    sqlite3* db = nullptr;
    if (sqlite3_open(db_path_.c_str(), &db) != SQLITE_OK) {
        return failure(AuthStatus::InvalidInput, "Failed to open user database.");
    }

    sqlite3_stmt* statement = nullptr;
    const char* sql = "SELECT password_hash, salt, rating FROM users WHERE username = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return failure(AuthStatus::InvalidInput, "Failed to prepare login.");
    }

    sqlite3_bind_text(statement, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    AuthResult result = failure(AuthStatus::UserNotFound, "User not found.");
    if (sqlite3_step(statement) == SQLITE_ROW) {
        const std::string hash = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
        const std::string salt = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
        const int rating = sqlite3_column_int(statement, 2);

        if (PasswordHasher::verify_password(password, salt, hash)) {
            result = success(UserRecord{username, rating});
        } else {
            result = failure(AuthStatus::InvalidCredentials, "Invalid password.");
        }
    }

    sqlite3_finalize(statement);
    sqlite3_close(db);
    return result;
}

std::optional<UserRecord> UserDatabase::get_user(const std::string& username) const {
    sqlite3* db = nullptr;
    if (sqlite3_open(db_path_.c_str(), &db) != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_stmt* statement = nullptr;
    const char* sql = "SELECT rating FROM users WHERE username = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return std::nullopt;
    }

    sqlite3_bind_text(statement, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<UserRecord> user;
    if (sqlite3_step(statement) == SQLITE_ROW) {
        user = UserRecord{username, sqlite3_column_int(statement, 0)};
    }

    sqlite3_finalize(statement);
    sqlite3_close(db);
    return user;
}

bool UserDatabase::update_rating(const std::string& username, int rating) {
    sqlite3* db = nullptr;
    if (sqlite3_open(db_path_.c_str(), &db) != SQLITE_OK) {
        return false;
    }

    sqlite3_stmt* statement = nullptr;
    const char* sql = "UPDATE users SET rating = ? WHERE username = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_int(statement, 1, rating);
    sqlite3_bind_text(statement, 2, username.c_str(), -1, SQLITE_TRANSIENT);

    const int step_result = sqlite3_step(statement);
    sqlite3_finalize(statement);
    sqlite3_close(db);
    return step_result == SQLITE_DONE;
}
