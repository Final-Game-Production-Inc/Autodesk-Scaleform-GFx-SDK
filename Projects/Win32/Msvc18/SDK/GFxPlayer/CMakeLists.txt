cmake_minimum_required(VERSION 4.30)

project(GFxPlayer)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Windows SDK version
set(CMAKE_SYSTEM_VERSION 10.0)

# Original Visual Studio platform toolset: v145

# Source files
set(SOURCES
    ../../../../../Apps/Samples/Common/FontConfigParser.cpp
    ../../../../../Apps/Samples/Common/FxAmpAppControlCallback.cpp
    ../../../../../Apps/Samples/Common/FxPlayerAppBase.cpp
    ../../../../../Apps/Samples/Common/FxPlayerAutotest.cpp
    ../../../../../Apps/Samples/Common/FxRenderThread.cpp
    ../../../../../Apps/Samples/Common/FxSharedObjectManager.cpp
    ../../../../../Apps/Samples/Common/FxSoundFMOD.cpp
    ../../../../../Apps/Samples/FxPlayer/FxHUD.cpp
    ../../../../../Apps/Samples/FxPlayer/FxPlayer.cpp
    ../../../../../Apps/Samples/Common/FontConfigParser.h
    ../../../../../Apps/Samples/Common/FxAmpAppControlCallback.h
    ../../../../../Apps/Samples/Common/FxPlayerAppBase.h
    ../../../../../Apps/Samples/Common/FxPlayerAutotest.h
    ../../../../../Apps/Samples/Common/FxPlayerLog.h
    ../../../../../Apps/Samples/Common/FxRenderThread.h
    ../../../../../Apps/Samples/Common/FxSharedObjectManager.h
    ../../../../../Apps/Samples/Common/FxSoundFMOD.h
    ../../../../../Apps/Samples/FxPlayer/FxHUD.h
)

# Resource files
set(RESOURCE_FILES
    GFxPlayer.rc
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${RESOURCE_FILES})

# Include directories (D3D9_Profiling)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D9_Profiling>:$(VS100COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Include directories (D3D10_Profiling)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D10_Profiling>:$(VS100COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Include directories (D3D11_Profiling)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D11_Profiling>:$(VS100COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Include directories (GL_Profiling)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:GL_Profiling>:$(VS100COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Include directories (D3D9_Release_Static)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D9_Release_Static>:$(VS110COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Include directories (D3D10_Release_Static)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D10_Release_Static>:$(VS100COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Include directories (GL_Release_Static)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:GL_Release_Static>:$(VS100COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Include directories (D3D9_Shipping)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D9_Shipping>:$(VS100COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Include directories (D3D10_Shipping)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D10_Shipping>:$(VS100COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Include directories (D3D11_Shipping)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D11_Shipping>:$(VS100COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Include directories (GL_Shipping)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:GL_Shipping>:$(VS100COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Include directories (D3D9_Debug_Static)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D9_Debug_Static>:$(VS100COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Include directories (D3D10_Debug_Static)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D10_Debug_Static>:$(VS100COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Include directories (D3D11_Debug_Static)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D11_Debug_Static>:$(VS110COMNTOOLS)../../VC/PlatformSDK/Include>
    $<$<CONFIG:D3D11_Debug_Static>:./../../../../../3rdParty/libpng-1.6.53>
    $<$<CONFIG:D3D11_Debug_Static>:./../../../../../3rdParty/jpeg-10>
    $<$<CONFIG:D3D11_Debug_Static>:./../../../../../3rdParty/fmod-2.03.13/core/inc>
    $<$<CONFIG:D3D11_Debug_Static>:./../../../../../3rdParty/pcre>
    $<$<CONFIG:D3D11_Debug_Static>:./../../../../../3rdParty/glext>
)

# Include directories (GL_Debug_Static)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:GL_Debug_Static>:$(VS100COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Include directories (D3D9_DebugOpt_Static)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D9_DebugOpt_Static>:$(VS100COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Include directories (D3D10_DebugOpt_Static)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D10_DebugOpt_Static>:$(VS100COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Include directories (D3D11_DebugOpt_Static)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D11_DebugOpt_Static>:$(VS100COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Include directories (GL_DebugOpt_Static)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:GL_DebugOpt_Static>:$(VS100COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Include directories (D3D12_DebugOpt_Static)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D12_DebugOpt_Static>:$(VS110COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Include directories (D3D12_Debug_Static)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D12_Debug_Static>:$(VS110COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Include directories (D3D12_Release_Static)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D12_Release_Static>:$(VS110COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Include directories (D3D12_Shipping)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D12_Shipping>:$(VS110COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Include directories (Vulkan_DebugOpt_Static)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Vulkan_DebugOpt_Static>:$(VS110COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Include directories (Vulkan_Debug_Static)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Vulkan_Debug_Static>:$(VS110COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Include directories (Vulkan_Release_Static)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Vulkan_Release_Static>:$(VS110COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Include directories (Vulkan_Shipping)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Vulkan_Shipping>:$(VS110COMNTOOLS)../../VC/PlatformSDK/Include>
)

