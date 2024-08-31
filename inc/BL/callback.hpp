#ifndef _BOUNDLESS_CALLBACK_CXX_FILE_
#define _BOUNDLESS_CALLBACK_CXX_FILE_
#include <functional>
#include <map>
#include <utility>
#include "log.hpp"
namespace BL {
class CallbackListVoid {
    std::map<int, std::function<void()>> callbacks;

   public:
    CallbackListVoid() = default;
    CallbackListVoid(const CallbackListVoid& other) = delete;
    CallbackListVoid(CallbackListVoid&& other) {
        callbacks = std::move(other.callbacks);
    }
    CallbackListVoid& operator=(CallbackListVoid&& other) {
        callbacks = std::move(other.callbacks);
        return *this;
    }
    void iterate() {
        try {
            for (auto it = callbacks.begin(); it != callbacks.end(); ++it) {
                (it->second)();
            }
        } catch (const std::exception& e) {
            print_error("CallbackList", "expection raised! what:", e.what());
        }
    }
    int insert(std::function<void()> fn) {
        static int baseID = 1;
        callbacks.insert({baseID, fn});
        return baseID++;
    }
    void erase(int id) {
        if (id <= 0)
            return;
        auto it = callbacks.find(id);
        if (it == callbacks.end()) {
            print_error("CallbackList", "ID erase not find:", id);
            return;
        }
        callbacks.erase(it);
    }
};
template<typename... ArgTypes>
class CallbackList {
    std::map<int, std::function<void(ArgTypes...)>> callbacks;

   public:
    CallbackList() = default;
    CallbackList(const CallbackList& other) = delete;
    CallbackList(CallbackList&& other) {
        callbacks = std::move(other.callbacks);
    }
    CallbackList& operator=(CallbackList&& other) {
        callbacks = std::move(other.callbacks);
        return *this;
    }
    void iterate(ArgTypes... args) {
        try {
            for (auto it = callbacks.begin(); it != callbacks.end(); ++it) {
                (it->second)((args)...);
            }
        } catch (const std::exception& e) {
            print_error("CallbackList", "expection raised! what:", e.what());
        }
    }
    int insert(std::function<void(ArgTypes...)> fn) {
        static int baseID = 1;
        callbacks.insert({baseID, fn});
        return baseID++;
    }
    void erase(int id) {
        if (id <= 0)
            return;
        auto it = callbacks.find(id);
        if (it == callbacks.end()) {
            print_error("CallbackList", "ID erase not find:", id);
            return;
        }
        callbacks.erase(it);
    }
};
}  // namespace  BL
#endif  // ! _BOUNDLESS_CALLBACK_CXX_FILE_