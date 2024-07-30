#ifndef _BOUNDLESS_RENDER_FILE_
#define _BOUNDLESS_RENDER_FILE_
#include <cstdint>
#include "init.hpp"
#include "shader.hpp"
#include "types.hpp"
namespace BL {
const uint32_t DEFAULT_MAX_FLIGHT_COUNT = 3;
struct RenderContext {
    CommandPool cmdPool;
    uint32_t maxFlightCount;
    uint32_t curFrame;
    std::vector<CommandBuffer> cmdBufs;
    std::vector<Fence> fences;
    std::vector<Semaphore> semsImageAvaliable;
    std::vector<Semaphore> semsRenderFinish;

    RenderContext(uint32_t c);
    ~RenderContext() = default;
};
struct RenderContextPlus {
    CommandPool cmdPool_graphics;
    CommandPool cmdPool_compute;
    CommandPool cmdPool_presentation;
    CommandBuffer cmdBuffer_transfer;
    CommandBuffer cmdBuffer_presentation;
};
void initRenderLoop();
void terminateRenderLoop();
void renderLoop();
VkResult createRenderContext();
void destroyRenderContext();
VkResult execute_cmdBuffer_graphics_wait(VkCommandBuffer commandBuffer);
VkResult execute_cmdBuffer_graphics(VkCommandBuffer commandBuffer,Fence& fence);

class RenderPassPack {
    void create();

   public:
    RenderPass renderPass;
    std::vector<Framebuffer> framebuffers;
    int callback_c_id, callback_d_id;
    RenderPassPack() { create(); }
    ~RenderPassPack() {
        removeCallback_CreateSwapchain(callback_c_id);
        removeCallback_DestroySwapchain(callback_d_id);
    }
};
class RenderPipeline {
    void create();

   public:
    Pipeline renderPipeline;
    PipelineLayout layout;
    Shader shader;
    int callback_c_id, callback_d_id;

    RenderPipeline() { create(); }
    ~RenderPipeline() {
        removeCallback_CreateSwapchain(callback_c_id);
        removeCallback_DestroySwapchain(callback_d_id);
    }
};
}  // namespace BL
#endif  //!_BOUNDLESS_RENDER_FILE_