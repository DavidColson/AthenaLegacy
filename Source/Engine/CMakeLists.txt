project(Engine)

set(SOURCES
    "Log.h"
    "Log.cpp"
    "Utility.h"
    "Utility.cpp"
    "TypeDB.h"
    "TypeDB.cpp"
    "TypeData.h"
    "ErrorHandling.h"
    "ErrorHandling.cpp"
    "GameFramework/World.h"
    "GameFramework/World.cpp"
    "Renderer/Renderer.h"
    "Renderer/Renderer.cpp"
    "Renderer/RenderProxy.h"
    "Renderer/RenderProxy.cpp"
    "Renderer/RenderFont.h"
    "Renderer/RenderFont.cpp"
    "Renderer/DebugDraw.h"
    "Renderer/DebugDraw.cpp"
    "Editor/Editor.h"
    "Editor/Editor.cpp"
    "Input/Input.h"
    "Input/Input.cpp"
    "Maths/Maths.h"
    "Maths/Maths.cpp"
    "Maths/Vec2.h"
    "Maths/Vec2.cpp"
    "Maths/Vec3.h"
    "Maths/Vec3.cpp"
    "Maths/Vec4.h"
    "Maths/Vec4.cpp"
    "Engine.h"
    "Engine.cpp"
    "IGame.h"
    )

set(SHADERS
    "../../Shaders/PostProcessing.hlsl"
    "../../Shaders/Shader.hlsl"
    )

foreach(source IN LISTS SOURCES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()

add_subdirectory(ThirdParty/Imgui)

# Bit of a hack to stop VS compiling the shaders
set_source_files_properties(${SHADERS} PROPERTIES HEADER_FILE_ONLY TRUE)

include_directories("/")
add_Library(Engine ${SOURCES} ${SHADERS})
