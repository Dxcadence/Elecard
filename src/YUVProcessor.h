#ifndef YUVPROCESSOR_H
#define YUVPROCESSOR_H

#include <vector>
#include <string>
#include "YUVConverter.h"
#include "BMPReader.h"

class YUVProcessor {
public:
    YUVProcessor(const std::string& bmpPath = "");
    bool loadBMP(const std::string& bmpPath);
    void overlayOnFrame(std::vector<uint8_t>& yPlane,
                        std::vector<uint8_t>& uPlane,
                        std::vector<uint8_t>& vPlane,
                        int frameWidth, int frameHeight,
                        int posX, int posY,
                        float opacity) const;
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

private:
    std::vector<uint8_t> m_yData, m_uData, m_vData;
    int m_width = 0, m_height = 0;
    int m_halfWidth = 0, m_halfHeight = 0;
};

#endif // YUVPROCESSOR_H