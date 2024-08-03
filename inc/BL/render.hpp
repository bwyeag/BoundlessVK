#ifndef _BOUNDLESS_RENDER_FILE_
#define _BOUNDLESS_RENDER_FILE_
#include <array>
#include <cstdint>
#include "init.hpp"
#include "shader.hpp"
#include "types.hpp"
#include "vertex.hpp"

#include <Eigen/Core>
namespace BL {
const bool FORCE_OWNERSHIP_TRANSFER = true;
struct RenderContext {
    uint32_t curFrame, numFrame;
    std::array<CommandBuffer, MAX_FLIGHT_NUM> cmdBufs;
    std::array<Fence, MAX_FLIGHT_NUM> fences;
    std::array<Semaphore, MAX_FLIGHT_NUM> semsImageAvaliable;
    std::array<Semaphore, MAX_FLIGHT_NUM> semsRenderFinish;
    std::array<Semaphore, MAX_FLIGHT_NUM> semsOwnershipIsTransfered;
    std::array<CommandBuffer, MAX_FLIGHT_NUM> cmdBuffer_presentation;
    CommandPool cmdPool_graphics;
    CommandPool cmdPool_compute;
    CommandPool cmdPool_presentation;
    CommandBuffer cmdBuffer_transfer;
    bool ownership_transfer = false;
};
struct RenderDataPackBase{
    std::function<void(CommandBuffer&, RenderDataPackBase&, uint32_t)> pRenderFunction;
};
extern RenderContext render_context;
bool initVulkanRenderer();
void terminateVulkanRenderer();
VkResult _createRenderContext();
void _destroyRenderContext();
void _createRenderLoop();
void _destroyRenderLoop();
void render(RenderDataPackBase& packet);
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
    virtual void create(Shader*, RenderPassPackBase*) = 0;
};
// ----------------------------------------------------------------------------
// 实际类型的定义
class RenderPassPack_simple1 : public RenderPassPackBase {
   public:
    virtual void create() override;
    RenderPassPack_simple1() { this->create(); }
};
class RenderPipeline_simple1 : public RenderPipelineBase {
   public:
    UniformBuffer uniformBuffer;
    DescriptorSetLayout setLayout;
    DescriptorPool descriptorPool;
    std::array<DescriptorSet, MAX_FLIGHT_NUM> descriptorSets;
    virtual void create(Shader*, RenderPassPackBase*) override;
    RenderPipeline_simple1() {}
    RenderPipeline_simple1(Shader* pShader, RenderPassPackBase* pRenderPass) {
        this->create(pShader, pRenderPass);
    }
    virtual ~RenderPipeline_simple1() {}
};
struct RenderDataPack_simple1 : public RenderDataPackBase{
    RenderPassPack_simple1* pRenderPass;
    RenderPipeline_simple1* pRenderPipline;
    Buffer* pVertexData;
};
}  // namespace BL
#endif  //!_BOUNDLESS_RENDER_FILE_