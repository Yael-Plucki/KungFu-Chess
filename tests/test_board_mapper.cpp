#include "../tests/test_framework.hpp"
#include "../input/BoardMapper.hpp"

static void test_maps_pixel_to_cell() {
    BoardMapper mapper(3, 3);

    auto cell = mapper.pixel_to_cell(50, 50);
    EXPECT_TRUE(cell.has_value());
    EXPECT_EQ(cell.value(), Position(0, 0));

    cell = mapper.pixel_to_cell(150, 50);
    EXPECT_TRUE(cell.has_value());
    EXPECT_EQ(cell.value(), Position(0, 1));

    cell = mapper.pixel_to_cell(50, 150);
    EXPECT_TRUE(cell.has_value());
    EXPECT_EQ(cell.value(), Position(1, 0));
}

static void test_rejects_outside_board() {
    BoardMapper mapper(3, 3);

    EXPECT_FALSE(mapper.pixel_to_cell(350, 50).has_value());
    EXPECT_FALSE(mapper.pixel_to_cell(50, 350).has_value());
}

static void test_uses_board_dimensions_not_hardcoded_8x8() {
    BoardMapper mapper(4, 5);

    auto cell = mapper.pixel_to_cell(450, 350);
    EXPECT_TRUE(cell.has_value());
    EXPECT_EQ(cell.value(), Position(3, 4));

    EXPECT_FALSE(mapper.pixel_to_cell(550, 350).has_value());
}

int main() {
    RUN_TEST(test_maps_pixel_to_cell);
    RUN_TEST(test_rejects_outside_board);
    RUN_TEST(test_uses_board_dimensions_not_hardcoded_8x8);
    return 0;
}