# Preprocessor definitions (D3D9_Profiling)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D9_Profiling>:UNICODE>
)

# Preprocessor definitions (D3D10_Profiling)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D10_Profiling>:UNICODE>
)

# Preprocessor definitions (D3D11_Profiling)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D11_Profiling>:UNICODE>
)

# Preprocessor definitions (GL_Profiling)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:GL_Profiling>:UNICODE>
)

# Preprocessor definitions (D3D9_Release_Static)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D9_Release_Static>:UNICODE>
)

# Preprocessor definitions (D3D10_Release_Static)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D10_Release_Static>:UNICODE>
)

# Preprocessor definitions (D3D11_Release_Static)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D11_Release_Static>:UNICODE>
)

# Preprocessor definitions (GL_Release_Static)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:GL_Release_Static>:UNICODE>
)

# Preprocessor definitions (D3D9_Shipping)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D9_Shipping>:UNICODE>
)

# Preprocessor definitions (D3D10_Shipping)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D10_Shipping>:UNICODE>
)

# Preprocessor definitions (D3D11_Shipping)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D11_Shipping>:UNICODE>
)

# Preprocessor definitions (GL_Shipping)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:GL_Shipping>:UNICODE>
)

# Preprocessor definitions (D3D9_Debug_Static)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D9_Debug_Static>:UNICODE>
)

# Preprocessor definitions (D3D10_Debug_Static)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D10_Debug_Static>:UNICODE>
)

# Preprocessor definitions (D3D11_Debug_Static)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D11_Debug_Static>:UNICODE>
    $<$<CONFIG:D3D11_Debug_Static>:_VC80_UPGRADE=0x0710>
)

# Preprocessor definitions (GL_Debug_Static)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:GL_Debug_Static>:UNICODE>
)

# Preprocessor definitions (D3D9_DebugOpt_Static)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D9_DebugOpt_Static>:UNICODE>
)

# Preprocessor definitions (D3D10_DebugOpt_Static)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D10_DebugOpt_Static>:UNICODE>
)

# Preprocessor definitions (D3D11_DebugOpt_Static)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D11_DebugOpt_Static>:UNICODE>
)

# Preprocessor definitions (GL_DebugOpt_Static)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:GL_DebugOpt_Static>:UNICODE>
)

# Preprocessor definitions (D3D12_DebugOpt_Static)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D12_DebugOpt_Static>:UNICODE>
)

# Preprocessor definitions (D3D12_Debug_Static)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D12_Debug_Static>:UNICODE>
)

# Preprocessor definitions (D3D12_Release_Static)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D12_Release_Static>:UNICODE>
)

# Preprocessor definitions (D3D12_Shipping)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D12_Shipping>:UNICODE>
)

# Preprocessor definitions (Vulkan_DebugOpt_Static)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Vulkan_DebugOpt_Static>:UNICODE>
)

# Preprocessor definitions (Vulkan_Debug_Static)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Vulkan_Debug_Static>:UNICODE>
)

# Preprocessor definitions (Vulkan_Release_Static)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Vulkan_Release_Static>:UNICODE>
)

# Preprocessor definitions (Vulkan_Shipping)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Vulkan_Shipping>:UNICODE>
)

# Link libraries (D3D9_Profiling)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D9_Profiling>:mfplat>
    $<$<CONFIG:D3D9_Profiling>:fmodL_vc>
    $<$<CONFIG:D3D9_Profiling>:fsbank_vc>
    $<$<CONFIG:D3D9_Profiling>:fmodstudioL_vc>
    $<$<CONFIG:D3D9_Profiling>:libjpeg>
    $<$<CONFIG:D3D9_Profiling>:LIBCMT>
    $<$<CONFIG:D3D9_Profiling>:dsound>
    $<$<CONFIG:D3D9_Profiling>:Msacm32>
    $<$<CONFIG:D3D9_Profiling>:Ws2_32>
)

# Link libraries (D3D10_Profiling)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D10_Profiling>:mfplat>
    $<$<CONFIG:D3D10_Profiling>:fmodL_vc>
    $<$<CONFIG:D3D10_Profiling>:fsbank_vc>
    $<$<CONFIG:D3D10_Profiling>:fmodstudioL_vc>
    $<$<CONFIG:D3D10_Profiling>:libjpeg>
    $<$<CONFIG:D3D10_Profiling>:LIBCMT>
    $<$<CONFIG:D3D10_Profiling>:dsound>
    $<$<CONFIG:D3D10_Profiling>:Msacm32>
    $<$<CONFIG:D3D10_Profiling>:Ws2_32>
)

