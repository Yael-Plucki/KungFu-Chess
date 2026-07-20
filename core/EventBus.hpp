#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

class EventBus {
public:
    using SubscriptionId = std::size_t;

    template<typename Event>
    SubscriptionId subscribe(std::function<void(const Event&)> handler) {
        auto& handlers = handlers_for<Event>();
        const SubscriptionId id = next_id_++;
        handlers.push_back({id, std::move(handler)});
        return id;
    }

    template<typename Event>
    void unsubscribe(SubscriptionId id) {
        auto& handlers = handlers_for<Event>();
        handlers.erase(
            std::remove_if(
                handlers.begin(),
                handlers.end(),
                [id](const HandlerEntry<Event>& entry) { return entry.id == id; }),
            handlers.end());
    }

    template<typename Event>
    void publish(const Event& event) {
        const auto handlers = handlers_for<Event>();
        for (const HandlerEntry<Event>& entry : handlers) {
            entry.handler(event);
        }
    }

    void clear() {
        handlers_.clear();
    }

private:
    template<typename Event>
    struct HandlerEntry {
        SubscriptionId id;
        std::function<void(const Event&)> handler;
    };

    template<typename Event>
    std::vector<HandlerEntry<Event>>& handlers_for() {
        const std::type_index key(typeid(Event));
        auto it = handlers_.find(key);
        if (it == handlers_.end()) {
            auto storage = std::make_shared<std::vector<HandlerEntry<Event>>>();
            it = handlers_.emplace(key, storage).first;
        }
        return *std::static_pointer_cast<std::vector<HandlerEntry<Event>>>(it->second);
    }

    std::unordered_map<std::type_index, std::shared_ptr<void>> handlers_;
    SubscriptionId next_id_ = 1;
};
