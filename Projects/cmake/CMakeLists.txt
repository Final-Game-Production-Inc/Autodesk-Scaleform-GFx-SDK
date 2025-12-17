
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions(-D_DEBUG)
else()
    add_definitions(-DNDEBUG)
endif()

# 3rdParty
if (WIN32)
add_subdirectory(3rdParty/expat-2.1.0)
else()
# use CMakeLists.txt from dev/external/gfx4/3rdParty/expat-2.1.0
endif()
add_subdirectory(3rdParty/jpeg-9f)
add_subdirectory(3rdParty/libpng-1.6.53)
add_subdirectory(3rdParty/pcre)
add_subdirectory(3rdParty/zlib-1.3.1)

# SDK
add_subdirectory(SDK/AIR_SCU)
add_subdirectory(SDK/AIR2_SCU)
add_subdirectory(SDK/AIR3_SCU)
add_subdirectory(SDK/Expat)
add_subdirectory(SDK/GFx)
