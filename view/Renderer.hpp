#pragma once

#include <string>

class GameEngine;
class Controller;

class Renderer {
public:
    Renderer(GameEngine& engine, Controller& controller, std::string assets_root = "CTD26/assets (2)/assets/images/pieces");

    void run();

private:
    GameEngine& engine;
    Controller& controller;
    std::string assets_root;

    static void on_mouse(int event, int x, int y, int flags, void* userdata);
};
