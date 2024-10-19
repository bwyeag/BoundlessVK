#ifndef _BOUNDLESS_WINDOW_HPP_FILE_
#define _BOUNDLESS_WINDOW_HPP_FILE_
// #define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstdint>
#include <deque>
#include <functional>
#include <string>
#include <sstream>
#include <system_error>
#include <variant>
#include <vector>
namespace BL {
struct WindowInitState_t {
    enum Type : uint16_t {
        InitUnvisiable = 0x1,
        FullScreen = 0x2,
        Maximized = 0x4,
        Specified = 0x6,
        SizeMask = 0x6,
        Decorated = 0x8,
        // MousePassThrough = 0x10,
        Resizable = 0x20,
        InitMouseCentered = 0x40,
        UsePrimaryMonitor = 0x80
    };
};
using WindowInitState = WindowInitState_t::Type;
struct WindowInit_t {
    using State = WindowInitState;
    State init_state = State(State::Specified | State::Decorated | State::Resizable |
                       State::UsePrimaryMonitor);
    uint32_t init_size_x, init_size_y;
    uint32_t init_pos_x = (~0u), init_pos_y = (~0u);
    uint32_t max_size_x = GLFW_DONT_CARE, max_size_y = GLFW_DONT_CARE,
             min_size_x = GLFW_DONT_CARE, min_size_y = GLFW_DONT_CARE;
    const char* init_title;
    std::function<bool(GLFWmonitor*)> monitor_choose;
};
struct WindowErrorEnum_t {
    enum Type {
        Success = 0,
        NoMonitor,
        NoSizeSpec,
        WindowCreateFailed,
        EmptyMonitorChooseFunc,
        GLFWInitFailed
    };
};
using WindowErrorEnum = WindowErrorEnum_t::Type;
class WindowErrorCategory : public std::error_category {
   public:
    WindowErrorCategory() {}
    [[nodiscard]]
    const char* name() const noexcept override {
        return "Window error";
    }
    [[nodiscard]]
    std::string message(int ev) const override;
};
[[nodiscard]]
std::error_code make_error_code(WindowErrorEnum e);
}  // namespace BL
namespace std {
template <>
struct is_error_code_enum<BL::WindowErrorEnum> : public true_type {};
}  // namespace std
namespace BL {
struct WindowContext;
struct WindowCallbackEnum_t {
    enum Type {
        AllCallbacks = 0,
        WindowPos,
        WindowSize,
        WindowClose,
        WindowRefresh,
        WindowFocus,
        WindowIconify,
        WindowMaximize,
        WindowContentScale,
        Keyboard,
        CharInput,
        CharMods,
        MouseButton,
        CursorPos,
        CursorEnter,
        Scroll,
        DropFile,
        MaxCallbackEnum  // 最后一个，标记枚举类型的数量
    };
};
using WindowCallbackEnum = WindowCallbackEnum_t::Type;
namespace _internal_windowcb {
using CBEnum = WindowCallbackEnum;
void __glfw_callback_windowpos(GLFWwindow* window, int xpos, int ypos);
void __glfw_callback_windowsize(GLFWwindow* window, int width, int height);
void __glfw_callback_windowclose(GLFWwindow* window);
void __glfw_callback_windowrefresh(GLFWwindow* window);
void __glfw_callback_windowfocus(GLFWwindow* window, int focused);
void __glfw_callback_windowiconify(GLFWwindow* window, int iconified);
void __glfw_callback_windowmaximize(GLFWwindow* window, int maximized);
void __glfw_callback_windowcontentscale(GLFWwindow* window,
                                        float xscale,
                                        float yscale);
void __glfw_callback_mousebutton(GLFWwindow* window,
                                 int button,
                                 int action,
                                 int mods);
void __glfw_callback_cursorpos(GLFWwindow* window, double xpos, double ypos);
void __glfw_callback_cursorenter(GLFWwindow* window, int entered);
void __glfw_callback_scroll(GLFWwindow* window, double xoffset, double yoffset);
void __glfw_callback_keybord(GLFWwindow* window,
                             int key,
                             int scancode,
                             int action,
                             int mods);
void __glfw_callback_charinput(GLFWwindow* window, unsigned int codepoint);
void __glfw_callback_charmods(GLFWwindow* window,
                              unsigned int codepoint,
                              int mods);
void __glfw_callback_drop(GLFWwindow* window,
                          int path_count,
                          const char* paths[]);
template <CBEnum Type>
struct FnType {
};
template <>
struct FnType<CBEnum::WindowPos> {
    using Type = std::function<void(WindowContext*, int, int)>;
    // void callbackfunc(WindowContext* window, int xpos, int ypos);
    Type value;
};
template <>
struct FnType<CBEnum::WindowSize> {
    using Type = std::function<void(WindowContext*, int, int)>;
    // void callbackfunc(WindowContext* window, int width, int height);
    Type value;
};
template <>
struct FnType<CBEnum::WindowClose> {
    using Type = std::function<void(WindowContext*)>;
    // void callbackfunc(WindowContext* window);
        Type value;
};
template <>
struct FnType<CBEnum::WindowRefresh> {
    using Type = std::function<void(WindowContext*)>;
    // void callbackfunc(WindowContext* window);
        Type value;
};
template <>
struct FnType<CBEnum::WindowFocus> {
    using Type = std::function<void(WindowContext*, int)>;
    // void callbackfunc(WindowContext* window, int focused);
        Type value;
};
template <>
struct FnType<CBEnum::WindowIconify> {
    using Type = std::function<void(WindowContext*, int)>;
    // void callbackfunc(WindowContext* window, int iconified);
        Type value;
};
template <>
struct FnType<CBEnum::WindowMaximize> {
    using Type = std::function<void(WindowContext*, int)>;
    // void callbackfunc(WindowContext* window, int maximized);
        Type value;
};
template <>
struct FnType<CBEnum::WindowContentScale> {
    using Type = std::function<void(WindowContext*, float, float)>;
    // void callbackfunc(WindowContext* window, float xscale, float yscale);
        Type value;
};
template <>
struct FnType<CBEnum::Keyboard> {
    using Type = std::function<void(WindowContext*, int, int, int, int)>;
    // void callbackfunc(WindowContext* window, int key, int scancode, int action, int mods);
        Type value;
};
template <>
struct FnType<CBEnum::CharInput> {
    using Type = std::function<void(WindowContext*, unsigned int)>;
    // void callbackfunc(WindowContext* window, unsigned int codepoint);
        Type value;
};
template <>
struct FnType<CBEnum::CharMods> {
    using Type = std::function<void(WindowContext*, unsigned int, int)>;
    // void callbackfunc(WindowContext* window, unsigned int codepoint, int mods);
        Type value;
};
template <>
struct FnType<CBEnum::MouseButton> {
    using Type = std::function<void(WindowContext*, int, int, int)>;
    // void callbackfunc(WindowContext* window, int button, int action, int mods);
        Type value;
};
template <>
struct FnType<CBEnum::CursorPos> {
    using Type = std::function<void(WindowContext*, double, double)>;
    // void callbackfunc(WindowContext* window, double xpos, double ypos);
        Type value;
};
template <>
struct FnType<CBEnum::CursorEnter> {
    using Type = std::function<void(WindowContext*, int)>;
    // void callbackfunc(WindowContext* window, int entered);
        Type value;
};
template <>
struct FnType<CBEnum::Scroll> {
    using Type = std::function<void(WindowContext*, double, double)>;
    // void callbackfunc(WindowContext* window, double xoffset, double yoffset);
        Type value;
};
template <>
struct FnType<CBEnum::DropFile> {
    using Type = std::function<void(WindowContext*, int, const char*[])>;
    // void callbackfunc(WindowContext* window, int path_count, const char* paths[]);
        Type value;
};
template <CBEnum Type>
void callback_set(WindowContext* ctx) {
    static_assert(false,"callback_set(): Wrong CBEnum Type!");
}
// 此处类型的顺序必须与 WindowCallbackEnum 中的枚举顺序相同
using Wrapper = std::variant<std::monostate,
                             FnType<CBEnum::WindowPos>,
                             FnType<CBEnum::WindowSize>,
                             FnType<CBEnum::WindowClose>,
                             FnType<CBEnum::WindowRefresh>,
                             FnType<CBEnum::WindowFocus>,
                             FnType<CBEnum::WindowIconify>,
                             FnType<CBEnum::WindowMaximize>,
                             FnType<CBEnum::WindowContentScale>,
                             FnType<CBEnum::Keyboard>,
                             FnType<CBEnum::CharInput>,
                             FnType<CBEnum::CharMods>,
                             FnType<CBEnum::MouseButton>,
                             FnType<CBEnum::CursorPos>,
                             FnType<CBEnum::CursorEnter>,
                             FnType<CBEnum::Scroll>,
                             FnType<CBEnum::DropFile>>;
}  // namespace _internal_windowcb
template <WindowCallbackEnum Type>
using WindowCallbackFunc = _internal_windowcb::FnType<Type>;
using WindowCallbackWrapper = _internal_windowcb::Wrapper;
struct CallbackNode {
    CallbackNode* next, *rear;
    WindowCallbackWrapper func;
    template<typename T>
    CallbackNode(T&& fn) {
        func = std::forward<T>(fn);
    }
};
using CallbackHandle = CallbackNode*;
struct WindowContext {
    GLFWwindow* pWindow = nullptr;
    GLFWmonitor* pMonitor = nullptr;
    std::string title;
    double current_time = 0.0, delta_time = 0.0;
    std::deque<CallbackNode> callbacks;
    CallbackHandle callback_free = nullptr;
    CallbackHandle
        callback_head[uint32_t(WindowCallbackEnum::MaxCallbackEnum)]{};

