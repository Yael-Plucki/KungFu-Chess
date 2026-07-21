#include "WebSocketClient.hpp"

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <stdexcept>

struct WebSocketClient::Impl {
    ix::WebSocket socket;
};

WebSocketClient::WebSocketClient() : impl_(new Impl()) {}

WebSocketClient::~WebSocketClient() {
    close();
    delete impl_;
    impl_ = nullptr;
}

void WebSocketClient::connect(const std::string& url) {
    if (connected_ || impl_ == nullptr) {
        return;
    }

    if (!ix::initNetSystem()) {
        throw std::runtime_error("Failed to initialize network system");
    }

    impl_->socket.setUrl(url);
    impl_->socket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            std::lock_guard<std::mutex> lock(inbound_mutex_);
            inbound_queue_.push(msg->str);
        }
    });

    const ix::WebSocketInitResult result = impl_->socket.connect(5);
    if (!result.success) {
        ix::uninitNetSystem();
        throw std::runtime_error("WebSocket connect failed: " + result.errorStr);
    }

    impl_->socket.start();
    connected_ = true;
}

void WebSocketClient::close() {
    if (!connected_ || impl_ == nullptr) {
        return;
    }

    impl_->socket.stop();
    connected_ = false;
    ix::uninitNetSystem();
}

bool WebSocketClient::is_connected() const {
    return connected_;
}

void WebSocketClient::send(const std::string& message) {
    if (!connected_ || impl_ == nullptr) {
        return;
    }

    impl_->socket.send(message);
}

bool WebSocketClient::pop_message(std::string& message) {
    std::lock_guard<std::mutex> lock(inbound_mutex_);
    if (inbound_queue_.empty()) {
        return false;
    }

    message = std::move(inbound_queue_.front());
    inbound_queue_.pop();
    return true;
}
