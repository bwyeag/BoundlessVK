#ifndef _BOUNDLESS_RENDER_FILE_
#define _BOUNDLESS_RENDER_FILE_
#include <array>
#include <cstdint>
#include "init.hpp"
#include "shader.hpp"
#include "types_plus.hpp"
#include "vertex.hpp"

#define BOUNDLESS_USE_UI_RENDER
#include <Eigen/Core>
namespace BL {
#ifdef BOUNDLESS_FORCE_OWNERSHIP_TRANSFER
const bool FORCE_OWNERSHIP_TRANSFER = true;
#else
const bool FORCE_OWNERSHIP_TRANSFER = false;
#endif // BOUNDLESS_FORCE_OWNERSHIP_TRANSFER
#ifdef BOUNDLESS_USE_UI_RENDER
const bool USE_UI_RENDER = true;
#else
const bool USE_UI_RENDER = false;
#endif // BOUNDLESS_USE_UI_RENDER
struct RenderContext {
    uint32_t curFrame, numFrame;
    std::array<CommandBuffer, MAX_FLIGHT_NUM> cmdBufs;
#ifdef BOUNDLESS_USE_UI_RENDER
    std::array<CommandBuffer, MAX_FLIGHT_NUM> cmdBufsUI;
#endif // BOUNDLESS_USE_UI_RENDER
    std::array<Fence, MAX_FLIGHT_NUM> fences;
    std::array<Semaphore, MAX_FLIGHT_NUM> semsImageAvaliable;
    std::array<Semaphore, MAX_FLIGHT_NUM> semsRenderMainPassFinish;
    std::array<Semaphore, MAX_FLIGHT_NUM> semsRenderFinish;
    std::array<Semaphore, MAX_FLIGHT_NUM> semsOwnershipIsTransfered;
    std::array<CommandBuffer, MAX_FLIGHT_NUM> cmdBuffer_presentation;
    CommandPool cmdPool_graphics;
    CommandPool cmdPool_compute;
    CommandPool cmdPool_presentation;
    CommandBuffer cmdBuffer_transfer;
    bool ownership_transfer = false;
    uint32_t image_index;

    VkFormat depthFormat;
    std::vector<DepthStencilAttachment> depthAttachments;
};
struct RenderDataPackBase{
    std::function<void(CommandBuffer&, RenderDataPackBase&, uint32_t)> pRenderFunction;
};
extern RenderContext render_CurContext();
extern VkExtent2D const& window_size;
bool initVulkanRenderer(bool UIRender);
void terminateVulkanRenderer();
VkResult _createRenderContext();
void _destroyRenderContext();
#ifdef BOUNDLESS_USE_UI_RENDER
VkResult _createUIRenderContext();
#endif // BOUNDLESS_USE_UI_RENDER
// 等待下一张图像可用并在有需要时重建交换链
void renderBegin();
void renderCall(RenderDataPackBase& packet);
void renderEnd(bool delayOwnershipTransfer);
#ifdef BOUNDLESS_USE_UI_RENDER
void renderUIBegin();
void renderUIEnd();
#endif // BOUNDLESS_USE_UI_RENDER
void renderPresent();
VkResult submit_cmdBuffer_graphics_wait(VkCommandBuffer commandBuffer);
VkResult submit_cmdBuffer_graphics(VkCommandBuffer commandBuffer,
                                   VkFence fence = VK_NULL_HANDLE);
VkResult submit_cmdBuffer_compute_wait(VkCommandBuffer commandBuffer);
VkResult submit_cmdBuffer_compute(VkCommandBuffer commandBuffer,
                                  VkFence fence = VK_NULL_HANDLE);
// 在commandBuffer中插入将index的交换链图像从图形队列向呈现队列转移所有权的指令
void _insertCmd_transfer_image_ownership(VkCommandBuffer commandBuffer,
                                         uint32_t index);
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
/*
 *   其他部分的初始化
 */
void initDepthAttachment();
void destroyDepthAttachment();


// 渲染的渲染通道的基类，负责（基类）对象的销毁及移动，创建由子类重写create()方法实现
// 子类需要实现渲染通道创建，帧缓冲创建和回调函数设置。创建即初始化
class RenderPassPackBase {
   protected:
    int callback_c_id = 0, callback_d_id = 0;

   public:
    RenderPass renderPass;
    std::vector<Framebuffer> framebuffers;

    RenderPassPackBase() {}
    RenderPassPackBase(RenderPassPackBase&) = delete;
    RenderPassPackBase(RenderPassPackBase&& other)
        : renderPass(std::move(other.renderPass)),
          framebuffers(std::move(other.framebuffers)) {
        callback_c_id = other.callback_c_id;
        callback_d_id = other.callback_d_id;
        other.callback_c_id = 0;
        other.callback_d_id = 0;
    }
    virtual ~RenderPassPackBase() = 0;
    virtual void create() = 0;
};
// 渲染管线的基类
class RenderPipelineBase {
   protected:
    int callback_c_id, callback_d_id;

   public:
    Pipeline renderPipeline;
    PipelineLayout layout;
    RenderPipelineBase() {}
    RenderPipelineBase(RenderPipelineBase&) = delete;
    RenderPipelineBase(RenderPipelineBase&& other)
        : renderPipeline(std::move(other.renderPipeline)),
          layout(std::move(other.layout)) {
        callback_c_id = other.callback_c_id;
        callback_d_id = other.callback_d_id;
        other.callback_c_id = 0;
        other.callback_d_id = 0;
    }
    virtual ~RenderPipelineBase() = 0;
    virtual void create(Shader*, RenderPassPackBase*,void*) = 0;
};
}  // namespace BL
#endif  //!_BOUNDLESS_RENDER_FILE_