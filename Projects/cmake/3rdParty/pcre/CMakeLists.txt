project(pcre)

include(../BuildOptions.cmake)

add_definitions(-DHAVE_CONFIG_H -D_CRT_SECURE_NO_WARNINGS -D_MBCS)
if (MSVC)
    add_compile_options(/W4 /wd4100 /wd4244)
endif()

set(SRC_FILES
    "../../../../3rdParty/pcre/pcre_chartables.c"
    "../../../../3rdParty/pcre/pcre_compile.c"
    "../../../../3rdParty/pcre/pcre_config.c"
    "../../../../3rdParty/pcre/pcre_dfa_exec.c"
    "../../../../3rdParty/pcre/pcre_exec.c"
    "../../../../3rdParty/pcre/pcre_fullinfo.c"
    "../../../../3rdParty/pcre/pcre_get.c"
    "../../../../3rdParty/pcre/pcre_globals.c"
    "../../../../3rdParty/pcre/pcre_info.c"
    "../../../../3rdParty/pcre/pcre_maketables.c"
    "../../../../3rdParty/pcre/pcre_newline.c"
    "../../../../3rdParty/pcre/pcre_ord2utf8.c"
    "../../../../3rdParty/pcre/pcre_refcount.c"
    "../../../../3rdParty/pcre/pcre_study.c"
    "../../../../3rdParty/pcre/pcre_tables.c"
    "../../../../3rdParty/pcre/pcre_try_flipped.c"
    "../../../../3rdParty/pcre/pcre_ucd.c"
    "../../../../3rdParty/pcre/pcre_valid_utf8.c"
    "../../../../3rdParty/pcre/pcre_version.c"
    "../../../../3rdParty/pcre/pcre_xclass.c"
)

add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

target_link_libraries(${PROJECT_NAME})
