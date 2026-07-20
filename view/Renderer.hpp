#pragma once

#include <string>

class RemoteController;
class RemoteGameSession;

class Renderer {
public:
    Renderer(
        RemoteGameSession& session,
        RemoteController& controller,
        std::string assets_root = "CTD26/assets (2)/assets/images/pieces"
    );

    void run();

private:
    RemoteGameSession& session;
    RemoteController& controller;
    std::string assets_root;

    static void on_mouse(int event, int x, int y, int flags, void* userdata);
};
