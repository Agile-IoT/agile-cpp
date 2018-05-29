add_library (AbstractAgileObject STATIC object/AbstractAgileObject.cpp)
add_library (Device STATIC Device.cpp)
target_include_directories (AbstractAgileObject PUBLIC ${GIO-2.0_INCLUDE_DIRS})
target_compile_options (AbstractAgileObject PUBLIC ${GIO-2.0_CFLAGS_OTHER})
