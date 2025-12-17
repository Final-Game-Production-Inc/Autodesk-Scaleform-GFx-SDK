project(expat)

include(../BuildOptions.cmake)

add_definitions(-DCOMPILED_FROM_DSP -D_MBCS)
if (MSVC)
    add_compile_options(/W4)
endif()

set(SRC_FILES
    "../../../../3rdParty/expat-2.1.0/lib/xmlparse.c"
    "../../../../3rdParty/expat-2.1.0/lib/xmlrole.c"
    "../../../../3rdParty/expat-2.1.0/lib/xmltok.c"
    "../../../../3rdParty/expat-2.1.0/lib/xmltok_impl.c"
    "../../../../3rdParty/expat-2.1.0/lib/xmltok_ns.c"
)

add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

target_link_libraries(${PROJECT_NAME})
