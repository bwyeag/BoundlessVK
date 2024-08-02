#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <vulkan/vulkan.h>
#include <zlib.h>
#include <assimp/Importer.hpp>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "BL/ftypes.hpp"
#include "BL/log.hpp"
// command:
// g++ command_program.cpp -ID:\c++programs\BoundlessVK\BoundlessVK\inc
// -ID:\vulkanSDK\Include
// -LD:\c++programs\BoundlessVK\BoundlessVK\utility_program -lzlib -lassimp -O3
// -oBLC
using namespace BL;
struct compressed_data {
    uint32_t real_size;
    uint32_t compress_size;
    uint8_t data[];
};
void compress_data(const void* real_data,
                   uint32_t length,
                   compressed_data** save,
                   uint32_t space,
                   int compress_level = 8) {
    uint32_t alloc_length = compressBound(length) + sizeof(compressed_data);
    *save = (compressed_data*)malloc(alloc_length + space);
    if (*save == nullptr) {
        print_error("compress", "malloc() failed!");
        return;
    }
    compressed_data* dataStart = (compressed_data*)((uint8_t*)(*save) + space);
    dataStart->real_size = length;
    dataStart->compress_size = alloc_length - sizeof(compressed_data);
    int r =
        compress2((Bytef*)&dataStart->data, (uLongf*)&dataStart->compress_size,
                  (Bytef*)real_data, (uLong)length, compress_level);
    if (r != Z_OK) {
        free(*save);
        *save = nullptr;
        print_error("compress", "Compressing failed! Code:", r);
    }
}
void uncompress_data(const compressed_data* data, void** save) {
    uint32_t size = data->real_size;
    *save = malloc(size);
    if (*save == nullptr) {
        print_error("compress", "malloc() failed!");
        return;
    }
    int r = uncompress((Bytef*)&(*save), (uLongf*)&size, (Bytef*)data->data,
                       (uLong)data->compress_size);
    if (r != Z_OK) {
        free(*save);
        *save = nullptr;
        print_error("compress", "Uncompressing failed! Code:", r);
    }
}
uint32_t crc_check(uint32_t crc, const uint8_t* data, uint32_t length) {
    uint32_t ncrc = crc32(crc, (Bytef*)data, (uInt)length);
    return ncrc;
}
enum struct Order {
    Quit,
    ShowMenu,
    GeneVertexCode,
    ExpressionCompute,
    CreateShader,
    CreateModel
};
static const std::map<std::string, Order> to_order_enum{
    {"q", Order::Quit},
    {"quit", Order::Quit},
    {"h", Order::ShowMenu},
    {"help", Order::ShowMenu},
    {"ep", Order::ExpressionCompute},
    {"expression", Order::ExpressionCompute},
    {"s", Order::CreateShader},
    {"shader", Order::CreateShader},
    {"m", Order::CreateModel},
    {"model", Order::CreateModel}};
void print_menu();
void generate_VertexCode();
void generate_Shader();
compressed_data* collectIndexData(const aiMesh* mesh);
void _makeMeshFile(const aiMesh* mesh,
                   const std::string& storePath,
                   const char* name);
std::vector<MeshFileHead::VertexAttr> queryMeshVertexAttr(const aiMesh* mesh,
                                                          uint32_t& stride);
void generate_Model();
void expression_compute();
void trim(std::string& s);
int main() {
    std::cout << "BL Command\n";
    // print_menu();
    std::string order;
    while (true) {
        std::cout << ">>";
        std::getline(std::cin, order);
        trim(order);
        auto it = to_order_enum.find(order);
        if (it == to_order_enum.end()) {
            std::cout << "invalid order:" << order << '\n';
        } else {
            switch (it->second) {
                case Order::Quit:
                    std::cout << "Quit." << std::endl;
                    exit(0);
                case Order::ShowMenu:
                    print_menu();
                    break;
                case Order::CreateShader:
                    try {
                        generate_Shader();
                    } catch (const std::exception& e) {
                        std::cerr << "exception rised:\n" << e.what() << '\n';
                    }
                    break;
                case Order::CreateModel:

                    try {
                        generate_Model();
                    } catch (const std::exception& e) {
                        std::cerr << "exception rised:\n" << e.what() << '\n';
                    }
                    break;
                case Order::ExpressionCompute:
                    try {
                        expression_compute();
                    } catch (const std::exception& e) {
                        std::cerr << "exception rised:\n" << e.what() << '\n';
                    }
                    break;
            }
        }
        std::cin.ignore();
    }
}
void trim(std::string& s) {
    s.erase(0, s.find_first_not_of(' '));
    s.erase(s.find_last_not_of(' ') + 1);
}
void tolower_str(std::string& s) {
    char* d = s.data();
    for (size_t i = 0; i < s.size(); i++) {
        d[i] = tolower(d[i]);
    }
}
void toupper_str(std::string& s) {
    char* d = s.data();
    for (size_t i = 0; i < s.size(); i++) {
        d[i] = toupper(d[i]);
    }
}
void print_menu() {
    static const char* str_menu =
        "0.q/quit:quit\n1.h/help: print menu "
        "\n2.ep/expression: mathexpression compute\n3.s/shader: create "
        "shader file\n4.m/model: create model file\n";
    std::cout << str_menu;
}

