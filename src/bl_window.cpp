#include "bl_window.hpp"

namespace BL {
std::string WindowErrorCategory::message(int ev) const {
    using Enum = WindowErrorEnum;
    switch (static_cast<Enum>(ev)) {
        case Enum::Success:
            return "no error";
        case Enum::NoMonitor:
            return "system no monitor";
        case Enum::NoSizeSpec:
            return "WindowInit_t no size spec";
        case Enum::WindowCreateFailed:
            return "failed to create window";
        case Enum::GLFWInitFailed:
            return "failed to init GLFW";
        default:
            return "unknown error";
    }
}
static WindowErrorCategory category;
[[nodiscard]]
std::error_code make_error_code(WindowErrorEnum e) {
    return {static_cast<int>(e), category};
}
namespace _internal_windowcb {
void __glfw_callback_windowpos(GLFWwindow* window, int xpos, int ypos) {
    WindowContext* ctx = (WindowContext*)glfwGetWindowUserPointer(window);
    CallbackHandle p = ctx->callback_head[CBEnum::WindowPos], end = p;
    while (p) {
        auto* fn = std::get_if<FnType<CBEnum::WindowPos>>(&p->func);
        if (fn)
            fn->value(ctx, xpos, ypos);
        p = p->next;
        if (p == end) break;
    }
}

void __glfw_callback_windowsize(GLFWwindow* window, int width, int height) {
    WindowContext* ctx = (WindowContext*)glfwGetWindowUserPointer(window);
    CallbackHandle p = ctx->callback_head[CBEnum::WindowSize], end = p;
    while (p) {
        auto* fn = std::get_if<FnType<CBEnum::WindowSize>>(&p->func);
        if (fn)
            fn->value(ctx, width, height);
        p = p->next;
        if (p == end) break;
    }
}

void __glfw_callback_windowclose(GLFWwindow* window) {
    WindowContext* ctx = (WindowContext*)glfwGetWindowUserPointer(window);
    CallbackHandle p = ctx->callback_head[CBEnum::WindowClose], end = p;
    while (p) {
        auto* fn = std::get_if<FnType<CBEnum::WindowClose>>(&p->func);
        if (fn)
            fn->value(ctx);
        p = p->next;
        if (p == end) break;
    }
}

void __glfw_callback_windowrefresh(GLFWwindow* window) {
    WindowContext* ctx = (WindowContext*)glfwGetWindowUserPointer(window);
    CallbackHandle p = ctx->callback_head[CBEnum::WindowRefresh], end = p;
    while (p) {
        auto* fn = std::get_if<FnType<CBEnum::WindowRefresh>>(&p->func);
        if (fn)
            fn->value(ctx);
        p = p->next;
        if (p == end) break;
    }
}

void __glfw_callback_windowfocus(GLFWwindow* window, int focused) {
    WindowContext* ctx = (WindowContext*)glfwGetWindowUserPointer(window);
    CallbackHandle p = ctx->callback_head[CBEnum::WindowFocus], end = p;
    while (p) {
        auto* fn = std::get_if<FnType<CBEnum::WindowFocus>>(&p->func);
        if (fn)
            fn->value(ctx, focused);
        p = p->next;
        if (p == end) break;
    }
}

void __glfw_callback_windowiconify(GLFWwindow* window, int iconified) {
    WindowContext* ctx = (WindowContext*)glfwGetWindowUserPointer(window);
    CallbackHandle p = ctx->callback_head[CBEnum::WindowIconify], end = p;
    while (p) {
        auto* fn = std::get_if<FnType<CBEnum::WindowIconify>>(&p->func);
        if (fn)
            fn->value(ctx, iconified);
        p = p->next;
        if (p == end) break;
    }
}

void __glfw_callback_windowmaximize(GLFWwindow* window, int maximized) {
    WindowContext* ctx = (WindowContext*)glfwGetWindowUserPointer(window);
    CallbackHandle p = ctx->callback_head[CBEnum::WindowMaximize], end = p;
    while (p) {
        auto* fn = std::get_if<FnType<CBEnum::WindowMaximize>>(&p->func);
        if (fn)
            fn->value(ctx, maximized);
        p = p->next;
        if (p == end) break;
    }
}

void __glfw_callback_windowcontentscale(GLFWwindow* window,
                                        float xscale,
                                        float yscale) {
    WindowContext* ctx = (WindowContext*)glfwGetWindowUserPointer(window);
    CallbackHandle p = ctx->callback_head[CBEnum::WindowContentScale], end = p;
    while (p) {
        auto* fn = std::get_if<FnType<CBEnum::WindowContentScale>>(&p->func);
        if (fn)
            fn->value(ctx, xscale, yscale);
        p = p->next;
        if (p == end) break;
    }
}

void __glfw_callback_mousebutton(GLFWwindow* window,
                                 int button,
                                 int action,
                                 int mods) {
    WindowContext* ctx = (WindowContext*)glfwGetWindowUserPointer(window);
    CallbackHandle p = ctx->callback_head[CBEnum::MouseButton], end = p;
    while (p) {
        auto* fn = std::get_if<FnType<CBEnum::MouseButton>>(&p->func);
        if (fn)
            fn->value(ctx, button, action, mods);
        p = p->next;
        if (p == end) break;
    }
}

void __glfw_callback_cursorpos(GLFWwindow* window, double xpos, double ypos) {
    WindowContext* ctx = (WindowContext*)glfwGetWindowUserPointer(window);
    CallbackHandle p = ctx->callback_head[CBEnum::CursorPos], end = p;
    while (p) {
        auto* fn = std::get_if<FnType<CBEnum::CursorPos>>(&p->func);
        if (fn)
            fn->value(ctx, xpos, ypos);
        p = p->next;
        if (p == end) break;
    }
}

void __glfw_callback_cursorenter(GLFWwindow* window, int entered) {
    WindowContext* ctx = (WindowContext*)glfwGetWindowUserPointer(window);
    CallbackHandle p = ctx->callback_head[CBEnum::CursorEnter], end = p;
    while (p) {
        auto* fn = std::get_if<FnType<CBEnum::CursorEnter>>(&p->func);
        if (fn)
            fn->value(ctx, entered);
        p = p->next;
        if (p == end) break;
    }
}

void __glfw_callback_scroll(GLFWwindow* window,
                            double xoffset,
                            double yoffset) {
    WindowContext* ctx = (WindowContext*)glfwGetWindowUserPointer(window);
    CallbackHandle p = ctx->callback_head[CBEnum::Scroll], end = p;
    while (p) {
        auto* fn = std::get_if<FnType<CBEnum::Scroll>>(&p->func);
        if (fn)
            fn->value(ctx, xoffset, yoffset);
        p = p->next;
        if (p == end) break;
    }
}

void __glfw_callback_keybord(GLFWwindow* window,
                             int key,
                             int scancode,
                             int action,
                             int mods) {
    WindowContext* ctx = (WindowContext*)glfwGetWindowUserPointer(window);
    CallbackHandle p = ctx->callback_head[CBEnum::Keyboard], end = p;
    while (p) {
        auto* fn = std::get_if<FnType<CBEnum::Keyboard>>(&p->func);
        if (fn)
            fn->value(ctx, key, scancode, action, mods);
        p = p->next;
        if (p == end) break;
    }
}

void __glfw_callback_charinput(GLFWwindow* window, unsigned int codepoint) {
    WindowContext* ctx = (WindowContext*)glfwGetWindowUserPointer(window);
    CallbackHandle p = ctx->callback_head[CBEnum::CharInput], end = p;
    while (p) {
        auto* fn = std::get_if<FnType<CBEnum::CharInput>>(&p->func);
        if (fn)
            fn->value(ctx, codepoint);
        p = p->next;
        if (p == end) break;
    }
}

void __glfw_callback_charmods(GLFWwindow* window,
                              unsigned int codepoint,
                              int mods) {
    WindowContext* ctx = (WindowContext*)glfwGetWindowUserPointer(window);
    CallbackHandle p = ctx->callback_head[CBEnum::CharMods], end = p;
    while (p) {
        auto* fn = std::get_if<FnType<CBEnum::CharMods>>(&p->func);
        if (fn)
            fn->value(ctx, codepoint, mods);
        p = p->next;
        if (p == end) break;
    }
}

void __glfw_callback_drop(GLFWwindow* window,
                          int path_count,
                          const char* paths[]) {
    WindowContext* ctx = (WindowContext*)glfwGetWindowUserPointer(window);
    CallbackHandle p = ctx->callback_head[CBEnum::DropFile], end = p;
    while (p) {
        auto* fn = std::get_if<FnType<CBEnum::DropFile>>(&p->func);
        if (fn)
            fn->value(ctx, path_count, paths);
        p = p->next;
        if (p == end) break;
    }
}
void __glfw_error_callback(int error_code, const char* description) {
    // todo
}
}  // namespace _internal_windowcb

void WindowContext::initialize(std::error_code& ec) {
    glfwSetErrorCallback(_internal_windowcb::__glfw_error_callback);
    if (!glfwInit()) {
        ec = make_error_code(WindowErrorEnum::GLFWInitFailed);
    }
}
void WindowContext::terminate() {
    glfwTerminate();
}
void WindowContext::create_window(const WindowInit_t& init,
                                  std::error_code& ec) noexcept {
    using State = WindowInitState;
    using Error = WindowErrorEnum;
    if (init.init_state & State::UsePrimaryMonitor) {
        pMonitor = glfwGetPrimaryMonitor();
    } else {
        int count;
        GLFWmonitor** pMonitors = glfwGetMonitors(&count);
        if (!pMonitors) {
            ec = make_error_code(Error::NoMonitor);
            return;
        }
        if (!init.monitor_choose) {
            ec = make_error_code(Error::EmptyMonitorChooseFunc);
            return;
        }
        for (int i = 0; i < count; i++) {
            if (init.monitor_choose(pMonitors[i])) {
                pMonitor = pMonitors[i];
                break;
            }
        }
    }
    if (!pMonitor) {
        ec = make_error_code(Error::NoMonitor);
        return;
    }
    title = init.init_title;
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE,
                   static_cast<bool>(init.init_state & State::Resizable));
    glfwWindowHint(GLFW_DECORATED,
                   static_cast<bool>(init.init_state & State::Decorated));
    glfwWindowHint(
        GLFW_CENTER_CURSOR,
        static_cast<bool>(init.init_state & State::InitMouseCentered));
    glfwWindowHint(GLFW_VISIBLE,
                   !static_cast<bool>(init.init_state & State::InitUnvisiable));
    State size_state = State(init.init_state & State::SizeMask);
    const GLFWvidmode* pMode = glfwGetVideoMode(pMonitor);
    if (size_state == State::FullScreen) {
        pWindow = glfwCreateWindow(pMode->width, pMode->height, title.c_str(),
                                   pMonitor, nullptr);
    } else if (size_state == State::Maximized) {
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
        pWindow = glfwCreateWindow(pMode->width, pMode->height, title.c_str(),
                                   nullptr, nullptr);
    } else if (size_state == State::Specified) {
        pWindow = glfwCreateWindow(init.init_size_x, init.init_size_y,
                                   title.c_str(), nullptr, nullptr);
    } else {
        pMonitor = nullptr;
        title.clear();
        ec = make_error_code(Error::NoSizeSpec);
        return;
    }
    if (!pWindow) {
        pMonitor = nullptr;
        title.clear();
        ec = make_error_code(Error::WindowCreateFailed);
        return;
    }
    if (init.init_pos_x != (~0u)&&init.init_pos_y != (~0u))
        glfwSetWindowPos(pWindow, init.init_pos_x, init.init_pos_y);
    glfwSetWindowSizeLimits(pWindow, init.min_size_x, init.min_size_y,
                            init.max_size_x, init.max_size_y);
    glfwSetWindowUserPointer(pWindow, this);
}
void WindowContext::destroy() noexcept {
    if (pWindow)
        glfwDestroyWindow(pWindow);
    pWindow = nullptr;
    pMonitor = nullptr;
}
void WindowContext::update() noexcept {
    double cur_time = glfwGetTime();
    delta_time = cur_time - current_time;
    current_time = cur_time;
}
void WindowContext::erase_callback(CallbackHandle handle) {
    size_t index = handle->func.index();
    if (0<index&&index<WindowCallbackEnum::MaxCallbackEnum) {
        if (callback_head[index] == handle) {
            if (callback_head[index]->next == callback_head[index]) {
                callback_head[index] = nullptr;
            } else {
                callback_head[index] = callback_head[index]->next;
            }
        }
        handle->rear->next = handle->next;
        handle->next->rear = handle->rear;
        handle->next = callback_free;
        callback_free = handle;
    }
}
void FPSTitle::operator()(WindowContext& ctx) {
    delta_frame++;
    double delta_time = ctx.current_time - last_time;
    if (delta_time >= 1.0) {
        str_builder.precision(1);
        str_builder << ctx.title << " " << std::fixed
                      << delta_frame / delta_time << " FPS";
        glfwSetWindowTitle(ctx.pWindow, str_builder.str().c_str());
        str_builder.str("");
        last_time = ctx.current_time;
        delta_time = 0;
    }
}
}  // namespace BL
