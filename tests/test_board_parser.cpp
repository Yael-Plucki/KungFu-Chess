#include "../tests/test_framework.hpp"
#include "../io/BoardParser.hpp"
#include "../io/BoardPrinter.hpp"

static void test_parses_rectangular_board() {
    BoardParser parser;
    Board board = parser.parseRows({
        "wK . bR",
        ". . .",
        ". wN bK"
    });

    EXPECT_EQ(board.getRows(), 3);
    EXPECT_EQ(board.getCols(), 3);
    EXPECT_EQ(board.at(0, 0).getKind(), Kind::King);
    EXPECT_EQ(board.at(0, 2).getKind(), Kind::Rook);
}

static void test_printer_round_trip() {
    BoardParser parser;
    BoardPrinter printer;
    Board board = parser.parseRows({
        ". wR .",
        ". . .",
        ". . bK"
    });

    std::vector<std::string> lines = printer.format(board);
    EXPECT_EQ(lines.size(), 3u);
    EXPECT_EQ(lines[0], ". wR .");
    EXPECT_EQ(lines[2], ". . bK");
}

int main() {
    RUN_TEST(test_parses_rectangular_board);
    RUN_TEST(test_printer_round_trip);
    return 0;
}
