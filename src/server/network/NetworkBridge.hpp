#pragma once

#include "model/GameSnapshot.hpp"
#include "model/Piece.hpp"
#include "network/JsonCodec.hpp"
#include "network/MatchmakingQueue.hpp"
#include "network/Protocol.hpp"
#include "storage/UserDatabase.hpp"
#include <chrono>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

class GameEngine;
class Lobby;
class UserDatabase;
class WebSocketServer;

class NetworkBridge {
public:
    static constexpr int kDisconnectForfeitSeconds = 20;

    NetworkBridge(GameEngine& engine, WebSocketServer& server, Lobby& lobby, UserDatabase& user_database);

    void process_inbound();
    void process_disconnects();
    void process_disconnect_timers();
    void broadcast_snapshot();
    void broadcast_lobby_state();
    bool try_start_game();
    void on_client_connected(const std::string& connection_id);

private:
    struct AuthenticatedUser {
        std::string username;
        int rating = 1200;
    };

    struct PendingForfeit {
        std::string disconnected_connection_id;
        std::chrono::steady_clock::time_point disconnect_time;
    };

    GameEngine& engine_;
    WebSocketServer& server_;
    Lobby& lobby_;
    UserDatabase& user_database_;
    MatchmakingQueue matchmaking_;
    std::unordered_set<std::string> authenticated_connections_;
    std::unordered_map<std::string, AuthenticatedUser> authenticated_users_;
    std::optional<PendingForfeit> pending_forfeit_;

    void wire_event_handlers();
    void handle_inbound(const Protocol::InboundFrame& frame);
    void handle_auth(const Protocol::InboundFrame& frame, const Protocol::InboundMessage& message);
    void handle_seek(const Protocol::InboundFrame& frame);
    void handle_cancel_seek(const Protocol::InboundFrame& frame);
    void handle_resign(const Protocol::InboundFrame& frame);
    void handle_disconnect(const std::string& connection_id);
    void handle_game_over();
    void handle_forfeit(Color winner);
    void try_match_and_start();
    GameSnapshot build_snapshot() const;
    LobbyStateMessage current_lobby_state() const;
    std::optional<AuthenticatedUser> authenticated_user_for(const std::string& connection_id) const;
    void clear_connection_state(const std::string& connection_id);
};
