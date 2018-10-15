cmake_minimum_required(VERSION 3.0)

project (Engine)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/Build/")

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
    "Source/main.cpp"
    "Source/GameFramework/World.h"
    "Source/Renderer/renderer.h"
    "Source/Renderer/renderer.cpp"
    "Source/Renderer/renderproxy.h"
    "Source/Renderer/renderproxy.cpp"
	"Source/Input/Input.h"
    "Source/Input/Input.cpp"
    "Source/Maths/Maths.h"
    "Source/Maths/Maths.cpp"
    )

set(SHADERS
    "Shaders/Shader.hlsl"
    )


foreach(source IN LISTS SOURCES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()

# Bit of a hack to stop VS compiling the shaders
set_source_files_properties(${SHADERS} PROPERTIES HEADER_FILE_ONLY TRUE)

# Find d3d headers and libs and set variables
include_directories(${SDL2_INCLUDE_DIRS} ${DIRECTX11_INCLUDE_DIR} "Source/")
link_directories(${DIRECTX11_LIB_DIR})

add_executable(Engine WIN32 ${SOURCES} ${SHADERS})
target_link_libraries(Engine ${SDL2_LIBRARIES} d3d11 d3d10 d3dcompiler)