    // VkSurfaceKHR surface = VK_NULL_HANDLE;
    // VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    // std::vector<VkImage> swapchainImages;
    // std::vector<VkImageView> swapchainImageViews;
    // VkSwapchainCreateInfoKHR swapchainCreateInfo = {};

    WindowContext() = default;
    WindowContext(const WindowInit_t& pInit, std::error_code& ec) {
        create_window(pInit, ec);
    }
    WindowContext(WindowContext&&) = delete;
    ~WindowContext() {}
    static void initialize(std::error_code& ec);
    static void terminate();
    static void wait_event() { glfwWaitEvents(); }
    static void wait_event(double seconds) { glfwWaitEventsTimeout(seconds); }
    static void poll_event() { glfwPollEvents(); }
    static void post_empty_event() { glfwPostEmptyEvent(); }

    void create_window(const WindowInit_t& pInit, std::error_code& ec) noexcept;
    void destroy() noexcept;
    void update() noexcept;

    void set_window_pos(uint32_t x,uint32_t y) {
        glfwSetWindowPos(pWindow,x,y);
    }
    void get_window_pos(uint32_t& x,uint32_t& y) {
        glfwGetWindowPos(pWindow, &x, &y);
    }
    void iconify_window() {
        glfwIconifyWindow(pWindow);
    }
    void restore_window() {
        glfwRestoreWindow(pWindow);
    }
    void maximize_window() {
        glfwMaximizeWindow(pWindow);
    }
    [[nodiscard]]
    int is_maximized() {
        return glfwGetWindowAttrib(pWindow, GLFW_MAXIMIZED);
    }
    void hide_window() {
        glfwHideWindow(pWindow);
    }
    void show_window() {
        glfwShowWindow(pWindow);
    }
    [[nodiscard]]
    int is_visible() {
        return glfwGetWindowAttrib(pWindow, GLFW_VISIBLE);
    }
    void focus_window() {
        glfwFocusWindow(pWindow);
    }
    [[nodiscard]]
    int is_focused() {
        return glfwGetWindowAttrib(pWindow, GLFW_FOCUSED);
    }
    void request_attention() {
        glfwRequestWindowAttention(pWindow);
    }
    void set_size_limits(uint32_t min_x,uint32_t min_y,uint32_t max_x,uint32_t max_y) {
    glfwSetWindowSizeLimits(pWindow, min_x, min_y,
                            max_x, max_y);
    }
    void clear_size_limits() {
    glfwSetWindowSizeLimits(pWindow, GLFW_DONT_CARE, GLFW_DONT_CARE,
                            GLFW_DONT_CARE, GLFW_DONT_CARE);
    }
    void set_size_aspect_ratio(uint32_t w,uint32_t h)
{glfwSetWindowAspectRatio(pWindow, w, h);}

