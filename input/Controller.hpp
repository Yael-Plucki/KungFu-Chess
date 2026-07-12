#pragma once
class Controller{
    public:
    void click(int x, int y);
private:
    std::optional<Position> selected_cell;}