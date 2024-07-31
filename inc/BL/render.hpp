#ifndef _BOUNDLESS_RENDER_FILE_
#define _BOUNDLESS_RENDER_FILE_
#include <cstdint>
#include "init.hpp"
#include "shader.hpp"
#include "types.hpp"
namespace BL {
const uint32_t DEFAULT_MAX_FLIGHT_COUNT = 3;
const bool FORCE_OWNERSHIP_TRANSFER = true;
struct RenderContext {
    uint32_t maxFlightCount;
    uint32_t curFrame;
    std::vector<CommandBuffer> cmdBufs = {};
    std::vector<Fence> fences = {};
    std::vector<Semaphore> semsImageAvaliable = {};
    std::vector<Semaphore> semsRenderFinish = {};
    std::vector<Semaphore> semsOwnershipIsTransfered = {};
    std::vector<CommandBuffer> cmdBuffer_presentation;
    CommandPool cmdPool_graphics;
    CommandPool cmdPool_compute;
    CommandPool cmdPool_presentation;
    CommandBuffer cmdBuffer_transfer;
    bool ownership_transfer = false;
};
extern RenderContext render_context;
bool initVulkanRenderer();
void terminateVulkanRenderer();
VkResult _createRenderContext(uint32_t c);
void _destroyRenderContext();
void _createRenderLoop();
void _destroyRenderLoop();
void render();
VkResult submit_cmdBuffer_graphics_wait(VkCommandBuffer commandBuffer);
VkResult submit_cmdBuffer_graphics(VkCommandBuffer commandBuffer, VkFence fence = VK_NULL_HANDLE);
VkResult submit_cmdBuffer_compute_wait(VkCommandBuffer commandBuffer);
VkResult submit_cmdBuffer_compute(VkCommandBuffer commandBuffer, VkFence fence = VK_NULL_HANDLE);
// 在commandBuffer中插入将index的交换链图像从图形队列向呈现队列转移所有权的指令
void _insertCmd_transfer_image_ownership(VkCommandBuffer commandBuffer,uint32_t index);
VkResult submit_buffer_presentation(
    VkCommandBuffer commandBuffer,
    VkFence fence = VK_NULL_HANDLE,
    VkSemaphore semaphore_renderingIsOver = VK_NULL_HANDLE,
    VkSemaphore semaphore_ownershipIsTransfered = VK_NULL_HANDLE);
VkResult acquire_next_image(uint32_t* index,
                            VkSemaphore semsImageAvaliable = VK_NULL_HANDLE,
                            VkFence fence = VK_NULL_HANDLE);
VkResult present_image(uint32_t* index,
                       uint32_t waitSemsCount = 0,
                       VkSemaphore* sems = nullptr);
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