#pragma once
#include <vector>
#include "BMPReader.h"

class YUVConverter{
    public:
        
        static void convert(const std::vector<RGB> &rgbData,
                            std::vector<uint8_t> &Y,
                            std::vector<uint8_t> &U,
                            std::vector<uint8_t> &V,
                            int width, int height,
                            int threads = 4);

    private:
        static void convertBlock(const RGB *rgb, uint8_t *Y, uint8_t *U, uint8_t *V, int width, int startRow, int endRow);
};