# Link libraries (D3D11_Profiling)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D11_Profiling>:mfplat>
    $<$<CONFIG:D3D11_Profiling>:fmodL_vc>
    $<$<CONFIG:D3D11_Profiling>:fsbank_vc>
    $<$<CONFIG:D3D11_Profiling>:fmodstudioL_vc>
    $<$<CONFIG:D3D11_Profiling>:libjpeg>
    $<$<CONFIG:D3D11_Profiling>:LIBCMT>
    $<$<CONFIG:D3D11_Profiling>:dsound>
    $<$<CONFIG:D3D11_Profiling>:Msacm32>
    $<$<CONFIG:D3D11_Profiling>:Ws2_32>
    $<$<CONFIG:D3D11_Profiling>:legacy_stdio_definitions>
)

# Link libraries (GL_Profiling)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:GL_Profiling>:mfplat>
    $<$<CONFIG:GL_Profiling>:fmodL_vc>
    $<$<CONFIG:GL_Profiling>:fsbank_vc>
    $<$<CONFIG:GL_Profiling>:fmodstudioL_vc>
    $<$<CONFIG:GL_Profiling>:libjpeg>
    $<$<CONFIG:GL_Profiling>:LIBCMT>
    $<$<CONFIG:GL_Profiling>:dsound>
    $<$<CONFIG:GL_Profiling>:Msacm32>
    $<$<CONFIG:GL_Profiling>:Ws2_32>
    $<$<CONFIG:GL_Profiling>:legacy_stdio_definitions>
)

# Link libraries (D3D9_Release_Static)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D9_Release_Static>:fmodL_vc>
    $<$<CONFIG:D3D9_Release_Static>:fsbank_vc>
    $<$<CONFIG:D3D9_Release_Static>:fmodstudioL_vc>
    $<$<CONFIG:D3D9_Release_Static>:libjpeg>
    $<$<CONFIG:D3D9_Release_Static>:LIBCMT>
    $<$<CONFIG:D3D9_Release_Static>:dsound>
    $<$<CONFIG:D3D9_Release_Static>:Msacm32>
    $<$<CONFIG:D3D9_Release_Static>:Ws2_32>
    $<$<CONFIG:D3D9_Release_Static>:mfplat>
    $<$<CONFIG:D3D9_Release_Static>:legacy_stdio_definitions>
)

# Link libraries (D3D10_Release_Static)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D10_Release_Static>:fmodL_vc>
    $<$<CONFIG:D3D10_Release_Static>:fsbank_vc>
    $<$<CONFIG:D3D10_Release_Static>:fmodstudioL_vc>
    $<$<CONFIG:D3D10_Release_Static>:libjpeg>
    $<$<CONFIG:D3D10_Release_Static>:LIBCMT>
    $<$<CONFIG:D3D10_Release_Static>:dsound>
    $<$<CONFIG:D3D10_Release_Static>:Msacm32>
    $<$<CONFIG:D3D10_Release_Static>:Ws2_32>
    $<$<CONFIG:D3D10_Release_Static>:mfplat>
    $<$<CONFIG:D3D10_Release_Static>:legacy_stdio_definitions>
)

# Link libraries (D3D11_Release_Static)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D11_Release_Static>:fmodL_vc>
    $<$<CONFIG:D3D11_Release_Static>:fsbank_vc>
    $<$<CONFIG:D3D11_Release_Static>:fmodstudioL_vc>
    $<$<CONFIG:D3D11_Release_Static>:libjpeg>
    $<$<CONFIG:D3D11_Release_Static>:LIBCMT>
    $<$<CONFIG:D3D11_Release_Static>:dsound>
    $<$<CONFIG:D3D11_Release_Static>:Msacm32>
    $<$<CONFIG:D3D11_Release_Static>:Ws2_32>
    $<$<CONFIG:D3D11_Release_Static>:mfplat>
    $<$<CONFIG:D3D11_Release_Static>:legacy_stdio_definitions>
)

# Link libraries (GL_Release_Static)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:GL_Release_Static>:fmodL_vc>
    $<$<CONFIG:GL_Release_Static>:fsbank_vc>
    $<$<CONFIG:GL_Release_Static>:fmodstudioL_vc>
    $<$<CONFIG:GL_Release_Static>:libjpeg>
    $<$<CONFIG:GL_Release_Static>:LIBCMT>
    $<$<CONFIG:GL_Release_Static>:dsound>
    $<$<CONFIG:GL_Release_Static>:Msacm32>
    $<$<CONFIG:GL_Release_Static>:Ws2_32>
    $<$<CONFIG:GL_Release_Static>:mfplat>
    $<$<CONFIG:GL_Release_Static>:legacy_stdio_definitions>
)

