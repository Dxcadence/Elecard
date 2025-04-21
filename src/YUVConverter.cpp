#include "YUVConverter.h"
#include <thread>
#include <emmintrin.h>
#include <iostream>

inline uint8_t clamp(int value, int min, int max) {
    return static_cast<uint8_t>(value < min ? min : (value > max ? max : value));
}

void YUVConverter::convert(const std::vector<RGB>& rgbData,
                           std::vector<uint8_t>& Y,
                           std::vector<uint8_t>& U,
                           std::vector<uint8_t>& V,
                           int width, int height, int threads) {
    Y.resize(width * height);
    int halfW = (width + 1) / 2, halfH = (height + 1) / 2;
    U.resize(halfW * halfH);
    V.resize(halfW * halfH);

    std::vector<std::thread> pool;
    int rowsPerThread = (height + threads - 1) / threads;

    for (int t = 0; t < threads; ++t) {
        int startRow = t * rowsPerThread;
        int endRow = std::min((t + 1) * rowsPerThread, height);

        pool.emplace_back([&, startRow, endRow]() {
            for (int y = startRow; y < endRow; ++y) {
                for (int x = 0; x < width; ++x) {
                    const RGB& p = rgbData[y * width + x];
                    Y[y * width + x] = clamp((77 * p.r + 150 * p.g + 29 * p.b) >> 8, 0, 255);
                }

                if (y % 2 == 0) {
                    for (int x = 0; x < width; x += 2) {
                        int rSum = 0, gSum = 0, bSum = 0;
                        int count = 0;
                        
                        for (int dy = 0; dy < 2 && y + dy < height; ++dy) {
                            for (int dx = 0; dx < 2 && x + dx < width; ++dx) {
                                const RGB& p = rgbData[(y + dy) * width + (x + dx)];
                                rSum += p.r;
                                gSum += p.g;
                                bSum += p.b;
                                count++;
                            }
                        }
                        
                        if (count > 0) {
                            int avgR = rSum / count;
                            int avgG = gSum / count;
                            int avgB = bSum / count;
                            
                            int u = (-43 * avgR - 85 * avgG + 128 * avgB + 32768) >> 8;
                            int v = (128 * avgR - 107 * avgG - 21 * avgB + 32768) >> 8;
                            
                            int uvIdx = (y / 2) * halfW + (x / 2);
                            U[uvIdx] = clamp(u, 0, 255);
                            V[uvIdx] = clamp(v, 0, 255);
                        }
                    }
                }
            }
        });
    }

    for (auto& th : pool) {
        th.join();
    }
}