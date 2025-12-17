project(libjpeg)

include(../BuildOptions.cmake)

add_definitions(-D_LIB -D_CRT_SECURE_NO_WARNINGS -D_MBCS)
if (MSVC)
    add_compile_options(/W3 /EHsc)
endif()

set(SRC_FILES
    "../../../../3rdParty/jpeg-9f/cdjpeg.c"
    "../../../../3rdParty/jpeg-9f/cjpeg.c"
    "../../../../3rdParty/jpeg-9f/jaricom.c"
    "../../../../3rdParty/jpeg-9f/jcapimin.c"
    "../../../../3rdParty/jpeg-9f/jcapistd.c"
    "../../../../3rdParty/jpeg-9f/jcarith.c"
    "../../../../3rdParty/jpeg-9f/jccoefct.c"
    "../../../../3rdParty/jpeg-9f/jccolor.c"
    "../../../../3rdParty/jpeg-9f/jcdctmgr.c"
    "../../../../3rdParty/jpeg-9f/jchuff.c"
    "../../../../3rdParty/jpeg-9f/jcinit.c"
    "../../../../3rdParty/jpeg-9f/jcmainct.c"
    "../../../../3rdParty/jpeg-9f/jcmarker.c"
    "../../../../3rdParty/jpeg-9f/jcmaster.c"
    "../../../../3rdParty/jpeg-9f/jcomapi.c"
    "../../../../3rdParty/jpeg-9f/jcparam.c"
    "../../../../3rdParty/jpeg-9f/jcprepct.c"
    "../../../../3rdParty/jpeg-9f/jcsample.c"
    "../../../../3rdParty/jpeg-9f/jctrans.c"
    "../../../../3rdParty/jpeg-9f/jdapimin.c"
    "../../../../3rdParty/jpeg-9f/jdapistd.c"
    "../../../../3rdParty/jpeg-9f/jdarith.c"
    "../../../../3rdParty/jpeg-9f/jdatadst.c"
    "../../../../3rdParty/jpeg-9f/jdatasrc.c"
    "../../../../3rdParty/jpeg-9f/jdcoefct.c"
    "../../../../3rdParty/jpeg-9f/jdcolor.c"
    "../../../../3rdParty/jpeg-9f/jddctmgr.c"
    "../../../../3rdParty/jpeg-9f/jdhuff.c"
    "../../../../3rdParty/jpeg-9f/jdinput.c"
    "../../../../3rdParty/jpeg-9f/jdmainct.c"
    "../../../../3rdParty/jpeg-9f/jdmarker.c"
    "../../../../3rdParty/jpeg-9f/jdmaster.c"
    "../../../../3rdParty/jpeg-9f/jdmerge.c"
    "../../../../3rdParty/jpeg-9f/jdpostct.c"
    "../../../../3rdParty/jpeg-9f/jdsample.c"
    "../../../../3rdParty/jpeg-9f/jdtrans.c"
    "../../../../3rdParty/jpeg-9f/jerror.c"
    "../../../../3rdParty/jpeg-9f/jfdctflt.c"
    "../../../../3rdParty/jpeg-9f/jfdctfst.c"
    "../../../../3rdParty/jpeg-9f/jfdctint.c"
    "../../../../3rdParty/jpeg-9f/jidctflt.c"
    "../../../../3rdParty/jpeg-9f/jidctfst.c"
    "../../../../3rdParty/jpeg-9f/jidctint.c"
    "../../../../3rdParty/jpeg-9f/jmemmgr.c"
    "../../../../3rdParty/jpeg-9f/jmemnobs.c"
    "../../../../3rdParty/jpeg-9f/jpegtran.c"
    "../../../../3rdParty/jpeg-9f/jquant1.c"
    "../../../../3rdParty/jpeg-9f/jquant2.c"
    "../../../../3rdParty/jpeg-9f/jutils.c"
    "../../../../3rdParty/jpeg-9f/rdswitch.c"
    "../../../../3rdParty/jpeg-9f/transupp.c"
)

add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

target_link_libraries(${PROJECT_NAME})
