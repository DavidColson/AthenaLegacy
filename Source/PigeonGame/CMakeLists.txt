project(PigeonGame)

include_directories("../Engine/Core/")
include_directories("../Engine/Devices/")
include_directories("../Engine/Modules/")
include_directories("../Engine/ThirdParty/")
include_directories("/")

add_executable (PigeonGame WIN32 "")
target_link_libraries(PigeonGame ${SDL2_LIBRARIES} Engine)

target_sources(PigeonGame
    PRIVATE
        "Pigeons.h"
        "Pigeons.cpp"
        "Components.h"
        "Components.cpp"
        "Systems.h"
        "Systems.cpp"
)

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

target_precompile_headers(Engine PRIVATE "../Engine/Core/PreCompiledHeader.h")

if(MSVC)
  target_compile_options(PigeonGame PRIVATE /W3 /WX)
else()
  target_compile_options(PigeonGame PRIVATE -Wall -Werror)
endif()

# target_link_options(PigeonGame PRIVATE /time+)
# target_compile_options(PigeonGame PRIVATE /Bt+ /d2cgsummary)