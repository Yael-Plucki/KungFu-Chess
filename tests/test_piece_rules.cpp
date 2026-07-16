#include "../tests/test_framework.hpp"
#include "../rules/PieceRules.hpp"
#include "../io/BoardParser.hpp"

static Board& make_board(const std::vector<std::string>& rows) {
    BoardParser parser;
    return parser.parseRows(rows);
}

static void test_pawn_forward_move() {
    Board& board = make_board({
        ". . .",
        ". wP .",
        ". . ."
    });

    EXPECT_TRUE(PieceRules::is_destination_legal(board, Position(1, 1), Position(0, 1)));
    EXPECT_FALSE(PieceRules::is_destination_legal(board, Position(1, 1), Position(2, 1)));
}

static void test_pawn_diagonal_capture_only() {
    Board& board = make_board({
        "bP . bP",
        ". wP .",
        ". . ."
    });

    EXPECT_TRUE(PieceRules::is_destination_legal(board, Position(1, 1), Position(0, 0)));
    EXPECT_TRUE(PieceRules::is_destination_legal(board, Position(1, 1), Position(0, 2)));
}

static void test_pawn_cannot_capture_forward() {
    Board& board = make_board({
        ". bP .",
        ". wP .",
        ". . ."
    });

    EXPECT_FALSE(PieceRules::is_destination_legal(board, Position(1, 1), Position(0, 1)));
}

static void test_pawn_double_step_from_start() {
    Board& white_board = make_board({
        ". . .",
        ". . .",
        ". wP ."
    });

    EXPECT_TRUE(PieceRules::is_destination_legal(white_board, Position(2, 1), Position(1, 1)));
    EXPECT_TRUE(PieceRules::is_destination_legal(white_board, Position(2, 1), Position(0, 1)));
}

static void test_rook_legal_destinations() {
    Board& board = make_board({
        ". wR .",
        ". bP .",
        ". . ."
    });

    std::set<Position> destinations = PieceRules::legal_destinations(board, board.at(Position(0, 1)));
    EXPECT_TRUE(destinations.count(Position(0, 0)) > 0);
    EXPECT_TRUE(destinations.count(Position(0, 2)) > 0);
    EXPECT_TRUE(destinations.count(Position(1, 1)) > 0);
    EXPECT_FALSE(destinations.count(Position(2, 1)) > 0);
}

int main() {
    RUN_TEST(test_pawn_forward_move);
    RUN_TEST(test_pawn_diagonal_capture_only);
    RUN_TEST(test_pawn_cannot_capture_forward);
    RUN_TEST(test_pawn_double_step_from_start);
    RUN_TEST(test_rook_legal_destinations);
    return 0;
}
