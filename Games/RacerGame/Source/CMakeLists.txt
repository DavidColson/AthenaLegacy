project(RacerGame)

include_directories("${ENGINE_SOURCE_PATH}/AssetDatabase/")
include_directories("${ENGINE_SOURCE_PATH}/Core/")
include_directories("${ENGINE_SOURCE_PATH}/Devices/")
include_directories("${ENGINE_SOURCE_PATH}/Modules/")

add_executable (RacerGame WIN32 "")
target_link_libraries(RacerGame Engine)

target_sources(RacerGame
    PRIVATE
        "RacerGame.h"
        "RacerGame.cpp"
        "Components.h"
        "Components.cpp"
        "Systems.h"
        "Systems.cpp"
)

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

target_precompile_headers(Engine PRIVATE "${ENGINE_SOURCE_PATH}/Core/PreCompiledHeader.h")

if(MSVC)
  target_compile_options(RacerGame PRIVATE /W3 /WX)
else()
  target_compile_options(RacerGame PRIVATE -Wall -Werror)
endif()

# target_link_options(RacerGame PRIVATE /time+)
# target_compile_options(RacerGame PRIVATE /Bt+ /d2cgsummary)