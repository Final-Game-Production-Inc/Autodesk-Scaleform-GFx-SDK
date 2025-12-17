project(libpng)

include(../BuildOptions.cmake)

if (MSVC)
    add_compile_options(/W3)
endif()

set(SRC_FILES
    "../../../../3rdParty/libpng-1.6.53/png.c"
    "../../../../3rdParty/libpng-1.6.53/pngerror.c"
    "../../../../3rdParty/libpng-1.6.53/pngget.c"
    "../../../../3rdParty/libpng-1.6.53/pngmem.c"
    "../../../../3rdParty/libpng-1.6.53/pngpread.c"
    "../../../../3rdParty/libpng-1.6.53/pngread.c"
    "../../../../3rdParty/libpng-1.6.53/pngrio.c"
    "../../../../3rdParty/libpng-1.6.53/pngrtran.c"
    "../../../../3rdParty/libpng-1.6.53/pngrutil.c"
    "../../../../3rdParty/libpng-1.6.53/pngset.c"
    "../../../../3rdParty/libpng-1.6.53/pngtrans.c"
    "../../../../3rdParty/libpng-1.6.53/pngwio.c"
    "../../../../3rdParty/libpng-1.6.53/pngwrite.c"
    "../../../../3rdParty/libpng-1.6.53/pngwtran.c"
    "../../../../3rdParty/libpng-1.6.53/pngwutil.c"
)

add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
    "../../../../3rdParty/libpng-1.6.53"
    "../../../../3rdParty/zlib-1.3.1"
)

target_link_libraries(${PROJECT_NAME})
