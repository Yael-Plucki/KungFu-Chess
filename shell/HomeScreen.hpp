#pragma once

#include <iosfwd>
#include <string>

enum class AuthMode {
    Login,
    Register
};

struct LoginResult {
    bool success = false;
    std::string username;
    std::string password;
    AuthMode mode = AuthMode::Login;
    std::string error;
};

enum class HomeMenuChoice {
    Play,
    Exit,
    Invalid
};

class RemoteGameSession;

class HomeScreen {
public:
    static void render(std::ostream& out);
    static LoginResult prompt_auth(std::istream& in, std::ostream& out, RemoteGameSession* session = nullptr);
    static bool wait_for_lobby_state(RemoteGameSession& session, std::ostream& out);
    static HomeMenuChoice show_menu(std::istream& in, std::ostream& out, const std::string& username);
    static bool wait_for_auth(RemoteGameSession& session, std::ostream& out, const LoginResult& login);
    static bool wait_for_match(RemoteGameSession& session, std::ostream& out, const std::string& username);
};
