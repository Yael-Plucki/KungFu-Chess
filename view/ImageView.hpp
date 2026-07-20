#pragma once

#include "../CTD26/cpp/src/img.hpp"
#include "../model/GameSnapshot.hpp"
#include <opencv2/opencv.hpp>
#include <string>
#include <unordered_map>

class AnimatorRegistry;
class BoardMapper;

class ImageView {
public:
    explicit ImageView(std::string assets_root = "CTD26/assets (2)/assets/images/pieces");

    void render(const GameSnapshot& snapshot, const AnimatorRegistry& animators);
    const cv::Mat& frame() const { return canvas.get_mat(); }

private:
    std::string assets_root;
    Img canvas;
    std::unordered_map<std::string, Img> sprite_cache;

    Img& load_sprite(const std::string& path, const BoardMapper& mapper);
    void draw_board(const GameSnapshot& snapshot, const BoardMapper& mapper);
    void draw_side_panels(const GameSnapshot& snapshot, const BoardMapper& mapper);
    void draw_selection(const GameSnapshot& snapshot, const BoardMapper& mapper);
    void draw_piece(
        const SnapshotPiece& piece,
        const std::string& sprite_path,
        const BoardMapper& mapper,
        long long current_time
    );
};
