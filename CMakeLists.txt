cmake_minimum_required(VERSION 3.0)

project (Engine)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/build/")

find_package(SDL2 REQUIRED)

# make an error message that complains of SDL2 can't be found

set (ProgramFiles_x86 "ProgramFiles(x86)")
if ("$ENV{${ProgramFiles_x86}}")
    set (ProgramFiles "$ENV{${ProgramFiles_x86}}")
else ()
    set (ProgramFiles "$ENV{ProgramFiles}")
endif ()

# Fix up with find_path function
set (DIRECTX11_INCLUDE_DIR $ENV{${ProgramFiles_x86}}/Windows Kits/10/Include/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/um) 
set (DIRECTX11_LIB_DIR $ENV{${ProgramFiles_x86}}/Windows Kits/10/Lib/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/um/x64)
# message("${DIRECTX11_LIB_DIR}")

# Make an error message that complains if directx files cannot be found

set(SOURCES
    "source/main.cpp"
    "source/entity_system.h"
    "source/renderer.h"
    "source/renderer.cpp"
    "source/renderproxy.h"
    "source/renderproxy.cpp"
	"source/InputAPI.h"
    "source/InputAPI.cpp"
    "source/maths/maths.h"
    "source/maths/maths.cpp"
    )

set(SHADERS
    "shaders/Shader.hlsl"
    )

# Bit of a hack to stop VS compiling the shaders
set_source_files_properties(${SHADERS} PROPERTIES HEADER_FILE_ONLY TRUE)

# Find d3d headers and libs and set variables
include_directories(${SDL2_INCLUDE_DIRS} ${DIRECTX11_INCLUDE_DIR})
link_directories(${DIRECTX11_LIB_DIR})

add_executable(Engine WIN32 ${SOURCES} ${SHADERS})
target_link_libraries(Engine ${SDL2_LIBRARIES} d3d11 d3d10 d3dcompiler)