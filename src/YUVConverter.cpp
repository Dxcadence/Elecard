#include "YUVConverter.h"
#include <thread>
#include <emmintrin.h>
#include <iostream>

void YUVConverter::convertBlock(const RGB* rgb, uint8_t* Y, uint8_t* U, uint8_t* V, 
                              int width, int startRow, int endRow) {
    const int simdStep = 8; 
    const int halfW = (width + 1) >> 1;

    for (int y = startRow; y < endRow; ++y) {
        for (int x = 0; x + simdStep <= width; x += simdStep) {
            const RGB* px = &rgb[y * width + x];

            __m128i r = _mm_setr_epi16(px[0].r, px[1].r, px[2].r, px[3].r,
                                      px[4].r, px[5].r, px[6].r, px[7].r);
            __m128i g = _mm_setr_epi16(px[0].g, px[1].g, px[2].g, px[3].g,
                                      px[4].g, px[5].g, px[6].g, px[7].g);
            __m128i b = _mm_setr_epi16(px[0].b, px[1].b, px[2].b, px[3].b,
                                      px[4].b, px[5].b, px[6].b, px[7].b);

            __m128i yVal = _mm_add_epi16(
                _mm_add_epi16(
                    _mm_mullo_epi16(r, _mm_set1_epi16(77)),
                    _mm_mullo_epi16(g, _mm_set1_epi16(150))),
                _mm_mullo_epi16(b, _mm_set1_epi16(29)));
            yVal = _mm_srli_epi16(yVal, 8);

            alignas(16) uint16_t tempY[8];
            _mm_store_si128((__m128i*)tempY, yVal);
            for (int i = 0; i < 8; i++) {
                Y[y * width + x + i] = static_cast<uint8_t>(tempY[i]);
            }
        }

        for (int x = width - (width % simdStep); x < width; ++x) {
            const RGB& p = rgb[y * width + x];
            Y[y * width + x] = (77 * p.r + 150 * p.g + 29 * p.b) >> 8;
        }

        if ((y & 1) == 0) {
            for (int x = 0; x < width; x += 2) { 
                __m128i r = _mm_setr_epi16(
                    rgb[(y * width + x)].r,
                    rgb[(y * width + x + 1)].r,
                    rgb[((y + 1) * width + x)].r,
                    rgb[((y + 1) * width + x + 1)].r,
                    0, 0, 0, 0);

                __m128i g = _mm_setr_epi16(
                    rgb[(y * width + x)].g,
                    rgb[(y * width + x + 1)].g,
                    rgb[((y + 1) * width + x)].g,
                    rgb[((y + 1) * width + x + 1)].g,
                    0, 0, 0, 0);

                __m128i b = _mm_setr_epi16(
                    rgb[(y * width + x)].b,
                    rgb[(y * width + x + 1)].b,
                    rgb[((y + 1) * width + x)].b,
                    rgb[((y + 1) * width + x + 1)].b,
                    0, 0, 0, 0);

                __m128i u = _mm_add_epi16(
                    _mm_add_epi16(
                        _mm_mullo_epi16(r, _mm_set1_epi16(-43)),
                        _mm_mullo_epi16(g, _mm_set1_epi16(-85))),
                    _mm_mullo_epi16(b, _mm_set1_epi16(128)));
                u = _mm_add_epi16(u, _mm_set1_epi16(32768));
                u = _mm_srli_epi16(u, 8);

                __m128i v = _mm_add_epi16(
                    _mm_add_epi16(
                        _mm_mullo_epi16(r, _mm_set1_epi16(128)),
                        _mm_mullo_epi16(g, _mm_set1_epi16(-107))),
                    _mm_mullo_epi16(b, _mm_set1_epi16(-21)));
                v = _mm_add_epi16(v, _mm_set1_epi16(32768));
                v = _mm_srli_epi16(v, 8);

                alignas(16) uint16_t tempU[8], tempV[8];
                _mm_store_si128((__m128i*)tempU, u);
                _mm_store_si128((__m128i*)tempV, v);

                int uSum = tempU[0] + tempU[1] + tempU[2] + tempU[3];
                int vSum = tempV[0] + tempV[1] + tempV[2] + tempV[3];

                int idx = (y >> 1) * halfW + (x >> 1);
                U[idx] = static_cast<uint8_t>(uSum >> 2); 
                V[idx] = static_cast<uint8_t>(vSum >> 2); 
            }
        }
    }
}

void YUVConverter::convert(const std::vector<RGB>& rgbData, 
                         std::vector<uint8_t>& Y,
                         std::vector<uint8_t>& U,
                         std::vector<uint8_t>& V,
                         int width, int height, int threads) {

    Y.resize(width * height);
    int halfW = (width + 1) >> 1;
    int halfH = (height + 1) >> 1;
    U.resize(halfW * halfH);
    V.resize(halfW * halfH);

    std::vector<std::thread> pool;
    int rowsPerThread = height / threads;

    for (int t = 0; t < threads; ++t) {
        int start = t * rowsPerThread;
        int end = (t == threads - 1) ? height : (t + 1) * rowsPerThread;
        
        pool.emplace_back([&rgbData, &Y, &U, &V, width, height, start, end]() {
            convertBlock(rgbData.data(), Y.data(), U.data(), V.data(),
                        width, start, end);
        });
    }

    for (auto& th : pool) {
        th.join();
    }
}