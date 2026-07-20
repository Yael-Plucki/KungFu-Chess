#pragma once

#include "../model/GameSnapshot.hpp"
#include "JsonCodec.hpp"
#include "Protocol.hpp"
#include "../storage/UserDatabase.hpp"
#include <string>
#include <unordered_set>

class GameEngine;
class Lobby;
class UserDatabase;
class WebSocketServer;

class NetworkBridge {
public:
    NetworkBridge(GameEngine& engine, WebSocketServer& server, Lobby& lobby, UserDatabase& user_database);

    void process_inbound();
    void broadcast_snapshot();
    void broadcast_lobby_state();
    bool try_start_game();
    void on_client_connected(const std::string& connection_id);

private:
    GameEngine& engine_;
    WebSocketServer& server_;
    Lobby& lobby_;
    UserDatabase& user_database_;
    std::unordered_set<std::string> authenticated_connections_;

    void wire_event_handlers();
    void handle_inbound(const Protocol::InboundFrame& frame);
    void handle_auth(const Protocol::InboundFrame& frame, const Protocol::InboundMessage& message);
    void handle_game_over();
    void join_lobby(const std::string& connection_id, const UserRecord& user);
    GameSnapshot build_snapshot() const;
    LobbyStateMessage current_lobby_state() const;
};
