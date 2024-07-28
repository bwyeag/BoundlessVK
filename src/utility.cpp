#include "utility.hpp"

namespace BL {
void compressData(const void* real_data,
                  uint32_t length,
                  compressed_data** save,
                  int compress_level) {
    uint32_t alloc_length = compressBound(length) + sizeof(compressed_data);
    *save = (compressed_data*)malloc(alloc_length);
    if (*save == nullptr) {
        print_error("compress", "malloc() failed!");
        return;
    }
    (*save)->real_size = length;
    (*save)->compress_size = alloc_length - sizeof(compressed_data);
    int r = compress2((Bytef*)&(*save)->data, (uLongf*)&(*save)->compress_size,
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
}  // namespace BL