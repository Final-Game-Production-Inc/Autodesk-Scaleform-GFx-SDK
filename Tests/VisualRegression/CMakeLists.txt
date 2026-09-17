# Visual regression test tools.

# ---------------------------------------------------------------------------
# CaptureVulkan -- headless Vulkan frame capture for visual regression
# ---------------------------------------------------------------------------
if(SF_RENDERER_VULKAN)
    add_executable(CaptureVulkan
        ${CMAKE_CURRENT_SOURCE_DIR}/CaptureVulkan.cpp
    )
    sf_apply_app_flags(CaptureVulkan)
    target_link_libraries(CaptureVulkan PRIVATE
        sf_core
        gfxrender_vulkan
    )
    target_link_options(CaptureVulkan PRIVATE
        /WHOLEARCHIVE:libgfxrender_vulkan
    )
    set_target_properties(CaptureVulkan PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,Win32>/Msvc10/Tests"
    )
endif()

# ---------------------------------------------------------------------------
# CaptureD3D11 -- headless D3D11 frame capture for visual regression
# ---------------------------------------------------------------------------
if(SF_RENDERER_D3D11)
    add_executable(CaptureD3D11
        ${CMAKE_CURRENT_SOURCE_DIR}/CaptureD3D11.cpp
    )
    sf_apply_app_flags(CaptureD3D11)
    target_link_libraries(CaptureD3D11 PRIVATE
        sf_core
        gfxrender_d3d1x
    )
    set_target_properties(CaptureD3D11 PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,Win32>/Msvc10/Tests"
    )
endif()
