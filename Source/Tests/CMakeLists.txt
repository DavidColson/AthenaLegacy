project(Tests)

set(SOURCES
    "main.cpp"
    "Catch/catch.hpp"
    )

foreach(source IN LISTS SOURCES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()

include_directories("../Engine/" "/")
add_executable (Tests ${SOURCES})
target_link_libraries(Tests ${SDL2_LIBRARIES} Engine d3d11 d3d10 d3dcompiler dxguid freetype Imgui)