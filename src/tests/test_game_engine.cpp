#include "test_framework.hpp"
#include "engine/GameEngine.hpp"
#include "io/BoardParser.hpp"
#include "model/Board.hpp"

static void test_snapshot_exposes_read_only_view_data() {
    BoardParser parser;
    parser.parseRows({
        ". wK .",
        ". . ."
    });
    GameEngine engine;

    GameSnapshot snap = engine.snapshot(Position(0, 1));
    EXPECT_EQ(snap.board_width, 3);
    EXPECT_EQ(snap.board_height, 2);
    EXPECT_TRUE(snap.selected_cell.has_value());
    EXPECT_EQ(snap.selected_cell.value(), Position(0, 1));
    EXPECT_FALSE(snap.game_over);
    EXPECT_EQ(snap.pieces.size(), 1u);
    EXPECT_EQ(snap.pieces[0].kind, Kind::King);
    EXPECT_EQ(snap.pieces[0].cell, Position(0, 1));
    EXPECT_FALSE(snap.pieces[0].motion.has_value());
}

static void test_snapshot_interpolates_active_motion() {
    BoardParser parser;
    parser.parseRows({
        ". wR .",
        ". . .",
        ". . ."
    });
    GameEngine engine;

    engine.request_move(Position(0, 1), Position(2, 1));
    engine.wait(1000);

    GameSnapshot snap = engine.snapshot();
    EXPECT_EQ(snap.pieces.size(), 1u);
    EXPECT_EQ(snap.pieces[0].state, State::Moving);
    EXPECT_TRUE(snap.pieces[0].motion.has_value());
    EXPECT_EQ(snap.pieces[0].motion.value().source, Position(0, 1));
    EXPECT_EQ(snap.pieces[0].motion.value().destination, Position(2, 1));
}

static void test_king_entering_jump_cell_ends_game() {
    BoardParser parser;
    parser.parseRows({
        "wK bP .",
        ". . .",
        ". . ."
    });
    GameEngine engine;

    engine.jump(Position(0, 1));
    engine.request_move(Position(0, 0), Position(0, 1));
    engine.wait(1000);

    EXPECT_TRUE(engine.is_game_over());
    GameSnapshot snap = engine.snapshot();
    EXPECT_EQ(snap.pieces.size(), 1u);
    EXPECT_EQ(snap.pieces[0].kind, Kind::Pawn);
    EXPECT_EQ(snap.pieces[0].cell, Position(0, 1));
}

static void test_motion_vacates_source_square() {
    BoardParser parser;
    parser.parseRows({
        ". wR .",
        ". . .",
        ". . ."
    });
    GameEngine engine;

    engine.request_move(Position(0, 1), Position(2, 1));
    EXPECT_TRUE(Board::getInstance().at(Position(0, 1)).getKind() == Kind::Empty);

    GameSnapshot snap = engine.snapshot();
    EXPECT_EQ(snap.pieces.size(), 1u);
    EXPECT_EQ(snap.pieces[0].cell, Position(0, 1));
    EXPECT_EQ(snap.pieces[0].state, State::Moving);
}

static void test_can_move_into_vacated_square() {
    BoardParser parser;
    parser.parseRows({
        "wK wR .",
        ". . .",
        ". . ."
    });
    GameEngine engine;

    engine.request_move(Position(0, 1), Position(2, 1));
    MoveResult result = engine.request_move(Position(0, 0), Position(0, 1));
    EXPECT_TRUE(result.is_accepted);
}

static void test_capture_idle_defender_after_source_vacated() {
    BoardParser parser;
    parser.parseRows({
        ". wR bP",
        ". . .",
        ". . ."
    });
    GameEngine engine;

    MoveResult result = engine.request_move(Position(0, 1), Position(0, 2));
    EXPECT_TRUE(result.is_accepted);
    engine.wait(2000);

    GameSnapshot snap = engine.snapshot();
    EXPECT_EQ(snap.pieces.size(), 1u);
    EXPECT_EQ(snap.pieces[0].kind, Kind::Rook);
    EXPECT_EQ(snap.pieces[0].cell, Position(0, 2));
    EXPECT_EQ(snap.stats.white.score, 1);
}

static void test_jump_capture_increases_score() {
    BoardParser parser;
    parser.parseRows({
        "wR bR .",
        ". . .",
        ". . ."
    });
    GameEngine engine;

    engine.jump(Position(0, 0));
    engine.request_move(Position(0, 1), Position(0, 0));
    engine.wait(2000);

    GameSnapshot snap = engine.snapshot();
    EXPECT_EQ(snap.pieces.size(), 1u);
    EXPECT_EQ(snap.pieces[0].kind, Kind::Rook);
    EXPECT_EQ(snap.pieces[0].color, Color::White);
    EXPECT_EQ(snap.stats.white.score, 5);
}

static void test_score_uses_standard_piece_values() {
    BoardParser parser;
    parser.parseRows({
        ". wR bQ",
        ". . .",
        ". . ."
    });
    GameEngine engine;

    engine.request_move(Position(0, 1), Position(0, 2));
    engine.wait(2000);

    GameSnapshot snap = engine.snapshot();
    EXPECT_EQ(snap.stats.white.score, 9);
}

int main() {
    RUN_TEST(test_snapshot_exposes_read_only_view_data);
    RUN_TEST(test_snapshot_interpolates_active_motion);
    RUN_TEST(test_king_entering_jump_cell_ends_game);
    RUN_TEST(test_motion_vacates_source_square);
    RUN_TEST(test_can_move_into_vacated_square);
    RUN_TEST(test_capture_idle_defender_after_source_vacated);
    RUN_TEST(test_jump_capture_increases_score);
    RUN_TEST(test_score_uses_standard_piece_values);
    return 0;
}
