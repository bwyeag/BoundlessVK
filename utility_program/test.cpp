#include "bl_log.hpp"
#include "bl_JSON.hpp"
using namespace BL;
using namespace BL::JSON;
int main() {
    std::string json;
    std::getline(std::cin,json);
    auto j = parse(json);
    std::cout << "parse done! [" << j.second << "]\n";
    auto new_str = dump(j.first);
    std::cout << new_str << '\n';
    system("pause");
}