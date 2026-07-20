#include "../tests/test_framework.hpp"
#include "../network/JsonCodec.hpp"
#include "../engine/GameEngine.hpp"
#include "../io/BoardParser.hpp"
#include <string>

static void test_parse_move_command() {
    const std::optional<Protocol::InboundMessage> message =
        JsonCodec::parse_inbound(R"({"type":"move","src":{"row":0,"col":1},"dest":{"row":2,"col":1}})");

    EXPECT_TRUE(message.has_value());
    EXPECT_TRUE(message->kind == Protocol::InboundMessage::Kind::Move);
    EXPECT_EQ(message->src, Position(0, 1));
    EXPECT_EQ(message->dest, Position(2, 1));
}

static void test_parse_jump_and_ping_commands() {
    const std::optional<Protocol::InboundMessage> jump =
        JsonCodec::parse_inbound(R"({"type":"jump","cell":{"row":1,"col":3}})");
    const std::optional<Protocol::InboundMessage> ping =
        JsonCodec::parse_inbound(R"({"type":"ping"})");

    EXPECT_TRUE(jump.has_value());
    EXPECT_TRUE(jump->kind == Protocol::InboundMessage::Kind::Jump);
    EXPECT_EQ(jump->cell, Position(1, 3));

    EXPECT_TRUE(ping.has_value());
    EXPECT_TRUE(ping->kind == Protocol::InboundMessage::Kind::Ping);
}

static void test_snapshot_round_trip_fields() {
    BoardParser parser;
    parser.parseRows({
        ". wR .",
        ". . .",
        ". . ."
    });

    GameEngine engine;
    engine.request_move(Position(0, 1), Position(2, 1));
    const GameSnapshot snapshot = engine.snapshot();

    const std::string json_text = JsonCodec::encode_snapshot(snapshot);
    EXPECT_TRUE(json_text.find("\"type\":\"snapshot\"") != std::string::npos);
    EXPECT_TRUE(json_text.find("\"board_width\":3") != std::string::npos);
    EXPECT_TRUE(json_text.find("\"kind\":\"rook\"") != std::string::npos);
    EXPECT_TRUE(json_text.find("\"state\":\"moving\"") != std::string::npos);
}

static void test_move_rejected_encoding_includes_reason() {
    const std::string json_text = JsonCodec::encode_move_rejected(
        MoveRejectedEvent{Position(0, 0), Position(1, 1), "empty_source"});

    EXPECT_TRUE(json_text.find("\"type\":\"move_rejected\"") != std::string::npos);
    EXPECT_TRUE(json_text.find("\"reason\":\"empty_source\"") != std::string::npos);
}

int main() {
    RUN_TEST(test_parse_move_command);
    RUN_TEST(test_parse_jump_and_ping_commands);
    RUN_TEST(test_snapshot_round_trip_fields);
    RUN_TEST(test_move_rejected_encoding_includes_reason);
    return 0;
}