    template <WindowCallbackEnum Type>
    [[nodiscard]]
    CallbackHandle insert_callback(WindowCallbackFunc<Type>::Type&& func);
    void erase_callback(CallbackHandle handle);
};
template <WindowCallbackEnum Type>
[[nodiscard]]
CallbackHandle WindowContext::insert_callback(
    WindowCallbackFunc<Type>::Type&& func) {
    static_assert(Type == WindowCallbackEnum::AllCallbacks ||
                      Type == WindowCallbackEnum::MaxCallbackEnum,
                  "insert_callback(): not a callback type!");
    using FuncType = WindowCallbackFunc<Type>::Type;
    CallbackHandle result;
    if (callback_free) {
        result = callback_free;
        callback_free = callback_free->next;
        result->func = WindowCallbackFunc<Type>{std::forward<FuncType>(func)};
    } else {
        callbacks.emplace_back(WindowCallbackFunc<Type>{std::forward<FuncType>(func)});
        result = &callbacks.back();
    }
    if (!callback_head[uint32_t(Type)]) {
        callback_set<Type>(this);
        result->next = result;
        result->rear = result;
    } else {
        CallbackHandle left = callback_head[uint32_t(Type)]->rear;
        CallbackHandle right = callback_head[uint32_t(Type)];
        left->next = result;
        result->next = right;
        right->rear = result;
        result->rear = left;
    }
    callback_head[uint32_t(Type)] = result;
    return result;
}
struct FPSTitle {
    uint32_t delta_frame = 0u;
    std::stringstream str_builder;
    double last_time;

