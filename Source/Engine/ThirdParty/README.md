# Third Party Libraries

## FreeType

- Source: https://www.freetype.org
- Version: 2.10.1

Files stripped down to just `include/`, `src/` except Jamfiles and tools subfolder, and license files. Custom CMakeLists.txt being used to build. 

## EASTL

- Source: https://github.com/electronicarts/EASTL
- Version: 3.16.05

Exact mirror of upstream repo.

## EABase

- Source: https://github.com/electronicarts/EABase 
- Version: 2.09.05

Exact mirror of upstream repo.

## Imgui

- Source: https://github.com/ocornut/imgui
- Version: 1.79

Have edited imconfig.h for our custom asserts and types. Also changed std to eastl in imgui_stdlib.h/.cpp. Removed the includes from sdl and d3d imp files. Added a custom cmake file for our project.

## stb_image

- Source: https://github.com/nothings/stb/
- Version: 2.26

Added an implementations cpp file for creating stb single header libs. Plus a CMakeLists file to compile it with the engine
