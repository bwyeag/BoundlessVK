#ifndef _BOUNDLESS_RENDER_TYPES_CXX_FILE_
#define _BOUNDLESS_RENDER_TYPES_CXX_FILE_
#include "mesh.hpp"
#include "render.hpp"
#include "transform.hpp"

#define BL_USE_RENDER_TYPE_PBR_3D_SIMPLE

#ifdef BL_USE_RENDER_TYPE_PBR_3D_SIMPLE
#ifndef BL_PBR_SIMPLE_MAX_LIGHT_NUM
#define BL_PBR_SIMPLE_MAX_LIGHT_NUM 8
#endif  // !BL_PBR_SIMPLE_MAX_LIGHT_NUM
#ifndef BL_PBR_SIMPLE_MAX_INSTANCE_NUM
#define BL_PBR_SIMPLE_MAX_INSTANCE_NUM 8
#endif  // !BL_PBR_SIMPLE_MAX_INSTANCE_NUM
#endif  // BL_USE_RENDER_TYPE_PBR_3D_SIMPLE

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
    virtual void create(Shader*, RenderPassPackBase*, void*) override;
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
    virtual void create(Shader*, RenderPassPackBase*, void*) override;
    RenderPipeline_3d_trans_simple1() {}
    RenderPipeline_3d_trans_simple1(Shader* pShader,
                                    RenderPassPackBase* pRenderPass,
                                    void* ptr) {
        this->create(pShader, pRenderPass, ptr);
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
    mat3f transNormal;
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

#ifdef BL_USE_RENDER_TYPE_PBR_3D_SIMPLE
struct alignas(16) LightData {
    alignas(16) vec3f lightPosition;
    alignas(16) vec3f lightColor;
};
struct Uniform_LightUniform {
    uint32_t lightCount;
    alignas(16) LightData light[BL_PBR_SIMPLE_MAX_LIGHT_NUM];
};
struct alignas(16) InstanceData {
    alignas(16) mat4f matrixModel;
    alignas(16) mat4x3f matrixNormal;  // 只用到它的3*3部分
    alignas(16) vec3f albedo;
    float metallic;
    float roughness;
    float ao;
};
struct Uniform_SenceUniform {
    alignas(16) vec3f cameraPosition;
    alignas(16) mat4f matrixCamera;
    alignas(16) InstanceData instance[BL_PBR_SIMPLE_MAX_INSTANCE_NUM];
};
struct RenderPassPack_PBR_3d_simple;
struct RenderPipeline_PBR_3d_simple;
struct RenderDataPack_PBR_3d_simple : public RenderDataPackBase {
    RenderPassPack_PBR_3d_simple* pRenderPass;
    RenderPipeline_PBR_3d_simple* pRenderPipeline;
    Mesh* meshData;
};
class RenderPassPack_PBR_3d_simple : public RenderPassPackBase {
   public:
    virtual void create() override;
    RenderPassPack_PBR_3d_simple() { this->create(); }
    virtual ~RenderPassPack_PBR_3d_simple() {}
};
class RenderPipeline_PBR_3d_simple : public RenderPipelineBase {
   public:
    UniformBuffer uniformBuf_LightUniform;  // binding = 0
    UniformBuffer uniformBuf_SenceData;     // binding = 1
    DescriptorSetLayout setLayout;
    DescriptorPool descriptorPool;
    std::array<DescriptorSet, MAX_FLIGHT_NUM> descriptorSets;
    virtual void create(Shader*, RenderPassPackBase*, void*) override;
    RenderPipeline_PBR_3d_simple() {}
    RenderPipeline_PBR_3d_simple(Shader* pShader,
                                 RenderPassPackBase* pRenderPass,
                                 void* ptr) {
        this->create(pShader, pRenderPass, ptr);
    }
    virtual ~RenderPipeline_PBR_3d_simple() {}
};
void render_funct_PBR_3d_simple(CommandBuffer& curBuf,
                                RenderDataPackBase& packBase,
                                uint32_t image_index);
#endif  // BL_USE_RENDER_TYPE_PBR_3D_SIMPLE
}  // namespace BL
#endif  //!_BOUNDLESS_RENDER_TYPES_CXX_FILE_