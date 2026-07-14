#include "../tests/test_framework.hpp"
#include "../engine/GameEngine.hpp"
#include "../io/BoardParser.hpp"

static void test_snapshot_exposes_read_only_view_data() {
    BoardParser parser;
    auto board = std::make_shared<Board>(parser.parseRows({
        ". wK .",
        ". . ."
    }));
    GameEngine engine(board);

    GameSnapshot snap = engine.snapshot(Position(0, 1));
    EXPECT_EQ(snap.board_width, 3);
    EXPECT_EQ(snap.board_height, 2);
    EXPECT_TRUE(snap.selected_cell.has_value());
    EXPECT_EQ(snap.selected_cell.value(), Position(0, 1));
    EXPECT_FALSE(snap.game_over);
    EXPECT_EQ(snap.pieces.size(), 1u);
    EXPECT_EQ(snap.pieces[0].kind, Kind::King);
    EXPECT_EQ(snap.pieces[0].pixel_x, 150);
    EXPECT_EQ(snap.pieces[0].pixel_y, 50);
}

static void test_snapshot_interpolates_active_motion() {
    BoardParser parser;
    auto board = std::make_shared<Board>(parser.parseRows({
        ". wR .",
        ". . .",
        ". . ."
    }));
    GameEngine engine(board);

    engine.request_move(Position(0, 1), Position(2, 1));
    engine.wait(1000);

    GameSnapshot snap = engine.snapshot();
    EXPECT_EQ(snap.pieces.size(), 1u);
    EXPECT_EQ(snap.pieces[0].state, State::Moving);
    EXPECT_EQ(snap.pieces[0].pixel_x, 150);
    EXPECT_EQ(snap.pieces[0].pixel_y, 150);
}

int main() {
    RUN_TEST(test_snapshot_exposes_read_only_view_data);
    RUN_TEST(test_snapshot_interpolates_active_motion);
    return 0;
}
