#define PIXEL_X 100
#define PIXEL_Y 100

class Board_mapper{
    std::optional<Position> pixel_to_cell(int x, int y) const;
}