void createShaderFile(const std::string& path,
                      const std::vector<std::string>& data,
                      const std::vector<VkShaderStageFlagBits>& stages);
void generate_Shader() {
    static std::map<std::string, VkShaderStageFlagBits> stage{
        {"vertex", VK_SHADER_STAGE_VERTEX_BIT},
        {"fragment", VK_SHADER_STAGE_FRAGMENT_BIT},
        {"geometry", VK_SHADER_STAGE_GEOMETRY_BIT},
        {"tesscontrol", VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT},
        {"tessevaluation", VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT}};
    static std::map<VkShaderStageFlagBits, std::string> cmd_stage_str{
        {VK_SHADER_STAGE_VERTEX_BIT, "vertex"},
        {VK_SHADER_STAGE_FRAGMENT_BIT, "fragment"},
        {VK_SHADER_STAGE_GEOMETRY_BIT, "geometry"},
        {VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, "tesscontrol"},
        {VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, "tesseval"}};
    std::cout << "stages:vertex,fragment,geometry,tesscontrol,tessevaluation\n";
    std::cout << "Create Shader:\nEnter save path('quit' to cancel):";
    std::string path;
    std::getline(std::cin, path);
    trim(path);
    tolower_str(path);
    if (path == "quit")
        return;
    std::cout << "Enter File('null' to stop) paths:";
    std::vector<std::string> fpaths;
    std::vector<VkShaderStageFlagBits> stages;
    std::string fpath, fstage, tmp;
    while (true) {
        std::getline(std::cin, fpath);
        trim(fpath);
        // 检查是否为退出指令
        if (fpath.size() <= 4) {
            tmp = fpath;
            tolower_str(tmp);
            if (tmp == "null") {
                break;
            } else if (tmp == "quit") {
                std::cout << "Canceled\n";
                return;
            }
        }
        // 检查着色器类型
        VkShaderStageFlagBits cur_stage;
        if (fpath.find(".vert") != std::string::npos) {
            cur_stage = VK_SHADER_STAGE_VERTEX_BIT;
            std::cout << "auto identified file type:" << "vertex\n";
        } else if (fpath.find(".frag") != std::string::npos) {
            cur_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            std::cout << "auto identified file type:" << "fragment\n";
        } else if (fpath.find(".geom") != std::string::npos) {
            cur_stage = VK_SHADER_STAGE_GEOMETRY_BIT;
            std::cout << "auto identified file type:" << "geometry\n";
        } else if (fpath.find(".tesc") != std::string::npos) {
            cur_stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            std::cout << "auto identified file type:"
                      << "tessellation_control\n";
        } else if (fpath.find(".tese") != std::string::npos) {
            cur_stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            std::cout << "auto identified file type:"
                      << "tessellation_evaluation\n";
        } else {
            std::cout << "Enter Type:";
            std::getline(std::cin, fstage);
            trim(fstage);
            tolower_str(fstage);
            cur_stage = stage[fstage];
        }
        // 自动编译着色器
        static std::stringstream sscmd;
        size_t postfix = fpath.find(".spv");
        if (postfix == std::string::npos) {
            sscmd << "glslc.exe " << fpath << " -c -o" << fpath << ".spv"
                  << " -fshader-stage=" << cmd_stage_str[cur_stage];
            system(sscmd.str().c_str());
            sscmd.str("");
            fpath += ".spv";
            std::cout << "auto compile file to:" << fpath << '\n';
        }
        stages.push_back(cur_stage);
        fpaths.push_back(fpath);
        std::cout << "Enter:";
    }
    createShaderFile(path, fpaths, stages);
    std::cout << "Create Success.\n";
}
void createShaderFile(const std::string& save_path,
                      const std::vector<std::string>& fpaths,
                      const std::vector<VkShaderStageFlagBits>& stages) {
    std::ofstream fout(save_path, std::ios::binary | std::ios::trunc);
    uint32_t code = BL::SHADER_HEAD_CODE;
    fout.write((char*)&code, 4);
    code = fpaths.size();
    fout.write((char*)&code, 4);
    size_t st = fout.tellp();
    fout.seekp(sizeof(BL::ShaderFileHead::Part) * fpaths.size(), std::ios::cur);
    BL::ShaderFileHead::Part write[fpaths.size()];
    for (uint32_t i = 0; i < fpaths.size(); i++) {
        std::ifstream in_file(fpaths[i],
                              std::ios::binary | std::ios::in | std::ios::ate);
        write[i].length = in_file.tellg();
        write[i].start = fout.tellp();
        write[i].stage = stages[i];
        char* tmp = new char[write[i].length];
        in_file.seekg(std::ios::beg);
        in_file.read(tmp, write[i].length);
        fout.write(tmp, write[i].length);
        delete tmp;
        in_file.close();
    }
    fout.seekp(st);
    fout.write((char*)write, sizeof(BL::ShaderFileHead::Part) * fpaths.size());
    fout.close();
}
void makeModelFile(const std::string& path, const std::string& storePath) {
    Assimp::Importer importer;
    const aiScene* scene =
        importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode) {
        throw std::runtime_error(importer.GetErrorString());
    }
    for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
        const char* name = scene->mMeshes[i]->mName.C_Str();
        _makeMeshFile(scene->mMeshes[i], storePath + "_" + name + ".mesh",
                      name);
    }
}
std::vector<MeshFileHead::VertexAttr> queryMeshVertexAttr(const aiMesh* mesh,
                                                          uint32_t& stride) {
    static VkFormat formats[4]{VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT,
                               VK_FORMAT_R32G32B32_SFLOAT,
                               VK_FORMAT_R32G32B32A32_SFLOAT};
    std::vector<MeshFileHead::VertexAttr> vertAttrs;
    MeshFileHead::VertexAttr curAttr;
    curAttr.format = formats[3];
    curAttr.location = 0;
    curAttr.offset = 0;
    vertAttrs.push_back(curAttr);
    curAttr.offset += sizeof(aiVector3D);
    std::stringstream stm_code;
    stm_code << "struct Vertex {\n\tvec3f position;\n";
    std::cout << "Vertex Format:\nPositions vec3\n";
    if (mesh->HasNormals()) {
        curAttr.format = formats[3];
        curAttr.location++;
        vertAttrs.push_back(curAttr);
        curAttr.offset += sizeof(aiVector3D);
        std::cout << "Normals vec3\n";
        stm_code << "\tvec3f normal;\n";
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
                stm_code << "\tvec" << mesh->mNumUVComponents[i] << "f texture_"
                         << i << "; // name:"
                         << (mesh->HasTextureCoordsName(i)
                                 ? mesh->mTextureCoordsNames[i]->C_Str()
                                 : "NULL")
                         << '\n';
            }
        }
    }
    if (mesh->GetNumColorChannels() > 0) {
        curAttr.format = formats[4];
        for (uint32_t i = 0; i < mesh->GetNumColorChannels(); i++) {
            curAttr.location++;
            vertAttrs.push_back(curAttr);
            curAttr.offset += sizeof(aiColor4D);
            stm_code << "\tvec4f color_" << i << ";\n";
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
        stm_code << "\tvec3f tangent;\n\tvec3f bitangent;\n";
    }
    stride = curAttr.offset;
    std::cout << "Length of vertex:" << curAttr.offset << '\n';
    stm_code << "};\n";
    std::cout << "vertex struct:\n";
    std::cout << stm_code.str() << '\n';
    return vertAttrs;
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
    uint8_t* data;
    compress_data(vertData, (stride * mesh->mNumVertices),
                  (compressed_data**)&data, space);
    free(vertData);
    return data;
}

