# Minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name and language
project(YUVConverter LANGUAGES CXX)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable warnings and optimizations
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2 -march=native")
endif()

# Add executable target
add_executable(YUVProcessor
    src/main.cpp
    src/YUVProcessor.cpp   # New file for YUVProcessor implementation
    src/YUVConverter.cpp
    src/BMPReader.cpp
)

# Include directories (if headers are in a separate folder, adjust accordingly)
target_include_directories(YUVProcessor PRIVATE ${CMAKE_SOURCE_DIR})

# Link necessary libraries
# If using SIMD intrinsics (e.g., SSE), ensure the compiler supports them
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(YUVProcessor PRIVATE -msse4.2)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(YUVProcessor PRIVATE /arch:AVX2)
endif()

# Optional: Install rules (for packaging or deployment)
install(TARGETS YUVProcessor DESTINATION bin)