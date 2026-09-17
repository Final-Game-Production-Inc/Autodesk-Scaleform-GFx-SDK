project(libjpeg)

include(../BuildOptions.cmake)

add_definitions(-D_LIB -D_CRT_SECURE_NO_WARNINGS -D_MBCS)
if (MSVC)
    add_compile_options(/W3 /EHsc)
endif()

set(SRC_FILES
    "../../../../3rdParty/jpeg-10/cdjpeg.c"
    "../../../../3rdParty/jpeg-10/cjpeg.c"
    "../../../../3rdParty/jpeg-10/jaricom.c"
    "../../../../3rdParty/jpeg-10/jcapimin.c"
    "../../../../3rdParty/jpeg-10/jcapistd.c"
    "../../../../3rdParty/jpeg-10/jcarith.c"
    "../../../../3rdParty/jpeg-10/jccoefct.c"
    "../../../../3rdParty/jpeg-10/jccolor.c"
    "../../../../3rdParty/jpeg-10/jcdctmgr.c"
    "../../../../3rdParty/jpeg-10/jchuff.c"
    "../../../../3rdParty/jpeg-10/jcinit.c"
    "../../../../3rdParty/jpeg-10/jcmainct.c"
    "../../../../3rdParty/jpeg-10/jcmarker.c"
    "../../../../3rdParty/jpeg-10/jcmaster.c"
    "../../../../3rdParty/jpeg-10/jcomapi.c"
    "../../../../3rdParty/jpeg-10/jcparam.c"
    "../../../../3rdParty/jpeg-10/jcprepct.c"
    "../../../../3rdParty/jpeg-10/jcsample.c"
    "../../../../3rdParty/jpeg-10/jctrans.c"
    "../../../../3rdParty/jpeg-10/jdapimin.c"
    "../../../../3rdParty/jpeg-10/jdapistd.c"
    "../../../../3rdParty/jpeg-10/jdarith.c"
    "../../../../3rdParty/jpeg-10/jdatadst.c"
    "../../../../3rdParty/jpeg-10/jdatasrc.c"
    "../../../../3rdParty/jpeg-10/jdcoefct.c"
    "../../../../3rdParty/jpeg-10/jdcolor.c"
    "../../../../3rdParty/jpeg-10/jddctmgr.c"
    "../../../../3rdParty/jpeg-10/jdhuff.c"
    "../../../../3rdParty/jpeg-10/jdinput.c"
    "../../../../3rdParty/jpeg-10/jdmainct.c"
    "../../../../3rdParty/jpeg-10/jdmarker.c"
    "../../../../3rdParty/jpeg-10/jdmaster.c"
    "../../../../3rdParty/jpeg-10/jdmerge.c"
    "../../../../3rdParty/jpeg-10/jdpostct.c"
    "../../../../3rdParty/jpeg-10/jdsample.c"
    "../../../../3rdParty/jpeg-10/jdtrans.c"
    "../../../../3rdParty/jpeg-10/jerror.c"
    "../../../../3rdParty/jpeg-10/jfdctflt.c"
    "../../../../3rdParty/jpeg-10/jfdctfst.c"
    "../../../../3rdParty/jpeg-10/jfdctint.c"
    "../../../../3rdParty/jpeg-10/jidctflt.c"
    "../../../../3rdParty/jpeg-10/jidctfst.c"
    "../../../../3rdParty/jpeg-10/jidctint.c"
    "../../../../3rdParty/jpeg-10/jmemmgr.c"
    "../../../../3rdParty/jpeg-10/jmemnobs.c"
    "../../../../3rdParty/jpeg-10/jpegtran.c"
    "../../../../3rdParty/jpeg-10/jquant1.c"
    "../../../../3rdParty/jpeg-10/jquant2.c"
    "../../../../3rdParty/jpeg-10/jutils.c"
    "../../../../3rdParty/jpeg-10/rdswitch.c"
    "../../../../3rdParty/jpeg-10/transupp.c"
)

add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

target_link_libraries(${PROJECT_NAME})
