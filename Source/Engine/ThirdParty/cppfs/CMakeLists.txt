project(cppfs)

set(VERSION_MAJOR "1")
set(VERSION_MINOR "3")
set(VERSION_PATCH "0")

#
# Sources
#

set(include_path "${CMAKE_CURRENT_SOURCE_DIR}/include/cppfs")
set(source_path  "${CMAKE_CURRENT_SOURCE_DIR}/source")

if("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
    set(localfs "windows")
else()
    set(localfs "posix")
endif()

set(headers
    ${include_path}/cppfs.h
    ${include_path}/system.h
    ${include_path}/fs.h
    ${include_path}/FileHandle.h
    ${include_path}/FileIterator.h
    ${include_path}/FileVisitor.h
    ${include_path}/FunctionalFileVisitor.h
    ${include_path}/FileWatcher.h
    ${include_path}/FileEventHandler.h
    ${include_path}/FunctionalFileEventHandler.h
    ${include_path}/AbstractFileSystem.h
    ${include_path}/AbstractFileHandleBackend.h
    ${include_path}/AbstractFileIteratorBackend.h
    ${include_path}/AbstractFileWatcherBackend.h
    ${include_path}/InputStream.h
    ${include_path}/OutputStream.h
    ${include_path}/FilePath.h
    ${include_path}/Url.h
    ${include_path}/Tree.h
    ${include_path}/Diff.h
    ${include_path}/Change.h
    ${include_path}/units.h

    ${include_path}/${localfs}/LocalFileSystem.h
    ${include_path}/${localfs}/LocalFileHandle.h
    ${include_path}/${localfs}/LocalFileIterator.h
)

set(sources
    ${source_path}/system.cpp
    ${source_path}/fs.cpp
    ${source_path}/FileHandle.cpp
    ${source_path}/FileIterator.cpp
    ${source_path}/FileVisitor.cpp
    ${source_path}/FunctionalFileVisitor.cpp
    ${source_path}/FileWatcher.cpp
    ${source_path}/FileEventHandler.cpp
    ${source_path}/FunctionalFileEventHandler.cpp
    ${source_path}/AbstractFileSystem.cpp
    ${source_path}/AbstractFileHandleBackend.cpp
    ${source_path}/AbstractFileIteratorBackend.cpp
    ${source_path}/AbstractFileWatcherBackend.cpp
    ${source_path}/InputStream.cpp
    ${source_path}/OutputStream.cpp
    ${source_path}/FilePath.cpp
    ${source_path}/Url.cpp
    ${source_path}/Tree.cpp
    ${source_path}/Diff.cpp
    ${source_path}/Change.cpp

    ${source_path}/${localfs}/LocalFileSystem.cpp
    ${source_path}/${localfs}/LocalFileHandle.cpp
    ${source_path}/${localfs}/LocalFileIterator.cpp
)

if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    set(headers ${headers}
        ${include_path}/linux/LocalFileWatcher.h
    )

    set(sources ${sources}
        ${source_path}/linux/LocalFileWatcher.cpp
    )
endif()

if("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
    set(headers ${headers}
        ${include_path}/windows/LocalFileWatcher.h
    )

    set(sources ${sources}
        ${source_path}/windows/LocalFileWatcher.cpp
    )
endif()


add_library(cppfs
    ${sources}
    ${headers}
)

#
# Include directories
#

target_include_directories(cppfs
    PRIVATE
    ${PROJECT_BINARY_DIR}/source/include
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include

    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
