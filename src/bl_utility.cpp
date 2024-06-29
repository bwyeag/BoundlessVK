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
uint32_t crc32(void* data, uint32_t length, uint32_t CRC) {
    for (uint32_t i = 0; i < length; ++i) {
        CRC = crc32_table[(CRC ^ ((uint8_t*)data)[i]) & 0xff] ^ (CRC >> 8);
    }
    return CRC;
}
#ifndef NDEBUG
void print_crc32_table() {
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
void byte_freq_count(void* data,
                     uint32_t length,
                     std::array<uint32_t, 256>& output) {
    std::memset(output.data(), 0, sizeof(uint32_t) * output.size());
    for (uint32_t i = 0; i < length; ++i) {
        ++output[((uint8_t*)data)[i]];
    }
}
convert_result encode_data(void* data,
                            uint32_t length,
                            encode_list list,
                            void* buff,
                            uint32_t buffLen,
                            uint32_t& finalLen) {
    uint32_t bitBuff = 0, bitBuffLen = 0, outPos = 0;
    for (uint32_t i = 0; i < length; i++) {
        uint8_t symbol = ((uint8_t*)data)[i];
        bitBuff = (bitBuff << list.lengths[symbol]) | list.codes[symbol];
        bitBuffLen += list.lengths[symbol];
        finalLen += list.lengths[symbol];
        if (bitBuffLen > 8) {
            if (outPos >= buffLen)
                return convert_result::BufferOutOfRange;
            bitBuffLen -= 8;
            ((uint8_t*)buff)[outPos++] =
                (bitBuff & (0xff << (bitBuffLen))) >> (bitBuffLen);
            bitBuff &= ~(~0 << (bitBuffLen));
        }
    }
    if (bitBuffLen > 0) {
        if (outPos >= buffLen)
            return convert_result::BufferOutOfRange;
        ((uint8_t*)buff)[outPos++] = (bitBuff<<(8-bitBuffLen)) & 0xff;
    }
    return convert_result::Success;
}
huffman_tree::huffman_tree() : root(nullptr) {}
static void tree_destroy(huffman_tree::ht_node* root) {
    using node = huffman_tree::ht_node;
    struct st_item {
        node* ptr;
        bool visited;
    };
    if (root) {
        std::stack<st_item> ptrst;
        ptrst.push({root, false});
        while (!ptrst.empty()) {
            st_item& cur = ptrst.top();
            if (cur.visited || !cur.ptr->rchild && !cur.ptr->lchild) {
                free(cur.ptr);
                ptrst.pop();
            } else {
                if (cur.ptr->rchild)
                    ptrst.push({cur.ptr->rchild, false});
                if (cur.ptr->lchild)
                    ptrst.push({cur.ptr->lchild, false});
                cur.visited = true;
            }
        }
    }
}
huffman_tree::~huffman_tree() {
    tree_destroy(root);
}
struct _huffman_tree_cmp {
    bool operator()(const huffman_tree::ht_node* a,
                    const huffman_tree::ht_node* b) {
        return a->weight > b->weight;
    }
};
void huffman_tree::make_tree(void* data, uint32_t length) {
    std::array<uint32_t, 256> weight;
    byte_freq_count(data, length, weight);
    std::priority_queue<ht_node*, std::vector<ht_node*>, _huffman_tree_cmp>
        huffman;
    for (int i = 0; i < 256; i++) {
        if (weight[i] > 0) {
            ht_node* node = (ht_node*)malloc(sizeof(ht_node));
            if (!node)
                throw std::bad_alloc();
            node->lchild = node->rchild = nullptr;
            node->symbol = i;
            node->weight = weight[i];
            huffman.push(node);
        }
    }
    if constexpr (huffman_debug) {
        std::cout << "huffman tree on address:" << this
                  << "\nsymbol count:" << huffman.size()
                  << "; symbols count:\n";
        for (uint32_t i = 0; i < 256; i++) {
            if (weight[i] > 0) {
                std::cout << "0x" << std::hex << i << "\t: " << std::dec
                          << weight[i] << '\n';
            }
        }
    }
    while (huffman.size() > 1) {
        ht_node* left = huffman.top();
        huffman.pop();
        ht_node* right = huffman.top();
        huffman.pop();
        ht_node* node = (ht_node*)malloc(sizeof(ht_node));
        if (!node)
            throw std::bad_alloc();
        node->lchild = left;
        node->rchild = right;
        node->symbol = ~0;
        node->weight = left->weight + right->weight;
        huffman.push(node);
    }
    root = huffman.top();
}
static void generate_huffman_code(uint16_t* codes,
                                  uint16_t* lens,
                                  huffman_tree::ht_node* r,
                                  uint16_t c,
                                  uint16_t top) {
    if (r->lchild) {
        generate_huffman_code(codes, lens, r->lchild, (c << 1), top + 1);
        if (r->rchild) {
            generate_huffman_code(codes, lens, r->rchild, (c << 1) | 1,
                                  top + 1);
        }
    } else {
        if (r->rchild) {
            generate_huffman_code(codes, lens, r->rchild, (c << 1) | 1,
                                  top + 1);
        } else {
            lens[r->symbol] = top;
            codes[r->symbol] = c;
            if constexpr (huffman_debug) {
                std::cout << "0x" << std::hex << r->symbol << '('
                          << char(r->symbol) << ')';
                std::cout << "\t0b" << std::bitset<8>(c) << '\t' << std::dec
                          << top << '\n';
            }
        }
    }
}
void huffman_tree::generate_code_list(encode_list& list) {
    if constexpr (huffman_debug) {
        std::cout << "symbol-code list:\nsymbol\tcode\t\tlength\n";
    }
    std::memset(list.codes.data(), 0, 2 * 256);
    std::memset(list.lengths.data(), 0, 2 * 256);
    if (root) {
        if (!root->lchild&&!root->rchild) {
            list.lengths[root->symbol] = 1;
        } else {
            generate_huffman_code(list.codes.data(), list.lengths.data(), root, 0, 0);
        }
    }
}
}  // namespace bl
