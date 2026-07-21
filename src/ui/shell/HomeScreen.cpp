#include "HomeScreen.hpp"

#include "network/MatchmakingQueue.hpp"
#include "network/RemoteGameSession.hpp"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

namespace {

std::string trim(const std::string& line) {
    const auto start = line.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    const auto end = line.find_last_not_of(" \t\r\n");
    return line.substr(start, end - start + 1);
}

bool read_line(std::istream& in, std::ostream& out, const std::string& prompt, std::string& line) {
    out << prompt;
    out.flush();
    if (!std::getline(in, line)) {
        return false;
    }
    line = trim(line);
    return true;
}

AuthMode parse_auth_mode(const std::string& choice) {
    if (choice == "2") {
        return AuthMode::Register;
    }
    return AuthMode::Login;
}

LoginResult read_credentials(std::istream& in, std::ostream& out) {
    LoginResult result;

    while (true) {
        if (!read_line(in, out, "Username: ", result.username)) {
            result.error = "Login cancelled.";
            return result;
        }
        if (!result.username.empty()) {
            break;
        }
        out << "Please enter a username.\n";
    }

    while (true) {
        if (!read_line(in, out, "Password: ", result.password)) {
            result.error = "Login cancelled.";
            return result;
        }
        if (!result.password.empty()) {
            break;
        }
        out << "Please enter a password.\n";
    }

    result.success = true;
    out << "\nWelcome, " << result.username << "!\n\n";
    return result;
}

}  // namespace

void HomeScreen::render(std::ostream& out) {
    out << "\n";
    out << "========================================\n";
    out << "           KUNG FU CHESS\n";
    out << "========================================\n";
    out << "Real-time chess. Move fast. Think faster.\n";
    out << "Starting rating: 1200 ELO\n";
    out << "========================================\n\n";
}

LoginResult HomeScreen::prompt_auth(std::istream& in, std::ostream& out, RemoteGameSession* session) {
    const bool fast_black_join = session != nullptr && session->can_join_as_black();

    if (fast_black_join) {
        const LobbyStateMessage& lobby = session->lobby_state();
        out << "White player is waiting";
        if (lobby.white_username.has_value()) {
            out << " (" << *lobby.white_username;
            if (lobby.white_rating.has_value()) {
                out << ", " << *lobby.white_rating << " ELO";
            }
            out << ")";
        }
        out << ".\n";
        out << "Log in now to join as Black.\n\n";
        out << "1) Login\n";
        out << "2) Create account\n";

        LoginResult result;
        std::string choice;
        if (!read_line(in, out, "\nChoice: ", choice)) {
            result.error = "Login cancelled.";
            return result;
        }
        result.mode = parse_auth_mode(choice);
        result = read_credentials(in, out);
        if (result.success) {
            result.mode = parse_auth_mode(choice);
        }
        return result;
    }

    render(out);
    out << "Account\n";
    out << "-------\n";
    out << "1) Login\n";
    out << "2) Create account\n";

    LoginResult result;
    std::string choice;
    if (!read_line(in, out, "\nChoice: ", choice)) {
        result.error = "Login cancelled.";
        return result;
    }

    result.mode = parse_auth_mode(choice);
    result = read_credentials(in, out);
    if (result.success) {
        result.mode = parse_auth_mode(choice);
    }
    return result;
}

