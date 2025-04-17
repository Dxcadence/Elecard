#include "BMPReader.h"
#include "YUVConverter.h"
#include "YUVProcessor.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc != 6) {
        std::cerr << "Usage: " << argv[0] << " <input_bmp> <input_yuv> <output_yuv> <frame_width> <frame_height>" << std::endl;
        std::cerr << "Example: " << argv[0] << " logo.bmp input.yuv output.yuv 1920 1080" << std::endl;
        return 1;
    }

    try {
        std::string bmpPath = argv[1];
        std::string inputYuvPath = argv[2];
        std::string outputYuvPath = argv[3];
        int frameWidth = std::stoi(argv[4]);
        int frameHeight = std::stoi(argv[5]);

        // Load and process BMP image using YUVProcessor
        YUVProcessor yuvProcessor;
        if (!yuvProcessor.loadBMP(bmpPath)) {
            std::cerr << "Failed to load BMP image: " << bmpPath << std::endl;
            return 1;
        }

        std::cout << "Loaded BMP image with width: " << yuvProcessor.getWidth()
                  << ", height: " << yuvProcessor.getHeight() << std::endl;

        // Open input YUV file
        std::ifstream inputYuvFile(inputYuvPath, std::ios::binary);
        if (!inputYuvFile) {
            std::cerr << "Error: Cannot open input YUV file: " << inputYuvPath << std::endl;
            return 1;
        }

        // Calculate frame sizes (assuming YUV420 format)
        size_t ySize = frameWidth * frameHeight;
        size_t uvSize = (frameWidth / 2) * (frameHeight / 2);

        // Determine total number of frames
        inputYuvFile.seekg(0, std::ios::end);
        size_t fileSize = inputYuvFile.tellg();
        inputYuvFile.seekg(0, std::ios::beg);

        size_t totalFrames = fileSize / (ySize + 2 * uvSize);
        std::cout << "Processing " << totalFrames << " frames..." << std::endl;

        // Open output YUV file
        std::ofstream outputYuvFile(outputYuvPath, std::ios::binary);
        if (!outputYuvFile) {
            std::cerr << "Error: Cannot open output YUV file: " << outputYuvPath << std::endl;
            return 1;
        }

        // Process each frame
        std::vector<uint8_t> frameY(ySize);
        std::vector<uint8_t> frameU(uvSize);
        std::vector<uint8_t> frameV(uvSize);

        for (size_t frameIdx = 0; frameIdx < totalFrames; ++frameIdx) {
            // Read one frame
            inputYuvFile.read(reinterpret_cast<char*>(frameY.data()), ySize);
            inputYuvFile.read(reinterpret_cast<char*>(frameU.data()), uvSize);
            inputYuvFile.read(reinterpret_cast<char*>(frameV.data()), uvSize);

            // Overlay BMP onto the current frame
            int posX = (frameWidth - yuvProcessor.getWidth()) / 2;
            int posY = (frameHeight - yuvProcessor.getHeight()) / 2;
            yuvProcessor.overlayOnFrame(frameY, frameU, frameV,
                                        frameWidth, frameHeight,
                                        posX, posY, 1.0f);

            // Write the modified frame to the output file
            outputYuvFile.write(reinterpret_cast<const char*>(frameY.data()), ySize);
            outputYuvFile.write(reinterpret_cast<const char*>(frameU.data()), uvSize);
            outputYuvFile.write(reinterpret_cast<const char*>(frameV.data()), uvSize);
        }

        inputYuvFile.close();
        outputYuvFile.close();

        std::cout << "Successfully processed YUV file with BMP overlay" << std::endl;
        std::cout << "Output saved to: " << outputYuvPath << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}