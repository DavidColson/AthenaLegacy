project(AsteroidsGame)

include_directories("../Engine/Core/")
include_directories("../Engine/Devices/")
include_directories("../Engine/Modules/")
include_directories("../Engine/ThirdParty/")
include_directories("/")

add_executable (AsteroidsGame WIN32 "")
target_link_libraries(AsteroidsGame ${SDL2_LIBRARIES} Engine d3d11 d3d10 d3dcompiler dxguid freetype Imgui EASTL)

target_sources(AsteroidsGame
    PRIVATE
        "Asteroids.h"
        "Asteroids.cpp"
        "Components.h"
        "Components.cpp"
        "Systems.h"
        "Systems.cpp"
)

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
  target_compile_options(AsteroidsGame PRIVATE /W4 /WX)
else()
  target_compile_options(AsteroidsGame PRIVATE -Wall -Werror)
endif()