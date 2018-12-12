cmake_minimum_required(VERSION 3.0)

project (Engine)

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


set(SOURCES
    "Source/Engine/main.cpp"
    "Source/Engine/Log.h"
    "Source/Engine/Log.cpp"
	"Source/Engine/Reflection.h"
    "Source/Engine/Reflection.cpp"
	"Source/Engine/ErrorHandling.h"
    "Source/Engine/ErrorHandling.cpp"
    "Source/Engine/GameFramework/World.h"
    "Source/Engine/GameFramework/World.cpp"
    "Source/Engine/Renderer/Renderer.h"
    "Source/Engine/Renderer/Renderer.cpp"
    "Source/Engine/Renderer/RenderProxy.h"
    "Source/Engine/Renderer/RenderProxy.cpp"
    "Source/Engine/Renderer/RenderFont.h"
    "Source/Engine/Renderer/RenderFont.cpp"
    "Source/Engine/Editor/Editor.h"
    "Source/Engine/Editor/Editor.cpp"
	"Source/Engine/Input/Input.h"
    "Source/Engine/Input/Input.cpp"
    "Source/Engine/Maths/Maths.h"
    "Source/Engine/Maths/Maths.cpp"
    "Source/Game/Asteroids.h"
    "Source/Game/Asteroids.cpp"
	"Source/Game/Systems/Systems.h"
    "Source/Game/Systems/Systems.cpp"
    "Source/Game/Components/Components.h"
    "Source/Game/Components/Components.cpp"
    )

set(SHADERS
    "Shaders/PostProcessing.hlsl"
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
include_directories(${SDL2_INCLUDE_DIRS} ${DIRECTX11_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIR} "Source/Engine/" "Source/Game")
link_directories(${DIRECTX11_LIB_DIR} ${FREETYPE_LIB_DIR})

add_subdirectory(Source/Engine/ThirdParty/Imgui)

add_executable(Engine WIN32 ${SOURCES} ${SHADERS})
target_link_libraries(Engine ${SDL2_LIBRARIES} d3d11 d3d10 d3dcompiler dxguid freetype Imgui)