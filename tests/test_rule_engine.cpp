#include "../tests/test_framework.hpp"
#include "../rules/RuleEngine.hpp"
#include "../io/BoardParser.hpp"

static Board& make_board(const std::vector<std::string>& rows) {
    BoardParser parser;
    return parser.parseRows(rows);
}

static void test_rule_engine_reasons() {
    RuleEngine engine;
    Board& board = make_board({
        ". wR .",
        ". . .",
        ". . ."
    });

    MoveValidation outside = engine.validate_move(board, Position(5, 5), Position(0, 1));
    EXPECT_FALSE(outside.is_valid);
    EXPECT_EQ(outside.reason, std::string("outside_board"));

    MoveValidation empty = engine.validate_move(board, Position(0, 0), Position(0, 2));
    EXPECT_FALSE(empty.is_valid);
    EXPECT_EQ(empty.reason, std::string("empty_source"));

    MoveValidation friendly = engine.validate_move(board, Position(0, 1), Position(0, 1));
    EXPECT_FALSE(friendly.is_valid);
    EXPECT_EQ(friendly.reason, std::string("friendly_destination"));

    MoveValidation illegal = engine.validate_move(board, Position(0, 1), Position(2, 2));
    EXPECT_FALSE(illegal.is_valid);
    EXPECT_EQ(illegal.reason, std::string("illegal_piece_move"));
}

int main() {
    RUN_TEST(test_rule_engine_reasons);
    return 0;
}
