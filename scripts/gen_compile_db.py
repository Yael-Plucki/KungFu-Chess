import json
import os
import sys

WORKSPACE = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OPENCV_INC = os.path.join("cpp", "OpenCV_451", "include")
OPENCV_SRCS = {
    os.path.join("view", "ImageVIew.cpp"),
    os.path.join("view", "Renderer.cpp"),
    "gui_main.cpp",
}
CORE_FLAGS = ["/nologo", "/std:c++17", "/W4", "/EHsc", "/I."]

SOURCES = [
    os.path.join("model", "Board.cpp"),
    os.path.join("model", "GameSnapshot.cpp"),
    os.path.join("model", "Piece.cpp"),
    os.path.join("model", "Position.cpp"),
    os.path.join("rules", "PieceRules.cpp"),
    os.path.join("rules", "RuleEngine.cpp"),
    os.path.join("realtime", "Motion.cpp"),
    os.path.join("realtime", "RealTimeArbiter.cpp"),
    os.path.join("engine", "GameEngine.cpp"),
    os.path.join("input", "BoardMapper.cpp"),
    os.path.join("input", "Controller.cpp"),
    os.path.join("io", "BoardParser.cpp"),
    os.path.join("io", "BoardPrinter.cpp"),
    os.path.join("texttests", "ScriptRunner.cpp"),
    "main.cpp",
    "gui_main.cpp",
    os.path.join("view", "ImageVIew.cpp"),
    os.path.join("view", "Renderer.cpp"),
    os.path.join("view", "animation", "AnimationConfig.cpp"),
    os.path.join("view", "animation", "PieceAnimator.cpp"),
    os.path.join("view", "animation", "AnimatorRegistry.cpp"),
    os.path.join("tests", "test_board.cpp"),
    os.path.join("tests", "test_board_mapper.cpp"),
    os.path.join("tests", "test_board_parser.cpp"),
    os.path.join("tests", "test_game_engine.cpp"),
    os.path.join("tests", "test_piece_animator.cpp"),
    os.path.join("tests", "test_piece_rules.cpp"),
    os.path.join("tests", "test_rule_engine.cpp"),
    os.path.join("tests", "test_script_runner.cpp"),
]


def main() -> int:
    entries = []
    for src in SOURCES:
        flags = list(CORE_FLAGS)
        if src in OPENCV_SRCS:
            flags.append("/I" + OPENCV_INC)
        obj = os.path.splitext(src)[0] + ".obj"
        cmd = "cl " + " ".join(flags) + " /c /Fo" + obj + " " + src
        entries.append(
            {
                "directory": WORKSPACE,
                "command": cmd,
                "file": os.path.join(WORKSPACE, src),
            }
        )

    output = os.path.join(WORKSPACE, "compile_commands.json")
    with open(output, "w", encoding="utf-8") as handle:
        json.dump(entries, handle, indent=2)
    print(f"Wrote {output} ({len(entries)} entries)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
