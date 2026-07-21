#pragma once

#include "network/RemoteGameSession.hpp"
#include "input/BoardMapper.hpp"
#include "model/Position.hpp"
#include <optional>

class RemoteController {
public:
    RemoteController(RemoteGameSession& session, const BoardMapper& mapper);

    void click(int x, int y);
    void jump(int x, int y);
    std::optional<Position> get_selected_cell() const;

private:
    RemoteGameSession& session_;
    const BoardMapper& board_mapper_;
    std::optional<Position> selected_cell_;
};
