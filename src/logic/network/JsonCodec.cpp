#include "JsonCodec.hpp"

#include <nlohmann/json.hpp>
#include <stdexcept>

namespace {

using json = nlohmann::json;

json position_to_json(const Position& pos) {
    return json{{"row", pos.getRow()}, {"col", pos.getCol()}};
}

Position position_from_json(const json& value) {
    return Position(value.at("row").get<int>(), value.at("col").get<int>());
}

std::string color_to_string(Color color) {
    return color == Color::White ? "white" : "black";
}

std::string kind_to_string(Kind kind) {
    switch (kind) {
        case Kind::King:
            return "king";
        case Kind::Queen:
            return "queen";
        case Kind::Rook:
            return "rook";
        case Kind::Bishop:
            return "bishop";
        case Kind::Knight:
            return "knight";
        case Kind::Pawn:
            return "pawn";
        default:
            return "empty";
    }
}

std::string state_to_string(State state) {
    switch (state) {
        case State::Moving:
            return "moving";
        case State::Captured:
            return "captured";
        default:
            return "idle";
    }
}

Color color_from_string(const std::string& value) {
    return value == "black" ? Color::Black : Color::White;
}

Kind kind_from_string(const std::string& value) {
    if (value == "king") return Kind::King;
    if (value == "queen") return Kind::Queen;
    if (value == "rook") return Kind::Rook;
    if (value == "bishop") return Kind::Bishop;
    if (value == "knight") return Kind::Knight;
    if (value == "pawn") return Kind::Pawn;
    return Kind::Empty;
}

State state_from_string(const std::string& value) {
    if (value == "moving") return State::Moving;
    if (value == "captured") return State::Captured;
    return State::Idle;
}

std::optional<std::string> optional_username(const json& payload, const char* key) {
    if (!payload.contains(key) || payload.at(key).is_null()) {
        return std::nullopt;
    }
    return payload.at(key).get<std::string>();
}

json motion_to_json(const ActiveMotionInfo& motion) {
    return json{
        {"source", position_to_json(motion.source)},
        {"destination", position_to_json(motion.destination)},
        {"start_time_ms", motion.start_time},
        {"duration_ms", motion.duration}
    };
}

json snapshot_piece_to_json(const SnapshotPiece& piece) {
    json value = json{
        {"id", piece.id},
        {"color", color_to_string(piece.color)},
        {"kind", kind_to_string(piece.kind)},
        {"state", state_to_string(piece.state)},
        {"row", piece.cell.getRow()},
        {"col", piece.cell.getCol()},
        {"is_jump_motion", piece.is_jump_motion}
    };

    if (piece.motion.has_value()) {
        value["motion"] = motion_to_json(piece.motion.value());
    }

    return value;
}

json move_event_to_json(const MoveEvent& move, int board_rows, int board_cols) {
    json value = json{
        {"piece_id", move.piece_id},
        {"color", color_to_string(move.color)},
        {"kind", kind_to_string(move.kind)},
        {"from", position_to_json(move.from)},
        {"to", position_to_json(move.to)},
        {"is_jump", move.is_jump},
        {"notation", format_move_event(move, board_rows, board_cols)}
    };

    if (move.captured.has_value()) {
        value["captured"] = kind_to_string(move.captured.value());
    }
    if (move.promoted_to.has_value()) {
        value["promoted_to"] = kind_to_string(move.promoted_to.value());
    }

    return value;
}

}  // namespace

