#ifndef _BOUNDLESS_BIN_FILE_CXX_HPP_
#define _BOUNDLESS_BIN_FILE_CXX_HPP_
#include <zlib.h>
#include <cstdint>
#include <fstream>
#include <vector>
#include "bl_log.hpp"
#include "bl_utility.hpp"
namespace BL::File {
struct HeadBlock {
    uint32_t head, type, crc32;
};
template <size_t len>
struct StringBlock {
    uint32_t length;
    char str[len];
};
struct ReferenceBlock {
    uint32_t offset;
    uint32_t size;
    uint32_t refOffset;
};
struct CompressedBlock {
    uint32_t realSize, compressSize;
    uint8_t data[];
};
class FileReader {
    std::ifstream _file;
    uint32_t crc32;

   public:
    FileReader() = default;
    FileReader(std::string path, uint32_t head, uint32_t type) {
        _file.open(path, std::ios::binary);
        if (!_file.is_open())
            print_error("FileReader", "Could not open file:", path);
        HeadBlock h;
        _file.read((char*)h, sizeof(h));
        if (_file.bad())
            print_error("FileReader", "Read Error!");
        if (h.head != head || h.type != type)
            print_error("FileReader", "File head error!");
    }
    FileReader(const FileReader&) = delete;
    FileReader(FileReader&& other) { _file = std::move(other._file); }
    std::ifstream& file() { return file; }
    void close() {
        crc32 = (~0u);
        _file.close();
    }
    template <size_t len>
    void read(StringBlock<len>* bp) {
        _file.read((char*)bp, sizeof(*bp));
    }
    template <typename T>
    void read(T* bp) {
        _file.read((char*)bp, sizeof(*bp));
    }
    void read(ReferenceBlock* bp) {
        _file.read((char*)bp, sizeof(uint32_t) * 2);
    }
    void read(CompressedBlock* bp, uint8_t** save) {
        _file.read((char*)bp, sizeof(*bp));
        uint8_t* data = new uint8_t[bp->compressSize];
        uint8_t* realData = new uint8_t[bp->realSize];
        _file.read((char*)data, bp->compressSize);
        uint32_t destLen = bp->realSize;
        int r = uncompress((Bytef*)realData, (uLongf*)&destLen, (Bytef*)data,
                           (uLong)bp->compress_size);
        if (r != Z_OK) {
            *save = nullptr;
            print_error("FileReader", "Uncompressing failed! Code:", r);
        }
        delete[] data;
        *save = realData;
    }
    template <size_t len>
    void read(StringBlock<len>* bp, ReferenceBlock* ref) {
        _file.seekg(ref->offset);
        _file.read((char*)bp, std::min(sizeof(*bp), ref->size));
    }
    template <typename T>
    void read(T* bp, ReferenceBlock* ref) {
        _file.seekg(ref->offset);
        _file.read((char*)bp, std::min(sizeof(*bp), ref->size));
    }
    void read(ReferenceBlock* bp, ReferenceBlock* ref) {
        _file.seekg(ref->offset);
        _file.read((char*)bp, sizeof(uint32_t) * 2);
    }
    void read(CompressedBlock* bp, uint8_t** save, ReferenceBlock* ref) {
        _file.seekg(ref->offset);
        read(bp, save);
    }
    template <size_t len>
    void read_array(StringBlock<len>* bp, ReferenceBlock* ref) {
        _file.seekg(ref->offset);
        _file.read((char*)bp, ref->size);
    }
    template <typename T>
    void read_array(T* bp, ReferenceBlock* ref) {
        _file.seekg(ref->offset);
        _file.read((char*)bp, ref->size);
    }
    void read_array(ReferenceBlock* bp, ReferenceBlock* ref) {
        _file.seekg(ref->offset);
        for (size_t i = 0; i < ref->size / sizeof(*bp); i++) {
            _file.read((char*)&bp[i], sizeof(uint32_t) * 2);
        }
    }
};
class FileWriter {
    std::ofstream _file;
    std::vector<uint8_t> _temp;
    uint32_t crc32 = (~0u);
    uint32_t curEof = 0;

   public:
    FileWriter() = default;
    FileWriter(std::string path, uint32_t head, uint32_t type) {
        _file.open(path, std::ios::binary | std::ios::trunc);
        if (!_file.is_open())
            print_error("FileWriter", "Could not open file:", path);
        HeadBlock h{.head = head, .type = type};
        _file.write((char*)h, sizeof(h));
        curEof += sizeof(h);
        if (_file.bad())
            print_error("FileWriter", "Write Error!");
    }
    FileWriter(const FileWriter&) = delete;
    FileWriter(FileWriter&& other) { _file = std::move(other._file); }
    void flush() {
        curEof += _temp.size();
        crc32 = calcCRC32(crc32, _temp.data(), _temp.size());
        _file.write((char*)_temp.data(), _temp.size());
        _temp.clear();
    }
    void reserve(uint32_t size) { _temp.reserve(size); }
    void close() {
        flush();
        _file.seekp(2 * sizeof(uint32_t));  // 移动到文件开头头部的CRC32位置
        _file.write((char*)&crc32, sizeof(crc32));
        _file.close();
    }
    uint32_t tellp() { return curEof + _temp.size(); }
    template <size_t len>
    void write(const StringBlock<len>* bp) {
        size_t st = _temp.size();
        _temp.resize(st + sizeof(*bp));
        memcpy((void*)_temp.data() + st, (void*)bp, sizeof(*bp));
    }
    template <typename T>
    void write(T* bp) {
        size_t st = _temp.size();
        _temp.resize(st + sizeof(*bp));
        memcpy((void*)_temp.data() + st, (void*)bp, sizeof(*bp));
    }
    void addRef(ReferenceBlock* bp) {
        bp->refOffset = _temp.size();
        _temp.resize(_temp.size() + sizeof(uint32_t) * 2);
    }
    void write(ReferenceBlock* bp) {
        memcpy((void*)_temp.data() + bp->refOffset, (void*)bp,
               sizeof(uint32_t) * 2);
    }
    void write(const uint8_t* data, uint32_t size, int compress_level = 7) {
        uint32_t allocSize = compressBound(size);
        size_t st = _temp.size();
        _temp.resize(st + allocSize);
        int r = compress2((Bytef*)_temp.data() + st, (uLongf*)&allocSize,
                          (Bytef*)data, (uLong)size, compress_level);
        if (r != Z_OK) {
            print_error("FileWriter", "Compressing failed! Code:", r);
            return;
        }
        _temp.resize(st + allocSize);
    }
};
}  // namespace BL::File
#endif  //!_BOUNDLESS_BIN_FILE_CXX_HPP_