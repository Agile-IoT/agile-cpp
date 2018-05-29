cmake_minimum_required (VERSION 2.6)
project (agile-cpp)

set (CMAKE_CXX_STANDARD 11)

find_package (PkgConfig REQUIRED)
pkg_check_modules (GIO-2.0 REQUIRED gio-2.0)

include_directories (org.eclipse.agail/src)
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set (LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

add_subdirectory (org.eclipse.agail/src)
add_subdirectory (org.eclipse.agail.Device/src)
add_subdirectory (org.eclipse.agail.DeviceCaller/src)
add_subdirectory (org.eclipse.agail.test/src)