std::optional<Protocol::InboundMessage> JsonCodec::parse_inbound(const std::string& json_text) {
    try {
        const json payload = json::parse(json_text);
        const std::string type = payload.at("type").get<std::string>();

        Protocol::InboundMessage message;
        if (type == Protocol::kAuth) {
            message.kind = Protocol::InboundMessage::Kind::Auth;
            message.username = payload.at("username").get<std::string>();
            message.password = payload.at("password").get<std::string>();
            message.register_account = payload.value("register", false);
            return message;
        }
        if (type == Protocol::kMove) {
            message.kind = Protocol::InboundMessage::Kind::Move;
            message.src = position_from_json(payload.at("src"));
            message.dest = position_from_json(payload.at("dest"));
            return message;
        }
        if (type == Protocol::kJump) {
            message.kind = Protocol::InboundMessage::Kind::Jump;
            message.cell = position_from_json(payload.at("cell"));
            return message;
        }
        if (type == Protocol::kPing) {
            message.kind = Protocol::InboundMessage::Kind::Ping;
            return message;
        }
        if (type == Protocol::kSeek) {
            message.kind = Protocol::InboundMessage::Kind::Seek;
            return message;
        }
        if (type == Protocol::kCancelSeek) {
            message.kind = Protocol::InboundMessage::Kind::CancelSeek;
            return message;
        }
        if (type == Protocol::kResign) {
            message.kind = Protocol::InboundMessage::Kind::Resign;
            return message;
        }

        message.kind = Protocol::InboundMessage::Kind::Unknown;
        return message;
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::string JsonCodec::encode_pong() {
    return json{{"type", Protocol::kPong}}.dump();
}

std::string JsonCodec::encode_auth(const std::string& username, const std::string& password, bool register_account) {
    return json{
        {"type", Protocol::kAuth},
        {"username", username},
        {"password", password},
        {"register", register_account}
    }.dump();
}

std::string JsonCodec::encode_seek() {
    return json{{"type", Protocol::kSeek}}.dump();
}

std::string JsonCodec::encode_cancel_seek() {
    return json{{"type", Protocol::kCancelSeek}}.dump();
}

std::string JsonCodec::encode_resign() {
    return json{{"type", Protocol::kResign}}.dump();
}

std::string JsonCodec::encode_auth_result(const AuthResultMessage& result) {
    return json{
        {"type", Protocol::kAuthResult},
        {"success", result.success},
        {"username", result.username},
        {"rating", result.rating},
        {"reason", result.reason}
    }.dump();
}

std::string JsonCodec::encode_snapshot(const GameSnapshot& snapshot) {
    json pieces = json::array();
    for (const SnapshotPiece& piece : snapshot.pieces) {
        pieces.push_back(snapshot_piece_to_json(piece));
    }

    json selected = nullptr;
    if (snapshot.selected_cell.has_value()) {
        selected = position_to_json(snapshot.selected_cell.value());
    }

    const json payload = json{
        {"type", Protocol::kSnapshot},
        {"board_width", snapshot.board_width},
        {"board_height", snapshot.board_height},
        {"game_over", snapshot.game_over},
        {"current_time_ms", snapshot.current_time},
        {"selected_cell", selected},
        {"pieces", pieces},
        {"stats", json{
            {"white_score", snapshot.stats.white.score},
            {"black_score", snapshot.stats.black.score}
        }},
        {"white_player", snapshot.white_player_name},
        {"black_player", snapshot.black_player_name},
        {"white_rating", snapshot.white_player_rating},
        {"black_rating", snapshot.black_player_rating}
    };

    return payload.dump();
}

std::string JsonCodec::encode_lobby_state(const LobbyStateMessage& lobby) {
    json white = lobby.white_username.has_value() ? json(*lobby.white_username) : json(nullptr);
    json black = lobby.black_username.has_value() ? json(*lobby.black_username) : json(nullptr);

    return json{
        {"type", Protocol::kLobbyState},
        {"white", white},
        {"black", black},
        {"white_rating", lobby.white_rating.has_value() ? json(*lobby.white_rating) : json(nullptr)},
        {"black_rating", lobby.black_rating.has_value() ? json(*lobby.black_rating) : json(nullptr)},
        {"players_joined", lobby.players_joined},
        {"game_started", lobby.game_started}
    }.dump();
}

std::string JsonCodec::encode_game_started(const GameStartedMessage& message) {
    return json{
        {"type", Protocol::kGameStarted},
        {"white", message.white_username},
        {"black", message.black_username},
        {"white_rating", message.white_rating},
        {"black_rating", message.black_rating}
    }.dump();
}

std::string JsonCodec::encode_move_accepted(const MoveAcceptedEvent& event) {
    return json{
        {"type", Protocol::kMoveAccepted},
        {"src", position_to_json(event.src)},
        {"dest", position_to_json(event.dest)}
    }.dump();
}

std::string JsonCodec::encode_move_rejected(const MoveRejectedEvent& event) {
    return json{
        {"type", Protocol::kMoveRejected},
        {"src", position_to_json(event.src)},
        {"dest", position_to_json(event.dest)},
        {"reason", event.reason}
    }.dump();
}

std::string JsonCodec::encode_jump_started(const JumpStartedEvent& event) {
    return json{
        {"type", Protocol::kJumpStarted},
        {"cell", position_to_json(event.cell)}
    }.dump();
}

std::string JsonCodec::encode_move_resolved(const MoveResolvedEvent& event, int board_rows, int board_cols) {
    return json{
        {"type", Protocol::kMoveResolved},
        {"move", move_event_to_json(event.move, board_rows, board_cols)}
    }.dump();
}

std::string JsonCodec::encode_game_over(const GameOverMessage& message) {
    return json{
        {"type", Protocol::kGameOver},
        {"winner_color", message.winner_color},
        {"white", message.white_username},
        {"black", message.black_username},
        {"white_rating", message.white_rating},
        {"black_rating", message.black_rating},
        {"white_rating_delta", message.white_rating_delta},
        {"black_rating_delta", message.black_rating_delta}
    }.dump();
}

std::string JsonCodec::encode_player_disconnected(const PlayerDisconnectedMessage& message) {
    return json{
        {"type", Protocol::kPlayerDisconnected},
        {"seconds_remaining", message.seconds_remaining}
    }.dump();
}

std::optional<GameSnapshot> JsonCodec::decode_snapshot(const std::string& json_text) {
    try {
        const json payload = json::parse(json_text);
        if (payload.at("type").get<std::string>() != Protocol::kSnapshot) {
            return std::nullopt;
        }

        GameSnapshot snapshot;
        snapshot.board_width = payload.at("board_width").get<int>();
        snapshot.board_height = payload.at("board_height").get<int>();
        snapshot.game_over = payload.at("game_over").get<bool>();
        snapshot.current_time = payload.at("current_time_ms").get<long long>();
        snapshot.stats.white.score = payload.at("stats").at("white_score").get<int>();
        snapshot.stats.black.score = payload.at("stats").at("black_score").get<int>();

        if (payload.contains("white_player") && !payload.at("white_player").is_null()) {
            snapshot.white_player_name = payload.at("white_player").get<std::string>();
        }
        if (payload.contains("black_player") && !payload.at("black_player").is_null()) {
            snapshot.black_player_name = payload.at("black_player").get<std::string>();
        }
        if (payload.contains("white_rating") && !payload.at("white_rating").is_null()) {
            snapshot.white_player_rating = payload.at("white_rating").get<int>();
        }
        if (payload.contains("black_rating") && !payload.at("black_rating").is_null()) {
            snapshot.black_player_rating = payload.at("black_rating").get<int>();
        }

        if (payload.contains("selected_cell") && !payload.at("selected_cell").is_null()) {
            snapshot.selected_cell = position_from_json(payload.at("selected_cell"));
        }

        for (const json& piece_json : payload.at("pieces")) {
            SnapshotPiece piece;
            piece.id = piece_json.at("id").get<int>();
            piece.color = color_from_string(piece_json.at("color").get<std::string>());
            piece.kind = kind_from_string(piece_json.at("kind").get<std::string>());
            piece.state = state_from_string(piece_json.at("state").get<std::string>());
            piece.cell = Position(piece_json.at("row").get<int>(), piece_json.at("col").get<int>());
            piece.is_jump_motion = piece_json.value("is_jump_motion", false);

            if (piece_json.contains("motion") && !piece_json.at("motion").is_null()) {
                const json& motion_json = piece_json.at("motion");
                ActiveMotionInfo motion;
                motion.active = true;
                motion.source = position_from_json(motion_json.at("source"));
                motion.destination = position_from_json(motion_json.at("destination"));
                motion.start_time = motion_json.at("start_time_ms").get<long long>();
                motion.duration = motion_json.at("duration_ms").get<long long>();
                motion.piece_id = piece.id;
                motion.color = piece.color;
                motion.kind = piece.kind;
                piece.motion = motion;
            }

            snapshot.pieces.push_back(piece);
        }

        return snapshot;
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<LobbyStateMessage> JsonCodec::decode_lobby_state(const std::string& json_text) {
    try {
        const json payload = json::parse(json_text);
        if (payload.at("type").get<std::string>() != Protocol::kLobbyState) {
            return std::nullopt;
        }

        LobbyStateMessage lobby;
        lobby.white_username = optional_username(payload, "white");
        lobby.black_username = optional_username(payload, "black");
        if (payload.contains("white_rating") && !payload.at("white_rating").is_null()) {
            lobby.white_rating = payload.at("white_rating").get<int>();
        }
        if (payload.contains("black_rating") && !payload.at("black_rating").is_null()) {
            lobby.black_rating = payload.at("black_rating").get<int>();
        }
        lobby.players_joined = payload.at("players_joined").get<int>();
        lobby.game_started = payload.at("game_started").get<bool>();
        return lobby;
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<GameStartedMessage> JsonCodec::decode_game_started(const std::string& json_text) {
    try {
        const json payload = json::parse(json_text);
        if (payload.at("type").get<std::string>() != Protocol::kGameStarted) {
            return std::nullopt;
        }

        return GameStartedMessage{
            payload.at("white").get<std::string>(),
            payload.at("black").get<std::string>(),
            payload.value("white_rating", 1200),
            payload.value("black_rating", 1200)
        };
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<AuthResultMessage> JsonCodec::decode_auth_result(const std::string& json_text) {
    try {
        const json payload = json::parse(json_text);
        if (payload.at("type").get<std::string>() != Protocol::kAuthResult) {
            return std::nullopt;
        }

        AuthResultMessage result;
        result.success = payload.at("success").get<bool>();
        result.username = payload.value("username", "");
        result.rating = payload.value("rating", 1200);
        result.reason = payload.value("reason", "");
        return result;
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<GameOverMessage> JsonCodec::decode_game_over(const std::string& json_text) {
    try {
        const json payload = json::parse(json_text);
        if (payload.at("type").get<std::string>() != Protocol::kGameOver) {
            return std::nullopt;
        }

        return GameOverMessage{
            payload.at("winner_color").get<std::string>(),
            payload.at("white").get<std::string>(),
            payload.at("black").get<std::string>(),
            payload.value("white_rating", 1200),
            payload.value("black_rating", 1200),
            payload.value("white_rating_delta", 0),
            payload.value("black_rating_delta", 0)
        };
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<PlayerDisconnectedMessage> JsonCodec::decode_player_disconnected(const std::string& json_text) {
    try {
        const json payload = json::parse(json_text);
        if (payload.at("type").get<std::string>() != Protocol::kPlayerDisconnected) {
            return std::nullopt;
        }

        return PlayerDisconnectedMessage{
            payload.value("seconds_remaining", 20)
        };
    } catch (const std::exception&) {
        return std::nullopt;
    }
}
