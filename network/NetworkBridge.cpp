#include "NetworkBridge.hpp"

#include "../engine/GameEngine.hpp"
#include "../model/Board.hpp"
#include "../model/GameSnapshot.hpp"
#include "../storage/EloRating.hpp"
#include "../storage/UserDatabase.hpp"
#include "JsonCodec.hpp"
#include "Lobby.hpp"
#include "WebSocketServer.hpp"

NetworkBridge::NetworkBridge(
    GameEngine& engine,
    WebSocketServer& server,
    Lobby& lobby,
    UserDatabase& user_database
)
    : engine_(engine), server_(server), lobby_(lobby), user_database_(user_database) {
    wire_event_handlers();
}

void NetworkBridge::wire_event_handlers() {
    EventBus& bus = engine_.event_bus();

    bus.subscribe<MoveAcceptedEvent>([this](const MoveAcceptedEvent& event) {
        server_.broadcast(JsonCodec::encode_move_accepted(event));
    });
    bus.subscribe<MoveRejectedEvent>([this](const MoveRejectedEvent& event) {
        server_.broadcast(JsonCodec::encode_move_rejected(event));
    });
    bus.subscribe<JumpStartedEvent>([this](const JumpStartedEvent& event) {
        server_.broadcast(JsonCodec::encode_jump_started(event));
    });
    bus.subscribe<MoveResolvedEvent>([this](const MoveResolvedEvent& event) {
        const Board& board = Board::getInstance();
        server_.broadcast(JsonCodec::encode_move_resolved(
            event, board.getRows(), board.getCols()));
        broadcast_snapshot();
    });
    bus.subscribe<GameOverEvent>([this](const GameOverEvent&) {
        handle_game_over();
    });
}

LobbyStateMessage NetworkBridge::current_lobby_state() const {
    LobbyStateMessage state;
    if (lobby_.white_player().has_value()) {
        state.white_username = lobby_.white_player()->username;
        state.white_rating = lobby_.white_player()->rating;
    }
    if (lobby_.black_player().has_value()) {
        state.black_username = lobby_.black_player()->username;
        state.black_rating = lobby_.black_player()->rating;
    }
    state.players_joined = lobby_.players_joined();
    state.game_started = lobby_.is_started();
    return state;
}

void NetworkBridge::broadcast_lobby_state() {
    server_.broadcast(JsonCodec::encode_lobby_state(current_lobby_state()));
}

bool NetworkBridge::try_start_game() {
    if (!lobby_.is_ready() || lobby_.is_started()) {
        return false;
    }

    lobby_.start_game();
    const GameStartedMessage message{
        lobby_.white_player()->username,
        lobby_.black_player()->username,
        lobby_.white_player()->rating,
        lobby_.black_player()->rating
    };
    server_.broadcast(JsonCodec::encode_game_started(message));
    broadcast_lobby_state();
    broadcast_snapshot();
    return true;
}

GameSnapshot NetworkBridge::build_snapshot() const {
    GameSnapshot snapshot = engine_.snapshot();
    if (lobby_.white_player().has_value()) {
        snapshot.white_player_name = lobby_.white_player()->username;
        snapshot.white_player_rating = lobby_.white_player()->rating;
    }
    if (lobby_.black_player().has_value()) {
        snapshot.black_player_name = lobby_.black_player()->username;
        snapshot.black_player_rating = lobby_.black_player()->rating;
    }
    return snapshot;
}

void NetworkBridge::process_inbound() {
    Protocol::InboundFrame frame;
    while (server_.pop_inbound(frame)) {
        handle_inbound(frame);
    }
}

void NetworkBridge::join_lobby(const std::string& connection_id, const UserRecord& user) {
    const Lobby::JoinResult result = lobby_.try_login(connection_id, user.username, user.rating);
    broadcast_lobby_state();

    if (result == Lobby::JoinResult::Full) {
        server_.send_to(connection_id, JsonCodec::encode_auth_result(
            AuthResultMessage{false, user.username, user.rating, "lobby_full"}));
    }

    try_start_game();
}