bool HomeScreen::wait_for_lobby_state(RemoteGameSession& session, std::ostream& out) {
    out << "Connecting to server...\n";

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (!session.has_lobby_state() && std::chrono::steady_clock::now() < deadline) {
        session.process_messages();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    return session.has_lobby_state();
}

HomeMenuChoice HomeScreen::show_menu(std::istream& in, std::ostream& out, const std::string& username) {
    out << "Main Menu\n";
    out << "---------\n";
    out << "Signed in as: " << username << "\n";
    out << "1) Play (text script mode)\n";
    out << "2) Exit\n";

    while (true) {
        std::string choice;
        if (!read_line(in, out, "\nChoice: ", choice)) {
            return HomeMenuChoice::Exit;
        }

        if (choice == "1") {
            out << "\nEnter board setup and commands. End input with Ctrl+Z (Windows) or Ctrl+D (Unix).\n\n";
            return HomeMenuChoice::Play;
        }
        if (choice == "2") {
            out << "Goodbye, " << username << "!\n";
            return HomeMenuChoice::Exit;
        }

        out << "Invalid choice. Enter 1 or 2.\n";
    }
}

HomeMenuChoice HomeScreen::show_gui_menu(std::istream& in, std::ostream& out, const std::string& username, int rating) {
    out << "Main Menu\n";
    out << "---------\n";
    out << "Signed in as: " << username << " (Rating: " << rating << ")\n";
    out << "1) Play (find online match)\n";
    out << "2) Exit\n";

    while (true) {
        std::string choice;
        if (!read_line(in, out, "\nChoice: ", choice)) {
            return HomeMenuChoice::Exit;
        }

        if (choice == "1") {
            return HomeMenuChoice::Play;
        }
        if (choice == "2") {
            out << "Goodbye, " << username << "!\n";
            return HomeMenuChoice::Exit;
        }

        out << "Invalid choice. Enter 1 or 2.\n";
    }
}

SeekResult HomeScreen::seek_and_wait_for_match(
    RemoteGameSession& session,
    std::ostream& out,
    const std::string& /*username*/
) {
    out << "\nSearching for opponent with ELO within +/-" << MatchmakingQueue::kRatingRange << "...\n";
    session.send_seek();

    const auto deadline = std::chrono::steady_clock::now() +
        std::chrono::seconds(RemoteGameSession::kSeekTimeoutSeconds);
    int last_seconds_remaining = RemoteGameSession::kSeekTimeoutSeconds + 1;

    while (!session.game_started()) {
        session.process_messages();

        const auto now = std::chrono::steady_clock::now();
        if (now >= deadline) {
            session.send_cancel_seek();
            out << "\nCould not find an opponent within 1 minute. Please try again later.\n";
            return SeekResult::Timeout;
        }

        const auto remaining = std::chrono::duration_cast<std::chrono::seconds>(deadline - now).count();
        if (remaining != last_seconds_remaining) {
            out << "Still searching... " << remaining << "s remaining\n";
            last_seconds_remaining = static_cast<int>(remaining);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    while (!session.has_snapshot()) {
        session.process_messages();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    const LobbyStateMessage& lobby = session.lobby_state();
    out << "\nMatch found!\n";
    if (lobby.white_username.has_value() && lobby.black_username.has_value()) {
        out << "White: " << *lobby.white_username;
        if (lobby.white_rating.has_value()) {
            out << " [" << *lobby.white_rating << "]";
        }
        out << "\n";
        out << "Black: " << *lobby.black_username;
        if (lobby.black_rating.has_value()) {
            out << " [" << *lobby.black_rating << "]";
        }
        out << "\n";
    }
    out << "Opening board...\n";
    return SeekResult::Matched;
}

namespace {

void print_lobby_status(const RemoteGameSession& session, std::ostream& out, const std::string& username) {
    const LobbyStateMessage& lobby = session.lobby_state();

    out << "\nLogged in as " << username << " (Rating: " << session.player_rating() << ")\n";
    out << "Players joined: " << lobby.players_joined << " / 2\n";

    out << "White: ";
    if (lobby.white_username.has_value()) {
        out << *lobby.white_username;
        if (lobby.white_rating.has_value()) {
            out << " [" << *lobby.white_rating << "]";
        }
    } else {
        out << "(waiting)";
    }
    out << "\n";

    out << "Black: ";
    if (lobby.black_username.has_value()) {
        out << *lobby.black_username;
        if (lobby.black_rating.has_value()) {
            out << " [" << *lobby.black_rating << "]";
        }
    } else {
        out << "(waiting)";
    }
    out << "\n";

    if (lobby.players_joined == 1 &&
        lobby.white_username.has_value() &&
        *lobby.white_username == username) {
        out << "\nWaiting for Black. Ask your opponent to run kungfu_gui now.\n";
    }
}

}  // namespace

bool HomeScreen::wait_for_auth(RemoteGameSession& session, std::ostream& out, const LoginResult& /*login*/) {
    out << "Authenticating...\n";

    while (!session.auth_completed()) {
        session.process_messages();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    if (!session.auth_succeeded()) {
        out << "Authentication failed: " << session.auth_error() << "\n";
        return false;
    }

    out << "Authenticated. Rating: " << session.player_rating() << "\n";
    return true;
}

bool HomeScreen::wait_for_match(RemoteGameSession& session, std::ostream& out, const std::string& username) {
    out << "Waiting for opponent...\n";

    int last_players_joined = -1;
    while (!session.game_started()) {
        session.process_messages();

        const LobbyStateMessage& lobby = session.lobby_state();
        if (lobby.players_joined != last_players_joined) {
            print_lobby_status(session, out, username);
            last_players_joined = lobby.players_joined;
        }

        if (session.game_started()) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    while (!session.has_snapshot()) {
        session.process_messages();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    const LobbyStateMessage& lobby = session.lobby_state();
    out << "\nGame starting!\n";
    if (lobby.white_username.has_value() && lobby.black_username.has_value()) {
        out << "White: " << *lobby.white_username;
        if (lobby.white_rating.has_value()) {
            out << " [" << *lobby.white_rating << "]";
        }
        out << "\n";
        out << "Black: " << *lobby.black_username;
        if (lobby.black_rating.has_value()) {
            out << " [" << *lobby.black_rating << "]";
        }
        out << "\n";
    }
    out << "Opening board...\n";
    return true;
}
