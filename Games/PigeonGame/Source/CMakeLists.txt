project(PigeonGame)

include_directories("${ENGINE_SOURCE_PATH}/AssetDatabase/")
include_directories("${ENGINE_SOURCE_PATH}/Core/")
include_directories("${ENGINE_SOURCE_PATH}/EntitySystem/")
include_directories("${ENGINE_SOURCE_PATH}/Devices/")
include_directories("${ENGINE_SOURCE_PATH}/Modules/")

add_executable (PigeonGame WIN32 "")
target_link_libraries(PigeonGame ${SDL2_LIBRARIES} Engine)

target_sources(PigeonGame
    PRIVATE
        "Pigeons.h"
        "Pigeons.cpp"
)

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

target_precompile_headers(Engine PRIVATE "${ENGINE_SOURCE_PATH}/Core/PreCompiledHeader.h")

if(MSVC)
  target_compile_options(PigeonGame PRIVATE /W3 /WX)
else()
  target_compile_options(PigeonGame PRIVATE -Wall -Werror)
endif()

# target_link_options(PigeonGame PRIVATE /time+)
# target_compile_options(PigeonGame PRIVATE /Bt+ /d2cgsummary)