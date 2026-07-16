#include "../tests/test_framework.hpp"
#include "../view/animation/AnimationConfig.hpp"
#include "../view/animation/PieceAnimator.hpp"

static void test_move_transitions_to_long_rest_when_motion_ends() {
    PieceAnimator animator(1, "KW");

    SnapshotPiece moving{};
    moving.id = 1;
    moving.color = Color::White;
    moving.kind = Kind::King;
    moving.state = State::Moving;
    moving.is_jump_motion = false;

    animator.sync(moving);
    EXPECT_EQ(animator.current_state(), "move");

    SnapshotPiece idle = moving;
    idle.state = State::Idle;
    animator.sync(idle);
    EXPECT_EQ(animator.current_state(), "long_rest");
}

static void test_long_rest_returns_to_idle_after_clip() {
    AnimationConfigRegistry configs("CTD26/assets (2)/assets/images/pieces");
    PieceAnimator animator(1, "KW");

    SnapshotPiece moving{};
    moving.state = State::Moving;
    moving.is_jump_motion = false;
    animator.sync(moving);

    SnapshotPiece idle = moving;
    idle.state = State::Idle;
    animator.sync(idle);
    EXPECT_EQ(animator.current_state(), "long_rest");

    const AnimationConfig& rest_config = configs.config("KW", "long_rest");
    const long long rest_duration_ms =
        (rest_config.frame_count * 1000LL) / rest_config.frames_per_sec;
    animator.update(static_cast<int>(rest_duration_ms), configs);
    EXPECT_EQ(animator.current_state(), "idle");
}

static void test_jump_transitions_to_short_rest_when_motion_ends() {
    PieceAnimator animator(2, "KW");

    SnapshotPiece jumping{};
    jumping.state = State::Moving;
    jumping.is_jump_motion = true;

    animator.sync(jumping);
    EXPECT_EQ(animator.current_state(), "jump");

    SnapshotPiece idle = jumping;
    idle.state = State::Idle;
    animator.sync(idle);
    EXPECT_EQ(animator.current_state(), "short_rest");
}

int main() {
    RUN_TEST(test_move_transitions_to_long_rest_when_motion_ends);
    RUN_TEST(test_long_rest_returns_to_idle_after_clip);
    RUN_TEST(test_jump_transitions_to_short_rest_when_motion_ends);
    return 0;
}
