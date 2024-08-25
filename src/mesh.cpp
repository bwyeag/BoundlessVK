#include "mesh.hpp"

namespace BL {
Mesh::Mesh(std::string path, uint32_t baseBinding) {
    load(path, baseBinding);
}
bool Mesh::load(std::string path, uint32_t baseBinding) {
    // 1.open
    std::ifstream inFile(path, std::ios::ate | std::ios::binary);
    if (!inFile.is_open()) {
        print_error("Mesh", "File not found! Path:", path);
        return false;
    }
    size_t length = inFile.tellg();
    // 2.head code
    inFile.seekg(std::ios::beg);
    MeshFileHead fileHead;
    inFile.read((char*)&fileHead._head, sizeof fileHead);
    if (fileHead._head != MESH_HEAD_CODE) {
        inFile.close();
        print_error("Mesh", "File head code error! Path:", path);
        return false;
    }
    name = fileHead.getName();
    // 3.load
    std::vector<Range> loadRanges;
    copy_mesh_info(&fileHead, &info);
    read_buffer_info_list(&fileHead, inFile, info.inputBindings,
                          info.inputAttributes, loadRanges, baseBinding);
    size_t max_length = fileHead.indexBuffer.length;  // 压缩后部分长度最大值
    for (size_t i = 0; i < loadRanges.size(); i++) {
        if (loadRanges[i].length > max_length) {
            max_length = loadRanges[i].length;
        }
    }
    Fence fence;
    fence.create();
    compressed_data* data_ptr = (compressed_data*)malloc(max_length);
    if (!data_ptr)
        throw std::bad_alloc();
    inFile.seekg(fileHead.indexBuffer.offset);
    inFile.read((char*)data_ptr, fileHead.indexBuffer.length);
    indexBuffer.create(data_ptr->real_size);
    TransferBuffer src_buf(data_ptr->real_size);
    uncompress_data(data_ptr, src_buf.get_pdata());
    src_buf.flush();
    VkBufferCopy copy_info = {
        .srcOffset = 0, .dstOffset = 0, .size = data_ptr->real_size};
    src_buf.transfer_to_buffer(CurContext().vulkanInfo.queue_graphics,
                               render_CurContext().cmdBuffer_transfer, indexBuffer,
                               &copy_info, 1, VkFence(fence));
    vertexBuffers.resize(loadRanges.size());
    for (size_t i = 0; i < loadRanges.size(); i++) {
        inFile.seekg(loadRanges[i].offset);
        inFile.read((char*)data_ptr, loadRanges[i].length);
        copy_info.size = data_ptr->real_size;
        vertexBuffers[i].create(data_ptr->real_size);
        fence.wait_and_reset();
        src_buf.resize(data_ptr->real_size);
        uncompress_data(data_ptr, src_buf.get_pdata());
        src_buf.flush();
        src_buf.transfer_to_buffer(CurContext().vulkanInfo.queue_graphics,
                                   render_CurContext().cmdBuffer_transfer,
                                   VkBuffer(vertexBuffers[i]), &copy_info, 1, VkFence(fence));
    }
    fence.wait();
    // 4.free
    free(data_ptr);
    inFile.close();
    return true;
}
void copy_mesh_info(MeshFileHead* pFileData, MeshInfo* info) {
    info->topology = pFileData->topology;
    info->indexType = pFileData->indexType;
    info->vertexCount = pFileData->vertexCount;
    info->indexCount = pFileData->vertexCount;
    info->restartIndex = pFileData->restartIndex;
    info->restartEnable = bool(pFileData->restartEnable);
}
void read_buffer_info_list(
    MeshFileHead* pHead,
    std::ifstream& inFile,
    std::vector<VkVertexInputBindingDescription>& inputBindings,
    std::vector<VkVertexInputAttributeDescription>& inputAttributes,
    std::vector<Range>& loadRanges,
    uint32_t baseBinding) {
    std::vector<MeshFileHead::BufferInfo> vertInfo;
    std::vector<MeshFileHead::VertexAttr> attrInfo;
    // 1.读取顶点缓存信息
    vertInfo.resize(pHead->vertexBuffers.length /
                    sizeof(MeshFileHead::BufferInfo));
    inFile.seekg(pHead->vertexBuffers.offset);
    inFile.read((char*)vertInfo.data(), pHead->vertexBuffers.length);
    loadRanges.resize(vertInfo.size());
    // 2.逐个读取顶点排布
    uint32_t part_length;
    for (size_t i = 0; i < vertInfo.size(); i++) {
        // 3.计算此缓存的绑定信息
        inputBindings.emplace_back(baseBinding + i, vertInfo[i].stride,
                                   vertInfo[i].rate);
        // 4.读取顶点排布
        part_length =
            vertInfo[i].attr.length / sizeof(MeshFileHead::VertexAttr);
        attrInfo.resize(part_length);
        inFile.seekg(vertInfo[i].attr.offset);
        inFile.read((char*)attrInfo.data(),
                    vertInfo[i].attr.length);
        loadRanges[i] = vertInfo[i].data;
        // 5.计算此缓存的顶点排布
        for (size_t j = 0; j < attrInfo.size(); j++) {
            inputAttributes.emplace_back(attrInfo[j].location, baseBinding + i,
                                         attrInfo[j].format,
                                         attrInfo[j].offset);
        }
    }
}
}  // namespace BL
