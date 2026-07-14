#include "engine/GameEngine.hpp"
#include "input/BoardMapper.hpp"
#include "input/Controller.hpp"
#include "io/BoardParser.hpp"
#include "view/Renderer.hpp"
#include <iostream>
#include <memory>

int main() {
    try {
        BoardParser parser;
        auto board = std::make_shared<Board>(parser.parseRows({
            ". wR .",
            ". . .",
            ". . bK"
        }));

        GameEngine engine(board);
        BoardMapper mapper(board->getRows(), board->getCols());
        Controller controller(engine, mapper);
        Renderer renderer(engine, controller);
        renderer.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
