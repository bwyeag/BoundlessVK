#include "bl_helpers_funct.hpp"

namespace bl {
void _makeMeshFile(const aiMesh* mesh,
                   const std::string& storePath,
                   const char* name);
std::vector<MeshFileHead::VertexAttr> queryMeshVertexAttr(const aiMesh* mesh,
                                                          uint32_t& stride);
void makeModelFile(const std::string& path, const std::string& storePath) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, 0);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode) {
        throw std::runtime_error(importer.GetErrorString());
    }
    for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
        _makeMeshFile(
            scene->mMeshes[i],
            storePath + "_" + scene->mMeshes[i]->mName.C_Str() + ".mesh",
            scene->mMeshes[i]->mName.C_Str());
    }
}
static std::vector<MeshFileHead::VertexAttr> queryMeshVertexAttr(
    const aiMesh* mesh,
    uint32_t& stride) {
    static vk::Format formats[3]{
        vk::Format::eR32Sfloat, vk::Format::eR32G32Sfloat,
        vk::Format::eR32G32B32Sfloat, vk::Format::eR32G32B32A32Sfloat};
    std::vector<MeshFileHead::VertexAttr> vertAttrs;
    MeshFileHead::VertexAttr curAttr;
    curAttr.format = formats[3];
    curAttr.location = 0;
    curAttr.offset = 0;
    vertAttrs.push_back(curAttr);
    curAttr.offset += sizeof(aiVector3D);
    std::cout << "Vertex Format:\nPositions vec3\n";
    if (mesh->HasNormals()) {
        curAttr.format = formats[3];
        curAttr.location++;
        vertAttrs.push_back(curAttr);
        curAttr.offset += sizeof(aiVector3D);
        std::cout << "Normals vec3\n";
    }
    if (mesh->GetNumUVChannels() > 0) {
        std::cout << "Texture Coords:\n";
        for (size_t i = 0; i < 8 /*uvCannel的最大数目*/; i++) {
            if (mesh->HasTextureCoords(i)) {
                curAttr.format = formats[mesh->mNumUVComponents[i]];
                curAttr.location++;
                vertAttrs.push_back(curAttr);
                curAttr.offset += sizeof(ai_real) * mesh->mNumUVComponents[i];
                std::cout << (mesh->HasTextureCoordsName(i)
                                  ? mesh->mTextureCoordsNames[i]->C_Str()
                                  : "NULL")
                          << ": vec" << mesh->mNumUVComponents[i] << '\n';
            }
        }
    }
    if (mesh->GetNumColorChannels() > 0) {
        curAttr.format = formats[4];
        for (uint32_t i = 0; i < mesh->GetNumColorChannels(); i++) {
            curAttr.location++;
            vertAttrs.push_back(curAttr);
            curAttr.offset += sizeof(aiColor4D);
        }
        std::cout << "Colors: vec4 *" << mesh->GetNumColorChannels() << '\n';
    }
    if (mesh->HasTangentsAndBitangents()) {
        curAttr.format = formats[3];
        curAttr.location++;
        vertAttrs.push_back(curAttr);
        curAttr.offset += sizeof(aiVector3D);
        curAttr.location++;
        vertAttrs.push_back(curAttr);
        curAttr.offset += sizeof(aiVector3D);
        std::cout << "Tangents vec3\nBitangents vec3\n";
    }
    stride = curAttr.offset;
    std::cout << "Length of vertex:" << curAttr.offset << '\n';
}
static uint8_t* collectVertexData(const aiMesh* mesh,
                                  uint32_t stride,
                                  uint32_t space) {
    uint8_t *vertData = (uint8_t*)malloc(stride * mesh->mNumVertices),
            *curPos = vertData;
    if (!vertData)
        throw std::bad_alloc();
    for (size_t i = 0; i < mesh->mNumVertices; i++) {
        *((aiVector3D*)curPos) = mesh->mVertices[i];
        curPos += sizeof(aiVector3D);
        if (mesh->HasNormals()) {
            *((aiVector3D*)curPos) = mesh->mNormals[i];
            curPos += sizeof(aiVector3D);
        }
        for (size_t j = 0; j < mesh->GetNumUVChannels(); j++) {
            if (mesh->HasTextureCoords(j)) {
                *((aiVector3D*)curPos) = mesh->mTextureCoords[j][i];
                curPos += sizeof(ai_real) * mesh->mNumUVComponents[j];
            }
        }
        for (size_t j = 0; j < 8 /*mColor的大小*/; j++) {
            if (mesh->HasVertexColors(j)) {
                *((aiColor4D*)curPos) = mesh->mColors[j][i];
                curPos += sizeof(aiColor4D);
            }
        }
        if (mesh->HasTangentsAndBitangents()) {
            *((aiVector3D*)curPos) = mesh->mTangents[i];
            curPos += sizeof(aiVector3D);
            *((aiVector3D*)curPos) = mesh->mBitangents[i];
            curPos += sizeof(aiVector3D);
        }
    }
    uint32_t* data =
        compressData(vertData, (stride * mesh->mNumVertices), space);
    free(vertData);
    return data;
}
static CompressedData* collectIndexData(const aiMesh* mesh) {
    uint8_t *indicesData = malloc(mesh->mNumFaces * sizeof(uint32_t) * 3),
            *curpos = indicesData;
    if (!indicesData)
        throw std::bad_alloc();
    for (size_t i = 0; i < mesh->mNumFaces; i++) {
        for (size_t j = 0; j < 3; j++) {
            *((uint32_t*)curpos) = mesh->mFaces[i].mIndices[j];
            curpos += sizeof(uuint32_t);
        }
    }
    CompressedData* data =
        compressData(vertData, (stride * mesh->mNumVertices));
    free(indicesData);
    return data;
}
static void _makeMeshFile(const aiMesh* mesh,
                          const std::string& storePath,
                          const char* name) {
    if (!mesh->HasFaces()) {
        std::cout << "mesh must has index\nGenerate canceled";
        return;
    }
    MeshFileHead head;
    head._head = MESH_HEAD_CODE;
    head.setName(name);
    std::cout << "File:" << storePath << '\t' << name << '\n';
    MeshFileHead::BufferInfo vertBufInfo;
    std::vector<MeshFileHead::VertexAttr> vertexInfo =
        queryMeshVertexAttr(mesh, vertBufInfo.stride);
    vertBufInfo.binding = 0;
    vertBufInfo.data.start = sizeof(head) +
                             sizeof(vertInfo[0]) * vertexInfo.size() +
                             sizeof(vertBufInfo); /*数据在此处之后都是压缩的*/
    vertBufInfo.attr = Range{0, vertexInfo.size()};
    vertBufInfo.rate = vk::VertexInputRate::eVertex;

    head.vertexBuffers.length = sizeof(vertBufInfo);
    head.vertexBuffers.start = sizeof(head);

    head.vertexAttrs.length = sizeof(vertInfo[0]) * vertexInfo.size();
    head.vertexAttrs.start = sizeof(head) + sizeof(vertBufInfo);

    head.topology = vk::PrimitiveTopology::eTriangleList;
    head.indexType = vk::IndexType::eUint32;
    head.vertexCount = mesh->mNumVertices;
    head.indexCount = mesh->mNumFaces * 3;
    head.restartEnable = 0; /*false*/
    head.restartIndex = 0;

    uint8_t* outData =
        collectVertexData(mesh, vertBufInfo.stride, vertBufInfo.data.start);
    vertBufInfo.data.length =
        ((CompressedData*)(outData + vertBufInfo.data.start))->cprLen +
        8 * 2 /*开头长度*/;
    memcpy(outData, &head, sizeof(head));
    memcpy(outData + sizeof(head), &vertBufInfo, sizeof(vertBufInfo));
    memcpy(outData + sizeof(head) + sizeof(vertBufInfo), vertexInfo.data(),
           head.vertexAttrs.length);
    uint64_t lastOffset = vertBufInfo.data.length + vertBufInfo.data.start;
    head._crc32 = crcCheckSum(0, outData + offsetof(MeshFileHead, nameLen),
                              lastOffset - offsetof(MeshFileHead, nameLen));
    if (mesh->HasFaces()) {
        std::cout << "Faces: triangles, unsigned int * 3\n";
    } else {
        std::cout << "mesh must has index\nGenerate canceled\n";
        return;
    }

    CompressedData* indexData = collectIndexData(mesh);
    head.indexBuffer.length = indexData->cprLen + 8 * 2;
    head.indexBuffer.start = lastOffset;
    lastOffset += head.indexBuffer.length;
    head._crc32 =
        crcCheckSum(head._crc32, (uint8_t*)indexData, head.indexBuffer.length);

    std::cout << "Vertices Count:" << mesh->mNumVertices << '\n';
    std::cout << "Indices Count:" << mesh->mNumFaces * 3 << '\n';
    uint32_t rawSize = vertBufInfo.data.start + 8 * 4 +
                       vertBufInfo.stride * mesh->mNumVertices +
                       sizeof(uint32_t) * 3 * mesh->mNumFaces;
    std::cout << "Raw Data size:" << rawSize << "Bytes\n";
    std::cout << "Compress Data size:" << lastOffset << "Bytes\n";
    std::cout << "Compress Rate:" << rawSize / static_cast<double>(lastOffset)
              << "Bytes\n";
    std::cout << "CRC32:" << std::ios::hex << head._crc32 << "\n";

    std::ofstream out(storePath,
                      std::ios::out | std::ios::binary | std::ios::trunc);
    if (!out.is_open())
        throw std::runtime_error("out file not find!");
    out.write((char*)outData, lastOffset - head.indexBuffer.length);
    out.write((char*)indexData, head.indexBuffer.length);
    out.close();
    free(outData);
    free(indexData);
    std::cout << "DONE;" << std::endl;
}
}  // namespace bl
