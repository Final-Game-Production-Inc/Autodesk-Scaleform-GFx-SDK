cmake_minimum_required(VERSION 4.2.0)
project(ScaleformSDK VERSION 4.3 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# Options — renderer selection
# ---------------------------------------------------------------------------
option(SF_RENDERER_VULKAN  "Build Vulkan renderer"          OFF)
option(SF_RENDERER_D3D12   "Build Direct3D 12 renderer"     OFF)
option(SF_RENDERER_D3D11   "Build Direct3D 11 renderer"     OFF)
option(SF_RENDERER_D3D10   "Build Direct3D 10 renderer"     OFF)
option(SF_RENDERER_D3D9    "Build Direct3D 9 renderer"      OFF)
option(SF_RENDERER_GL      "Build OpenGL renderer"          OFF)

# Options — component selection
option(SF_BUILD_SAMPLES    "Build sample applications"      ON)
option(SF_BUILD_KITS       "Build kit demo applications"    OFF)
option(SF_BUILD_TESTS      "Build test applications"        OFF)
option(SF_USE_SCU          "Use single compilation units"   OFF)
option(SF_ENABLE_VIDEO     "Build video playback (CRI SDK)" OFF)

# ---------------------------------------------------------------------------
# Global compile settings
# ---------------------------------------------------------------------------
include(cmake/ScaleformCompileFlags.cmake)

# ---------------------------------------------------------------------------
# 3rdParty dependencies (static libs built from source)
# ---------------------------------------------------------------------------
add_subdirectory(3rdParty)

# ---------------------------------------------------------------------------
# SDK libraries (libgfx, AS2, AS3, AIR, renderers, platform, etc.)
# ---------------------------------------------------------------------------
add_subdirectory(Src)

# ---------------------------------------------------------------------------
# Sample applications
# ---------------------------------------------------------------------------
if(SF_BUILD_SAMPLES)
    add_subdirectory(Apps)
endif()

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
if(SF_BUILD_TESTS)
    add_subdirectory(Tests)
endif()

# ---------------------------------------------------------------------------
# GFxShaderMaker (.NET tool — built via dotnet CLI)
# ---------------------------------------------------------------------------
find_program(DOTNET_EXECUTABLE dotnet)
if(DOTNET_EXECUTABLE AND EXISTS "${CMAKE_SOURCE_DIR}/GFxShaderMaker/GFxShaderMaker.csproj")
    set(GFX_SHADER_MAKER_OUT "${CMAKE_SOURCE_DIR}/Apps/Tools/GFxShaderMaker")
    add_custom_target(GFxShaderMaker
        COMMAND ${DOTNET_EXECUTABLE} build
            "${CMAKE_SOURCE_DIR}/GFxShaderMaker/GFxShaderMaker.csproj"
            -f net8.0 -c Release -p:ReleaseOnly=true
            -o "${GFX_SHADER_MAKER_OUT}"
        COMMENT "Building GFxShaderMaker (.NET)"
        VERBATIM
    )
endif()

# ---------------------------------------------------------------------------
# ScaleformVideoEncoder (.NET tool — built via dotnet CLI)
# ---------------------------------------------------------------------------
find_program(DOTNET_EXECUTABLE dotnet)
if(DOTNET_EXECUTABLE AND EXISTS "${CMAKE_SOURCE_DIR}/ScaleformVideoEncoder/ScaleformVideoEncoder/ScaleformVideoEncoder.csproj")
    set(GFX_SHADER_MAKER_OUT "${CMAKE_SOURCE_DIR}/Apps/Tools/ScaleformVideoEncoder/ScaleformVideoEncoder/")
    add_custom_target(GFxShaderMaker
        COMMAND ${DOTNET_EXECUTABLE} build
            "${CMAKE_SOURCE_DIR}/GFxShaderMaker/ScaleformVideoEncoder.csproj"
            -f net8.0 -c Release -p:ReleaseOnly=true
            -o "${GFX_SHADER_MAKER_OUT}"
        COMMENT "Building ScaleformVideoEncoder (.NET)"
        VERBATIM
    )
endif()

find_program(DOTNET_EXECUTABLE dotnet)
if(DOTNET_EXECUTABLE AND EXISTS "${CMAKE_SOURCE_DIR}/ScaleformVideoEncoder/Sofdec2EncoderControl.csproj")
    set(GFX_SHADER_MAKER_OUT "${CMAKE_SOURCE_DIR}/Apps/Tools/ScaleformVideoEncoder/Sofdec2EncoderControl")
    add_custom_target(GFxShaderMaker
        COMMAND ${DOTNET_EXECUTABLE} build
            "${CMAKE_SOURCE_DIR}/ScaleformVideoEncoder/Sofdec2EncoderContorl/Sofdec2EncoderControl.csproj"
            -f net8.0 -c Release -p:ReleaseOnly=true
            -o "${GFX_SHADER_MAKER_OUT}"
        COMMENT "Building ScaleformVideoEncoder (.NET)"
        VERBATIM
    )
endif()
