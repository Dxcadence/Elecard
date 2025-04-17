#include "BMPReader.h"
#include <fstream>
#include <iostream>

bool BMPReader::load(const std::string& filename, int& width, int& height, std::vector<RGB>& data) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Cannot open file: " << filename << std::endl;
        return false;
    }

    // BMP file header (14 bytes)
    uint16_t fileType;
    file.read(reinterpret_cast<char*>(&fileType), sizeof(fileType));
    if (fileType != 0x4D42) {
        std::cerr << "Error: Not a BMP file\n";
        return false;
    }

    file.ignore(8); // skip file size + reserved
    uint32_t dataOffset;
    file.read(reinterpret_cast<char*>(&dataOffset), sizeof(dataOffset));

    // DIB header 
    uint32_t dibHeaderSize;
    file.read(reinterpret_cast<char*>(&dibHeaderSize), sizeof(dibHeaderSize));
    if (dibHeaderSize < 40) {
        std::cerr << "Error: Unsupported DIB header size: " << dibHeaderSize << std::endl;
        return false;
    }

    int32_t w, h;
    uint16_t planes, bitCount;
    uint32_t compression;

    file.read(reinterpret_cast<char*>(&w), sizeof(w));
    file.read(reinterpret_cast<char*>(&h), sizeof(h));
    file.read(reinterpret_cast<char*>(&planes), sizeof(planes));
    file.read(reinterpret_cast<char*>(&bitCount), sizeof(bitCount));
    file.read(reinterpret_cast<char*>(&compression), sizeof(compression));

    if (bitCount != 24 || compression != 0) {
        std::cerr << "Error: Only uncompressed 24-bit BMP supported\n";
        return false;
    }

    // Skip the rest of the DIB header (if larger than 40 bytes)
    if (dibHeaderSize > 40) {
        file.seekg(dibHeaderSize - 40, std::ios::cur);
    }

    width = w;
    height = h;
    int row_padded = (width * 3 + 3) & (~3);

    data.resize(width * height);
    std::vector<uint8_t> row(row_padded);

    file.seekg(dataOffset, std::ios::beg);
    for (int y = height - 1; y >= 0; y--) {
        file.read(reinterpret_cast<char*>(row.data()), row_padded);
        for (int x = 0; x < width; ++x) {
            RGB& pixel = data[y * width + x];
            pixel.b = row[x * 3 + 0];
            pixel.g = row[x * 3 + 1];
            pixel.r = row[x * 3 + 2];
        }
    }

    return true;
}