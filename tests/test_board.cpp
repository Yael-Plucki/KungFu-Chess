#include "../tests/test_framework.hpp"
#include "../model/Board.hpp"
#include "../io/BoardParser.hpp"

static void test_move_piece_updates_board() {
    BoardParser parser;
    Board board = parser.parseRows({
        ". wR .",
        ". . ."
    });

    board.move_piece(Position(0, 1), Position(1, 1));
    EXPECT_EQ(board.at(0, 1).getKind(), Kind::Empty);
    EXPECT_EQ(board.at(1, 1).getKind(), Kind::Rook);
}

static void test_add_piece_rejects_duplicate_occupancy() {
    BoardParser parser;
    Board board = parser.parseRows({". wK ."});

    bool threw = false;
    try {
        board.add_piece(Piece(Color::Black, Kind::Rook, Position(0, 1)));
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    EXPECT_TRUE(threw);
}

int main() {
    RUN_TEST(test_move_piece_updates_board);
    RUN_TEST(test_add_piece_rejects_duplicate_occupancy);
    return 0;
}
