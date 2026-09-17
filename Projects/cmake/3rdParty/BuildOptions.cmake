# 3rdParty

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    add_definitions(-DNDEBUG)
else()
    add_definitions(-D_DEBUG -DDEBUG -DPNG_DEBUG=1)
endif()

if (WIN32)
    add_definitions(-DWIN32)
endif()

if (MSVC)
    if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
        add_compile_options(/GS /Gy /Zc:wchar_t /Gm- /O2 /Ob1 /Zc:inline /fp:precise /errorReport:prompt /GF /WX- /Zc:forScope /GR- /Gd /MT /FC /nologo /Zl /diagnostics:column)
    else()
        add_compile_options(/GS /Zc:wchar_t /Z7 /Gm- /Od /Zc:inline /fp:precise /errorReport:prompt /WX- /Zc:forScope /RTC1 /GR- /Gd /MTd /FC /nologo /Zl /diagnostics:column)
    endif()
else()
    add_compile_options(-fPIC -ffast-math -finput-charset=UTF-8 -fshort-wchar)
    add_compile_options(-ffunction-sections -fdata-sections)
    add_compile_options(-mfpmath=sse -msse3)

    # Verbose output
    add_compile_options(-v)
    add_link_options(-v)
endif()
