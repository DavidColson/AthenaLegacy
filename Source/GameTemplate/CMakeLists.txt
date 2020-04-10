project(Game)

include_directories("../Engine/Core/")
include_directories("../Engine/Devices/")
include_directories("../Engine/Modules/")
include_directories("../Engine/ThirdParty/")
include_directories("/")

add_executable (Game WIN32 "")
target_link_libraries(Game ${SDL2_LIBRARIES} Engine d3d11 d3d10 d3dcompiler dxguid freetype Imgui EASTL)

target_sources(Game
    PRIVATE
        "Game.h"
        "Game.cpp"
        "Components.h"
        "Components.cpp"
        "Systems.h"
        "Systems.cpp"
)

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

target_precompile_headers(Engine PRIVATE "../Engine/Core/PreCompiledHeader.h")

if(MSVC)
  target_compile_options(Game PRIVATE /W3 /WX)
else()
  target_compile_options(Game PRIVATE -Wall -Werror)
endif()

# target_link_options(Game PRIVATE /time+)
# target_compile_options(Game PRIVATE /Bt+ /d2cgsummary)