# Link libraries (D3D9_Shipping)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D9_Shipping>:fmodL_vc>
    $<$<CONFIG:D3D9_Shipping>:fsbank_vc>
    $<$<CONFIG:D3D9_Shipping>:fmodstudioL_vc>
    $<$<CONFIG:D3D9_Shipping>:libjpeg>
    $<$<CONFIG:D3D9_Shipping>:LIBCMT>
    $<$<CONFIG:D3D9_Shipping>:dsound>
    $<$<CONFIG:D3D9_Shipping>:Msacm32>
    $<$<CONFIG:D3D9_Shipping>:Ws2_32>
    $<$<CONFIG:D3D9_Shipping>:mfplat>
    $<$<CONFIG:D3D9_Shipping>:legacy_stdio_definitions>
)

# Link libraries (D3D10_Shipping)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D10_Shipping>:fmodL_vc>
    $<$<CONFIG:D3D10_Shipping>:fsbank_vc>
    $<$<CONFIG:D3D10_Shipping>:fmodstudioL_vc>
    $<$<CONFIG:D3D10_Shipping>:libjpeg>
    $<$<CONFIG:D3D10_Shipping>:LIBCMT>
    $<$<CONFIG:D3D10_Shipping>:dsound>
    $<$<CONFIG:D3D10_Shipping>:Msacm32>
    $<$<CONFIG:D3D10_Shipping>:Ws2_32>
    $<$<CONFIG:D3D10_Shipping>:mfplat>
    $<$<CONFIG:D3D10_Shipping>:legacy_stdio_definitions>
)

# Link libraries (D3D11_Shipping)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D11_Shipping>:fmodL_vc>
    $<$<CONFIG:D3D11_Shipping>:fsbank_vc>
    $<$<CONFIG:D3D11_Shipping>:fmodstudioL_vc>
    $<$<CONFIG:D3D11_Shipping>:libjpeg>
    $<$<CONFIG:D3D11_Shipping>:LIBCMT>
    $<$<CONFIG:D3D11_Shipping>:dsound>
    $<$<CONFIG:D3D11_Shipping>:Msacm32>
    $<$<CONFIG:D3D11_Shipping>:Ws2_32>
    $<$<CONFIG:D3D11_Shipping>:mfplat>
    $<$<CONFIG:D3D11_Shipping>:legacy_stdio_definitions>
)

# Link libraries (GL_Shipping)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:GL_Shipping>:fmodL_vc>
    $<$<CONFIG:GL_Shipping>:fsbank_vc>
    $<$<CONFIG:GL_Shipping>:fmodstudioL_vc>
    $<$<CONFIG:GL_Shipping>:libjpeg>
    $<$<CONFIG:GL_Shipping>:LIBCMT>
    $<$<CONFIG:GL_Shipping>:dsound>
    $<$<CONFIG:GL_Shipping>:Msacm32>
    $<$<CONFIG:GL_Shipping>:Ws2_32>
    $<$<CONFIG:GL_Shipping>:mfplat>
    $<$<CONFIG:GL_Shipping>:legacy_stdio_definitions>
)

# Link libraries (D3D9_Debug_Static)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D9_Debug_Static>:fmodL_vc>
    $<$<CONFIG:D3D9_Debug_Static>:fsbank_vc>
    $<$<CONFIG:D3D9_Debug_Static>:fmodstudioL_vc>
    $<$<CONFIG:D3D9_Debug_Static>:libjpeg>
    $<$<CONFIG:D3D9_Debug_Static>:LIBCMT>
    $<$<CONFIG:D3D9_Debug_Static>:dsound>
    $<$<CONFIG:D3D9_Debug_Static>:mfplat>
    $<$<CONFIG:D3D9_Debug_Static>:Msacm32>
    $<$<CONFIG:D3D9_Debug_Static>:Ws2_32>
    $<$<CONFIG:D3D9_Debug_Static>:legacy_stdio_definitions>
)

# Link libraries (D3D10_Debug_Static)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D10_Debug_Static>:mfplat>
    $<$<CONFIG:D3D10_Debug_Static>:fmodL_vc>
    $<$<CONFIG:D3D10_Debug_Static>:fsbank_vc>
    $<$<CONFIG:D3D10_Debug_Static>:fmodstudioL_vc>
    $<$<CONFIG:D3D10_Debug_Static>:libjpeg>
    $<$<CONFIG:D3D10_Debug_Static>:LIBCMT>
    $<$<CONFIG:D3D10_Debug_Static>:dsound>
    $<$<CONFIG:D3D10_Debug_Static>:Msacm32>
    $<$<CONFIG:D3D10_Debug_Static>:Ws2_32>
    $<$<CONFIG:D3D10_Debug_Static>:legacy_stdio_definitions>
)

