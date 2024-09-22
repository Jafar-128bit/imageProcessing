#include "lodepng.h"
#include "turbojpeg.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

// Function to check output folder exists or not
bool checkFolderPath(const std::string &path) {
    if (fs::exists(path) && fs::is_directory(path)) {
        return true;
    } else {
        std::cout << "Output folder does not exist.\n";
        return false;
    }
}

// Function to create a folder at the given path
void createFolder(const std::string &path) {
    try {
        if (fs::exists(path)) {
            std::cout << "Folder already exists: " << path << std::endl;
        } else {
            if (fs::create_directory(path)) {
                std::cout << "Folder created successfully: " << path << std::endl;
            } else {
                std::cerr << "Failed to create folder: " << path << std::endl;
            }
        }
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

// Function to get the file extension
std::string getFileExtension(const std::string &filename) {
    size_t dotPos = filename.find_last_of(".");
    if (dotPos == std::string::npos)
        return "";
    return filename.substr(dotPos + 1);
}

// Function to convert PNG to JPEG and compress
void convertAndCompressPng(const char *inputPath, const char *outputPath, int quality) {
    tjhandle handle = tjInitCompress();
    if (!handle) {
        throw std::runtime_error("Failed to initialize TurboJPEG compressor.");
    }

    // Read the PNG file
    std::vector<unsigned char> pngData;
    unsigned width, height;
    unsigned error = lodepng::decode(pngData, width, height, inputPath);
    if (error) {
        throw std::runtime_error("Failed to decode PNG file: " + std::string(lodepng_error_text(error)));
    }

    // Ensure the PNG data is in RGB format
    std::vector<unsigned char> rgbData(width * height * 3); // 3 bytes per pixel (RGB)
    for (size_t i = 0; i < pngData.size(); i += 4) {
        size_t j = (i / 4) * 3;
        rgbData[j] = pngData[i];         // Red
        rgbData[j + 1] = pngData[i + 1]; // Green
        rgbData[j + 2] = pngData[i + 2]; // Blue
    }

    // Compress the image to JPEG format
    unsigned long jpegSize = 0;
    unsigned char *jpegBuf = nullptr;

    // Compress the image
    if (tjCompress2(handle, rgbData.data(), width, 0, height, TJPF_RGB, &jpegBuf, &jpegSize, TJSAMP_444, quality,
                    TJFLAG_FASTDCT) < 0) {
        tjDestroy(handle);
        throw std::runtime_error("Failed to compress image to JPEG: " + std::string(tjGetErrorStr()));
    }

    tjDestroy(handle);

    // Write the JPEG file
    std::ofstream file(outputPath, std::ios::binary);
    if (!file.is_open()) {
        tjFree(jpegBuf);
        throw std::runtime_error("Failed to open JPEG file for writing.");
    }

    file.write(reinterpret_cast<const char *>(jpegBuf), jpegSize);
    file.close();

    tjFree(jpegBuf);

    std::cout << "JPEG file successfully created at " << outputPath << std::endl;
}

// Function to compress JPEG
void compressJpeg(const char *inputPath, const char *outputPath, int quality) {
    tjhandle handle = tjInitDecompress();
    if (!handle) {
        throw std::runtime_error("Failed to initialize TurboJPEG decompressor.");
    }

    // Open the JPEG file
    FILE *infile = fopen(inputPath, "rb");
    if (!infile) {
        tjDestroy(handle);
        throw std::runtime_error("Failed to open input JPEG file.");
    }

    fseek(infile, 0, SEEK_END);
    unsigned long jpegSize = ftell(infile);
    rewind(infile);

    unsigned char *jpegBuf = new unsigned char[jpegSize];
    fread(jpegBuf, 1, jpegSize, infile);
    fclose(infile);

    // Decompress the JPEG header
    int width, height, subsamp, colorspace;
    if (tjDecompressHeader3(handle, jpegBuf, jpegSize, &width, &height, &subsamp, &colorspace) < 0) {
        tjDestroy(handle);
        delete[] jpegBuf;
        throw std::runtime_error("Failed to decompress JPEG header: " + std::string(tjGetErrorStr()));
    }

    unsigned char *rgbBuf = new unsigned char[width * height * 3];
    if (tjDecompress2(handle, jpegBuf, jpegSize, rgbBuf, width, 0, height, TJPF_RGB, 0) < 0) {
        tjDestroy(handle);
        delete[] jpegBuf;
        delete[] rgbBuf;
        throw std::runtime_error("Failed to decompress JPEG: " + std::string(tjGetErrorStr()));
    }

    tjDestroy(handle);

    // Now compress the RGB data back to JPEG
    tjhandle compressHandle = tjInitCompress();
    if (!compressHandle) {
        delete[] jpegBuf;
        delete[] rgbBuf;
        throw std::runtime_error("Failed to initialize TurboJPEG compressor.");
    }

    unsigned long compressedSize = 0;
    unsigned char *compressedBuf = nullptr;

    if (tjCompress2(compressHandle, rgbBuf, width, 0, height, TJPF_RGB, &compressedBuf, &compressedSize, TJSAMP_444,
                    quality, TJFLAG_FASTDCT) < 0) {
        tjDestroy(compressHandle);
        delete[] jpegBuf;
        delete[] rgbBuf;
        throw std::runtime_error("Failed to compress JPEG: " + std::string(tjGetErrorStr()));
    }

    tjDestroy(compressHandle);

    // Write the compressed JPEG file
    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile.is_open()) {
        delete[] jpegBuf;
        delete[] rgbBuf;
        tjFree(compressedBuf);
        throw std::runtime_error("Failed to open output JPEG file for writing.");
    }

    outFile.write(reinterpret_cast<const char *>(compressedBuf), compressedSize);
    outFile.close();

    delete[] jpegBuf;
    delete[] rgbBuf;
    tjFree(compressedBuf);

    std::cout << "JPEG file successfully compressed and saved at " << outputPath << std::endl;
}

// Main function
int main() {
    const std::string inputFolder = "./input/";   // Path to the input folder
    const std::string outputFolder = "./output/"; // Path to the output folder
    int quality;

    try {
        std::cout << "******* Welcome to Image Processing - Convert PNG to JPEG or Compress JPEG *******\n";

        // Check if the input folder exists
        if (!checkFolderPath(inputFolder)) {
            createFolder(inputFolder);
        }
        // Check if the output folder exists, if not create it
        if (!checkFolderPath(outputFolder)) {
            createFolder(outputFolder);
        }
        std::cout << "Paste the Image file inside input folder to process!\n";

        // Ask for output quality between 0 and 100
        std::cout << "Enter Image Output quality between 0 to 100: ";
        std::cin >> quality;

        // Iterate through files in the input folder
        for (const auto &entry: fs::directory_iterator(inputFolder)) {
            if (entry.is_regular_file()) {                           // Check if it's a file
                std::string inputFile = entry.path().string();       // Full path of the input file
                std::string fileName = entry.path().stem().string(); // File name without extension
                std::string extension = getFileExtension(inputFile); // Get the file extension

                // Convert extension to lowercase for consistency
                for (char &c: extension) {
                    c = std::tolower(c);
                }

                // Process only files with supported extensions
                if (extension == "png" || extension == "jpg" || extension == "jpeg") {
                    // Construct output file path (append "_processed" before the extension)
                    std::string outputFile = outputFolder + fileName + "_processed." + extension;

                    // Convert std::string to const char* for function compatibility
                    const char *inputPathCStr = inputFile.c_str();
                    const char *outputPathCStr = outputFile.c_str();

                    // Determine file type and process accordingly
                    if (extension == "png") {
                        convertAndCompressPng(inputPathCStr, outputPathCStr, quality);
                    } else if (extension == "jpg" || extension == "jpeg") {
                        compressJpeg(inputPathCStr, outputPathCStr, quality);
                    }

                    std::cout << "Processed file: " << fileName << " -> Saved as: " << outputFile << "\n";
                } else {
                    std::cout << "Skipping unsupported file: " << inputFile << " (Extension: " << extension << ")\n";
                }
            }
        }
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}