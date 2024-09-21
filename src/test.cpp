#include "lodepng.h"
#include <iostream>
#include <vector>

int main() {
    const unsigned width = 256;
    const unsigned height = 256;
    std::vector<unsigned char> image(width * height * 4); // RGBA

    // Fill the image with a gradient
    for (unsigned y = 0; y < height; ++y) {
        for (unsigned x = 0; x < width; ++x) {
            image[4 * width * y + 4 * x + 0] = x;   // Red
            image[4 * width * y + 4 * x + 1] = y;   // Green
            image[4 * width * y + 4 * x + 2] = 0;   // Blue
            image[4 * width * y + 4 * x + 3] = 255; // Alpha
        }
    }

    // Save the image as PNG using lodepng
    unsigned error = lodepng::encode("test_output.png", image, width, height);
    std::cout << lodepng_error_text(error) << std::endl;
    if (error) {
        std::cerr << "Error encoding PNG: " << lodepng_error_text(error) << std::endl;
        return 1;
    }

    std::cout << "Test image created successfully." << std::endl;
    return 0;
}
