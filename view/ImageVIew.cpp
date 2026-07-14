#include "ImageView.hpp"
#include "animation/AnimatorRegistry.hpp"
#include "../model/GameConstants.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
void blend_bgra_onto_bgr(const cv::Mat& source, cv::Mat& target) {
    for (int row = 0; row < source.rows; ++row) {
        for (int col = 0; col < source.cols; ++col) {
            cv::Vec4b src = source.at<cv::Vec4b>(row, col);
            if (src[3] == 0) {
                continue;
            }

            const float alpha = src[3] / 255.0f;
            cv::Vec3b& dst = target.at<cv::Vec3b>(row, col);
            for (int channel = 0; channel < 3; ++channel) {
                dst[channel] = cv::saturate_cast<uchar>(
                    (1.0f - alpha) * dst[channel] + alpha * src[channel]
                );
            }
        }
    }
}

void draw_sprite_on_canvas(cv::Mat& canvas, const cv::Mat& sprite, int x, int y) {
    if (sprite.empty() || canvas.empty()) {
        return;
    }

    cv::Mat source = sprite;
    if (source.channels() == 3 && canvas.channels() == 4) {
        cv::cvtColor(source, source, cv::COLOR_BGR2BGRA);
    } else if (source.channels() == 4 && canvas.channels() == 3) {
        cv::Mat bgr;
        cv::cvtColor(source, bgr, cv::COLOR_BGRA2BGR);
        source = bgr;
    }

    const int draw_h = source.rows;
    const int draw_w = source.cols;
    if (x < 0 || y < 0 || x + draw_w > canvas.cols || y + draw_h > canvas.rows) {
        return;
    }

    cv::Mat region = canvas(cv::Rect(x, y, draw_w, draw_h));
    if (sprite.channels() == 4 && canvas.channels() == 3) {
        blend_bgra_onto_bgr(sprite, region);
    } else {
        source.copyTo(region);
    }
}
}

Img::Img() = default;

Img& Img::read(const std::string& path,
               const std::pair<int, int>& size,
               bool keep_aspect,
               int interpolation) {
    image = cv::imread(path, cv::IMREAD_UNCHANGED);
    if (image.empty()) {
        throw std::runtime_error("Cannot load image: " + path);
    }

    if (size.first <= 0 || size.second <= 0) {
        return *this;
    }

    const int width = image.cols;
    const int height = image.rows;
    if (keep_aspect) {
        const double scale = std::min(
            static_cast<double>(size.first) / width,
            static_cast<double>(size.second) / height
        );
        cv::resize(
            image,
            image,
            cv::Size(static_cast<int>(width * scale), static_cast<int>(height * scale)),
            0,
            0,
            interpolation
        );
    } else {
        cv::resize(image, image, cv::Size(size.first, size.second), 0, 0, interpolation);
    }

    return *this;
}

void Img::draw_on(Img& target, int x, int y) const {
    if (!loaded() || !target.loaded()) {
        throw std::runtime_error("Both images must be loaded before drawing.");
    }

    draw_sprite_on_canvas(target.image, image, x, y);
}

ImageView::ImageView(std::string assets_root)
    : assets_root(std::move(assets_root)) {}

void ImageView::render(const GameSnapshot& snapshot, const AnimatorRegistry& animators) {
    const int width = snapshot.board_width * GameConstants::CELL_SIZE;
    const int height = snapshot.board_height * GameConstants::CELL_SIZE;
    canvas = cv::Mat(height, width, CV_8UC3, cv::Scalar(40, 40, 40));

    draw_board(snapshot);
    for (const SnapshotPiece& piece : snapshot.pieces) {
        draw_piece(piece, animators.sprite_path_for(piece));
    }
    draw_selection(snapshot);
}

cv::Mat ImageView::load_sprite(const std::string& path) {
    const auto cached = sprite_cache.find(path);
    if (cached != sprite_cache.end()) {
        return cached->second;
    }

    cv::Mat sprite = cv::imread(path, cv::IMREAD_UNCHANGED);
    if (sprite.empty()) {
        throw std::runtime_error("Cannot load sprite: " + path);
    }

    const int target = GameConstants::CELL_SIZE - 10;
    const double scale = std::min(
        static_cast<double>(target) / sprite.cols,
        static_cast<double>(target) / sprite.rows
    );
    cv::resize(
        sprite,
        sprite,
        cv::Size(static_cast<int>(sprite.cols * scale), static_cast<int>(sprite.rows * scale)),
        0,
        0,
        cv::INTER_AREA
    );

    sprite_cache.emplace(path, sprite);
    return sprite;
}

void ImageView::draw_board(const GameSnapshot& snapshot) {
    for (int row = 0; row < snapshot.board_height; ++row) {
        for (int col = 0; col < snapshot.board_width; ++col) {
            const cv::Scalar light(210, 180, 140);
            const cv::Scalar dark(120, 80, 40);
            const cv::Scalar color = ((row + col) % 2 == 0) ? light : dark;
            const cv::Point top_left(col * GameConstants::CELL_SIZE, row * GameConstants::CELL_SIZE);
            const cv::Point bottom_right(
                (col + 1) * GameConstants::CELL_SIZE,
                (row + 1) * GameConstants::CELL_SIZE
            );
            cv::rectangle(canvas, top_left, bottom_right, color, cv::FILLED);
        }
    }
}

void ImageView::draw_selection(const GameSnapshot& snapshot) {
    if (!snapshot.selected_cell.has_value()) {
        return;
    }

    const Position cell = snapshot.selected_cell.value();
    const cv::Point top_left(
        cell.getCol() * GameConstants::CELL_SIZE,
        cell.getRow() * GameConstants::CELL_SIZE
    );
    const cv::Point bottom_right(
        (cell.getCol() + 1) * GameConstants::CELL_SIZE,
        (cell.getRow() + 1) * GameConstants::CELL_SIZE
    );
    cv::rectangle(canvas, top_left, bottom_right, cv::Scalar(0, 255, 255), 3);
}

void ImageView::draw_piece(const SnapshotPiece& piece, const std::string& sprite_file) {
    cv::Mat sprite;
    try {
        sprite = load_sprite(sprite_file);
    } catch (const std::exception&) {
        cv::circle(
            canvas,
            cv::Point(piece.pixel_x, piece.pixel_y),
            GameConstants::CELL_SIZE / 4,
            piece.color == Color::White ? cv::Scalar(240, 240, 240) : cv::Scalar(30, 30, 30),
            cv::FILLED
        );
        return;
    }

    const int draw_x = piece.pixel_x - sprite.cols / 2;
    const int draw_y = piece.pixel_y - sprite.rows / 2;
    draw_sprite_on_canvas(canvas, sprite, draw_x, draw_y);
}
