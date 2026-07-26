#include "PasswordHasher.hpp"

#include <functional>
#include <random>
#include <sstream>
#include <iomanip>

namespace {

std::string to_hex(unsigned long long value) {
    std::ostringstream out;
    out << std::hex << value;
    return out.str();
}

}  // namespace

namespace PasswordHasher {

std::string generate_salt() {
    std::random_device device;
    std::mt19937_64 rng(device());
    return to_hex(rng()) + to_hex(rng());
}

std::string hash_password(const std::string& password, const std::string& salt) {
    std::hash<std::string> hasher;
    const std::string material = salt + ":" + password + ":kungfu";
    return to_hex(hasher(material));
}

bool verify_password(const std::string& password, const std::string& salt, const std::string& hash) {
    return hash_password(password, salt) == hash;
}

std::string format_stored_password(const std::string& salt, const std::string& hash) {
    return salt + ":" + hash;
}

bool verify_stored_password(const std::string& password, const std::string& stored) {
    const std::size_t separator = stored.find(':');
    if (separator == std::string::npos) {
        return false;
    }

    const std::string salt = stored.substr(0, separator);
    const std::string hash = stored.substr(separator + 1);
    return verify_password(password, salt, hash);
}

}  // namespace PasswordHasher
