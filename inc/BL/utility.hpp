#ifndef _BOUNDLESS_UTILITY_FILE_
#define _BOUNDLESS_UTILITY_FILE_
#include <cstdlib>
#include <zlib.h>
#include <cstdint>
#include <log.hpp>
namespace BL {
    struct compressed_data {
        uint32_t real_size;
        uint32_t compress_size;
        uint8_t data[];
    };
    void compressData(const void *real_data,uint32_t length,compressed_data**save,int compress_level = 8);
    void uncompressData(const compressed_data*data,void**save);
} // namespace BL
#endif //!_BOUNDLESS_UTILITY_FILE_