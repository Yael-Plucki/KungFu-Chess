#include "../tests/test_framework.hpp"
#include "../input/BoardMapper.hpp"

static void test_maps_pixel_to_cell() {
    BoardMapper mapper(3, 3);

    auto cell = mapper.pixel_to_cell(210, 30);
    EXPECT_TRUE(cell.has_value());
    EXPECT_EQ(cell.value(), Position(0, 0));

    cell = mapper.pixel_to_cell(270, 30);
    EXPECT_TRUE(cell.has_value());
    EXPECT_EQ(cell.value(), Position(0, 1));

    cell = mapper.pixel_to_cell(210, 90);
    EXPECT_TRUE(cell.has_value());
    EXPECT_EQ(cell.value(), Position(1, 0));
}

static void test_rejects_outside_board() {
    BoardMapper mapper(3, 3);

    EXPECT_FALSE(mapper.pixel_to_cell(30, 30).has_value());
    EXPECT_FALSE(mapper.pixel_to_cell(540, 30).has_value());
}

static void test_uses_board_dimensions_not_hardcoded_8x8() {
    BoardMapper mapper(4, 5);

    auto cell = mapper.pixel_to_cell(450, 210);
    EXPECT_TRUE(cell.has_value());
    EXPECT_EQ(cell.value(), Position(3, 4));

    EXPECT_FALSE(mapper.pixel_to_cell(510, 210).has_value());
}

static void test_cell_center() {
    BoardMapper mapper(3, 3);

    int x = 0;
    int y = 0;
    mapper.cell_center(Position(0, 1), x, y);
    EXPECT_EQ(x, 270);
    EXPECT_EQ(y, 30);
}

static void test_motion_center_interpolates() {
    BoardMapper mapper(3, 3);

    ActiveMotionInfo motion{};
    motion.active = true;
    motion.source = Position(0, 1);
    motion.destination = Position(2, 1);
    motion.start_time = 0;
    motion.duration = 2000;

    int x = 0;
    int y = 0;
    mapper.motion_center(motion, 1000, x, y);
    EXPECT_EQ(x, 270);
    EXPECT_EQ(y, 90);
}

int main() {
    RUN_TEST(test_maps_pixel_to_cell);
    RUN_TEST(test_rejects_outside_board);
    RUN_TEST(test_uses_board_dimensions_not_hardcoded_8x8);
    RUN_TEST(test_cell_center);
    RUN_TEST(test_motion_center_interpolates);
    return 0;
}
