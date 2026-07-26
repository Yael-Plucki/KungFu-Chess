#include "NetworkBridge.hpp"

#include "core/GameEvents.hpp"
#include "engine/GameEngine.hpp"
#include "model/Board.hpp"
#include "model/GameSnapshot.hpp"
#include "network/JsonCodec.hpp"
#include "network/Lobby.hpp"
#include "network/WebSocketServer.hpp"
#include "storage/EloRating.hpp"
#include "storage/UserDatabase.hpp"

namespace {

constexpr int kDisconnectForfeitSeconds = 20;

const std::vector<std::string> kDefaultBoardRows = {
    "bR bN bB bQ bK bB bN bR",
    "bP bP bP bP bP bP bP bP",
    ". . . . . . . .",
    ". . . . . . . .",
    ". . . . . . . .",
    ". . . . . . . .",
    "wP wP wP wP wP wP wP wP",
    "wR wN wB wQ wK wB wN wR"
};

}  // namespace

NetworkBridge::NetworkBridge(
    GameEngine& engine,
    WebSocketServer& server,
    Lobby& lobby,
    UserDatabase& user_database
)
    : engine_(engine), server_(server), lobby_(lobby), user_database_(user_database) {
    server_.set_on_connect([this](const std::string& connection_id) {
        on_client_connected(connection_id);
    });
    wire_event_handlers();
}