# Link libraries (D3D11_Debug_Static)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D11_Debug_Static>:msvcrt>
    $<$<CONFIG:D3D11_Debug_Static>:libucrt>
    $<$<CONFIG:D3D11_Debug_Static>:fmodL_vc>
    $<$<CONFIG:D3D11_Debug_Static>:fsbank_vc>
    $<$<CONFIG:D3D11_Debug_Static>:fmodstudioL_vc>
    $<$<CONFIG:D3D11_Debug_Static>:libjpeg>
    $<$<CONFIG:D3D11_Debug_Static>:libcmt>
    $<$<CONFIG:D3D11_Debug_Static>:dsound>
    $<$<CONFIG:D3D11_Debug_Static>:Msacm32>
    $<$<CONFIG:D3D11_Debug_Static>:Ws2_32>
    $<$<CONFIG:D3D11_Debug_Static>:legacy_stdio_definitions>
    $<$<CONFIG:D3D11_Debug_Static>:winmm>
    $<$<CONFIG:D3D11_Debug_Static>:mfplat>
)

# Link libraries (GL_Debug_Static)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:GL_Debug_Static>:fmodL_vc>
    $<$<CONFIG:GL_Debug_Static>:fsbank_vc>
    $<$<CONFIG:GL_Debug_Static>:fmodstudioL_vc>
    $<$<CONFIG:GL_Debug_Static>:libjpeg>
    $<$<CONFIG:GL_Debug_Static>:LIBCMT>
    $<$<CONFIG:GL_Debug_Static>:dsound>
    $<$<CONFIG:GL_Debug_Static>:mfplat>
    $<$<CONFIG:GL_Debug_Static>:Msacm32>
    $<$<CONFIG:GL_Debug_Static>:Ws2_32>
    $<$<CONFIG:GL_Debug_Static>:legacy_stdio_definitions>
)

# Link libraries (D3D9_DebugOpt_Static)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D9_DebugOpt_Static>:fmodL_vc>
    $<$<CONFIG:D3D9_DebugOpt_Static>:fsbank_vc>
    $<$<CONFIG:D3D9_DebugOpt_Static>:fmodstudioL_vc>
    $<$<CONFIG:D3D9_DebugOpt_Static>:mfplat>
    $<$<CONFIG:D3D9_DebugOpt_Static>:libjpeg>
    $<$<CONFIG:D3D9_DebugOpt_Static>:LIBCMT>
    $<$<CONFIG:D3D9_DebugOpt_Static>:dsound>
    $<$<CONFIG:D3D9_DebugOpt_Static>:Msacm32>
    $<$<CONFIG:D3D9_DebugOpt_Static>:Ws2_32>
    $<$<CONFIG:D3D9_DebugOpt_Static>:legacy_stdio_definitions>
)

# Link libraries (D3D10_DebugOpt_Static)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D10_DebugOpt_Static>:fmodL_vc>
    $<$<CONFIG:D3D10_DebugOpt_Static>:fsbank_vc>
    $<$<CONFIG:D3D10_DebugOpt_Static>:fmodstudioL_vc>
    $<$<CONFIG:D3D10_DebugOpt_Static>:libjpeg>
    $<$<CONFIG:D3D10_DebugOpt_Static>:LIBCMT>
    $<$<CONFIG:D3D10_DebugOpt_Static>:dsound>
    $<$<CONFIG:D3D10_DebugOpt_Static>:Msacm32>
    $<$<CONFIG:D3D10_DebugOpt_Static>:mfplat>
    $<$<CONFIG:D3D10_DebugOpt_Static>:Ws2_32>
    $<$<CONFIG:D3D10_DebugOpt_Static>:legacy_stdio_definitions>
)

# Link libraries (D3D11_DebugOpt_Static)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D11_DebugOpt_Static>:fmodL_vc>
    $<$<CONFIG:D3D11_DebugOpt_Static>:fsbank_vc>
    $<$<CONFIG:D3D11_DebugOpt_Static>:fmodstudioL_vc>
    $<$<CONFIG:D3D11_DebugOpt_Static>:mfplat>
    $<$<CONFIG:D3D11_DebugOpt_Static>:ucrt>
    $<$<CONFIG:D3D11_DebugOpt_Static>:libjpeg>
    $<$<CONFIG:D3D11_DebugOpt_Static>:LIBCMT>
    $<$<CONFIG:D3D11_DebugOpt_Static>:dsound>
    $<$<CONFIG:D3D11_DebugOpt_Static>:Msacm32>
    $<$<CONFIG:D3D11_DebugOpt_Static>:Ws2_32>
    $<$<CONFIG:D3D11_DebugOpt_Static>:legacy_stdio_definitions>
)

# Link libraries (GL_DebugOpt_Static)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:GL_DebugOpt_Static>:fmodL_vc>
    $<$<CONFIG:GL_DebugOpt_Static>:mfplat>
    $<$<CONFIG:GL_DebugOpt_Static>:fsbank_vc>
    $<$<CONFIG:GL_DebugOpt_Static>:fmodstudioL_vc>
    $<$<CONFIG:GL_DebugOpt_Static>:libjpeg>
    $<$<CONFIG:GL_DebugOpt_Static>:LIBCMT>
    $<$<CONFIG:GL_DebugOpt_Static>:dsound>
    $<$<CONFIG:GL_DebugOpt_Static>:Msacm32>
    $<$<CONFIG:GL_DebugOpt_Static>:Ws2_32>
    $<$<CONFIG:GL_DebugOpt_Static>:legacy_stdio_definitions>
)

