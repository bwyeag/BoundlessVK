#ifndef _BOUNDLESS_VERTEX_CXX_FILE_
#define _BOUNDLESS_VERTEX_CXX_FILE_
#include <vulkan/vulkan.h>
#include "transform.hpp"
namespace BL {
struct Vertex_2d {
    vec2f position;
    vec4f color;

    static void fill_attribute(
        std::vector<VkVertexInputAttributeDescription>& v) {
        v.reserve(2);
        v.emplace_back(0, 0, VK_FORMAT_R32G32_SFLOAT,
                       offsetof(Vertex_2d, position));
        v.emplace_back(1, 0, VK_FORMAT_R32G32B32A32_SFLOAT,
                       offsetof(Vertex_2d, color));
    }
};
}
#endif  //!_BOUNDLESS_VERTEX_CXX_FILE_