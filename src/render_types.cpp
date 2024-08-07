#include "render_types.hpp"
namespace BL {
#ifdef BL_USE_RNEDER_TYPE_SIMPLE1
void RenderPassPack_simple1::create() {
    VkAttachmentDescription attachmentDescription = {
        .format = context.vulkanInfo.swapchainCreateInfo.imageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};
    VkAttachmentReference attachmentReference = {
        0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpassDescription = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentReference};
    VkSubpassDependency subpassDependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT};
    VkRenderPassCreateInfo renderPassCreateInfo = {
        .attachmentCount = 1,
        .pAttachments = &attachmentDescription,
        .subpassCount = 1,
        .pSubpasses = &subpassDescription,
        .dependencyCount = 1,
        .pDependencies = &subpassDependency};
    renderPass.create(renderPassCreateInfo);

    auto CreateFramebuffers = [this] {
        framebuffers.resize(context.getSwapChainImageCount());
        VkFramebufferCreateInfo framebufferCreateInfo = {
            .renderPass = renderPass,
            .attachmentCount = 1,
            .width = window_size.width,
            .height = window_size.height,
            .layers = 1};
        VkImageView attachment;
        for (size_t i = 0; i < context.getSwapChainImageCount(); i++) {
            attachment = context.vulkanInfo.swapchainImageViews[i];
            framebufferCreateInfo.pAttachments = &attachment;
            framebuffers[i].create(framebufferCreateInfo);
        }
    };
    auto DestroyFramebuffers = [this] { framebuffers.clear(); };
    CreateFramebuffers();
    callback_c_id = addCallback_CreateSwapchain(CreateFramebuffers);
    callback_d_id = addCallback_DestroySwapchain(DestroyFramebuffers);
}
void RenderPipeline_simple1::create(Shader* pShader,
                                    RenderPassPackBase* pRenderPass,
                                    void* data) {
    VkDescriptorSetLayoutBinding setLayoutBinding = {
        .binding = 0,  // 描述符被绑定到0号binding
        .descriptorType =
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,  // 类型为uniform缓冲区
        .descriptorCount = 1,                   // 个数是1个
        .stageFlags =
            VK_SHADER_STAGE_VERTEX_BIT  // 在顶点着色器阶段读取uniform缓冲区
    };
    VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &setLayoutBinding};
    setLayout.create(setLayoutCreateInfo);
    VkDescriptorPoolSize poolSizes[1] = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3}};
    descriptorPool.create(3, 1, poolSizes);
    VkDescriptorSetLayout layouts[MAX_FLIGHT_NUM];
    for (uint32_t i = 0; i < MAX_FLIGHT_NUM; i++) {
        layouts[i] = VkDescriptorSetLayout(setLayout);
    }
    descriptorPool.allocate_sets(MAX_FLIGHT_NUM, descriptorSets[0].getPointer(),
                                 layouts);
    VkDescriptorBufferInfo bufferInfo = {
        .buffer = VkBuffer(uniformBuffer),
        .offset = 0,
        .range = uniformBuffer.get_block_size()};
    for (uint32_t i = 0; i < MAX_FLIGHT_NUM; i++) {
        descriptorSets[i].write(&bufferInfo, 1,
                                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        bufferInfo.offset += uniformBuffer.get_alignment();
    }
    auto Create = [this, pShader, pRenderPass] {
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1,
            .pSetLayouts = setLayout.getPointer()};
        layout.create(pipelineLayoutCreateInfo);
        PipelineCreateInfosPack pack;
        pack.createInfo.layout = VkPipelineLayout(layout);
        pack.createInfo.renderPass = VkRenderPass(pRenderPass->renderPass);
        pack.inputAssemblyStateCi.topology =
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pack.viewports.emplace_back(0.0f, 0.0f, float(window_size.width),
                                    float(window_size.height), 0.f, 1.f);
        pack.scissors.emplace_back(VkOffset2D{}, window_size);
        pack.multisampleStateCi.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        pack.colorBlendAttachmentStates.push_back(
            VkPipelineColorBlendAttachmentState{.colorWriteMask = 0b1111});
        //---------------------------------------------------------------------
        pack.vertexInputBindings.emplace_back(0, sizeof(Vertex_2d),
                                              VK_VERTEX_INPUT_RATE_VERTEX);
        Vertex_2d::fill_attribute(pack.vertexInputAttributes);
        //---------------------------------------------------------------------
        pack.update_all_arrays();
        pack.createInfo.stageCount = pShader->getStages().size();
        pack.createInfo.pStages = pShader->getStages().data();
        renderPipeline.create(pack);
    };
    auto Destroy = [this] {
        renderPipeline.~Pipeline();
        layout.~PipelineLayout();
    };
    Create();
    callback_c_id = addCallback_CreateSwapchain(Create);
    callback_d_id = addCallback_DestroySwapchain(Destroy);
}
void render_funct_simple1(CommandBuffer& curBuf,
                          RenderDataPackBase& packBase,
                          uint32_t image_index) {
    RenderDataPack_simple1& packet =
        reinterpret_cast<RenderDataPack_simple1&>(packBase);
    uint32_t curFrame = render_context.curFrame;
    VkClearValue clearColor = {.color = {0.1f, 0.1f, 0.1f, 1.0f}};
    packet.pRenderPass->renderPass.cmd_begin(
        curBuf, packet.pRenderPass->framebuffers[image_index],
        {{}, context.vulkanInfo.swapchainCreateInfo.imageExtent}, &clearColor,
        1);
    vkCmdBindPipeline(curBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      VkPipeline(packet.pRenderPipeline->renderPipeline));
    vkCmdBindDescriptorSets(
        curBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, packet.pRenderPipeline->layout,
        0, 1, packet.pRenderPipeline->descriptorSets[curFrame].getPointer(), 0,
        nullptr);
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(curBuf, 0, 1, packet.pVertexData->getPointer(),
                           &offset);
    vkCmdDraw(curBuf, 3, 3, 0, 0);
    packet.pRenderPass->renderPass.cmd_end(curBuf);
}
#endif  // BL_USE_RNEDER_TYPE_SIMPLE1

