#ifndef _BOUNDLESS_CALLBACK_CXX_FILE_
#define _BOUNDLESS_CALLBACK_CXX_FILE_
#include <functional>
#include <map>
#include <utility>
#include "log.hpp"
namespace BL {
template<typename Res, typename... ArgTypes>
class CallbackList<Res(ArgTypes...)> {
    std::map<int, std::function<Res(ArgTypes...)>> callbacks;

   public:
    CallbackList() = default;
    CallbackList(const CallbackList& other) = default;
    CallbackList(CallbackList&& other) {
        callbacks = std::move(other.callbacks);
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
    int insert(std::function<Res(ArgTypes...)> fn) {
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