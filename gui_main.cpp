#include "engine/GameEngine.hpp"
#include "input/BoardMapper.hpp"
#include "input/Controller.hpp"
#include "io/BoardParser.hpp"
#include "view/Renderer.hpp"
#include <filesystem>
#include <iostream>

namespace {
void use_project_root_as_working_directory() {
    namespace fs = std::filesystem;
    fs::path dir = fs::current_path();
    for (int depth = 0; depth < 6; ++depth) {
        if (fs::exists(dir / "CMakeLists.txt") && fs::exists(dir / "main.cpp")) {
            fs::current_path(dir);
            return;
        }
        if (!dir.has_parent_path()) {
            break;
        }
        dir = dir.parent_path();
    }
}
}  // namespace

int main() {
    try {
        use_project_root_as_working_directory();
        BoardParser parser;
        parser.parseRows({
            "bR bN bB bQ bK bB bN bR",
            "bP bP bP bP bP bP bP bP",
            ". . . . . . . .",
            ". . . . . . . .",
            ". . . . . . . .",
            ". . . . . . . .",
            "wP wP wP wP wP wP wP wP",
            "wR wN wB wQ wK wB wN wR"
        });

        GameEngine engine;
        BoardMapper mapper(Board::getInstance().getRows(), Board::getInstance().getCols());
        Controller controller(engine, mapper);
        Renderer renderer(engine, controller);
        renderer.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
