project(AsteroidsGame)

set(SOURCES
    "Asteroids.cpp"
    "Systems/Systems.h"
    "Systems/Systems.cpp"
    "Components/Components.h"
    "Components/Components.cpp"
    )

foreach(source IN LISTS SOURCES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()

include_directories("../Engine/" "/")
add_executable (AsteroidsGame WIN32 ${SOURCES})
target_link_libraries(AsteroidsGame ${SDL2_LIBRARIES} Engine d3d11 d3d10 d3dcompiler dxguid freetype Imgui)