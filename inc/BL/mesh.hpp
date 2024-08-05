#ifndef BOUNDLESS_MESH_FILE
#define BOUNDLESS_MESH_FILE
#include <fstream>
#include <string>
#include "ftypes.hpp"
#include "render.hpp"
#include "log.hpp"
#include "types.hpp"
#include "utility.hpp"
namespace BL {
struct MeshInfo {
    VkPrimitiveTopology topology;
    VkIndexType indexType;
    uint32_t vertexCount;
    uint32_t indexCount;
    uint32_t restartIndex;
    bool restartEnable;
    std::vector<VkVertexInputBindingDescription> inputBindings;
    std::vector<VkVertexInputAttributeDescription> inputAttributes;
};
class Mesh {
    std::string name;
    IndexBuffer indexBuffer;
    std::vector<VertexBuffer> vertexBuffers;
   public:
    MeshInfo info;

    Mesh() = default;
    Mesh(std::string path, uint32_t baseBinding = 0);
    ~Mesh() {}
    std::vector<VertexBuffer>& get_vertexbuffer() { return vertexBuffers; }
    VkBuffer get_indicesbuffer() { return VkBuffer(indexBuffer); }
    bool load(std::string path,
              uint32_t baseBinding = 0);
};
void copy_mesh_info(MeshFileHead* pFileData,MeshInfo* info);
void read_buffer_info_list(
    MeshFileHead* pHead,
    std::ifstream& inFile,
    std::vector<VkVertexInputBindingDescription>& inputBindings,
    std::vector<VkVertexInputAttributeDescription>& inputAttributes,
    std::vector<Range>& loadRanges,
    uint32_t baseBinding = 0);
}  // namespace BL
#endif  //! BOUNDLESS_MESH_FILE