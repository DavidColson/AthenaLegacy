project(Engine)

include_directories("AssetDatabase/")
include_directories("Core/")
include_directories("EntitySystem/")
include_directories("Devices/")
include_directories("Modules/")
include_directories("ThirdParty/")
include_directories("ThirdParty/EASTL/include/")
include_directories("ThirdParty/EABase/include/Common/")
include_directories("ThirdParty/FreeType/include")
add_Library(Engine "")

add_subdirectory(AssetDatabase)
add_subdirectory(Core)
add_subdirectory(EntitySystem)
add_subdirectory(Devices)
add_subdirectory(Modules)
add_subdirectory(ThirdParty/Imgui)
add_subdirectory(ThirdParty/EASTL)
add_subdirectory(ThirdParty/EABase)
add_subdirectory(ThirdParty/FreeType)
add_subdirectory(ThirdParty/stb)

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

target_precompile_headers(Engine PRIVATE "Core/PreCompiledHeader.h")

target_link_libraries(Engine ${SDL2_LIBRARIES} freetype Imgui EASTL d3d11 d3d10 d3dcompiler dxguid stb)

if(MSVC)
  target_compile_options(Engine PRIVATE /W3 /WX)
else()
  target_compile_options(Engine PRIVATE -Wall -Werror)
endif()

# target_compile_options(Engine PRIVATE /Bt)
# target_compile_options(Engine PRIVATE /showIncludes)