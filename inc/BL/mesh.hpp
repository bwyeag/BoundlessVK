#ifndef BOUNDLESS_MESH_FILE
#define BOUNDLESS_MESH_FILE
#include <string>
#include <fstream>
#include "log.hpp"
#include "types.hpp"
#include "ftypes.hpp"
namespace BL {
struct VertexFormat {

};
struct MeshInfo {
    VkPrimitiveTopology topology;
    VkIndexType indexType;
    uint32_t vertexCount;
    uint32_t indexCount;
    uint32_t restartIndex;
    bool restartEnable;

    void load(MeshFileHead* pFileData, bool loadVertexFormat);
};
class Mesh {
    std::string name;
    Buffer vertexBuffer;
    Buffer indexBuffer;
    MeshInfo info;
   public:
    Mesh() = default;
    Mesh(std::string path, bool validate = false);
    Mesh(MeshFileHead* pFileData, bool validate = false);
    ~Mesh() {}
    VkBuffer get_vertexbuffer() {return VkBuffer(vertexBuffer);}
    VkBuffer get_indicesbuffer() {return VkBuffer(indexBuffer);}
    void load(std::string path, bool validate = false);
    void load(MeshFileHead* pFileData, bool validate = false);
};
}  // namespace BL
#endif  //! BOUNDLESS_MESH_FILE