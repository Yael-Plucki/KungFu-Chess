#include "Board_Mapper.hpp"

pair<int, int> Board_Mapper::position(int x, int y, Board grid){
    x=x/PIXEL_X;
    y=y/PIXEL_Y;
    if(x<0 || y<0 || x>grid.size() || y>grid[0].size())
        return null;
    return make_pair(x, y);
}