# Link libraries (D3D12_DebugOpt_Static)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D12_DebugOpt_Static>:xaudio2>
    $<$<CONFIG:D3D12_DebugOpt_Static>:Msacm32>
    $<$<CONFIG:D3D12_DebugOpt_Static>:Ws2_32>
    $<$<CONFIG:D3D12_DebugOpt_Static>:mfplat>
    $<$<CONFIG:D3D12_DebugOpt_Static>:legacy_stdio_definitions>
)

# Link libraries (D3D12_Debug_Static)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D12_Debug_Static>:xaudio2>
    $<$<CONFIG:D3D12_Debug_Static>:Msacm32>
    $<$<CONFIG:D3D12_Debug_Static>:Ws2_32>
    $<$<CONFIG:D3D12_Debug_Static>:mfplat>
    $<$<CONFIG:D3D12_Debug_Static>:legacy_stdio_definitions>
)

# Link libraries (D3D12_Release_Static)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D12_Release_Static>:xaudio2>
    $<$<CONFIG:D3D12_Release_Static>:Msacm32>
    $<$<CONFIG:D3D12_Release_Static>:Ws2_32>
    $<$<CONFIG:D3D12_Release_Static>:mfplat>
    $<$<CONFIG:D3D12_Release_Static>:legacy_stdio_definitions>
)

# Link libraries (D3D12_Shipping)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D12_Shipping>:xaudio2>
    $<$<CONFIG:D3D12_Shipping>:Msacm32>
    $<$<CONFIG:D3D12_Shipping>:Ws2_32>
    $<$<CONFIG:D3D12_Shipping>:mfplat>
    $<$<CONFIG:D3D12_Shipping>:legacy_stdio_definitions>
)

# Link libraries (Vulkan_DebugOpt_Static)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Vulkan_DebugOpt_Static>:xaudio2>
    $<$<CONFIG:Vulkan_DebugOpt_Static>:Msacm32>
    $<$<CONFIG:Vulkan_DebugOpt_Static>:Ws2_32>
    $<$<CONFIG:Vulkan_DebugOpt_Static>:mfplat>
    $<$<CONFIG:Vulkan_DebugOpt_Static>:legacy_stdio_definitions>
)

# Link libraries (Vulkan_Debug_Static)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Vulkan_Debug_Static>:xaudio2>
    $<$<CONFIG:Vulkan_Debug_Static>:Msacm32>
    $<$<CONFIG:Vulkan_Debug_Static>:Ws2_32>
    $<$<CONFIG:Vulkan_Debug_Static>:mfplat>
    $<$<CONFIG:Vulkan_Debug_Static>:legacy_stdio_definitions>
)

# Link libraries (Vulkan_Release_Static)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Vulkan_Release_Static>:xaudio2>
    $<$<CONFIG:Vulkan_Release_Static>:Msacm32>
    $<$<CONFIG:Vulkan_Release_Static>:Ws2_32>
    $<$<CONFIG:Vulkan_Release_Static>:mfplat>
    $<$<CONFIG:Vulkan_Release_Static>:legacy_stdio_definitions>
)

# Link libraries (Vulkan_Shipping)
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Vulkan_Shipping>:xaudio2>
    $<$<CONFIG:Vulkan_Shipping>:Msacm32>
    $<$<CONFIG:Vulkan_Shipping>:Ws2_32>
    $<$<CONFIG:Vulkan_Shipping>:mfplat>
    $<$<CONFIG:Vulkan_Shipping>:legacy_stdio_definitions>
)

# Compiler options (D3D9_Profiling)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D9_Profiling>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:D3D9_Profiling>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:D3D9_Profiling>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:D3D9_Profiling>:$<$<CXX_COMPILER_ID:MSVC>:/Gy>>
)

# Compiler options (D3D10_Profiling)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D10_Profiling>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:D3D10_Profiling>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:D3D10_Profiling>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:D3D10_Profiling>:$<$<CXX_COMPILER_ID:MSVC>:/Gy>>
)

# Compiler options (D3D11_Profiling)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D11_Profiling>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:D3D11_Profiling>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:D3D11_Profiling>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:D3D11_Profiling>:$<$<CXX_COMPILER_ID:MSVC>:/Gy>>
)

# Compiler options (GL_Profiling)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:GL_Profiling>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:GL_Profiling>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:GL_Profiling>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:GL_Profiling>:$<$<CXX_COMPILER_ID:MSVC>:/Gy>>
)

