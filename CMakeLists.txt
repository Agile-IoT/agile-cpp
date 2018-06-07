cmake_minimum_required (VERSION 2.6)
project (agile-cpp)

option (BUILD_TESTS "Build tests programs" ON)
option (BUILD_MICROPHONE "Build microphone device" OFF)
option (BUILD_SPEAKER "Build speaker device" OFF)
option (BUILD_NEXT "Build Next software" OFF)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

find_package (PkgConfig REQUIRED)
pkg_check_modules (GIO-2.0 REQUIRED gio-2.0)

include_directories (org.eclipse.agail/src)
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set (LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

add_subdirectory (org.eclipse.agail/src)
add_subdirectory (org.eclipse.agail.Device/src)
add_subdirectory (org.eclipse.agail.DeviceCaller/src)
if (BUILD_TESTS)
    add_subdirectory (org.eclipse.agail.test/src)
endif (BUILD_TESTS)
if (BUILD_NEXT)
    add_subdirectory (org.eclipse.agail.Adele-NEXT/src)
endif (BUILD_NEXT)