#ifdef BL_USE_RENDER_TYPE_3D_TRANSFORM
void RenderPassPack_3d_trans_simple1::create() {
    VkAttachmentDescription attachmentDescriptions[2] = {
        {// 颜色附件
         .format = context.vulkanInfo.swapchainCreateInfo.imageFormat,
         .samples = VK_SAMPLE_COUNT_1_BIT,
         .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
         .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
         .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
         .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR},
        {// 深度模板附件
         .format = render_context.depthFormat,
         .samples = VK_SAMPLE_COUNT_1_BIT,
         .loadOp = render_context.depthFormat != VK_FORMAT_S8_UINT
                       ? VK_ATTACHMENT_LOAD_OP_CLEAR
                       : VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
         .stencilLoadOp = render_context.depthFormat >= VK_FORMAT_S8_UINT
                              ? VK_ATTACHMENT_LOAD_OP_CLEAR
                              : VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
         .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}};
    VkAttachmentReference attachmentReferences[2] = {
        {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}};
    VkSubpassDescription subpassDescription = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = attachmentReferences,
        .pDepthStencilAttachment = attachmentReferences + 1};
    VkSubpassDependency subpassDependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT};
    VkRenderPassCreateInfo renderPassCreateInfo = {
        .attachmentCount = 2,
        .pAttachments = attachmentDescriptions,
        .subpassCount = 1,
        .pSubpasses = &subpassDescription,
        .dependencyCount = 1,
        .pDependencies = &subpassDependency};
    renderPass.create(renderPassCreateInfo);
    auto Create = [this] {
        framebuffers.resize(context.getSwapChainImageCount());
        VkFramebufferCreateInfo framebufferCreateInfo = {
            .renderPass = VkRenderPass(renderPass),
            .attachmentCount = 2,
            .width = window_size.width,
            .height = window_size.height,
            .layers = 1};
        for (size_t i = 0; i < context.getSwapChainImageCount(); i++) {
            VkImageView attachments[2] = {
                context.vulkanInfo.swapchainImageViews[i],
                render_context.depthAttachments[i].get_image_view()};
            framebufferCreateInfo.pAttachments = attachments;
            framebuffers[i].create(framebufferCreateInfo);
        }
    };
    auto Destroy = [this] { framebuffers.clear(); };
    Create();
    callback_c_id = addCallback_CreateSwapchain(Create);
    callback_d_id = addCallback_DestroySwapchain(Destroy);
}
void RenderPipeline_3d_trans_simple1::create(
    Shader* shader,
    RenderPassPackBase* _renderPassPack,
    void* _renderDataPack) {
    RenderPassPack_3d_trans_simple1* renderPassPack =
        dynamic_cast<RenderPassPack_3d_trans_simple1*>(_renderPassPack);
    RenderDataPack_3d_trans_simple1* renderDataPack =
        reinterpret_cast<RenderDataPack_3d_trans_simple1*>(_renderDataPack);
    VkDescriptorSetLayoutBinding setLayoutBinding = {
        .binding = 0,  // 描述符被绑定到0号binding
        .descriptorType =
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,  // 类型为uniform缓冲区
        .descriptorCount = 1,                   // 个数是1个
        .stageFlags =
            VK_SHADER_STAGE_VERTEX_BIT |
            VK_SHADER_STAGE_FRAGMENT_BIT  // 在顶点着色器阶段读取uniform缓冲区
    };
    VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &setLayoutBinding};
    setLayout.create(setLayoutCreateInfo);
    VkDescriptorPoolSize poolSizes[1] = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3}};
    descriptorPool.create(3, 1, poolSizes);
    VkDescriptorSetLayout layouts[MAX_FLIGHT_NUM];
    for (uint32_t i = 0; i < MAX_FLIGHT_NUM; i++) {
        layouts[i] = VkDescriptorSetLayout(setLayout);
    }
    descriptorPool.allocate_sets(MAX_FLIGHT_NUM, descriptorSets[0].getPointer(),
                                 layouts);
    VkDescriptorBufferInfo bufferInfo = {
        .buffer = VkBuffer(uniformBuffer),
        .offset = 0,
        .range = uniformBuffer.get_block_size()};
    for (uint32_t i = 0; i < MAX_FLIGHT_NUM; i++) {
        descriptorSets[i].write(&bufferInfo, 1,
                                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        bufferInfo.offset += uniformBuffer.get_alignment();
    }
    auto Create = [this, shader, renderPassPack, renderDataPack] {
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1,
            .pSetLayouts = setLayout.getPointer()};
        layout.create(pipelineLayoutCreateInfo);
        PipelineCreateInfosPack pack;
        pack.createInfo.layout = this->layout;
        pack.createInfo.renderPass = renderPassPack->renderPass;
        pack.vertexInputBindings = renderDataPack->meshData->info.inputBindings;

        pack.vertexInputAttributes =
            renderDataPack->meshData->info.inputAttributes;
        pack.inputAssemblyStateCi.topology =
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pack.viewports.emplace_back(0.f, 0.f, float(window_size.width),
                                    float(window_size.height), 0.f, 1.f);
        pack.scissors.emplace_back(VkOffset2D{}, window_size);

        // 开启背面剔除
        pack.rasterizationStateCi.cullMode = VK_CULL_MODE_FRONT_BIT;
        pack.rasterizationStateCi.lineWidth = 1.0f;
        pack.rasterizationStateCi.polygonMode = VK_POLYGON_MODE_FILL;
        pack.rasterizationStateCi.frontFace =
            VK_FRONT_FACE_COUNTER_CLOCKWISE;  // 默认值，为0

        pack.multisampleStateCi.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // 开启深度测试
        pack.depthStencilStateCi.depthTestEnable = VK_TRUE;
        pack.depthStencilStateCi.depthWriteEnable = VK_TRUE;
        pack.depthStencilStateCi.depthCompareOp =
            VK_COMPARE_OP_LESS;  // 若新片元的深度更小，则通过测试

        pack.colorBlendAttachmentStates.push_back({.colorWriteMask = 0b1111});
        pack.shaderStages = shader->getStages();
        pack.update_all_arrays();
        renderPipeline.create(pack);
    };
    auto Destroy = [this] {
        renderPipeline.~Pipeline();
        layout.~PipelineLayout();
    };
    Create();
    callback_c_id = addCallback_CreateSwapchain(Create);
    callback_d_id = addCallback_DestroySwapchain(Destroy);
}
void render_funct_3d_trans_simple1(CommandBuffer& curBuf,
                                   RenderDataPackBase& packBase,
                                   uint32_t image_index) {
    RenderDataPack_3d_trans_simple1& packet =
        reinterpret_cast<RenderDataPack_3d_trans_simple1&>(packBase);
    uint32_t curFrame = render_context.curFrame;
    VkClearValue clearValues[2] = {{.color = {0.1f, 0.1f, 0.1f, 1.0f}},
                                   {.depthStencil = {1.0f, 0}}};
    packet.pRenderPass->renderPass.cmd_begin(
        curBuf, packet.pRenderPass->framebuffers[image_index],
        {{}, window_size}, clearValues, 2);
    vkCmdBindPipeline(curBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      VkPipeline(packet.pRenderPipeline->renderPipeline));
    vkCmdBindDescriptorSets(
        curBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, packet.pRenderPipeline->layout,
        0, 1, packet.pRenderPipeline->descriptorSets[curFrame].getPointer(), 0,
        nullptr);
    VkDeviceSize offset = 0;
    auto& vert_bufs = packet.meshData->get_vertexbuffer();
    VkBuffer vert_bufs_handle[vert_bufs.size()];
    for (size_t i = 0; i < vert_bufs.size(); i++) {
        vert_bufs_handle[i] = VkBuffer(vert_bufs[i]);
    }
    vkCmdBindVertexBuffers(curBuf, 0, vert_bufs.size(), vert_bufs_handle,
                           &offset);
    vkCmdBindIndexBuffer(curBuf, packet.meshData->get_indicesbuffer(), 0,
                         packet.meshData->info.indexType);
    vkCmdDrawIndexed(curBuf, packet.meshData->info.indexCount, 3, 0, 0, 0);
    packet.pRenderPass->renderPass.cmd_end(curBuf);
}
#endif  // BL_USE_RENDER_TYPE_3D_TRANSFORM

