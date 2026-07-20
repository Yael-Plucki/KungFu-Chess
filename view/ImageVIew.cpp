#include "ImageView.hpp"
#include "animation/AnimatorRegistry.hpp"
#include "../input/BoardMapper.hpp"
#include "../model/GameConstants.hpp"
#include "../model/GameStats.hpp"
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace {
int measure_text_width(const std::string& text, double scale) {
    int baseline = 0;
    const cv::Size size = cv::getTextSize(
        text,
        cv::FONT_HERSHEY_SIMPLEX,
        scale,
        1,
        &baseline
    );
    return size.width;
}

std::vector<std::string> wrap_text(
    const std::string& text,
    int max_width,
    double scale,
    const std::string& indent = "  "
) {
    if (text.empty()) {
        return {};
    }
    if (measure_text_width(text, scale) <= max_width) {
        return {text};
    }

    std::vector<std::string> words;
    {
        std::istringstream stream(text);
        std::string word;
        while (stream >> word) {
            words.push_back(word);
        }
    }

    std::vector<std::string> lines;
    std::string current;
    bool needs_indent = false;

    auto flush = [&]() {
        if (!current.empty()) {
            lines.push_back(current);
            current.clear();
            needs_indent = true;
        }
    };

    for (const std::string& word : words) {
        const std::string candidate = current.empty()
            ? (needs_indent ? indent + word : word)
            : current + " " + word;

        if (measure_text_width(candidate, scale) <= max_width) {
            current = candidate;
            continue;
        }

        flush();

        const std::string next = needs_indent ? indent + word : word;
        if (measure_text_width(next, scale) <= max_width) {
            current = next;
            continue;
        }

        std::string chunk_prefix = needs_indent ? indent : "";
        std::string chunk;
        for (char ch : word) {
            const std::string try_chunk = chunk + ch;
            if (measure_text_width(chunk_prefix + try_chunk, scale) <= max_width) {
                chunk = try_chunk;
            } else {
                if (!chunk.empty()) {
                    lines.push_back(chunk_prefix + chunk);
                    chunk_prefix = indent;
                    chunk = std::string(1, ch);
                } else {
                    lines.push_back(chunk_prefix + std::string(1, ch));
                    chunk_prefix = indent;
                    chunk.clear();
                }
            }
        }
        if (!chunk.empty()) {
            current = chunk_prefix + chunk;
        }
    }

    flush();
    return lines;
}
}  // namespace

ImageView::ImageView(std::string assets_root)
    : assets_root(std::move(assets_root)) {}

void ImageView::render(const GameSnapshot& snapshot, const AnimatorRegistry& animators) {
    const BoardMapper mapper(snapshot.board_height, snapshot.board_width);
    canvas.create(mapper.display_width(), mapper.display_height(), CV_8UC3, cv::Scalar(40, 40, 40));

    draw_side_panels(snapshot, mapper);
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
    const int board_x = mapper.board_offset_x();
    const std::string board_path =
        (std::filesystem::path(assets_root).parent_path() / "board.png").string();

    try {
        const auto cached = sprite_cache.find(board_path);
        if (cached != sprite_cache.end()) {
            cached->second.draw_on(canvas, board_x, 0);
            return;
        }

        Img board;
        const int board_width = snapshot.board_width * mapper.cell_display_size();
        const int board_height = snapshot.board_height * mapper.cell_display_size();
        board.read(board_path, {board_width, board_height}, false, cv::INTER_AREA);
        board.draw_on(canvas, board_x, 0);
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

namespace {
void draw_panel_text(
    cv::Mat& canvas,
    int x,
    int y,
    const std::string& text,
    const cv::Scalar& color,
    double scale = 0.45
) {
    cv::putText(canvas, text, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, scale, color, 1, cv::LINE_AA);
}

void draw_color_panel(
    cv::Mat& canvas,
    int panel_x,
    int panel_width,
    int panel_height,
    Color color,
    const ColorSideStats& stats,
    const std::string& player_name,
    int player_rating
) {
    const cv::Scalar panel_bg = color == Color::White ? cv::Scalar(235, 235, 235) : cv::Scalar(45, 45, 45);
    const cv::Scalar title_color = color == Color::White ? cv::Scalar(20, 20, 20) : cv::Scalar(230, 230, 230);
    const cv::Scalar text_color = color == Color::White ? cv::Scalar(40, 40, 40) : cv::Scalar(210, 210, 210);

    cv::rectangle(
        canvas,
        cv::Point(panel_x, 0),
        cv::Point(panel_x + panel_width, panel_height),
        panel_bg,
        cv::FILLED
    );

    int y = 24;
    const std::string side_label = color == Color::White ? "White" : "Black";
    const std::string title = player_name.empty() ? side_label : player_name;
    draw_panel_text(canvas, panel_x + 10, y, title, title_color, 0.6);
    y += 22;
    if (!player_name.empty()) {
        draw_panel_text(canvas, panel_x + 10, y, side_label, text_color, 0.45);
        y += 20;
    }
    draw_panel_text(canvas, panel_x + 10, y, "Rating: " + std::to_string(player_rating), title_color, 0.5);
    y += 24;
    draw_panel_text(canvas, panel_x + 10, y, "Score: " + std::to_string(stats.score), title_color, 0.5);
    y += 28;
    draw_panel_text(canvas, panel_x + 10, y, "Moves:", title_color, 0.5);
    y += 22;

    constexpr int line_height = 18;
    constexpr double move_scale = 0.4;
    const int text_left = panel_x + 10;
    const int max_text_width = panel_width - 20;
    const int max_y = panel_height - 10;

    auto draw_wrapped = [&](const std::string& text, const std::string& indent = "  ") -> bool {
        for (const std::string& line : wrap_text(text, max_text_width, move_scale, indent)) {
            if (y > max_y) {
                draw_panel_text(canvas, text_left, y, "...", text_color, move_scale);
                return false;
            }
            draw_panel_text(canvas, text_left, y, line, text_color, move_scale);
            y += line_height;
        }
        return true;
    };

    for (const PieceMoveHistory& piece : stats.pieces) {
        if (y > max_y) {
            draw_panel_text(canvas, text_left, y, "...", text_color, move_scale);
            return;
        }

        std::ostringstream line;
        line << format_piece_label(piece.color, piece.kind) << ":";
        if (!piece.moves.empty()) {
            line << " ";
            for (std::size_t i = 0; i < piece.moves.size(); ++i) {
                if (i > 0) {
                    line << ", ";
                }
                line << piece.moves[i];
            }
        }
        if (!draw_wrapped(line.str())) {
            return;
        }
    }
}
}  // namespace

void ImageView::draw_side_panels(const GameSnapshot& snapshot, const BoardMapper& mapper) {
    cv::Mat& frame = canvas.mat();
    const int panel_width = GameConstants::SIDE_PANEL_WIDTH;
    const int panel_height = mapper.display_height();

    draw_color_panel(frame, 0, panel_width, panel_height, Color::White, snapshot.stats.white,
        snapshot.white_player_name, snapshot.white_player_rating);
    draw_color_panel(
        frame,
        mapper.board_offset_x() + snapshot.board_width * mapper.cell_display_size(),
        panel_width,
        panel_height,
        Color::Black,
        snapshot.stats.black,
        snapshot.black_player_name,
        snapshot.black_player_rating
    );
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
