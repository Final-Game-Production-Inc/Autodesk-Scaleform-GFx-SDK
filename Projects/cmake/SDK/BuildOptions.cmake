# SDK

add_definitions(-DSF_BUILD_STATICLIB -DCURL_STATICLIB -D_MBCS)
if (${GAME_CONFIG} STREQUAL "Final")
    add_definitions(-DSF_BUILD_SHIPPING)
endif()
if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    add_definitions(-DNDEBUG)
else()
    add_definitions(-D_DEBUG)
endif()

if (WIN32)
    add_definitions(-DWIN32 -D_WINDOWS)
endif()

if (MSVC)
    if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
        add_compile_options(/MP /GS /W4 /Gy /Zc:wchar_t /Z7 /Gm- /Ox /Ob2 /Zc:inline /fp:precise /Zp8 /errorReport:prompt /GF /WX- /Zc:forScope /GR- /Gd /MT /FC /nologo /Zl /Ot /diagnostics:column)
    else()
        add_compile_options(/MP /GS /W4 /Zc:wchar_t /Z7 /Gm- /Od /Zc:inline /fp:precise /Zp8 /errorReport:prompt /WX- /Zc:forScope /RTC1 /GR- /Gd /MTd /FC /nologo /Zl /diagnostics:column)
    endif()
else()
    add_compile_options(-std=c++17)

    add_compile_options(-fPIC -ffast-math -finput-charset=UTF-8 -fshort-wchar)
    add_compile_options(-ffunction-sections -fdata-sections)
    add_compile_options(-mfpmath=sse -msse3)
    add_compile_options(-Wno-narrowing)

    # Verbose output
    add_compile_options(-v)
    add_link_options(-v)
endif()

add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
    "../../../../Include"
    "../../../../Src"
    "../../../../3rdParty/zlib-1.3.1"
    "../../../../3rdParty/jpeg-9f"
    "../../../../3rdParty/libpng-1.6.53"
    "../../../../3rdParty/expat-2.1.0/lib"
    "../../../../3rdParty/pcre"
    "../../../../3rdParty/cri/pc/include"
)
