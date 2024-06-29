#include "bl_utility.hpp"

namespace bl {
static constexpr std::array<uint32_t, 256> _calcCRC32Table() {
    std::array<uint32_t, 256> tbl;
    uint32_t CRC;
    for (int i = 0; i < 256; ++i) {
        CRC = i;
        for (int j = 0; j < 8; ++j) {
            if (CRC & 1)
                CRC = (CRC >> 1) ^ 0xEDB88320;
            else
                CRC >>= 1;
        }
        tbl[i] = CRC;
    }
    return tbl;
}
static constexpr std::array<uint32_t, 256> crc32_table = _calcCRC32Table();
uint32_t CRC32(void* data, uint32_t length, uint32_t CRC) {
    for (uint32_t i = 0; i < length; ++i) {
        CRC = crc32_table[(CRC ^ ((uint8_t*)data)[i]) & 0xff] ^ (CRC >> 8);
    }
    return CRC;
}
struct huffman_node {
    uint32_t symbol;  // 符号
    uint32_t freq;    // 频率
    huffman_node *left, *right;
};
void geneHuffmanCode(uint16_t *codes, uint16_t *lens, huffman_node *r,uint16_t c, uint16_t top){
    if (r->left) geneHuffmanCode(codes,lens,r->left,(c<<1),top+1);
    if (r->right) geneHuffmanCode(codes,lens,r->right,(c<<1)|1,top+1);
    if (!r->left&&!r->right) {
        lens[r->symbol] = top;
        codes[r->symbol] = c;
    }
}
void huffmanEncode(void* data,
                   uint32_t length,
                   void** output,
                   uint32_t* buflen) {
    // 步骤1：统计字符频率
    uint32_t frequency[256];
    std::memset(frequency, 0, sizeof(frequency));
    for (uint32_t i = 0; i < length; ++i)
        ++frequency[((uint8_t*)data)[i]];
    huffman_node* root;
    {
        //  步骤2：构建小端堆
        huffman_node* min_heap[256];
        int n = 0;
        for (int i = 0; i < 256; i++) {
            if (frequency[i] > 0) {
                min_heap[n] = new huffman_node();
                min_heap[n]->symbol = i;
                min_heap[n]->freq = frequency[i];
                min_heap[n]->left = min_heap[n]->right = nullptr;
                ++n;
            }
        }
        auto cmp = [](huffman_node* r, huffman_node* s) {
            return r->freq > s->freq;
        };
        std::make_heap(min_heap, min_heap + n, cmp);
        //  步骤3：排序创建哈夫曼树
        while (n > 1) {
            huffman_node* left = std::pop_heap(min_heap, min_heap + n--, cmp);
            huffman_node* right = std::pop_heap(min_heap, min_heap + n--, cmp);
            huffman_node* node = new huffman_node();
            node->freq = left->freq + right->freq;
            node->symbol = 0;
            node->right = right;
            node->left = left;
            std::push_heap(min_heap, min_heap + n++, cmp);
        }
        root = min_heap[0];
    }
    // 步骤4：创建编码表
    std::memset(frequency, 0, sizeof(frequency));
    uint16_t *codes = (uint16_t*)frequency;
    uint16_t *lens = (uint16_t*)((uint8_t*)frequency + sizeof(frequency)/2);
    geneHuffmanCode(codes,lens,root,0,0);
    // 步骤5：编码
    
}
#ifndef NDEBUG
void printCRC32Table() {
    std::cout.setf(std::ios::hex, std::ios::basefield);
    std::cout.fill('0');
    for (int i = 0; i < crc32_table.size(); ++i) {
        std::cout << "0x" << std::setw(2) << std::right << i;
        std::cout << ":0x" << std::setw(8) << std::right << crc32_table[i]
                  << '\t';
    }
    std::cout.fill(' ');
    std::cout.unsetf(std::ios::basefield);
}
#endif  //! NDEBUG
}  // namespace bl
