project(Engine)

include_directories("Core/")
include_directories("Devices/")
include_directories("Modules/")
include_directories("ThirdParty/")
add_Library(Engine "")

add_subdirectory(Core)
add_subdirectory(Devices)
add_subdirectory(Modules)
add_subdirectory(ThirdParty/Imgui)

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})