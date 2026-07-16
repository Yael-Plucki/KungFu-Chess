#include "Renderer.hpp"
#include "ImageView.hpp"
#include "animation/AnimatorRegistry.hpp"
#include "../engine/GameEngine.hpp"
#include "../input/Controller.hpp"
#include <opencv2/opencv.hpp>

namespace {
constexpr const char* kWindowName = "KungFu Chess";
constexpr int kFrameMs = 16;
}  // namespace
Renderer::Renderer(GameEngine& engine, Controller& controller, std::string assets_root)
    : engine(engine), controller(controller), assets_root(std::move(assets_root)) {}

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

void Renderer::run() {
    ImageView view(assets_root);
    AnimatorRegistry animators(assets_root);
    cv::namedWindow(kWindowName);
    cv::setMouseCallback(kWindowName, on_mouse, this);

    while (true) {
        engine.wait(kFrameMs);
        const GameSnapshot snapshot = engine.snapshot(controller.get_selected_cell());
        animators.sync(snapshot);
        animators.update(kFrameMs);
        view.render(snapshot, animators);
        cv::imshow(kWindowName, view.frame());

        const int key = cv::waitKey(1);
        if (key == 27 || key == 'q' || key == 'Q') {
            break;
        }
    }

    cv::destroyAllWindows();
}
