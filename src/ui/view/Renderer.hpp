#pragma once

#include "core/EventBus.hpp"
#include <chrono>
#include <optional>
#include <string>

class RemoteController;
class RemoteGameSession;
class ClientBoardSync;

class Renderer {
public:
    Renderer(
        RemoteGameSession& session,
        ClientBoardSync& board_sync,
        RemoteController& controller,
        std::string assets_root = "CTD26/assets (2)/assets/images/pieces",
        std::string window_title = "KungFu Chess"
    );

    void run();

private:
    RemoteGameSession& session;
    ClientBoardSync& board_sync;
    RemoteController& controller;
    std::string assets_root;
    std::string window_title;
    EventBus::SubscriptionId move_rejected_subscription_ = 0;
    std::optional<std::string> error_message_;
    std::chrono::steady_clock::time_point error_visible_until_{};

    void pump_ui() const;
    void show_loading_screen() const;
    void subscribe_to_errors();
    static std::string format_rejection_reason(const std::string& reason);

    static void on_mouse(int event, int x, int y, int flags, void* userdata);
};
