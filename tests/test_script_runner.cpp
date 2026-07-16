#include "../tests/test_framework.hpp"
#include "../texttests/ScriptRunner.hpp"

static void test_script_runner_passes_rook_move() {
    ScriptRunner runner;
    ScriptResult result = runner.run_script({
        "Board",
        ". wR .",
        ". . .",
        ". . bK",
        "click 90 30",
        "click 90 150",
        "wait 1000",
        "print board",
        ". wR .",
        ". . .",
        ". . bK",
        "wait 1000",
        "print board",
        ". . .",
        ". . .",
        ". wR bK"
    });

    EXPECT_TRUE(result.passed);
    if (!result.passed) {
        std::cerr << result.error << "\n";
    }
}

static void test_script_runner_detects_board_mismatch() {
    ScriptRunner runner;
    ScriptResult result = runner.run_script({
        "Board",
        ". wK .",
        "print board",
        ". bK ."
    });

    EXPECT_FALSE(result.passed);
}

int main() {
    RUN_TEST(test_script_runner_passes_rook_move);
    RUN_TEST(test_script_runner_detects_board_mismatch);
    return 0;
}
