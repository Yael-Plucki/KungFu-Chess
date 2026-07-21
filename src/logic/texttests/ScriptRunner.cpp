#include "ScriptRunner.hpp"
#include "../engine/GameEngine.hpp"
#include "../input/BoardMapper.hpp"
#include "../input/Controller.hpp"
#include "../io/BoardParser.hpp"
#include "../io/BoardPrinter.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

namespace {
std::string trim(const std::string& line) {
    const auto start = line.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    const auto end = line.find_last_not_of(" \t\r\n");
    return line.substr(start, end - start + 1);
}
}

bool ScriptRunner::is_command_line(const std::string& line) {
    return line == "Board" ||
           line == "Board:" ||
           line == "Commands:" ||
           line == "print board" ||
           line.rfind("click", 0) == 0 ||
           line.rfind("wait", 0) == 0 ||
           line.rfind("jump", 0) == 0;
}

ScriptResult ScriptRunner::run(const std::vector<std::string>& lines, ScriptMode mode) {
    BoardParser parser;
    BoardPrinter printer;

    std::unique_ptr<GameEngine> engine;
    std::unique_ptr<BoardMapper> mapper;
    std::unique_ptr<Controller> controller;

    for (size_t i = 0; i < lines.size(); ++i) {
        std::string line = trim(lines[i]);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        if (line == "Board" || line == "Board:") {
            std::vector<std::string> board_rows;
            ++i;
            while (i < lines.size() && !is_command_line(trim(lines[i])) && !trim(lines[i]).empty()) {
                board_rows.push_back(trim(lines[i]));
                ++i;
            }
            --i;

            try {
                parser.parseRows(board_rows);
            } catch (const std::exception& e) {
                return {false, e.what()};
            }

            engine = std::make_unique<GameEngine>();
            mapper = std::make_unique<BoardMapper>(
                Board::getInstance().getRows(),
                Board::getInstance().getCols()
            );
            controller = std::make_unique<Controller>(*engine, *mapper);
            continue;
        }

        if (!engine || !controller || !mapper) {
            return {false, "Board must be defined before commands"};
        }

        if (line.rfind("click", 0) == 0) {
            std::stringstream ss(line);
            std::string command;
            int x = 0;
            int y = 0;
            ss >> command >> x >> y;
            controller->click(x, y);
        } else if (line.rfind("wait", 0) == 0) {
            std::stringstream ss(line);
            std::string command;
            int milliseconds = 0;
            ss >> command >> milliseconds;
            engine->wait(milliseconds);
        } else if (line.rfind("jump", 0) == 0) {
            std::stringstream ss(line);
            std::string command;
            int x = 0;
            int y = 0;
            ss >> command >> x >> y;
            std::optional<Position> cell = mapper->pixel_to_cell(x, y);
            if (cell.has_value()) {
                engine->jump(cell.value());
            }
        } else if (line == "print board") {
            GameSnapshot snap = engine->snapshot(controller->get_selected_cell());
            if (mode == ScriptMode::Interactive) {
                printer.print(snap);
                continue;
            }

            std::vector<std::string> expected;
            ++i;
            while (i < lines.size() && !is_command_line(trim(lines[i])) && !trim(lines[i]).empty()) {
                expected.push_back(trim(lines[i]));
                ++i;
            }
            --i;

            std::vector<std::string> actual = printer.format(snap);
            if (actual != expected) {
                std::ostringstream error;
                error << "Board mismatch\nExpected:\n";
                for (const std::string& row : expected) {
                    error << row << "\n";
                }
                error << "Actual:\n";
                for (const std::string& row : actual) {
                    error << row << "\n";
                }
                return {false, error.str()};
            }
        }
    }

    return {true, ""};
}

ScriptResult ScriptRunner::run_script(const std::vector<std::string>& lines) {
    return run(lines, ScriptMode::Test);
}

ScriptResult ScriptRunner::run_script_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return {false, "Could not open script file: " + filename};
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    return run_script(lines);
}

ScriptResult ScriptRunner::run_interactive() {
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(std::cin, line)) {
        lines.push_back(line);
    }
    return run(lines, ScriptMode::Interactive);
}
