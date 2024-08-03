#include "mesh.hpp"

namespace BL {
void MeshInfo::load(MeshFileHead* pFileData, bool loadVertexFormat) {
    topology = pFileData->topology;
    indexType = pFileData->indexType;
    vertexCount = pFileData->vertexCount;
    indexCount = pFileData->vertexCount;
    restartIndex = pFileData->restartIndex;
    restartEnable = pFileData==0?false:true;
}
Mesh::Mesh(std::string path, bool validate = false) {
    load(path, validate);
}
Mesh::Mesh(MeshFileHead* pFileData, bool validate = false) {
    load(pFileData, validate);
}
void Mesh::load(std::string path, bool validate = false) {
    std::ifstream inFile(path, std::ios::ate | std::ios::binary);
    if (!inFile.is_open()) {
        print_error("Mesh", "File not found! Path:", path);
        return;
    }
    size_t length = inFile.tellg();
    inFile.seekg(std::ios::beg);
    uint32_t head_code;
    inFile.read((char*)&head_code, sizeof head_code);
    if (head_code != MESH_HEAD_CODE) {
        print_error("Mesh", "File head code error! Path:", path);
        return;
    }
    inFile.seekg(std::ios::beg);
    MeshFileHead* pFile = (MeshFileHead*)malloc(length);
    if (!pFile) {
        throw std::bad_alloc();
    }

    inFile.read((char*)pFile, length);
    load(pFile, validate);
    free(pFile);
}
void Mesh::load(MeshFileHead* pFileData, bool validate = false) {
    if (pFileData->_head != MESH_HEAD_CODE) {
        print_error("Mesh", "File head code error!");
        return;
    }
    name = pFileData->getName();
    info.load(pFileData);
}
}  // namespace BL
