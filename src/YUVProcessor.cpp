#include "YUVProcessor.h"
#include <algorithm>

YUVProcessor::YUVProcessor(const std::string& bmpPath) {
    if (!bmpPath.empty()) {
        loadBMP(bmpPath);
    }
}

bool YUVProcessor::loadBMP(const std::string& bmpPath) {
    int width = 0, height = 0;
    std::vector<RGB> rgbData;
    if (!BMPReader::load(bmpPath, width, height, rgbData)) {
        return false;
    }

    // Initialize the YUV vectors
    m_yData.resize(width * height);
    m_uData.resize((width / 2) * (height / 2));
    m_vData.resize((width / 2) * (height / 2));

    // Convert to YUV
    YUVConverter::convert(rgbData, m_yData, m_uData, m_vData, width, height);
    m_width = width;
    m_height = height;
    m_halfWidth = width / 2;
    m_halfHeight = height / 2;

    return true;
}

void YUVProcessor::overlayOnFrame(std::vector<uint8_t>& yPlane,
                                  std::vector<uint8_t>& uPlane,
                                  std::vector<uint8_t>& vPlane,
                                  int frameWidth, int frameHeight,
                                  int posX, int posY,
                                  float opacity) const {
    if (m_yData.empty()) return;

    // Clamp opacity between 0 and 1
    opacity = std::clamp(opacity, 0.0f, 1.0f);

    // Overlay Y plane
    for (int y = 0; y < m_height && (y + posY) < frameHeight; y++) {
        for (int x = 0; x < m_width && (x + posX) < frameWidth; x++) {
            int frameIdx = (y + posY) * frameWidth + (x + posX);
            int bmpIdx = y * m_width + x;

            if (opacity >= 1.0f) {
                // Full opacity - replace completely
                yPlane[frameIdx] = m_yData[bmpIdx];
            } else {
                // Partial opacity - blend
                yPlane[frameIdx] = static_cast<uint8_t>(
                    yPlane[frameIdx] * (1.0f - opacity) + m_yData[bmpIdx] * opacity
                );
            }
        }
    }

    // Overlay UV planes
    for (int y = 0; y < m_halfHeight && (y + posY / 2) < frameHeight / 2; y++) {
        for (int x = 0; x < m_halfWidth && (x + posX / 2) < frameWidth / 2; x++) {
            int frameUIdx = (y + posY / 2) * (frameWidth / 2) + (x + posX / 2);
            int frameVIdx = frameUIdx;
            int bmpIdx = y * m_halfWidth + x;

            if (opacity >= 1.0f) {
                // Full opacity - replace completely
                uPlane[frameUIdx] = m_uData[bmpIdx];
                vPlane[frameVIdx] = m_vData[bmpIdx];
            } else {
                // Partial opacity - blend
                uPlane[frameUIdx] = static_cast<uint8_t>(
                    uPlane[frameUIdx] * (1.0f - opacity) + m_uData[bmpIdx] * opacity
                );
                vPlane[frameVIdx] = static_cast<uint8_t>(
                    vPlane[frameVIdx] * (1.0f - opacity) + m_vData[bmpIdx] * opacity
                );
            }
        }
    }
}