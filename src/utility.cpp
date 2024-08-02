#include "utility.hpp"

namespace BL {
void compressData(const void* real_data,
                  uint32_t length,
                  compressed_data** save,
                  uint32_t space,
                  int compress_level) {
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
void uncompressData(const compressed_data* data, void** save) {
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
uint32_t crcCheckSum(uint32_t crc, const uint8_t* data, uint32_t length) {
    uint32_t ncrc = crc32(crc, (Bytef*)data, (uInt)length);
    return ncrc;
}
}  // namespace BL