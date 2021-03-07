project(AsteroidsGame)

include_directories("${ENGINE_SOURCE_PATH}/AssetDatabase/")
include_directories("${ENGINE_SOURCE_PATH}/Core/")
include_directories("${ENGINE_SOURCE_PATH}/Core/")
include_directories("${ENGINE_SOURCE_PATH}/EntitySystem/")
include_directories("${ENGINE_SOURCE_PATH}/Devices/")
include_directories("${ENGINE_SOURCE_PATH}/Modules/")
include_directories("${ENGINE_SOURCE_PATH}/Modules/")
include_directories("${ENGINE_SOURCE_PATH}/ThirdParty/FreeType/include/")
include_directories("${ENGINE_SOURCE_PATH}/ThirdParty/Imgui/")

add_executable (AsteroidsGame WIN32 "")
target_link_libraries(AsteroidsGame Engine)

target_sources(AsteroidsGame
    PRIVATE
        "Asteroids.h"
        "Asteroids.cpp"
        "Components.h"
        "Components.cpp"
        "Systems_Old.h"
        "Systems_Old.cpp"
        "PolylineDrawSystem.h"
        "PolylineDrawSystem.cpp"
)

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

target_precompile_headers(Engine PRIVATE "${ENGINE_SOURCE_PATH}/Core/PreCompiledHeader.h")

if(MSVC)
  target_compile_options(AsteroidsGame PRIVATE /W3 /WX)
else()
  target_compile_options(AsteroidsGame PRIVATE -Wall -Werror)
endif()

# target_link_options(AsteroidsGame PRIVATE /time+)
# target_compile_options(AsteroidsGame PRIVATE /Bt+ /d2cgsummary)