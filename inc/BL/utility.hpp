#ifndef _BOUNDLESS_UTILITY_FILE_
#define _BOUNDLESS_UTILITY_FILE_
#include <zlib.h>
#include <cstdint>
#include <cstdlib>
#include <log.hpp>
namespace BL {
struct compressed_data {
    uint32_t real_size;
    uint32_t compress_size;
    uint8_t data[];
};
void compressData(const void* real_data,
                  uint32_t length,
                  compressed_data** save,
                  uint32_t space = 0,
                  int compress_level = 8);
void uncompressData(const compressed_data* data, void** save);
uint32_t crcCheckSum(uint32_t crc, const uint8_t* data, uint32_t length);
}  // namespace BL
#endif  //!_BOUNDLESS_UTILITY_FILE_