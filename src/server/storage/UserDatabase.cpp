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

bool table_has_column(sqlite3* db, const char* column_name) {
    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(db, "PRAGMA table_info(users);", -1, &statement, nullptr) != SQLITE_OK) {
        return false;
    }

    bool found = false;
    while (sqlite3_step(statement) == SQLITE_ROW) {
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
        if (name != nullptr && std::string(name) == column_name) {
            found = true;
            break;
        }
    }

    sqlite3_finalize(statement);
    return found;
}

void migrate_legacy_schema(sqlite3* db) {
    if (!table_has_column(db, "password_hash")) {
        return;
    }

    const char* migration_sql =
        "BEGIN;"
        "ALTER TABLE users RENAME TO users_legacy;"
        "CREATE TABLE users ("
        "username TEXT PRIMARY KEY,"
        "password TEXT NOT NULL,"
        "score INTEGER NOT NULL DEFAULT 1200"
        ");"
        "INSERT INTO users (username, password, score) "
        "SELECT username, salt || ':' || password_hash, rating FROM users_legacy;"
        "DROP TABLE users_legacy;"
        "COMMIT;";

    char* error_message = nullptr;
    if (sqlite3_exec(db, migration_sql, nullptr, nullptr, &error_message) != SQLITE_OK) {
        std::string error = error_message != nullptr ? error_message : "migration error";
        sqlite3_free(error_message);
        throw std::runtime_error(error);
    }
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

    migrate_legacy_schema(db);

    const char* sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "username TEXT PRIMARY KEY,"
        "password TEXT NOT NULL,"
        "score INTEGER NOT NULL DEFAULT 1200"
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
    const std::string stored_password = PasswordHasher::format_stored_password(salt, hash);

    sqlite3_stmt* statement = nullptr;
    const char* sql = "INSERT INTO users (username, password, score) VALUES (?, ?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return failure(AuthStatus::InvalidInput, "Failed to prepare registration.");
    }

    sqlite3_bind_text(statement, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, stored_password.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(statement, 3, EloRating::kStartingRating);

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
    const char* sql = "SELECT password, score FROM users WHERE username = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return failure(AuthStatus::InvalidInput, "Failed to prepare login.");
    }

    sqlite3_bind_text(statement, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    AuthResult result = failure(AuthStatus::UserNotFound, "User not found.");
    if (sqlite3_step(statement) == SQLITE_ROW) {
        const std::string stored_password =
            reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
        const int score = sqlite3_column_int(statement, 1);

        if (PasswordHasher::verify_stored_password(password, stored_password)) {
            result = success(UserRecord{username, score});
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
    const char* sql = "SELECT score FROM users WHERE username = ?;";
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

bool UserDatabase::update_score(const std::string& username, int score) {
    sqlite3* db = nullptr;
    if (sqlite3_open(db_path_.c_str(), &db) != SQLITE_OK) {
        return false;
    }

    sqlite3_stmt* statement = nullptr;
    const char* sql = "UPDATE users SET score = ? WHERE username = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_int(statement, 1, score);
    sqlite3_bind_text(statement, 2, username.c_str(), -1, SQLITE_TRANSIENT);

    const int step_result = sqlite3_step(statement);
    sqlite3_finalize(statement);
    sqlite3_close(db);
    return step_result == SQLITE_DONE;
}
