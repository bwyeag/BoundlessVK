#include "bl_utility.hpp"
using namespace bl;
#include <string>

int main() {
    std::cout.sync_with_stdio(false);
    std::cin.sync_with_stdio(false);
    std::string string;
    std::cin >> string;
    char out[string.size()*2] = {0};
    huffman_tree t;
    t.make_tree(string.data(), sizeof(char)*string.size());
    encode_list convlist;
    t.generate_code_list(convlist);
    uint32_t len = 0;
    encode_data(string.data(),sizeof(char)*string.size(),convlist,out,string.size()*2,len);
    std::cout << "result:\nlength(bit):"<< len <<"\n0x";
    std::cout.fill('0');
    for (int i = 0; i < string.size()*2; i++)
    {
        std::cout << std::hex << std::setw(2) << uint32_t(out[i]&0xff);
    }
}