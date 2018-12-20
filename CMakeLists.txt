cmake_minimum_required(VERSION 3.0)

project (UnnamedEngine)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/Build/")

message("This project currently only supports 64 bit platforms")

# Find SDL2 directories
# make an error message that complains of SDL2 can't be found
find_package(SDL2 REQUIRED)


set (ProgramFiles_x86 "ProgramFiles(x86)")
if ("$ENV{${ProgramFiles_x86}}")
    set (ProgramFiles "$ENV{${ProgramFiles_x86}}")
else ()
    set (ProgramFiles "$ENV{ProgramFiles}")
endif ()

# Find DirectX include directories
# Make an error message that complains if directx files cannot be found
set (DIRECTX11_INCLUDE_DIR $ENV{${ProgramFiles_x86}}/Windows Kits/10/Include/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/um) 
set (DIRECTX11_LIB_DIR $ENV{${ProgramFiles_x86}}/Windows Kits/10/Lib/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/um/x64)
# message("${DIRECTX11_LIB_DIR}")

# Find Freetype include directories
set(FREETYPE_INCLUDE_DIR "ThirdParty/freetype-2.9/include/")
set(FREETYPE_LIB_DIR "ThirdParty/freetype-2.9/objs/x64/Release")

# Find d3d headers and libs and set variables
link_directories(${DIRECTX11_LIB_DIR} ${FREETYPE_LIB_DIR})
include_directories(${SDL2_INCLUDE_DIRS} ${DIRECTX11_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIR})

add_subdirectory(Source)