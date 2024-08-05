#ifndef _BOUNDLESS_RENDER_TYPES_CXX_FILE_
#define _BOUNDLESS_RENDER_TYPES_CXX_FILE_
#include "render.hpp"
#include "mesh.hpp"
#include "transform.hpp"

#define BL_USE_RENDER_TYPE_3D_TRANSFORM

namespace BL {
#ifdef BL_USE_RNEDER_TYPE_SIMPLE1
class RenderPassPack_simple1 : public RenderPassPackBase {
   public:
    virtual void create() override;
    RenderPassPack_simple1() { this->create(); }
    virtual ~RenderPassPack_simple1() {}
};
class RenderPipeline_simple1 : public RenderPipelineBase {
   public:
    UniformBuffer uniformBuffer;
    DescriptorSetLayout setLayout;
    DescriptorPool descriptorPool;
    std::array<DescriptorSet, MAX_FLIGHT_NUM> descriptorSets;
    virtual void create(Shader*, RenderPassPackBase*,void*) override;
    RenderPipeline_simple1() {}
    RenderPipeline_simple1(Shader* pShader, RenderPassPackBase* pRenderPass) {
        this->create(pShader, pRenderPass);
    }
    virtual ~RenderPipeline_simple1() {}
};
struct RenderDataPack_simple1 : public RenderDataPackBase {
    RenderPassPack_simple1* pRenderPass;
    RenderPipeline_simple1* pRenderPipeline;
    Buffer* pVertexData;
};
void render_funct_simple1(CommandBuffer& curBuf,
                          RenderDataPackBase& packBase,
                          uint32_t image_index);
#endif  // BL_USE_RNEDER_TYPE_SIMPLE1

#ifdef BL_USE_RENDER_TYPE_3D_TRANSFORM
class RenderPassPack_3d_trans_simple1 : public RenderPassPackBase {
   public:
    virtual void create() override;
    RenderPassPack_3d_trans_simple1() { this->create(); }
    virtual ~RenderPassPack_3d_trans_simple1() {}
};
struct RenderDataPack_3d_trans_simple1;
class RenderPipeline_3d_trans_simple1 : public RenderPipelineBase {
   public:
    UniformBuffer uniformBuffer;
    DescriptorSetLayout setLayout;
    DescriptorPool descriptorPool;
    std::array<DescriptorSet, MAX_FLIGHT_NUM> descriptorSets;
    virtual void create(Shader*, RenderPassPackBase*,void*) override;
    RenderPipeline_3d_trans_simple1() {}
    RenderPipeline_3d_trans_simple1(Shader* pShader,
                                    RenderPassPackBase* pRenderPass, void* ptr) {
        this->create(pShader, pRenderPass,ptr);
    }
    virtual ~RenderPipeline_3d_trans_simple1() {}
};
struct UniformData_3d_trans_simple1 {
    vec4f lightColor;
    vec4f lightPosition;
    float ambientStrength;
    float specularStrength;
    float _nd[2];
    vec4f objectColor;
    vec4f cameraPos;
    mat4f cameraMat;
    mat4f model;
};
struct RenderDataPack_3d_trans_simple1 : public RenderDataPackBase {
    RenderPassPack_3d_trans_simple1* pRenderPass;
    RenderPipeline_3d_trans_simple1* pRenderPipeline;
    Mesh* meshData;
};
void render_funct_3d_trans_simple1(CommandBuffer& curBuf,
                          RenderDataPackBase& packBase,
                          uint32_t image_index);
#endif  // BL_USE_RENDER_TYPE_3D_TRANSFORM
}  // namespace BL
#endif  //!_BOUNDLESS_RENDER_TYPES_CXX_FILE_