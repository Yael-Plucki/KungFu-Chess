#pragma once

#include <string>

namespace PasswordHasher {

std::string generate_salt();
std::string hash_password(const std::string& password, const std::string& salt);
bool verify_password(const std::string& password, const std::string& salt, const std::string& hash);

}  // namespace PasswordHasher
