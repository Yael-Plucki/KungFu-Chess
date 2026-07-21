#pragma once

#include <functional>
#include <mutex>
#include <queue>
#include <string>

#include "network/Protocol.hpp"

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
    void set_on_connect(std::function<void(const std::string& connection_id)> callback);
    void enqueue_inbound(const std::string& connection_id, const std::string& message);
    void enqueue_disconnect(const std::string& connection_id);
    bool pop_inbound(Protocol::InboundFrame& frame);
    bool pop_disconnect(std::string& connection_id);

private:
    int port_;
    std::string host_;
    bool running_ = false;

    std::mutex inbound_mutex_;
    std::queue<Protocol::InboundFrame> inbound_queue_;
    std::queue<std::string> disconnect_queue_;

    struct Impl;
    Impl* impl_ = nullptr;
};
