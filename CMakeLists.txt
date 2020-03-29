cmake_minimum_required(VERSION 3.13)

project (Athena)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/Build/")

message("This project currently only supports 64 bit platforms")

set(SDL2_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/Lib/SDL2-2.0.8/include/")
set(SDL2_LIB_DIRS "${CMAKE_SOURCE_DIR}/Lib/SDL2-2.0.8/lib/x64/")
set(SDL2_LIBRARIES SDL2 SDL2main)

macro(GroupSources dir)
    file(GLOB_RECURSE sources RELATIVE ${dir} *.h *.hpp *.c *.cpp *.cc)
    foreach(source ${sources})
        get_filename_component(source_path "${source}" PATH)
        string(REPLACE "/" "\\" source_path_msvc "${source_path}")
        source_group("${source_path_msvc}" FILES "${source}")
    endforeach()
endmacro()

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

# Find d3d headers and libs and set variables
link_directories(${DIRECTX11_LIB_DIR})
link_directories(${SDL2_LIB_DIRS})
include_directories(${SDL2_INCLUDE_DIRS} ${DIRECTX11_INCLUDE_DIR})

add_subdirectory(Source)