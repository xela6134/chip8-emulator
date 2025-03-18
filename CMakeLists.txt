cmake_minimum_required(VERSION 3.22)
project(Chip8Emulator)

# Find SDL2 using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

# Include directories
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR})

# Add the executable from your source files
add_executable(Chip8Emulator main.cpp chip8.cpp)

# Link the SDL2 libraries to the executable
target_link_libraries(Chip8Emulator ${SDL2_LIBRARIES})