    void operator()(WindowContext& ctx);
};
namespace _internal_windowcb {
template <>
void callback_set<CBEnum::WindowPos>(WindowContext* ctx) {
        glfwSetWindowPosCallback(ctx->pWindow, __glfw_callback_windowpos);
}
template<>
void callback_set<CBEnum::WindowSize>(WindowContext* ctx) {
    glfwSetWindowSizeCallback(ctx->pWindow, __glfw_callback_windowsize);
}
template<>
void callback_set<CBEnum::WindowClose>(WindowContext* ctx) {
    glfwSetWindowCloseCallback(ctx->pWindow, __glfw_callback_windowclose);
}
template<>
void callback_set<CBEnum::WindowRefresh>(WindowContext* ctx) {
    glfwSetWindowRefreshCallback(ctx->pWindow, __glfw_callback_windowrefresh);
}
template<>
void callback_set<CBEnum::WindowFocus>(WindowContext* ctx) {
    glfwSetWindowFocusCallback(ctx->pWindow, __glfw_callback_windowfocus);
}
template<>
void callback_set<CBEnum::WindowIconify>(WindowContext* ctx) {
    glfwSetWindowIconifyCallback(ctx->pWindow,__glfw_callback_windowiconify);
}
template<>
void callback_set<CBEnum::WindowMaximize>(WindowContext* ctx) {
    glfwSetWindowMaximizeCallback(ctx->pWindow,__glfw_callback_windowmaximize);
}
template<>
void callback_set<CBEnum::WindowContentScale>(WindowContext* ctx) {
    glfwSetWindowContentScaleCallback(ctx->pWindow,__glfw_callback_windowcontentscale);
}
template<>
void callback_set<CBEnum::Keyboard>(WindowContext* ctx) {
    glfwSetKeyCallback(ctx->pWindow, __glfw_callback_keybord);
}
template<>
void callback_set<CBEnum::CharInput>(WindowContext* ctx) {
    glfwSetCharCallback(ctx->pWindow, __glfw_callback_charinput);
}
template<>
void callback_set<CBEnum::CharMods>(WindowContext* ctx) {
    glfwSetCharModsCallback(ctx->pWindow, __glfw_callback_charmods);
}
template<>
void callback_set<CBEnum::MouseButton>(WindowContext* ctx) {
    glfwSetMouseButtonCallback(ctx->pWindow, __glfw_callback_mousebutton);
}
template<>
void callback_set<CBEnum::CursorPos>(WindowContext* ctx) {
    glfwSetCursorPosCallback(ctx->pWindow, __glfw_callback_cursorpos);
}
template<>
void callback_set<CBEnum::CursorEnter>(WindowContext* ctx) {
    glfwSetCursorEnterCallback(ctx->pWindow, __glfw_callback_cursorenter);
}
template<>
void callback_set<CBEnum::Scroll>(WindowContext* ctx) {
    glfwSetScrollCallback(ctx->pWindow, __glfw_callback_scroll);
}
template<>
void callback_set<CBEnum::DropFile>(WindowContext* ctx) {
    glfwSetDropCallback(ctx->pWindow, __glfw_callback_drop);
}
}  // namespace _internal_windowcb
}  // namespace BL
#endif  //!_BOUNDLESS_WINDOW_HPP_FILE_