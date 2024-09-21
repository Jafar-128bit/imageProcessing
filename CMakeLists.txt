cmake_minimum_required(VERSION 3.15)
project(CImgProject)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Adjust this path according to where you have Vcpkg installed
set(CMAKE_TOOLCHAIN_FILE "C:/Users/Jafar Iqbal/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

# Use the correct target triplet for MinGW
set(VCPKG_TARGET_TRIPLET "x64-mingw-dynamic")

# Add your executable and source files
add_executable(Main src/main.cpp)

# Manually specify the include directory for JPEG
include_directories("C:/Users/Jafar Iqbal/vcpkg/installed/x64-mingw-dynamic/include/")

# Manually specify the link library for JPEG
target_link_libraries(Main PRIVATE "C:/Users/Jafar Iqbal/vcpkg/installed/x64-mingw-dynamic/lib/libturbojpeg.dll.a" "C:/Users/Jafar Iqbal/vcpkg/installed/x64-mingw-dynamic/lib/liblodepng.a" gdi32)
