#pragma once

#include "network/ClientBoardSync.hpp"
#include "network/RemoteGameSession.hpp"
#include "input/BoardMapper.hpp"
#include "model/Position.hpp"
#include <optional>

class RemoteController {
public:
    RemoteController(RemoteGameSession& session, ClientBoardSync& board_sync, const BoardMapper& mapper);

    void click(int x, int y);
    void jump(int x, int y);
    std::optional<Position> get_selected_cell() const;

private:
    RemoteGameSession& session_;
    ClientBoardSync& board_sync_;
    const BoardMapper& board_mapper_;
    std::optional<Position> selected_cell_;
    EventBus::SubscriptionId move_rejected_subscription_ = 0;
};
