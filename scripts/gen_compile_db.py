import json
import os
import sys

WORKSPACE = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OPENCV_INC = os.path.join("cpp", "OpenCV_451", "include")
OPENCV_SRCS = {
    os.path.join("view", "ImageVIew.cpp"),
    os.path.join("view", "Renderer.cpp"),
    os.path.join("CTD26", "cpp", "src", "img.cpp"),
    "gui_main.cpp",
}
BASE_FLAGS = ["-std=c++17", "-Wall", "-I.", "--target=x86_64-pc-windows-msvc"]

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
    os.path.join("CTD26", "cpp", "src", "img.cpp"),
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


def quote_path(path: str) -> str:
    return f'"{path}"' if " " in path else path


def discover_system_includes() -> list[str]:
    if sys.platform != "win32":
        return []

    includes: list[str] = []

    program_files = os.environ.get("ProgramFiles", r"C:\Program Files")
    msvc_root = os.path.join(
        program_files, "Microsoft Visual Studio", "2022", "Community", "VC", "Tools", "MSVC"
    )
    if os.path.isdir(msvc_root):
        for version in sorted(os.listdir(msvc_root), reverse=True):
            include_dir = os.path.join(msvc_root, version, "include")
            if os.path.isdir(include_dir):
                includes.append(include_dir)
                break

    kits_root = os.path.join(
        os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)"),
        "Windows Kits",
        "10",
        "Include",
    )
    if os.path.isdir(kits_root):
        sdk_versions = sorted(
            (name for name in os.listdir(kits_root) if name.startswith("10.")),
            reverse=True,
        )
        for version in sdk_versions:
            sdk_base = os.path.join(kits_root, version)
            sdk_dirs = [os.path.join(sdk_base, sub) for sub in ("ucrt", "shared", "um")]
            if all(os.path.isdir(path) for path in sdk_dirs):
                includes.extend(sdk_dirs)
                break

    return includes


def isystem_flags(paths: list[str]) -> list[str]:
    flags: list[str] = []
    for path in paths:
        flags.extend(["-isystem", quote_path(path)])
    return flags


def build_command(src: str, system_includes: list[str]) -> str:
    flags = list(BASE_FLAGS)
    flags.extend(isystem_flags(system_includes))
    if src in OPENCV_SRCS:
        flags.append("-I" + OPENCV_INC)
    obj = os.path.splitext(src)[0] + ".obj"
    return "clang++ " + " ".join(flags) + " -c -o " + obj + " " + src


def write_clangd(system_includes: list[str]) -> None:
    lines = [
        "CompileFlags:",
        "  Add:",
        "    - --target=x86_64-pc-windows-msvc",
        "    - -Wno-unknown-warning-option",
        "    - -Wno-microsoft-enum-value",
    ]
    for path in system_includes:
        lines.append("    - -isystem")
        lines.append(f"    - {quote_path(path.replace(chr(92), '/'))}")
    lines.extend(
        [
            "",
            "---",
            "If:",
            "  PathMatch: cpp/OpenCV_451/.*",
            "Index:",
            "  Background: Skip",
            "",
        ]
    )
    output = os.path.join(WORKSPACE, ".clangd")
    with open(output, "w", encoding="utf-8", newline="\n") as handle:
        handle.write("\n".join(lines))


def main() -> int:
    system_includes = discover_system_includes()
    if not system_includes:
        print("Warning: no MSVC/Windows SDK include paths found; clangd may show false errors.")

    entries = []
    for src in SOURCES:
        entries.append(
            {
                "directory": WORKSPACE,
                "command": build_command(src, system_includes),
                "file": os.path.join(WORKSPACE, src),
            }
        )

    compile_db = os.path.join(WORKSPACE, "compile_commands.json")
    with open(compile_db, "w", encoding="utf-8") as handle:
        json.dump(entries, handle, indent=2)

    write_clangd(system_includes)
    print(f"Wrote {compile_db} ({len(entries)} entries)")
    print(f"Wrote {os.path.join(WORKSPACE, '.clangd')} ({len(system_includes)} system include paths)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
