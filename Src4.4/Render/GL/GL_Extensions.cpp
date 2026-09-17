/**********************************************************************

Filename    :   GL_Extensions.cpp
Content     :   GL  extension implementation.
Created     :   Automatically generated.
Authors     :   Extensions.pl

Copyright   :   (c) 2001-2014 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#include "Render/GL/GL_Extensions.h"
#include "Render/GL/GL_Common.h"
#include "Kernel/SF_Alg.h"
#include "Kernel/SF_String.h"

namespace Scaleform { namespace Render { namespace GL {

GLInterface::GLInterface() :
    MajorVersion(0), MinorVersion(0)
{
    memset(ExtensionSupported, 0, sizeof(ExtensionSupported));
}

GLInterface::GLInterface(const GLInterface& other) :
    MajorVersion(other.MajorVersion),
    MinorVersion(other.MinorVersion)
{
    memcpy(ExtensionSupported, other.ExtensionSupported, sizeof(ExtensionSupported));
}

bool GLInterface::CheckExtension(GLExtensionType ext)
{
    return ExtensionSupported[ext];
}

bool GLInterface::CheckGLVersion(unsigned reqMajor, unsigned reqMinor)
{
    return (MajorVersion > reqMajor || (MajorVersion == reqMajor && MinorVersion >= reqMinor));
}


GLImmediate::GLImmediate()
#if defined(SF_GL_RUNTIME_LINK)
    :
    p_glActiveTexture(0),
    p_glAttachShader(0),
    p_glBindAttribLocation(0),
    p_glBindBuffer(0),
    p_glBindFragDataLocation(0),
    p_glBindFramebuffer(0),
    p_glBindProgramPipeline(0),
    p_glBindRenderbuffer(0),
    p_glBindVertexArray(0),
    p_glBlendEquation(0),
    p_glBlendEquationSeparate(0),
    p_glBlendFuncSeparate(0),
    p_glBufferData(0),
    p_glBufferSubData(0),
    p_glCheckFramebufferStatus(0),
    p_glClientWaitSync(0),
    p_glCompileShader(0),
    p_glCompressedTexImage2D(0),
    p_glCreateProgram(0),
    p_glCreateShader(0),
    p_glDebugMessageCallback(0),
    p_glDebugMessageControl(0),
    p_glDeleteBuffers(0),
    p_glDeleteFramebuffers(0),
    p_glDeleteProgram(0),
    p_glDeleteProgramPipelines(0),
    p_glDeleteRenderbuffers(0),
    p_glDeleteShader(0),
    p_glDeleteSync(0),
    p_glDeleteVertexArrays(0),
    p_glDisableVertexAttribArray(0),
    p_glDrawElementsInstanced(0),
    p_glEnableVertexAttribArray(0),
    p_glFenceSync(0),
    p_glFlushMappedBufferRange(0),
    p_glFramebufferRenderbuffer(0),
    p_glFramebufferTexture2D(0),
    p_glGenBuffers(0),
    p_glGenFramebuffers(0),
    p_glGenProgramPipelines(0),
    p_glGenRenderbuffers(0),
    p_glGenVertexArrays(0),
    p_glGenerateMipmap(0),
    p_glGetActiveUniform(0),
    p_glGetFragDataLocation(0),
    p_glGetFramebufferAttachmentParameteriv(0),
    p_glGetProgramBinary(0),
    p_glGetProgramInfoLog(0),
    p_glGetProgramiv(0),
    p_glGetRenderbufferParameteriv(0),
    p_glGetShaderInfoLog(0),
    p_glGetShaderiv(0),
    p_glGetStringi(0),
    p_glGetSynciv(0),
    p_glGetTexImage(0),
    p_glGetTexLevelParameteriv(0),
    p_glGetUniformLocation(0),
    p_glIsFramebuffer(0),
    p_glIsProgram(0),
    p_glIsRenderbuffer(0),
    p_glLinkProgram(0),
    p_glMapBuffer(0),
    p_glMapBufferRange(0),
    p_glPolygonMode(0),
    p_glPopGroupMarker(0),
    p_glProgramBinary(0),
    p_glProgramParameteri(0),
    p_glProgramUniform1fv(0),
    p_glProgramUniform1iv(0),
    p_glProgramUniform2fv(0),
    p_glProgramUniform3fv(0),
    p_glProgramUniform4fv(0),
    p_glProgramUniformMatrix4fv(0),
    p_glPushGroupMarker(0),
    p_glReleaseShaderCompiler(0),
    p_glRenderbufferStorage(0),
    p_glShaderSource(0),
    p_glStringMarker(0),
    p_glUniform1f(0),
    p_glUniform1fv(0),
    p_glUniform1i(0),
    p_glUniform1iv(0),
    p_glUniform2f(0),
    p_glUniform2fv(0),
    p_glUniform3fv(0),
    p_glUniform4fv(0),
    p_glUniformMatrix4fv(0),
    p_glUnmapBuffer(0),
    p_glUseProgram(0),
    p_glUseProgramStages(0),
    p_glVertexAttribPointer(0)
#endif // SF_GL_RUNTIME_LINK
{
}

static const char* ExtensionStrings[] =
{
    "SF_GL_NONE",
    "GL_3DFX_multisample",
    "GL_3DFX_tbuffer",
    "GL_3DFX_texture_compression_FXT1",
    "GL_AMD_blend_minmax_factor",
    "GL_AMD_compressed_3DC_texture",
    "GL_AMD_compressed_ATC_texture",
    "GL_AMD_conservative_depth",
    "GL_AMD_debug_output",
    "GL_AMD_depth_clamp_separate",
    "GL_AMD_draw_buffers_blend",
    "GL_AMD_interleaved_elements",
    "GL_AMD_multi_draw_indirect",
    "GL_AMD_name_gen_delete",
    "GL_AMD_occlusion_query_event",
    "GL_AMD_performance_monitor",
    "GL_AMD_pinned_memory",
    "GL_AMD_program_binary_Z400",
    "GL_AMD_query_buffer_object",
    "GL_AMD_sample_positions",
    "GL_AMD_seamless_cubemap_per_texture",
    "GL_AMD_shader_atomic_counter_ops",
    "GL_AMD_shader_stencil_export",
    "GL_AMD_shader_trinary_minmax",
    "GL_AMD_sparse_texture",
    "GL_AMD_stencil_operation_extended",
    "GL_AMD_texture_texture4",
    "GL_AMD_transform_feedback3_lines_triangles",
    "GL_AMD_vertex_shader_layer",
    "GL_AMD_vertex_shader_tessellator",
    "GL_AMD_vertex_shader_viewport_index",
    "GL_ANGLE_depth_texture",
    "GL_ANGLE_framebuffer_blit",
    "GL_ANGLE_framebuffer_multisample",
    "GL_ANGLE_instanced_arrays",
    "GL_ANGLE_pack_reverse_row_order",
    "GL_ANGLE_program_binary",
    "GL_ANGLE_texture_compression_dxt3",
    "GL_ANGLE_texture_compression_dxt5",
    "GL_ANGLE_texture_usage",
    "GL_ANGLE_translated_shader_source",
    "GL_APPLE_aux_depth_stencil",
    "GL_APPLE_client_storage",
    "GL_APPLE_copy_texture_levels",
    "GL_APPLE_element_array",
    "GL_APPLE_fence",
    "GL_APPLE_float_pixels",
    "GL_APPLE_flush_buffer_range",
    "GL_APPLE_framebuffer_multisample",
    "GL_APPLE_object_purgeable",
    "GL_APPLE_rgb_422",
    "GL_APPLE_row_bytes",
    "GL_APPLE_specular_vector",
    "GL_APPLE_sync",
    "GL_APPLE_texture_format_BGRA8888",
    "GL_APPLE_texture_max_level",
    "GL_APPLE_texture_range",
    "GL_APPLE_transform_hint",
    "GL_APPLE_vertex_array_object",
    "GL_APPLE_vertex_array_range",
    "GL_APPLE_vertex_program_evaluators",
    "GL_APPLE_ycbcr_422",
    "GL_ARB_ES2_compatibility",
    "GL_ARB_ES3_compatibility",
    "GL_ARB_arrays_of_arrays",
    "GL_ARB_base_instance",
    "GL_ARB_bindless_texture",
    "GL_ARB_blend_func_extended",
    "GL_ARB_buffer_storage",
    "GL_ARB_cl_event",
    "GL_ARB_clear_buffer_object",
    "GL_ARB_clear_texture",
    "GL_ARB_color_buffer_float",
    "GL_ARB_compatibility",
    "GL_ARB_compressed_texture_pixel_storage",
    "GL_ARB_compute_shader",
    "GL_ARB_compute_variable_group_size",
    "GL_ARB_conservative_depth",
    "GL_ARB_copy_buffer",
    "GL_ARB_copy_image",
    "GL_ARB_debug_output",
    "GL_ARB_depth_buffer_float",
    "GL_ARB_depth_clamp",
    "GL_ARB_depth_texture",
    "GL_ARB_draw_buffers",
    "GL_ARB_draw_buffers_blend",
    "GL_ARB_draw_elements_base_vertex",
    "GL_ARB_draw_indirect",
    "GL_ARB_draw_instanced",
    "GL_ARB_enhanced_layouts",
    "GL_ARB_explicit_attrib_location",
    "GL_ARB_explicit_uniform_location",
    "GL_ARB_fragment_coord_conventions",
    "GL_ARB_fragment_layer_viewport",
    "GL_ARB_fragment_program",
    "GL_ARB_fragment_program_shadow",
    "GL_ARB_fragment_shader",
    "GL_ARB_framebuffer_no_attachments",
    "GL_ARB_framebuffer_object",
    "GL_ARB_framebuffer_sRGB",
    "GL_ARB_geometry_shader4",
    "GL_ARB_get_program_binary",
    "GL_ARB_gpu_shader5",
    "GL_ARB_gpu_shader_fp64",
    "GL_ARB_half_float_pixel",
    "GL_ARB_half_float_vertex",
    "GL_ARB_imaging",
    "GL_ARB_indirect_parameters",
    "GL_ARB_instanced_arrays",
    "GL_ARB_internalformat_query",
    "GL_ARB_internalformat_query2",
    "GL_ARB_invalidate_subdata",
    "GL_ARB_map_buffer_alignment",
    "GL_ARB_map_buffer_range",
    "GL_ARB_matrix_palette",
    "GL_ARB_multi_bind",
    "GL_ARB_multi_draw_indirect",
    "GL_ARB_multisample",
    "GL_ARB_multitexture",
    "GL_ARB_occlusion_query",
    "GL_ARB_occlusion_query2",
    "GL_ARB_pixel_buffer_object",
    "GL_ARB_point_parameters",
    "GL_ARB_point_sprite",
    "GL_ARB_program_interface_query",
    "GL_ARB_provoking_vertex",
    "GL_ARB_query_buffer_object",
    "GL_ARB_robust_buffer_access_behavior",
    "GL_ARB_robustness",
    "GL_ARB_robustness_isolation",
    "GL_ARB_sample_shading",
    "GL_ARB_sampler_objects",
    "GL_ARB_seamless_cube_map",
    "GL_ARB_seamless_cubemap_per_texture",
    "GL_ARB_separate_shader_objects",
    "GL_ARB_shader_atomic_counters",
    "GL_ARB_shader_bit_encoding",
    "GL_ARB_shader_draw_parameters",
    "GL_ARB_shader_group_vote",
    "GL_ARB_shader_image_load_store",
    "GL_ARB_shader_image_size",
    "GL_ARB_shader_objects",
    "GL_ARB_shader_precision",
    "GL_ARB_shader_stencil_export",
    "GL_ARB_shader_storage_buffer_object",
    "GL_ARB_shader_subroutine",
    "GL_ARB_shader_texture_lod",
    "GL_ARB_shading_language_100",
    "GL_ARB_shading_language_420pack",
    "GL_ARB_shading_language_include",
    "GL_ARB_shading_language_packing",
    "GL_ARB_shadow",
    "GL_ARB_shadow_ambient",
    "GL_ARB_sparse_texture",
    "GL_ARB_stencil_texturing",
    "GL_ARB_sync",
    "GL_ARB_tessellation_shader",
    "GL_ARB_texture_border_clamp",
    "GL_ARB_texture_buffer_object",
    "GL_ARB_texture_buffer_object_rgb32",
    "GL_ARB_texture_buffer_range",
    "GL_ARB_texture_compression",
    "GL_ARB_texture_compression_bptc",
    "GL_ARB_texture_compression_rgtc",
    "GL_ARB_texture_cube_map",
    "GL_ARB_texture_cube_map_array",
    "GL_ARB_texture_env_add",
    "GL_ARB_texture_env_combine",
    "GL_ARB_texture_env_crossbar",
    "GL_ARB_texture_env_dot3",
    "GL_ARB_texture_float",
    "GL_ARB_texture_gather",
    "GL_ARB_texture_mirror_clamp_to_edge",
    "GL_ARB_texture_mirrored_repeat",
    "GL_ARB_texture_multisample",
    "GL_ARB_texture_non_power_of_two",
    "GL_ARB_texture_query_levels",
    "GL_ARB_texture_query_lod",
    "GL_ARB_texture_rectangle",
    "GL_ARB_texture_rg",
    "GL_ARB_texture_rgb10_a2ui",
    "GL_ARB_texture_stencil8",
    "GL_ARB_texture_storage",
    "GL_ARB_texture_storage_multisample",
    "GL_ARB_texture_swizzle",
    "GL_ARB_texture_view",
    "GL_ARB_timer_query",
    "GL_ARB_transform_feedback2",
    "GL_ARB_transform_feedback3",
    "GL_ARB_transform_feedback_instanced",
    "GL_ARB_transpose_matrix",
    "GL_ARB_uniform_buffer_object",
    "GL_ARB_vertex_array_bgra",
    "GL_ARB_vertex_array_object",
    "GL_ARB_vertex_attrib_64bit",
    "GL_ARB_vertex_attrib_binding",
    "GL_ARB_vertex_blend",
    "GL_ARB_vertex_buffer_object",
    "GL_ARB_vertex_program",
    "GL_ARB_vertex_shader",
    "GL_ARB_vertex_type_10f_11f_11f_rev",
    "GL_ARB_vertex_type_2_10_10_10_rev",
    "GL_ARB_viewport_array",
    "GL_ARB_window_pos",
    "GL_ARM_mali_program_binary",
    "GL_ARM_mali_shader_binary",
    "GL_ARM_rgba8",
    "GL_ATI_draw_buffers",
    "GL_ATI_element_array",
    "GL_ATI_envmap_bumpmap",
    "GL_ATI_fragment_shader",
    "GL_ATI_map_object_buffer",
    "GL_ATI_meminfo",
    "GL_ATI_pixel_format_float",
    "GL_ATI_pn_triangles",
    "GL_ATI_separate_stencil",
    "GL_ATI_text_fragment_shader",
    "GL_ATI_texture_env_combine3",
    "GL_ATI_texture_float",
    "GL_ATI_texture_mirror_once",
    "GL_ATI_vertex_array_object",
    "GL_ATI_vertex_attrib_array_object",
    "GL_ATI_vertex_streams",
    "GL_DMP_shader_binary",
    "GL_ES_VERSION_2_0",
    "GL_ES_VERSION_3_0",
    "GL_EXT_422_pixels",
    "GL_EXT_abgr",
    "GL_EXT_bgra",
    "GL_EXT_bindable_uniform",
    "GL_EXT_blend_color",
    "GL_EXT_blend_equation_separate",
    "GL_EXT_blend_func_separate",
    "GL_EXT_blend_logic_op",
    "GL_EXT_blend_minmax",
    "GL_EXT_blend_subtract",
    "GL_EXT_clip_volume_hint",
    "GL_EXT_cmyka",
    "GL_EXT_color_buffer_half_float",
    "GL_EXT_color_subtable",
    "GL_EXT_compiled_vertex_array",
    "GL_EXT_convolution",
    "GL_EXT_coordinate_frame",
    "GL_EXT_copy_texture",
    "GL_EXT_cull_vertex",
    "GL_EXT_debug_label",
    "GL_EXT_debug_marker",
    "GL_EXT_depth_bounds_test",
    "GL_EXT_direct_state_access",
    "GL_EXT_discard_framebuffer",
    "GL_EXT_disjoint_timer_query",
    "GL_EXT_draw_buffers",
    "GL_EXT_draw_buffers2",
    "GL_EXT_draw_instanced",
    "GL_EXT_draw_range_elements",
    "GL_EXT_fog_coord",
    "GL_EXT_framebuffer_blit",
    "GL_EXT_framebuffer_multisample",
    "GL_EXT_framebuffer_multisample_blit_scaled",
    "GL_EXT_framebuffer_object",
    "GL_EXT_framebuffer_sRGB",
    "GL_EXT_geometry_shader4",
    "GL_EXT_gpu_program_parameters",
    "GL_EXT_gpu_shader4",
    "GL_EXT_histogram",
    "GL_EXT_index_array_formats",
    "GL_EXT_index_func",
    "GL_EXT_index_material",
    "GL_EXT_index_texture",
    "GL_EXT_instanced_arrays",
    "GL_EXT_light_texture",
    "GL_EXT_map_buffer_range",
    "GL_EXT_misc_attribute",
    "GL_EXT_multi_draw_arrays",
    "GL_EXT_multisample",
    "GL_EXT_multisampled_render_to_texture",
    "GL_EXT_multiview_draw_buffers",
    "GL_EXT_occlusion_query_boolean",
    "GL_EXT_packed_depth_stencil",
    "GL_EXT_packed_float",
    "GL_EXT_packed_pixels",
    "GL_EXT_paletted_texture",
    "GL_EXT_pixel_buffer_object",
    "GL_EXT_pixel_transform",
    "GL_EXT_pixel_transform_color_table",
    "GL_EXT_point_parameters",
    "GL_EXT_polygon_offset",
    "GL_EXT_provoking_vertex",
    "GL_EXT_pvrtc_sRGB",
    "GL_EXT_read_format_bgra",
    "GL_EXT_rescale_normal",
    "GL_EXT_robustness",
    "GL_EXT_sRGB",
    "GL_EXT_sRGB_write_control",
    "GL_EXT_secondary_color",
    "GL_EXT_separate_shader_objects",
    "GL_EXT_separate_specular_color",
    "GL_EXT_shader_framebuffer_fetch",
    "GL_EXT_shader_image_load_store",
    "GL_EXT_shader_integer_mix",
    "GL_EXT_shader_texture_lod",
    "GL_EXT_shadow_funcs",
    "GL_EXT_shadow_samplers",
    "GL_EXT_shared_texture_palette",
    "GL_EXT_stencil_clear_tag",
    "GL_EXT_stencil_two_side",
    "GL_EXT_stencil_wrap",
    "GL_EXT_subtexture",
    "GL_EXT_texture",
    "GL_EXT_texture3D",
    "GL_EXT_texture_array",
    "GL_EXT_texture_buffer_object",
    "GL_EXT_texture_compression_dxt1",
    "GL_EXT_texture_compression_latc",
    "GL_EXT_texture_compression_rgtc",
    "GL_EXT_texture_compression_s3tc",
    "GL_EXT_texture_cube_map",
    "GL_EXT_texture_env_add",
    "GL_EXT_texture_env_combine",
    "GL_EXT_texture_env_dot3",
    "GL_EXT_texture_filter_anisotropic",
    "GL_EXT_texture_format_BGRA8888",
    "GL_EXT_texture_integer",
    "GL_EXT_texture_lod_bias",
    "GL_EXT_texture_mirror_clamp",
    "GL_EXT_texture_object",
    "GL_EXT_texture_perturb_normal",
    "GL_EXT_texture_rg",
    "GL_EXT_texture_sRGB",
    "GL_EXT_texture_sRGB_decode",
    "GL_EXT_texture_shared_exponent",
    "GL_EXT_texture_snorm",
    "GL_EXT_texture_storage",
    "GL_EXT_texture_swizzle",
    "GL_EXT_texture_type_2_10_10_10_REV",
    "GL_EXT_timer_query",
    "GL_EXT_transform_feedback",
    "GL_EXT_unpack_subimage",
    "GL_EXT_vertex_array",
    "GL_EXT_vertex_array_bgra",
    "GL_EXT_vertex_attrib_64bit",
    "GL_EXT_vertex_shader",
    "GL_EXT_vertex_weighting",
    "GL_EXT_x11_sync_object",
    "GL_FJ_shader_binary_GCCSO",
    "GL_GREMEDY_frame_terminator",
    "GL_GREMEDY_string_marker",
    "GL_HP_convolution_border_modes",
    "GL_HP_image_transform",
    "GL_HP_occlusion_test",
    "GL_HP_texture_lighting",
    "GL_IBM_cull_vertex",
    "GL_IBM_multimode_draw_arrays",
    "GL_IBM_rasterpos_clip",
    "GL_IBM_static_data",
    "GL_IBM_texture_mirrored_repeat",
    "GL_IBM_vertex_array_lists",
    "GL_IMG_multisampled_render_to_texture",
    "GL_IMG_program_binary",
    "GL_IMG_read_format",
    "GL_IMG_shader_binary",
    "GL_IMG_texture_compression_pvrtc",
    "GL_IMG_texture_compression_pvrtc2",
    "GL_INGR_blend_func_separate",
    "GL_INGR_color_clamp",
    "GL_INGR_interlace_read",
    "GL_INTEL_fragment_shader_ordering",
    "GL_INTEL_map_texture",
    "GL_INTEL_parallel_arrays",
    "GL_INTEL_performance_query",
    "GL_KHR_debug",
    "GL_KHR_texture_compression_astc_hdr",
    "GL_KHR_texture_compression_astc_ldr",
    "GL_MESAX_texture_stack",
    "GL_MESA_pack_invert",
    "GL_MESA_resize_buffers",
    "GL_MESA_window_pos",
    "GL_MESA_ycbcr_texture",
    "GL_NVX_conditional_render",
    "GL_NVX_gpu_memory_info",
    "GL_NV_bindless_multi_draw_indirect",
    "GL_NV_bindless_texture",
    "GL_NV_blend_equation_advanced",
    "GL_NV_blend_equation_advanced_coherent",
    "GL_NV_blend_square",
    "GL_NV_compute_program5",
    "GL_NV_conditional_render",
    "GL_NV_copy_buffer",
    "GL_NV_copy_depth_to_color",
    "GL_NV_copy_image",
    "GL_NV_coverage_sample",
    "GL_NV_deep_texture3D",
    "GL_NV_depth_buffer_float",
    "GL_NV_depth_clamp",
    "GL_NV_depth_nonlinear",
    "GL_NV_draw_buffers",
    "GL_NV_draw_instanced",
    "GL_NV_draw_texture",
    "GL_NV_evaluators",
    "GL_NV_explicit_attrib_location",
    "GL_NV_explicit_multisample",
    "GL_NV_fbo_color_attachments",
    "GL_NV_fence",
    "GL_NV_float_buffer",
    "GL_NV_fog_distance",
    "GL_NV_fragment_program",
    "GL_NV_fragment_program2",
    "GL_NV_fragment_program4",
    "GL_NV_fragment_program_option",
    "GL_NV_framebuffer_blit",
    "GL_NV_framebuffer_multisample",
    "GL_NV_framebuffer_multisample_coverage",
    "GL_NV_generate_mipmap_sRGB",
    "GL_NV_geometry_program4",
    "GL_NV_geometry_shader4",
    "GL_NV_gpu_program4",
    "GL_NV_gpu_program5",
    "GL_NV_gpu_program5_mem_extended",
    "GL_NV_gpu_shader5",
    "GL_NV_half_float",
    "GL_NV_instanced_arrays",
    "GL_NV_light_max_exponent",
    "GL_NV_multisample_coverage",
    "GL_NV_multisample_filter_hint",
    "GL_NV_non_square_matrices",
    "GL_NV_occlusion_query",
    "GL_NV_packed_depth_stencil",
    "GL_NV_parameter_buffer_object",
    "GL_NV_parameter_buffer_object2",
    "GL_NV_path_rendering",
    "GL_NV_pixel_data_range",
    "GL_NV_point_sprite",
    "GL_NV_present_video",
    "GL_NV_primitive_restart",
    "GL_NV_read_buffer",
    "GL_NV_read_buffer_front",
    "GL_NV_read_depth",
    "GL_NV_read_depth_stencil",
    "GL_NV_read_stencil",
    "GL_NV_register_combiners",
    "GL_NV_register_combiners2",
    "GL_NV_sRGB_formats",
    "GL_NV_shader_atomic_counters",
    "GL_NV_shader_atomic_float",
    "GL_NV_shader_buffer_load",
    "GL_NV_shader_buffer_store",
    "GL_NV_shader_storage_buffer_object",
    "GL_NV_shadow_samplers_array",
    "GL_NV_shadow_samplers_cube",
    "GL_NV_tessellation_program5",
    "GL_NV_texgen_emboss",
    "GL_NV_texgen_reflection",
    "GL_NV_texture_barrier",
    "GL_NV_texture_border_clamp",
    "GL_NV_texture_compression_s3tc_update",
    "GL_NV_texture_compression_vtc",
    "GL_NV_texture_env_combine4",
    "GL_NV_texture_expand_normal",
    "GL_NV_texture_multisample",
    "GL_NV_texture_npot_2D_mipmap",
    "GL_NV_texture_rectangle",
    "GL_NV_texture_shader",
    "GL_NV_texture_shader2",
    "GL_NV_texture_shader3",
    "GL_NV_transform_feedback",
    "GL_NV_transform_feedback2",
    "GL_NV_vdpau_interop",
    "GL_NV_vertex_array_range",
    "GL_NV_vertex_array_range2",
    "GL_NV_vertex_attrib_integer_64bit",
    "GL_NV_vertex_buffer_unified_memory",
    "GL_NV_vertex_program",
    "GL_NV_vertex_program1_1",
    "GL_NV_vertex_program2",
    "GL_NV_vertex_program2_option",
    "GL_NV_vertex_program3",
    "GL_NV_vertex_program4",
    "GL_NV_video_capture",
    "GL_OES_EGL_image",
    "GL_OES_EGL_image_external",
    "GL_OES_byte_coordinates",
    "GL_OES_compressed_ETC1_RGB8_texture",
    "GL_OES_compressed_paletted_texture",
    "GL_OES_depth24",
    "GL_OES_depth32",
    "GL_OES_depth_texture",
    "GL_OES_element_index_uint",
    "GL_OES_fbo_render_mipmap",
    "GL_OES_fixed_point",
    "GL_OES_fragment_precision_high",
    "GL_OES_get_program_binary",
    "GL_OES_mapbuffer",
    "GL_OES_packed_depth_stencil",
    "GL_OES_query_matrix",
    "GL_OES_read_format",
    "GL_OES_required_internalformat",
    "GL_OES_rgb8_rgba8",
    "GL_OES_single_precision",
    "GL_OES_standard_derivatives",
    "GL_OES_stencil1",
    "GL_OES_stencil4",
    "GL_OES_surfaceless_context",
    "GL_OES_texture_3D",
    "GL_OES_texture_compression_astc",
    "GL_OES_texture_float",
    "GL_OES_texture_float_linear",
    "GL_OES_texture_half_float",
    "GL_OES_texture_half_float_linear",
    "GL_OES_texture_npot",
    "GL_OES_vertex_array_object",
    "GL_OES_vertex_half_float",
    "GL_OES_vertex_type_10_10_10_2",
    "GL_OML_interlace",
    "GL_OML_resample",
    "GL_OML_subsample",
    "GL_PGI_misc_hints",
    "GL_PGI_vertex_hints",
    "GL_QCOM_alpha_test",
    "GL_QCOM_binning_control",
    "GL_QCOM_driver_control",
    "GL_QCOM_extended_get",
    "GL_QCOM_extended_get2",
    "GL_QCOM_perfmon_global_mode",
    "GL_QCOM_tiled_rendering",
    "GL_QCOM_writeonly_rendering",
    "GL_REND_screen_coordinates",
    "GL_S3_s3tc",
    "GL_SGIS_detail_texture",
    "GL_SGIS_fog_function",
    "GL_SGIS_generate_mipmap",
    "GL_SGIS_multisample",
    "GL_SGIS_pixel_texture",
    "GL_SGIS_point_line_texgen",
    "GL_SGIS_point_parameters",
    "GL_SGIS_sharpen_texture",
    "GL_SGIS_texture4D",
    "GL_SGIS_texture_border_clamp",
    "GL_SGIS_texture_color_mask",
    "GL_SGIS_texture_edge_clamp",
    "GL_SGIS_texture_filter4",
    "GL_SGIS_texture_lod",
    "GL_SGIS_texture_select",
    "GL_SGIX_async",
    "GL_SGIX_async_histogram",
    "GL_SGIX_async_pixel",
    "GL_SGIX_blend_alpha_minmax",
    "GL_SGIX_calligraphic_fragment",
    "GL_SGIX_clipmap",
    "GL_SGIX_convolution_accuracy",
    "GL_SGIX_depth_pass_instrument",
    "GL_SGIX_depth_texture",
    "GL_SGIX_flush_raster",
    "GL_SGIX_fog_offset",
    "GL_SGIX_fragment_lighting",
    "GL_SGIX_framezoom",
    "GL_SGIX_igloo_interface",
    "GL_SGIX_instruments",
    "GL_SGIX_interlace",
    "GL_SGIX_ir_instrument1",
    "GL_SGIX_list_priority",
    "GL_SGIX_pixel_texture",
    "GL_SGIX_pixel_tiles",
    "GL_SGIX_polynomial_ffd",
    "GL_SGIX_reference_plane",
    "GL_SGIX_resample",
    "GL_SGIX_scalebias_hint",
    "GL_SGIX_shadow",
    "GL_SGIX_shadow_ambient",
    "GL_SGIX_sprite",
    "GL_SGIX_subsample",
    "GL_SGIX_tag_sample_buffer",
    "GL_SGIX_texture_add_env",
    "GL_SGIX_texture_coordinate_clamp",
    "GL_SGIX_texture_lod_bias",
    "GL_SGIX_texture_multi_buffer",
    "GL_SGIX_texture_scale_bias",
    "GL_SGIX_vertex_preclip",
    "GL_SGIX_ycrcb",
    "GL_SGIX_ycrcb_subsample",
    "GL_SGIX_ycrcba",
    "GL_SGI_color_matrix",
    "GL_SGI_color_table",
    "GL_SGI_texture_color_table",
    "GL_SUNX_constant_data",
    "GL_SUN_convolution_border_modes",
    "GL_SUN_global_alpha",
    "GL_SUN_mesh_array",
    "GL_SUN_slice_accum",
    "GL_SUN_triangle_list",
    "GL_SUN_vertex",
    "GL_VERSION_1_1",
    "GL_VERSION_1_2",
    "GL_VERSION_1_3",
    "GL_VERSION_1_4",
    "GL_VERSION_1_5",
    "GL_VERSION_2_0",
    "GL_VERSION_2_1",
    "GL_VERSION_3_0",
    "GL_VERSION_3_1",
    "GL_VERSION_3_2",
    "GL_VERSION_3_3",
    "GL_VERSION_4_0",
    "GL_VERSION_4_1",
    "GL_VERSION_4_2",
    "GL_VERSION_4_3",
    "GL_VERSION_4_4",
    "GL_VIV_shader_binary",
    "GL_WIN_phong_shading",
    "GL_WIN_specular_fog",
};

static const unsigned ExtensionsDefinedInGLEXT[] =
{
    0,  // SF_GL_NONE    
#if defined(GL_3DFX_multisample)
    GL_3DFX_multisample,
#else
    0,
#endif
#if defined(GL_3DFX_tbuffer)
    GL_3DFX_tbuffer,
#else
    0,
#endif
#if defined(GL_3DFX_texture_compression_FXT1)
    GL_3DFX_texture_compression_FXT1,
#else
    0,
#endif
#if defined(GL_AMD_blend_minmax_factor)
    GL_AMD_blend_minmax_factor,
#else
    0,
#endif
#if defined(GL_AMD_compressed_3DC_texture)
    GL_AMD_compressed_3DC_texture,
#else
    0,
#endif
#if defined(GL_AMD_compressed_ATC_texture)
    GL_AMD_compressed_ATC_texture,
#else
    0,
#endif
#if defined(GL_AMD_conservative_depth)
    GL_AMD_conservative_depth,
#else
    0,
#endif
#if defined(GL_AMD_debug_output)
    GL_AMD_debug_output,
#else
    0,
#endif
#if defined(GL_AMD_depth_clamp_separate)
    GL_AMD_depth_clamp_separate,
#else
    0,
#endif
#if defined(GL_AMD_draw_buffers_blend)
    GL_AMD_draw_buffers_blend,
#else
    0,
#endif
#if defined(GL_AMD_interleaved_elements)
    GL_AMD_interleaved_elements,
#else
    0,
#endif
#if defined(GL_AMD_multi_draw_indirect)
    GL_AMD_multi_draw_indirect,
#else
    0,
#endif
#if defined(GL_AMD_name_gen_delete)
    GL_AMD_name_gen_delete,
#else
    0,
#endif
#if defined(GL_AMD_occlusion_query_event)
    GL_AMD_occlusion_query_event,
#else
    0,
#endif
#if defined(GL_AMD_performance_monitor)
    GL_AMD_performance_monitor,
#else
    0,
#endif
#if defined(GL_AMD_pinned_memory)
    GL_AMD_pinned_memory,
#else
    0,
#endif
#if defined(GL_AMD_program_binary_Z400)
    GL_AMD_program_binary_Z400,
#else
    0,
#endif
#if defined(GL_AMD_query_buffer_object)
    GL_AMD_query_buffer_object,
#else
    0,
#endif
#if defined(GL_AMD_sample_positions)
    GL_AMD_sample_positions,
#else
    0,
#endif
#if defined(GL_AMD_seamless_cubemap_per_texture)
    GL_AMD_seamless_cubemap_per_texture,
#else
    0,
#endif
#if defined(GL_AMD_shader_atomic_counter_ops)
    GL_AMD_shader_atomic_counter_ops,
#else
    0,
#endif
#if defined(GL_AMD_shader_stencil_export)
    GL_AMD_shader_stencil_export,
#else
    0,
#endif
#if defined(GL_AMD_shader_trinary_minmax)
    GL_AMD_shader_trinary_minmax,
#else
    0,
#endif
#if defined(GL_AMD_sparse_texture)
    GL_AMD_sparse_texture,
#else
    0,
#endif
#if defined(GL_AMD_stencil_operation_extended)
    GL_AMD_stencil_operation_extended,
#else
    0,
#endif
#if defined(GL_AMD_texture_texture4)
    GL_AMD_texture_texture4,
#else
    0,
#endif
#if defined(GL_AMD_transform_feedback3_lines_triangles)
    GL_AMD_transform_feedback3_lines_triangles,
#else
    0,
#endif
#if defined(GL_AMD_vertex_shader_layer)
    GL_AMD_vertex_shader_layer,
#else
    0,
#endif
#if defined(GL_AMD_vertex_shader_tessellator)
    GL_AMD_vertex_shader_tessellator,
#else
    0,
#endif
#if defined(GL_AMD_vertex_shader_viewport_index)
    GL_AMD_vertex_shader_viewport_index,
#else
    0,
#endif
#if defined(GL_ANGLE_depth_texture)
    GL_ANGLE_depth_texture,
#else
    0,
#endif
#if defined(GL_ANGLE_framebuffer_blit)
    GL_ANGLE_framebuffer_blit,
#else
    0,
#endif
#if defined(GL_ANGLE_framebuffer_multisample)
    GL_ANGLE_framebuffer_multisample,
#else
    0,
#endif
#if defined(GL_ANGLE_instanced_arrays)
    GL_ANGLE_instanced_arrays,
#else
    0,
#endif
#if defined(GL_ANGLE_pack_reverse_row_order)
    GL_ANGLE_pack_reverse_row_order,
#else
    0,
#endif
#if defined(GL_ANGLE_program_binary)
    GL_ANGLE_program_binary,
#else
    0,
#endif
#if defined(GL_ANGLE_texture_compression_dxt3)
    GL_ANGLE_texture_compression_dxt3,
#else
    0,
#endif
#if defined(GL_ANGLE_texture_compression_dxt5)
    GL_ANGLE_texture_compression_dxt5,
#else
    0,
#endif
#if defined(GL_ANGLE_texture_usage)
    GL_ANGLE_texture_usage,
#else
    0,
#endif
#if defined(GL_ANGLE_translated_shader_source)
    GL_ANGLE_translated_shader_source,
#else
    0,
#endif
#if defined(GL_APPLE_aux_depth_stencil)
    GL_APPLE_aux_depth_stencil,
#else
    0,
#endif
#if defined(GL_APPLE_client_storage)
    GL_APPLE_client_storage,
#else
    0,
#endif
#if defined(GL_APPLE_copy_texture_levels)
    GL_APPLE_copy_texture_levels,
#else
    0,
#endif
#if defined(GL_APPLE_element_array)
    GL_APPLE_element_array,
#else
    0,
#endif
#if defined(GL_APPLE_fence)
    GL_APPLE_fence,
#else
    0,
#endif
#if defined(GL_APPLE_float_pixels)
    GL_APPLE_float_pixels,
#else
    0,
#endif
#if defined(GL_APPLE_flush_buffer_range)
    GL_APPLE_flush_buffer_range,
#else
    0,
#endif
#if defined(GL_APPLE_framebuffer_multisample)
    GL_APPLE_framebuffer_multisample,
#else
    0,
#endif
#if defined(GL_APPLE_object_purgeable)
    GL_APPLE_object_purgeable,
#else
    0,
#endif
#if defined(GL_APPLE_rgb_422)
    GL_APPLE_rgb_422,
#else
    0,
#endif
#if defined(GL_APPLE_row_bytes)
    GL_APPLE_row_bytes,
#else
    0,
#endif
#if defined(GL_APPLE_specular_vector)
    GL_APPLE_specular_vector,
#else
    0,
#endif
#if defined(GL_APPLE_sync)
    GL_APPLE_sync,
#else
    0,
#endif
#if defined(GL_APPLE_texture_format_BGRA8888)
    GL_APPLE_texture_format_BGRA8888,
#else
    0,
#endif
#if defined(GL_APPLE_texture_max_level)
    GL_APPLE_texture_max_level,
#else
    0,
#endif
#if defined(GL_APPLE_texture_range)
    GL_APPLE_texture_range,
#else
    0,
#endif
#if defined(GL_APPLE_transform_hint)
    GL_APPLE_transform_hint,
#else
    0,
#endif
#if defined(GL_APPLE_vertex_array_object)
    GL_APPLE_vertex_array_object,
#else
    0,
#endif
#if defined(GL_APPLE_vertex_array_range)
    GL_APPLE_vertex_array_range,
#else
    0,
#endif
#if defined(GL_APPLE_vertex_program_evaluators)
    GL_APPLE_vertex_program_evaluators,
#else
    0,
#endif
#if defined(GL_APPLE_ycbcr_422)
    GL_APPLE_ycbcr_422,
#else
    0,
#endif
#if defined(GL_ARB_ES2_compatibility)
    GL_ARB_ES2_compatibility,
#else
    0,
#endif
#if defined(GL_ARB_ES3_compatibility)
    GL_ARB_ES3_compatibility,
#else
    0,
#endif
#if defined(GL_ARB_arrays_of_arrays)
    GL_ARB_arrays_of_arrays,
#else
    0,
#endif
#if defined(GL_ARB_base_instance)
    GL_ARB_base_instance,
#else
    0,
#endif
#if defined(GL_ARB_bindless_texture)
    GL_ARB_bindless_texture,
#else
    0,
#endif
#if defined(GL_ARB_blend_func_extended)
    GL_ARB_blend_func_extended,
#else
    0,
#endif
#if defined(GL_ARB_buffer_storage)
    GL_ARB_buffer_storage,
#else
    0,
#endif
#if defined(GL_ARB_cl_event)
    GL_ARB_cl_event,
#else
    0,
#endif
#if defined(GL_ARB_clear_buffer_object)
    GL_ARB_clear_buffer_object,
#else
    0,
#endif
#if defined(GL_ARB_clear_texture)
    GL_ARB_clear_texture,
#else
    0,
#endif
#if defined(GL_ARB_color_buffer_float)
    GL_ARB_color_buffer_float,
#else
    0,
#endif
#if defined(GL_ARB_compatibility)
    GL_ARB_compatibility,
#else
    0,
#endif
#if defined(GL_ARB_compressed_texture_pixel_storage)
    GL_ARB_compressed_texture_pixel_storage,
#else
    0,
#endif
#if defined(GL_ARB_compute_shader)
    GL_ARB_compute_shader,
#else
    0,
#endif
#if defined(GL_ARB_compute_variable_group_size)
    GL_ARB_compute_variable_group_size,
#else
    0,
#endif
#if defined(GL_ARB_conservative_depth)
    GL_ARB_conservative_depth,
#else
    0,
#endif
#if defined(GL_ARB_copy_buffer)
    GL_ARB_copy_buffer,
#else
    0,
#endif
#if defined(GL_ARB_copy_image)
    GL_ARB_copy_image,
#else
    0,
#endif
#if defined(GL_ARB_debug_output)
    GL_ARB_debug_output,
#else
    0,
#endif
#if defined(GL_ARB_depth_buffer_float)
    GL_ARB_depth_buffer_float,
#else
    0,
#endif
#if defined(GL_ARB_depth_clamp)
    GL_ARB_depth_clamp,
#else
    0,
#endif
#if defined(GL_ARB_depth_texture)
    GL_ARB_depth_texture,
#else
    0,
#endif
#if defined(GL_ARB_draw_buffers)
    GL_ARB_draw_buffers,
#else
    0,
#endif
#if defined(GL_ARB_draw_buffers_blend)
    GL_ARB_draw_buffers_blend,
#else
    0,
#endif
#if defined(GL_ARB_draw_elements_base_vertex)
    GL_ARB_draw_elements_base_vertex,
#else
    0,
#endif
#if defined(GL_ARB_draw_indirect)
    GL_ARB_draw_indirect,
#else
    0,
#endif
#if defined(GL_ARB_draw_instanced)
    GL_ARB_draw_instanced,
#else
    0,
#endif
#if defined(GL_ARB_enhanced_layouts)
    GL_ARB_enhanced_layouts,
#else
    0,
#endif
#if defined(GL_ARB_explicit_attrib_location)
    GL_ARB_explicit_attrib_location,
#else
    0,
#endif
#if defined(GL_ARB_explicit_uniform_location)
    GL_ARB_explicit_uniform_location,
#else
    0,
#endif
#if defined(GL_ARB_fragment_coord_conventions)
    GL_ARB_fragment_coord_conventions,
#else
    0,
#endif
#if defined(GL_ARB_fragment_layer_viewport)
    GL_ARB_fragment_layer_viewport,
#else
    0,
#endif
#if defined(GL_ARB_fragment_program)
    GL_ARB_fragment_program,
#else
    0,
#endif
#if defined(GL_ARB_fragment_program_shadow)
    GL_ARB_fragment_program_shadow,
#else
    0,
#endif
#if defined(GL_ARB_fragment_shader)
    GL_ARB_fragment_shader,
#else
    0,
#endif
#if defined(GL_ARB_framebuffer_no_attachments)
    GL_ARB_framebuffer_no_attachments,
#else
    0,
#endif
#if defined(GL_ARB_framebuffer_object)
    GL_ARB_framebuffer_object,
#else
    0,
#endif
#if defined(GL_ARB_framebuffer_sRGB)
    GL_ARB_framebuffer_sRGB,
#else
    0,
#endif
#if defined(GL_ARB_geometry_shader4)
    GL_ARB_geometry_shader4,
#else
    0,
#endif
#if defined(GL_ARB_get_program_binary)
    GL_ARB_get_program_binary,
#else
    0,
#endif
#if defined(GL_ARB_gpu_shader5)
    GL_ARB_gpu_shader5,
#else
    0,
#endif
#if defined(GL_ARB_gpu_shader_fp64)
    GL_ARB_gpu_shader_fp64,
#else
    0,
#endif
#if defined(GL_ARB_half_float_pixel)
    GL_ARB_half_float_pixel,
#else
    0,
#endif
#if defined(GL_ARB_half_float_vertex)
    GL_ARB_half_float_vertex,
#else
    0,
#endif
#if defined(GL_ARB_imaging)
    GL_ARB_imaging,
#else
    0,
#endif
#if defined(GL_ARB_indirect_parameters)
    GL_ARB_indirect_parameters,
#else
    0,
#endif
#if defined(GL_ARB_instanced_arrays)
    GL_ARB_instanced_arrays,
#else
    0,
#endif
#if defined(GL_ARB_internalformat_query)
    GL_ARB_internalformat_query,
#else
    0,
#endif
#if defined(GL_ARB_internalformat_query2)
    GL_ARB_internalformat_query2,
#else
    0,
#endif
#if defined(GL_ARB_invalidate_subdata)
    GL_ARB_invalidate_subdata,
#else
    0,
#endif
#if defined(GL_ARB_map_buffer_alignment)
    GL_ARB_map_buffer_alignment,
#else
    0,
#endif
#if defined(GL_ARB_map_buffer_range)
    GL_ARB_map_buffer_range,
#else
    0,
#endif
#if defined(GL_ARB_matrix_palette)
    GL_ARB_matrix_palette,
#else
    0,
#endif
#if defined(GL_ARB_multi_bind)
    GL_ARB_multi_bind,
#else
    0,
#endif
#if defined(GL_ARB_multi_draw_indirect)
    GL_ARB_multi_draw_indirect,
#else
    0,
#endif
#if defined(GL_ARB_multisample)
    GL_ARB_multisample,
#else
    0,
#endif
#if defined(GL_ARB_multitexture)
    GL_ARB_multitexture,
#else
    0,
#endif
#if defined(GL_ARB_occlusion_query)
    GL_ARB_occlusion_query,
#else
    0,
#endif
#if defined(GL_ARB_occlusion_query2)
    GL_ARB_occlusion_query2,
#else
    0,
#endif
#if defined(GL_ARB_pixel_buffer_object)
    GL_ARB_pixel_buffer_object,
#else
    0,
#endif
#if defined(GL_ARB_point_parameters)
    GL_ARB_point_parameters,
#else
    0,
#endif
#if defined(GL_ARB_point_sprite)
    GL_ARB_point_sprite,
#else
    0,
#endif
#if defined(GL_ARB_program_interface_query)
    GL_ARB_program_interface_query,
#else
    0,
#endif
#if defined(GL_ARB_provoking_vertex)
    GL_ARB_provoking_vertex,
#else
    0,
#endif
#if defined(GL_ARB_query_buffer_object)
    GL_ARB_query_buffer_object,
#else
    0,
#endif
#if defined(GL_ARB_robust_buffer_access_behavior)
    GL_ARB_robust_buffer_access_behavior,
#else
    0,
#endif
#if defined(GL_ARB_robustness)
    GL_ARB_robustness,
#else
    0,
#endif
#if defined(GL_ARB_robustness_isolation)
    GL_ARB_robustness_isolation,
#else
    0,
#endif
#if defined(GL_ARB_sample_shading)
    GL_ARB_sample_shading,
#else
    0,
#endif
#if defined(GL_ARB_sampler_objects)
    GL_ARB_sampler_objects,
#else
    0,
#endif
#if defined(GL_ARB_seamless_cube_map)
    GL_ARB_seamless_cube_map,
#else
    0,
#endif
#if defined(GL_ARB_seamless_cubemap_per_texture)
    GL_ARB_seamless_cubemap_per_texture,
#else
    0,
#endif
#if defined(GL_ARB_separate_shader_objects)
    GL_ARB_separate_shader_objects,
#else
    0,
#endif
#if defined(GL_ARB_shader_atomic_counters)
    GL_ARB_shader_atomic_counters,
#else
    0,
#endif
#if defined(GL_ARB_shader_bit_encoding)
    GL_ARB_shader_bit_encoding,
#else
    0,
#endif
#if defined(GL_ARB_shader_draw_parameters)
    GL_ARB_shader_draw_parameters,
#else
    0,
#endif
#if defined(GL_ARB_shader_group_vote)
    GL_ARB_shader_group_vote,
#else
    0,
#endif
#if defined(GL_ARB_shader_image_load_store)
    GL_ARB_shader_image_load_store,
#else
    0,
#endif
#if defined(GL_ARB_shader_image_size)
    GL_ARB_shader_image_size,
#else
    0,
#endif
#if defined(GL_ARB_shader_objects)
    GL_ARB_shader_objects,
#else
    0,
#endif
#if defined(GL_ARB_shader_precision)
    GL_ARB_shader_precision,
#else
    0,
#endif
#if defined(GL_ARB_shader_stencil_export)
    GL_ARB_shader_stencil_export,
#else
    0,
#endif
#if defined(GL_ARB_shader_storage_buffer_object)
    GL_ARB_shader_storage_buffer_object,
#else
    0,
#endif
#if defined(GL_ARB_shader_subroutine)
    GL_ARB_shader_subroutine,
#else
    0,
#endif
#if defined(GL_ARB_shader_texture_lod)
    GL_ARB_shader_texture_lod,
#else
    0,
#endif
#if defined(GL_ARB_shading_language_100)
    GL_ARB_shading_language_100,
#else
    0,
#endif
#if defined(GL_ARB_shading_language_420pack)
    GL_ARB_shading_language_420pack,
#else
    0,
#endif
#if defined(GL_ARB_shading_language_include)
    GL_ARB_shading_language_include,
#else
    0,
#endif
#if defined(GL_ARB_shading_language_packing)
    GL_ARB_shading_language_packing,
#else
    0,
#endif
#if defined(GL_ARB_shadow)
    GL_ARB_shadow,
#else
    0,
#endif
#if defined(GL_ARB_shadow_ambient)
    GL_ARB_shadow_ambient,
#else
    0,
#endif
#if defined(GL_ARB_sparse_texture)
    GL_ARB_sparse_texture,
#else
    0,
#endif
#if defined(GL_ARB_stencil_texturing)
    GL_ARB_stencil_texturing,
#else
    0,
#endif
#if defined(GL_ARB_sync)
    GL_ARB_sync,
#else
    0,
#endif
#if defined(GL_ARB_tessellation_shader)
    GL_ARB_tessellation_shader,
#else
    0,
#endif
#if defined(GL_ARB_texture_border_clamp)
    GL_ARB_texture_border_clamp,
#else
    0,
#endif
#if defined(GL_ARB_texture_buffer_object)
    GL_ARB_texture_buffer_object,
#else
    0,
#endif
#if defined(GL_ARB_texture_buffer_object_rgb32)
    GL_ARB_texture_buffer_object_rgb32,
#else
    0,
#endif
#if defined(GL_ARB_texture_buffer_range)
    GL_ARB_texture_buffer_range,
#else
    0,
#endif
#if defined(GL_ARB_texture_compression)
    GL_ARB_texture_compression,
#else
    0,
#endif
#if defined(GL_ARB_texture_compression_bptc)
    GL_ARB_texture_compression_bptc,
#else
    0,
#endif
#if defined(GL_ARB_texture_compression_rgtc)
    GL_ARB_texture_compression_rgtc,
#else
    0,
#endif
#if defined(GL_ARB_texture_cube_map)
    GL_ARB_texture_cube_map,
#else
    0,
#endif
#if defined(GL_ARB_texture_cube_map_array)
    GL_ARB_texture_cube_map_array,
#else
    0,
#endif
#if defined(GL_ARB_texture_env_add)
    GL_ARB_texture_env_add,
#else
    0,
#endif
#if defined(GL_ARB_texture_env_combine)
    GL_ARB_texture_env_combine,
#else
    0,
#endif
#if defined(GL_ARB_texture_env_crossbar)
    GL_ARB_texture_env_crossbar,
#else
    0,
#endif
#if defined(GL_ARB_texture_env_dot3)
    GL_ARB_texture_env_dot3,
#else
    0,
#endif
#if defined(GL_ARB_texture_float)
    GL_ARB_texture_float,
#else
    0,
#endif
#if defined(GL_ARB_texture_gather)
    GL_ARB_texture_gather,
#else
    0,
#endif
#if defined(GL_ARB_texture_mirror_clamp_to_edge)
    GL_ARB_texture_mirror_clamp_to_edge,
#else
    0,
#endif
#if defined(GL_ARB_texture_mirrored_repeat)
    GL_ARB_texture_mirrored_repeat,
#else
    0,
#endif
#if defined(GL_ARB_texture_multisample)
    GL_ARB_texture_multisample,
#else
    0,
#endif
#if defined(GL_ARB_texture_non_power_of_two)
    GL_ARB_texture_non_power_of_two,
#else
    0,
#endif
#if defined(GL_ARB_texture_query_levels)
    GL_ARB_texture_query_levels,
#else
    0,
#endif
#if defined(GL_ARB_texture_query_lod)
    GL_ARB_texture_query_lod,
#else
    0,
#endif
#if defined(GL_ARB_texture_rectangle)
    GL_ARB_texture_rectangle,
#else
    0,
#endif
#if defined(GL_ARB_texture_rg)
    GL_ARB_texture_rg,
#else
    0,
#endif
#if defined(GL_ARB_texture_rgb10_a2ui)
    GL_ARB_texture_rgb10_a2ui,
#else
    0,
#endif
#if defined(GL_ARB_texture_stencil8)
    GL_ARB_texture_stencil8,
#else
    0,
#endif
#if defined(GL_ARB_texture_storage)
    GL_ARB_texture_storage,
#else
    0,
#endif
#if defined(GL_ARB_texture_storage_multisample)
    GL_ARB_texture_storage_multisample,
#else
    0,
#endif
#if defined(GL_ARB_texture_swizzle)
    GL_ARB_texture_swizzle,
#else
    0,
#endif
#if defined(GL_ARB_texture_view)
    GL_ARB_texture_view,
#else
    0,
#endif
#if defined(GL_ARB_timer_query)
    GL_ARB_timer_query,
#else
    0,
#endif
#if defined(GL_ARB_transform_feedback2)
    GL_ARB_transform_feedback2,
#else
    0,
#endif
#if defined(GL_ARB_transform_feedback3)
    GL_ARB_transform_feedback3,
#else
    0,
#endif
#if defined(GL_ARB_transform_feedback_instanced)
    GL_ARB_transform_feedback_instanced,
#else
    0,
#endif
#if defined(GL_ARB_transpose_matrix)
    GL_ARB_transpose_matrix,
#else
    0,
#endif
#if defined(GL_ARB_uniform_buffer_object)
    GL_ARB_uniform_buffer_object,
#else
    0,
#endif
#if defined(GL_ARB_vertex_array_bgra)
    GL_ARB_vertex_array_bgra,
#else
    0,
#endif
#if defined(GL_ARB_vertex_array_object)
    GL_ARB_vertex_array_object,
#else
    0,
#endif
#if defined(GL_ARB_vertex_attrib_64bit)
    GL_ARB_vertex_attrib_64bit,
#else
    0,
#endif
#if defined(GL_ARB_vertex_attrib_binding)
    GL_ARB_vertex_attrib_binding,
#else
    0,
#endif
#if defined(GL_ARB_vertex_blend)
    GL_ARB_vertex_blend,
#else
    0,
#endif
#if defined(GL_ARB_vertex_buffer_object)
    GL_ARB_vertex_buffer_object,
#else
    0,
#endif
#if defined(GL_ARB_vertex_program)
    GL_ARB_vertex_program,
#else
    0,
#endif
#if defined(GL_ARB_vertex_shader)
    GL_ARB_vertex_shader,
#else
    0,
#endif
#if defined(GL_ARB_vertex_type_10f_11f_11f_rev)
    GL_ARB_vertex_type_10f_11f_11f_rev,
#else
    0,
#endif
#if defined(GL_ARB_vertex_type_2_10_10_10_rev)
    GL_ARB_vertex_type_2_10_10_10_rev,
#else
    0,
#endif
#if defined(GL_ARB_viewport_array)
    GL_ARB_viewport_array,
#else
    0,
#endif
#if defined(GL_ARB_window_pos)
    GL_ARB_window_pos,
#else
    0,
#endif
#if defined(GL_ARM_mali_program_binary)
    GL_ARM_mali_program_binary,
#else
    0,
#endif
#if defined(GL_ARM_mali_shader_binary)
    GL_ARM_mali_shader_binary,
#else
    0,
#endif
#if defined(GL_ARM_rgba8)
    GL_ARM_rgba8,
#else
    0,
#endif
#if defined(GL_ATI_draw_buffers)
    GL_ATI_draw_buffers,
#else
    0,
#endif
#if defined(GL_ATI_element_array)
    GL_ATI_element_array,
#else
    0,
#endif
#if defined(GL_ATI_envmap_bumpmap)
    GL_ATI_envmap_bumpmap,
#else
    0,
#endif
#if defined(GL_ATI_fragment_shader)
    GL_ATI_fragment_shader,
#else
    0,
#endif
#if defined(GL_ATI_map_object_buffer)
    GL_ATI_map_object_buffer,
#else
    0,
#endif
#if defined(GL_ATI_meminfo)
    GL_ATI_meminfo,
#else
    0,
#endif
#if defined(GL_ATI_pixel_format_float)
    GL_ATI_pixel_format_float,
#else
    0,
#endif
#if defined(GL_ATI_pn_triangles)
    GL_ATI_pn_triangles,
#else
    0,
#endif
#if defined(GL_ATI_separate_stencil)
    GL_ATI_separate_stencil,
#else
    0,
#endif
#if defined(GL_ATI_text_fragment_shader)
    GL_ATI_text_fragment_shader,
#else
    0,
#endif
#if defined(GL_ATI_texture_env_combine3)
    GL_ATI_texture_env_combine3,
#else
    0,
#endif
#if defined(GL_ATI_texture_float)
    GL_ATI_texture_float,
#else
    0,
#endif
#if defined(GL_ATI_texture_mirror_once)
    GL_ATI_texture_mirror_once,
#else
    0,
#endif
#if defined(GL_ATI_vertex_array_object)
    GL_ATI_vertex_array_object,
#else
    0,
#endif
#if defined(GL_ATI_vertex_attrib_array_object)
    GL_ATI_vertex_attrib_array_object,
#else
    0,
#endif
#if defined(GL_ATI_vertex_streams)
    GL_ATI_vertex_streams,
#else
    0,
#endif
#if defined(GL_DMP_shader_binary)
    GL_DMP_shader_binary,
#else
    0,
#endif
#if defined(GL_ES_VERSION_2_0)
    GL_ES_VERSION_2_0,
#else
    0,
#endif
#if defined(GL_ES_VERSION_3_0)
    GL_ES_VERSION_3_0,
#else
    0,
#endif
#if defined(GL_EXT_422_pixels)
    GL_EXT_422_pixels,
#else
    0,
#endif
#if defined(GL_EXT_abgr)
    GL_EXT_abgr,
#else
    0,
#endif
#if defined(GL_EXT_bgra)
    GL_EXT_bgra,
#else
    0,
#endif
#if defined(GL_EXT_bindable_uniform)
    GL_EXT_bindable_uniform,
#else
    0,
#endif
#if defined(GL_EXT_blend_color)
    GL_EXT_blend_color,
#else
    0,
#endif
#if defined(GL_EXT_blend_equation_separate)
    GL_EXT_blend_equation_separate,
#else
    0,
#endif
#if defined(GL_EXT_blend_func_separate)
    GL_EXT_blend_func_separate,
#else
    0,
#endif
#if defined(GL_EXT_blend_logic_op)
    GL_EXT_blend_logic_op,
#else
    0,
#endif
#if defined(GL_EXT_blend_minmax)
    GL_EXT_blend_minmax,
#else
    0,
#endif
#if defined(GL_EXT_blend_subtract)
    GL_EXT_blend_subtract,
#else
    0,
#endif
#if defined(GL_EXT_clip_volume_hint)
    GL_EXT_clip_volume_hint,
#else
    0,
#endif
#if defined(GL_EXT_cmyka)
    GL_EXT_cmyka,
#else
    0,
#endif
#if defined(GL_EXT_color_buffer_half_float)
    GL_EXT_color_buffer_half_float,
#else
    0,
#endif
#if defined(GL_EXT_color_subtable)
    GL_EXT_color_subtable,
#else
    0,
#endif
#if defined(GL_EXT_compiled_vertex_array)
    GL_EXT_compiled_vertex_array,
#else
    0,
#endif
#if defined(GL_EXT_convolution)
    GL_EXT_convolution,
#else
    0,
#endif
#if defined(GL_EXT_coordinate_frame)
    GL_EXT_coordinate_frame,
#else
    0,
#endif
#if defined(GL_EXT_copy_texture)
    GL_EXT_copy_texture,
#else
    0,
#endif
#if defined(GL_EXT_cull_vertex)
    GL_EXT_cull_vertex,
#else
    0,
#endif
#if defined(GL_EXT_debug_label)
    GL_EXT_debug_label,
#else
    0,
#endif
#if defined(GL_EXT_debug_marker)
    GL_EXT_debug_marker,
#else
    0,
#endif
#if defined(GL_EXT_depth_bounds_test)
    GL_EXT_depth_bounds_test,
#else
    0,
#endif
#if defined(GL_EXT_direct_state_access)
    GL_EXT_direct_state_access,
#else
    0,
#endif
#if defined(GL_EXT_discard_framebuffer)
    GL_EXT_discard_framebuffer,
#else
    0,
#endif
#if defined(GL_EXT_disjoint_timer_query)
    GL_EXT_disjoint_timer_query,
#else
    0,
#endif
#if defined(GL_EXT_draw_buffers)
    GL_EXT_draw_buffers,
#else
    0,
#endif
#if defined(GL_EXT_draw_buffers2)
    GL_EXT_draw_buffers2,
#else
    0,
#endif
#if defined(GL_EXT_draw_instanced)
    GL_EXT_draw_instanced,
#else
    0,
#endif
#if defined(GL_EXT_draw_range_elements)
    GL_EXT_draw_range_elements,
#else
    0,
#endif
#if defined(GL_EXT_fog_coord)
    GL_EXT_fog_coord,
#else
    0,
#endif
#if defined(GL_EXT_framebuffer_blit)
    GL_EXT_framebuffer_blit,
#else
    0,
#endif
#if defined(GL_EXT_framebuffer_multisample)
    GL_EXT_framebuffer_multisample,
#else
    0,
#endif
#if defined(GL_EXT_framebuffer_multisample_blit_scaled)
    GL_EXT_framebuffer_multisample_blit_scaled,
#else
    0,
#endif
#if defined(GL_EXT_framebuffer_object)
    GL_EXT_framebuffer_object,
#else
    0,
#endif
#if defined(GL_EXT_framebuffer_sRGB)
    GL_EXT_framebuffer_sRGB,
#else
    0,
#endif
#if defined(GL_EXT_geometry_shader4)
    GL_EXT_geometry_shader4,
#else
    0,
#endif
#if defined(GL_EXT_gpu_program_parameters)
    GL_EXT_gpu_program_parameters,
#else
    0,
#endif
#if defined(GL_EXT_gpu_shader4)
    GL_EXT_gpu_shader4,
#else
    0,
#endif
#if defined(GL_EXT_histogram)
    GL_EXT_histogram,
#else
    0,
#endif
#if defined(GL_EXT_index_array_formats)
    GL_EXT_index_array_formats,
#else
    0,
#endif
#if defined(GL_EXT_index_func)
    GL_EXT_index_func,
#else
    0,
#endif
#if defined(GL_EXT_index_material)
    GL_EXT_index_material,
#else
    0,
#endif
#if defined(GL_EXT_index_texture)
    GL_EXT_index_texture,
#else
    0,
#endif
#if defined(GL_EXT_instanced_arrays)
    GL_EXT_instanced_arrays,
#else
    0,
#endif
#if defined(GL_EXT_light_texture)
    GL_EXT_light_texture,
#else
    0,
#endif
#if defined(GL_EXT_map_buffer_range)
    GL_EXT_map_buffer_range,
#else
    0,
#endif
#if defined(GL_EXT_misc_attribute)
    GL_EXT_misc_attribute,
#else
    0,
#endif
#if defined(GL_EXT_multi_draw_arrays)
    GL_EXT_multi_draw_arrays,
#else
    0,
#endif
#if defined(GL_EXT_multisample)
    GL_EXT_multisample,
#else
    0,
#endif
#if defined(GL_EXT_multisampled_render_to_texture)
    GL_EXT_multisampled_render_to_texture,
#else
    0,
#endif
#if defined(GL_EXT_multiview_draw_buffers)
    GL_EXT_multiview_draw_buffers,
#else
    0,
#endif
#if defined(GL_EXT_occlusion_query_boolean)
    GL_EXT_occlusion_query_boolean,
#else
    0,
#endif
#if defined(GL_EXT_packed_depth_stencil)
    GL_EXT_packed_depth_stencil,
#else
    0,
#endif
#if defined(GL_EXT_packed_float)
    GL_EXT_packed_float,
#else
    0,
#endif
#if defined(GL_EXT_packed_pixels)
    GL_EXT_packed_pixels,
#else
    0,
#endif
#if defined(GL_EXT_paletted_texture)
    GL_EXT_paletted_texture,
#else
    0,
#endif
#if defined(GL_EXT_pixel_buffer_object)
    GL_EXT_pixel_buffer_object,
#else
    0,
#endif
#if defined(GL_EXT_pixel_transform)
    GL_EXT_pixel_transform,
#else
    0,
#endif
#if defined(GL_EXT_pixel_transform_color_table)
    GL_EXT_pixel_transform_color_table,
#else
    0,
#endif
#if defined(GL_EXT_point_parameters)
    GL_EXT_point_parameters,
#else
    0,
#endif
#if defined(GL_EXT_polygon_offset)
    GL_EXT_polygon_offset,
#else
    0,
#endif
#if defined(GL_EXT_provoking_vertex)
    GL_EXT_provoking_vertex,
#else
    0,
#endif
#if defined(GL_EXT_pvrtc_sRGB)
    GL_EXT_pvrtc_sRGB,
#else
    0,
#endif
#if defined(GL_EXT_read_format_bgra)
    GL_EXT_read_format_bgra,
#else
    0,
#endif
#if defined(GL_EXT_rescale_normal)
    GL_EXT_rescale_normal,
#else
    0,
#endif
#if defined(GL_EXT_robustness)
    GL_EXT_robustness,
#else
    0,
#endif
#if defined(GL_EXT_sRGB)
    GL_EXT_sRGB,
#else
    0,
#endif
#if defined(GL_EXT_sRGB_write_control)
    GL_EXT_sRGB_write_control,
#else
    0,
#endif
#if defined(GL_EXT_secondary_color)
    GL_EXT_secondary_color,
#else
    0,
#endif
#if defined(GL_EXT_separate_shader_objects)
    GL_EXT_separate_shader_objects,
#else
    0,
#endif
#if defined(GL_EXT_separate_specular_color)
    GL_EXT_separate_specular_color,
#else
    0,
#endif
#if defined(GL_EXT_shader_framebuffer_fetch)
    GL_EXT_shader_framebuffer_fetch,
#else
    0,
#endif
#if defined(GL_EXT_shader_image_load_store)
    GL_EXT_shader_image_load_store,
#else
    0,
#endif
#if defined(GL_EXT_shader_integer_mix)
    GL_EXT_shader_integer_mix,
#else
    0,
#endif
#if defined(GL_EXT_shader_texture_lod)
    GL_EXT_shader_texture_lod,
#else
    0,
#endif
#if defined(GL_EXT_shadow_funcs)
    GL_EXT_shadow_funcs,
#else
    0,
#endif
#if defined(GL_EXT_shadow_samplers)
    GL_EXT_shadow_samplers,
#else
    0,
#endif
#if defined(GL_EXT_shared_texture_palette)
    GL_EXT_shared_texture_palette,
#else
    0,
#endif
#if defined(GL_EXT_stencil_clear_tag)
    GL_EXT_stencil_clear_tag,
#else
    0,
#endif
#if defined(GL_EXT_stencil_two_side)
    GL_EXT_stencil_two_side,
#else
    0,
#endif
#if defined(GL_EXT_stencil_wrap)
    GL_EXT_stencil_wrap,
#else
    0,
#endif
#if defined(GL_EXT_subtexture)
    GL_EXT_subtexture,
#else
    0,
#endif
#if defined(GL_EXT_texture)
    GL_EXT_texture,
#else
    0,
#endif
#if defined(GL_EXT_texture3D)
    GL_EXT_texture3D,
#else
    0,
#endif
#if defined(GL_EXT_texture_array)
    GL_EXT_texture_array,
#else
    0,
#endif
#if defined(GL_EXT_texture_buffer_object)
    GL_EXT_texture_buffer_object,
#else
    0,
#endif
#if defined(GL_EXT_texture_compression_dxt1)
    GL_EXT_texture_compression_dxt1,
#else
    0,
#endif
#if defined(GL_EXT_texture_compression_latc)
    GL_EXT_texture_compression_latc,
#else
    0,
#endif
#if defined(GL_EXT_texture_compression_rgtc)
    GL_EXT_texture_compression_rgtc,
#else
    0,
#endif
#if defined(GL_EXT_texture_compression_s3tc)
    GL_EXT_texture_compression_s3tc,
#else
    0,
#endif
#if defined(GL_EXT_texture_cube_map)
    GL_EXT_texture_cube_map,
#else
    0,
#endif
#if defined(GL_EXT_texture_env_add)
    GL_EXT_texture_env_add,
#else
    0,
#endif
#if defined(GL_EXT_texture_env_combine)
    GL_EXT_texture_env_combine,
#else
    0,
#endif
#if defined(GL_EXT_texture_env_dot3)
    GL_EXT_texture_env_dot3,
#else
    0,
#endif
#if defined(GL_EXT_texture_filter_anisotropic)
    GL_EXT_texture_filter_anisotropic,
#else
    0,
#endif
#if defined(GL_EXT_texture_format_BGRA8888)
    GL_EXT_texture_format_BGRA8888,
#else
    0,
#endif
#if defined(GL_EXT_texture_integer)
    GL_EXT_texture_integer,
#else
    0,
#endif
#if defined(GL_EXT_texture_lod_bias)
    GL_EXT_texture_lod_bias,
#else
    0,
#endif
#if defined(GL_EXT_texture_mirror_clamp)
    GL_EXT_texture_mirror_clamp,
#else
    0,
#endif
#if defined(GL_EXT_texture_object)
    GL_EXT_texture_object,
#else
    0,
#endif
#if defined(GL_EXT_texture_perturb_normal)
    GL_EXT_texture_perturb_normal,
#else
    0,
#endif
#if defined(GL_EXT_texture_rg)
    GL_EXT_texture_rg,
#else
    0,
#endif
#if defined(GL_EXT_texture_sRGB)
    GL_EXT_texture_sRGB,
#else
    0,
#endif
#if defined(GL_EXT_texture_sRGB_decode)
    GL_EXT_texture_sRGB_decode,
#else
    0,
#endif
#if defined(GL_EXT_texture_shared_exponent)
    GL_EXT_texture_shared_exponent,
#else
    0,
#endif
#if defined(GL_EXT_texture_snorm)
    GL_EXT_texture_snorm,
#else
    0,
#endif
#if defined(GL_EXT_texture_storage)
    GL_EXT_texture_storage,
#else
    0,
#endif
#if defined(GL_EXT_texture_swizzle)
    GL_EXT_texture_swizzle,
#else
    0,
#endif
#if defined(GL_EXT_texture_type_2_10_10_10_REV)
    GL_EXT_texture_type_2_10_10_10_REV,
#else
    0,
#endif
#if defined(GL_EXT_timer_query)
    GL_EXT_timer_query,
#else
    0,
#endif
#if defined(GL_EXT_transform_feedback)
    GL_EXT_transform_feedback,
#else
    0,
#endif
#if defined(GL_EXT_unpack_subimage)
    GL_EXT_unpack_subimage,
#else
    0,
#endif
#if defined(GL_EXT_vertex_array)
    GL_EXT_vertex_array,
#else
    0,
#endif
#if defined(GL_EXT_vertex_array_bgra)
    GL_EXT_vertex_array_bgra,
#else
    0,
#endif
#if defined(GL_EXT_vertex_attrib_64bit)
    GL_EXT_vertex_attrib_64bit,
#else
    0,
#endif
#if defined(GL_EXT_vertex_shader)
    GL_EXT_vertex_shader,
#else
    0,
#endif
#if defined(GL_EXT_vertex_weighting)
    GL_EXT_vertex_weighting,
#else
    0,
#endif
#if defined(GL_EXT_x11_sync_object)
    GL_EXT_x11_sync_object,
#else
    0,
#endif
#if defined(GL_FJ_shader_binary_GCCSO)
    GL_FJ_shader_binary_GCCSO,
#else
    0,
#endif
#if defined(GL_GREMEDY_frame_terminator)
    GL_GREMEDY_frame_terminator,
#else
    0,
#endif
#if defined(GL_GREMEDY_string_marker)
    GL_GREMEDY_string_marker,
#else
    0,
#endif
#if defined(GL_HP_convolution_border_modes)
    GL_HP_convolution_border_modes,
#else
    0,
#endif
#if defined(GL_HP_image_transform)
    GL_HP_image_transform,
#else
    0,
#endif
#if defined(GL_HP_occlusion_test)
    GL_HP_occlusion_test,
#else
    0,
#endif
#if defined(GL_HP_texture_lighting)
    GL_HP_texture_lighting,
#else
    0,
#endif
#if defined(GL_IBM_cull_vertex)
    GL_IBM_cull_vertex,
#else
    0,
#endif
#if defined(GL_IBM_multimode_draw_arrays)
    GL_IBM_multimode_draw_arrays,
#else
    0,
#endif
#if defined(GL_IBM_rasterpos_clip)
    GL_IBM_rasterpos_clip,
#else
    0,
#endif
#if defined(GL_IBM_static_data)
    GL_IBM_static_data,
#else
    0,
#endif
#if defined(GL_IBM_texture_mirrored_repeat)
    GL_IBM_texture_mirrored_repeat,
#else
    0,
#endif
#if defined(GL_IBM_vertex_array_lists)
    GL_IBM_vertex_array_lists,
#else
    0,
#endif
#if defined(GL_IMG_multisampled_render_to_texture)
    GL_IMG_multisampled_render_to_texture,
#else
    0,
#endif
#if defined(GL_IMG_program_binary)
    GL_IMG_program_binary,
#else
    0,
#endif
#if defined(GL_IMG_read_format)
    GL_IMG_read_format,
#else
    0,
#endif
#if defined(GL_IMG_shader_binary)
    GL_IMG_shader_binary,
#else
    0,
#endif
#if defined(GL_IMG_texture_compression_pvrtc)
    GL_IMG_texture_compression_pvrtc,
#else
    0,
#endif
#if defined(GL_IMG_texture_compression_pvrtc2)
    GL_IMG_texture_compression_pvrtc2,
#else
    0,
#endif
#if defined(GL_INGR_blend_func_separate)
    GL_INGR_blend_func_separate,
#else
    0,
#endif
#if defined(GL_INGR_color_clamp)
    GL_INGR_color_clamp,
#else
    0,
#endif
#if defined(GL_INGR_interlace_read)
    GL_INGR_interlace_read,
#else
    0,
#endif
#if defined(GL_INTEL_fragment_shader_ordering)
    GL_INTEL_fragment_shader_ordering,
#else
    0,
#endif
#if defined(GL_INTEL_map_texture)
    GL_INTEL_map_texture,
#else
    0,
#endif
#if defined(GL_INTEL_parallel_arrays)
    GL_INTEL_parallel_arrays,
#else
    0,
#endif
#if defined(GL_INTEL_performance_query)
    GL_INTEL_performance_query,
#else
    0,
#endif
#if defined(GL_KHR_debug)
    GL_KHR_debug,
#else
    0,
#endif
#if defined(GL_KHR_texture_compression_astc_hdr)
    GL_KHR_texture_compression_astc_hdr,
#else
    0,
#endif
#if defined(GL_KHR_texture_compression_astc_ldr)
    GL_KHR_texture_compression_astc_ldr,
#else
    0,
#endif
#if defined(GL_MESAX_texture_stack)
    GL_MESAX_texture_stack,
#else
    0,
#endif
#if defined(GL_MESA_pack_invert)
    GL_MESA_pack_invert,
#else
    0,
#endif
#if defined(GL_MESA_resize_buffers)
    GL_MESA_resize_buffers,
#else
    0,
#endif
#if defined(GL_MESA_window_pos)
    GL_MESA_window_pos,
#else
    0,
#endif
#if defined(GL_MESA_ycbcr_texture)
    GL_MESA_ycbcr_texture,
#else
    0,
#endif
#if defined(GL_NVX_conditional_render)
    GL_NVX_conditional_render,
#else
    0,
#endif
#if defined(GL_NVX_gpu_memory_info)
    GL_NVX_gpu_memory_info,
#else
    0,
#endif
#if defined(GL_NV_bindless_multi_draw_indirect)
    GL_NV_bindless_multi_draw_indirect,
#else
    0,
#endif
#if defined(GL_NV_bindless_texture)
    GL_NV_bindless_texture,
#else
    0,
#endif
#if defined(GL_NV_blend_equation_advanced)
    GL_NV_blend_equation_advanced,
#else
    0,
#endif
#if defined(GL_NV_blend_equation_advanced_coherent)
    GL_NV_blend_equation_advanced_coherent,
#else
    0,
#endif
#if defined(GL_NV_blend_square)
    GL_NV_blend_square,
#else
    0,
#endif
#if defined(GL_NV_compute_program5)
    GL_NV_compute_program5,
#else
    0,
#endif
#if defined(GL_NV_conditional_render)
    GL_NV_conditional_render,
#else
    0,
#endif
#if defined(GL_NV_copy_buffer)
    GL_NV_copy_buffer,
#else
    0,
#endif
#if defined(GL_NV_copy_depth_to_color)
    GL_NV_copy_depth_to_color,
#else
    0,
#endif
#if defined(GL_NV_copy_image)
    GL_NV_copy_image,
#else
    0,
#endif
#if defined(GL_NV_coverage_sample)
    GL_NV_coverage_sample,
#else
    0,
#endif
#if defined(GL_NV_deep_texture3D)
    GL_NV_deep_texture3D,
#else
    0,
#endif
#if defined(GL_NV_depth_buffer_float)
    GL_NV_depth_buffer_float,
#else
    0,
#endif
#if defined(GL_NV_depth_clamp)
    GL_NV_depth_clamp,
#else
    0,
#endif
#if defined(GL_NV_depth_nonlinear)
    GL_NV_depth_nonlinear,
#else
    0,
#endif
#if defined(GL_NV_draw_buffers)
    GL_NV_draw_buffers,
#else
    0,
#endif
#if defined(GL_NV_draw_instanced)
    GL_NV_draw_instanced,
#else
    0,
#endif
#if defined(GL_NV_draw_texture)
    GL_NV_draw_texture,
#else
    0,
#endif
#if defined(GL_NV_evaluators)
    GL_NV_evaluators,
#else
    0,
#endif
#if defined(GL_NV_explicit_attrib_location)
    GL_NV_explicit_attrib_location,
#else
    0,
#endif
#if defined(GL_NV_explicit_multisample)
    GL_NV_explicit_multisample,
#else
    0,
#endif
#if defined(GL_NV_fbo_color_attachments)
    GL_NV_fbo_color_attachments,
#else
    0,
#endif
#if defined(GL_NV_fence)
    GL_NV_fence,
#else
    0,
#endif
#if defined(GL_NV_float_buffer)
    GL_NV_float_buffer,
#else
    0,
#endif
#if defined(GL_NV_fog_distance)
    GL_NV_fog_distance,
#else
    0,
#endif
#if defined(GL_NV_fragment_program)
    GL_NV_fragment_program,
#else
    0,
#endif
#if defined(GL_NV_fragment_program2)
    GL_NV_fragment_program2,
#else
    0,
#endif
#if defined(GL_NV_fragment_program4)
    GL_NV_fragment_program4,
#else
    0,
#endif
#if defined(GL_NV_fragment_program_option)
    GL_NV_fragment_program_option,
#else
    0,
#endif
#if defined(GL_NV_framebuffer_blit)
    GL_NV_framebuffer_blit,
#else
    0,
#endif
#if defined(GL_NV_framebuffer_multisample)
    GL_NV_framebuffer_multisample,
#else
    0,
#endif
#if defined(GL_NV_framebuffer_multisample_coverage)
    GL_NV_framebuffer_multisample_coverage,
#else
    0,
#endif
#if defined(GL_NV_generate_mipmap_sRGB)
    GL_NV_generate_mipmap_sRGB,
#else
    0,
#endif
#if defined(GL_NV_geometry_program4)
    GL_NV_geometry_program4,
#else
    0,
#endif
#if defined(GL_NV_geometry_shader4)
    GL_NV_geometry_shader4,
#else
    0,
#endif
#if defined(GL_NV_gpu_program4)
    GL_NV_gpu_program4,
#else
    0,
#endif
#if defined(GL_NV_gpu_program5)
    GL_NV_gpu_program5,
#else
    0,
#endif
#if defined(GL_NV_gpu_program5_mem_extended)
    GL_NV_gpu_program5_mem_extended,
#else
    0,
#endif
#if defined(GL_NV_gpu_shader5)
    GL_NV_gpu_shader5,
#else
    0,
#endif
#if defined(GL_NV_half_float)
    GL_NV_half_float,
#else
    0,
#endif
#if defined(GL_NV_instanced_arrays)
    GL_NV_instanced_arrays,
#else
    0,
#endif
#if defined(GL_NV_light_max_exponent)
    GL_NV_light_max_exponent,
#else
    0,
#endif
#if defined(GL_NV_multisample_coverage)
    GL_NV_multisample_coverage,
#else
    0,
#endif
#if defined(GL_NV_multisample_filter_hint)
    GL_NV_multisample_filter_hint,
#else
    0,
#endif
#if defined(GL_NV_non_square_matrices)
    GL_NV_non_square_matrices,
#else
    0,
#endif
#if defined(GL_NV_occlusion_query)
    GL_NV_occlusion_query,
#else
    0,
#endif
#if defined(GL_NV_packed_depth_stencil)
    GL_NV_packed_depth_stencil,
#else
    0,
#endif
#if defined(GL_NV_parameter_buffer_object)
    GL_NV_parameter_buffer_object,
#else
    0,
#endif
#if defined(GL_NV_parameter_buffer_object2)
    GL_NV_parameter_buffer_object2,
#else
    0,
#endif
#if defined(GL_NV_path_rendering)
    GL_NV_path_rendering,
#else
    0,
#endif
#if defined(GL_NV_pixel_data_range)
    GL_NV_pixel_data_range,
#else
    0,
#endif
#if defined(GL_NV_point_sprite)
    GL_NV_point_sprite,
#else
    0,
#endif
#if defined(GL_NV_present_video)
    GL_NV_present_video,
#else
    0,
#endif
#if defined(GL_NV_primitive_restart)
    GL_NV_primitive_restart,
#else
    0,
#endif
#if defined(GL_NV_read_buffer)
    GL_NV_read_buffer,
#else
    0,
#endif
#if defined(GL_NV_read_buffer_front)
    GL_NV_read_buffer_front,
#else
    0,
#endif
#if defined(GL_NV_read_depth)
    GL_NV_read_depth,
#else
    0,
#endif
#if defined(GL_NV_read_depth_stencil)
    GL_NV_read_depth_stencil,
#else
    0,
#endif
#if defined(GL_NV_read_stencil)
    GL_NV_read_stencil,
#else
    0,
#endif
#if defined(GL_NV_register_combiners)
    GL_NV_register_combiners,
#else
    0,
#endif
#if defined(GL_NV_register_combiners2)
    GL_NV_register_combiners2,
#else
    0,
#endif
#if defined(GL_NV_sRGB_formats)
    GL_NV_sRGB_formats,
#else
    0,
#endif
#if defined(GL_NV_shader_atomic_counters)
    GL_NV_shader_atomic_counters,
#else
    0,
#endif
#if defined(GL_NV_shader_atomic_float)
    GL_NV_shader_atomic_float,
#else
    0,
#endif
#if defined(GL_NV_shader_buffer_load)
    GL_NV_shader_buffer_load,
#else
    0,
#endif
#if defined(GL_NV_shader_buffer_store)
    GL_NV_shader_buffer_store,
#else
    0,
#endif
#if defined(GL_NV_shader_storage_buffer_object)
    GL_NV_shader_storage_buffer_object,
#else
    0,
#endif
#if defined(GL_NV_shadow_samplers_array)
    GL_NV_shadow_samplers_array,
#else
    0,
#endif
#if defined(GL_NV_shadow_samplers_cube)
    GL_NV_shadow_samplers_cube,
#else
    0,
#endif
#if defined(GL_NV_tessellation_program5)
    GL_NV_tessellation_program5,
#else
    0,
#endif
#if defined(GL_NV_texgen_emboss)
    GL_NV_texgen_emboss,
#else
    0,
#endif
#if defined(GL_NV_texgen_reflection)
    GL_NV_texgen_reflection,
#else
    0,
#endif
#if defined(GL_NV_texture_barrier)
    GL_NV_texture_barrier,
#else
    0,
#endif
#if defined(GL_NV_texture_border_clamp)
    GL_NV_texture_border_clamp,
#else
    0,
#endif
#if defined(GL_NV_texture_compression_s3tc_update)
    GL_NV_texture_compression_s3tc_update,
#else
    0,
#endif
#if defined(GL_NV_texture_compression_vtc)
    GL_NV_texture_compression_vtc,
#else
    0,
#endif
#if defined(GL_NV_texture_env_combine4)
    GL_NV_texture_env_combine4,
#else
    0,
#endif
#if defined(GL_NV_texture_expand_normal)
    GL_NV_texture_expand_normal,
#else
    0,
#endif
#if defined(GL_NV_texture_multisample)
    GL_NV_texture_multisample,
#else
    0,
#endif
#if defined(GL_NV_texture_npot_2D_mipmap)
    GL_NV_texture_npot_2D_mipmap,
#else
    0,
#endif
#if defined(GL_NV_texture_rectangle)
    GL_NV_texture_rectangle,
#else
    0,
#endif
#if defined(GL_NV_texture_shader)
    GL_NV_texture_shader,
#else
    0,
#endif
#if defined(GL_NV_texture_shader2)
    GL_NV_texture_shader2,
#else
    0,
#endif
#if defined(GL_NV_texture_shader3)
    GL_NV_texture_shader3,
#else
    0,
#endif
#if defined(GL_NV_transform_feedback)
    GL_NV_transform_feedback,
#else
    0,
#endif
#if defined(GL_NV_transform_feedback2)
    GL_NV_transform_feedback2,
#else
    0,
#endif
#if defined(GL_NV_vdpau_interop)
    GL_NV_vdpau_interop,
#else
    0,
#endif
#if defined(GL_NV_vertex_array_range)
    GL_NV_vertex_array_range,
#else
    0,
#endif
#if defined(GL_NV_vertex_array_range2)
    GL_NV_vertex_array_range2,
#else
    0,
#endif
#if defined(GL_NV_vertex_attrib_integer_64bit)
    GL_NV_vertex_attrib_integer_64bit,
#else
    0,
#endif
#if defined(GL_NV_vertex_buffer_unified_memory)
    GL_NV_vertex_buffer_unified_memory,
#else
    0,
#endif
#if defined(GL_NV_vertex_program)
    GL_NV_vertex_program,
#else
    0,
#endif
#if defined(GL_NV_vertex_program1_1)
    GL_NV_vertex_program1_1,
#else
    0,
#endif
#if defined(GL_NV_vertex_program2)
    GL_NV_vertex_program2,
#else
    0,
#endif
#if defined(GL_NV_vertex_program2_option)
    GL_NV_vertex_program2_option,
#else
    0,
#endif
#if defined(GL_NV_vertex_program3)
    GL_NV_vertex_program3,
#else
    0,
#endif
#if defined(GL_NV_vertex_program4)
    GL_NV_vertex_program4,
#else
    0,
#endif
#if defined(GL_NV_video_capture)
    GL_NV_video_capture,
#else
    0,
#endif
#if defined(GL_OES_EGL_image)
    GL_OES_EGL_image,
#else
    0,
#endif
#if defined(GL_OES_EGL_image_external)
    GL_OES_EGL_image_external,
#else
    0,
#endif
#if defined(GL_OES_byte_coordinates)
    GL_OES_byte_coordinates,
#else
    0,
#endif
#if defined(GL_OES_compressed_ETC1_RGB8_texture)
    GL_OES_compressed_ETC1_RGB8_texture,
#else
    0,
#endif
#if defined(GL_OES_compressed_paletted_texture)
    GL_OES_compressed_paletted_texture,
#else
    0,
#endif
#if defined(GL_OES_depth24)
    GL_OES_depth24,
#else
    0,
#endif
#if defined(GL_OES_depth32)
    GL_OES_depth32,
#else
    0,
#endif
#if defined(GL_OES_depth_texture)
    GL_OES_depth_texture,
#else
    0,
#endif
#if defined(GL_OES_element_index_uint)
    GL_OES_element_index_uint,
#else
    0,
#endif
#if defined(GL_OES_fbo_render_mipmap)
    GL_OES_fbo_render_mipmap,
#else
    0,
#endif
#if defined(GL_OES_fixed_point)
    GL_OES_fixed_point,
#else
    0,
#endif
#if defined(GL_OES_fragment_precision_high)
    GL_OES_fragment_precision_high,
#else
    0,
#endif
#if defined(GL_OES_get_program_binary)
    GL_OES_get_program_binary,
#else
    0,
#endif
#if defined(GL_OES_mapbuffer)
    GL_OES_mapbuffer,
#else
    0,
#endif
#if defined(GL_OES_packed_depth_stencil)
    GL_OES_packed_depth_stencil,
#else
    0,
#endif
#if defined(GL_OES_query_matrix)
    GL_OES_query_matrix,
#else
    0,
#endif
#if defined(GL_OES_read_format)
    GL_OES_read_format,
#else
    0,
#endif
#if defined(GL_OES_required_internalformat)
    GL_OES_required_internalformat,
#else
    0,
#endif
#if defined(GL_OES_rgb8_rgba8)
    GL_OES_rgb8_rgba8,
#else
    0,
#endif
#if defined(GL_OES_single_precision)
    GL_OES_single_precision,
#else
    0,
#endif
#if defined(GL_OES_standard_derivatives)
    GL_OES_standard_derivatives,
#else
    0,
#endif
#if defined(GL_OES_stencil1)
    GL_OES_stencil1,
#else
    0,
#endif
#if defined(GL_OES_stencil4)
    GL_OES_stencil4,
#else
    0,
#endif
#if defined(GL_OES_surfaceless_context)
    GL_OES_surfaceless_context,
#else
    0,
#endif
#if defined(GL_OES_texture_3D)
    GL_OES_texture_3D,
#else
    0,
#endif
#if defined(GL_OES_texture_compression_astc)
    GL_OES_texture_compression_astc,
#else
    0,
#endif
#if defined(GL_OES_texture_float)
    GL_OES_texture_float,
#else
    0,
#endif
#if defined(GL_OES_texture_float_linear)
    GL_OES_texture_float_linear,
#else
    0,
#endif
#if defined(GL_OES_texture_half_float)
    GL_OES_texture_half_float,
#else
    0,
#endif
#if defined(GL_OES_texture_half_float_linear)
    GL_OES_texture_half_float_linear,
#else
    0,
#endif
#if defined(GL_OES_texture_npot)
    GL_OES_texture_npot,
#else
    0,
#endif
#if defined(GL_OES_vertex_array_object)
    GL_OES_vertex_array_object,
#else
    0,
#endif
#if defined(GL_OES_vertex_half_float)
    GL_OES_vertex_half_float,
#else
    0,
#endif
#if defined(GL_OES_vertex_type_10_10_10_2)
    GL_OES_vertex_type_10_10_10_2,
#else
    0,
#endif
#if defined(GL_OML_interlace)
    GL_OML_interlace,
#else
    0,
#endif
#if defined(GL_OML_resample)
    GL_OML_resample,
#else
    0,
#endif
#if defined(GL_OML_subsample)
    GL_OML_subsample,
#else
    0,
#endif
#if defined(GL_PGI_misc_hints)
    GL_PGI_misc_hints,
#else
    0,
#endif
#if defined(GL_PGI_vertex_hints)
    GL_PGI_vertex_hints,
#else
    0,
#endif
#if defined(GL_QCOM_alpha_test)
    GL_QCOM_alpha_test,
#else
    0,
#endif
#if defined(GL_QCOM_binning_control)
    GL_QCOM_binning_control,
#else
    0,
#endif
#if defined(GL_QCOM_driver_control)
    GL_QCOM_driver_control,
#else
    0,
#endif
#if defined(GL_QCOM_extended_get)
    GL_QCOM_extended_get,
#else
    0,
#endif
#if defined(GL_QCOM_extended_get2)
    GL_QCOM_extended_get2,
#else
    0,
#endif
#if defined(GL_QCOM_perfmon_global_mode)
    GL_QCOM_perfmon_global_mode,
#else
    0,
#endif
#if defined(GL_QCOM_tiled_rendering)
    GL_QCOM_tiled_rendering,
#else
    0,
#endif
#if defined(GL_QCOM_writeonly_rendering)
    GL_QCOM_writeonly_rendering,
#else
    0,
#endif
#if defined(GL_REND_screen_coordinates)
    GL_REND_screen_coordinates,
#else
    0,
#endif
#if defined(GL_S3_s3tc)
    GL_S3_s3tc,
#else
    0,
#endif
#if defined(GL_SGIS_detail_texture)
    GL_SGIS_detail_texture,
#else
    0,
#endif
#if defined(GL_SGIS_fog_function)
    GL_SGIS_fog_function,
#else
    0,
#endif
#if defined(GL_SGIS_generate_mipmap)
    GL_SGIS_generate_mipmap,
#else
    0,
#endif
#if defined(GL_SGIS_multisample)
    GL_SGIS_multisample,
#else
    0,
#endif
#if defined(GL_SGIS_pixel_texture)
    GL_SGIS_pixel_texture,
#else
    0,
#endif
#if defined(GL_SGIS_point_line_texgen)
    GL_SGIS_point_line_texgen,
#else
    0,
#endif
#if defined(GL_SGIS_point_parameters)
    GL_SGIS_point_parameters,
#else
    0,
#endif
#if defined(GL_SGIS_sharpen_texture)
    GL_SGIS_sharpen_texture,
#else
    0,
#endif
#if defined(GL_SGIS_texture4D)
    GL_SGIS_texture4D,
#else
    0,
#endif
#if defined(GL_SGIS_texture_border_clamp)
    GL_SGIS_texture_border_clamp,
#else
    0,
#endif
#if defined(GL_SGIS_texture_color_mask)
    GL_SGIS_texture_color_mask,
#else
    0,
#endif
#if defined(GL_SGIS_texture_edge_clamp)
    GL_SGIS_texture_edge_clamp,
#else
    0,
#endif
#if defined(GL_SGIS_texture_filter4)
    GL_SGIS_texture_filter4,
#else
    0,
#endif
#if defined(GL_SGIS_texture_lod)
    GL_SGIS_texture_lod,
#else
    0,
#endif
#if defined(GL_SGIS_texture_select)
    GL_SGIS_texture_select,
#else
    0,
#endif
#if defined(GL_SGIX_async)
    GL_SGIX_async,
#else
    0,
#endif
#if defined(GL_SGIX_async_histogram)
    GL_SGIX_async_histogram,
#else
    0,
#endif
#if defined(GL_SGIX_async_pixel)
    GL_SGIX_async_pixel,
#else
    0,
#endif
#if defined(GL_SGIX_blend_alpha_minmax)
    GL_SGIX_blend_alpha_minmax,
#else
    0,
#endif
#if defined(GL_SGIX_calligraphic_fragment)
    GL_SGIX_calligraphic_fragment,
#else
    0,
#endif
#if defined(GL_SGIX_clipmap)
    GL_SGIX_clipmap,
#else
    0,
#endif
#if defined(GL_SGIX_convolution_accuracy)
    GL_SGIX_convolution_accuracy,
#else
    0,
#endif
#if defined(GL_SGIX_depth_pass_instrument)
    GL_SGIX_depth_pass_instrument,
#else
    0,
#endif
#if defined(GL_SGIX_depth_texture)
    GL_SGIX_depth_texture,
#else
    0,
#endif
#if defined(GL_SGIX_flush_raster)
    GL_SGIX_flush_raster,
#else
    0,
#endif
#if defined(GL_SGIX_fog_offset)
    GL_SGIX_fog_offset,
#else
    0,
#endif
#if defined(GL_SGIX_fragment_lighting)
    GL_SGIX_fragment_lighting,
#else
    0,
#endif
#if defined(GL_SGIX_framezoom)
    GL_SGIX_framezoom,
#else
    0,
#endif
#if defined(GL_SGIX_igloo_interface)
    GL_SGIX_igloo_interface,
#else
    0,
#endif
#if defined(GL_SGIX_instruments)
    GL_SGIX_instruments,
#else
    0,
#endif
#if defined(GL_SGIX_interlace)
    GL_SGIX_interlace,
#else
    0,
#endif
#if defined(GL_SGIX_ir_instrument1)
    GL_SGIX_ir_instrument1,
#else
    0,
#endif
#if defined(GL_SGIX_list_priority)
    GL_SGIX_list_priority,
#else
    0,
#endif
#if defined(GL_SGIX_pixel_texture)
    GL_SGIX_pixel_texture,
#else
    0,
#endif
#if defined(GL_SGIX_pixel_tiles)
    GL_SGIX_pixel_tiles,
#else
    0,
#endif
#if defined(GL_SGIX_polynomial_ffd)
    GL_SGIX_polynomial_ffd,
#else
    0,
#endif
#if defined(GL_SGIX_reference_plane)
    GL_SGIX_reference_plane,
#else
    0,
#endif
#if defined(GL_SGIX_resample)
    GL_SGIX_resample,
#else
    0,
#endif
#if defined(GL_SGIX_scalebias_hint)
    GL_SGIX_scalebias_hint,
#else
    0,
#endif
#if defined(GL_SGIX_shadow)
    GL_SGIX_shadow,
#else
    0,
#endif
#if defined(GL_SGIX_shadow_ambient)
    GL_SGIX_shadow_ambient,
#else
    0,
#endif
#if defined(GL_SGIX_sprite)
    GL_SGIX_sprite,
#else
    0,
#endif
#if defined(GL_SGIX_subsample)
    GL_SGIX_subsample,
#else
    0,
#endif
#if defined(GL_SGIX_tag_sample_buffer)
    GL_SGIX_tag_sample_buffer,
#else
    0,
#endif
#if defined(GL_SGIX_texture_add_env)
    GL_SGIX_texture_add_env,
#else
    0,
#endif
#if defined(GL_SGIX_texture_coordinate_clamp)
    GL_SGIX_texture_coordinate_clamp,
#else
    0,
#endif
#if defined(GL_SGIX_texture_lod_bias)
    GL_SGIX_texture_lod_bias,
#else
    0,
#endif
#if defined(GL_SGIX_texture_multi_buffer)
    GL_SGIX_texture_multi_buffer,
#else
    0,
#endif
#if defined(GL_SGIX_texture_scale_bias)
    GL_SGIX_texture_scale_bias,
#else
    0,
#endif
#if defined(GL_SGIX_vertex_preclip)
    GL_SGIX_vertex_preclip,
#else
    0,
#endif
#if defined(GL_SGIX_ycrcb)
    GL_SGIX_ycrcb,
#else
    0,
#endif
#if defined(GL_SGIX_ycrcb_subsample)
    GL_SGIX_ycrcb_subsample,
#else
    0,
#endif
#if defined(GL_SGIX_ycrcba)
    GL_SGIX_ycrcba,
#else
    0,
#endif
#if defined(GL_SGI_color_matrix)
    GL_SGI_color_matrix,
#else
    0,
#endif
#if defined(GL_SGI_color_table)
    GL_SGI_color_table,
#else
    0,
#endif
#if defined(GL_SGI_texture_color_table)
    GL_SGI_texture_color_table,
#else
    0,
#endif
#if defined(GL_SUNX_constant_data)
    GL_SUNX_constant_data,
#else
    0,
#endif
#if defined(GL_SUN_convolution_border_modes)
    GL_SUN_convolution_border_modes,
#else
    0,
#endif
#if defined(GL_SUN_global_alpha)
    GL_SUN_global_alpha,
#else
    0,
#endif
#if defined(GL_SUN_mesh_array)
    GL_SUN_mesh_array,
#else
    0,
#endif
#if defined(GL_SUN_slice_accum)
    GL_SUN_slice_accum,
#else
    0,
#endif
#if defined(GL_SUN_triangle_list)
    GL_SUN_triangle_list,
#else
    0,
#endif
#if defined(GL_SUN_vertex)
    GL_SUN_vertex,
#else
    0,
#endif
#if defined(GL_VERSION_1_1)
    GL_VERSION_1_1,
#else
    0,
#endif
#if defined(GL_VERSION_1_2)
    GL_VERSION_1_2,
#else
    0,
#endif
#if defined(GL_VERSION_1_3)
    GL_VERSION_1_3,
#else
    0,
#endif
#if defined(GL_VERSION_1_4)
    GL_VERSION_1_4,
#else
    0,
#endif
#if defined(GL_VERSION_1_5)
    GL_VERSION_1_5,
#else
    0,
#endif
#if defined(GL_VERSION_2_0)
    GL_VERSION_2_0,
#else
    0,
#endif
#if defined(GL_VERSION_2_1)
    GL_VERSION_2_1,
#else
    0,
#endif
#if defined(GL_VERSION_3_0)
    GL_VERSION_3_0,
#else
    0,
#endif
#if defined(GL_VERSION_3_1)
    GL_VERSION_3_1,
#else
    0,
#endif
#if defined(GL_VERSION_3_2)
    GL_VERSION_3_2,
#else
    0,
#endif
#if defined(GL_VERSION_3_3)
    GL_VERSION_3_3,
#else
    0,
#endif
#if defined(GL_VERSION_4_0)
    GL_VERSION_4_0,
#else
    0,
#endif
#if defined(GL_VERSION_4_1)
    GL_VERSION_4_1,
#else
    0,
#endif
#if defined(GL_VERSION_4_2)
    GL_VERSION_4_2,
#else
    0,
#endif
#if defined(GL_VERSION_4_3)
    GL_VERSION_4_3,
#else
    0,
#endif
#if defined(GL_VERSION_4_4)
    GL_VERSION_4_4,
#else
    0,
#endif
#if defined(GL_VIV_shader_binary)
    GL_VIV_shader_binary,
#else
    0,
#endif
#if defined(GL_WIN_phong_shading)
    GL_WIN_phong_shading,
#else
    0,
#endif
#if defined(GL_WIN_specular_fog)
    GL_WIN_specular_fog,
#else
    0,
#endif
};

class StringLess
{
public:
    bool operator() (const char* s0, const char* s1)
    {
        return SFstrcmp(s0, s1) < 0;
    }
};

void GLImmediate::initExtensions(unsigned verbosity)
{
    SF_UNUSED(verbosity); // for release.

    // Set GL and GLES version 'extensions', based on the version of GL.
#if defined(SF_RENDER_OPENGL)
    if (CheckGLVersion(1, 1)) ExtensionSupported[SF_GL_VERSION_1_1] = true;
    if (CheckGLVersion(1, 2)) ExtensionSupported[SF_GL_VERSION_1_2] = true;
    if (CheckGLVersion(1, 3)) ExtensionSupported[SF_GL_VERSION_1_3] = true;
    if (CheckGLVersion(1, 4)) ExtensionSupported[SF_GL_VERSION_1_4] = true;
    if (CheckGLVersion(1, 5)) ExtensionSupported[SF_GL_VERSION_1_5] = true;
    if (CheckGLVersion(2, 0)) ExtensionSupported[SF_GL_VERSION_2_0] = true;
    if (CheckGLVersion(2, 1)) ExtensionSupported[SF_GL_VERSION_2_1] = true;
    if (CheckGLVersion(3, 0)) ExtensionSupported[SF_GL_VERSION_3_0] = true;
    if (CheckGLVersion(3, 1)) ExtensionSupported[SF_GL_VERSION_3_1] = true;
    if (CheckGLVersion(3, 2)) ExtensionSupported[SF_GL_VERSION_3_2] = true;
    if (CheckGLVersion(3, 3)) ExtensionSupported[SF_GL_VERSION_3_3] = true;
    if (CheckGLVersion(4, 0)) ExtensionSupported[SF_GL_VERSION_4_0] = true;
    if (CheckGLVersion(4, 1)) ExtensionSupported[SF_GL_VERSION_4_1] = true;
    if (CheckGLVersion(4, 2)) ExtensionSupported[SF_GL_VERSION_4_2] = true;
    if (CheckGLVersion(4, 3)) ExtensionSupported[SF_GL_VERSION_4_3] = true;
    if (CheckGLVersion(4, 4)) ExtensionSupported[SF_GL_VERSION_4_4] = true;
#elif defined(SF_RENDER_GLES)
    if (CheckGLVersion(2, 0)) ExtensionSupported[SF_GL_ES_VERSION_2_0] = true;
    if (CheckGLVersion(3, 0)) ExtensionSupported[SF_GL_ES_VERSION_3_0] = true;
#endif

    // Initialize the GL version.
    if (MajorVersion == 0 && MinorVersion == 0)
    {
        const char* version = (const char*)glGetString(GL_VERSION);
#if defined(SF_RENDER_OPENGL)
        SFsscanf(version, "%d.%d", &MajorVersion, &MinorVersion);
#elif defined(SF_RENDER_GLES)
        SFsscanf(version, "OpenGL ES %d.%d", &MajorVersion, &MinorVersion);
#endif
    }
    
    // NOTE: There must be storage for the extension strings, because in GL 3.0, the lifetime of the pointer returned from 
    // glGetStringi is not guaranteed. In fact, on some implementations, glGetStringi with increasing index will always return 
    // a pointer to the same memory, just with the string overwritten.
    const int MaximumExtensionLength = 128;
    char extensionStorage[SF_GL_Extension_Count][MaximumExtensionLength];
    const char* extensions[SF_GL_Extension_Count];
    memset(extensions, 0, sizeof(extensions));
    memset(extensionStorage, 0, sizeof(extensionStorage));    
    unsigned extIndex = 0;
   
    if (!CheckGLVersion(3,0))
    {
        String extensionOutput;
        extensionOutput = ((const char *) glGetString(GL_EXTENSIONS));
        const char* extensionString = const_cast<char*>(extensionOutput.ToCStr());
        SPInt index = 0;
        StringDataPtr wrapper(extensionOutput);
        while (index < (SPInt)extensionOutput.GetLength())
        {
            SPInt nextChar = wrapper.FindChar(' ', index);
            if (nextChar < 0)
                nextChar = extensionOutput.GetLength();

            SFstrncpy(extensionStorage[extIndex], MaximumExtensionLength, &extensionString[index], nextChar-index);
            extensions[extIndex] = extensionStorage[extIndex];
            extIndex++;
            index = nextChar+1;
        }
    }
    else
    {
        // NOTE: In OpenGL 3.0+, we must specifically runtime link glGetStringi.
#if defined(SF_GL_RUNTIME_LINK)
        if (!p_glGetStringi)
        {
            p_glGetStringi = (PFNGLGETSTRINGIPROC) SF_GL_RUNTIME_LINK("glGetStringi");
            SF_DEBUG_ASSERT(p_glGetStringi , "Function glGetStringi required for GL 3.0+ was not found.\n");
        }
#endif // SF_GL_RUNTIME_LINK

        GLint extCount;
        glGetIntegerv(GL_NUM_EXTENSIONS, &extCount);
        SF_DEBUG_ASSERT2(extCount < SF_GL_Extension_Count, "Too many GL extensions reported (max=%d, reported=%d)", SF_GL_Extension_Count, extCount);
        for (extIndex = 0; extIndex < (unsigned)extCount; ++extIndex)
        {
            const char* str = (const char*)glGetStringi(GL_EXTENSIONS, extIndex);
            SFstrncpy(extensionStorage[extIndex], MaximumExtensionLength, str, SFstrlen(str));
            extensions[extIndex] = extensionStorage[extIndex];
        }
    }

    // Now sort the strings.
    Alg::ArrayAdaptor<const char*> adapter(extensions, extIndex);
    StringLess compare;
    Alg::QuickSort(adapter, compare);



    unsigned index1 = 0;
    unsigned index2 = SF_GL_NONE+1; // The NONE string is not sorted, so skip it.
    while (index1 < extIndex)
    {
        // the GL_VERSION_* 'extensions' have already been set, so, skip them.
        if (index2 < SF_GL_Extension_Count && ExtensionSupported[index2])
        {
            index2++;
            continue;
        }

        int compareResult = index2 >= SF_GL_Extension_Count ? -1 : SFstrcmp(extensions[index1], ExtensionStrings[index2]);

        if (compareResult == 0) // Strings are equal. This extension is supported on this system.
        {
            SF_DEBUG_MESSAGE1(verbosity >= Verbosity_Info, "Extension is reported (%s)\n", ExtensionStrings[index2]);
            ExtensionSupported[index2] = true;
            if (ExtensionsDefinedInGLEXT[index2] == 0)
            {
                SF_DEBUG_MESSAGE1(verbosity >= Verbosity_Info, "Extension is reported, but not defined in extension header (%s). Disabling usage.\n", ExtensionStrings[index2]);
                ExtensionSupported[index2] = false;
            }
            index1++;
            index2++;
        }
        else if (compareResult > 0) // First extension is not supported.
        {
            SF_DEBUG_MESSAGE1(verbosity >= Verbosity_Info, "Extension is not reported (%s)\n", ExtensionStrings[index2]);
            ExtensionSupported[index2] = false;
            index2++;
        }
        else if (compareResult < 0) // Found an extension that wasn't in the list.
        {
            SF_DEBUG_MESSAGE1(verbosity >= Verbosity_Warnings, "Extension was reported that the system was unaware of (%s)\n", extensions[index1]);
            index1++;
        }
    }
}

void GLImmediate::Init(unsigned verbosity)
{
    Verbosity = (VerbosityType)verbosity;
    initExtensions(verbosity);
#if defined(SF_GL_RUNTIME_LINK)
   #if defined(SF_RENDER_OPENGL)
   if (!p_glActiveTexture && true)
   {
       p_glActiveTexture = (PFNGLACTIVETEXTUREPROC) SF_GL_RUNTIME_LINK("glActiveTexture");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glActiveTexture , "Found binding: glActiveTexture\n");
   }
   #endif
    #if defined(GL_ARB_multitexture)
    if (!p_glActiveTexture && CheckExtension(SF_GL_ARB_multitexture))
    {
        p_glActiveTexture = (PFNGLACTIVETEXTUREPROC) SF_GL_RUNTIME_LINK("glActiveTextureARB");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glActiveTexture, "Found binding: glActiveTexture (glActiveTextureARB)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glAttachShader && true)
   {
       p_glAttachShader = (PFNGLATTACHSHADERPROC) SF_GL_RUNTIME_LINK("glAttachShader");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glAttachShader , "Found binding: glAttachShader\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glBindAttribLocation && true)
   {
       p_glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC) SF_GL_RUNTIME_LINK("glBindAttribLocation");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBindAttribLocation , "Found binding: glBindAttribLocation\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glBindBuffer && true)
   {
       p_glBindBuffer = (PFNGLBINDBUFFERPROC) SF_GL_RUNTIME_LINK("glBindBuffer");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBindBuffer , "Found binding: glBindBuffer\n");
   }
   #endif
    #if defined(GL_ARB_vertex_buffer_object)
    if (!p_glBindBuffer && CheckExtension(SF_GL_ARB_vertex_buffer_object))
    {
        p_glBindBuffer = (PFNGLBINDBUFFERPROC) SF_GL_RUNTIME_LINK("glBindBufferARB");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBindBuffer, "Found binding: glBindBuffer (glBindBufferARB)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glBindFragDataLocation && true)
   {
       p_glBindFragDataLocation = (PFNGLBINDFRAGDATALOCATIONPROC) SF_GL_RUNTIME_LINK("glBindFragDataLocation");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBindFragDataLocation , "Found binding: glBindFragDataLocation\n");
   }
   #endif
    #if defined(GL_EXT_gpu_shader4)
    if (!p_glBindFragDataLocation && CheckExtension(SF_GL_EXT_gpu_shader4))
    {
        p_glBindFragDataLocation = (PFNGLBINDFRAGDATALOCATIONPROC) SF_GL_RUNTIME_LINK("glBindFragDataLocationEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBindFragDataLocation, "Found binding: glBindFragDataLocation (glBindFragDataLocationEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glBindFramebuffer && true)
   {
       p_glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) SF_GL_RUNTIME_LINK("glBindFramebuffer");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBindFramebuffer , "Found binding: glBindFramebuffer\n");
   }
   #endif
    #if defined(GL_EXT_framebuffer_object)
    if (!p_glBindFramebuffer && CheckExtension(SF_GL_EXT_framebuffer_object))
    {
        p_glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) SF_GL_RUNTIME_LINK("glBindFramebufferEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBindFramebuffer, "Found binding: glBindFramebuffer (glBindFramebufferEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glBindProgramPipeline && true)
   {
       p_glBindProgramPipeline = (PFNGLBINDPROGRAMPIPELINEPROC) SF_GL_RUNTIME_LINK("glBindProgramPipeline");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBindProgramPipeline , "Found binding: glBindProgramPipeline\n");
   }
   #endif
   #if defined(GL_ARB_separate_shader_objects)
   if (!p_glBindProgramPipeline && CheckExtension(SF_GL_ARB_separate_shader_objects))
   {
       p_glBindProgramPipeline = (PFNGLBINDPROGRAMPIPELINEPROC) SF_GL_RUNTIME_LINK("glBindProgramPipeline");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBindProgramPipeline , "Found binding: glBindProgramPipeline\n");
   }
   #endif
    #if defined(GL_EXT_separate_shader_objects)
    if (!p_glBindProgramPipeline && CheckExtension(SF_GL_EXT_separate_shader_objects))
    {
        p_glBindProgramPipeline = (PFNGLBINDPROGRAMPIPELINEPROC) SF_GL_RUNTIME_LINK("glBindProgramPipelineEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBindProgramPipeline, "Found binding: glBindProgramPipeline (glBindProgramPipelineEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glBindRenderbuffer && true)
   {
       p_glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) SF_GL_RUNTIME_LINK("glBindRenderbuffer");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBindRenderbuffer , "Found binding: glBindRenderbuffer\n");
   }
   #endif
    #if defined(GL_EXT_framebuffer_object)
    if (!p_glBindRenderbuffer && CheckExtension(SF_GL_EXT_framebuffer_object))
    {
        p_glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) SF_GL_RUNTIME_LINK("glBindRenderbufferEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBindRenderbuffer, "Found binding: glBindRenderbuffer (glBindRenderbufferEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glBindVertexArray && true)
   {
       p_glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC) SF_GL_RUNTIME_LINK("glBindVertexArray");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBindVertexArray , "Found binding: glBindVertexArray\n");
   }
   #endif
   #if defined(SF_RENDER_GLES)
   if (!p_glBindVertexArray && CheckGLVersion(3,0))
   {
       p_glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC) SF_GL_RUNTIME_LINK("glBindVertexArray");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBindVertexArray , "Found binding: glBindVertexArray\n");
   }
   #endif
   #if defined(GL_ARB_vertex_array_object)
   if (!p_glBindVertexArray && CheckExtension(SF_GL_ARB_vertex_array_object))
   {
       p_glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC) SF_GL_RUNTIME_LINK("glBindVertexArray");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBindVertexArray , "Found binding: glBindVertexArray\n");
   }
   #endif
    #if defined(GL_APPLE_vertex_array_object)
    if (!p_glBindVertexArray && CheckExtension(SF_GL_APPLE_vertex_array_object))
    {
        p_glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC) SF_GL_RUNTIME_LINK("glBindVertexArrayAPPLE");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBindVertexArray, "Found binding: glBindVertexArray (glBindVertexArrayAPPLE)\n");
    }
    #endif
    #if defined(GL_OES_vertex_array_object)
    if (!p_glBindVertexArray && CheckExtension(SF_GL_OES_vertex_array_object))
    {
        p_glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC) SF_GL_RUNTIME_LINK("glBindVertexArrayOES");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBindVertexArray, "Found binding: glBindVertexArray (glBindVertexArrayOES)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glBlendEquation && true)
   {
       p_glBlendEquation = (PFNGLBLENDEQUATIONPROC) SF_GL_RUNTIME_LINK("glBlendEquation");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBlendEquation , "Found binding: glBlendEquation\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glBlendEquationSeparate && true)
   {
       p_glBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC) SF_GL_RUNTIME_LINK("glBlendEquationSeparate");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBlendEquationSeparate , "Found binding: glBlendEquationSeparate\n");
   }
   #endif
    #if defined(GL_EXT_blend_equation_separate)
    if (!p_glBlendEquationSeparate && CheckExtension(SF_GL_EXT_blend_equation_separate))
    {
        p_glBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC) SF_GL_RUNTIME_LINK("glBlendEquationSeparateEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBlendEquationSeparate, "Found binding: glBlendEquationSeparate (glBlendEquationSeparateEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glBlendFuncSeparate && true)
   {
       p_glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC) SF_GL_RUNTIME_LINK("glBlendFuncSeparate");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBlendFuncSeparate , "Found binding: glBlendFuncSeparate\n");
   }
   #endif
    #if defined(GL_INGR_blend_func_separate)
    if (!p_glBlendFuncSeparate && CheckExtension(SF_GL_INGR_blend_func_separate))
    {
        p_glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC) SF_GL_RUNTIME_LINK("glBlendFuncSeparateINGR");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBlendFuncSeparate, "Found binding: glBlendFuncSeparate (glBlendFuncSeparateINGR)\n");
    }
    #endif
    #if defined(GL_EXT_blend_func_separate)
    if (!p_glBlendFuncSeparate && CheckExtension(SF_GL_EXT_blend_func_separate))
    {
        p_glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC) SF_GL_RUNTIME_LINK("glBlendFuncSeparateEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBlendFuncSeparate, "Found binding: glBlendFuncSeparate (glBlendFuncSeparateEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glBufferData && true)
   {
       p_glBufferData = (PFNGLBUFFERDATAPROC) SF_GL_RUNTIME_LINK("glBufferData");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBufferData , "Found binding: glBufferData\n");
   }
   #endif
    #if defined(GL_ARB_vertex_buffer_object)
    if (!p_glBufferData && CheckExtension(SF_GL_ARB_vertex_buffer_object))
    {
        p_glBufferData = (PFNGLBUFFERDATAPROC) SF_GL_RUNTIME_LINK("glBufferDataARB");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBufferData, "Found binding: glBufferData (glBufferDataARB)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glBufferSubData && true)
   {
       p_glBufferSubData = (PFNGLBUFFERSUBDATAPROC) SF_GL_RUNTIME_LINK("glBufferSubData");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBufferSubData , "Found binding: glBufferSubData\n");
   }
   #endif
    #if defined(GL_ARB_vertex_buffer_object)
    if (!p_glBufferSubData && CheckExtension(SF_GL_ARB_vertex_buffer_object))
    {
        p_glBufferSubData = (PFNGLBUFFERSUBDATAPROC) SF_GL_RUNTIME_LINK("glBufferSubDataARB");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glBufferSubData, "Found binding: glBufferSubData (glBufferSubDataARB)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glCheckFramebufferStatus && true)
   {
       p_glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) SF_GL_RUNTIME_LINK("glCheckFramebufferStatus");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glCheckFramebufferStatus , "Found binding: glCheckFramebufferStatus\n");
   }
   #endif
    #if defined(GL_EXT_framebuffer_object)
    if (!p_glCheckFramebufferStatus && CheckExtension(SF_GL_EXT_framebuffer_object))
    {
        p_glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) SF_GL_RUNTIME_LINK("glCheckFramebufferStatusEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glCheckFramebufferStatus, "Found binding: glCheckFramebufferStatus (glCheckFramebufferStatusEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_GLES)
   if (!p_glClientWaitSync && CheckGLVersion(3,0))
   {
       p_glClientWaitSync = (PFNGLCLIENTWAITSYNCPROC) SF_GL_RUNTIME_LINK("glClientWaitSync");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glClientWaitSync , "Found binding: glClientWaitSync\n");
   }
   #endif
   #if defined(GL_ARB_sync)
   if (!p_glClientWaitSync && CheckExtension(SF_GL_ARB_sync))
   {
       p_glClientWaitSync = (PFNGLCLIENTWAITSYNCPROC) SF_GL_RUNTIME_LINK("glClientWaitSync");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glClientWaitSync , "Found binding: glClientWaitSync\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glClientWaitSync && true)
   {
       p_glClientWaitSync = (PFNGLCLIENTWAITSYNCPROC) SF_GL_RUNTIME_LINK("glClientWaitSync");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glClientWaitSync , "Found binding: glClientWaitSync\n");
   }
   #endif
    #if defined(GL_APPLE_sync)
    if (!p_glClientWaitSync && CheckExtension(SF_GL_APPLE_sync))
    {
        p_glClientWaitSync = (PFNGLCLIENTWAITSYNCPROC) SF_GL_RUNTIME_LINK("glClientWaitSyncAPPLE");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glClientWaitSync, "Found binding: glClientWaitSync (glClientWaitSyncAPPLE)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glCompileShader && true)
   {
       p_glCompileShader = (PFNGLCOMPILESHADERPROC) SF_GL_RUNTIME_LINK("glCompileShader");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glCompileShader , "Found binding: glCompileShader\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glCompressedTexImage2D && true)
   {
       p_glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC) SF_GL_RUNTIME_LINK("glCompressedTexImage2D");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glCompressedTexImage2D , "Found binding: glCompressedTexImage2D\n");
   }
   #endif
    #if defined(GL_ARB_texture_compression)
    if (!p_glCompressedTexImage2D && CheckExtension(SF_GL_ARB_texture_compression))
    {
        p_glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC) SF_GL_RUNTIME_LINK("glCompressedTexImage2DARB");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glCompressedTexImage2D, "Found binding: glCompressedTexImage2D (glCompressedTexImage2DARB)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glCreateProgram && true)
   {
       p_glCreateProgram = (PFNGLCREATEPROGRAMPROC) SF_GL_RUNTIME_LINK("glCreateProgram");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glCreateProgram , "Found binding: glCreateProgram\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glCreateShader && true)
   {
       p_glCreateShader = (PFNGLCREATESHADERPROC) SF_GL_RUNTIME_LINK("glCreateShader");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glCreateShader , "Found binding: glCreateShader\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glDebugMessageCallback && true)
   {
       p_glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC) SF_GL_RUNTIME_LINK("glDebugMessageCallback");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDebugMessageCallback , "Found binding: glDebugMessageCallback\n");
   }
   #endif
    #if defined(GL_ARB_debug_output)
    if (!p_glDebugMessageCallback && CheckExtension(SF_GL_ARB_debug_output))
    {
        p_glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC) SF_GL_RUNTIME_LINK("glDebugMessageCallbackARB");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDebugMessageCallback, "Found binding: glDebugMessageCallback (glDebugMessageCallbackARB)\n");
    }
    #endif
    #if defined(GL_AMD_debug_output)
    if (!p_glDebugMessageCallback && CheckExtension(SF_GL_AMD_debug_output))
    {
        p_glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC) SF_GL_RUNTIME_LINK("glDebugMessageCallbackAMD");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDebugMessageCallback, "Found binding: glDebugMessageCallback (glDebugMessageCallbackAMD)\n");
    }
    #endif
    #if defined(GL_KHR_debug)
    if (!p_glDebugMessageCallback && CheckExtension(SF_GL_KHR_debug))
    {
        p_glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC) SF_GL_RUNTIME_LINK("glDebugMessageCallbackKHR");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDebugMessageCallback, "Found binding: glDebugMessageCallback (glDebugMessageCallbackKHR)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glDebugMessageControl && true)
   {
       p_glDebugMessageControl = (PFNGLDEBUGMESSAGECONTROLPROC) SF_GL_RUNTIME_LINK("glDebugMessageControl");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDebugMessageControl , "Found binding: glDebugMessageControl\n");
   }
   #endif
    #if defined(GL_ARB_debug_output)
    if (!p_glDebugMessageControl && CheckExtension(SF_GL_ARB_debug_output))
    {
        p_glDebugMessageControl = (PFNGLDEBUGMESSAGECONTROLPROC) SF_GL_RUNTIME_LINK("glDebugMessageControlARB");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDebugMessageControl, "Found binding: glDebugMessageControl (glDebugMessageControlARB)\n");
    }
    #endif
    #if defined(GL_KHR_debug)
    if (!p_glDebugMessageControl && CheckExtension(SF_GL_KHR_debug))
    {
        p_glDebugMessageControl = (PFNGLDEBUGMESSAGECONTROLPROC) SF_GL_RUNTIME_LINK("glDebugMessageControlKHR");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDebugMessageControl, "Found binding: glDebugMessageControl (glDebugMessageControlKHR)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glDeleteBuffers && true)
   {
       p_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) SF_GL_RUNTIME_LINK("glDeleteBuffers");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDeleteBuffers , "Found binding: glDeleteBuffers\n");
   }
   #endif
    #if defined(GL_ARB_vertex_buffer_object)
    if (!p_glDeleteBuffers && CheckExtension(SF_GL_ARB_vertex_buffer_object))
    {
        p_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) SF_GL_RUNTIME_LINK("glDeleteBuffersARB");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDeleteBuffers, "Found binding: glDeleteBuffers (glDeleteBuffersARB)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glDeleteFramebuffers && true)
   {
       p_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) SF_GL_RUNTIME_LINK("glDeleteFramebuffers");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDeleteFramebuffers , "Found binding: glDeleteFramebuffers\n");
   }
   #endif
    #if defined(GL_EXT_framebuffer_object)
    if (!p_glDeleteFramebuffers && CheckExtension(SF_GL_EXT_framebuffer_object))
    {
        p_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) SF_GL_RUNTIME_LINK("glDeleteFramebuffersEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDeleteFramebuffers, "Found binding: glDeleteFramebuffers (glDeleteFramebuffersEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glDeleteProgram && true)
   {
       p_glDeleteProgram = (PFNGLDELETEPROGRAMPROC) SF_GL_RUNTIME_LINK("glDeleteProgram");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDeleteProgram , "Found binding: glDeleteProgram\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glDeleteProgramPipelines && true)
   {
       p_glDeleteProgramPipelines = (PFNGLDELETEPROGRAMPIPELINESPROC) SF_GL_RUNTIME_LINK("glDeleteProgramPipelines");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDeleteProgramPipelines , "Found binding: glDeleteProgramPipelines\n");
   }
   #endif
   #if defined(GL_ARB_separate_shader_objects)
   if (!p_glDeleteProgramPipelines && CheckExtension(SF_GL_ARB_separate_shader_objects))
   {
       p_glDeleteProgramPipelines = (PFNGLDELETEPROGRAMPIPELINESPROC) SF_GL_RUNTIME_LINK("glDeleteProgramPipelines");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDeleteProgramPipelines , "Found binding: glDeleteProgramPipelines\n");
   }
   #endif
    #if defined(GL_EXT_separate_shader_objects)
    if (!p_glDeleteProgramPipelines && CheckExtension(SF_GL_EXT_separate_shader_objects))
    {
        p_glDeleteProgramPipelines = (PFNGLDELETEPROGRAMPIPELINESPROC) SF_GL_RUNTIME_LINK("glDeleteProgramPipelinesEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDeleteProgramPipelines, "Found binding: glDeleteProgramPipelines (glDeleteProgramPipelinesEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glDeleteRenderbuffers && true)
   {
       p_glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) SF_GL_RUNTIME_LINK("glDeleteRenderbuffers");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDeleteRenderbuffers , "Found binding: glDeleteRenderbuffers\n");
   }
   #endif
    #if defined(GL_EXT_framebuffer_object)
    if (!p_glDeleteRenderbuffers && CheckExtension(SF_GL_EXT_framebuffer_object))
    {
        p_glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) SF_GL_RUNTIME_LINK("glDeleteRenderbuffersEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDeleteRenderbuffers, "Found binding: glDeleteRenderbuffers (glDeleteRenderbuffersEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glDeleteShader && true)
   {
       p_glDeleteShader = (PFNGLDELETESHADERPROC) SF_GL_RUNTIME_LINK("glDeleteShader");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDeleteShader , "Found binding: glDeleteShader\n");
   }
   #endif
   #if defined(SF_RENDER_GLES)
   if (!p_glDeleteSync && CheckGLVersion(3,0))
   {
       p_glDeleteSync = (PFNGLDELETESYNCPROC) SF_GL_RUNTIME_LINK("glDeleteSync");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDeleteSync , "Found binding: glDeleteSync\n");
   }
   #endif
   #if defined(GL_ARB_sync)
   if (!p_glDeleteSync && CheckExtension(SF_GL_ARB_sync))
   {
       p_glDeleteSync = (PFNGLDELETESYNCPROC) SF_GL_RUNTIME_LINK("glDeleteSync");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDeleteSync , "Found binding: glDeleteSync\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glDeleteSync && true)
   {
       p_glDeleteSync = (PFNGLDELETESYNCPROC) SF_GL_RUNTIME_LINK("glDeleteSync");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDeleteSync , "Found binding: glDeleteSync\n");
   }
   #endif
    #if defined(GL_APPLE_sync)
    if (!p_glDeleteSync && CheckExtension(SF_GL_APPLE_sync))
    {
        p_glDeleteSync = (PFNGLDELETESYNCPROC) SF_GL_RUNTIME_LINK("glDeleteSyncAPPLE");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDeleteSync, "Found binding: glDeleteSync (glDeleteSyncAPPLE)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glDeleteVertexArrays && true)
   {
       p_glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC) SF_GL_RUNTIME_LINK("glDeleteVertexArrays");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDeleteVertexArrays , "Found binding: glDeleteVertexArrays\n");
   }
   #endif
   #if defined(SF_RENDER_GLES)
   if (!p_glDeleteVertexArrays && CheckGLVersion(3,0))
   {
       p_glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC) SF_GL_RUNTIME_LINK("glDeleteVertexArrays");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDeleteVertexArrays , "Found binding: glDeleteVertexArrays\n");
   }
   #endif
   #if defined(GL_ARB_vertex_array_object)
   if (!p_glDeleteVertexArrays && CheckExtension(SF_GL_ARB_vertex_array_object))
   {
       p_glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC) SF_GL_RUNTIME_LINK("glDeleteVertexArrays");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDeleteVertexArrays , "Found binding: glDeleteVertexArrays\n");
   }
   #endif
    #if defined(GL_APPLE_vertex_array_object)
    if (!p_glDeleteVertexArrays && CheckExtension(SF_GL_APPLE_vertex_array_object))
    {
        p_glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC) SF_GL_RUNTIME_LINK("glDeleteVertexArraysAPPLE");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDeleteVertexArrays, "Found binding: glDeleteVertexArrays (glDeleteVertexArraysAPPLE)\n");
    }
    #endif
    #if defined(GL_OES_vertex_array_object)
    if (!p_glDeleteVertexArrays && CheckExtension(SF_GL_OES_vertex_array_object))
    {
        p_glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC) SF_GL_RUNTIME_LINK("glDeleteVertexArraysOES");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDeleteVertexArrays, "Found binding: glDeleteVertexArrays (glDeleteVertexArraysOES)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glDisableVertexAttribArray && true)
   {
       p_glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) SF_GL_RUNTIME_LINK("glDisableVertexAttribArray");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDisableVertexAttribArray , "Found binding: glDisableVertexAttribArray\n");
   }
   #endif
    #if defined(GL_ARB_vertex_program)
    if (!p_glDisableVertexAttribArray && CheckExtension(SF_GL_ARB_vertex_program))
    {
        p_glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) SF_GL_RUNTIME_LINK("glDisableVertexAttribArrayARB");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDisableVertexAttribArray, "Found binding: glDisableVertexAttribArray (glDisableVertexAttribArrayARB)\n");
    }
    #endif
   #if defined(SF_RENDER_GLES)
   if (!p_glDrawElementsInstanced && CheckGLVersion(3,0))
   {
       p_glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC) SF_GL_RUNTIME_LINK("glDrawElementsInstanced");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDrawElementsInstanced , "Found binding: glDrawElementsInstanced\n");
   }
   #endif
   #if defined(GL_ARB_instanced_arrays)
   if (!p_glDrawElementsInstanced && CheckExtension(SF_GL_ARB_instanced_arrays))
   {
       p_glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC) SF_GL_RUNTIME_LINK("glDrawElementsInstanced");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDrawElementsInstanced , "Found binding: glDrawElementsInstanced\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glDrawElementsInstanced && true)
   {
       p_glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC) SF_GL_RUNTIME_LINK("glDrawElementsInstanced");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDrawElementsInstanced , "Found binding: glDrawElementsInstanced\n");
   }
   #endif
    #if defined(GL_ARB_draw_instanced)
    if (!p_glDrawElementsInstanced && CheckExtension(SF_GL_ARB_draw_instanced))
    {
        p_glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC) SF_GL_RUNTIME_LINK("glDrawElementsInstancedARB");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDrawElementsInstanced, "Found binding: glDrawElementsInstanced (glDrawElementsInstancedARB)\n");
    }
    #endif
    #if defined(GL_EXT_draw_instanced)
    if (!p_glDrawElementsInstanced && CheckExtension(SF_GL_EXT_draw_instanced))
    {
        p_glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC) SF_GL_RUNTIME_LINK("glDrawElementsInstancedEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDrawElementsInstanced, "Found binding: glDrawElementsInstanced (glDrawElementsInstancedEXT)\n");
    }
    #endif
    #if defined(GL_NV_draw_instanced)
    if (!p_glDrawElementsInstanced && CheckExtension(SF_GL_NV_draw_instanced))
    {
        p_glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC) SF_GL_RUNTIME_LINK("glDrawElementsInstancedNV");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDrawElementsInstanced, "Found binding: glDrawElementsInstanced (glDrawElementsInstancedNV)\n");
    }
    #endif
    #if defined(GL_ANGLE_instanced_arrays)
    if (!p_glDrawElementsInstanced && CheckExtension(SF_GL_ANGLE_instanced_arrays))
    {
        p_glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC) SF_GL_RUNTIME_LINK("glDrawElementsInstancedANGLE");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glDrawElementsInstanced, "Found binding: glDrawElementsInstanced (glDrawElementsInstancedANGLE)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glEnableVertexAttribArray && true)
   {
       p_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) SF_GL_RUNTIME_LINK("glEnableVertexAttribArray");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glEnableVertexAttribArray , "Found binding: glEnableVertexAttribArray\n");
   }
   #endif
    #if defined(GL_ARB_vertex_program)
    if (!p_glEnableVertexAttribArray && CheckExtension(SF_GL_ARB_vertex_program))
    {
        p_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) SF_GL_RUNTIME_LINK("glEnableVertexAttribArrayARB");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glEnableVertexAttribArray, "Found binding: glEnableVertexAttribArray (glEnableVertexAttribArrayARB)\n");
    }
    #endif
   #if defined(SF_RENDER_GLES)
   if (!p_glFenceSync && CheckGLVersion(3,0))
   {
       p_glFenceSync = (PFNGLFENCESYNCPROC) SF_GL_RUNTIME_LINK("glFenceSync");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glFenceSync , "Found binding: glFenceSync\n");
   }
   #endif
   #if defined(GL_ARB_sync)
   if (!p_glFenceSync && CheckExtension(SF_GL_ARB_sync))
   {
       p_glFenceSync = (PFNGLFENCESYNCPROC) SF_GL_RUNTIME_LINK("glFenceSync");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glFenceSync , "Found binding: glFenceSync\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glFenceSync && true)
   {
       p_glFenceSync = (PFNGLFENCESYNCPROC) SF_GL_RUNTIME_LINK("glFenceSync");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glFenceSync , "Found binding: glFenceSync\n");
   }
   #endif
    #if defined(GL_APPLE_sync)
    if (!p_glFenceSync && CheckExtension(SF_GL_APPLE_sync))
    {
        p_glFenceSync = (PFNGLFENCESYNCPROC) SF_GL_RUNTIME_LINK("glFenceSyncAPPLE");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glFenceSync, "Found binding: glFenceSync (glFenceSyncAPPLE)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glFlushMappedBufferRange && true)
   {
       p_glFlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC) SF_GL_RUNTIME_LINK("glFlushMappedBufferRange");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glFlushMappedBufferRange , "Found binding: glFlushMappedBufferRange\n");
   }
   #endif
   #if defined(SF_RENDER_GLES)
   if (!p_glFlushMappedBufferRange && CheckGLVersion(3,0))
   {
       p_glFlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC) SF_GL_RUNTIME_LINK("glFlushMappedBufferRange");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glFlushMappedBufferRange , "Found binding: glFlushMappedBufferRange\n");
   }
   #endif
   #if defined(GL_ARB_map_buffer_range)
   if (!p_glFlushMappedBufferRange && CheckExtension(SF_GL_ARB_map_buffer_range))
   {
       p_glFlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC) SF_GL_RUNTIME_LINK("glFlushMappedBufferRange");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glFlushMappedBufferRange , "Found binding: glFlushMappedBufferRange\n");
   }
   #endif
    #if defined(GL_APPLE_flush_buffer_range)
    if (!p_glFlushMappedBufferRange && CheckExtension(SF_GL_APPLE_flush_buffer_range))
    {
        p_glFlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC) SF_GL_RUNTIME_LINK("glFlushMappedBufferRangeAPPLE");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glFlushMappedBufferRange, "Found binding: glFlushMappedBufferRange (glFlushMappedBufferRangeAPPLE)\n");
    }
    #endif
    #if defined(GL_EXT_map_buffer_range)
    if (!p_glFlushMappedBufferRange && CheckExtension(SF_GL_EXT_map_buffer_range))
    {
        p_glFlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC) SF_GL_RUNTIME_LINK("glFlushMappedBufferRangeEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glFlushMappedBufferRange, "Found binding: glFlushMappedBufferRange (glFlushMappedBufferRangeEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glFramebufferRenderbuffer && true)
   {
       p_glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) SF_GL_RUNTIME_LINK("glFramebufferRenderbuffer");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glFramebufferRenderbuffer , "Found binding: glFramebufferRenderbuffer\n");
   }
   #endif
    #if defined(GL_EXT_framebuffer_object)
    if (!p_glFramebufferRenderbuffer && CheckExtension(SF_GL_EXT_framebuffer_object))
    {
        p_glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) SF_GL_RUNTIME_LINK("glFramebufferRenderbufferEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glFramebufferRenderbuffer, "Found binding: glFramebufferRenderbuffer (glFramebufferRenderbufferEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glFramebufferTexture2D && true)
   {
       p_glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) SF_GL_RUNTIME_LINK("glFramebufferTexture2D");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glFramebufferTexture2D , "Found binding: glFramebufferTexture2D\n");
   }
   #endif
    #if defined(GL_EXT_framebuffer_object)
    if (!p_glFramebufferTexture2D && CheckExtension(SF_GL_EXT_framebuffer_object))
    {
        p_glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) SF_GL_RUNTIME_LINK("glFramebufferTexture2DEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glFramebufferTexture2D, "Found binding: glFramebufferTexture2D (glFramebufferTexture2DEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glGenBuffers && true)
   {
       p_glGenBuffers = (PFNGLGENBUFFERSPROC) SF_GL_RUNTIME_LINK("glGenBuffers");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGenBuffers , "Found binding: glGenBuffers\n");
   }
   #endif
    #if defined(GL_ARB_vertex_buffer_object)
    if (!p_glGenBuffers && CheckExtension(SF_GL_ARB_vertex_buffer_object))
    {
        p_glGenBuffers = (PFNGLGENBUFFERSPROC) SF_GL_RUNTIME_LINK("glGenBuffersARB");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGenBuffers, "Found binding: glGenBuffers (glGenBuffersARB)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glGenFramebuffers && true)
   {
       p_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) SF_GL_RUNTIME_LINK("glGenFramebuffers");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGenFramebuffers , "Found binding: glGenFramebuffers\n");
   }
   #endif
    #if defined(GL_EXT_framebuffer_object)
    if (!p_glGenFramebuffers && CheckExtension(SF_GL_EXT_framebuffer_object))
    {
        p_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) SF_GL_RUNTIME_LINK("glGenFramebuffersEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGenFramebuffers, "Found binding: glGenFramebuffers (glGenFramebuffersEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glGenProgramPipelines && true)
   {
       p_glGenProgramPipelines = (PFNGLGENPROGRAMPIPELINESPROC) SF_GL_RUNTIME_LINK("glGenProgramPipelines");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGenProgramPipelines , "Found binding: glGenProgramPipelines\n");
   }
   #endif
   #if defined(GL_ARB_separate_shader_objects)
   if (!p_glGenProgramPipelines && CheckExtension(SF_GL_ARB_separate_shader_objects))
   {
       p_glGenProgramPipelines = (PFNGLGENPROGRAMPIPELINESPROC) SF_GL_RUNTIME_LINK("glGenProgramPipelines");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGenProgramPipelines , "Found binding: glGenProgramPipelines\n");
   }
   #endif
    #if defined(GL_EXT_separate_shader_objects)
    if (!p_glGenProgramPipelines && CheckExtension(SF_GL_EXT_separate_shader_objects))
    {
        p_glGenProgramPipelines = (PFNGLGENPROGRAMPIPELINESPROC) SF_GL_RUNTIME_LINK("glGenProgramPipelinesEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGenProgramPipelines, "Found binding: glGenProgramPipelines (glGenProgramPipelinesEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glGenRenderbuffers && true)
   {
       p_glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) SF_GL_RUNTIME_LINK("glGenRenderbuffers");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGenRenderbuffers , "Found binding: glGenRenderbuffers\n");
   }
   #endif
    #if defined(GL_EXT_framebuffer_object)
    if (!p_glGenRenderbuffers && CheckExtension(SF_GL_EXT_framebuffer_object))
    {
        p_glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) SF_GL_RUNTIME_LINK("glGenRenderbuffersEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGenRenderbuffers, "Found binding: glGenRenderbuffers (glGenRenderbuffersEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glGenVertexArrays && true)
   {
       p_glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC) SF_GL_RUNTIME_LINK("glGenVertexArrays");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGenVertexArrays , "Found binding: glGenVertexArrays\n");
   }
   #endif
   #if defined(SF_RENDER_GLES)
   if (!p_glGenVertexArrays && CheckGLVersion(3,0))
   {
       p_glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC) SF_GL_RUNTIME_LINK("glGenVertexArrays");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGenVertexArrays , "Found binding: glGenVertexArrays\n");
   }
   #endif
   #if defined(GL_ARB_vertex_array_object)
   if (!p_glGenVertexArrays && CheckExtension(SF_GL_ARB_vertex_array_object))
   {
       p_glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC) SF_GL_RUNTIME_LINK("glGenVertexArrays");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGenVertexArrays , "Found binding: glGenVertexArrays\n");
   }
   #endif
    #if defined(GL_APPLE_vertex_array_object)
    if (!p_glGenVertexArrays && CheckExtension(SF_GL_APPLE_vertex_array_object))
    {
        p_glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC) SF_GL_RUNTIME_LINK("glGenVertexArraysAPPLE");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGenVertexArrays, "Found binding: glGenVertexArrays (glGenVertexArraysAPPLE)\n");
    }
    #endif
    #if defined(GL_OES_vertex_array_object)
    if (!p_glGenVertexArrays && CheckExtension(SF_GL_OES_vertex_array_object))
    {
        p_glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC) SF_GL_RUNTIME_LINK("glGenVertexArraysOES");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGenVertexArrays, "Found binding: glGenVertexArrays (glGenVertexArraysOES)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glGenerateMipmap && true)
   {
       p_glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) SF_GL_RUNTIME_LINK("glGenerateMipmap");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGenerateMipmap , "Found binding: glGenerateMipmap\n");
   }
   #endif
    #if defined(GL_EXT_framebuffer_object)
    if (!p_glGenerateMipmap && CheckExtension(SF_GL_EXT_framebuffer_object))
    {
        p_glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) SF_GL_RUNTIME_LINK("glGenerateMipmapEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGenerateMipmap, "Found binding: glGenerateMipmap (glGenerateMipmapEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glGetActiveUniform && true)
   {
       p_glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC) SF_GL_RUNTIME_LINK("glGetActiveUniform");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGetActiveUniform , "Found binding: glGetActiveUniform\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glGetFragDataLocation && true)
   {
       p_glGetFragDataLocation = (PFNGLGETFRAGDATALOCATIONPROC) SF_GL_RUNTIME_LINK("glGetFragDataLocation");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGetFragDataLocation , "Found binding: glGetFragDataLocation\n");
   }
   #endif
   #if defined(SF_RENDER_GLES)
   if (!p_glGetFragDataLocation && CheckGLVersion(3,0))
   {
       p_glGetFragDataLocation = (PFNGLGETFRAGDATALOCATIONPROC) SF_GL_RUNTIME_LINK("glGetFragDataLocation");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGetFragDataLocation , "Found binding: glGetFragDataLocation\n");
   }
   #endif
    #if defined(GL_EXT_gpu_shader4)
    if (!p_glGetFragDataLocation && CheckExtension(SF_GL_EXT_gpu_shader4))
    {
        p_glGetFragDataLocation = (PFNGLGETFRAGDATALOCATIONPROC) SF_GL_RUNTIME_LINK("glGetFragDataLocationEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGetFragDataLocation, "Found binding: glGetFragDataLocation (glGetFragDataLocationEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glGetFramebufferAttachmentParameteriv && true)
   {
       p_glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) SF_GL_RUNTIME_LINK("glGetFramebufferAttachmentParameteriv");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGetFramebufferAttachmentParameteriv , "Found binding: glGetFramebufferAttachmentParameteriv\n");
   }
   #endif
    #if defined(GL_EXT_framebuffer_object)
    if (!p_glGetFramebufferAttachmentParameteriv && CheckExtension(SF_GL_EXT_framebuffer_object))
    {
        p_glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) SF_GL_RUNTIME_LINK("glGetFramebufferAttachmentParameterivEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGetFramebufferAttachmentParameteriv, "Found binding: glGetFramebufferAttachmentParameteriv (glGetFramebufferAttachmentParameterivEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_GLES)
   if (!p_glGetProgramBinary && CheckGLVersion(3,0))
   {
       p_glGetProgramBinary = (PFNGLGETPROGRAMBINARYPROC) SF_GL_RUNTIME_LINK("glGetProgramBinary");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGetProgramBinary , "Found binding: glGetProgramBinary\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glGetProgramBinary && true)
   {
       p_glGetProgramBinary = (PFNGLGETPROGRAMBINARYPROC) SF_GL_RUNTIME_LINK("glGetProgramBinary");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGetProgramBinary , "Found binding: glGetProgramBinary\n");
   }
   #endif
   #if defined(GL_ARB_get_program_binary)
   if (!p_glGetProgramBinary && CheckExtension(SF_GL_ARB_get_program_binary))
   {
       p_glGetProgramBinary = (PFNGLGETPROGRAMBINARYPROC) SF_GL_RUNTIME_LINK("glGetProgramBinary");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGetProgramBinary , "Found binding: glGetProgramBinary\n");
   }
   #endif
    #if defined(GL_OES_get_program_binary)
    if (!p_glGetProgramBinary && CheckExtension(SF_GL_OES_get_program_binary))
    {
        p_glGetProgramBinary = (PFNGLGETPROGRAMBINARYPROC) SF_GL_RUNTIME_LINK("glGetProgramBinaryOES");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGetProgramBinary, "Found binding: glGetProgramBinary (glGetProgramBinaryOES)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glGetProgramInfoLog && true)
   {
       p_glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) SF_GL_RUNTIME_LINK("glGetProgramInfoLog");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGetProgramInfoLog , "Found binding: glGetProgramInfoLog\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glGetProgramiv && true)
   {
       p_glGetProgramiv = (PFNGLGETPROGRAMIVPROC) SF_GL_RUNTIME_LINK("glGetProgramiv");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGetProgramiv , "Found binding: glGetProgramiv\n");
   }
   #endif
    #if defined(GL_ARB_fragment_program)
    if (!p_glGetProgramiv && CheckExtension(SF_GL_ARB_fragment_program))
    {
        p_glGetProgramiv = (PFNGLGETPROGRAMIVPROC) SF_GL_RUNTIME_LINK("glGetProgramivARB");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGetProgramiv, "Found binding: glGetProgramiv (glGetProgramivARB)\n");
    }
    #endif
    #if defined(GL_NV_vertex_program)
    if (!p_glGetProgramiv && CheckExtension(SF_GL_NV_vertex_program))
    {
        p_glGetProgramiv = (PFNGLGETPROGRAMIVPROC) SF_GL_RUNTIME_LINK("glGetProgramivNV");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGetProgramiv, "Found binding: glGetProgramiv (glGetProgramivNV)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glGetRenderbufferParameteriv && true)
   {
       p_glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC) SF_GL_RUNTIME_LINK("glGetRenderbufferParameteriv");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGetRenderbufferParameteriv , "Found binding: glGetRenderbufferParameteriv\n");
   }
   #endif
    #if defined(GL_EXT_framebuffer_object)
    if (!p_glGetRenderbufferParameteriv && CheckExtension(SF_GL_EXT_framebuffer_object))
    {
        p_glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC) SF_GL_RUNTIME_LINK("glGetRenderbufferParameterivEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGetRenderbufferParameteriv, "Found binding: glGetRenderbufferParameteriv (glGetRenderbufferParameterivEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glGetShaderInfoLog && true)
   {
       p_glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) SF_GL_RUNTIME_LINK("glGetShaderInfoLog");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGetShaderInfoLog , "Found binding: glGetShaderInfoLog\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glGetShaderiv && true)
   {
       p_glGetShaderiv = (PFNGLGETSHADERIVPROC) SF_GL_RUNTIME_LINK("glGetShaderiv");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGetShaderiv , "Found binding: glGetShaderiv\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glGetStringi && true)
   {
       p_glGetStringi = (PFNGLGETSTRINGIPROC) SF_GL_RUNTIME_LINK("glGetStringi");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGetStringi , "Found binding: glGetStringi\n");
   }
   #endif
   #if defined(SF_RENDER_GLES)
   if (!p_glGetStringi && CheckGLVersion(3,0))
   {
       p_glGetStringi = (PFNGLGETSTRINGIPROC) SF_GL_RUNTIME_LINK("glGetStringi");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGetStringi , "Found binding: glGetStringi\n");
   }
   #endif
   #if defined(SF_RENDER_GLES)
   if (!p_glGetSynciv && CheckGLVersion(3,0))
   {
       p_glGetSynciv = (PFNGLGETSYNCIVPROC) SF_GL_RUNTIME_LINK("glGetSynciv");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGetSynciv , "Found binding: glGetSynciv\n");
   }
   #endif
   #if defined(GL_ARB_sync)
   if (!p_glGetSynciv && CheckExtension(SF_GL_ARB_sync))
   {
       p_glGetSynciv = (PFNGLGETSYNCIVPROC) SF_GL_RUNTIME_LINK("glGetSynciv");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGetSynciv , "Found binding: glGetSynciv\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glGetSynciv && true)
   {
       p_glGetSynciv = (PFNGLGETSYNCIVPROC) SF_GL_RUNTIME_LINK("glGetSynciv");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGetSynciv , "Found binding: glGetSynciv\n");
   }
   #endif
    #if defined(GL_APPLE_sync)
    if (!p_glGetSynciv && CheckExtension(SF_GL_APPLE_sync))
    {
        p_glGetSynciv = (PFNGLGETSYNCIVPROC) SF_GL_RUNTIME_LINK("glGetSyncivAPPLE");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGetSynciv, "Found binding: glGetSynciv (glGetSyncivAPPLE)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glGetUniformLocation && true)
   {
       p_glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) SF_GL_RUNTIME_LINK("glGetUniformLocation");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glGetUniformLocation , "Found binding: glGetUniformLocation\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glIsFramebuffer && true)
   {
       p_glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC) SF_GL_RUNTIME_LINK("glIsFramebuffer");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glIsFramebuffer , "Found binding: glIsFramebuffer\n");
   }
   #endif
    #if defined(GL_EXT_framebuffer_object)
    if (!p_glIsFramebuffer && CheckExtension(SF_GL_EXT_framebuffer_object))
    {
        p_glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC) SF_GL_RUNTIME_LINK("glIsFramebufferEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glIsFramebuffer, "Found binding: glIsFramebuffer (glIsFramebufferEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glIsProgram && true)
   {
       p_glIsProgram = (PFNGLISPROGRAMPROC) SF_GL_RUNTIME_LINK("glIsProgram");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glIsProgram , "Found binding: glIsProgram\n");
   }
   #endif
    #if defined(GL_ARB_fragment_program)
    if (!p_glIsProgram && CheckExtension(SF_GL_ARB_fragment_program))
    {
        p_glIsProgram = (PFNGLISPROGRAMPROC) SF_GL_RUNTIME_LINK("glIsProgramARB");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glIsProgram, "Found binding: glIsProgram (glIsProgramARB)\n");
    }
    #endif
    #if defined(GL_NV_vertex_program)
    if (!p_glIsProgram && CheckExtension(SF_GL_NV_vertex_program))
    {
        p_glIsProgram = (PFNGLISPROGRAMPROC) SF_GL_RUNTIME_LINK("glIsProgramNV");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glIsProgram, "Found binding: glIsProgram (glIsProgramNV)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glIsRenderbuffer && true)
   {
       p_glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC) SF_GL_RUNTIME_LINK("glIsRenderbuffer");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glIsRenderbuffer , "Found binding: glIsRenderbuffer\n");
   }
   #endif
    #if defined(GL_EXT_framebuffer_object)
    if (!p_glIsRenderbuffer && CheckExtension(SF_GL_EXT_framebuffer_object))
    {
        p_glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC) SF_GL_RUNTIME_LINK("glIsRenderbufferEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glIsRenderbuffer, "Found binding: glIsRenderbuffer (glIsRenderbufferEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glLinkProgram && true)
   {
       p_glLinkProgram = (PFNGLLINKPROGRAMPROC) SF_GL_RUNTIME_LINK("glLinkProgram");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glLinkProgram , "Found binding: glLinkProgram\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glMapBuffer && true)
   {
       p_glMapBuffer = (PFNGLMAPBUFFERPROC) SF_GL_RUNTIME_LINK("glMapBuffer");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glMapBuffer , "Found binding: glMapBuffer\n");
   }
   #endif
    #if defined(GL_ARB_vertex_buffer_object)
    if (!p_glMapBuffer && CheckExtension(SF_GL_ARB_vertex_buffer_object))
    {
        p_glMapBuffer = (PFNGLMAPBUFFERPROC) SF_GL_RUNTIME_LINK("glMapBufferARB");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glMapBuffer, "Found binding: glMapBuffer (glMapBufferARB)\n");
    }
    #endif
    #if defined(GL_OES_mapbuffer)
    if (!p_glMapBuffer && CheckExtension(SF_GL_OES_mapbuffer))
    {
        p_glMapBuffer = (PFNGLMAPBUFFERPROC) SF_GL_RUNTIME_LINK("glMapBufferOES");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glMapBuffer, "Found binding: glMapBuffer (glMapBufferOES)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glMapBufferRange && true)
   {
       p_glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC) SF_GL_RUNTIME_LINK("glMapBufferRange");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glMapBufferRange , "Found binding: glMapBufferRange\n");
   }
   #endif
   #if defined(SF_RENDER_GLES)
   if (!p_glMapBufferRange && CheckGLVersion(3,0))
   {
       p_glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC) SF_GL_RUNTIME_LINK("glMapBufferRange");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glMapBufferRange , "Found binding: glMapBufferRange\n");
   }
   #endif
   #if defined(GL_ARB_map_buffer_range)
   if (!p_glMapBufferRange && CheckExtension(SF_GL_ARB_map_buffer_range))
   {
       p_glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC) SF_GL_RUNTIME_LINK("glMapBufferRange");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glMapBufferRange , "Found binding: glMapBufferRange\n");
   }
   #endif
    #if defined(GL_EXT_map_buffer_range)
    if (!p_glMapBufferRange && CheckExtension(SF_GL_EXT_map_buffer_range))
    {
        p_glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC) SF_GL_RUNTIME_LINK("glMapBufferRangeEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glMapBufferRange, "Found binding: glMapBufferRange (glMapBufferRangeEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_GLES)
   if (!p_glProgramBinary && CheckGLVersion(3,0))
   {
       p_glProgramBinary = (PFNGLPROGRAMBINARYPROC) SF_GL_RUNTIME_LINK("glProgramBinary");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramBinary , "Found binding: glProgramBinary\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glProgramBinary && true)
   {
       p_glProgramBinary = (PFNGLPROGRAMBINARYPROC) SF_GL_RUNTIME_LINK("glProgramBinary");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramBinary , "Found binding: glProgramBinary\n");
   }
   #endif
   #if defined(GL_ARB_get_program_binary)
   if (!p_glProgramBinary && CheckExtension(SF_GL_ARB_get_program_binary))
   {
       p_glProgramBinary = (PFNGLPROGRAMBINARYPROC) SF_GL_RUNTIME_LINK("glProgramBinary");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramBinary , "Found binding: glProgramBinary\n");
   }
   #endif
    #if defined(GL_OES_get_program_binary)
    if (!p_glProgramBinary && CheckExtension(SF_GL_OES_get_program_binary))
    {
        p_glProgramBinary = (PFNGLPROGRAMBINARYPROC) SF_GL_RUNTIME_LINK("glProgramBinaryOES");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramBinary, "Found binding: glProgramBinary (glProgramBinaryOES)\n");
    }
    #endif
   #if defined(SF_RENDER_GLES)
   if (!p_glProgramParameteri && CheckGLVersion(3,0))
   {
       p_glProgramParameteri = (PFNGLPROGRAMPARAMETERIPROC) SF_GL_RUNTIME_LINK("glProgramParameteri");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramParameteri , "Found binding: glProgramParameteri\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glProgramParameteri && true)
   {
       p_glProgramParameteri = (PFNGLPROGRAMPARAMETERIPROC) SF_GL_RUNTIME_LINK("glProgramParameteri");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramParameteri , "Found binding: glProgramParameteri\n");
   }
   #endif
   #if defined(GL_ARB_separate_shader_objects)
   if (!p_glProgramParameteri && CheckExtension(SF_GL_ARB_separate_shader_objects))
   {
       p_glProgramParameteri = (PFNGLPROGRAMPARAMETERIPROC) SF_GL_RUNTIME_LINK("glProgramParameteri");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramParameteri , "Found binding: glProgramParameteri\n");
   }
   #endif
    #if defined(GL_ARB_geometry_shader4)
    if (!p_glProgramParameteri && CheckExtension(SF_GL_ARB_geometry_shader4))
    {
        p_glProgramParameteri = (PFNGLPROGRAMPARAMETERIPROC) SF_GL_RUNTIME_LINK("glProgramParameteriARB");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramParameteri, "Found binding: glProgramParameteri (glProgramParameteriARB)\n");
    }
    #endif
    #if defined(GL_EXT_geometry_shader4)
    if (!p_glProgramParameteri && CheckExtension(SF_GL_EXT_geometry_shader4))
    {
        p_glProgramParameteri = (PFNGLPROGRAMPARAMETERIPROC) SF_GL_RUNTIME_LINK("glProgramParameteriEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramParameteri, "Found binding: glProgramParameteri (glProgramParameteriEXT)\n");
    }
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    if (!p_glProgramParameteri && CheckExtension(SF_GL_EXT_separate_shader_objects))
    {
        p_glProgramParameteri = (PFNGLPROGRAMPARAMETERIPROC) SF_GL_RUNTIME_LINK("glProgramParameteriEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramParameteri, "Found binding: glProgramParameteri (glProgramParameteriEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glProgramUniform1fv && true)
   {
       p_glProgramUniform1fv = (PFNGLPROGRAMUNIFORM1FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform1fv");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramUniform1fv , "Found binding: glProgramUniform1fv\n");
   }
   #endif
   #if defined(GL_ARB_separate_shader_objects)
   if (!p_glProgramUniform1fv && CheckExtension(SF_GL_ARB_separate_shader_objects))
   {
       p_glProgramUniform1fv = (PFNGLPROGRAMUNIFORM1FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform1fv");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramUniform1fv , "Found binding: glProgramUniform1fv\n");
   }
   #endif
    #if defined(GL_EXT_direct_state_access)
    if (!p_glProgramUniform1fv && CheckExtension(SF_GL_EXT_direct_state_access))
    {
        p_glProgramUniform1fv = (PFNGLPROGRAMUNIFORM1FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform1fvEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramUniform1fv, "Found binding: glProgramUniform1fv (glProgramUniform1fvEXT)\n");
    }
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    if (!p_glProgramUniform1fv && CheckExtension(SF_GL_EXT_separate_shader_objects))
    {
        p_glProgramUniform1fv = (PFNGLPROGRAMUNIFORM1FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform1fvEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramUniform1fv, "Found binding: glProgramUniform1fv (glProgramUniform1fvEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glProgramUniform1iv && true)
   {
       p_glProgramUniform1iv = (PFNGLPROGRAMUNIFORM1IVPROC) SF_GL_RUNTIME_LINK("glProgramUniform1iv");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramUniform1iv , "Found binding: glProgramUniform1iv\n");
   }
   #endif
   #if defined(GL_ARB_separate_shader_objects)
   if (!p_glProgramUniform1iv && CheckExtension(SF_GL_ARB_separate_shader_objects))
   {
       p_glProgramUniform1iv = (PFNGLPROGRAMUNIFORM1IVPROC) SF_GL_RUNTIME_LINK("glProgramUniform1iv");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramUniform1iv , "Found binding: glProgramUniform1iv\n");
   }
   #endif
    #if defined(GL_EXT_direct_state_access)
    if (!p_glProgramUniform1iv && CheckExtension(SF_GL_EXT_direct_state_access))
    {
        p_glProgramUniform1iv = (PFNGLPROGRAMUNIFORM1IVPROC) SF_GL_RUNTIME_LINK("glProgramUniform1ivEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramUniform1iv, "Found binding: glProgramUniform1iv (glProgramUniform1ivEXT)\n");
    }
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    if (!p_glProgramUniform1iv && CheckExtension(SF_GL_EXT_separate_shader_objects))
    {
        p_glProgramUniform1iv = (PFNGLPROGRAMUNIFORM1IVPROC) SF_GL_RUNTIME_LINK("glProgramUniform1ivEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramUniform1iv, "Found binding: glProgramUniform1iv (glProgramUniform1ivEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glProgramUniform2fv && true)
   {
       p_glProgramUniform2fv = (PFNGLPROGRAMUNIFORM2FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform2fv");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramUniform2fv , "Found binding: glProgramUniform2fv\n");
   }
   #endif
   #if defined(GL_ARB_separate_shader_objects)
   if (!p_glProgramUniform2fv && CheckExtension(SF_GL_ARB_separate_shader_objects))
   {
       p_glProgramUniform2fv = (PFNGLPROGRAMUNIFORM2FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform2fv");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramUniform2fv , "Found binding: glProgramUniform2fv\n");
   }
   #endif
    #if defined(GL_EXT_direct_state_access)
    if (!p_glProgramUniform2fv && CheckExtension(SF_GL_EXT_direct_state_access))
    {
        p_glProgramUniform2fv = (PFNGLPROGRAMUNIFORM2FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform2fvEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramUniform2fv, "Found binding: glProgramUniform2fv (glProgramUniform2fvEXT)\n");
    }
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    if (!p_glProgramUniform2fv && CheckExtension(SF_GL_EXT_separate_shader_objects))
    {
        p_glProgramUniform2fv = (PFNGLPROGRAMUNIFORM2FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform2fvEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramUniform2fv, "Found binding: glProgramUniform2fv (glProgramUniform2fvEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glProgramUniform3fv && true)
   {
       p_glProgramUniform3fv = (PFNGLPROGRAMUNIFORM3FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform3fv");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramUniform3fv , "Found binding: glProgramUniform3fv\n");
   }
   #endif
   #if defined(GL_ARB_separate_shader_objects)
   if (!p_glProgramUniform3fv && CheckExtension(SF_GL_ARB_separate_shader_objects))
   {
       p_glProgramUniform3fv = (PFNGLPROGRAMUNIFORM3FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform3fv");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramUniform3fv , "Found binding: glProgramUniform3fv\n");
   }
   #endif
    #if defined(GL_EXT_direct_state_access)
    if (!p_glProgramUniform3fv && CheckExtension(SF_GL_EXT_direct_state_access))
    {
        p_glProgramUniform3fv = (PFNGLPROGRAMUNIFORM3FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform3fvEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramUniform3fv, "Found binding: glProgramUniform3fv (glProgramUniform3fvEXT)\n");
    }
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    if (!p_glProgramUniform3fv && CheckExtension(SF_GL_EXT_separate_shader_objects))
    {
        p_glProgramUniform3fv = (PFNGLPROGRAMUNIFORM3FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform3fvEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramUniform3fv, "Found binding: glProgramUniform3fv (glProgramUniform3fvEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glProgramUniform4fv && true)
   {
       p_glProgramUniform4fv = (PFNGLPROGRAMUNIFORM4FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform4fv");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramUniform4fv , "Found binding: glProgramUniform4fv\n");
   }
   #endif
   #if defined(GL_ARB_separate_shader_objects)
   if (!p_glProgramUniform4fv && CheckExtension(SF_GL_ARB_separate_shader_objects))
   {
       p_glProgramUniform4fv = (PFNGLPROGRAMUNIFORM4FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform4fv");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramUniform4fv , "Found binding: glProgramUniform4fv\n");
   }
   #endif
    #if defined(GL_EXT_direct_state_access)
    if (!p_glProgramUniform4fv && CheckExtension(SF_GL_EXT_direct_state_access))
    {
        p_glProgramUniform4fv = (PFNGLPROGRAMUNIFORM4FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform4fvEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramUniform4fv, "Found binding: glProgramUniform4fv (glProgramUniform4fvEXT)\n");
    }
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    if (!p_glProgramUniform4fv && CheckExtension(SF_GL_EXT_separate_shader_objects))
    {
        p_glProgramUniform4fv = (PFNGLPROGRAMUNIFORM4FVPROC) SF_GL_RUNTIME_LINK("glProgramUniform4fvEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramUniform4fv, "Found binding: glProgramUniform4fv (glProgramUniform4fvEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glProgramUniformMatrix4fv && true)
   {
       p_glProgramUniformMatrix4fv = (PFNGLPROGRAMUNIFORMMATRIX4FVPROC) SF_GL_RUNTIME_LINK("glProgramUniformMatrix4fv");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramUniformMatrix4fv , "Found binding: glProgramUniformMatrix4fv\n");
   }
   #endif
   #if defined(GL_ARB_separate_shader_objects)
   if (!p_glProgramUniformMatrix4fv && CheckExtension(SF_GL_ARB_separate_shader_objects))
   {
       p_glProgramUniformMatrix4fv = (PFNGLPROGRAMUNIFORMMATRIX4FVPROC) SF_GL_RUNTIME_LINK("glProgramUniformMatrix4fv");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramUniformMatrix4fv , "Found binding: glProgramUniformMatrix4fv\n");
   }
   #endif
    #if defined(GL_EXT_direct_state_access)
    if (!p_glProgramUniformMatrix4fv && CheckExtension(SF_GL_EXT_direct_state_access))
    {
        p_glProgramUniformMatrix4fv = (PFNGLPROGRAMUNIFORMMATRIX4FVPROC) SF_GL_RUNTIME_LINK("glProgramUniformMatrix4fvEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramUniformMatrix4fv, "Found binding: glProgramUniformMatrix4fv (glProgramUniformMatrix4fvEXT)\n");
    }
    #endif
    #if defined(GL_EXT_separate_shader_objects)
    if (!p_glProgramUniformMatrix4fv && CheckExtension(SF_GL_EXT_separate_shader_objects))
    {
        p_glProgramUniformMatrix4fv = (PFNGLPROGRAMUNIFORMMATRIX4FVPROC) SF_GL_RUNTIME_LINK("glProgramUniformMatrix4fvEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glProgramUniformMatrix4fv, "Found binding: glProgramUniformMatrix4fv (glProgramUniformMatrix4fvEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glReleaseShaderCompiler && true)
   {
       p_glReleaseShaderCompiler = (PFNGLRELEASESHADERCOMPILERPROC) SF_GL_RUNTIME_LINK("glReleaseShaderCompiler");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glReleaseShaderCompiler , "Found binding: glReleaseShaderCompiler\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glRenderbufferStorage && true)
   {
       p_glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) SF_GL_RUNTIME_LINK("glRenderbufferStorage");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glRenderbufferStorage , "Found binding: glRenderbufferStorage\n");
   }
   #endif
    #if defined(GL_EXT_framebuffer_object)
    if (!p_glRenderbufferStorage && CheckExtension(SF_GL_EXT_framebuffer_object))
    {
        p_glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) SF_GL_RUNTIME_LINK("glRenderbufferStorageEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glRenderbufferStorage, "Found binding: glRenderbufferStorage (glRenderbufferStorageEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glShaderSource && true)
   {
       p_glShaderSource = (PFNGLSHADERSOURCEPROC) SF_GL_RUNTIME_LINK("glShaderSource");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glShaderSource , "Found binding: glShaderSource\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glUniform1f && true)
   {
       p_glUniform1f = (PFNGLUNIFORM1FPROC) SF_GL_RUNTIME_LINK("glUniform1f");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glUniform1f , "Found binding: glUniform1f\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glUniform1fv && true)
   {
       p_glUniform1fv = (PFNGLUNIFORM1FVPROC) SF_GL_RUNTIME_LINK("glUniform1fv");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glUniform1fv , "Found binding: glUniform1fv\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glUniform1i && true)
   {
       p_glUniform1i = (PFNGLUNIFORM1IPROC) SF_GL_RUNTIME_LINK("glUniform1i");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glUniform1i , "Found binding: glUniform1i\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glUniform1iv && true)
   {
       p_glUniform1iv = (PFNGLUNIFORM1IVPROC) SF_GL_RUNTIME_LINK("glUniform1iv");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glUniform1iv , "Found binding: glUniform1iv\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glUniform2f && true)
   {
       p_glUniform2f = (PFNGLUNIFORM2FPROC) SF_GL_RUNTIME_LINK("glUniform2f");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glUniform2f , "Found binding: glUniform2f\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glUniform2fv && true)
   {
       p_glUniform2fv = (PFNGLUNIFORM2FVPROC) SF_GL_RUNTIME_LINK("glUniform2fv");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glUniform2fv , "Found binding: glUniform2fv\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glUniform3fv && true)
   {
       p_glUniform3fv = (PFNGLUNIFORM3FVPROC) SF_GL_RUNTIME_LINK("glUniform3fv");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glUniform3fv , "Found binding: glUniform3fv\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glUniform4fv && true)
   {
       p_glUniform4fv = (PFNGLUNIFORM4FVPROC) SF_GL_RUNTIME_LINK("glUniform4fv");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glUniform4fv , "Found binding: glUniform4fv\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glUniformMatrix4fv && true)
   {
       p_glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) SF_GL_RUNTIME_LINK("glUniformMatrix4fv");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glUniformMatrix4fv , "Found binding: glUniformMatrix4fv\n");
   }
   #endif
   #if defined(SF_RENDER_GLES)
   if (!p_glUnmapBuffer && CheckGLVersion(3,0))
   {
       p_glUnmapBuffer = (PFNGLUNMAPBUFFERPROC) SF_GL_RUNTIME_LINK("glUnmapBuffer");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glUnmapBuffer , "Found binding: glUnmapBuffer\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glUnmapBuffer && true)
   {
       p_glUnmapBuffer = (PFNGLUNMAPBUFFERPROC) SF_GL_RUNTIME_LINK("glUnmapBuffer");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glUnmapBuffer , "Found binding: glUnmapBuffer\n");
   }
   #endif
    #if defined(GL_ARB_vertex_buffer_object)
    if (!p_glUnmapBuffer && CheckExtension(SF_GL_ARB_vertex_buffer_object))
    {
        p_glUnmapBuffer = (PFNGLUNMAPBUFFERPROC) SF_GL_RUNTIME_LINK("glUnmapBufferARB");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glUnmapBuffer, "Found binding: glUnmapBuffer (glUnmapBufferARB)\n");
    }
    #endif
    #if defined(GL_OES_mapbuffer)
    if (!p_glUnmapBuffer && CheckExtension(SF_GL_OES_mapbuffer))
    {
        p_glUnmapBuffer = (PFNGLUNMAPBUFFERPROC) SF_GL_RUNTIME_LINK("glUnmapBufferOES");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glUnmapBuffer, "Found binding: glUnmapBuffer (glUnmapBufferOES)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glUseProgram && true)
   {
       p_glUseProgram = (PFNGLUSEPROGRAMPROC) SF_GL_RUNTIME_LINK("glUseProgram");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glUseProgram , "Found binding: glUseProgram\n");
   }
   #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glUseProgramStages && true)
   {
       p_glUseProgramStages = (PFNGLUSEPROGRAMSTAGESPROC) SF_GL_RUNTIME_LINK("glUseProgramStages");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glUseProgramStages , "Found binding: glUseProgramStages\n");
   }
   #endif
   #if defined(GL_ARB_separate_shader_objects)
   if (!p_glUseProgramStages && CheckExtension(SF_GL_ARB_separate_shader_objects))
   {
       p_glUseProgramStages = (PFNGLUSEPROGRAMSTAGESPROC) SF_GL_RUNTIME_LINK("glUseProgramStages");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glUseProgramStages , "Found binding: glUseProgramStages\n");
   }
   #endif
    #if defined(GL_EXT_separate_shader_objects)
    if (!p_glUseProgramStages && CheckExtension(SF_GL_EXT_separate_shader_objects))
    {
        p_glUseProgramStages = (PFNGLUSEPROGRAMSTAGESPROC) SF_GL_RUNTIME_LINK("glUseProgramStagesEXT");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glUseProgramStages, "Found binding: glUseProgramStages (glUseProgramStagesEXT)\n");
    }
    #endif
   #if defined(SF_RENDER_OPENGL)
   if (!p_glVertexAttribPointer && true)
   {
       p_glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) SF_GL_RUNTIME_LINK("glVertexAttribPointer");
       SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glVertexAttribPointer , "Found binding: glVertexAttribPointer\n");
   }
   #endif
    #if defined(GL_ARB_vertex_program)
    if (!p_glVertexAttribPointer && CheckExtension(SF_GL_ARB_vertex_program))
    {
        p_glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) SF_GL_RUNTIME_LINK("glVertexAttribPointerARB");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glVertexAttribPointer, "Found binding: glVertexAttribPointer (glVertexAttribPointerARB)\n");
    }
    #endif
    #if defined(GL_NV_vertex_program)
    if (!p_glVertexAttribPointer && CheckExtension(SF_GL_NV_vertex_program))
    {
        p_glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) SF_GL_RUNTIME_LINK("glVertexAttribPointerNV");
        SF_DEBUG_MESSAGE(verbosity >= Verbosity_Info && p_glVertexAttribPointer, "Found binding: glVertexAttribPointer (glVertexAttribPointerNV)\n");
    }
    #endif

#endif // SF_GL_RUNTIME_LINK
}
void GLImmediate::glActiveTexture(GLenum  a0)
{
    ScopedGLErrorCheck check("glActiveTexture");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%x)\n", "glActiveTexture" ,  a0);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glActiveTexture, "Attempt to call runtime link GL function: glActiveTexture, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glActiveTexture) return;
     p_glActiveTexture(a0);
#else
#if  defined(GL_ARB_multitexture)
     ::glActiveTextureARB(a0);
#elif  defined(GL_ES_VERSION_2_0)
     ::glActiveTexture(a0);
#elif  defined(GL_VERSION_1_3)
     ::glActiveTexture(a0);
#else
    SF_UNUSED1(a0);
    SF_DEBUG_ASSERT(0, "Calling function glActiveTexture, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glAttachShader(GLuint  a0, GLuint  a1)
{
    ScopedGLErrorCheck check("glAttachShader");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d)\n", "glAttachShader" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glAttachShader, "Attempt to call runtime link GL function: glAttachShader, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glAttachShader) return;
     p_glAttachShader(a0, a1);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glAttachShader(a0, a1);
#elif  defined(GL_VERSION_2_0)
     ::glAttachShader(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glAttachShader, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glBindAttribLocation(GLuint  a0, GLuint  a1, const GLchar * a2)
{
    ScopedGLErrorCheck check("glBindAttribLocation");
    SF_DEBUG_MESSAGE4(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d, %p)\n", "glBindAttribLocation" ,  a0, a1, a2);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glBindAttribLocation, "Attempt to call runtime link GL function: glBindAttribLocation, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glBindAttribLocation) return;
     p_glBindAttribLocation(a0, a1, a2);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glBindAttribLocation(a0, a1, a2);
#elif  defined(GL_VERSION_2_0)
     ::glBindAttribLocation(a0, a1, a2);
#else
    SF_UNUSED3(a0, a1, a2);
    SF_DEBUG_ASSERT(0, "Calling function glBindAttribLocation, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glBindBuffer(GLenum  a0, GLuint  a1)
{
    ScopedGLErrorCheck check("glBindBuffer");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%x, %d)\n", "glBindBuffer" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glBindBuffer, "Attempt to call runtime link GL function: glBindBuffer, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glBindBuffer) return;
     p_glBindBuffer(a0, a1);
#else
#if  defined(GL_ARB_vertex_buffer_object)
     ::glBindBufferARB(a0, a1);
#elif  defined(GL_ES_VERSION_2_0)
     ::glBindBuffer(a0, a1);
#elif  defined(GL_VERSION_1_5)
     ::glBindBuffer(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glBindBuffer, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

 void GLImmediate::glBindFragDataLocation(GLuint  a0, GLuint  a1, const GLchar * a2)
{
    ScopedGLErrorCheck check("glBindFragDataLocation");
    SF_DEBUG_MESSAGE4(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d, %p)\n", "glBindFragDataLocation" ,  a0, a1, a2);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glBindFragDataLocation, "Attempt to call runtime link GL function: glBindFragDataLocation, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glBindFragDataLocation) return ( void)(0);
    return p_glBindFragDataLocation(a0, a1, a2);
#else
#if  defined(GL_VERSION_3_0)
    return ::glBindFragDataLocation(a0, a1, a2);
#elif  defined(GL_EXT_gpu_shader4)
    return ::glBindFragDataLocationEXT(a0, a1, a2);
#else
    SF_UNUSED3(a0, a1, a2);
    SF_DEBUG_ASSERT(0, "Calling function glBindFragDataLocation, but no version of extension supports it.");
    return ( void)(0);
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glBindFramebuffer(GLenum  a0, GLuint  a1)
{
    ScopedGLErrorCheck check("glBindFramebuffer");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%x, %d)\n", "glBindFramebuffer" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glBindFramebuffer, "Attempt to call runtime link GL function: glBindFramebuffer, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glBindFramebuffer) return;
     p_glBindFramebuffer(a0, a1);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glBindFramebuffer(a0, a1);
#elif  defined(GL_VERSION_3_0)
     ::glBindFramebuffer(a0, a1);
#elif  defined(GL_EXT_framebuffer_object)
     ::glBindFramebufferEXT(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glBindFramebuffer, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glBindProgramPipeline(GLuint  a0)
{
    ScopedGLErrorCheck check("glBindProgramPipeline");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%d)\n", "glBindProgramPipeline" ,  a0);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glBindProgramPipeline, "Attempt to call runtime link GL function: glBindProgramPipeline, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glBindProgramPipeline) return;
     p_glBindProgramPipeline(a0);
#else
#if  defined(GL_VERSION_4_1)
     ::glBindProgramPipeline(a0);
#elif  defined(GL_ARB_separate_shader_objects)
     ::glBindProgramPipeline(a0);
#elif  defined(GL_EXT_separate_shader_objects)
     ::glBindProgramPipelineEXT(a0);
#else
    SF_UNUSED1(a0);
    SF_DEBUG_ASSERT(0, "Calling function glBindProgramPipeline, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glBindRenderbuffer(GLenum  a0, GLuint  a1)
{
    ScopedGLErrorCheck check("glBindRenderbuffer");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%x, %d)\n", "glBindRenderbuffer" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glBindRenderbuffer, "Attempt to call runtime link GL function: glBindRenderbuffer, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glBindRenderbuffer) return;
     p_glBindRenderbuffer(a0, a1);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glBindRenderbuffer(a0, a1);
#elif  defined(GL_VERSION_3_0)
     ::glBindRenderbuffer(a0, a1);
#elif  defined(GL_EXT_framebuffer_object)
     ::glBindRenderbufferEXT(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glBindRenderbuffer, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glBindTexture(GLenum  a0, GLuint  a1)
{
    ScopedGLErrorCheck check("glBindTexture");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%x, %d)\n", "glBindTexture" ,  a0, a1);
     ::glBindTexture(a0, a1);
}
void GLImmediate::glBindVertexArray(GLuint  a0)
{
    ScopedGLErrorCheck check("glBindVertexArray");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%d)\n", "glBindVertexArray" ,  a0);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glBindVertexArray, "Attempt to call runtime link GL function: glBindVertexArray, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glBindVertexArray) return;
     p_glBindVertexArray(a0);
#else
#if  defined(GL_VERSION_3_0)
     ::glBindVertexArray(a0);
#elif  defined(GL_ES_VERSION_3_0)
     ::glBindVertexArray(a0);
#elif  defined(GL_ARB_vertex_array_object)
     ::glBindVertexArray(a0);
#elif  defined(GL_APPLE_vertex_array_object)
     ::glBindVertexArrayAPPLE(a0);
#elif  defined(GL_OES_vertex_array_object)
     ::glBindVertexArrayOES(a0);
#else
    SF_UNUSED1(a0);
    SF_DEBUG_ASSERT(0, "Calling function glBindVertexArray, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glBlendEquation(GLenum  a0)
{
    ScopedGLErrorCheck check("glBlendEquation");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%x)\n", "glBlendEquation" ,  a0);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glBlendEquation, "Attempt to call runtime link GL function: glBlendEquation, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glBlendEquation) return;
     p_glBlendEquation(a0);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glBlendEquation(a0);
#elif  defined(GL_VERSION_1_4)
     ::glBlendEquation(a0);
#else
    SF_UNUSED1(a0);
    SF_DEBUG_ASSERT(0, "Calling function glBlendEquation, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glBlendEquationSeparate(GLenum  a0, GLenum  a1)
{
    ScopedGLErrorCheck check("glBlendEquationSeparate");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%x, %x)\n", "glBlendEquationSeparate" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glBlendEquationSeparate, "Attempt to call runtime link GL function: glBlendEquationSeparate, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glBlendEquationSeparate) return;
     p_glBlendEquationSeparate(a0, a1);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glBlendEquationSeparate(a0, a1);
#elif  defined(GL_VERSION_2_0)
     ::glBlendEquationSeparate(a0, a1);
#elif  defined(GL_EXT_blend_equation_separate)
     ::glBlendEquationSeparateEXT(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glBlendEquationSeparate, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glBlendFunc(GLenum  a0, GLenum  a1)
{
    ScopedGLErrorCheck check("glBlendFunc");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%x, %x)\n", "glBlendFunc" ,  a0, a1);
     ::glBlendFunc(a0, a1);
}
void GLImmediate::glBlendFuncSeparate(GLenum  a0, GLenum  a1, GLenum  a2, GLenum  a3)
{
    ScopedGLErrorCheck check("glBlendFuncSeparate");
    SF_DEBUG_MESSAGE5(Verbosity >= Verbosity_DumpCalls, "%s(%x, %x, %x, %x)\n", "glBlendFuncSeparate" ,  a0, a1, a2, a3);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glBlendFuncSeparate, "Attempt to call runtime link GL function: glBlendFuncSeparate, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glBlendFuncSeparate) return;
     p_glBlendFuncSeparate(a0, a1, a2, a3);
#else
#if  defined(GL_INGR_blend_func_separate)
     ::glBlendFuncSeparateINGR(a0, a1, a2, a3);
#elif  defined(GL_ES_VERSION_2_0)
     ::glBlendFuncSeparate(a0, a1, a2, a3);
#elif  defined(GL_VERSION_1_4)
     ::glBlendFuncSeparate(a0, a1, a2, a3);
#elif  defined(GL_EXT_blend_func_separate)
     ::glBlendFuncSeparateEXT(a0, a1, a2, a3);
#else
    SF_UNUSED4(a0, a1, a2, a3);
    SF_DEBUG_ASSERT(0, "Calling function glBlendFuncSeparate, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glBufferData(GLenum  a0, GLsizeiptr  a1, const void * a2, GLenum  a3)
{
    ScopedGLErrorCheck check("glBufferData");
    SF_DEBUG_MESSAGE5(Verbosity >= Verbosity_DumpCalls, "%s(%x, %ld, %p, %x)\n", "glBufferData" ,  a0, a1, a2, a3);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glBufferData, "Attempt to call runtime link GL function: glBufferData, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glBufferData) return;
     p_glBufferData(a0, a1, a2, a3);
#else
#if  defined(GL_ARB_vertex_buffer_object)
     ::glBufferDataARB(a0, a1, a2, a3);
#elif  defined(GL_ES_VERSION_2_0)
     ::glBufferData(a0, a1, a2, a3);
#elif  defined(GL_VERSION_1_5)
     ::glBufferData(a0, a1, a2, a3);
#else
    SF_UNUSED4(a0, a1, a2, a3);
    SF_DEBUG_ASSERT(0, "Calling function glBufferData, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glBufferSubData(GLenum  a0, GLintptr  a1, GLsizeiptr  a2, const void * a3)
{
    ScopedGLErrorCheck check("glBufferSubData");
    SF_DEBUG_MESSAGE5(Verbosity >= Verbosity_DumpCalls, "%s(%x, %ld, %ld, %p)\n", "glBufferSubData" ,  a0, a1, a2, a3);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glBufferSubData, "Attempt to call runtime link GL function: glBufferSubData, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glBufferSubData) return;
     p_glBufferSubData(a0, a1, a2, a3);
#else
#if  defined(GL_ARB_vertex_buffer_object)
     ::glBufferSubDataARB(a0, a1, a2, a3);
#elif  defined(GL_ES_VERSION_2_0)
     ::glBufferSubData(a0, a1, a2, a3);
#elif  defined(GL_VERSION_1_5)
     ::glBufferSubData(a0, a1, a2, a3);
#else
    SF_UNUSED4(a0, a1, a2, a3);
    SF_DEBUG_ASSERT(0, "Calling function glBufferSubData, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

GLenum GLImmediate::glCheckFramebufferStatus(GLenum  a0)
{
    ScopedGLErrorCheck check("glCheckFramebufferStatus");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%x)\n", "glCheckFramebufferStatus" ,  a0);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glCheckFramebufferStatus, "Attempt to call runtime link GL function: glCheckFramebufferStatus, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glCheckFramebufferStatus) return (GLenum)(0);
    return p_glCheckFramebufferStatus(a0);
#else
#if  defined(GL_ES_VERSION_2_0)
    return ::glCheckFramebufferStatus(a0);
#elif  defined(GL_VERSION_3_0)
    return ::glCheckFramebufferStatus(a0);
#elif  defined(GL_EXT_framebuffer_object)
    return ::glCheckFramebufferStatusEXT(a0);
#else
    SF_UNUSED1(a0);
    SF_DEBUG_ASSERT(0, "Calling function glCheckFramebufferStatus, but no version of extension supports it.");
    return (GLenum)(0);
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glClear(GLbitfield  a0)
{
    ScopedGLErrorCheck check("glClear");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%x)\n", "glClear" ,  a0);
     ::glClear(a0);
}
void GLImmediate::glClearColor(GLclampf  a0, GLclampf  a1, GLclampf  a2, GLclampf  a3)
{
    ScopedGLErrorCheck check("glClearColor");
    SF_DEBUG_MESSAGE5(Verbosity >= Verbosity_DumpCalls, "%s(%f, %f, %f, %f)\n", "glClearColor" ,  a0, a1, a2, a3);
     ::glClearColor(a0, a1, a2, a3);
}
GLenum GLImmediate::glClientWaitSync(GLsync  a0, GLbitfield  a1, GLuint64  a2)
{
    ScopedGLErrorCheck check("glClientWaitSync");
    SF_DEBUG_MESSAGE4(Verbosity >= Verbosity_DumpCalls, "%s(%p, %x, %lld)\n", "glClientWaitSync" ,  a0, a1, a2);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glClientWaitSync, "Attempt to call runtime link GL function: glClientWaitSync, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glClientWaitSync) return (GLenum)(0);
    return p_glClientWaitSync(a0, a1, a2);
#else
#if  defined(GL_ES_VERSION_3_0)
    return ::glClientWaitSync(a0, a1, a2);
#elif  defined(GL_ARB_sync)
    return ::glClientWaitSync(a0, a1, a2);
#elif  defined(GL_VERSION_3_2)
    return ::glClientWaitSync(a0, a1, a2);
#elif  defined(GL_APPLE_sync)
    return ::glClientWaitSyncAPPLE(a0, a1, a2);
#else
    SF_UNUSED3(a0, a1, a2);
    SF_DEBUG_ASSERT(0, "Calling function glClientWaitSync, but no version of extension supports it.");
    return (GLenum)(0);
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glColorMask(GLboolean  a0, GLboolean  a1, GLboolean  a2, GLboolean  a3)
{
    ScopedGLErrorCheck check("glColorMask");
    SF_DEBUG_MESSAGE5(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d, %d, %d)\n", "glColorMask" ,  a0, a1, a2, a3);
     ::glColorMask(a0, a1, a2, a3);
}
void GLImmediate::glCompileShader(GLuint  a0)
{
    ScopedGLErrorCheck check("glCompileShader");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%d)\n", "glCompileShader" ,  a0);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glCompileShader, "Attempt to call runtime link GL function: glCompileShader, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glCompileShader) return;
     p_glCompileShader(a0);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glCompileShader(a0);
#elif  defined(GL_VERSION_2_0)
     ::glCompileShader(a0);
#else
    SF_UNUSED1(a0);
    SF_DEBUG_ASSERT(0, "Calling function glCompileShader, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glCompressedTexImage2D(GLenum  a0, GLint  a1, GLenum  a2, GLsizei  a3, GLsizei  a4, GLint  a5, GLsizei  a6, const void * a7)
{
    ScopedGLErrorCheck check("glCompressedTexImage2D");
    SF_DEBUG_MESSAGE9(Verbosity >= Verbosity_DumpCalls, "%s(%x, %d, %x, %d, %d, %d, %d, %p)\n", "glCompressedTexImage2D" ,  a0, a1, a2, a3, a4, a5, a6, a7);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glCompressedTexImage2D, "Attempt to call runtime link GL function: glCompressedTexImage2D, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glCompressedTexImage2D) return;
     p_glCompressedTexImage2D(a0, a1, a2, a3, a4, a5, a6, a7);
#else
#if  defined(GL_ARB_texture_compression)
     ::glCompressedTexImage2DARB(a0, a1, a2, a3, a4, a5, a6, a7);
#elif  defined(GL_ES_VERSION_2_0)
     ::glCompressedTexImage2D(a0, a1, a2, a3, a4, a5, a6, a7);
#elif  defined(GL_VERSION_1_3)
     ::glCompressedTexImage2D(a0, a1, a2, a3, a4, a5, a6, a7);
#else
    SF_UNUSED8(a0, a1, a2, a3, a4, a5, a6, a7);
    SF_DEBUG_ASSERT(0, "Calling function glCompressedTexImage2D, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

GLuint GLImmediate::glCreateProgram()
{
    ScopedGLErrorCheck check("glCreateProgram");
    SF_DEBUG_MESSAGE1(Verbosity >= Verbosity_DumpCalls, "%s(void)\n", "glCreateProgram"  );
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glCreateProgram, "Attempt to call runtime link GL function: glCreateProgram, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glCreateProgram) return (GLuint)(0);
    return p_glCreateProgram();
#else
#if  defined(GL_ES_VERSION_2_0)
    return ::glCreateProgram();
#elif  defined(GL_VERSION_2_0)
    return ::glCreateProgram();
#else
    SF_DEBUG_ASSERT(0, "Calling function glCreateProgram, but no version of extension supports it.");
    return (GLuint)(0);
#endif
#endif // SF_GL_RUNTIME_LINK
}

GLuint GLImmediate::glCreateShader(GLenum  a0)
{
    ScopedGLErrorCheck check("glCreateShader");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%x)\n", "glCreateShader" ,  a0);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glCreateShader, "Attempt to call runtime link GL function: glCreateShader, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glCreateShader) return (GLuint)(0);
    return p_glCreateShader(a0);
#else
#if  defined(GL_ES_VERSION_2_0)
    return ::glCreateShader(a0);
#elif  defined(GL_VERSION_2_0)
    return ::glCreateShader(a0);
#else
    SF_UNUSED1(a0);
    SF_DEBUG_ASSERT(0, "Calling function glCreateShader, but no version of extension supports it.");
    return (GLuint)(0);
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glDebugMessageCallback(GLDEBUGPROCKHR  a0, const void * a1)
{
    ScopedGLErrorCheck check("glDebugMessageCallback");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%p, %p)\n", "glDebugMessageCallback" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glDebugMessageCallback, "Attempt to call runtime link GL function: glDebugMessageCallback, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glDebugMessageCallback) return;
     p_glDebugMessageCallback(a0, a1);
#else
#if  defined(GL_ARB_debug_output)
     ::glDebugMessageCallbackARB(a0, a1);
#elif  defined(GL_VERSION_4_3)
     ::glDebugMessageCallback(a0, a1);
#elif  defined(GL_AMD_debug_output)
     ::glDebugMessageCallbackAMD(a0, a1);
#elif  defined(GL_KHR_debug)
     ::glDebugMessageCallbackKHR(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glDebugMessageCallback, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glDebugMessageControl(GLenum  a0, GLenum  a1, GLenum  a2, GLsizei  a3, const GLuint * a4, GLboolean  a5)
{
    ScopedGLErrorCheck check("glDebugMessageControl");
    SF_DEBUG_MESSAGE7(Verbosity >= Verbosity_DumpCalls, "%s(%x, %x, %x, %d, %p, %d)\n", "glDebugMessageControl" ,  a0, a1, a2, a3, a4, a5);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glDebugMessageControl, "Attempt to call runtime link GL function: glDebugMessageControl, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glDebugMessageControl) return;
     p_glDebugMessageControl(a0, a1, a2, a3, a4, a5);
#else
#if  defined(GL_ARB_debug_output)
     ::glDebugMessageControlARB(a0, a1, a2, a3, a4, a5);
#elif  defined(GL_VERSION_4_3)
     ::glDebugMessageControl(a0, a1, a2, a3, a4, a5);
#elif  defined(GL_KHR_debug)
     ::glDebugMessageControlKHR(a0, a1, a2, a3, a4, a5);
#else
    SF_UNUSED6(a0, a1, a2, a3, a4, a5);
    SF_DEBUG_ASSERT(0, "Calling function glDebugMessageControl, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glDeleteBuffers(GLsizei  a0, const GLuint * a1)
{
    ScopedGLErrorCheck check("glDeleteBuffers");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%d, %p)\n", "glDeleteBuffers" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glDeleteBuffers, "Attempt to call runtime link GL function: glDeleteBuffers, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glDeleteBuffers) return;
     p_glDeleteBuffers(a0, a1);
#else
#if  defined(GL_ARB_vertex_buffer_object)
     ::glDeleteBuffersARB(a0, a1);
#elif  defined(GL_ES_VERSION_2_0)
     ::glDeleteBuffers(a0, a1);
#elif  defined(GL_VERSION_1_5)
     ::glDeleteBuffers(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glDeleteBuffers, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glDeleteFramebuffers(GLsizei  a0, const GLuint * a1)
{
    ScopedGLErrorCheck check("glDeleteFramebuffers");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%d, %p)\n", "glDeleteFramebuffers" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glDeleteFramebuffers, "Attempt to call runtime link GL function: glDeleteFramebuffers, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glDeleteFramebuffers) return;
     p_glDeleteFramebuffers(a0, a1);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glDeleteFramebuffers(a0, a1);
#elif  defined(GL_VERSION_3_0)
     ::glDeleteFramebuffers(a0, a1);
#elif  defined(GL_EXT_framebuffer_object)
     ::glDeleteFramebuffersEXT(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glDeleteFramebuffers, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glDeleteProgram(GLuint  a0)
{
    ScopedGLErrorCheck check("glDeleteProgram");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%d)\n", "glDeleteProgram" ,  a0);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glDeleteProgram, "Attempt to call runtime link GL function: glDeleteProgram, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glDeleteProgram) return;
     p_glDeleteProgram(a0);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glDeleteProgram(a0);
#elif  defined(GL_VERSION_2_0)
     ::glDeleteProgram(a0);
#else
    SF_UNUSED1(a0);
    SF_DEBUG_ASSERT(0, "Calling function glDeleteProgram, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glDeleteProgramPipelines(GLsizei  a0, const GLuint * a1)
{
    ScopedGLErrorCheck check("glDeleteProgramPipelines");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%d, %p)\n", "glDeleteProgramPipelines" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glDeleteProgramPipelines, "Attempt to call runtime link GL function: glDeleteProgramPipelines, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glDeleteProgramPipelines) return;
     p_glDeleteProgramPipelines(a0, a1);
#else
#if  defined(GL_VERSION_4_1)
     ::glDeleteProgramPipelines(a0, a1);
#elif  defined(GL_ARB_separate_shader_objects)
     ::glDeleteProgramPipelines(a0, a1);
#elif  defined(GL_EXT_separate_shader_objects)
     ::glDeleteProgramPipelinesEXT(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glDeleteProgramPipelines, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glDeleteRenderbuffers(GLsizei  a0, const GLuint * a1)
{
    ScopedGLErrorCheck check("glDeleteRenderbuffers");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%d, %p)\n", "glDeleteRenderbuffers" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glDeleteRenderbuffers, "Attempt to call runtime link GL function: glDeleteRenderbuffers, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glDeleteRenderbuffers) return;
     p_glDeleteRenderbuffers(a0, a1);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glDeleteRenderbuffers(a0, a1);
#elif  defined(GL_VERSION_3_0)
     ::glDeleteRenderbuffers(a0, a1);
#elif  defined(GL_EXT_framebuffer_object)
     ::glDeleteRenderbuffersEXT(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glDeleteRenderbuffers, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glDeleteShader(GLuint  a0)
{
    ScopedGLErrorCheck check("glDeleteShader");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%d)\n", "glDeleteShader" ,  a0);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glDeleteShader, "Attempt to call runtime link GL function: glDeleteShader, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glDeleteShader) return;
     p_glDeleteShader(a0);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glDeleteShader(a0);
#elif  defined(GL_VERSION_2_0)
     ::glDeleteShader(a0);
#else
    SF_UNUSED1(a0);
    SF_DEBUG_ASSERT(0, "Calling function glDeleteShader, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glDeleteSync(GLsync  a0)
{
    ScopedGLErrorCheck check("glDeleteSync");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%p)\n", "glDeleteSync" ,  a0);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glDeleteSync, "Attempt to call runtime link GL function: glDeleteSync, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glDeleteSync) return;
     p_glDeleteSync(a0);
#else
#if  defined(GL_ES_VERSION_3_0)
     ::glDeleteSync(a0);
#elif  defined(GL_ARB_sync)
     ::glDeleteSync(a0);
#elif  defined(GL_VERSION_3_2)
     ::glDeleteSync(a0);
#elif  defined(GL_APPLE_sync)
     ::glDeleteSyncAPPLE(a0);
#else
    SF_UNUSED1(a0);
    SF_DEBUG_ASSERT(0, "Calling function glDeleteSync, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glDeleteTextures(GLsizei  a0, const GLuint * a1)
{
    ScopedGLErrorCheck check("glDeleteTextures");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%d, %p)\n", "glDeleteTextures" ,  a0, a1);
     ::glDeleteTextures(a0, a1);
}
void GLImmediate::glDeleteVertexArrays(GLsizei  a0, const GLuint * a1)
{
    ScopedGLErrorCheck check("glDeleteVertexArrays");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%d, %p)\n", "glDeleteVertexArrays" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glDeleteVertexArrays, "Attempt to call runtime link GL function: glDeleteVertexArrays, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glDeleteVertexArrays) return;
     p_glDeleteVertexArrays(a0, a1);
#else
#if  defined(GL_VERSION_3_0)
     ::glDeleteVertexArrays(a0, a1);
#elif  defined(GL_ES_VERSION_3_0)
     ::glDeleteVertexArrays(a0, a1);
#elif  defined(GL_ARB_vertex_array_object)
     ::glDeleteVertexArrays(a0, a1);
#elif  defined(GL_APPLE_vertex_array_object)
     ::glDeleteVertexArraysAPPLE(a0, a1);
#elif  defined(GL_OES_vertex_array_object)
     ::glDeleteVertexArraysOES(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glDeleteVertexArrays, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glDepthFunc(GLenum  a0)
{
    ScopedGLErrorCheck check("glDepthFunc");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%x)\n", "glDepthFunc" ,  a0);
     ::glDepthFunc(a0);
}
void GLImmediate::glDepthMask(GLboolean  a0)
{
    ScopedGLErrorCheck check("glDepthMask");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%d)\n", "glDepthMask" ,  a0);
     ::glDepthMask(a0);
}
void GLImmediate::glDisable(GLenum  a0)
{
    ScopedGLErrorCheck check("glDisable");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%x)\n", "glDisable" ,  a0);
     ::glDisable(a0);
}
void GLImmediate::glDisableVertexAttribArray(GLuint  a0)
{
    ScopedGLErrorCheck check("glDisableVertexAttribArray");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%d)\n", "glDisableVertexAttribArray" ,  a0);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glDisableVertexAttribArray, "Attempt to call runtime link GL function: glDisableVertexAttribArray, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glDisableVertexAttribArray) return;
     p_glDisableVertexAttribArray(a0);
#else
#if  defined(GL_ARB_vertex_program)
     ::glDisableVertexAttribArrayARB(a0);
#elif  defined(GL_ES_VERSION_2_0)
     ::glDisableVertexAttribArray(a0);
#elif  defined(GL_VERSION_2_0)
     ::glDisableVertexAttribArray(a0);
#else
    SF_UNUSED1(a0);
    SF_DEBUG_ASSERT(0, "Calling function glDisableVertexAttribArray, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glDrawArrays(GLenum  a0, GLint  a1, GLsizei  a2)
{
    ScopedGLErrorCheck check("glDrawArrays");
    SF_DEBUG_MESSAGE4(Verbosity >= Verbosity_DumpCalls, "%s(%x, %d, %d)\n", "glDrawArrays" ,  a0, a1, a2);
     ::glDrawArrays(a0, a1, a2);
}
void GLImmediate::glDrawElements(GLenum  a0, GLsizei  a1, GLenum  a2, const GLvoid * a3)
{
    ScopedGLErrorCheck check("glDrawElements");
    SF_DEBUG_MESSAGE5(Verbosity >= Verbosity_DumpCalls, "%s(%x, %d, %x, %p)\n", "glDrawElements" ,  a0, a1, a2, a3);
     ::glDrawElements(a0, a1, a2, a3);
}
void GLImmediate::glDrawElementsInstanced(GLenum  a0, GLsizei  a1, GLenum  a2, const void * a3, GLsizei  a4)
{
    ScopedGLErrorCheck check("glDrawElementsInstanced");
    SF_DEBUG_MESSAGE6(Verbosity >= Verbosity_DumpCalls, "%s(%x, %d, %x, %p, %d)\n", "glDrawElementsInstanced" ,  a0, a1, a2, a3, a4);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glDrawElementsInstanced, "Attempt to call runtime link GL function: glDrawElementsInstanced, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glDrawElementsInstanced) return;
     p_glDrawElementsInstanced(a0, a1, a2, a3, a4);
#else
#if  defined(GL_ARB_draw_instanced)
     ::glDrawElementsInstancedARB(a0, a1, a2, a3, a4);
#elif  defined(GL_ES_VERSION_3_0)
     ::glDrawElementsInstanced(a0, a1, a2, a3, a4);
#elif  defined(GL_ARB_instanced_arrays)
     ::glDrawElementsInstanced(a0, a1, a2, a3, a4);
#elif  defined(GL_VERSION_3_1)
     ::glDrawElementsInstanced(a0, a1, a2, a3, a4);
#elif  defined(GL_EXT_draw_instanced)
     ::glDrawElementsInstancedEXT(a0, a1, a2, a3, a4);
#elif  defined(GL_NV_draw_instanced)
     ::glDrawElementsInstancedNV(a0, a1, a2, a3, a4);
#elif  defined(GL_ANGLE_instanced_arrays)
     ::glDrawElementsInstancedANGLE(a0, a1, a2, a3, a4);
#else
    SF_UNUSED5(a0, a1, a2, a3, a4);
    SF_DEBUG_ASSERT(0, "Calling function glDrawElementsInstanced, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glEnable(GLenum  a0)
{
    ScopedGLErrorCheck check("glEnable");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%x)\n", "glEnable" ,  a0);
     ::glEnable(a0);
}
void GLImmediate::glEnableVertexAttribArray(GLuint  a0)
{
    ScopedGLErrorCheck check("glEnableVertexAttribArray");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%d)\n", "glEnableVertexAttribArray" ,  a0);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glEnableVertexAttribArray, "Attempt to call runtime link GL function: glEnableVertexAttribArray, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glEnableVertexAttribArray) return;
     p_glEnableVertexAttribArray(a0);
#else
#if  defined(GL_ARB_vertex_program)
     ::glEnableVertexAttribArrayARB(a0);
#elif  defined(GL_ES_VERSION_2_0)
     ::glEnableVertexAttribArray(a0);
#elif  defined(GL_VERSION_2_0)
     ::glEnableVertexAttribArray(a0);
#else
    SF_UNUSED1(a0);
    SF_DEBUG_ASSERT(0, "Calling function glEnableVertexAttribArray, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

GLsync GLImmediate::glFenceSync(GLenum  a0, GLbitfield  a1)
{
    ScopedGLErrorCheck check("glFenceSync");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%x, %x)\n", "glFenceSync" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glFenceSync, "Attempt to call runtime link GL function: glFenceSync, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glFenceSync) return (GLsync)(0);
    return p_glFenceSync(a0, a1);
#else
#if  defined(GL_ES_VERSION_3_0)
    return ::glFenceSync(a0, a1);
#elif  defined(GL_ARB_sync)
    return ::glFenceSync(a0, a1);
#elif  defined(GL_VERSION_3_2)
    return ::glFenceSync(a0, a1);
#elif  defined(GL_APPLE_sync)
    return ::glFenceSyncAPPLE(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glFenceSync, but no version of extension supports it.");
    return (GLsync)(0);
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glFlush()
{
    ScopedGLErrorCheck check("glFlush");
    SF_DEBUG_MESSAGE1(Verbosity >= Verbosity_DumpCalls, "%s(void)\n", "glFlush"  );
     ::glFlush();
}
void GLImmediate::glFlushMappedBufferRange(GLenum  a0, GLintptr  a1, GLsizeiptr  a2)
{
    ScopedGLErrorCheck check("glFlushMappedBufferRange");
    SF_DEBUG_MESSAGE4(Verbosity >= Verbosity_DumpCalls, "%s(%x, %ld, %ld)\n", "glFlushMappedBufferRange" ,  a0, a1, a2);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glFlushMappedBufferRange, "Attempt to call runtime link GL function: glFlushMappedBufferRange, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glFlushMappedBufferRange) return;
     p_glFlushMappedBufferRange(a0, a1, a2);
#else
#if  defined(GL_VERSION_3_0)
     ::glFlushMappedBufferRange(a0, a1, a2);
#elif  defined(GL_ES_VERSION_3_0)
     ::glFlushMappedBufferRange(a0, a1, a2);
#elif  defined(GL_ARB_map_buffer_range)
     ::glFlushMappedBufferRange(a0, a1, a2);
#elif  defined(GL_APPLE_flush_buffer_range)
     ::glFlushMappedBufferRangeAPPLE(a0, a1, a2);
#elif  defined(GL_EXT_map_buffer_range)
     ::glFlushMappedBufferRangeEXT(a0, a1, a2);
#else
    SF_UNUSED3(a0, a1, a2);
    SF_DEBUG_ASSERT(0, "Calling function glFlushMappedBufferRange, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glFramebufferRenderbuffer(GLenum  a0, GLenum  a1, GLenum  a2, GLuint  a3)
{
    ScopedGLErrorCheck check("glFramebufferRenderbuffer");
    SF_DEBUG_MESSAGE5(Verbosity >= Verbosity_DumpCalls, "%s(%x, %x, %x, %d)\n", "glFramebufferRenderbuffer" ,  a0, a1, a2, a3);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glFramebufferRenderbuffer, "Attempt to call runtime link GL function: glFramebufferRenderbuffer, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glFramebufferRenderbuffer) return;
     p_glFramebufferRenderbuffer(a0, a1, a2, a3);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glFramebufferRenderbuffer(a0, a1, a2, a3);
#elif  defined(GL_VERSION_3_0)
     ::glFramebufferRenderbuffer(a0, a1, a2, a3);
#elif  defined(GL_EXT_framebuffer_object)
     ::glFramebufferRenderbufferEXT(a0, a1, a2, a3);
#else
    SF_UNUSED4(a0, a1, a2, a3);
    SF_DEBUG_ASSERT(0, "Calling function glFramebufferRenderbuffer, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glFramebufferTexture2D(GLenum  a0, GLenum  a1, GLenum  a2, GLuint  a3, GLint  a4)
{
    ScopedGLErrorCheck check("glFramebufferTexture2D");
    SF_DEBUG_MESSAGE6(Verbosity >= Verbosity_DumpCalls, "%s(%x, %x, %x, %d, %d)\n", "glFramebufferTexture2D" ,  a0, a1, a2, a3, a4);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glFramebufferTexture2D, "Attempt to call runtime link GL function: glFramebufferTexture2D, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glFramebufferTexture2D) return;
     p_glFramebufferTexture2D(a0, a1, a2, a3, a4);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glFramebufferTexture2D(a0, a1, a2, a3, a4);
#elif  defined(GL_VERSION_3_0)
     ::glFramebufferTexture2D(a0, a1, a2, a3, a4);
#elif  defined(GL_EXT_framebuffer_object)
     ::glFramebufferTexture2DEXT(a0, a1, a2, a3, a4);
#else
    SF_UNUSED5(a0, a1, a2, a3, a4);
    SF_DEBUG_ASSERT(0, "Calling function glFramebufferTexture2D, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glGenBuffers(GLsizei  a0, GLuint * a1)
{
    ScopedGLErrorCheck check("glGenBuffers");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%d, %p)\n", "glGenBuffers" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glGenBuffers, "Attempt to call runtime link GL function: glGenBuffers, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glGenBuffers) return;
     p_glGenBuffers(a0, a1);
#else
#if  defined(GL_ARB_vertex_buffer_object)
     ::glGenBuffersARB(a0, a1);
#elif  defined(GL_ES_VERSION_2_0)
     ::glGenBuffers(a0, a1);
#elif  defined(GL_VERSION_1_5)
     ::glGenBuffers(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glGenBuffers, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glGenFramebuffers(GLsizei  a0, GLuint * a1)
{
    ScopedGLErrorCheck check("glGenFramebuffers");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%d, %p)\n", "glGenFramebuffers" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glGenFramebuffers, "Attempt to call runtime link GL function: glGenFramebuffers, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glGenFramebuffers) return;
     p_glGenFramebuffers(a0, a1);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glGenFramebuffers(a0, a1);
#elif  defined(GL_VERSION_3_0)
     ::glGenFramebuffers(a0, a1);
#elif  defined(GL_EXT_framebuffer_object)
     ::glGenFramebuffersEXT(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glGenFramebuffers, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glGenProgramPipelines(GLsizei  a0, GLuint * a1)
{
    ScopedGLErrorCheck check("glGenProgramPipelines");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%d, %p)\n", "glGenProgramPipelines" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glGenProgramPipelines, "Attempt to call runtime link GL function: glGenProgramPipelines, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glGenProgramPipelines) return;
     p_glGenProgramPipelines(a0, a1);
#else
#if  defined(GL_VERSION_4_1)
     ::glGenProgramPipelines(a0, a1);
#elif  defined(GL_ARB_separate_shader_objects)
     ::glGenProgramPipelines(a0, a1);
#elif  defined(GL_EXT_separate_shader_objects)
     ::glGenProgramPipelinesEXT(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glGenProgramPipelines, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glGenRenderbuffers(GLsizei  a0, GLuint * a1)
{
    ScopedGLErrorCheck check("glGenRenderbuffers");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%d, %p)\n", "glGenRenderbuffers" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glGenRenderbuffers, "Attempt to call runtime link GL function: glGenRenderbuffers, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glGenRenderbuffers) return;
     p_glGenRenderbuffers(a0, a1);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glGenRenderbuffers(a0, a1);
#elif  defined(GL_VERSION_3_0)
     ::glGenRenderbuffers(a0, a1);
#elif  defined(GL_EXT_framebuffer_object)
     ::glGenRenderbuffersEXT(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glGenRenderbuffers, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glGenTextures(GLsizei  a0, GLuint * a1)
{
    ScopedGLErrorCheck check("glGenTextures");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%d, %p)\n", "glGenTextures" ,  a0, a1);
     ::glGenTextures(a0, a1);
}
void GLImmediate::glGenVertexArrays(GLsizei  a0, GLuint * a1)
{
    ScopedGLErrorCheck check("glGenVertexArrays");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%d, %p)\n", "glGenVertexArrays" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glGenVertexArrays, "Attempt to call runtime link GL function: glGenVertexArrays, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glGenVertexArrays) return;
     p_glGenVertexArrays(a0, a1);
#else
#if  defined(GL_VERSION_3_0)
     ::glGenVertexArrays(a0, a1);
#elif  defined(GL_ES_VERSION_3_0)
     ::glGenVertexArrays(a0, a1);
#elif  defined(GL_ARB_vertex_array_object)
     ::glGenVertexArrays(a0, a1);
#elif  defined(GL_APPLE_vertex_array_object)
     ::glGenVertexArraysAPPLE(a0, a1);
#elif  defined(GL_OES_vertex_array_object)
     ::glGenVertexArraysOES(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glGenVertexArrays, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glGenerateMipmap(GLenum  a0)
{
    ScopedGLErrorCheck check("glGenerateMipmap");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%x)\n", "glGenerateMipmap" ,  a0);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glGenerateMipmap, "Attempt to call runtime link GL function: glGenerateMipmap, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glGenerateMipmap) return;
     p_glGenerateMipmap(a0);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glGenerateMipmap(a0);
#elif  defined(GL_VERSION_3_0)
     ::glGenerateMipmap(a0);
#elif  defined(GL_EXT_framebuffer_object)
     ::glGenerateMipmapEXT(a0);
#else
    SF_UNUSED1(a0);
    SF_DEBUG_ASSERT(0, "Calling function glGenerateMipmap, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glGetActiveUniform(GLuint  a0, GLuint  a1, GLsizei  a2, GLsizei * a3, GLint * a4, GLenum * a5, GLchar * a6)
{
    ScopedGLErrorCheck check("glGetActiveUniform");
    SF_DEBUG_MESSAGE8(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d, %d, %p, %p, %p, %p)\n", "glGetActiveUniform" ,  a0, a1, a2, a3, a4, a5, a6);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glGetActiveUniform, "Attempt to call runtime link GL function: glGetActiveUniform, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glGetActiveUniform) return;
     p_glGetActiveUniform(a0, a1, a2, a3, a4, a5, a6);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glGetActiveUniform(a0, a1, a2, a3, a4, a5, a6);
#elif  defined(GL_VERSION_2_0)
     ::glGetActiveUniform(a0, a1, a2, a3, a4, a5, a6);
#else
    SF_UNUSED7(a0, a1, a2, a3, a4, a5, a6);
    SF_DEBUG_ASSERT(0, "Calling function glGetActiveUniform, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

GLenum GLImmediate::glGetError()
{
    ScopedGLErrorCheck check("glGetError");
    SF_DEBUG_MESSAGE1(Verbosity >= Verbosity_DumpCalls, "%s(void)\n", "glGetError"  );
    return ::glGetError();
}
void GLImmediate::glGetFloatv(GLenum  a0, GLfloat * a1)
{
    ScopedGLErrorCheck check("glGetFloatv");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%x, %p)\n", "glGetFloatv" ,  a0, a1);
     ::glGetFloatv(a0, a1);
}
GLint GLImmediate::glGetFragDataLocation(GLuint  a0, const GLchar * a1)
{
    ScopedGLErrorCheck check("glGetFragDataLocation");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%d, %p)\n", "glGetFragDataLocation" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glGetFragDataLocation, "Attempt to call runtime link GL function: glGetFragDataLocation, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glGetFragDataLocation) return (GLint)(0);
    return p_glGetFragDataLocation(a0, a1);
#else
#if  defined(GL_VERSION_3_0)
    return ::glGetFragDataLocation(a0, a1);
#elif  defined(GL_ES_VERSION_3_0)
    return ::glGetFragDataLocation(a0, a1);
#elif  defined(GL_EXT_gpu_shader4)
    return ::glGetFragDataLocationEXT(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glGetFragDataLocation, but no version of extension supports it.");
    return (GLint)(0);
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glGetFramebufferAttachmentParameteriv(GLenum  a0, GLenum  a1, GLenum  a2, GLint * a3)
{
    ScopedGLErrorCheck check("glGetFramebufferAttachmentParameteriv");
    SF_DEBUG_MESSAGE5(Verbosity >= Verbosity_DumpCalls, "%s(%x, %x, %x, %p)\n", "glGetFramebufferAttachmentParameteriv" ,  a0, a1, a2, a3);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glGetFramebufferAttachmentParameteriv, "Attempt to call runtime link GL function: glGetFramebufferAttachmentParameteriv, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glGetFramebufferAttachmentParameteriv) return;
     p_glGetFramebufferAttachmentParameteriv(a0, a1, a2, a3);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glGetFramebufferAttachmentParameteriv(a0, a1, a2, a3);
#elif  defined(GL_VERSION_3_0)
     ::glGetFramebufferAttachmentParameteriv(a0, a1, a2, a3);
#elif  defined(GL_EXT_framebuffer_object)
     ::glGetFramebufferAttachmentParameterivEXT(a0, a1, a2, a3);
#else
    SF_UNUSED4(a0, a1, a2, a3);
    SF_DEBUG_ASSERT(0, "Calling function glGetFramebufferAttachmentParameteriv, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glGetIntegerv(GLenum  a0, GLint * a1)
{
    ScopedGLErrorCheck check("glGetIntegerv");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%x, %p)\n", "glGetIntegerv" ,  a0, a1);
     ::glGetIntegerv(a0, a1);
}
void GLImmediate::glGetProgramBinary(GLuint  a0, GLsizei  a1, GLsizei * a2, GLenum * a3, void * a4)
{
    ScopedGLErrorCheck check("glGetProgramBinary");
    SF_DEBUG_MESSAGE6(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d, %p, %p, %p)\n", "glGetProgramBinary" ,  a0, a1, a2, a3, a4);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glGetProgramBinary, "Attempt to call runtime link GL function: glGetProgramBinary, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glGetProgramBinary) return;
     p_glGetProgramBinary(a0, a1, a2, a3, a4);
#else
#if  defined(GL_ES_VERSION_3_0)
     ::glGetProgramBinary(a0, a1, a2, a3, a4);
#elif  defined(GL_VERSION_4_1)
     ::glGetProgramBinary(a0, a1, a2, a3, a4);
#elif  defined(GL_ARB_get_program_binary)
     ::glGetProgramBinary(a0, a1, a2, a3, a4);
#elif  defined(GL_OES_get_program_binary)
     ::glGetProgramBinaryOES(a0, a1, a2, a3, a4);
#else
    SF_UNUSED5(a0, a1, a2, a3, a4);
    SF_DEBUG_ASSERT(0, "Calling function glGetProgramBinary, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glGetProgramInfoLog(GLuint  a0, GLsizei  a1, GLsizei * a2, GLchar * a3)
{
    ScopedGLErrorCheck check("glGetProgramInfoLog");
    SF_DEBUG_MESSAGE5(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d, %p, %p)\n", "glGetProgramInfoLog" ,  a0, a1, a2, a3);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glGetProgramInfoLog, "Attempt to call runtime link GL function: glGetProgramInfoLog, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glGetProgramInfoLog) return;
     p_glGetProgramInfoLog(a0, a1, a2, a3);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glGetProgramInfoLog(a0, a1, a2, a3);
#elif  defined(GL_VERSION_2_0)
     ::glGetProgramInfoLog(a0, a1, a2, a3);
#else
    SF_UNUSED4(a0, a1, a2, a3);
    SF_DEBUG_ASSERT(0, "Calling function glGetProgramInfoLog, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glGetProgramiv(GLuint  a0, GLenum  a1, GLint * a2)
{
    ScopedGLErrorCheck check("glGetProgramiv");
    SF_DEBUG_MESSAGE4(Verbosity >= Verbosity_DumpCalls, "%s(%d, %x, %p)\n", "glGetProgramiv" ,  a0, a1, a2);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glGetProgramiv, "Attempt to call runtime link GL function: glGetProgramiv, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glGetProgramiv) return;
     p_glGetProgramiv(a0, a1, a2);
#else
#if  defined(GL_ARB_fragment_program)
     ::glGetProgramivARB(a0, a1, a2);
#elif  defined(GL_ES_VERSION_2_0)
     ::glGetProgramiv(a0, a1, a2);
#elif  defined(GL_VERSION_2_0)
     ::glGetProgramiv(a0, a1, a2);
#elif  defined(GL_NV_vertex_program)
     ::glGetProgramivNV(a0, a1, a2);
#else
    SF_UNUSED3(a0, a1, a2);
    SF_DEBUG_ASSERT(0, "Calling function glGetProgramiv, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glGetRenderbufferParameteriv(GLenum  a0, GLenum  a1, GLint * a2)
{
    ScopedGLErrorCheck check("glGetRenderbufferParameteriv");
    SF_DEBUG_MESSAGE4(Verbosity >= Verbosity_DumpCalls, "%s(%x, %x, %p)\n", "glGetRenderbufferParameteriv" ,  a0, a1, a2);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glGetRenderbufferParameteriv, "Attempt to call runtime link GL function: glGetRenderbufferParameteriv, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glGetRenderbufferParameteriv) return;
     p_glGetRenderbufferParameteriv(a0, a1, a2);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glGetRenderbufferParameteriv(a0, a1, a2);
#elif  defined(GL_VERSION_3_0)
     ::glGetRenderbufferParameteriv(a0, a1, a2);
#elif  defined(GL_EXT_framebuffer_object)
     ::glGetRenderbufferParameterivEXT(a0, a1, a2);
#else
    SF_UNUSED3(a0, a1, a2);
    SF_DEBUG_ASSERT(0, "Calling function glGetRenderbufferParameteriv, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glGetShaderInfoLog(GLuint  a0, GLsizei  a1, GLsizei * a2, GLchar * a3)
{
    ScopedGLErrorCheck check("glGetShaderInfoLog");
    SF_DEBUG_MESSAGE5(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d, %p, %p)\n", "glGetShaderInfoLog" ,  a0, a1, a2, a3);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glGetShaderInfoLog, "Attempt to call runtime link GL function: glGetShaderInfoLog, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glGetShaderInfoLog) return;
     p_glGetShaderInfoLog(a0, a1, a2, a3);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glGetShaderInfoLog(a0, a1, a2, a3);
#elif  defined(GL_VERSION_2_0)
     ::glGetShaderInfoLog(a0, a1, a2, a3);
#else
    SF_UNUSED4(a0, a1, a2, a3);
    SF_DEBUG_ASSERT(0, "Calling function glGetShaderInfoLog, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glGetShaderiv(GLuint  a0, GLenum  a1, GLint * a2)
{
    ScopedGLErrorCheck check("glGetShaderiv");
    SF_DEBUG_MESSAGE4(Verbosity >= Verbosity_DumpCalls, "%s(%d, %x, %p)\n", "glGetShaderiv" ,  a0, a1, a2);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glGetShaderiv, "Attempt to call runtime link GL function: glGetShaderiv, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glGetShaderiv) return;
     p_glGetShaderiv(a0, a1, a2);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glGetShaderiv(a0, a1, a2);
#elif  defined(GL_VERSION_2_0)
     ::glGetShaderiv(a0, a1, a2);
#else
    SF_UNUSED3(a0, a1, a2);
    SF_DEBUG_ASSERT(0, "Calling function glGetShaderiv, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

const GLubyte * GLImmediate::glGetString(GLenum  a0)
{
    ScopedGLErrorCheck check("glGetString");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%x)\n", "glGetString" ,  a0);
    return ::glGetString(a0);
}
const GLubyte * GLImmediate::glGetStringi(GLenum  a0, GLuint  a1)
{
    ScopedGLErrorCheck check("glGetStringi");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%x, %d)\n", "glGetStringi" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glGetStringi, "Attempt to call runtime link GL function: glGetStringi, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glGetStringi) return (const GLubyte *)(0);
    return p_glGetStringi(a0, a1);
#else
#if  defined(GL_VERSION_3_0)
    return ::glGetStringi(a0, a1);
#elif  defined(GL_ES_VERSION_3_0)
    return ::glGetStringi(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glGetStringi, but no version of extension supports it.");
    return (const GLubyte *)(0);
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glGetSynciv(GLsync  a0, GLenum  a1, GLsizei  a2, GLsizei * a3, GLint * a4)
{
    ScopedGLErrorCheck check("glGetSynciv");
    SF_DEBUG_MESSAGE6(Verbosity >= Verbosity_DumpCalls, "%s(%p, %x, %d, %p, %p)\n", "glGetSynciv" ,  a0, a1, a2, a3, a4);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glGetSynciv, "Attempt to call runtime link GL function: glGetSynciv, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glGetSynciv) return;
     p_glGetSynciv(a0, a1, a2, a3, a4);
#else
#if  defined(GL_ES_VERSION_3_0)
     ::glGetSynciv(a0, a1, a2, a3, a4);
#elif  defined(GL_ARB_sync)
     ::glGetSynciv(a0, a1, a2, a3, a4);
#elif  defined(GL_VERSION_3_2)
     ::glGetSynciv(a0, a1, a2, a3, a4);
#elif  defined(GL_APPLE_sync)
     ::glGetSyncivAPPLE(a0, a1, a2, a3, a4);
#else
    SF_UNUSED5(a0, a1, a2, a3, a4);
    SF_DEBUG_ASSERT(0, "Calling function glGetSynciv, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glGetTexImage(GLenum  a0, GLint  a1, GLenum  a2, GLenum  a3, GLvoid * a4)
{
    ScopedGLErrorCheck check("glGetTexImage");
    SF_DEBUG_MESSAGE6(Verbosity >= Verbosity_DumpCalls, "%s(%x, %d, %x, %x, %p)\n", "glGetTexImage" ,  a0, a1, a2, a3, a4);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_OPENGL))
    SF_DEBUG_ASSERT(p_glGetTexImage, "Attempt to call runtime link GL function: glGetTexImage, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glGetTexImage) return;
     p_glGetTexImage(a0, a1, a2, a3, a4);
#else
#if  defined(GL_VERSION_1_1)
     ::glGetTexImage(a0, a1, a2, a3, a4);
#else
    SF_UNUSED5(a0, a1, a2, a3, a4);
    SF_DEBUG_ASSERT(0, "Calling function glGetTexImage, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glGetTexLevelParameteriv(GLenum  a0, GLint  a1, GLenum  a2, GLint * a3)
{
    ScopedGLErrorCheck check("glGetTexLevelParameteriv");
    SF_DEBUG_MESSAGE5(Verbosity >= Verbosity_DumpCalls, "%s(%x, %d, %x, %p)\n", "glGetTexLevelParameteriv" ,  a0, a1, a2, a3);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_OPENGL))
    SF_DEBUG_ASSERT(p_glGetTexLevelParameteriv, "Attempt to call runtime link GL function: glGetTexLevelParameteriv, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glGetTexLevelParameteriv) return;
     p_glGetTexLevelParameteriv(a0, a1, a2, a3);
#else
#if  defined(GL_VERSION_1_1)
     ::glGetTexLevelParameteriv(a0, a1, a2, a3);
#else
    SF_UNUSED4(a0, a1, a2, a3);
    SF_DEBUG_ASSERT(0, "Calling function glGetTexLevelParameteriv, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glGetTexParameteriv(GLenum  a0, GLenum  a1, GLint * a2)
{
    ScopedGLErrorCheck check("glGetTexParameteriv");
    SF_DEBUG_MESSAGE4(Verbosity >= Verbosity_DumpCalls, "%s(%x, %x, %p)\n", "glGetTexParameteriv" ,  a0, a1, a2);
     ::glGetTexParameteriv(a0, a1, a2);
}
GLint GLImmediate::glGetUniformLocation(GLuint  a0, const GLchar * a1)
{
    ScopedGLErrorCheck check("glGetUniformLocation");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%d, %p)\n", "glGetUniformLocation" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glGetUniformLocation, "Attempt to call runtime link GL function: glGetUniformLocation, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glGetUniformLocation) return (GLint)(0);
    return p_glGetUniformLocation(a0, a1);
#else
#if  defined(GL_ES_VERSION_2_0)
    return ::glGetUniformLocation(a0, a1);
#elif  defined(GL_VERSION_2_0)
    return ::glGetUniformLocation(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glGetUniformLocation, but no version of extension supports it.");
    return (GLint)(0);
#endif
#endif // SF_GL_RUNTIME_LINK
}

GLboolean GLImmediate::glIsFramebuffer(GLuint  a0)
{
    ScopedGLErrorCheck check("glIsFramebuffer");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%d)\n", "glIsFramebuffer" ,  a0);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glIsFramebuffer, "Attempt to call runtime link GL function: glIsFramebuffer, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glIsFramebuffer) return (GLboolean)(0);
    return p_glIsFramebuffer(a0);
#else
#if  defined(GL_ES_VERSION_2_0)
    return ::glIsFramebuffer(a0);
#elif  defined(GL_VERSION_3_0)
    return ::glIsFramebuffer(a0);
#elif  defined(GL_EXT_framebuffer_object)
    return ::glIsFramebufferEXT(a0);
#else
    SF_UNUSED1(a0);
    SF_DEBUG_ASSERT(0, "Calling function glIsFramebuffer, but no version of extension supports it.");
    return (GLboolean)(0);
#endif
#endif // SF_GL_RUNTIME_LINK
}

GLboolean GLImmediate::glIsProgram(GLuint  a0)
{
    ScopedGLErrorCheck check("glIsProgram");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%d)\n", "glIsProgram" ,  a0);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glIsProgram, "Attempt to call runtime link GL function: glIsProgram, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glIsProgram) return (GLboolean)(0);
    return p_glIsProgram(a0);
#else
#if  defined(GL_ARB_fragment_program)
    return ::glIsProgramARB(a0);
#elif  defined(GL_ES_VERSION_2_0)
    return ::glIsProgram(a0);
#elif  defined(GL_VERSION_2_0)
    return ::glIsProgram(a0);
#elif  defined(GL_NV_vertex_program)
    return ::glIsProgramNV(a0);
#else
    SF_UNUSED1(a0);
    SF_DEBUG_ASSERT(0, "Calling function glIsProgram, but no version of extension supports it.");
    return (GLboolean)(0);
#endif
#endif // SF_GL_RUNTIME_LINK
}

GLboolean GLImmediate::glIsRenderbuffer(GLuint  a0)
{
    ScopedGLErrorCheck check("glIsRenderbuffer");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%d)\n", "glIsRenderbuffer" ,  a0);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glIsRenderbuffer, "Attempt to call runtime link GL function: glIsRenderbuffer, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glIsRenderbuffer) return (GLboolean)(0);
    return p_glIsRenderbuffer(a0);
#else
#if  defined(GL_ES_VERSION_2_0)
    return ::glIsRenderbuffer(a0);
#elif  defined(GL_VERSION_3_0)
    return ::glIsRenderbuffer(a0);
#elif  defined(GL_EXT_framebuffer_object)
    return ::glIsRenderbufferEXT(a0);
#else
    SF_UNUSED1(a0);
    SF_DEBUG_ASSERT(0, "Calling function glIsRenderbuffer, but no version of extension supports it.");
    return (GLboolean)(0);
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glLinkProgram(GLuint  a0)
{
    ScopedGLErrorCheck check("glLinkProgram");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%d)\n", "glLinkProgram" ,  a0);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glLinkProgram, "Attempt to call runtime link GL function: glLinkProgram, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glLinkProgram) return;
     p_glLinkProgram(a0);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glLinkProgram(a0);
#elif  defined(GL_VERSION_2_0)
     ::glLinkProgram(a0);
#else
    SF_UNUSED1(a0);
    SF_DEBUG_ASSERT(0, "Calling function glLinkProgram, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void * GLImmediate::glMapBuffer(GLenum  a0, GLenum  a1)
{
    ScopedGLErrorCheck check("glMapBuffer");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%x, %x)\n", "glMapBuffer" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glMapBuffer, "Attempt to call runtime link GL function: glMapBuffer, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glMapBuffer) return (void *)(0);
    return p_glMapBuffer(a0, a1);
#else
#if  defined(GL_ARB_vertex_buffer_object)
    return ::glMapBufferARB(a0, a1);
#elif  defined(GL_VERSION_1_5)
    return ::glMapBuffer(a0, a1);
#elif  defined(GL_OES_mapbuffer)
    return ::glMapBufferOES(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glMapBuffer, but no version of extension supports it.");
    return (void *)(0);
#endif
#endif // SF_GL_RUNTIME_LINK
}

void * GLImmediate::glMapBufferRange(GLenum  a0, GLintptr  a1, GLsizeiptr  a2, GLbitfield  a3)
{
    ScopedGLErrorCheck check("glMapBufferRange");
    SF_DEBUG_MESSAGE5(Verbosity >= Verbosity_DumpCalls, "%s(%x, %ld, %ld, %x)\n", "glMapBufferRange" ,  a0, a1, a2, a3);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glMapBufferRange, "Attempt to call runtime link GL function: glMapBufferRange, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glMapBufferRange) return (void *)(0);
    return p_glMapBufferRange(a0, a1, a2, a3);
#else
#if  defined(GL_VERSION_3_0)
    return ::glMapBufferRange(a0, a1, a2, a3);
#elif  defined(GL_ES_VERSION_3_0)
    return ::glMapBufferRange(a0, a1, a2, a3);
#elif  defined(GL_ARB_map_buffer_range)
    return ::glMapBufferRange(a0, a1, a2, a3);
#elif  defined(GL_EXT_map_buffer_range)
    return ::glMapBufferRangeEXT(a0, a1, a2, a3);
#else
    SF_UNUSED4(a0, a1, a2, a3);
    SF_DEBUG_ASSERT(0, "Calling function glMapBufferRange, but no version of extension supports it.");
    return (void *)(0);
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glPixelStorei(GLenum  a0, GLint  a1)
{
    ScopedGLErrorCheck check("glPixelStorei");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%x, %d)\n", "glPixelStorei" ,  a0, a1);
     ::glPixelStorei(a0, a1);
}
void GLImmediate::glPolygonMode(GLenum  a0, GLenum  a1)
{
    ScopedGLErrorCheck check("glPolygonMode");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%x, %x)\n", "glPolygonMode" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_OPENGL))
    SF_DEBUG_ASSERT(p_glPolygonMode, "Attempt to call runtime link GL function: glPolygonMode, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glPolygonMode) return;
     p_glPolygonMode(a0, a1);
#else
#if  defined(GL_VERSION_1_1)
     ::glPolygonMode(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glPolygonMode, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glPopGroupMarker()
{
    ScopedGLErrorCheck check("glPopGroupMarker");
    SF_DEBUG_MESSAGE1(Verbosity >= Verbosity_DumpCalls, "%s(void)\n", "glPopGroupMarker"  );
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glPopGroupMarker, "Attempt to call runtime link GL function: glPopGroupMarker, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glPopGroupMarker) return;
     p_glPopGroupMarker();
#else
#if  defined(GL_EXT_debug_marker)
     ::glPopGroupMarkerEXT();
#else
    SF_DEBUG_ASSERT(0, "Calling function glPopGroupMarker, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glProgramBinary(GLuint  a0, GLenum  a1, const void * a2, GLint  a3)
{
    ScopedGLErrorCheck check("glProgramBinary");
    SF_DEBUG_MESSAGE5(Verbosity >= Verbosity_DumpCalls, "%s(%d, %x, %p, %d)\n", "glProgramBinary" ,  a0, a1, a2, a3);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glProgramBinary, "Attempt to call runtime link GL function: glProgramBinary, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glProgramBinary) return;
     p_glProgramBinary(a0, a1, a2, a3);
#else
#if  defined(GL_ES_VERSION_3_0)
     ::glProgramBinary(a0, a1, a2, a3);
#elif  defined(GL_VERSION_4_1)
     ::glProgramBinary(a0, a1, a2, a3);
#elif  defined(GL_ARB_get_program_binary)
     ::glProgramBinary(a0, a1, a2, a3);
#elif  defined(GL_OES_get_program_binary)
     ::glProgramBinaryOES(a0, a1, a2, a3);
#else
    SF_UNUSED4(a0, a1, a2, a3);
    SF_DEBUG_ASSERT(0, "Calling function glProgramBinary, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glProgramParameteri(GLuint  a0, GLenum  a1, GLint  a2)
{
    ScopedGLErrorCheck check("glProgramParameteri");
    SF_DEBUG_MESSAGE4(Verbosity >= Verbosity_DumpCalls, "%s(%d, %x, %d)\n", "glProgramParameteri" ,  a0, a1, a2);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glProgramParameteri, "Attempt to call runtime link GL function: glProgramParameteri, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glProgramParameteri) return;
     p_glProgramParameteri(a0, a1, a2);
#else
#if  defined(GL_ARB_geometry_shader4)
     ::glProgramParameteriARB(a0, a1, a2);
#elif  defined(GL_ES_VERSION_3_0)
     ::glProgramParameteri(a0, a1, a2);
#elif  defined(GL_VERSION_4_1)
     ::glProgramParameteri(a0, a1, a2);
#elif  defined(GL_ARB_separate_shader_objects)
     ::glProgramParameteri(a0, a1, a2);
#elif  defined(GL_EXT_geometry_shader4)
     ::glProgramParameteriEXT(a0, a1, a2);
#elif  defined(GL_EXT_separate_shader_objects)
     ::glProgramParameteriEXT(a0, a1, a2);
#else
    SF_UNUSED3(a0, a1, a2);
    SF_DEBUG_ASSERT(0, "Calling function glProgramParameteri, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glProgramUniform1fv(GLuint  a0, GLint  a1, GLsizei  a2, const GLfloat * a3)
{
    ScopedGLErrorCheck check("glProgramUniform1fv");
    SF_DEBUG_MESSAGE5(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d, %d, %p)\n", "glProgramUniform1fv" ,  a0, a1, a2, a3);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glProgramUniform1fv, "Attempt to call runtime link GL function: glProgramUniform1fv, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glProgramUniform1fv) return;
     p_glProgramUniform1fv(a0, a1, a2, a3);
#else
#if  defined(GL_VERSION_4_1)
     ::glProgramUniform1fv(a0, a1, a2, a3);
#elif  defined(GL_ARB_separate_shader_objects)
     ::glProgramUniform1fv(a0, a1, a2, a3);
#elif  defined(GL_EXT_direct_state_access)
     ::glProgramUniform1fvEXT(a0, a1, a2, a3);
#elif  defined(GL_EXT_separate_shader_objects)
     ::glProgramUniform1fvEXT(a0, a1, a2, a3);
#else
    SF_UNUSED4(a0, a1, a2, a3);
    SF_DEBUG_ASSERT(0, "Calling function glProgramUniform1fv, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glProgramUniform1iv(GLuint  a0, GLint  a1, GLsizei  a2, const GLint * a3)
{
    ScopedGLErrorCheck check("glProgramUniform1iv");
    SF_DEBUG_MESSAGE5(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d, %d, %p)\n", "glProgramUniform1iv" ,  a0, a1, a2, a3);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glProgramUniform1iv, "Attempt to call runtime link GL function: glProgramUniform1iv, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glProgramUniform1iv) return;
     p_glProgramUniform1iv(a0, a1, a2, a3);
#else
#if  defined(GL_VERSION_4_1)
     ::glProgramUniform1iv(a0, a1, a2, a3);
#elif  defined(GL_ARB_separate_shader_objects)
     ::glProgramUniform1iv(a0, a1, a2, a3);
#elif  defined(GL_EXT_direct_state_access)
     ::glProgramUniform1ivEXT(a0, a1, a2, a3);
#elif  defined(GL_EXT_separate_shader_objects)
     ::glProgramUniform1ivEXT(a0, a1, a2, a3);
#else
    SF_UNUSED4(a0, a1, a2, a3);
    SF_DEBUG_ASSERT(0, "Calling function glProgramUniform1iv, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glProgramUniform2fv(GLuint  a0, GLint  a1, GLsizei  a2, const GLfloat * a3)
{
    ScopedGLErrorCheck check("glProgramUniform2fv");
    SF_DEBUG_MESSAGE5(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d, %d, %p)\n", "glProgramUniform2fv" ,  a0, a1, a2, a3);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glProgramUniform2fv, "Attempt to call runtime link GL function: glProgramUniform2fv, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glProgramUniform2fv) return;
     p_glProgramUniform2fv(a0, a1, a2, a3);
#else
#if  defined(GL_VERSION_4_1)
     ::glProgramUniform2fv(a0, a1, a2, a3);
#elif  defined(GL_ARB_separate_shader_objects)
     ::glProgramUniform2fv(a0, a1, a2, a3);
#elif  defined(GL_EXT_direct_state_access)
     ::glProgramUniform2fvEXT(a0, a1, a2, a3);
#elif  defined(GL_EXT_separate_shader_objects)
     ::glProgramUniform2fvEXT(a0, a1, a2, a3);
#else
    SF_UNUSED4(a0, a1, a2, a3);
    SF_DEBUG_ASSERT(0, "Calling function glProgramUniform2fv, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glProgramUniform3fv(GLuint  a0, GLint  a1, GLsizei  a2, const GLfloat * a3)
{
    ScopedGLErrorCheck check("glProgramUniform3fv");
    SF_DEBUG_MESSAGE5(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d, %d, %p)\n", "glProgramUniform3fv" ,  a0, a1, a2, a3);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glProgramUniform3fv, "Attempt to call runtime link GL function: glProgramUniform3fv, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glProgramUniform3fv) return;
     p_glProgramUniform3fv(a0, a1, a2, a3);
#else
#if  defined(GL_VERSION_4_1)
     ::glProgramUniform3fv(a0, a1, a2, a3);
#elif  defined(GL_ARB_separate_shader_objects)
     ::glProgramUniform3fv(a0, a1, a2, a3);
#elif  defined(GL_EXT_direct_state_access)
     ::glProgramUniform3fvEXT(a0, a1, a2, a3);
#elif  defined(GL_EXT_separate_shader_objects)
     ::glProgramUniform3fvEXT(a0, a1, a2, a3);
#else
    SF_UNUSED4(a0, a1, a2, a3);
    SF_DEBUG_ASSERT(0, "Calling function glProgramUniform3fv, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glProgramUniform4fv(GLuint  a0, GLint  a1, GLsizei  a2, const GLfloat * a3)
{
    ScopedGLErrorCheck check("glProgramUniform4fv");
    SF_DEBUG_MESSAGE5(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d, %d, %p)\n", "glProgramUniform4fv" ,  a0, a1, a2, a3);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glProgramUniform4fv, "Attempt to call runtime link GL function: glProgramUniform4fv, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glProgramUniform4fv) return;
     p_glProgramUniform4fv(a0, a1, a2, a3);
#else
#if  defined(GL_VERSION_4_1)
     ::glProgramUniform4fv(a0, a1, a2, a3);
#elif  defined(GL_ARB_separate_shader_objects)
     ::glProgramUniform4fv(a0, a1, a2, a3);
#elif  defined(GL_EXT_direct_state_access)
     ::glProgramUniform4fvEXT(a0, a1, a2, a3);
#elif  defined(GL_EXT_separate_shader_objects)
     ::glProgramUniform4fvEXT(a0, a1, a2, a3);
#else
    SF_UNUSED4(a0, a1, a2, a3);
    SF_DEBUG_ASSERT(0, "Calling function glProgramUniform4fv, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glProgramUniformMatrix4fv(GLuint  a0, GLint  a1, GLsizei  a2, GLboolean  a3, const GLfloat * a4)
{
    ScopedGLErrorCheck check("glProgramUniformMatrix4fv");
    SF_DEBUG_MESSAGE6(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d, %d, %d, %p)\n", "glProgramUniformMatrix4fv" ,  a0, a1, a2, a3, a4);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glProgramUniformMatrix4fv, "Attempt to call runtime link GL function: glProgramUniformMatrix4fv, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glProgramUniformMatrix4fv) return;
     p_glProgramUniformMatrix4fv(a0, a1, a2, a3, a4);
#else
#if  defined(GL_VERSION_4_1)
     ::glProgramUniformMatrix4fv(a0, a1, a2, a3, a4);
#elif  defined(GL_ARB_separate_shader_objects)
     ::glProgramUniformMatrix4fv(a0, a1, a2, a3, a4);
#elif  defined(GL_EXT_direct_state_access)
     ::glProgramUniformMatrix4fvEXT(a0, a1, a2, a3, a4);
#elif  defined(GL_EXT_separate_shader_objects)
     ::glProgramUniformMatrix4fvEXT(a0, a1, a2, a3, a4);
#else
    SF_UNUSED5(a0, a1, a2, a3, a4);
    SF_DEBUG_ASSERT(0, "Calling function glProgramUniformMatrix4fv, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glPushGroupMarker(GLsizei  a0, const GLchar * a1)
{
    ScopedGLErrorCheck check("glPushGroupMarker");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%d, %p)\n", "glPushGroupMarker" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glPushGroupMarker, "Attempt to call runtime link GL function: glPushGroupMarker, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glPushGroupMarker) return;
     p_glPushGroupMarker(a0, a1);
#else
#if  defined(GL_EXT_debug_marker)
     ::glPushGroupMarkerEXT(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glPushGroupMarker, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glReadPixels(GLint  a0, GLint  a1, GLsizei  a2, GLsizei  a3, GLenum  a4, GLenum  a5, GLvoid * a6)
{
    ScopedGLErrorCheck check("glReadPixels");
    SF_DEBUG_MESSAGE8(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d, %d, %d, %x, %x, %p)\n", "glReadPixels" ,  a0, a1, a2, a3, a4, a5, a6);
     ::glReadPixels(a0, a1, a2, a3, a4, a5, a6);
}
void GLImmediate::glReleaseShaderCompiler()
{
    ScopedGLErrorCheck check("glReleaseShaderCompiler");
    SF_DEBUG_MESSAGE1(Verbosity >= Verbosity_DumpCalls, "%s(void)\n", "glReleaseShaderCompiler"  );
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glReleaseShaderCompiler, "Attempt to call runtime link GL function: glReleaseShaderCompiler, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glReleaseShaderCompiler) return;
     p_glReleaseShaderCompiler();
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glReleaseShaderCompiler();
#elif  defined(GL_VERSION_4_1)
     ::glReleaseShaderCompiler();
#else
    SF_DEBUG_ASSERT(0, "Calling function glReleaseShaderCompiler, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glRenderbufferStorage(GLenum  a0, GLenum  a1, GLsizei  a2, GLsizei  a3)
{
    ScopedGLErrorCheck check("glRenderbufferStorage");
    SF_DEBUG_MESSAGE5(Verbosity >= Verbosity_DumpCalls, "%s(%x, %x, %d, %d)\n", "glRenderbufferStorage" ,  a0, a1, a2, a3);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glRenderbufferStorage, "Attempt to call runtime link GL function: glRenderbufferStorage, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glRenderbufferStorage) return;
     p_glRenderbufferStorage(a0, a1, a2, a3);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glRenderbufferStorage(a0, a1, a2, a3);
#elif  defined(GL_VERSION_3_0)
     ::glRenderbufferStorage(a0, a1, a2, a3);
#elif  defined(GL_EXT_framebuffer_object)
     ::glRenderbufferStorageEXT(a0, a1, a2, a3);
#else
    SF_UNUSED4(a0, a1, a2, a3);
    SF_DEBUG_ASSERT(0, "Calling function glRenderbufferStorage, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glScissor(GLint  a0, GLint  a1, GLsizei  a2, GLsizei  a3)
{
    ScopedGLErrorCheck check("glScissor");
    SF_DEBUG_MESSAGE5(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d, %d, %d)\n", "glScissor" ,  a0, a1, a2, a3);
     ::glScissor(a0, a1, a2, a3);
}
void GLImmediate::glShaderSource(GLuint  a0, GLsizei  a1, GLSHADERSOURCEA2TYPE a2, const GLint * a3)
{
    ScopedGLErrorCheck check("glShaderSource");
    SF_DEBUG_MESSAGE5(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d, %p, %p)\n", "glShaderSource" ,  a0, a1, a2, a3);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glShaderSource, "Attempt to call runtime link GL function: glShaderSource, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glShaderSource) return;
     p_glShaderSource(a0, a1, const_cast<GLSHADERSOURCEA2TYPE>(a2), a3);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glShaderSource(a0, a1, const_cast<GLSHADERSOURCEA2TYPE>(a2), a3);
#elif  defined(GL_VERSION_2_0)
     ::glShaderSource(a0, a1, const_cast<GLSHADERSOURCEA2TYPE>(a2), a3);
#else
    SF_UNUSED4(a0, a1, a2, a3);
    SF_DEBUG_ASSERT(0, "Calling function glShaderSource, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glStencilFunc(GLenum  a0, GLint  a1, GLuint  a2)
{
    ScopedGLErrorCheck check("glStencilFunc");
    SF_DEBUG_MESSAGE4(Verbosity >= Verbosity_DumpCalls, "%s(%x, %d, %d)\n", "glStencilFunc" ,  a0, a1, a2);
     ::glStencilFunc(a0, a1, a2);
}
void GLImmediate::glStencilMask(GLuint  a0)
{
    ScopedGLErrorCheck check("glStencilMask");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%d)\n", "glStencilMask" ,  a0);
     ::glStencilMask(a0);
}
void GLImmediate::glStencilOp(GLenum  a0, GLenum  a1, GLenum  a2)
{
    ScopedGLErrorCheck check("glStencilOp");
    SF_DEBUG_MESSAGE4(Verbosity >= Verbosity_DumpCalls, "%s(%x, %x, %x)\n", "glStencilOp" ,  a0, a1, a2);
     ::glStencilOp(a0, a1, a2);
}
 void GLImmediate::glStringMarker(GLsizei  a0, const void * a1)
{
    ScopedGLErrorCheck check("glStringMarker");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%d, %p)\n", "glStringMarker" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glStringMarker, "Attempt to call runtime link GL function: glStringMarker, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glStringMarker) return ( void)(0);
    return p_glStringMarker(a0, a1);
#else
#if  defined(GL_GREMEDY_string_marker)
    return ::glStringMarkerGREMEDY(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glStringMarker, but no version of extension supports it.");
    return ( void)(0);
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glTexImage2D(GLenum  a0, GLint  a1, GLint  a2, GLsizei  a3, GLsizei  a4, GLint  a5, GLenum  a6, GLenum  a7, const GLvoid * a8)
{
    ScopedGLErrorCheck check("glTexImage2D");
    SF_DEBUG_MESSAGE10(Verbosity >= Verbosity_DumpCalls, "%s(%x, %d, %d, %d, %d, %d, %x, %x, %p)\n", "glTexImage2D" ,  a0, a1, a2, a3, a4, a5, a6, a7, a8);
     ::glTexImage2D(a0, a1, a2, a3, a4, a5, a6, a7, a8);
}
void GLImmediate::glTexParameteri(GLenum  a0, GLenum  a1, GLint  a2)
{
    ScopedGLErrorCheck check("glTexParameteri");
    SF_DEBUG_MESSAGE4(Verbosity >= Verbosity_DumpCalls, "%s(%x, %x, %d)\n", "glTexParameteri" ,  a0, a1, a2);
     ::glTexParameteri(a0, a1, a2);
}
void GLImmediate::glTexSubImage2D(GLenum  a0, GLint  a1, GLint  a2, GLint  a3, GLsizei  a4, GLsizei  a5, GLenum  a6, GLenum  a7, const GLvoid * a8)
{
    ScopedGLErrorCheck check("glTexSubImage2D");
    SF_DEBUG_MESSAGE10(Verbosity >= Verbosity_DumpCalls, "%s(%x, %d, %d, %d, %d, %d, %x, %x, %p)\n", "glTexSubImage2D" ,  a0, a1, a2, a3, a4, a5, a6, a7, a8);
     ::glTexSubImage2D(a0, a1, a2, a3, a4, a5, a6, a7, a8);
}
void GLImmediate::glUniform1f(GLint  a0, GLfloat  a1)
{
    ScopedGLErrorCheck check("glUniform1f");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%d, %f)\n", "glUniform1f" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glUniform1f, "Attempt to call runtime link GL function: glUniform1f, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glUniform1f) return;
     p_glUniform1f(a0, a1);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glUniform1f(a0, a1);
#elif  defined(GL_VERSION_2_0)
     ::glUniform1f(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glUniform1f, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glUniform1fv(GLint  a0, GLsizei  a1, const GLfloat * a2)
{
    ScopedGLErrorCheck check("glUniform1fv");
    SF_DEBUG_MESSAGE4(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d, %p)\n", "glUniform1fv" ,  a0, a1, a2);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glUniform1fv, "Attempt to call runtime link GL function: glUniform1fv, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glUniform1fv) return;
     p_glUniform1fv(a0, a1, a2);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glUniform1fv(a0, a1, a2);
#elif  defined(GL_VERSION_2_0)
     ::glUniform1fv(a0, a1, a2);
#else
    SF_UNUSED3(a0, a1, a2);
    SF_DEBUG_ASSERT(0, "Calling function glUniform1fv, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glUniform1i(GLint  a0, GLint  a1)
{
    ScopedGLErrorCheck check("glUniform1i");
    SF_DEBUG_MESSAGE3(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d)\n", "glUniform1i" ,  a0, a1);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glUniform1i, "Attempt to call runtime link GL function: glUniform1i, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glUniform1i) return;
     p_glUniform1i(a0, a1);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glUniform1i(a0, a1);
#elif  defined(GL_VERSION_2_0)
     ::glUniform1i(a0, a1);
#else
    SF_UNUSED2(a0, a1);
    SF_DEBUG_ASSERT(0, "Calling function glUniform1i, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glUniform1iv(GLint  a0, GLsizei  a1, const GLint * a2)
{
    ScopedGLErrorCheck check("glUniform1iv");
    SF_DEBUG_MESSAGE4(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d, %p)\n", "glUniform1iv" ,  a0, a1, a2);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glUniform1iv, "Attempt to call runtime link GL function: glUniform1iv, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glUniform1iv) return;
     p_glUniform1iv(a0, a1, a2);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glUniform1iv(a0, a1, a2);
#elif  defined(GL_VERSION_2_0)
     ::glUniform1iv(a0, a1, a2);
#else
    SF_UNUSED3(a0, a1, a2);
    SF_DEBUG_ASSERT(0, "Calling function glUniform1iv, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glUniform2f(GLint  a0, GLfloat  a1, GLfloat  a2)
{
    ScopedGLErrorCheck check("glUniform2f");
    SF_DEBUG_MESSAGE4(Verbosity >= Verbosity_DumpCalls, "%s(%d, %f, %f)\n", "glUniform2f" ,  a0, a1, a2);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glUniform2f, "Attempt to call runtime link GL function: glUniform2f, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glUniform2f) return;
     p_glUniform2f(a0, a1, a2);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glUniform2f(a0, a1, a2);
#elif  defined(GL_VERSION_2_0)
     ::glUniform2f(a0, a1, a2);
#else
    SF_UNUSED3(a0, a1, a2);
    SF_DEBUG_ASSERT(0, "Calling function glUniform2f, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glUniform2fv(GLint  a0, GLsizei  a1, const GLfloat * a2)
{
    ScopedGLErrorCheck check("glUniform2fv");
    SF_DEBUG_MESSAGE4(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d, %p)\n", "glUniform2fv" ,  a0, a1, a2);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glUniform2fv, "Attempt to call runtime link GL function: glUniform2fv, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glUniform2fv) return;
     p_glUniform2fv(a0, a1, a2);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glUniform2fv(a0, a1, a2);
#elif  defined(GL_VERSION_2_0)
     ::glUniform2fv(a0, a1, a2);
#else
    SF_UNUSED3(a0, a1, a2);
    SF_DEBUG_ASSERT(0, "Calling function glUniform2fv, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glUniform3fv(GLint  a0, GLsizei  a1, const GLfloat * a2)
{
    ScopedGLErrorCheck check("glUniform3fv");
    SF_DEBUG_MESSAGE4(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d, %p)\n", "glUniform3fv" ,  a0, a1, a2);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glUniform3fv, "Attempt to call runtime link GL function: glUniform3fv, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glUniform3fv) return;
     p_glUniform3fv(a0, a1, a2);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glUniform3fv(a0, a1, a2);
#elif  defined(GL_VERSION_2_0)
     ::glUniform3fv(a0, a1, a2);
#else
    SF_UNUSED3(a0, a1, a2);
    SF_DEBUG_ASSERT(0, "Calling function glUniform3fv, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glUniform4fv(GLint  a0, GLsizei  a1, const GLfloat * a2)
{
    ScopedGLErrorCheck check("glUniform4fv");
    SF_DEBUG_MESSAGE4(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d, %p)\n", "glUniform4fv" ,  a0, a1, a2);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glUniform4fv, "Attempt to call runtime link GL function: glUniform4fv, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glUniform4fv) return;
     p_glUniform4fv(a0, a1, a2);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glUniform4fv(a0, a1, a2);
#elif  defined(GL_VERSION_2_0)
     ::glUniform4fv(a0, a1, a2);
#else
    SF_UNUSED3(a0, a1, a2);
    SF_DEBUG_ASSERT(0, "Calling function glUniform4fv, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glUniformMatrix4fv(GLint  a0, GLsizei  a1, GLboolean  a2, const GLfloat * a3)
{
    ScopedGLErrorCheck check("glUniformMatrix4fv");
    SF_DEBUG_MESSAGE5(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d, %d, %p)\n", "glUniformMatrix4fv" ,  a0, a1, a2, a3);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glUniformMatrix4fv, "Attempt to call runtime link GL function: glUniformMatrix4fv, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glUniformMatrix4fv) return;
     p_glUniformMatrix4fv(a0, a1, a2, a3);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glUniformMatrix4fv(a0, a1, a2, a3);
#elif  defined(GL_VERSION_2_0)
     ::glUniformMatrix4fv(a0, a1, a2, a3);
#else
    SF_UNUSED4(a0, a1, a2, a3);
    SF_DEBUG_ASSERT(0, "Calling function glUniformMatrix4fv, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

GLboolean GLImmediate::glUnmapBuffer(GLenum  a0)
{
    ScopedGLErrorCheck check("glUnmapBuffer");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%x)\n", "glUnmapBuffer" ,  a0);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glUnmapBuffer, "Attempt to call runtime link GL function: glUnmapBuffer, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glUnmapBuffer) return (GLboolean)(0);
    return p_glUnmapBuffer(a0);
#else
#if  defined(GL_ARB_vertex_buffer_object)
    return ::glUnmapBufferARB(a0);
#elif  defined(GL_ES_VERSION_3_0)
    return ::glUnmapBuffer(a0);
#elif  defined(GL_VERSION_1_5)
    return ::glUnmapBuffer(a0);
#elif  defined(GL_OES_mapbuffer)
    return ::glUnmapBufferOES(a0);
#else
    SF_UNUSED1(a0);
    SF_DEBUG_ASSERT(0, "Calling function glUnmapBuffer, but no version of extension supports it.");
    return (GLboolean)(0);
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glUseProgram(GLuint  a0)
{
    ScopedGLErrorCheck check("glUseProgram");
    SF_DEBUG_MESSAGE2(Verbosity >= Verbosity_DumpCalls, "%s(%d)\n", "glUseProgram" ,  a0);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glUseProgram, "Attempt to call runtime link GL function: glUseProgram, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glUseProgram) return;
     p_glUseProgram(a0);
#else
#if  defined(GL_ES_VERSION_2_0)
     ::glUseProgram(a0);
#elif  defined(GL_VERSION_2_0)
     ::glUseProgram(a0);
#else
    SF_UNUSED1(a0);
    SF_DEBUG_ASSERT(0, "Calling function glUseProgram, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glUseProgramStages(GLuint  a0, GLbitfield  a1, GLuint  a2)
{
    ScopedGLErrorCheck check("glUseProgramStages");
    SF_DEBUG_MESSAGE4(Verbosity >= Verbosity_DumpCalls, "%s(%d, %x, %d)\n", "glUseProgramStages" ,  a0, a1, a2);
#if defined(SF_GL_RUNTIME_LINK)
    SF_DEBUG_ASSERT(p_glUseProgramStages, "Attempt to call runtime link GL function: glUseProgramStages, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glUseProgramStages) return;
     p_glUseProgramStages(a0, a1, a2);
#else
#if  defined(GL_VERSION_4_1)
     ::glUseProgramStages(a0, a1, a2);
#elif  defined(GL_ARB_separate_shader_objects)
     ::glUseProgramStages(a0, a1, a2);
#elif  defined(GL_EXT_separate_shader_objects)
     ::glUseProgramStagesEXT(a0, a1, a2);
#else
    SF_UNUSED3(a0, a1, a2);
    SF_DEBUG_ASSERT(0, "Calling function glUseProgramStages, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glVertexAttribPointer(GLuint  a0, GLint  a1, GLenum  a2, GLboolean  a3, GLsizei  a4, const void * a5)
{
    ScopedGLErrorCheck check("glVertexAttribPointer");
    SF_DEBUG_MESSAGE7(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d, %x, %d, %d, %p)\n", "glVertexAttribPointer" ,  a0, a1, a2, a3, a4, a5);
#if defined(SF_GL_RUNTIME_LINK)&& (!defined(SF_RENDER_GLES))
    SF_DEBUG_ASSERT(p_glVertexAttribPointer, "Attempt to call runtime link GL function: glVertexAttribPointer, but it was NULL. Either GLImmediate::Init() has not been called, or the function is not available." );
    if (!p_glVertexAttribPointer) return;
     p_glVertexAttribPointer(a0, a1, a2, a3, a4, a5);
#else
#if  defined(GL_ARB_vertex_program)
     ::glVertexAttribPointerARB(a0, a1, a2, a3, a4, a5);
#elif  defined(GL_ES_VERSION_2_0)
     ::glVertexAttribPointer(a0, a1, a2, a3, a4, a5);
#elif  defined(GL_VERSION_2_0)
     ::glVertexAttribPointer(a0, a1, a2, a3, a4, a5);
#elif  defined(GL_NV_vertex_program)
     ::glVertexAttribPointerNV(a0, a1, a2, a3, a4, a5);
#else
    SF_UNUSED6(a0, a1, a2, a3, a4, a5);
    SF_DEBUG_ASSERT(0, "Calling function glVertexAttribPointer, but no version of extension supports it.");
    return;
#endif
#endif // SF_GL_RUNTIME_LINK
}

void GLImmediate::glViewport(GLint  a0, GLint  a1, GLsizei  a2, GLsizei  a3)
{
    ScopedGLErrorCheck check("glViewport");
    SF_DEBUG_MESSAGE5(Verbosity >= Verbosity_DumpCalls, "%s(%d, %d, %d, %d)\n", "glViewport" ,  a0, a1, a2, a3);
     ::glViewport(a0, a1, a2, a3);
}


}}} // Scaleform::Render::GL 

