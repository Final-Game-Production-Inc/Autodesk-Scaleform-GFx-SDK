project(zlib)

include(../BuildOptions.cmake)

if (MSVC)
    add_compile_options(/W3 /EHsc)
endif()

set(SRC_FILES
    "../../../../3rdParty/zlib-1.3.1/adler32.c"
    "../../../../3rdParty/zlib-1.3.1/compress.c"
    "../../../../3rdParty/zlib-1.3.1/crc32.c"
    "../../../../3rdParty/zlib-1.3.1/deflate.c"
    "../../../../3rdParty/zlib-1.3.1/infback.c"
    "../../../../3rdParty/zlib-1.3.1/inffast.c"
    "../../../../3rdParty/zlib-1.3.1/inflate.c"
    "../../../../3rdParty/zlib-1.3.1/inftrees.c"
    "../../../../3rdParty/zlib-1.3.1/trees.c"
    "../../../../3rdParty/zlib-1.3.1/uncompr.c"
    "../../../../3rdParty/zlib-1.3.1/zutil.c"
)

add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

target_link_libraries(${PROJECT_NAME})