# Compiler options (D3D9_Release_Static)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D9_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:D3D9_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:D3D9_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:D3D9_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Gy>>
)

# Compiler options (D3D10_Release_Static)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D10_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:D3D10_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:D3D10_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:D3D10_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Gy>>
)

# Compiler options (D3D11_Release_Static)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D11_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:D3D11_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:D3D11_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:D3D11_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Gy>>
)

# Compiler options (GL_Release_Static)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:GL_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:GL_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:GL_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:GL_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Gy>>
)

# Compiler options (D3D9_Shipping)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D9_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:D3D9_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:D3D9_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:D3D9_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/Gy>>
)

# Compiler options (D3D10_Shipping)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D10_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:D3D10_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:D3D10_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:D3D10_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/Gy>>
)

# Compiler options (D3D11_Shipping)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D11_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:D3D11_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:D3D11_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/Gy>>
)

# Compiler options (GL_Shipping)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:GL_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:GL_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:GL_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:GL_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/Gy>>
)

# Compiler options (D3D9_Debug_Static)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D9_Debug_Static>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:D3D9_Debug_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:D3D9_Debug_Static>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:D3D9_Debug_Static>:$<$<CXX_COMPILER_ID:MSVC>:/RTC1>>
)

# Compiler options (D3D10_Debug_Static)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D10_Debug_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:D3D10_Debug_Static>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:D3D10_Debug_Static>:$<$<CXX_COMPILER_ID:MSVC>:/RTC1>>
)

# Compiler options (D3D11_Debug_Static)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D11_Debug_Static>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:D3D11_Debug_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:D3D11_Debug_Static>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:D3D11_Debug_Static>:$<$<CXX_COMPILER_ID:MSVC>:/RTC1>>
)

# Compiler options (GL_Debug_Static)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:GL_Debug_Static>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:GL_Debug_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:GL_Debug_Static>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:GL_Debug_Static>:$<$<CXX_COMPILER_ID:MSVC>:/RTC1>>
)

# Compiler options (D3D9_DebugOpt_Static)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D9_DebugOpt_Static>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:D3D9_DebugOpt_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:D3D9_DebugOpt_Static>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:D3D9_DebugOpt_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Gy>>
)

# Compiler options (D3D10_DebugOpt_Static)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D10_DebugOpt_Static>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:D3D10_DebugOpt_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:D3D10_DebugOpt_Static>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:D3D10_DebugOpt_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Gy>>
)

# Compiler options (D3D11_DebugOpt_Static)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D11_DebugOpt_Static>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:D3D11_DebugOpt_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:D3D11_DebugOpt_Static>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:D3D11_DebugOpt_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Gy>>
)

# Compiler options (GL_DebugOpt_Static)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:GL_DebugOpt_Static>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:GL_DebugOpt_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:GL_DebugOpt_Static>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:GL_DebugOpt_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Gy>>
)

# Compiler options (D3D12_DebugOpt_Static)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D12_DebugOpt_Static>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:D3D12_DebugOpt_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:D3D12_DebugOpt_Static>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:D3D12_DebugOpt_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Gy>>
)

# Compiler options (D3D12_Debug_Static)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D12_Debug_Static>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:D3D12_Debug_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:D3D12_Debug_Static>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:D3D12_Debug_Static>:$<$<CXX_COMPILER_ID:MSVC>:/RTC1>>
)

# Compiler options (D3D12_Release_Static)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D12_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:D3D12_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:D3D12_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:D3D12_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Gy>>
)

# Compiler options (D3D12_Shipping)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D12_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:D3D12_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:D3D12_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:D3D12_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/Gy>>
)

# Compiler options (Vulkan_DebugOpt_Static)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Vulkan_DebugOpt_Static>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:Vulkan_DebugOpt_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:Vulkan_DebugOpt_Static>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:Vulkan_DebugOpt_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Gy>>
)

# Compiler options (Vulkan_Debug_Static)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Vulkan_Debug_Static>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:Vulkan_Debug_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:Vulkan_Debug_Static>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:Vulkan_Debug_Static>:$<$<CXX_COMPILER_ID:MSVC>:/RTC1>>
)

# Compiler options (Vulkan_Release_Static)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Vulkan_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:Vulkan_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:Vulkan_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:Vulkan_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/Gy>>
)

# Compiler options (Vulkan_Shipping)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Vulkan_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>
    $<$<CONFIG:Vulkan_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/Z7>>
    $<$<CONFIG:Vulkan_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/GR->>
    $<$<CONFIG:Vulkan_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/Gy>>
)

# Linker options (D3D9_Profiling)
target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D9_Profiling>:$<$<CXX_COMPILER_ID:MSVC>:/MAP>>
)

# Linker options (D3D10_Profiling)
target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D10_Profiling>:$<$<CXX_COMPILER_ID:MSVC>:/MAP>>
)

