#include "test_framework.hpp"
#include "core/EventBus.hpp"
#include "core/GameEvents.hpp"
#include "engine/GameEngine.hpp"
#include "io/BoardParser.hpp"
#include <string>
#include <vector>

static void test_subscribers_receive_published_events() {
    EventBus bus;
    int call_count = 0;
    std::string last_reason;

    bus.subscribe<MoveRejectedEvent>([&](const MoveRejectedEvent& event) {
        ++call_count;
        last_reason = event.reason;
    });

    bus.publish(MoveRejectedEvent{Position(0, 0), Position(1, 1), "invalid_move"});

    EXPECT_EQ(call_count, 1);
    EXPECT_EQ(last_reason, std::string("invalid_move"));
}

static void test_unsubscribe_stops_delivery() {
    EventBus bus;
    int call_count = 0;

    EventBus::SubscriptionId id = bus.subscribe<GameOverEvent>([&](const GameOverEvent&) {
        ++call_count;
    });

    bus.publish(GameOverEvent{});
    bus.unsubscribe<GameOverEvent>(id);
    bus.publish(GameOverEvent{});

    EXPECT_EQ(call_count, 1);
}

static void test_different_event_types_are_isolated() {
    EventBus bus;
    int move_events = 0;
    int time_events = 0;

    bus.subscribe<MoveResolvedEvent>([&](const MoveResolvedEvent&) { ++move_events; });
    bus.subscribe<TimeAdvancedEvent>([&](const TimeAdvancedEvent&) { ++time_events; });

    bus.publish(MoveResolvedEvent{MoveEvent{}});
    bus.publish(TimeAdvancedEvent{16, 16});

    EXPECT_EQ(move_events, 1);
    EXPECT_EQ(time_events, 1);
}

static void test_game_engine_publishes_move_lifecycle_events() {
    BoardParser parser;
    parser.parseRows({
        ". wR bP",
        ". . .",
        ". . ."
    });

    GameEngine engine;
    bool move_accepted = false;
    int resolved_moves = 0;
    std::string rejected_reason;

    engine.event_bus().subscribe<MoveAcceptedEvent>([&](const MoveAcceptedEvent&) {
        move_accepted = true;
    });
    engine.event_bus().subscribe<MoveRejectedEvent>([&](const MoveRejectedEvent& event) {
        rejected_reason = event.reason;
    });
    engine.event_bus().subscribe<MoveResolvedEvent>([&](const MoveResolvedEvent&) {
        ++resolved_moves;
    });

    MoveResult rejected = engine.request_move(Position(0, 0), Position(2, 2));
    EXPECT_FALSE(rejected.is_accepted);
    EXPECT_EQ(rejected_reason, std::string("empty_source"));

    MoveResult accepted = engine.request_move(Position(0, 1), Position(0, 2));
    EXPECT_TRUE(accepted.is_accepted);
    EXPECT_TRUE(move_accepted);
    engine.wait(2000);

    EXPECT_EQ(resolved_moves, 1);
}

static void test_game_engine_publishes_game_over_event() {
    BoardParser parser;
    parser.parseRows({
        "wK bP .",
        ". . .",
        ". . ."
    });

    GameEngine engine;
    int game_over_count = 0;

    engine.event_bus().subscribe<GameOverEvent>([&](const GameOverEvent&) {
        ++game_over_count;
    });

    engine.jump(Position(0, 1));
    engine.request_move(Position(0, 0), Position(0, 1));
    engine.wait(1000);

    EXPECT_EQ(game_over_count, 1);
    EXPECT_TRUE(engine.is_game_over());
}

int main() {
    RUN_TEST(test_subscribers_receive_published_events);
    RUN_TEST(test_unsubscribe_stops_delivery);
    RUN_TEST(test_different_event_types_are_isolated);
    RUN_TEST(test_game_engine_publishes_move_lifecycle_events);
    RUN_TEST(test_game_engine_publishes_game_over_event);
    return 0;
}
