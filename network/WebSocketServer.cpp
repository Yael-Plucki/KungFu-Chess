#include "WebSocketServer.hpp"

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocketServer.h>
#include <memory>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <utility>

struct WebSocketServer::Impl {
    ix::WebSocketServer server;
    std::mutex connections_mutex;
    std::unordered_map<std::string, std::weak_ptr<ix::WebSocket>> connections;
    std::function<void(const std::string& connection_id)> on_connect;

    explicit Impl(int port, std::string host)
        : server(port, std::move(host)) {}
};

WebSocketServer::WebSocketServer(int port, const std::string& host)
    : port_(port), host_(host), impl_(new Impl(port, host)) {}

WebSocketServer::~WebSocketServer() {
    stop();
    delete impl_;
    impl_ = nullptr;
}

void WebSocketServer::start() {
    if (running_ || impl_ == nullptr) {
        return;
    }

    if (!ix::initNetSystem()) {
        throw std::runtime_error("Failed to initialize network system");
    }

    impl_->server.setOnConnectionCallback(
        [this](std::weak_ptr<ix::WebSocket> weak_socket,
               std::shared_ptr<ix::ConnectionState> connection_state) {
            const std::shared_ptr<ix::WebSocket> socket = weak_socket.lock();
            if (socket == nullptr || connection_state == nullptr) {
                return;
            }

            {
                std::lock_guard<std::mutex> lock(impl_->connections_mutex);
                impl_->connections[connection_state->getId()] = socket;
            }

            if (impl_->on_connect) {
                impl_->on_connect(connection_state->getId());
            }

            socket->setOnMessageCallback(
                [this, connection_state](const ix::WebSocketMessagePtr& msg) {
                    if (msg->type != ix::WebSocketMessageType::Message || connection_state == nullptr) {
                        return;
                    }
                    enqueue_inbound(connection_state->getId(), msg->str);
                });
        });

    if (!impl_->server.listenAndStart()) {
        ix::uninitNetSystem();
        throw std::runtime_error("WebSocket listen failed on port " + std::to_string(port_));
    }

    running_ = true;
}

void WebSocketServer::stop() {
    if (!running_ || impl_ == nullptr) {
        return;
    }

    impl_->server.stop();
    running_ = false;
    ix::uninitNetSystem();
}

bool WebSocketServer::is_running() const {
    return running_;
}

void WebSocketServer::send_to(const std::string& connection_id, const std::string& message) {
    if (!running_ || impl_ == nullptr) {
        return;
    }

    std::shared_ptr<ix::WebSocket> socket;
    {
        std::lock_guard<std::mutex> lock(impl_->connections_mutex);
        const auto it = impl_->connections.find(connection_id);
        if (it != impl_->connections.end()) {
            socket = it->second.lock();
        }
    }

    if (socket != nullptr) {
        socket->send(message);
    }
}

void WebSocketServer::set_on_connect(std::function<void(const std::string& connection_id)> callback) {
    if (impl_ != nullptr) {
        impl_->on_connect = std::move(callback);
    }
}

void WebSocketServer::broadcast(const std::string& message) {
    if (!running_ || impl_ == nullptr) {
        return;
    }

    const std::set<std::shared_ptr<ix::WebSocket>> clients = impl_->server.getClients();
    for (const std::shared_ptr<ix::WebSocket>& client : clients) {
        client->send(message);
    }
}

void WebSocketServer::enqueue_inbound(const std::string& connection_id, const std::string& message) {
    std::lock_guard<std::mutex> lock(inbound_mutex_);
    inbound_queue_.push(Protocol::InboundFrame{connection_id, message});
}

bool WebSocketServer::pop_inbound(Protocol::InboundFrame& frame) {
    std::lock_guard<std::mutex> lock(inbound_mutex_);
    if (inbound_queue_.empty()) {
        return false;
    }

    frame = std::move(inbound_queue_.front());
    inbound_queue_.pop();
    return true;
}