void NetworkBridge::wire_event_handlers() {
    EventBus& bus = engine_.event_bus();

    bus.subscribe<MoveAcceptedEvent>([this](const MoveAcceptedEvent& event) {
        server_.broadcast(JsonCodec::encode_move_accepted(event));
        broadcast_snapshot();
    });
    bus.subscribe<MoveRejectedEvent>([this](const MoveRejectedEvent& event) {
        server_.broadcast(JsonCodec::encode_move_rejected(event));
    });
    bus.subscribe<JumpStartedEvent>([this](const JumpStartedEvent& event) {
        server_.broadcast(JsonCodec::encode_jump_started(event));
        broadcast_snapshot();
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

std::optional<NetworkBridge::AuthenticatedUser> NetworkBridge::authenticated_user_for(
    const std::string& connection_id
) const {
    const auto it = authenticated_users_.find(connection_id);
    if (it == authenticated_users_.end()) {
        return std::nullopt;
    }
    return it->second;
}

void NetworkBridge::on_client_connected(const std::string& connection_id) {
    server_.send_to(connection_id, JsonCodec::encode_lobby_state(current_lobby_state()));
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

void NetworkBridge::try_match_and_start() {
    if (lobby_.is_started() && !engine_.is_game_over()) {
        return;
    }

    if (engine_.is_game_over()) {
        prepare_rematch();
    }

    if (!lobby_.is_ready()) {
        return;
    }

    const std::optional<LobbyPlayer>& white = lobby_.white_player();
    const std::optional<LobbyPlayer>& black = lobby_.black_player();
    if (!white.has_value() || !black.has_value()) {
        return;
    }

    if (!matchmaking_.is_seeking(white->connection_id) ||
        !matchmaking_.is_seeking(black->connection_id)) {
        return;
    }

    matchmaking_.remove_seeker(white->connection_id);
    matchmaking_.remove_seeker(black->connection_id);
    try_start_game();
}

void NetworkBridge::prepare_rematch() {
    lobby_.end_game();
    pending_forfeit_.reset();
    engine_.reset_for_new_game(kDefaultBoardRows);
    broadcast_lobby_state();
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

void NetworkBridge::process_disconnects() {
    std::string connection_id;
    while (server_.pop_disconnect(connection_id)) {
        handle_disconnect(connection_id);
    }
}

void NetworkBridge::process_disconnect_timers() {
    if (!pending_forfeit_.has_value() || !lobby_.is_started() || engine_.is_game_over()) {
        return;
    }

    const auto elapsed = std::chrono::steady_clock::now() - pending_forfeit_->disconnect_time;
    if (elapsed < std::chrono::seconds(kDisconnectForfeitSeconds)) {
        return;
    }

    const std::optional<Color> disconnected_color =
        lobby_.color_for_connection(pending_forfeit_->disconnected_connection_id);
    pending_forfeit_.reset();

    if (!disconnected_color.has_value()) {
        return;
    }

    const Color winner = disconnected_color.value() == Color::White ? Color::Black : Color::White;
    handle_forfeit(winner);
}

void NetworkBridge::handle_auth(const Protocol::InboundFrame& frame, const Protocol::InboundMessage& message) {
    AuthResult auth = message.register_account
        ? user_database_.register_user(message.username, message.password)
        : user_database_.authenticate(message.username, message.password);

    if (auth.status != AuthStatus::Success) {
        server_.send_to(frame.connection_id, JsonCodec::encode_auth_result(
            AuthResultMessage{false, message.username, 0, auth.message}));
        return;
    }

    authenticated_connections_.insert(frame.connection_id);
    authenticated_users_[frame.connection_id] = AuthenticatedUser{auth.user.username, auth.user.score};

    AuthResultMessage result_message{true, auth.user.username, auth.user.score, "ok"};
    switch (lobby_.try_login(frame.connection_id, auth.user.username, auth.user.score)) {
        case Lobby::JoinResult::White:
            result_message.assigned_color = "white";
            break;
        case Lobby::JoinResult::Black:
            result_message.assigned_color = "black";
            break;
        case Lobby::JoinResult::Full:
            result_message.reason = "lobby_full";
            break;
        case Lobby::JoinResult::AlreadyJoined:
            if (const std::optional<Color> color = lobby_.color_for_connection(frame.connection_id)) {
                result_message.assigned_color = color == Color::White ? "white" : "black";
            }
            break;
    }

    server_.send_to(frame.connection_id, JsonCodec::encode_auth_result(result_message));
    broadcast_lobby_state();
}

void NetworkBridge::handle_seek(const Protocol::InboundFrame& frame) {
    if (authenticated_connections_.count(frame.connection_id) == 0 ||
        lobby_.is_started() ||
        !lobby_.color_for_connection(frame.connection_id).has_value()) {
        return;
    }

    const std::optional<AuthenticatedUser> user = authenticated_user_for(frame.connection_id);
    if (!user.has_value()) {
        return;
    }

    matchmaking_.add_seeker(Seeker{frame.connection_id, user->username, user->rating});
    try_match_and_start();
}

void NetworkBridge::handle_cancel_seek(const Protocol::InboundFrame& frame) {
    matchmaking_.remove_seeker(frame.connection_id);
}

void NetworkBridge::handle_resign(const Protocol::InboundFrame& frame) {
    if (!lobby_.is_started() || engine_.is_game_over()) {
        return;
    }

    const std::optional<Color> resigned_color = lobby_.color_for_connection(frame.connection_id);
    if (!resigned_color.has_value()) {
        return;
    }

    pending_forfeit_.reset();
    const Color winner = resigned_color.value() == Color::White ? Color::Black : Color::White;
    handle_forfeit(winner);
}

void NetworkBridge::sync_authenticated_user_rating(const std::string& username, int rating) {
    for (auto& entry : authenticated_users_) {
        if (entry.second.username == username) {
            entry.second.rating = rating;
        }
    }
}

void NetworkBridge::finalize_match_with_elo(double white_score, const std::string& winner_color) {
    if (!lobby_.white_player().has_value() || !lobby_.black_player().has_value()) {
        server_.broadcast(JsonCodec::encode_game_over(GameOverMessage{}));
        broadcast_snapshot();
        return;
    }

    const int white_rating = lobby_.white_player()->rating;
    const int black_rating = lobby_.black_player()->rating;
    const EloRating::RatingChange rating_change =
        EloRating::apply_match_result(white_rating, black_rating, white_score);

    user_database_.update_score(lobby_.white_player()->username, rating_change.white_rating);
    user_database_.update_score(lobby_.black_player()->username, rating_change.black_rating);
    lobby_.update_ratings(rating_change.white_rating, rating_change.black_rating);
    sync_authenticated_user_rating(lobby_.white_player()->username, rating_change.white_rating);
    sync_authenticated_user_rating(lobby_.black_player()->username, rating_change.black_rating);

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
    prepare_rematch();
}

void NetworkBridge::handle_forfeit(Color winner) {
    if (!lobby_.white_player().has_value() || !lobby_.black_player().has_value() || engine_.is_game_over()) {
        return;
    }

    lobby_.set_winner(winner);
    engine_.force_game_over(false);

    const double white_score = winner == Color::White ? 1.0 : 0.0;
    finalize_match_with_elo(white_score, winner == Color::White ? "white" : "black");
}

void NetworkBridge::handle_game_over() {
    if (lobby_.winner_color().has_value()) {
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

    finalize_match_with_elo(white_score, winner_color);
}

void NetworkBridge::clear_connection_state(const std::string& connection_id) {
    matchmaking_.remove_seeker(connection_id);
    authenticated_connections_.erase(connection_id);
    authenticated_users_.erase(connection_id);
}

void NetworkBridge::handle_disconnect(const std::string& connection_id) {
    matchmaking_.remove_seeker(connection_id);

    if (!lobby_.is_started()) {
        lobby_.remove_player(connection_id);
        clear_connection_state(connection_id);
        broadcast_lobby_state();
        return;
    }

    if (engine_.is_game_over()) {
        clear_connection_state(connection_id);
        return;
    }

    const std::optional<Color> disconnected_color = lobby_.color_for_connection(connection_id);
    if (!disconnected_color.has_value()) {
        clear_connection_state(connection_id);
        return;
    }

    if (pending_forfeit_.has_value()) {
        clear_connection_state(connection_id);
        return;
    }

    pending_forfeit_ = PendingForfeit{
        connection_id,
        std::chrono::steady_clock::now()
    };

    server_.broadcast(JsonCodec::encode_player_disconnected(
        PlayerDisconnectedMessage{kDisconnectForfeitSeconds}));
    clear_connection_state(connection_id);
}

bool NetworkBridge::can_control_piece(const std::string& connection_id, const Position& cell) const {
    const std::optional<Color> player_color = lobby_.color_for_connection(connection_id);
    if (!player_color.has_value()) {
        return false;
    }

    const Board& board = Board::getInstance();
    if (!board.isValidPosition(cell)) {
        return false;
    }

    const Piece piece = board.at(cell);
    if (piece.getKind() == Kind::Empty) {
        return false;
    }

    return piece.getColor() == player_color.value();
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
        case Protocol::InboundMessage::Kind::Seek:
            handle_seek(frame);
            break;
        case Protocol::InboundMessage::Kind::CancelSeek:
            handle_cancel_seek(frame);
            break;
        case Protocol::InboundMessage::Kind::Resign:
            handle_resign(frame);
            break;
        case Protocol::InboundMessage::Kind::Move:
            if (!lobby_.is_started() ||
                authenticated_connections_.count(frame.connection_id) == 0) {
                return;
            }
            if (!can_control_piece(frame.connection_id, parsed->src)) {
                server_.send_to(
                    frame.connection_id,
                    JsonCodec::encode_move_rejected(
                        MoveRejectedEvent{parsed->src, parsed->dest, "wrong_player"}
                    )
                );
                return;
            }
            engine_.request_move(parsed->src, parsed->dest);
            break;
        case Protocol::InboundMessage::Kind::Jump:
            if (!lobby_.is_started() ||
                authenticated_connections_.count(frame.connection_id) == 0) {
                return;
            }
            if (!can_control_piece(frame.connection_id, parsed->cell)) {
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