compressed_data* collectIndexData(const aiMesh* mesh) {
    uint8_t *indicesData =
                (uint8_t*)malloc(mesh->mNumFaces * sizeof(uint32_t) * 3),
            *curpos = indicesData;
    if (!indicesData)
        throw std::bad_alloc();
    for (size_t i = 0; i < mesh->mNumFaces; i++) {
        for (size_t j = 0; j < 3; j++) {
            *((uint32_t*)curpos) = mesh->mFaces[i].mIndices[j];
            curpos += sizeof(uint32_t);
        }
    }
    compressed_data* data;
    compress_data(indicesData, mesh->mNumFaces * sizeof(uint32_t) * 3, &data,
                  0);
    free(indicesData);
    return data;
}
void _makeMeshFile(const aiMesh* mesh,
                   const std::string& storePath,
                   const char* name) {
    std::cout << "\nFile:" << storePath << '\t' << name << '\n';
    if (!mesh->HasFaces()) {
        std::cout << "mesh must has index\nGenerate canceled";
        return;
    }
    // head | vertBufInfo | vertexInfo[](attr) | vertBufInfo.data | vertexData |
    // indicesData
    MeshFileHead head;
    head._head = MESH_HEAD_CODE;
    head.setName(name);

    MeshFileHead::BufferInfo vertBufInfo;

    std::vector<MeshFileHead::VertexAttr> vertexInfo =
        queryMeshVertexAttr(mesh, vertBufInfo.stride);

    vertBufInfo.binding = 0;
    vertBufInfo.data.offset =
        sizeof(head) + sizeof(vertBufInfo) +
        sizeof(vertexInfo[0]) * vertexInfo.size(); /*数据在此处之后都是压缩的*/
    vertBufInfo.attr = {sizeof(head) + sizeof(vertBufInfo),
                        uint32_t(sizeof(vertexInfo[0]) * vertexInfo.size())};
    vertBufInfo.rate = VK_VERTEX_INPUT_RATE_VERTEX;

    head.vertexBuffers.length = sizeof(vertBufInfo) * 1;
    head.vertexBuffers.offset = sizeof(head);

    head.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    head.indexType = VK_INDEX_TYPE_UINT32;
    head.vertexCount = mesh->mNumVertices;
    head.indexCount = mesh->mNumFaces * 3;
    head.restartEnable = 0; /*false*/
    head.restartIndex = 0;

    uint8_t* outData =
        collectVertexData(mesh, vertBufInfo.stride, vertBufInfo.data.offset);
    vertBufInfo.data.length =
        ((compressed_data*)(outData + vertBufInfo.data.offset))->compress_size +
        sizeof(compressed_data) /*开头长度*/;
    memcpy(outData + sizeof(head), &vertBufInfo, sizeof(vertBufInfo));
    memcpy(outData + sizeof(head) + sizeof(vertBufInfo), vertexInfo.data(),
           vertBufInfo.attr.length);
    uint64_t lastOffset = vertBufInfo.data.length + vertBufInfo.data.offset;
    std::cout << "Basical infomation:\n";
    if (mesh->HasFaces()) {
        std::cout << "Faces: triangles, unsigned int * 3\n";
    } else {
        std::cout << "mesh must has index\nGenerate canceled\n";
        free(outData);
        return;
    }

    compressed_data* indexData = collectIndexData(mesh);
    head.indexBuffer.length =
        indexData->compress_size + sizeof(compressed_data);
    head.indexBuffer.offset = lastOffset;
    memcpy(outData, &head, sizeof(head));
    head._crc32 =
        crc_check(UINT32_MAX, outData + offsetof(MeshFileHead, nameLen),
                  lastOffset - offsetof(MeshFileHead, nameLen));
    lastOffset += head.indexBuffer.length;
    head._crc32 =
        crc_check(head._crc32, (uint8_t*)indexData, head.indexBuffer.length);
    memcpy(outData, &head, offsetof(MeshFileHead, nameLen));

    std::cout << "Vertices Count:" << mesh->mNumVertices << '\n';
    std::cout << "Indices Count:" << mesh->mNumFaces * 3 << '\n';
    uint32_t rawSize = vertBufInfo.data.offset + sizeof(compressed_data) * 2 +
                       vertBufInfo.stride * mesh->mNumVertices +
                       sizeof(uint32_t) * 3 * mesh->mNumFaces;
    std::cout << "Raw Data size:" << rawSize << "Bytes\n";
    std::cout << "Compress Data size:" << lastOffset << "Bytes\n";
    std::cout << "Compress Rate:" << rawSize / static_cast<double>(lastOffset)
              << '\n';
    std::hex(std::cout);
    std::cout << "CRC32:" << head._crc32 << "\n";
    std::dec(std::cout);
    std::ofstream out(storePath,
                      std::ios::out | std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        free(outData);
        free(indexData);
        throw std::runtime_error("out file not find!");
    }
    out.write((char*)outData, lastOffset - head.indexBuffer.length);
    out.write((char*)indexData, head.indexBuffer.length);
    out.close();
    free(outData);
    free(indexData);
    std::cout << "DONE;" << std::endl;
}
void generate_Model() {
    std::string path, outPath, t;
    std::cout << "Enter model file(type 'quit' to quit):\n";
    std::getline(std::cin, path);
    trim(path);
    t = path;
    tolower_str(t);
    if (t == "quit") {
        std::cout << "canceled\n";
        return;
    }
    std::cout << "Enter out file(type null to ignore or quit to quit):\n";
    std::getline(std::cin, outPath);
    trim(outPath);
    t = outPath;
    tolower_str(t);
    if (t == "quit") {
        std::cout << "canceled\n";
        return;
    } else if (t == "null") {
        outPath = path;
    }
    makeModelFile(path, outPath);
}
void expression_compute() {
    std::cout << "to do function.\n";
}