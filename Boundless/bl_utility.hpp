#ifndef _BL_UTILITY_HPP_FILE_
#define _BL_UTILITY_HPP_FILE_
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <array>
#include <algorithm>
#include <functional>
namespace bl {
uint32_t CRC32(void* data, uint32_t length, uint32_t CRC = (~0));
void huffmanEncode(void* data, uint32_t length, void** output, uint32_t* buflen);
#ifndef NDEBUG
void printCRC32Table();
#endif //!NDEBUG
}  // namespace  bl
#endif  //!_BL_UTILITY_HPP_FILE_