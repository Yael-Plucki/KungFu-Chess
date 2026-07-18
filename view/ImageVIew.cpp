#include "ImageView.hpp"
#include "animation/AnimatorRegistry.hpp"
#include "../input/BoardMapper.hpp"
#include <filesystem>
#include <stdexcept>

ImageView::ImageView(std::string assets_root)
    : assets_root(std::move(assets_root)) {}

void ImageView::render(const GameSnapshot& snapshot, const AnimatorRegistry& animators) {
    const BoardMapper mapper(snapshot.board_height, snapshot.board_width);
    canvas.create(mapper.display_width(), mapper.display_height(), CV_8UC3, cv::Scalar(40, 40, 40));

    draw_board(snapshot, mapper);
    for (const SnapshotPiece& piece : snapshot.pieces) {
        draw_piece(piece, animators.sprite_path_for(piece), mapper, snapshot.current_time);
    }
    draw_selection(snapshot, mapper);
}

Img& ImageView::load_sprite(const std::string& path, const BoardMapper& mapper) {
    const auto cached = sprite_cache.find(path);
    if (cached != sprite_cache.end()) {
        return cached->second;
    }

    const int target = mapper.cell_display_size() - 6;
    Img sprite;
    sprite.read(path, {target, target}, true, cv::INTER_AREA);
    const auto inserted = sprite_cache.emplace(path, std::move(sprite));
    return inserted.first->second;
}

void ImageView::draw_board(const GameSnapshot& snapshot, const BoardMapper& mapper) {
    const std::string board_path =
        (std::filesystem::path(assets_root).parent_path() / "board.png").string();

    try {
        const auto cached = sprite_cache.find(board_path);
        if (cached != sprite_cache.end()) {
            cached->second.draw_on(canvas, 0, 0);
            return;
        }

        Img board;
        board.read(board_path, {mapper.display_width(), mapper.display_height()}, false, cv::INTER_AREA);
        board.draw_on(canvas, 0, 0);
        sprite_cache.emplace(board_path, std::move(board));
        return;
    } catch (const std::exception&) {
    }

    cv::Mat& board = canvas.mat();
    for (int row = 0; row < snapshot.board_height; ++row) {
        for (int col = 0; col < snapshot.board_width; ++col) {
            const cv::Scalar light(210, 180, 140);
            const cv::Scalar dark(120, 80, 40);
            const cv::Scalar color = ((row + col) % 2 == 0) ? light : dark;
            int top_left_x = 0;
            int top_left_y = 0;
            int bottom_right_x = 0;
            int bottom_right_y = 0;
            mapper.cell_origin(Position(row, col), top_left_x, top_left_y);
            mapper.cell_origin(Position(row + 1, col + 1), bottom_right_x, bottom_right_y);
            cv::rectangle(
                board,
                cv::Point(top_left_x, top_left_y),
                cv::Point(bottom_right_x, bottom_right_y),
                color,
                cv::FILLED
            );
        }
    }
}

void ImageView::draw_selection(const GameSnapshot& snapshot, const BoardMapper& mapper) {
    if (!snapshot.selected_cell.has_value()) {
        return;
    }

    const Position cell = snapshot.selected_cell.value();
    int top_left_x = 0;
    int top_left_y = 0;
    int bottom_right_x = 0;
    int bottom_right_y = 0;
    mapper.cell_origin(cell, top_left_x, top_left_y);
    mapper.cell_origin(Position(cell.getRow() + 1, cell.getCol() + 1), bottom_right_x, bottom_right_y);
    cv::rectangle(
        canvas.mat(),
        cv::Point(top_left_x, top_left_y),
        cv::Point(bottom_right_x, bottom_right_y),
        cv::Scalar(0, 255, 255),
        3
    );
}

void ImageView::draw_piece(
    const SnapshotPiece& piece,
    const std::string& sprite_file,
    const BoardMapper& mapper,
    long long current_time
) {
    int center_x = 0;
    int center_y = 0;
    if (piece.motion.has_value()) {
        mapper.motion_center(piece.motion.value(), current_time, center_x, center_y);
    } else {
        mapper.cell_center(piece.cell, center_x, center_y);
    }

    try {
        Img& sprite = load_sprite(sprite_file, mapper);
        const cv::Mat& sprite_mat = sprite.get_mat();
        const int draw_x = center_x - sprite_mat.cols / 2;
        const int draw_y = center_y - sprite_mat.rows / 2;

        sprite.draw_on(canvas, draw_x, draw_y);
    } catch (const std::exception&) {
        cv::circle(
            canvas.mat(),
            cv::Point(center_x, center_y),
            mapper.cell_display_size() / 4,
            piece.color == Color::White ? cv::Scalar(240, 240, 240) : cv::Scalar(30, 30, 30),
            cv::FILLED
        );
    }
}