#ifdef BL_USE_RENDER_TYPE_PBR_3D_SIMPLE
void RenderPassPack_PBR_3d_simple::create() {
    VkAttachmentDescription attachmentDescriptions[2] = {
        {// 颜色附件
         .format = context.vulkanInfo.swapchainCreateInfo.imageFormat,
         .samples = VK_SAMPLE_COUNT_1_BIT,
         .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
         .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
         .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
         .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR},
        {// 深度模板附件
         .format = render_context.depthFormat,
         .samples = VK_SAMPLE_COUNT_1_BIT,
         .loadOp = render_context.depthFormat != VK_FORMAT_S8_UINT
                       ? VK_ATTACHMENT_LOAD_OP_CLEAR
                       : VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
         .stencilLoadOp = render_context.depthFormat >= VK_FORMAT_S8_UINT
                              ? VK_ATTACHMENT_LOAD_OP_CLEAR
                              : VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
         .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}};
    VkAttachmentReference attachmentReferences[2] = {
        {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}};
    VkSubpassDescription subpassDescription = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentReferences[0],
        .pDepthStencilAttachment = &attachmentReferences[1]};
    VkSubpassDependency subpassDependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT};
    VkRenderPassCreateInfo renderPassCreateInfo = {
        .attachmentCount = 2,
        .pAttachments = attachmentDescriptions,
        .subpassCount = 1,
        .pSubpasses = &subpassDescription,
        .dependencyCount = 1,
        .pDependencies = &subpassDependency};
    renderPass.create(renderPassCreateInfo);
    auto Create = [this] {
        framebuffers.resize(context.getSwapChainImageCount());
        VkFramebufferCreateInfo framebufferCreateInfo = {
            .renderPass = VkRenderPass(renderPass),
            .attachmentCount = 2,
            .width = window_size.width,
            .height = window_size.height,
            .layers = 1};
        for (size_t i = 0; i < context.getSwapChainImageCount(); i++) {
            VkImageView attachments[2] = {
                context.vulkanInfo.swapchainImageViews[i],
                render_context.depthAttachments[i].get_image_view()};
            framebufferCreateInfo.pAttachments = attachments;
            framebuffers[i].create(framebufferCreateInfo);
        }
    };
    auto Destroy = [this] { framebuffers.clear(); };
    Create();
    callback_c_id = addCallback_CreateSwapchain(Create);
    callback_d_id = addCallback_DestroySwapchain(Destroy);
}
void RenderPipeline_PBR_3d_simple::create(Shader* shader,
                                          RenderPassPackBase* _renderPassPack,
                                          void* _meshData) {
    RenderPassPack_PBR_3d_simple& renderPassPack =
        *dynamic_cast<RenderPassPack_PBR_3d_simple*>(_renderPassPack);
    Mesh& meshData = *reinterpret_cast<Mesh*>(_meshData);
    VkDescriptorSetLayoutBinding setLayoutBinding[2] = {
        {
            // uniformBuf_LightUniform
            .binding = 0,  // 描述符被绑定到0号binding
            .descriptorType =
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,  // 类型为uniform缓冲区
            .descriptorCount = 1,                   // 个数是1个
            .stageFlags =
                VK_SHADER_STAGE_VERTEX_BIT |
                VK_SHADER_STAGE_FRAGMENT_BIT  // 在片段和顶点阶段读取uniform缓冲区(vulkan标准规定两个的descriptorType
                                              // 和 stageFlags 必须相同)
        },
        {
            // uniformBuf_SenceData
            .binding = 1,  // 描述符被绑定到1号binding
            .descriptorType =
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,  // 类型为uniform缓冲区
            .descriptorCount = 1,                   // 个数是1个
            .stageFlags =
                VK_SHADER_STAGE_VERTEX_BIT |
                VK_SHADER_STAGE_FRAGMENT_BIT  // 在片段和顶点阶段读取uniform缓冲区
        }};
    VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 2,
        .pBindings = setLayoutBinding};
    setLayout.create(setLayoutCreateInfo);
    VkDescriptorPoolSize poolSizes[1] = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
         2 * MAX_FLIGHT_NUM /*每个Flight需要两个Uniform*/}};
    descriptorPool.create(MAX_FLIGHT_NUM, 1, poolSizes);
    VkDescriptorSetLayout layouts[MAX_FLIGHT_NUM];
    for (uint32_t i = 0; i < MAX_FLIGHT_NUM; i++) {
        layouts[i] = VkDescriptorSetLayout(setLayout);
    }
    descriptorPool.allocate_sets(descriptorSets.size(),
                                 descriptorSets[0].getPointer(), layouts);
    VkDescriptorBufferInfo bufferInfo[2] = {
        {.buffer = VkBuffer(uniformBuf_LightUniform),
         .offset = 0,
         .range = uniformBuf_LightUniform.get_block_size()},
        {.buffer = VkBuffer(uniformBuf_SenceData),
         .offset = 0,
         .range = uniformBuf_SenceData.get_block_size()}};
    for (uint32_t i = 0; i < descriptorSets.size(); i++) {
        descriptorSets[i].write(bufferInfo, 2,
                                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        bufferInfo[0].offset += uniformBuf_LightUniform.get_alignment();
        bufferInfo[1].offset += uniformBuf_SenceData.get_alignment();
    }
    auto Create = [this, shader, &renderPassPack, &meshData] {
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1,
            .pSetLayouts = setLayout.getPointer()};
        layout.create(pipelineLayoutCreateInfo);
        PipelineCreateInfosPack pack;
        pack.createInfo.layout = this->layout;
        pack.createInfo.renderPass = renderPassPack.renderPass;
        pack.vertexInputBindings = meshData.info.inputBindings;

        pack.vertexInputAttributes = meshData.info.inputAttributes;
        pack.inputAssemblyStateCi.topology =
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pack.viewports.emplace_back(0.f, 0.f, float(window_size.width),
                                    float(window_size.height), 0.f, 1.f);
        pack.scissors.emplace_back(VkOffset2D{}, window_size);

        // 开启背面剔除
        pack.rasterizationStateCi.cullMode = VK_CULL_MODE_NONE;
        pack.rasterizationStateCi.polygonMode = VK_POLYGON_MODE_FILL;
        pack.rasterizationStateCi.lineWidth = 1.0f;
        pack.rasterizationStateCi.frontFace = VK_FRONT_FACE_CLOCKWISE;

        pack.multisampleStateCi.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // 开启深度测试
        pack.depthStencilStateCi.depthTestEnable = VK_TRUE;
        pack.depthStencilStateCi.depthWriteEnable = VK_TRUE;
        pack.depthStencilStateCi.depthCompareOp =
            VK_COMPARE_OP_LESS;  // 若新片元的深度更小，则通过测试

        pack.colorBlendAttachmentStates.push_back({.colorWriteMask = 0b1111});
        pack.update_all_arrays();
        pack.createInfo.pStages = shader->getStages().data();
        pack.createInfo.stageCount = shader->getStages().size();
        renderPipeline.create(pack);
    };
    auto Destroy = [this] {
        renderPipeline.~Pipeline();
        layout.~PipelineLayout();
    };
    Create();
    callback_c_id = addCallback_CreateSwapchain(Create);
    callback_d_id = addCallback_DestroySwapchain(Destroy);
}
void render_funct_PBR_3d_simple(CommandBuffer& curBuf,
                                RenderDataPackBase& packBase,
                                uint32_t image_index) {
    RenderDataPack_PBR_3d_simple& packet =
        reinterpret_cast<RenderDataPack_PBR_3d_simple&>(packBase);
    uint32_t curFrame = render_context.curFrame;
    VkClearValue clearValues[2] = {{.color = {0.1f, 0.1f, 0.1f, 1.0f}},
                                   {.depthStencil = {1.0f, 0}}};
    packet.pRenderPass->renderPass.cmd_begin(
        curBuf, packet.pRenderPass->framebuffers[image_index],
        {{}, window_size}, clearValues, 2);
    vkCmdBindPipeline(curBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      VkPipeline(packet.pRenderPipeline->renderPipeline));
    vkCmdBindDescriptorSets(
        curBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, packet.pRenderPipeline->layout,
        0, 1, packet.pRenderPipeline->descriptorSets[curFrame].getPointer(), 0,
        nullptr);
    VkDeviceSize offset = 0;
    auto& vert_bufs = packet.meshData->get_vertexbuffer();
    VkBuffer vert_bufs_handle[vert_bufs.size()];
    for (size_t i = 0; i < vert_bufs.size(); i++) {
        vert_bufs_handle[i] = VkBuffer(vert_bufs[i]);
    }
    vkCmdBindVertexBuffers(curBuf, 0, vert_bufs.size(), vert_bufs_handle,
                           &offset);
    vkCmdBindIndexBuffer(curBuf, packet.meshData->get_indicesbuffer(), 0,
                         packet.meshData->info.indexType);
    vkCmdDrawIndexed(curBuf, packet.meshData->info.indexCount, 100, 0, 0, 0);
    packet.pRenderPass->renderPass.cmd_end(curBuf);
}
#endif  // BL_USE_RENDER_TYPE_PBR_3D_SIMPLE
}  // namespace BL