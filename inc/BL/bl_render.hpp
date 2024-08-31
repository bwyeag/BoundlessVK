#ifndef _BOUNDLESS_RENDER_CXX_FILE_
#define _BOUNDLESS_RENDER_CXX_FILE_
#include "bl_context.hpp"
namespace BL {
struct RenderContextInit {
    WindowContext* windowContext;
    uint32_t renderPassCount;
    bool force_ownership_transfer = false;
};
struct RenderContext {
    WindowContext* windowContext;
    std::array<CommandBuffer, MAX_FLIGHT_NUM> cmdBuffer_presentation;
    std::array<Fence, MAX_FLIGHT_COUNT> fences;
    std::array<Semaphore, MAX_FLIGHT_NUM> semsOwnershipIsTransfered;
    std::vector<CommandBuffer> cmdBuffers;
    std::vector<Semaphore> semaphores;
    CommandPool cmdPool_graphics;
    CommandPool cmdPool_compute;
    CommandPool cmdPool_presentation;
    bool ownership_transfer;
    uint32_t image_index, maxRenderPassCount, currentRenderPass;

    RenderContext(const RenderContextInit* pInit) {
        create(pInit);
    }
    bool create(const RenderContextInit* pInit);
    void destroy();
    void beginRender();
    void beginRenderPass();
    void endRenderPass();
    void endRender();
    void present();
    ~RenderContext() {}
};
}  // namespace BL
#endif  // ! _BOUNDLESS_RENDER_CXX_FILE_