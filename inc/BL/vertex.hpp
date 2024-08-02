#ifndef _BOUNDLESS_VERTEX_CXX_FILE_
#define _BOUNDLESS_VERTEX_CXX_FILE_
#include <vulkan/vulkan.h>
#include <Eigen/Core>
namespace BL {
using vec2i = Eigen::Vector2i;
using vec3i = Eigen::Vector3i;
using vec4i = Eigen::Vector4i;
using vec2f = Eigen::Vector2f;
using vec3f = Eigen::Vector3f;
using vec4f = Eigen::Vector4f;
using vec2d = Eigen::Vector2d;
using vec3d = Eigen::Vector3d;
using vec4d = Eigen::Vector4d;
using mat2f = Eigen::Matrix2f;
using mat3f = Eigen::Matrix3f;
using mat4f = Eigen::Matrix4f;
using mat2d = Eigen::Matrix2d;
using mat3d = Eigen::Matrix3d;
using mat4d = Eigen::Matrix4d;

struct Vertex_2d {
    vec2f position;
    vec4f color;

    static void fill_attribute(std::vector<VkVertexInputBindingDescription>& v);
};
void Vertex_2d::fill_attribute(
    std::vector<VkVertexInputBindingDescription>& v) {
    v.reserve(2);
    pack.vertexInputAttributes.emplace_back(0, 0, VK_FORMAT_R32G32_SFLOAT,
                                            offsetof(Vertex_2d, position));
    pack.vertexInputAttributes.emplace_back(1, 0, VK_FORMAT_R32G32B32A32_SFLOAT,
                                            offsetof(Vertex_2d, color));
}
}  // namespace BL
#endif  //!_BOUNDLESS_VERTEX_CXX_FILE_