# Linker options (D3D11_Profiling)
target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D11_Profiling>:$<$<CXX_COMPILER_ID:MSVC>:/MAP>>
)

# Linker options (GL_Profiling)
target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:GL_Profiling>:$<$<CXX_COMPILER_ID:MSVC>:/MAP>>
)

# Linker options (D3D9_Release_Static)
target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D9_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/MAP>>
)

# Linker options (D3D10_Release_Static)
target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D10_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/MAP>>
)

# Linker options (D3D11_Release_Static)
target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D11_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/MAP>>
)

# Linker options (GL_Release_Static)
target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:GL_Release_Static>:$<$<CXX_COMPILER_ID:MSVC>:/MAP>>
)

# Linker options (D3D9_Shipping)
target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D9_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/MAP>>
)

# Linker options (D3D10_Shipping)
target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D10_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/MAP>>
)

# Linker options (D3D11_Shipping)
target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D11_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/MAP>>
)

# Linker options (GL_Shipping)
target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:GL_Shipping>:$<$<CXX_COMPILER_ID:MSVC>:/MAP>>
)

# Linker options (D3D9_Debug_Static)
target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:D3D9_Debug_Static>:$<$<CXX_COMPILER_ID:MSVC>:/MAP>>
)

# Linker options (GL_DebugOpt_Static)
target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:GL_DebugOpt_Static>:$<$<CXX_COMPILER_ID:MSVC>:/MAP>>
)

# MSVC runtime library
set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "$<$<CONFIG:D3D9_Profiling>:MultiThreaded>$<$<CONFIG:D3D10_Profiling>:MultiThreaded>$<$<CONFIG:D3D11_Profiling>:MultiThreaded>$<$<CONFIG:GL_Profiling>:MultiThreaded>$<$<CONFIG:D3D9_Release_Static>:MultiThreaded>$<$<CONFIG:D3D10_Release_Static>:MultiThreaded>$<$<CONFIG:D3D11_Release_Static>:MultiThreaded>$<$<CONFIG:GL_Release_Static>:MultiThreaded>$<$<CONFIG:D3D9_Shipping>:MultiThreaded>$<$<CONFIG:D3D10_Shipping>:MultiThreaded>$<$<CONFIG:D3D11_Shipping>:MultiThreaded>$<$<CONFIG:GL_Shipping>:MultiThreaded>$<$<CONFIG:D3D9_Debug_Static>:MultiThreaded>$<$<CONFIG:D3D10_Debug_Static>:MultiThreaded>$<$<CONFIG:D3D11_Debug_Static>:MultiThreaded>$<$<CONFIG:GL_Debug_Static>:MultiThreaded>$<$<CONFIG:D3D9_DebugOpt_Static>:MultiThreaded>$<$<CONFIG:D3D10_DebugOpt_Static>:MultiThreaded>$<$<CONFIG:D3D11_DebugOpt_Static>:MultiThreaded>$<$<CONFIG:GL_DebugOpt_Static>:MultiThreaded>$<$<CONFIG:D3D12_DebugOpt_Static>:MultiThreaded>$<$<CONFIG:D3D12_Debug_Static>:MultiThreadedDebug>$<$<CONFIG:D3D12_Release_Static>:MultiThreaded>$<$<CONFIG:D3D12_Shipping>:MultiThreaded>$<$<CONFIG:Vulkan_DebugOpt_Static>:MultiThreaded>$<$<CONFIG:Vulkan_Debug_Static>:MultiThreadedDebug>$<$<CONFIG:Vulkan_Release_Static>:MultiThreaded>$<$<CONFIG:Vulkan_Shipping>:MultiThreaded>")

# Project dependencies
# Dependency: Expat (../../SDK/Expat/Expat.vcxproj)
# add_dependencies(${PROJECT_NAME} Expat)
# Dependency: Platform (../../SDK/Platform/Platform.vcxproj)
# add_dependencies(${PROJECT_NAME} Platform)
# Dependency: Render_D3D1x (../../SDK/Render/Render_D3D1x.vcxproj)
# add_dependencies(${PROJECT_NAME} Render_D3D1x)
# Dependency: Render_D3D9 (../../SDK/Render/Render_D3D9.vcxproj)
# add_dependencies(${PROJECT_NAME} Render_D3D9)
# Dependency: Render_GL (../../SDK/Render/Render_GL.vcxproj)
# add_dependencies(${PROJECT_NAME} Render_GL)
# Dependency: Sound (../../SDK/Sound/Sound.vcxproj)
# add_dependencies(${PROJECT_NAME} Sound)
# Dependency: Render_D3D12 (../../SDK/Render/Render_D3D12.vcxproj)
# add_dependencies(${PROJECT_NAME} Render_D3D12)
# Dependency: Render_Vulkan (../../SDK/Render/Render_Vulkan.vcxproj)
# add_dependencies(${PROJECT_NAME} Render_Vulkan)
