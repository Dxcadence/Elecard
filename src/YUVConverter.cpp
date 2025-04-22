#include "YUVConverter.h"
#include <thread>
#include <emmintrin.h>
#include <iostream>

void YUVConverter::convertYBlock(const RGB *rgb, uint8_t *Y, int width, int startRow, int endRow)
{
    int simdStep = 4;
    for(int y = startRow; y < endRow; ++y){
        for(int x = 0; x + simdStep <= width; x += simdStep){
            const RGB* px = &rgb[y * width + x];
            __m128i r = _mm_setr_epi16(px[0].r, px[1].r, px[2].r, px[3].r, px[4].r, px[5].r, px[6].r, px[7].r);
            __m128i g = _mm_setr_epi16(px[0].g, px[1].g, px[2].g, px[3].g, px[4].g, px[5].g, px[6].g, px[7].g);
            __m128i b = _mm_setr_epi16(px[0].b, px[1].b, px[2].b, px[3].b, px[4].b, px[5].b, px[6].b, px[7].b);

            r = _mm_mullo_epi16(r, _mm_set1_epi16(77));
            g = _mm_mullo_epi16(g, _mm_set1_epi16(150));
            b = _mm_mullo_epi16(b, _mm_set1_epi16(29));

            __m128i yVal = _mm_add_epi16(_mm_add_epi16(r, g), b);

            yVal = _mm_srli_epi16(yVal, 8);

            alignas(16) uint16_t temp[8];
            _mm_store_si128((__m128i*)temp, yVal);

            for(int i = 0; i < 8; i++){
                Y[y * width + x + i] = static_cast<uint8_t>(temp[i]);
            }
        }
        for(int x = width - (width & simdStep); x < width; ++x){
            const RGB& p = rgb[y * width + x];
            Y[y * width + x] = (77 * p.r + 150 * p.g + 29 * p.b) >> 8;
        }
        
    }
}

void YUVConverter::convertUV(const std::vector<RGB>& rgbData,
                            std::vector<uint8_t>& U,
                            std::vector<uint8_t>& V,
                            int width, int height) {
    int halfW = width / 2, halfH = height / 2;
    U.resize(halfW * halfH);
    V.resize(halfW * halfH);

    for(int y = 0; y < height; y += 2){
        for( int x = 0; x < width; x += 2){
            int uSum = 0, vSum = 0, count = 0;
            for(int dy = 0; dy < 2; ++dy){
                for(int dx = 0; dx < 2; ++dx){
                    int px = x + dx;
                    int py = y + dy;
                    if (px >= width || py >= height){
                        continue;
                    }
                    const RGB& p = rgbData[py * width + px];
                    int u = (-43 * p.r - 85 * p.g + 128 * p.b + 32768) >> 8;
                    int v = (128 * p.r - 107 * p.g - 21 * p.b + 32768) >> 8;

                    uSum += u;
                    vSum += v;
                    ++count;
                }
            }
            int idx = (y / 2) * halfW + (x / 2);
            U[idx] = static_cast<uint8_t>(uSum / count);
            V[idx] = static_cast<uint8_t>(vSum / count);
        }
    }
}

void YUVConverter::convert(const std::vector<RGB> &rgbData, std::vector<uint8_t> &Y, std::vector<uint8_t> &U, std::vector<uint8_t> &V, int width, int height, int threads)
{
    Y.resize(width * height);
    std::vector<std::thread> pool;
    int rowsPerThread = height / threads;

    for(int t = 0; t < threads; t++){
        int start = t * rowsPerThread;
        int end = (t == threads - 1) ? height : (t + 1) * rowsPerThread;
        pool.emplace_back(&YUVConverter::convertYBlock, rgbData.data(), Y.data(), width, start, end);
    }

    for (auto& th : pool){
        th.join();
    }

    convertUV(rgbData, U, V, width, height);
}