void NetworkBridge::handle_auth(const Protocol::InboundFrame& frame, const Protocol::InboundMessage& message) {
    AuthResult auth = message.register_account
        ? user_database_.register_user(message.username, message.password)
        : user_database_.authenticate(message.username, message.password);

    AuthResultMessage response;
    response.username = message.username;
    response.success = auth.status == AuthStatus::Success;
    response.reason = auth.message;
    response.rating = auth.user.rating;

    server_.send_to(frame.connection_id, JsonCodec::encode_auth_result(response));
    if (!response.success) {
        return;
    }

    authenticated_connections_.insert(frame.connection_id);
    join_lobby(frame.connection_id, auth.user);
}

void NetworkBridge::handle_game_over() {
    if (!lobby_.white_player().has_value() || !lobby_.black_player().has_value()) {
        server_.broadcast(JsonCodec::encode_game_over(GameOverMessage{}));
        broadcast_snapshot();
        return;
    }

    const Board& board = Board::getInstance();
    bool white_king_alive = false;
    bool black_king_alive = false;
    for (int row = 0; row < board.getRows(); ++row) {
        for (int col = 0; col < board.getCols(); ++col) {
            const Piece piece = board.at(Position(row, col));
            if (piece.getKind() != Kind::King) {
                continue;
            }
            if (piece.getColor() == Color::White) {
                white_king_alive = true;
            } else {
                black_king_alive = true;
            }
        }
    }

    double white_score = 0.5;
    std::string winner_color = "draw";
    if (white_king_alive && !black_king_alive) {
        white_score = 1.0;
        winner_color = "white";
        lobby_.set_winner(Color::White);
    } else if (black_king_alive && !white_king_alive) {
        white_score = 0.0;
        winner_color = "black";
        lobby_.set_winner(Color::Black);
    }

    const int white_rating = lobby_.white_player()->rating;
    const int black_rating = lobby_.black_player()->rating;
    const EloRating::RatingChange rating_change =
        EloRating::apply_match_result(white_rating, black_rating, white_score);

    user_database_.update_rating(lobby_.white_player()->username, rating_change.white_rating);
    user_database_.update_rating(lobby_.black_player()->username, rating_change.black_rating);
    lobby_.update_ratings(rating_change.white_rating, rating_change.black_rating);

    const GameOverMessage message{
        winner_color,
        lobby_.white_player()->username,
        lobby_.black_player()->username,
        rating_change.white_rating,
        rating_change.black_rating,
        rating_change.white_delta,
        rating_change.black_delta
    };
    server_.broadcast(JsonCodec::encode_game_over(message));
    broadcast_snapshot();
}

void NetworkBridge::handle_inbound(const Protocol::InboundFrame& frame) {
    const std::optional<Protocol::InboundMessage> parsed = JsonCodec::parse_inbound(frame.payload);
    if (!parsed.has_value()) {
        return;
    }

    switch (parsed->kind) {
        case Protocol::InboundMessage::Kind::Auth:
            handle_auth(frame, *parsed);
            break;
        case Protocol::InboundMessage::Kind::Move:
            if (!lobby_.is_started() ||
                authenticated_connections_.count(frame.connection_id) == 0) {
                return;
            }
            engine_.request_move(parsed->src, parsed->dest);
            break;
        case Protocol::InboundMessage::Kind::Jump:
            if (!lobby_.is_started() ||
                authenticated_connections_.count(frame.connection_id) == 0) {
                return;
            }
            engine_.jump(parsed->cell);
            break;
        case Protocol::InboundMessage::Kind::Ping:
            server_.send_to(frame.connection_id, JsonCodec::encode_pong());
            break;
        case Protocol::InboundMessage::Kind::Unknown:
            break;
    }
}

void NetworkBridge::broadcast_snapshot() {
    if (!lobby_.is_started()) {
        return;
    }
    server_.broadcast(JsonCodec::encode_snapshot(build_snapshot()));
}
