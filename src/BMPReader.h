#pragma once
#include <vector>
#include <cstdint>
#include <string>

struct RGB
{
    uint8_t r, g, b;
};


struct BMPHeader {
    uint16_t fileType;
    uint32_t fileSize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t dataOffset;
};

struct BMPInfo
{
    uint32_t headerSize;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitCount;
    uint32_t compression;
    uint32_t imageSize;
    int32_t xPixelPerMeter;
    int32_t yPixelPerMeter;
    uint32_t colorsUsed;
    uint32_t importantColors;
};


class BMPReader {
public:
    static bool load(const std::string& filename, int& width, int& height, std::vector<RGB>& data);
};
