#pragma once

#include <functional>
#include <mutex>
#include <queue>
#include <string>

#include "Protocol.hpp"

class WebSocketServer {
public:
    WebSocketServer(int port, const std::string& host = "0.0.0.0");
    ~WebSocketServer();

    WebSocketServer(const WebSocketServer&) = delete;
    WebSocketServer& operator=(const WebSocketServer&) = delete;

    void start();
    void stop();
    bool is_running() const;

    void broadcast(const std::string& message);
    void send_to(const std::string& connection_id, const std::string& message);
    void enqueue_inbound(const std::string& connection_id, const std::string& message);
    bool pop_inbound(Protocol::InboundFrame& frame);

private:
    int port_;
    std::string host_;
    bool running_ = false;

    std::mutex inbound_mutex_;
    std::queue<Protocol::InboundFrame> inbound_queue_;

    struct Impl;
    Impl* impl_ = nullptr;
};
