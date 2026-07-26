#include "Renderer.hpp"
#include "ImageView.hpp"
#include "animation/AnimatorRegistry.hpp"
#include "core/GameEvents.hpp"
#include "input/RemoteController.hpp"
#include "network/ClientBoardSync.hpp"
#include "network/RemoteGameSession.hpp"
#include <algorithm>
#include <chrono>
#include <opencv2/opencv.hpp>

namespace {
constexpr int kFrameMs = 16;
constexpr int kErrorDisplayMs = 3000;
}  // namespace

Renderer::Renderer(
    RemoteGameSession& session,
    ClientBoardSync& board_sync,
    RemoteController& controller,
    std::string assets_root,
    std::string window_title
)
    : session(session),
      board_sync(board_sync),
      controller(controller),
      assets_root(std::move(assets_root)),
      window_title(std::move(window_title)) {}

void Renderer::on_mouse(int event, int x, int y, int /*flags*/, void* userdata) {
    if (userdata == nullptr) {
        return;
    }

    auto* renderer = static_cast<Renderer*>(userdata);
    if (event == cv::EVENT_LBUTTONDOWN) {
        renderer->controller.click(x, y);
    } else if (event == cv::EVENT_RBUTTONDOWN) {
        renderer->controller.jump(x, y);
    }
}

void Renderer::pump_ui() const {
    cv::waitKey(1);
}

std::string Renderer::format_rejection_reason(const std::string& reason) {
    if (reason == "outside_board") {
        return "Move is outside the board";
    }
    if (reason == "empty_source") {
        return "No piece at source square";
    }
    if (reason == "friendly_destination") {
        return "Cannot move to your own piece";
    }
    if (reason == "illegal_piece_move") {
        return "Illegal move for this piece";
    }
    if (reason == "game_over") {
        return "Game is over";
    }
    if (reason == "motion_start_failed") {
        return "Piece is already moving";
    }
    if (reason == "wrong_player") {
        return "Not your turn";
    }
    return reason;
}

void Renderer::subscribe_to_errors() {
    move_rejected_subscription_ = board_sync.event_bus().subscribe<MoveRejectedEvent>(
        [this](const MoveRejectedEvent& event) {
            error_message_ = format_rejection_reason(event.reason);
            error_visible_until_ = std::chrono::steady_clock::now() +
                std::chrono::milliseconds(kErrorDisplayMs);
        }
    );
}

void Renderer::show_loading_screen() const {
    cv::Mat loading(320, 640, CV_8UC3, cv::Scalar(40, 40, 40));
    cv::putText(
        loading,
        "Loading board...",
        cv::Point(220, 160),
        cv::FONT_HERSHEY_SIMPLEX,
        0.8,
        cv::Scalar(230, 230, 230),
        2,
        cv::LINE_AA
    );
    cv::imshow(window_title, loading);
    pump_ui();
}

void Renderer::run() {
    ImageView view(assets_root);
    AnimatorRegistry animators(assets_root);

#ifdef _WIN32
    cv::startWindowThread();
#endif
    cv::namedWindow(window_title, cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback(window_title, on_mouse, this);
    subscribe_to_errors();
    show_loading_screen();

    session.process_messages();
    GameSnapshot snapshot = board_sync.snapshot_with_selection(std::nullopt);
    animators.sync(snapshot);
    view.warm_cache(snapshot, animators);
    session.process_messages();
    pump_ui();

    auto last_frame_time = std::chrono::steady_clock::now();

    while (true) {
        const auto frame_start = std::chrono::steady_clock::now();
        const int frame_delta_ms = static_cast<int>(std::clamp<std::int64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(frame_start - last_frame_time).count(),
            1,
            100
        ));
        last_frame_time = frame_start;

        session.process_messages();
        snapshot = board_sync.snapshot_with_selection(controller.get_selected_cell());
        animators.sync(snapshot);
        animators.update(frame_delta_ms);
        pump_ui();
        view.render(snapshot, animators);

        if (snapshot.game_over) {
            view.draw_game_over_banner();
        } else if (session.opponent_disconnected()) {
            view.draw_disconnect_banner(session.disconnect_countdown_seconds());
        } else if (
            error_message_.has_value() &&
            std::chrono::steady_clock::now() < error_visible_until_
        ) {
            view.draw_error_banner(*error_message_);
        }

        cv::imshow(window_title, view.frame());

        const int key = cv::waitKey(kFrameMs);
        if (key == 27 || key == 'q' || key == 'Q') {
            if (!snapshot.game_over) {
                session.send_resign();
            }
            break;
        }
    }

    cv::destroyWindow(window_title);
}
