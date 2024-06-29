#ifndef _BL_UTILITY_HPP_FILE_
#define _BL_UTILITY_HPP_FILE_
#include <algorithm>
#include <array>
#include <bitset>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <queue>
#include <stack>
namespace bl {
uint32_t crc32(void* data, uint32_t length, uint32_t CRC = (~0));
#ifndef NDEBUG
void print_crc32_table();
#endif  //! NDEBUG

const bool huffman_debug = true;
struct encode_list {
    std::array<uint16_t, 256> codes;
    std::array<uint16_t, 256> lengths;
};
struct decode_list {
    
};
void byte_freq_count(void* data,
                     uint32_t length,
                     std::array<uint32_t, 256>& output);
enum struct convert_result { Success, BufferOutOfRange };
convert_result encode_data(void* data,
                            uint32_t length,
                            encode_list list,
                            void* buff,
                            uint32_t buffLen,
                            uint32_t& finalLen);
convert_result decode_data(void* data,
                            uint32_t length,
                            encode_list list,
                            void* buff,
                            uint32_t buffLen,
                            uint32_t& finalLen);
class huffman_tree {
   public:
    struct ht_node {
        ht_node *lchild, *rchild;
        uint32_t symbol, weight;
    };
    huffman_tree();
    ~huffman_tree();
    void make_tree(void* data, uint32_t length);
    void generate_code_list(encode_list& list);

   private:
    ht_node* root;
};
}  // namespace  bl
#endif  //!_BL_UTILITY_HPP_FILE_