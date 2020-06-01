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
- Version: 1.66

Exact mirror of upstream repo plus a CMakeLists file to compile within the engine.

## cppfs

- Source: https://github.com/cginternals/cppfs/
- Version: 1.3.0

Lots of modifications on this one! Files stripped down to just source. Examples, scripts and tests all stripped out. Custom CMakeLists.txt being used. Removed all SSH code, including login credentials and URL parsing code. Stripped out base64 encoding functions, we have our own in engine. Replaced all stl use with EASTL. Removed Input/OutputStream classes and replaced with a custom FileStream class. Removed cppfs namespace and changed "fs" to FileSys for better naming with our engine.