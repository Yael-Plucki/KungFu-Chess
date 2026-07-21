#pragma once

#include <mutex>
#include <queue>
#include <string>

class WebSocketClient {
public:
    WebSocketClient();
    ~WebSocketClient();

    WebSocketClient(const WebSocketClient&) = delete;
    WebSocketClient& operator=(const WebSocketClient&) = delete;

    void connect(const std::string& url);
    void close();
    bool is_connected() const;

    void send(const std::string& message);
    bool pop_message(std::string& message);

private:
    bool connected_ = false;
    std::mutex inbound_mutex_;
    std::queue<std::string> inbound_queue_;

    struct Impl;
    Impl* impl_ = nullptr;
};
