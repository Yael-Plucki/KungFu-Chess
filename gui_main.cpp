#include "engine/GameEngine.hpp"
#include "input/BoardMapper.hpp"
#include "input/Controller.hpp"
#include "io/BoardParser.hpp"
#include "view/Renderer.hpp"
#include <iostream>

int main() {
    try {
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
