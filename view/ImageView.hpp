#pragma once

#include "../model/GameSnapshot.hpp"
#include <opencv2/opencv.hpp>
#include <string>
#include <unordered_map>

class AnimatorRegistry;
class Img {
public:
    Img();

    Img& read(const std::string& path,
              const std::pair<int, int>& size = {},
              bool keep_aspect = false,
              int interpolation = cv::INTER_AREA);

    void draw_on(Img& target, int x, int y) const;
    const cv::Mat& mat() const { return image; }
    bool loaded() const { return !image.empty(); }

private:
    cv::Mat image;
};

class ImageView {
public:
    explicit ImageView(std::string assets_root = "CTD26/pieces2");

    void render(const GameSnapshot& snapshot, const AnimatorRegistry& animators);
    const cv::Mat& frame() const { return canvas; }

private:
    std::string assets_root;
    cv::Mat canvas;
    std::unordered_map<std::string, cv::Mat> sprite_cache;

    cv::Mat load_sprite(const std::string& path);
    void draw_board(const GameSnapshot& snapshot);
    void draw_selection(const GameSnapshot& snapshot);
    void draw_piece(const SnapshotPiece& piece, const std::string& sprite_path);
};
