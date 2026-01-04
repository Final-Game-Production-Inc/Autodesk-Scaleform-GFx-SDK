/**********************************************************************

Filename    :   GL_Extensions.h
Content     :   GL  extension header.
Created     :   Automatically generated.
Authors     :   Extensions.pl

Copyright   :   (c) 2001-2014 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#ifndef INC_SF_Render_GL_Extensions_H
#define INC_SF_Render_GL_Extensions_H

#include "Kernel/SF_Debug.h"
#include "Render/GL/GL_Common.h"
#include "Kernel/SF_String.h"
#include "Render/Render_GraphicsDevice.h"

typedef void (GL_APIENTRYP PFNGLACTIVETEXTUREPROC) (GLenum );
typedef void (GL_APIENTRYP PFNGLATTACHSHADERPROC) (GLuint , GLuint );
typedef void (GL_APIENTRYP PFNGLBINDATTRIBLOCATIONPROC) (GLuint , GLuint , const GLchar *);
typedef void (GL_APIENTRYP PFNGLBINDBUFFERPROC) (GLenum , GLuint );
typedef  void (GL_APIENTRYP PFNGLBINDFRAGDATALOCATIONPROC) (GLuint , GLuint , const GLchar *);
typedef void (GL_APIENTRYP PFNGLBINDFRAMEBUFFERPROC) (GLenum , GLuint );
typedef void (GL_APIENTRYP PFNGLBINDPROGRAMPIPELINEPROC) (GLuint );
typedef void (GL_APIENTRYP PFNGLBINDRENDERBUFFERPROC) (GLenum , GLuint );
typedef void (GL_APIENTRYP PFNGLBINDTEXTUREPROC) (GLenum , GLuint );
typedef void (GL_APIENTRYP PFNGLBINDVERTEXARRAYPROC) (GLuint );
typedef void (GL_APIENTRYP PFNGLBLENDEQUATIONPROC) (GLenum );
typedef void (GL_APIENTRYP PFNGLBLENDEQUATIONSEPARATEPROC) (GLenum , GLenum );
typedef void (GL_APIENTRYP PFNGLBLENDFUNCPROC) (GLenum , GLenum );
typedef void (GL_APIENTRYP PFNGLBLENDFUNCSEPARATEPROC) (GLenum , GLenum , GLenum , GLenum );
typedef void (GL_APIENTRYP PFNGLBUFFERDATAPROC) (GLenum , GLsizeiptr , const void *, GLenum );
typedef void (GL_APIENTRYP PFNGLBUFFERSUBDATAPROC) (GLenum , GLintptr , GLsizeiptr , const void *);
typedef GLenum (GL_APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSPROC) (GLenum );
typedef void (GL_APIENTRYP PFNGLCLEARPROC) (GLbitfield );
typedef void (GL_APIENTRYP PFNGLCLEARCOLORPROC) (GLclampf , GLclampf , GLclampf , GLclampf );
typedef GLenum (GL_APIENTRYP PFNGLCLIENTWAITSYNCPROC) (GLsync , GLbitfield , GLuint64 );
typedef void (GL_APIENTRYP PFNGLCOLORMASKPROC) (GLboolean , GLboolean , GLboolean , GLboolean );
typedef void (GL_APIENTRYP PFNGLCOMPILESHADERPROC) (GLuint );
typedef void (GL_APIENTRYP PFNGLCOMPRESSEDTEXIMAGE2DPROC) (GLenum , GLint , GLenum , GLsizei , GLsizei , GLint , GLsizei , const void *);
typedef GLuint (GL_APIENTRYP PFNGLCREATEPROGRAMPROC) (void);
typedef GLuint (GL_APIENTRYP PFNGLCREATESHADERPROC) (GLenum );
typedef void (GL_APIENTRYP PFNGLDEBUGMESSAGECALLBACKPROC) (GLDEBUGPROCKHR , const void *);
typedef void (GL_APIENTRYP PFNGLDEBUGMESSAGECONTROLPROC) (GLenum , GLenum , GLenum , GLsizei , const GLuint *, GLboolean );
typedef void (GL_APIENTRYP PFNGLDELETEBUFFERSPROC) (GLsizei , const GLuint *);
typedef void (GL_APIENTRYP PFNGLDELETEFRAMEBUFFERSPROC) (GLsizei , const GLuint *);
typedef void (GL_APIENTRYP PFNGLDELETEPROGRAMPROC) (GLuint );
typedef void (GL_APIENTRYP PFNGLDELETEPROGRAMPIPELINESPROC) (GLsizei , const GLuint *);
typedef void (GL_APIENTRYP PFNGLDELETERENDERBUFFERSPROC) (GLsizei , const GLuint *);
typedef void (GL_APIENTRYP PFNGLDELETESHADERPROC) (GLuint );
typedef void (GL_APIENTRYP PFNGLDELETESYNCPROC) (GLsync );
typedef void (GL_APIENTRYP PFNGLDELETETEXTURESPROC) (GLsizei , const GLuint *);
typedef void (GL_APIENTRYP PFNGLDELETEVERTEXARRAYSPROC) (GLsizei , const GLuint *);
typedef void (GL_APIENTRYP PFNGLDEPTHFUNCPROC) (GLenum );
typedef void (GL_APIENTRYP PFNGLDEPTHMASKPROC) (GLboolean );
typedef void (GL_APIENTRYP PFNGLDISABLEPROC) (GLenum );
typedef void (GL_APIENTRYP PFNGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint );
typedef void (GL_APIENTRYP PFNGLDRAWARRAYSPROC) (GLenum , GLint , GLsizei );
typedef void (GL_APIENTRYP PFNGLDRAWELEMENTSPROC) (GLenum , GLsizei , GLenum , const GLvoid *);
typedef void (GL_APIENTRYP PFNGLDRAWELEMENTSINSTANCEDPROC) (GLenum , GLsizei , GLenum , const void *, GLsizei );
typedef void (GL_APIENTRYP PFNGLENABLEPROC) (GLenum );
typedef void (GL_APIENTRYP PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint );
typedef GLsync (GL_APIENTRYP PFNGLFENCESYNCPROC) (GLenum , GLbitfield );
typedef void (GL_APIENTRYP PFNGLFLUSHPROC) (void);
typedef void (GL_APIENTRYP PFNGLFLUSHMAPPEDBUFFERRANGEPROC) (GLenum , GLintptr , GLsizeiptr );
typedef void (GL_APIENTRYP PFNGLFRAMEBUFFERRENDERBUFFERPROC) (GLenum , GLenum , GLenum , GLuint );
typedef void (GL_APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DPROC) (GLenum , GLenum , GLenum , GLuint , GLint );
typedef void (GL_APIENTRYP PFNGLGENBUFFERSPROC) (GLsizei , GLuint *);
typedef void (GL_APIENTRYP PFNGLGENFRAMEBUFFERSPROC) (GLsizei , GLuint *);
typedef void (GL_APIENTRYP PFNGLGENPROGRAMPIPELINESPROC) (GLsizei , GLuint *);
typedef void (GL_APIENTRYP PFNGLGENRENDERBUFFERSPROC) (GLsizei , GLuint *);
typedef void (GL_APIENTRYP PFNGLGENTEXTURESPROC) (GLsizei , GLuint *);
typedef void (GL_APIENTRYP PFNGLGENVERTEXARRAYSPROC) (GLsizei , GLuint *);
typedef void (GL_APIENTRYP PFNGLGENERATEMIPMAPPROC) (GLenum );
typedef void (GL_APIENTRYP PFNGLGETACTIVEUNIFORMPROC) (GLuint , GLuint , GLsizei , GLsizei *, GLint *, GLenum *, GLchar *);
typedef GLenum (GL_APIENTRYP PFNGLGETERRORPROC) (void);
typedef void (GL_APIENTRYP PFNGLGETFLOATVPROC) (GLenum , GLfloat *);
typedef GLint (GL_APIENTRYP PFNGLGETFRAGDATALOCATIONPROC) (GLuint , const GLchar *);
typedef void (GL_APIENTRYP PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) (GLenum , GLenum , GLenum , GLint *);
typedef void (GL_APIENTRYP PFNGLGETINTEGERVPROC) (GLenum , GLint *);
typedef void (GL_APIENTRYP PFNGLGETPROGRAMBINARYPROC) (GLuint , GLsizei , GLsizei *, GLenum *, void *);
typedef void (GL_APIENTRYP PFNGLGETPROGRAMINFOLOGPROC) (GLuint , GLsizei , GLsizei *, GLchar *);
typedef void (GL_APIENTRYP PFNGLGETPROGRAMIVPROC) (GLuint , GLenum , GLint *);
typedef void (GL_APIENTRYP PFNGLGETRENDERBUFFERPARAMETERIVPROC) (GLenum , GLenum , GLint *);
typedef void (GL_APIENTRYP PFNGLGETSHADERINFOLOGPROC) (GLuint , GLsizei , GLsizei *, GLchar *);
typedef void (GL_APIENTRYP PFNGLGETSHADERIVPROC) (GLuint , GLenum , GLint *);
typedef const GLubyte * (GL_APIENTRYP PFNGLGETSTRINGPROC) (GLenum );
typedef const GLubyte * (GL_APIENTRYP PFNGLGETSTRINGIPROC) (GLenum , GLuint );
typedef void (GL_APIENTRYP PFNGLGETSYNCIVPROC) (GLsync , GLenum , GLsizei , GLsizei *, GLint *);
typedef void (GL_APIENTRYP PFNGLGETTEXIMAGEPROC) (GLenum , GLint , GLenum , GLenum , GLvoid *);
typedef void (GL_APIENTRYP PFNGLGETTEXLEVELPARAMETERIVPROC) (GLenum , GLint , GLenum , GLint *);
typedef void (GL_APIENTRYP PFNGLGETTEXPARAMETERIVPROC) (GLenum , GLenum , GLint *);
typedef GLint (GL_APIENTRYP PFNGLGETUNIFORMLOCATIONPROC) (GLuint , const GLchar *);
typedef GLboolean (GL_APIENTRYP PFNGLISFRAMEBUFFERPROC) (GLuint );
typedef GLboolean (GL_APIENTRYP PFNGLISPROGRAMPROC) (GLuint );
typedef GLboolean (GL_APIENTRYP PFNGLISRENDERBUFFERPROC) (GLuint );
typedef void (GL_APIENTRYP PFNGLLINKPROGRAMPROC) (GLuint );
typedef void * (GL_APIENTRYP PFNGLMAPBUFFERPROC) (GLenum , GLenum );
typedef void * (GL_APIENTRYP PFNGLMAPBUFFERRANGEPROC) (GLenum , GLintptr , GLsizeiptr , GLbitfield );
typedef void (GL_APIENTRYP PFNGLPIXELSTOREIPROC) (GLenum , GLint );
typedef void (GL_APIENTRYP PFNGLPOLYGONMODEPROC) (GLenum , GLenum );
typedef void (GL_APIENTRYP PFNGLPOPGROUPMARKERPROC) (void);
typedef void (GL_APIENTRYP PFNGLPROGRAMBINARYPROC) (GLuint , GLenum , const void *, GLint );
typedef void (GL_APIENTRYP PFNGLPROGRAMPARAMETERIPROC) (GLuint , GLenum , GLint );
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM1FVPROC) (GLuint , GLint , GLsizei , const GLfloat *);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM1IVPROC) (GLuint , GLint , GLsizei , const GLint *);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM2FVPROC) (GLuint , GLint , GLsizei , const GLfloat *);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM3FVPROC) (GLuint , GLint , GLsizei , const GLfloat *);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORM4FVPROC) (GLuint , GLint , GLsizei , const GLfloat *);
typedef void (GL_APIENTRYP PFNGLPROGRAMUNIFORMMATRIX4FVPROC) (GLuint , GLint , GLsizei , GLboolean , const GLfloat *);
typedef void (GL_APIENTRYP PFNGLPUSHGROUPMARKERPROC) (GLsizei , const GLchar *);
typedef void (GL_APIENTRYP PFNGLREADPIXELSPROC) (GLint , GLint , GLsizei , GLsizei , GLenum , GLenum , GLvoid *);
typedef void (GL_APIENTRYP PFNGLRELEASESHADERCOMPILERPROC) (void);
typedef void (GL_APIENTRYP PFNGLRENDERBUFFERSTORAGEPROC) (GLenum , GLenum , GLsizei , GLsizei );
typedef void (GL_APIENTRYP PFNGLSCISSORPROC) (GLint , GLint , GLsizei , GLsizei );
typedef void (GL_APIENTRYP PFNGLSHADERSOURCEPROC) (GLuint , GLsizei , GLSHADERSOURCEA2TYPE, const GLint *);
typedef void (GL_APIENTRYP PFNGLSTENCILFUNCPROC) (GLenum , GLint , GLuint );
typedef void (GL_APIENTRYP PFNGLSTENCILMASKPROC) (GLuint );
typedef void (GL_APIENTRYP PFNGLSTENCILOPPROC) (GLenum , GLenum , GLenum );
typedef  void (GL_APIENTRYP PFNGLSTRINGMARKERPROC) (GLsizei , const void *);
typedef void (GL_APIENTRYP PFNGLTEXIMAGE2DPROC) (GLenum , GLint , GLint , GLsizei , GLsizei , GLint , GLenum , GLenum , const GLvoid *);
typedef void (GL_APIENTRYP PFNGLTEXPARAMETERIPROC) (GLenum , GLenum , GLint );
typedef void (GL_APIENTRYP PFNGLTEXSUBIMAGE2DPROC) (GLenum , GLint , GLint , GLint , GLsizei , GLsizei , GLenum , GLenum , const GLvoid *);
typedef void (GL_APIENTRYP PFNGLUNIFORM1FPROC) (GLint , GLfloat );
typedef void (GL_APIENTRYP PFNGLUNIFORM1FVPROC) (GLint , GLsizei , const GLfloat *);
typedef void (GL_APIENTRYP PFNGLUNIFORM1IPROC) (GLint , GLint );
typedef void (GL_APIENTRYP PFNGLUNIFORM1IVPROC) (GLint , GLsizei , const GLint *);
typedef void (GL_APIENTRYP PFNGLUNIFORM2FPROC) (GLint , GLfloat , GLfloat );
typedef void (GL_APIENTRYP PFNGLUNIFORM2FVPROC) (GLint , GLsizei , const GLfloat *);
typedef void (GL_APIENTRYP PFNGLUNIFORM3FVPROC) (GLint , GLsizei , const GLfloat *);
typedef void (GL_APIENTRYP PFNGLUNIFORM4FVPROC) (GLint , GLsizei , const GLfloat *);
typedef void (GL_APIENTRYP PFNGLUNIFORMMATRIX4FVPROC) (GLint , GLsizei , GLboolean , const GLfloat *);
typedef GLboolean (GL_APIENTRYP PFNGLUNMAPBUFFERPROC) (GLenum );
typedef void (GL_APIENTRYP PFNGLUSEPROGRAMPROC) (GLuint );
typedef void (GL_APIENTRYP PFNGLUSEPROGRAMSTAGESPROC) (GLuint , GLbitfield , GLuint );
typedef void (GL_APIENTRYP PFNGLVERTEXATTRIBPOINTERPROC) (GLuint , GLint , GLenum , GLboolean , GLsizei , const void *);
typedef void (GL_APIENTRYP PFNGLVIEWPORTPROC) (GLint , GLint , GLsizei , GLsizei );
#ifndef GL_RENDERBUFFER_FREE_MEMORY
#define GL_RENDERBUFFER_FREE_MEMORY                                  0x000087FD
#endif
#ifndef GL_VERTEX_PROGRAM_TWO_SIDE
#define GL_VERTEX_PROGRAM_TWO_SIDE                                   0x00008643
#endif
#ifndef GL_COLOR_ATTACHMENT14
#define GL_COLOR_ATTACHMENT14                                        0x00008CEE
#endif
#ifndef GL_COLOR_TABLE
#define GL_COLOR_TABLE                                               0x000080D0
#endif
#ifndef GL_FLOAT_MAT4x3
#define GL_FLOAT_MAT4x3                                              0x00008B6A
#endif
#ifndef GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL
#define GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL                        0x000087F1
#endif
#ifndef GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH
#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH                          0x00008243
#endif
#ifndef GL_TRANSLATE_3D
#define GL_TRANSLATE_3D                                              0x00009091
#endif
#ifndef GL_TEXTURE_2D_STACK
#define GL_TEXTURE_2D_STACK                                          0x0000875A
#endif
#ifndef GL_INVALID_INDEX
#define GL_INVALID_INDEX                                             0xFFFFFFFF
#endif
#ifndef GL_EVAL_VERTEX_ATTRIB14
#define GL_EVAL_VERTEX_ATTRIB14                                      0x000086D4
#endif
#ifndef GL_TEXTURE_BLUE_TYPE
#define GL_TEXTURE_BLUE_TYPE                                         0x00008C12
#endif
#ifndef GL_PACK_INVERT
#define GL_PACK_INVERT                                               0x00008758
#endif
#ifndef GL_NUM_PASSES
#define GL_NUM_PASSES                                                0x00008970
#endif
#ifndef GL_MAP2_VERTEX_ATTRIB13_4
#define GL_MAP2_VERTEX_ATTRIB13_4                                    0x0000867D
#endif
#ifndef GL_LUMINANCE12
#define GL_LUMINANCE12                                               0x00008041
#endif
#ifndef GL_CON_19
#define GL_CON_19                                                    0x00008954
#endif
#ifndef GL_PROGRAM_NATIVE_INSTRUCTIONS
#define GL_PROGRAM_NATIVE_INSTRUCTIONS                               0x000088A2
#endif
#ifndef GL_ONE_MINUS_CONSTANT_ALPHA
#define GL_ONE_MINUS_CONSTANT_ALPHA                                  0x00008004
#endif
#ifndef GL_VERTEX_BLEND
#define GL_VERTEX_BLEND                                              0x000086A7
#endif
#ifndef GL_SRGB_DECODE
#define GL_SRGB_DECODE                                               0x00008299
#endif
#ifndef GL_RELATIVE_SMALL_CCW_ARC_TO
#define GL_RELATIVE_SMALL_CCW_ARC_TO                                 0x00000013
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD17
#define GL_OUTPUT_TEXTURE_COORD17                                    0x000087AE
#endif
#ifndef GL_HI_BIAS
#define GL_HI_BIAS                                                   0x00008714
#endif
#ifndef GL_SAMPLER
#define GL_SAMPLER                                                   0x000082E6
#endif
#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE                                               0x0000809D
#endif
#ifndef GL_POST_COLOR_MATRIX_GREEN_BIAS
#define GL_POST_COLOR_MATRIX_GREEN_BIAS                              0x000080B9
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_4x3x3
#define GL_COMPRESSED_RGBA_ASTC_4x3x3                                0x000093C1
#endif
#ifndef GL_DUAL_ALPHA16
#define GL_DUAL_ALPHA16                                              0x00008113
#endif
#ifndef GL_PURGEABLE
#define GL_PURGEABLE                                                 0x00008A1D
#endif
#ifndef GL_STRICT_LIGHTING_HINT
#define GL_STRICT_LIGHTING_HINT                                      0x0001A217
#endif
#ifndef GL_CONVOLUTION_1D
#define GL_CONVOLUTION_1D                                            0x00008010
#endif
#ifndef GL_FONT_HEIGHT_BIT
#define GL_FONT_HEIGHT_BIT                                           0x00800000
#endif
#ifndef GL_FRAMEBUFFER_COMPLETE
#define GL_FRAMEBUFFER_COMPLETE                                      0x00008CD5
#endif
#ifndef GL_MAX_VERTEX_UNITS
#define GL_MAX_VERTEX_UNITS                                          0x000086A4
#endif
#ifndef GL_NEGATE_BIT
#define GL_NEGATE_BIT                                                0x00000004
#endif
#ifndef GL_COMPRESSED_LUMINANCE_ALPHA_LATC2
#define GL_COMPRESSED_LUMINANCE_ALPHA_LATC2                          0x00008C72
#endif
#ifndef GL_PATH_INITIAL_END_CAP
#define GL_PATH_INITIAL_END_CAP                                      0x00009077
#endif
#ifndef GL_QUERY_DEPTH_PASS_EVENT_BIT
#define GL_QUERY_DEPTH_PASS_EVENT_BIT                                0x00000001
#endif
#ifndef GL_MAX_COMBINED_UNIFORM_BLOCKS
#define GL_MAX_COMBINED_UNIFORM_BLOCKS                               0x00008A2E
#endif
#ifndef GL_DOUBLE
#define GL_DOUBLE                                                    0x00000000
#endif
#ifndef GL_PROXY_TEXTURE_1D_STACK
#define GL_PROXY_TEXTURE_1D_STACK                                    0x0000875B
#endif
#ifndef GL_DECODE
#define GL_DECODE                                                    0x00008A49
#endif
#ifndef GL_INDEX_ARRAY
#define GL_INDEX_ARRAY                                               0x00008077
#endif
#ifndef GL_WRITE_PIXEL_DATA_RANGE_LENGTH
#define GL_WRITE_PIXEL_DATA_RANGE_LENGTH                             0x0000887A
#endif
#ifndef GL_VERTEX_WEIGHT_ARRAY_TYPE
#define GL_VERTEX_WEIGHT_ARRAY_TYPE                                  0x0000850E
#endif
#ifndef GL_MAX_ACTIVE_LIGHTS
#define GL_MAX_ACTIVE_LIGHTS                                         0x00008405
#endif
#ifndef GL_MAP1_VERTEX_ATTRIB12_4
#define GL_MAP1_VERTEX_ATTRIB12_4                                    0x0000866C
#endif
#ifndef GL_PIXEL_BUFFER_BARRIER_BIT
#define GL_PIXEL_BUFFER_BARRIER_BIT                                  0x00000080
#endif
#ifndef GL_MODELVIEW6
#define GL_MODELVIEW6                                                0x00008726
#endif
#ifndef GL_NATIVE_GRAPHICS_END_HINT
#define GL_NATIVE_GRAPHICS_END_HINT                                  0x0001A204
#endif
#ifndef GL_STENCIL_INDEX16
#define GL_STENCIL_INDEX16                                           0x00008D49
#endif
#ifndef GL_BUMP_TARGET
#define GL_BUMP_TARGET                                               0x0000877C
#endif
#ifndef GL_INTENSITY8I
#define GL_INTENSITY8I                                               0x00008D91
#endif
#ifndef GL_PACK_CMYK_HINT
#define GL_PACK_CMYK_HINT                                            0x0000800E
#endif
#ifndef GL_SIGNED_RGB
#define GL_SIGNED_RGB                                                0x000086FE
#endif
#ifndef GL_FONT_UNDERLINE_POSITION_BIT
#define GL_FONT_UNDERLINE_POSITION_BIT                               0x04000000
#endif
#ifndef GL_LIGHTEN
#define GL_LIGHTEN                                                   0x00009298
#endif
#ifndef GL_COMPRESSED_RED_GREEN_RGTC2
#define GL_COMPRESSED_RED_GREEN_RGTC2                                0x00008DBD
#endif
#ifndef GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET                 0x00008CD4
#endif
#ifndef GL_FOG_DISTANCE_MODE
#define GL_FOG_DISTANCE_MODE                                         0x0000855A
#endif
#ifndef GL_MIRROR_CLAMP_TO_EDGE
#define GL_MIRROR_CLAMP_TO_EDGE                                      0x00008743
#endif
#ifndef GL_DEPTH_BUFFER_BIT6
#define GL_DEPTH_BUFFER_BIT6                                         0x00004000
#endif
#ifndef GL_TRANSPOSE_AFFINE_3D
#define GL_TRANSPOSE_AFFINE_3D                                       0x00009098
#endif
#ifndef GL_MAX_PROGRAM_TEX_INDIRECTIONS
#define GL_MAX_PROGRAM_TEX_INDIRECTIONS                              0x0000880D
#endif
#ifndef GL_COMBINER_BIAS
#define GL_COMBINER_BIAS                                             0x00008549
#endif
#ifndef GL_MAP_ATTRIB_V_ORDER
#define GL_MAP_ATTRIB_V_ORDER                                        0x000086C4
#endif
#ifndef GL_MINMAX
#define GL_MINMAX                                                    0x0000802E
#endif
#ifndef GL_PROGRAM_NATIVE_ADDRESS_REGISTERS
#define GL_PROGRAM_NATIVE_ADDRESS_REGISTERS                          0x000088B2
#endif
#ifndef GL_PROGRAM_PARAMETERS
#define GL_PROGRAM_PARAMETERS                                        0x000088A8
#endif
#ifndef GL_VIDEO_CAPTURE_FIELD_LOWER_HEIGHT
#define GL_VIDEO_CAPTURE_FIELD_LOWER_HEIGHT                          0x0000903B
#endif
#ifndef GL_DISCRETE
#define GL_DISCRETE                                                  0x00009006
#endif
#ifndef GL_FRAGMENT_LIGHT6
#define GL_FRAGMENT_LIGHT6                                           0x00008412
#endif
#ifndef GL_VERTEX_ARRAY_RANGE
#define GL_VERTEX_ARRAY_RANGE                                        0x0000851D
#endif
#ifndef GL_SAMPLER_1D
#define GL_SAMPLER_1D                                                0x00008B5D
#endif
#ifndef GL_PROXY_TEXTURE_1D
#define GL_PROXY_TEXTURE_1D                                          0x00008063
#endif
#ifndef GL_TEXTURE_HEIGHT
#define GL_TEXTURE_HEIGHT                                            0x00008BD3
#endif
#ifndef GL_LIGHT_MODEL_COLOR_CONTROL
#define GL_LIGHT_MODEL_COLOR_CONTROL                                 0x000081F8
#endif
#ifndef GL_MAX_GEOMETRY_VARYING_COMPONENTS
#define GL_MAX_GEOMETRY_VARYING_COMPONENTS                           0x00008DDD
#endif
#ifndef GL_SIGNED_INTENSITY8
#define GL_SIGNED_INTENSITY8                                         0x00008708
#endif
#ifndef GL_MAP_WRITE_BIT
#define GL_MAP_WRITE_BIT                                             0x00000002
#endif
#ifndef GL_TEXTURE_INTERNAL_FORMAT
#define GL_TEXTURE_INTERNAL_FORMAT                                   0x00008BD5
#endif
#ifndef GL_TEXTURE_COMPARE_MODE
#define GL_TEXTURE_COMPARE_MODE                                      0x0000884C
#endif
#ifndef GL_CIRCULAR_CCW_ARC_TO
#define GL_CIRCULAR_CCW_ARC_TO                                       0x000000F8
#endif
#ifndef GL_CALLIGRAPHIC_FRAGMENT
#define GL_CALLIGRAPHIC_FRAGMENT                                     0x00008183
#endif
#ifndef GL_DOUBLE_MAT4x3
#define GL_DOUBLE_MAT4x3                                             0x00008F4E
#endif
#ifndef GL_MULTISAMPLE_COVERAGE_MODES
#define GL_MULTISAMPLE_COVERAGE_MODES                                0x00008E12
#endif
#ifndef GL_CONVOLUTION_BORDER_MODE
#define GL_CONVOLUTION_BORDER_MODE                                   0x00008013
#endif
#ifndef GL_PARALLEL_ARRAYS
#define GL_PARALLEL_ARRAYS                                           0x000083F4
#endif
#ifndef GL_QUAD_LUMINANCE8
#define GL_QUAD_LUMINANCE8                                           0x00008121
#endif
#ifndef GL_MAX_PROGRAM_EXEC_INSTRUCTIONS
#define GL_MAX_PROGRAM_EXEC_INSTRUCTIONS                             0x000088F4
#endif
#ifndef GL_COMPRESSED_RGBA_PVRTC_4BPPV2
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV2                              0x00009138
#endif
#ifndef GL_VERTEX_ID
#define GL_VERTEX_ID                                                 0x00008C7B
#endif
#ifndef GL_FLOAT_MAT3x4
#define GL_FLOAT_MAT3x4                                              0x00008B68
#endif
#ifndef GL_FLOAT16_VEC2
#define GL_FLOAT16_VEC2                                              0x00008FF9
#endif
#ifndef GL_INT_VEC4
#define GL_INT_VEC4                                                  0x00008B55
#endif
#ifndef GL_ALPHA4
#define GL_ALPHA4                                                    0x0000803B
#endif
#ifndef GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS
#define GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS                           0x00008808
#endif
#ifndef GL_UNSIGNED_INT64_VEC3
#define GL_UNSIGNED_INT64_VEC3                                       0x00008FF6
#endif
#ifndef GL_DRAW_BUFFER14
#define GL_DRAW_BUFFER14                                             0x00008833
#endif
#ifndef GL_MATRIX_INDEX_ARRAY_SIZE
#define GL_MATRIX_INDEX_ARRAY_SIZE                                   0x00008846
#endif
#ifndef GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS
#define GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS                      0x000088B3
#endif
#ifndef GL_TRANSPOSE_TEXTURE_MATRIX
#define GL_TRANSPOSE_TEXTURE_MATRIX                                  0x000084E5
#endif
#ifndef GL_LINEAR_DETAIL_COLOR
#define GL_LINEAR_DETAIL_COLOR                                       0x00008099
#endif
#ifndef GL_TRANSPOSE_PROJECTION_MATRIX
#define GL_TRANSPOSE_PROJECTION_MATRIX                               0x000084E4
#endif
#ifndef GL_PERFQUERY_DONOT_FLUSH
#define GL_PERFQUERY_DONOT_FLUSH                                     0x000083F9
#endif
#ifndef GL_FONT_MAX_ADVANCE_WIDTH_BIT
#define GL_FONT_MAX_ADVANCE_WIDTH_BIT                                0x01000000
#endif
#ifndef GL_ZERO
#define GL_ZERO                                                      0x000087DD
#endif
#ifndef GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE                        0x00008D56
#endif
#ifndef GL_PACK_SKIP_ROWS
#define GL_PACK_SKIP_ROWS                                            0x00000D03
#endif
#ifndef GL_BLEND_OVERLAP
#define GL_BLEND_OVERLAP                                             0x00009281
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_12x12
#define GL_COMPRESSED_RGBA_ASTC_12x12                                0x000093BD
#endif
#ifndef GL_TEXTURE_BINDING_CUBE_MAP_ARRAY
#define GL_TEXTURE_BINDING_CUBE_MAP_ARRAY                            0x0000900A
#endif
#ifndef GL_TEXTURE_MAX_LOD
#define GL_TEXTURE_MAX_LOD                                           0x0000813B
#endif
#ifndef GL_FRAGMENT_PROGRAM_BINDING
#define GL_FRAGMENT_PROGRAM_BINDING                                  0x00008873
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY15
#define GL_VERTEX_ATTRIB_ARRAY15                                     0x0000865F
#endif
#ifndef GL_IMAGE_CUBIC_WEIGHT
#define GL_IMAGE_CUBIC_WEIGHT                                        0x0000815E
#endif
#ifndef GL_MAP2_VERTEX_ATTRIB1_4
#define GL_MAP2_VERTEX_ATTRIB1_4                                     0x00008671
#endif
#ifndef GL_TEXTURE_GREEN_SIZE
#define GL_TEXTURE_GREEN_SIZE                                        0x0000805D
#endif
#ifndef GL_LAST_VIDEO_CAPTURE_STATUS
#define GL_LAST_VIDEO_CAPTURE_STATUS                                 0x00009027
#endif
#ifndef GL_PROXY_TEXTURE_CUBE_MAP
#define GL_PROXY_TEXTURE_CUBE_MAP                                    0x0000851B
#endif
#ifndef GL_PIXEL_TILE_GRID_HEIGHT
#define GL_PIXEL_TILE_GRID_HEIGHT                                    0x00008143
#endif
#ifndef GL_LUMINANCE32F
#define GL_LUMINANCE32F                                              0x00008818
#endif
#ifndef GL_MAP1_VERTEX_ATTRIB5_4
#define GL_MAP1_VERTEX_ATTRIB5_4                                     0x00008665
#endif
#ifndef GL_TRANSLATE_X
#define GL_TRANSLATE_X                                               0x0000908E
#endif
#ifndef GL_DRAW_BUFFER8
#define GL_DRAW_BUFFER8                                              0x0000882D
#endif
#ifndef GL_MATRIX12
#define GL_MATRIX12                                                  0x000088CC
#endif
#ifndef GL_MAX_VERTEX_BINDABLE_UNIFORMS
#define GL_MAX_VERTEX_BINDABLE_UNIFORMS                              0x00008DE2
#endif
#ifndef GL_INTENSITY4
#define GL_INTENSITY4                                                0x0000804A
#endif
#ifndef GL_TEXTURE_FREE_MEMORY
#define GL_TEXTURE_FREE_MEMORY                                       0x000087FC
#endif
#ifndef GL_CON_0
#define GL_CON_0                                                     0x00008941
#endif
#ifndef GL_VERTEX_WEIGHT_ARRAY_POINTER
#define GL_VERTEX_WEIGHT_ARRAY_POINTER                               0x00008510
#endif
#ifndef GL_STENCIL_CLEAR_TAG_VALUE
#define GL_STENCIL_CLEAR_TAG_VALUE                                   0x000088F3
#endif
#ifndef GL_MATRIX24
#define GL_MATRIX24                                                  0x000088D8
#endif
#ifndef GL_STENCIL_BUFFER_BIT3
#define GL_STENCIL_BUFFER_BIT3                                       0x00080000
#endif
#ifndef GL_TEXTURE_UNSIGNED_REMAP_MODE
#define GL_TEXTURE_UNSIGNED_REMAP_MODE                               0x0000888F
#endif
#ifndef GL_SAMPLE_SHADING
#define GL_SAMPLE_SHADING                                            0x00008C36
#endif
#ifndef GL_MATRIX17
#define GL_MATRIX17                                                  0x000088D1
#endif
#ifndef GL_DSDT8
#define GL_DSDT8                                                     0x00008709
#endif
#ifndef GL_DEPTH_BOUNDS
#define GL_DEPTH_BOUNDS                                              0x00008891
#endif
#ifndef GL_PHONG_HINT
#define GL_PHONG_HINT                                                0x000080EB
#endif
#ifndef GL_STREAM_DRAW
#define GL_STREAM_DRAW                                               0x000088E0
#endif
#ifndef GL_TEXTURE_COLOR_WRITEMASK
#define GL_TEXTURE_COLOR_WRITEMASK                                   0x000081EF
#endif
#ifndef GL_SIGNED_NEGATE
#define GL_SIGNED_NEGATE                                             0x0000853D
#endif
#ifndef GL_RGBA_INTEGER
#define GL_RGBA_INTEGER                                              0x00008D99
#endif
#ifndef GL_GPU_OPTIMIZED
#define GL_GPU_OPTIMIZED                                             0x00008FB2
#endif
#ifndef GL_OP_ADD
#define GL_OP_ADD                                                    0x00008787
#endif
#ifndef GL_INVALID_FRAMEBUFFER_OPERATION
#define GL_INVALID_FRAMEBUFFER_OPERATION                             0x00000506
#endif
#ifndef GL_FLOAT_CLEAR_COLOR_VALUE
#define GL_FLOAT_CLEAR_COLOR_VALUE                                   0x0000888D
#endif
#ifndef GL_ALPHA16UI
#define GL_ALPHA16UI                                                 0x00008D78
#endif
#ifndef GL_ALPHA_FLOAT32
#define GL_ALPHA_FLOAT32                                             0x00008816
#endif
#ifndef GL_RELATIVE_LARGE_CCW_ARC_TO
#define GL_RELATIVE_LARGE_CCW_ARC_TO                                 0x00000017
#endif
#ifndef GL_TEXTURE_BUFFER
#define GL_TEXTURE_BUFFER                                            0x00008C2A
#endif
#ifndef GL_VIVIDLIGHT
#define GL_VIVIDLIGHT                                                0x000092A6
#endif
#ifndef GL_VERTEX_STREAM5
#define GL_VERTEX_STREAM5                                            0x00008771
#endif
#ifndef GL_PROGRAM_LENGTH
#define GL_PROGRAM_LENGTH                                            0x00008627
#endif
#ifndef GL_SHADER
#define GL_SHADER                                                    0x000082E1
#endif
#ifndef GL_IMAGE_BINDING_LAYER
#define GL_IMAGE_BINDING_LAYER                                       0x00008F3D
#endif
#ifndef GL_MAX_SHADER_BUFFER_ADDRESS
#define GL_MAX_SHADER_BUFFER_ADDRESS                                 0x00008F35
#endif
#ifndef GL_MAX_DEBUG_LOGGED_MESSAGES
#define GL_MAX_DEBUG_LOGGED_MESSAGES                                 0x00009144
#endif
#ifndef GL_RECT
#define GL_RECT                                                      0x000000F6
#endif
#ifndef GL_UNPACK_SKIP_PIXELS
#define GL_UNPACK_SKIP_PIXELS                                        0x00000CF4
#endif
#ifndef GL_MAX_COMPUTE_FIXED_GROUP_SIZE
#define GL_MAX_COMPUTE_FIXED_GROUP_SIZE                              0x000091BF
#endif
#ifndef GL_SOURCE0_ALPHA
#define GL_SOURCE0_ALPHA                                             0x00008588
#endif
#ifndef GL_HORIZONTAL_LINE_TO
#define GL_HORIZONTAL_LINE_TO                                        0x00000006
#endif
#ifndef GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2
#define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2                 0x00009277
#endif
#ifndef GL_DEPTH_BUFFER_BIT1
#define GL_DEPTH_BUFFER_BIT1                                         0x00000200
#endif
#ifndef GL_FOG_COORD_ARRAY_LENGTH
#define GL_FOG_COORD_ARRAY_LENGTH                                    0x00008F32
#endif
#ifndef GL_MAP1_VERTEX_ATTRIB10_4
#define GL_MAP1_VERTEX_ATTRIB10_4                                    0x0000866A
#endif
#ifndef GL_MODELVIEW18
#define GL_MODELVIEW18                                               0x00008732
#endif
#ifndef GL_OBJECT_BUFFER_USAGE
#define GL_OBJECT_BUFFER_USAGE                                       0x00008765
#endif
#ifndef GL_TEXTURE_WIDTH
#define GL_TEXTURE_WIDTH                                             0x00008BD2
#endif
#ifndef GL_INTENSITY32I
#define GL_INTENSITY32I                                              0x00008D85
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY_LENGTH
#define GL_VERTEX_ATTRIB_ARRAY_LENGTH                                0x00008F2A
#endif
#ifndef GL_TEXTURE_COORD_ARRAY_ADDRESS
#define GL_TEXTURE_COORD_ARRAY_ADDRESS                               0x00008F25
#endif
#ifndef GL_XOR
#define GL_XOR                                                       0x00001506
#endif
#ifndef GL_INSTRUMENT_BUFFER_POINTER
#define GL_INSTRUMENT_BUFFER_POINTER                                 0x00008180
#endif
#ifndef GL_SCALAR
#define GL_SCALAR                                                    0x000087BE
#endif
#ifndef GL_MAX_VERTEX_VARYING_COMPONENTS
#define GL_MAX_VERTEX_VARYING_COMPONENTS                             0x00008DDE
#endif
#ifndef GL_VIDEO_COLOR_CONVERSION_MAX
#define GL_VIDEO_COLOR_CONVERSION_MAX                                0x0000902A
#endif
#ifndef GL_SHADER_OBJECT
#define GL_SHADER_OBJECT                                             0x00008B48
#endif
#ifndef GL_RESAMPLE_DECIMATE
#define GL_RESAMPLE_DECIMATE                                         0x00008989
#endif
#ifndef GL_LOCAL_CONSTANT_VALUE
#define GL_LOCAL_CONSTANT_VALUE                                      0x000087EC
#endif
#ifndef GL_MAX_CONVOLUTION_WIDTH
#define GL_MAX_CONVOLUTION_WIDTH                                     0x0000801A
#endif
#ifndef GL_PN_TRIANGLES_NORMAL_MODE_LINEAR
#define GL_PN_TRIANGLES_NORMAL_MODE_LINEAR                           0x000087F7
#endif
#ifndef GL_EVAL_VERTEX_ATTRIB11
#define GL_EVAL_VERTEX_ATTRIB11                                      0x000086D1
#endif
#ifndef GL_CIRCULAR_TANGENT_ARC_TO
#define GL_CIRCULAR_TANGENT_ARC_TO                                   0x000000FC
#endif
#ifndef GL_E_TIMES_F
#define GL_E_TIMES_F                                                 0x00008531
#endif
#ifndef GL_VARIABLE_G
#define GL_VARIABLE_G                                                0x00008529
#endif
#ifndef GL_COLOR4_BIT
#define GL_COLOR4_BIT                                                0x00020000
#endif
#ifndef GL_COMPRESSED_R11_EAC
#define GL_COMPRESSED_R11_EAC                                        0x00009270
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4                        0x000093E5
#endif
#ifndef GL_RGBA8I
#define GL_RGBA8I                                                    0x00008D8E
#endif
#ifndef GL_MULTISAMPLE_BUFFER_BIT2
#define GL_MULTISAMPLE_BUFFER_BIT2                                   0x04000000
#endif
#ifndef GL_R1UI_T2F_V3F
#define GL_R1UI_T2F_V3F                                              0x000085C9
#endif
#ifndef GL_TEXTURE_2D_ARRAY
#define GL_TEXTURE_2D_ARRAY                                          0x00008C1A
#endif
#ifndef GL_TEXTURE16
#define GL_TEXTURE16                                                 0x000084D0
#endif
#ifndef GL_BUFFER_SIZE
#define GL_BUFFER_SIZE                                               0x00008764
#endif
#ifndef GL_UNPACK_ROW_BYTES
#define GL_UNPACK_ROW_BYTES                                          0x00008A16
#endif
#ifndef GL_HISTOGRAM_BLUE_SIZE
#define GL_HISTOGRAM_BLUE_SIZE                                       0x0000802A
#endif
#ifndef GL_INT_IMAGE_2D_RECT
#define GL_INT_IMAGE_2D_RECT                                         0x0000905A
#endif
#ifndef GL_TEXTURE_SAMPLES
#define GL_TEXTURE_SAMPLES                                           0x00009106
#endif
#ifndef GL_TEXTURE_PRE_SPECULAR
#define GL_TEXTURE_PRE_SPECULAR                                      0x00008169
#endif
#ifndef GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2
#define GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2                   0x00008C73
#endif
#ifndef GL_BUMP_ENVMAP
#define GL_BUMP_ENVMAP                                               0x0000877B
#endif
#ifndef GL_NEAREST_CLIPMAP_NEAREST
#define GL_NEAREST_CLIPMAP_NEAREST                                   0x0000844D
#endif
#ifndef GL_TEXTURE_LO_SIZE
#define GL_TEXTURE_LO_SIZE                                           0x0000871C
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_10x8
#define GL_COMPRESSED_RGBA_ASTC_10x8                                 0x000093BA
#endif
#ifndef GL_OBJECT_LINE
#define GL_OBJECT_LINE                                               0x000081F7
#endif
#ifndef GL_TRANSPOSE_COLOR_MATRIX
#define GL_TRANSPOSE_COLOR_MATRIX                                    0x000084E6
#endif
#ifndef GL_MAX_PROGRAM_OUTPUT_VERTICES
#define GL_MAX_PROGRAM_OUTPUT_VERTICES                               0x00008C27
#endif
#ifndef GL_INDEX_ARRAY_TYPE
#define GL_INDEX_ARRAY_TYPE                                          0x00008085
#endif
#ifndef GL_SAMPLE_MASK_VALUE
#define GL_SAMPLE_MASK_VALUE                                         0x000080AA
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5                          0x000093D3
#endif
#ifndef GL_FRAMEBUFFER_DEFAULT
#define GL_FRAMEBUFFER_DEFAULT                                       0x00008218
#endif
#ifndef GL_WEIGHT_ARRAY_SIZE
#define GL_WEIGHT_ARRAY_SIZE                                         0x000086AB
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8                          0x000093D7
#endif
#ifndef GL_POST_TEXTURE_FILTER_BIAS
#define GL_POST_TEXTURE_FILTER_BIAS                                  0x00008179
#endif
#ifndef GL_FONT_UNITS_PER_EM_BIT
#define GL_FONT_UNITS_PER_EM_BIT                                     0x00100000
#endif
#ifndef GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER
#define GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER                            0x00009160
#endif
#ifndef GL_SCALE_BY_TWO
#define GL_SCALE_BY_TWO                                              0x0000853E
#endif
#ifndef GL_CONVOLUTION_FORMAT
#define GL_CONVOLUTION_FORMAT                                        0x00008017
#endif
#ifndef GL_UTF16
#define GL_UTF16                                                     0x0000909B
#endif
#ifndef GL_NEGATIVE_W
#define GL_NEGATIVE_W                                                0x000087DC
#endif
#ifndef GL_COLOR_BUFFER_BIT6
#define GL_COLOR_BUFFER_BIT6                                         0x00000040
#endif
#ifndef GL_DUP_LAST_CUBIC_CURVE_TO
#define GL_DUP_LAST_CUBIC_CURVE_TO                                   0x000000F4
#endif
#ifndef GL_MAX_FRAGMENT_UNIFORM_BLOCKS
#define GL_MAX_FRAGMENT_UNIFORM_BLOCKS                               0x00008A2D
#endif
#ifndef GL_MAP2_VERTEX_ATTRIB3_4
#define GL_MAP2_VERTEX_ATTRIB3_4                                     0x00008673
#endif
#ifndef GL_3DC_XY
#define GL_3DC_XY                                                    0x000087FA
#endif
#ifndef GL_TEXTURE_1D_BINDING
#define GL_TEXTURE_1D_BINDING                                        0x00008068
#endif
#ifndef GL_SKIP_DECODE
#define GL_SKIP_DECODE                                               0x00008A4A
#endif
#ifndef GL_SOURCE3_ALPHA
#define GL_SOURCE3_ALPHA                                             0x0000858B
#endif
#ifndef GL_TEXTURE14
#define GL_TEXTURE14                                                 0x000084CE
#endif
#ifndef GL_YCRCBA
#define GL_YCRCBA                                                    0x00008319
#endif
#ifndef GL_DEBUG_CATEGORY_WINDOW_SYSTEM
#define GL_DEBUG_CATEGORY_WINDOW_SYSTEM                              0x0000914A
#endif
#ifndef GL_TEXTURE_COLOR_TABLE
#define GL_TEXTURE_COLOR_TABLE                                       0x000080BC
#endif
#ifndef GL_DEBUG_LOGGED_MESSAGES
#define GL_DEBUG_LOGGED_MESSAGES                                     0x00009145
#endif
#ifndef GL_REG_21
#define GL_REG_21                                                    0x00008936
#endif
#ifndef GL_RESTART
#define GL_RESTART                                                   0x00000001
#endif
#ifndef GL_FRAGMENT_PROGRAM
#define GL_FRAGMENT_PROGRAM                                          0x00008804
#endif
#ifndef GL_INT_SAMPLER_CUBE_MAP_ARRAY
#define GL_INT_SAMPLER_CUBE_MAP_ARRAY                                0x0000900E
#endif
#ifndef GL_MATRIX6
#define GL_MATRIX6                                                   0x000088C6
#endif
#ifndef GL_SQUARE
#define GL_SQUARE                                                    0x000090A3
#endif
#ifndef GL_COLOR_TABLE_LUMINANCE_SIZE
#define GL_COLOR_TABLE_LUMINANCE_SIZE                                0x000080DE
#endif
#ifndef GL_LOSE_CONTEXT_ON_RESET
#define GL_LOSE_CONTEXT_ON_RESET                                     0x00008252
#endif
#ifndef GL_IMAGE_2D_MULTISAMPLE
#define GL_IMAGE_2D_MULTISAMPLE                                      0x00009055
#endif
#ifndef GL_GPU_MEMORY_INFO_EVICTED_MEMORY
#define GL_GPU_MEMORY_INFO_EVICTED_MEMORY                            0x0000904B
#endif
#ifndef GL_RENDERBUFFER_RED_SIZE
#define GL_RENDERBUFFER_RED_SIZE                                     0x00008D50
#endif
#ifndef GL_RGBA16I
#define GL_RGBA16I                                                   0x00008D88
#endif
#ifndef GL_MAX_TRACK_MATRIX_STACK_DEPTH
#define GL_MAX_TRACK_MATRIX_STACK_DEPTH                              0x0000862E
#endif
#ifndef GL_MAX_ELEMENT_INDEX
#define GL_MAX_ELEMENT_INDEX                                         0x00008D6B
#endif
#ifndef GL_COLOR_INDEX4
#define GL_COLOR_INDEX4                                              0x000080E4
#endif
#ifndef GL_FONT_UNDERLINE_THICKNESS_BIT
#define GL_FONT_UNDERLINE_THICKNESS_BIT                              0x08000000
#endif
#ifndef GL_NUM_VIRTUAL_PAGE_SIZES
#define GL_NUM_VIRTUAL_PAGE_SIZES                                    0x000091A8
#endif
#ifndef GL_OP_CLAMP
#define GL_OP_CLAMP                                                  0x0000878E
#endif
#ifndef GL_TEXTURE_COMPARE
#define GL_TEXTURE_COMPARE                                           0x0000819A
#endif
#ifndef GL_PATH_COVER_DEPTH_FUNC
#define GL_PATH_COVER_DEPTH_FUNC                                     0x000090BF
#endif
#ifndef GL_RELEASED
#define GL_RELEASED                                                  0x00008A19
#endif
#ifndef GL_MODELVIEW28
#define GL_MODELVIEW28                                               0x0000873C
#endif
#ifndef GL_ALPHA_MAX
#define GL_ALPHA_MAX                                                 0x00008321
#endif
#ifndef GL_COLOR_MATRIX_STACK_DEPTH
#define GL_COLOR_MATRIX_STACK_DEPTH                                  0x000080B2
#endif
#ifndef GL_MAP1_VERTEX_ATTRIB1_4
#define GL_MAP1_VERTEX_ATTRIB1_4                                     0x00008661
#endif
#ifndef GL_UNDEFINED
#define GL_UNDEFINED                                                 0x00008A1C
#endif
#ifndef GL_TEXTURE30
#define GL_TEXTURE30                                                 0x000084DE
#endif
#ifndef GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH
#define GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH                      0x00008A35
#endif
#ifndef GL_PER_STAGE_CONSTANTS
#define GL_PER_STAGE_CONSTANTS                                       0x00008535
#endif
#ifndef GL_RELATIVE_SMOOTH_QUADRATIC_CURVE_TO
#define GL_RELATIVE_SMOOTH_QUADRATIC_CURVE_TO                        0x0000000F
#endif
#ifndef GL_REG_6
#define GL_REG_6                                                     0x00008927
#endif
#ifndef GL_MIN_PROGRAM_TEXEL_OFFSET
#define GL_MIN_PROGRAM_TEXEL_OFFSET                                  0x00008904
#endif
#ifndef GL_PIXEL_TEX_GEN
#define GL_PIXEL_TEX_GEN                                             0x00008139
#endif
#ifndef GL_INTENSITY
#define GL_INTENSITY                                                 0x00008049
#endif
#ifndef GL_DUAL_ALPHA8
#define GL_DUAL_ALPHA8                                               0x00008111
#endif
#ifndef GL_VERTEX_PROGRAM_PARAMETER_BUFFER
#define GL_VERTEX_PROGRAM_PARAMETER_BUFFER                           0x00008DA2
#endif
#ifndef GL_PACK_RESAMPLE
#define GL_PACK_RESAMPLE                                             0x00008984
#endif
#ifndef GL_POST_COLOR_MATRIX_ALPHA_BIAS
#define GL_POST_COLOR_MATRIX_ALPHA_BIAS                              0x000080BB
#endif
#ifndef GL_VIDEO_COLOR_CONVERSION_MATRIX
#define GL_VIDEO_COLOR_CONVERSION_MATRIX                             0x00009029
#endif
#ifndef GL_VERTICAL_LINE_TO
#define GL_VERTICAL_LINE_TO                                          0x00000008
#endif
#ifndef GL_TRANSLATE_2D
#define GL_TRANSLATE_2D                                              0x00009090
#endif
#ifndef GL_FOG_COORDINATE_ARRAY_LIST
#define GL_FOG_COORDINATE_ARRAY_LIST                                 0x000192A4
#endif
#ifndef GL_NUM_EXTENSIONS
#define GL_NUM_EXTENSIONS                                            0x0000821D
#endif
#ifndef GL_LUMINANCE_ALPHA_INTEGER
#define GL_LUMINANCE_ALPHA_INTEGER                                   0x00008D9D
#endif
#ifndef GL_ALLOW_DRAW_MEM_HINT
#define GL_ALLOW_DRAW_MEM_HINT                                       0x0001A211
#endif
#ifndef GL_COLOR_TABLE_GREEN_SIZE
#define GL_COLOR_TABLE_GREEN_SIZE                                    0x000080DB
#endif
#ifndef GL_MAX_MULTISAMPLE_COVERAGE_MODES
#define GL_MAX_MULTISAMPLE_COVERAGE_MODES                            0x00008E11
#endif
#ifndef GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT
#define GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT                            0x00000004
#endif
#ifndef GL_TEXTURE_IMMUTABLE_FORMAT
#define GL_TEXTURE_IMMUTABLE_FORMAT                                  0x0000912F
#endif
#ifndef GL_IMPLEMENTATION_COLOR_READ_TYPE
#define GL_IMPLEMENTATION_COLOR_READ_TYPE                            0x00008B9A
#endif
#ifndef GL_PERFQUERY_FLUSH
#define GL_PERFQUERY_FLUSH                                           0x000083FA
#endif
#ifndef GL_DRAW_BUFFER15
#define GL_DRAW_BUFFER15                                             0x00008834
#endif
#ifndef GL_BGRA_INTEGER
#define GL_BGRA_INTEGER                                              0x00008D9B
#endif
#ifndef GL_HARDLIGHT
#define GL_HARDLIGHT                                                 0x0000929B
#endif
#ifndef GL_OP_FRAC
#define GL_OP_FRAC                                                   0x00008789
#endif
#ifndef GL_PROGRAM_ATTRIB_COMPONENTS
#define GL_PROGRAM_ATTRIB_COMPONENTS                                 0x00008906
#endif
#ifndef GL_CON_30
#define GL_CON_30                                                    0x0000895F
#endif
#ifndef GL_MAX_SHININESS
#define GL_MAX_SHININESS                                             0x00008504
#endif
#ifndef GL_PROGRAM_MATRIX
#define GL_PROGRAM_MATRIX                                            0x00008E2D
#endif
#ifndef GL_COUNTER_RANGE
#define GL_COUNTER_RANGE                                             0x00008BC1
#endif
#ifndef GL_VERTEX_ARRAY_LIST_STRIDE
#define GL_VERTEX_ARRAY_LIST_STRIDE                                  0x000192A8
#endif
#ifndef GL_TEXTURE_CONSTANT_DATA
#define GL_TEXTURE_CONSTANT_DATA                                     0x000081D6
#endif
#ifndef GL_MOVE_TO_RESETS
#define GL_MOVE_TO_RESETS                                            0x000090B5
#endif
#ifndef GL_IMAGE_2D_RECT
#define GL_IMAGE_2D_RECT                                             0x0000904F
#endif
#ifndef GL_DS_SCALE
#define GL_DS_SCALE                                                  0x00008710
#endif
#ifndef GL_DEBUG_GROUP_STACK_DEPTH
#define GL_DEBUG_GROUP_STACK_DEPTH                                   0x0000826D
#endif
#ifndef GL_LINEAR_CLIPMAP_NEAREST
#define GL_LINEAR_CLIPMAP_NEAREST                                    0x0000844F
#endif
#ifndef GL_UNSIGNED_INT_10_10_10_2
#define GL_UNSIGNED_INT_10_10_10_2                                   0x00008036
#endif
#ifndef GL_R1UI_T2F_C4F_N3F_V3F
#define GL_R1UI_T2F_C4F_N3F_V3F                                      0x000085CB
#endif
#ifndef GL_GLYPH_VERTICAL_BEARING_X_BIT
#define GL_GLYPH_VERTICAL_BEARING_X_BIT                              0x00000020
#endif
#ifndef GL_TEXTURE_SWIZZLE_A
#define GL_TEXTURE_SWIZZLE_A                                         0x00008E45
#endif
#ifndef GL_TEXTURE_COORD_ARRAY_COUNT
#define GL_TEXTURE_COORD_ARRAY_COUNT                                 0x0000808B
#endif
#ifndef GL_RED_MAX_CLAMP
#define GL_RED_MAX_CLAMP                                             0x00008564
#endif
#ifndef GL_PIXEL_TILE_HEIGHT
#define GL_PIXEL_TILE_HEIGHT                                         0x00008141
#endif
#ifndef GL_REPLACEMENT_CODE_ARRAY_TYPE
#define GL_REPLACEMENT_CODE_ARRAY_TYPE                               0x000085C1
#endif
#ifndef GL_PATH_STENCIL_DEPTH_OFFSET_FACTOR
#define GL_PATH_STENCIL_DEPTH_OFFSET_FACTOR                          0x000090BD
#endif
#ifndef GL_PERFQUERY_COUNTER_DATA_FLOAT
#define GL_PERFQUERY_COUNTER_DATA_FLOAT                              0x000094FA
#endif
#ifndef GL_FLOAT_32_UNSIGNED_INT_24_8_REV
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV                            0x00008DAD
#endif
#ifndef GL_TEXTURE_CUBE_MAP_ARRAY
#define GL_TEXTURE_CUBE_MAP_ARRAY                                    0x00009009
#endif
#ifndef GL_SPRITE_AXIAL
#define GL_SPRITE_AXIAL                                              0x0000814C
#endif
#ifndef GL_MAX_VERTEX_SHADER_VARIANTS
#define GL_MAX_VERTEX_SHADER_VARIANTS                                0x000087C6
#endif
#ifndef GL_COLOR3_BIT
#define GL_COLOR3_BIT                                                0x00010000
#endif
#ifndef GL_CLAMP_FRAGMENT_COLOR
#define GL_CLAMP_FRAGMENT_COLOR                                      0x0000891B
#endif
#ifndef GL_RENDERBUFFER_COVERAGE_SAMPLES
#define GL_RENDERBUFFER_COVERAGE_SAMPLES                             0x00008CAB
#endif
#ifndef GL_PERFQUERY_COUNTER_DATA_BOOL32
#define GL_PERFQUERY_COUNTER_DATA_BOOL32                             0x000094FC
#endif
#ifndef GL_DEPENDENT_RGB_TEXTURE_CUBE_MAP
#define GL_DEPENDENT_RGB_TEXTURE_CUBE_MAP                            0x0000885A
#endif
#ifndef GL_OFFSET_PROJECTIVE_TEXTURE_2D
#define GL_OFFSET_PROJECTIVE_TEXTURE_2D                              0x00008850
#endif
#ifndef GL_R16UI
#define GL_R16UI                                                     0x00008234
#endif
#ifndef GL_LUMINANCE16F
#define GL_LUMINANCE16F                                              0x0000881E
#endif
#ifndef GL_DRAW_BUFFER1
#define GL_DRAW_BUFFER1                                              0x00008826
#endif
#ifndef GL_TRIANGLE_LIST
#define GL_TRIANGLE_LIST                                             0x000081D7
#endif
#ifndef GL_FLOAT_RGBA32
#define GL_FLOAT_RGBA32                                              0x0000888B
#endif
#ifndef GL_DEPTH_COMPONENT24
#define GL_DEPTH_COMPONENT24                                         0x000081A6
#endif
#ifndef GL_FORCE_BLUE_TO_ONE
#define GL_FORCE_BLUE_TO_ONE                                         0x00008860
#endif
#ifndef GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE
#define GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE                         0x00008213
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY_UNIFIED
#define GL_VERTEX_ATTRIB_ARRAY_UNIFIED                               0x00008F1E
#endif
#ifndef GL_MULTISAMPLE_FILTER_HINT
#define GL_MULTISAMPLE_FILTER_HINT                                   0x00008534
#endif
#ifndef GL_READ_PIXEL_DATA_RANGE_POINTER
#define GL_READ_PIXEL_DATA_RANGE_POINTER                             0x0000887D
#endif
#ifndef GL_MAP2_VERTEX_ATTRIB0_4
#define GL_MAP2_VERTEX_ATTRIB0_4                                     0x00008670
#endif
#ifndef GL_NORMAL_BIT
#define GL_NORMAL_BIT                                                0x08000000
#endif
#ifndef GL_MAX_TEXTURE_BUFFER_SIZE
#define GL_MAX_TEXTURE_BUFFER_SIZE                                   0x00008C2B
#endif
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE                                             0x0000812F
#endif
#ifndef GL_IMAGE_CUBE_MAP_ARRAY
#define GL_IMAGE_CUBE_MAP_ARRAY                                      0x00009054
#endif
#ifndef GL_FLOAT_RG
#define GL_FLOAT_RG                                                  0x00008881
#endif
#ifndef GL_MODELVIEW3
#define GL_MODELVIEW3                                                0x00008723
#endif
#ifndef GL_TRANSFORM_FEEDBACK_BUFFER_START
#define GL_TRANSFORM_FEEDBACK_BUFFER_START                           0x00008C84
#endif
#ifndef GL_TEXTURE_SHARED_SIZE
#define GL_TEXTURE_SHARED_SIZE                                       0x00008C3F
#endif
#ifndef GL_NAMED_STRING_LENGTH
#define GL_NAMED_STRING_LENGTH                                       0x00008DE9
#endif
#ifndef GL_MATRIX31
#define GL_MATRIX31                                                  0x000088DF
#endif
#ifndef GL_TRANSFORM_FEEDBACK_ACTIVE
#define GL_TRANSFORM_FEEDBACK_ACTIVE                                 0x00008E24
#endif
#ifndef GL_MAX_COMPUTE_VARIABLE_GROUP_INVOCATIONS
#define GL_MAX_COMPUTE_VARIABLE_GROUP_INVOCATIONS                    0x00009344
#endif
#ifndef GL_SHADER_BINARY
#define GL_SHADER_BINARY                                             0x00009250
#endif
#ifndef GL_TEXCOORD4_BIT
#define GL_TEXCOORD4_BIT                                             0x80000000
#endif
#ifndef GL_MAX_4D_TEXTURE_SIZE
#define GL_MAX_4D_TEXTURE_SIZE                                       0x00008138
#endif
#ifndef GL_VERTEX_ATTRIB_MAP2_ORDER
#define GL_VERTEX_ATTRIB_MAP2_ORDER                                  0x00008A08
#endif
#ifndef GL_MIRRORED_REPEAT
#define GL_MIRRORED_REPEAT                                           0x00008370
#endif
#ifndef GL_INTENSITY16F
#define GL_INTENSITY16F                                              0x0000881D
#endif
#ifndef GL_TEXTURE_BINDING_BUFFER
#define GL_TEXTURE_BINDING_BUFFER                                    0x00008C2C
#endif
#ifndef GL_SPRITE_EYE_ALIGNED
#define GL_SPRITE_EYE_ALIGNED                                        0x0000814E
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD21
#define GL_OUTPUT_TEXTURE_COORD21                                    0x000087B2
#endif
#ifndef GL_STENCIL_BACK_FAIL
#define GL_STENCIL_BACK_FAIL                                         0x00008801
#endif
#ifndef GL_UNSIGNED_INT_SAMPLER_2D_RECT
#define GL_UNSIGNED_INT_SAMPLER_2D_RECT                              0x00008DD5
#endif
#ifndef GL_STENCIL_BACK_FUNC
#define GL_STENCIL_BACK_FUNC                                         0x00008800
#endif
#ifndef GL_YCBCR
#define GL_YCBCR                                                     0x00008757
#endif
#ifndef GL_NO_RESET_NOTIFICATION
#define GL_NO_RESET_NOTIFICATION                                     0x00008261
#endif
#ifndef GL_INT_SAMPLER_BUFFER
#define GL_INT_SAMPLER_BUFFER                                        0x00009002
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD13
#define GL_OUTPUT_TEXTURE_COORD13                                    0x000087AA
#endif
#ifndef GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED
#define GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED                          0x00008E23
#endif
#ifndef GL_IUI_N3F_V2F
#define GL_IUI_N3F_V2F                                               0x000081AF
#endif
#ifndef GL_WRITEONLY_RENDERING
#define GL_WRITEONLY_RENDERING                                       0x00008823
#endif
#ifndef GL_MAX_CLIPMAP_DEPTH
#define GL_MAX_CLIPMAP_DEPTH                                         0x00008177
#endif
#ifndef GL_VERTEX_STREAM1
#define GL_VERTEX_STREAM1                                            0x0000876D
#endif
#ifndef GL_HI_SCALE
#define GL_HI_SCALE                                                  0x0000870E
#endif
#ifndef GL_MATRIX3
#define GL_MATRIX3                                                   0x000088C3
#endif
#ifndef GL_RENDERBUFFER_GREEN_SIZE
#define GL_RENDERBUFFER_GREEN_SIZE                                   0x00008D51
#endif
#ifndef GL_HISTOGRAM_LUMINANCE_SIZE
#define GL_HISTOGRAM_LUMINANCE_SIZE                                  0x0000802C
#endif
#ifndef GL_RGB9_E5
#define GL_RGB9_E5                                                   0x00008C3D
#endif
#ifndef GL_UNPACK_CONSTANT_DATA
#define GL_UNPACK_CONSTANT_DATA                                      0x000081D5
#endif
#ifndef GL_SIGNED_RGB_UNSIGNED_ALPHA
#define GL_SIGNED_RGB_UNSIGNED_ALPHA                                 0x0000870C
#endif
#ifndef GL_RGBA2
#define GL_RGBA2                                                     0x00008055
#endif
#ifndef GL_TEXTURE3
#define GL_TEXTURE3                                                  0x000084C3
#endif
#ifndef GL_SECONDARY_COLOR_ARRAY_LIST
#define GL_SECONDARY_COLOR_ARRAY_LIST                                0x000192A5
#endif
#ifndef GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY
#define GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY                       0x0000900F
#endif
#ifndef GL_RGB_RAW_422
#define GL_RGB_RAW_422                                               0x00008A51
#endif
#ifndef GL_MULTISAMPLE_BUFFER_BIT5
#define GL_MULTISAMPLE_BUFFER_BIT5                                   0x20000000
#endif
#ifndef GL_SIGNED_HILO
#define GL_SIGNED_HILO                                               0x000086F9
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD25
#define GL_OUTPUT_TEXTURE_COORD25                                    0x000087B6
#endif
#ifndef GL_OP_SET_LT
#define GL_OP_SET_LT                                                 0x0000878D
#endif
#ifndef GL_MAP2_VERTEX_ATTRIB15_4
#define GL_MAP2_VERTEX_ATTRIB15_4                                    0x0000867F
#endif
#ifndef GL_CLAMP_VERTEX_COLOR
#define GL_CLAMP_VERTEX_COLOR                                        0x0000891A
#endif
#ifndef GL_IMAGE_1D_ARRAY
#define GL_IMAGE_1D_ARRAY                                            0x00009052
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_8x8
#define GL_COMPRESSED_RGBA_ASTC_8x8                                  0x000093B7
#endif
#ifndef GL_MAX_PROGRAM_LOOP_DEPTH
#define GL_MAX_PROGRAM_LOOP_DEPTH                                    0x000088F7
#endif
#ifndef GL_DEPTH32F_STENCIL8
#define GL_DEPTH32F_STENCIL8                                         0x00008CAD
#endif
#ifndef GL_PATH_ERROR_POSITION
#define GL_PATH_ERROR_POSITION                                       0x000090AB
#endif
#ifndef GL_OP_MADD
#define GL_OP_MADD                                                   0x00008788
#endif
#ifndef GL_STENCIL_TEST_TWO_SIDE
#define GL_STENCIL_TEST_TWO_SIDE                                     0x00008910
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3
#define GL_COMPRESSED_RGBA_S3TC_DXT3                                 0x000083F2
#endif
#ifndef GL_OP_FLOOR
#define GL_OP_FLOOR                                                  0x0000878F
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY_TYPE
#define GL_VERTEX_ATTRIB_ARRAY_TYPE                                  0x00008625
#endif
#ifndef GL_COPY_READ_BUFFER_BINDING
#define GL_COPY_READ_BUFFER_BINDING                                  0x00008F36
#endif
#ifndef GL_RGBA_FLOAT16
#define GL_RGBA_FLOAT16                                              0x0000881A
#endif
#ifndef GL_COMPRESSED_INTENSITY
#define GL_COMPRESSED_INTENSITY                                      0x000084EC
#endif
#ifndef GL_MAX_PROGRAM_ATTRIBS
#define GL_MAX_PROGRAM_ATTRIBS                                       0x000088AD
#endif
#ifndef GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT
#define GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT                           0x00008A34
#endif
#ifndef GL_UNIFORM_SIZE
#define GL_UNIFORM_SIZE                                              0x00008A38
#endif
#ifndef GL_IMAGE_TRANSLATE_Y
#define GL_IMAGE_TRANSLATE_Y                                         0x00008158
#endif
#ifndef GL_PALETTE4_R5_G6_B5
#define GL_PALETTE4_R5_G6_B5                                         0x00008B92
#endif
#ifndef GL_SIGNED_LUMINANCE8
#define GL_SIGNED_LUMINANCE8                                         0x00008702
#endif
#ifndef GL_ALPHA_FLOAT16
#define GL_ALPHA_FLOAT16                                             0x0000881C
#endif
#ifndef GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT                           0x00000020
#endif
#ifndef GL_TEXT_FRAGMENT_SHADER
#define GL_TEXT_FRAGMENT_SHADER                                      0x00008200
#endif
#ifndef GL_SAMPLE_ALPHA_TO_ONE
#define GL_SAMPLE_ALPHA_TO_ONE                                       0x0000809F
#endif
#ifndef GL_SLUMINANCE_ALPHA
#define GL_SLUMINANCE_ALPHA                                          0x00008C44
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD29
#define GL_OUTPUT_TEXTURE_COORD29                                    0x000087BA
#endif
#ifndef GL_MAX_PROGRAM_GENERIC_RESULTS
#define GL_MAX_PROGRAM_GENERIC_RESULTS                               0x00008DA6
#endif
#ifndef GL_INTERPOLATE
#define GL_INTERPOLATE                                               0x00008575
#endif
#ifndef GL_PERFQUERY_GPA_EXTENDED_COUNTERS
#define GL_PERFQUERY_GPA_EXTENDED_COUNTERS                           0x00009500
#endif
#ifndef GL_REPLACEMENT_CODE
#define GL_REPLACEMENT_CODE                                          0x000081D8
#endif
#ifndef GL_NEAREST_CLIPMAP_LINEAR
#define GL_NEAREST_CLIPMAP_LINEAR                                    0x0000844E
#endif
#ifndef GL_RENDERBUFFER_DEPTH_SIZE
#define GL_RENDERBUFFER_DEPTH_SIZE                                   0x00008D54
#endif
#ifndef GL_COMBINER7
#define GL_COMBINER7                                                 0x00008557
#endif
#ifndef GL_SIGNED_IDENTITY
#define GL_SIGNED_IDENTITY                                           0x0000853C
#endif
#ifndef GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2
#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2                  0x00009276
#endif
#ifndef GL_LINEAR_SHARPEN_COLOR
#define GL_LINEAR_SHARPEN_COLOR                                      0x000080AF
#endif
#ifndef GL_RGBA32F
#define GL_RGBA32F                                                   0x00008814
#endif
#ifndef GL_BLUE_MAX_CLAMP
#define GL_BLUE_MAX_CLAMP                                            0x00008566
#endif
#ifndef GL_SWIZZLE_STQ_DQ
#define GL_SWIZZLE_STQ_DQ                                            0x00008979
#endif
#ifndef GL_REG_27
#define GL_REG_27                                                    0x0000893C
#endif
#ifndef GL_FRAGMENT_PROGRAM_PARAMETER_BUFFER
#define GL_FRAGMENT_PROGRAM_PARAMETER_BUFFER                         0x00008DA4
#endif
#ifndef GL_PATH_STROKE_WIDTH
#define GL_PATH_STROKE_WIDTH                                         0x00009075
#endif
#ifndef GL_LUMINANCE16
#define GL_LUMINANCE16                                               0x00008042
#endif
#ifndef GL_STREAM_COPY
#define GL_STREAM_COPY                                               0x000088E2
#endif
#ifndef GL_DOUBLE_MAT2x3
#define GL_DOUBLE_MAT2x3                                             0x00008F49
#endif
#ifndef GL_MAX_3D_TEXTURE_SIZE
#define GL_MAX_3D_TEXTURE_SIZE                                       0x00008073
#endif
#ifndef GL_BINORMAL_ARRAY_TYPE
#define GL_BINORMAL_ARRAY_TYPE                                       0x00008440
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10                        0x000093DC
#endif
#ifndef GL_TEXTURE_NORMAL
#define GL_TEXTURE_NORMAL                                            0x000085AF
#endif
#ifndef GL_MAX_ASYNC_DRAW_PIXELS
#define GL_MAX_ASYNC_DRAW_PIXELS                                     0x00008360
#endif
#ifndef GL_MAGNITUDE_SCALE
#define GL_MAGNITUDE_SCALE                                           0x00008712
#endif
#ifndef GL_MAX_UNIFORM_BUFFER_BINDINGS
#define GL_MAX_UNIFORM_BUFFER_BINDINGS                               0x00008A2F
#endif
#ifndef GL_CON_15
#define GL_CON_15                                                    0x00008950
#endif
#ifndef GL_MAT_AMBIENT_AND_DIFFUSE_BIT
#define GL_MAT_AMBIENT_AND_DIFFUSE_BIT                               0x00200000
#endif
#ifndef GL_CONVOLUTION_HEIGHT
#define GL_CONVOLUTION_HEIGHT                                        0x00008019
#endif
#ifndef GL_DEBUG_CALLBACK_FUNCTION
#define GL_DEBUG_CALLBACK_FUNCTION                                   0x00008244
#endif
#ifndef GL_UNSIGNED_INT_10F_11F_11F_REV
#define GL_UNSIGNED_INT_10F_11F_11F_REV                              0x00008C3B
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD19
#define GL_OUTPUT_TEXTURE_COORD19                                    0x000087B0
#endif
#ifndef GL_QUAD_TEXTURE_SELECT
#define GL_QUAD_TEXTURE_SELECT                                       0x00008125
#endif
#ifndef GL_SOURCE1_RGB
#define GL_SOURCE1_RGB                                               0x00008581
#endif
#ifndef GL_ASYNC_TEX_IMAGE
#define GL_ASYNC_TEX_IMAGE                                           0x0000835C
#endif
#ifndef GL_PATH_FORMAT_SVG
#define GL_PATH_FORMAT_SVG                                           0x00009070
#endif
#ifndef GL_TEXTURE19
#define GL_TEXTURE19                                                 0x000084D3
#endif
#ifndef GL_PROGRAM_BINARY_RETRIEVABLE_HINT
#define GL_PROGRAM_BINARY_RETRIEVABLE_HINT                           0x00008257
#endif
#ifndef GL_BLEND_SRC_RGB
#define GL_BLEND_SRC_RGB                                             0x000080C9
#endif
#ifndef GL_VERTEX_PROGRAM_POINT_SIZE
#define GL_VERTEX_PROGRAM_POINT_SIZE                                 0x00008642
#endif
#ifndef GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET
#define GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET                         0x00008E5F
#endif
#ifndef GL_LUMINANCE8I
#define GL_LUMINANCE8I                                               0x00008D92
#endif
#ifndef GL_TEXTURE26
#define GL_TEXTURE26                                                 0x000084DA
#endif
#ifndef GL_MAP_TESSELLATION
#define GL_MAP_TESSELLATION                                          0x000086C2
#endif
#ifndef GL_TEXTURE24
#define GL_TEXTURE24                                                 0x000084D8
#endif
#ifndef GL_EVAL_VERTEX_ATTRIB8
#define GL_EVAL_VERTEX_ATTRIB8                                       0x000086CE
#endif
#ifndef GL_INCR_WRAP
#define GL_INCR_WRAP                                                 0x00008507
#endif
#ifndef GL_REPLACE_MIDDLE
#define GL_REPLACE_MIDDLE                                            0x00000002
#endif
#ifndef GL_SIGNED_LUMINANCE_ALPHA
#define GL_SIGNED_LUMINANCE_ALPHA                                    0x00008703
#endif
#ifndef GL_RESTART_PATH
#define GL_RESTART_PATH                                              0x000000F0
#endif
#ifndef GL_DOT3_RGB
#define GL_DOT3_RGB                                                  0x000086AE
#endif
#ifndef GL_OFFSET_PROJECTIVE_TEXTURE_2D_SCALE
#define GL_OFFSET_PROJECTIVE_TEXTURE_2D_SCALE                        0x00008851
#endif
#ifndef GL_MODELVIEW24
#define GL_MODELVIEW24                                               0x00008738
#endif
#ifndef GL_SIGNED_ALPHA
#define GL_SIGNED_ALPHA                                              0x00008705
#endif
#ifndef GL_REG_15
#define GL_REG_15                                                    0x00008930
#endif
#ifndef GL_RENDERBUFFER_BINDING
#define GL_RENDERBUFFER_BINDING                                      0x00008CA7
#endif
#ifndef GL_WRITE_PIXEL_DATA_RANGE_POINTER
#define GL_WRITE_PIXEL_DATA_RANGE_POINTER                            0x0000887C
#endif
#ifndef GL_COMBINE
#define GL_COMBINE                                                   0x00008570
#endif
#ifndef GL_MATRIX8
#define GL_MATRIX8                                                   0x000088C8
#endif
#ifndef GL_IMAGE_3D
#define GL_IMAGE_3D                                                  0x0000904E
#endif
#ifndef GL_ASYNC_HISTOGRAM
#define GL_ASYNC_HISTOGRAM                                           0x0000832C
#endif
#ifndef GL_UNIFORM_BUFFER_BINDING
#define GL_UNIFORM_BUFFER_BINDING                                    0x00008A28
#endif
#ifndef GL_DEPTH_BUFFER_BIT2
#define GL_DEPTH_BUFFER_BIT2                                         0x00000400
#endif
#ifndef GL_OCCLUSION_QUERY_EVENT_MASK
#define GL_OCCLUSION_QUERY_EVENT_MASK                                0x0000874F
#endif
#ifndef GL_IMAGE_BINDING_FORMAT
#define GL_IMAGE_BINDING_FORMAT                                      0x0000906E
#endif
#ifndef GL_CON_10
#define GL_CON_10                                                    0x0000894B
#endif
#ifndef GL_VIDEO_COLOR_CONVERSION_OFFSET
#define GL_VIDEO_COLOR_CONVERSION_OFFSET                             0x0000902C
#endif
#ifndef GL_MAX
#define GL_MAX                                                       0x00008008
#endif
#ifndef GL_STATIC
#define GL_STATIC                                                    0x00008760
#endif
#ifndef GL_ATTENUATION
#define GL_ATTENUATION                                               0x0000834D
#endif
#ifndef GL_REG_10
#define GL_REG_10                                                    0x0000892B
#endif
#ifndef GL_PALETTE4_RGBA4
#define GL_PALETTE4_RGBA4                                            0x00008B93
#endif
#ifndef GL_ELEMENT_ARRAY_BARRIER_BIT
#define GL_ELEMENT_ARRAY_BARRIER_BIT                                 0x00000002
#endif
#ifndef GL_SIGNED_RGBA8
#define GL_SIGNED_RGBA8                                              0x000086FC
#endif
#ifndef GL_PATH_STENCIL_VALUE_MASK
#define GL_PATH_STENCIL_VALUE_MASK                                   0x000090B9
#endif
#ifndef GL_INDEX_ARRAY_COUNT
#define GL_INDEX_ARRAY_COUNT                                         0x00008087
#endif
#ifndef GL_PATH_GEN_COEFF
#define GL_PATH_GEN_COEFF                                            0x000090B1
#endif
#ifndef GL_CONJOINT
#define GL_CONJOINT                                                  0x00009284
#endif
#ifndef GL_CONSTANT_COLOR1
#define GL_CONSTANT_COLOR1                                           0x0000852B
#endif
#ifndef GL_SAMPLES_PASSED
#define GL_SAMPLES_PASSED                                            0x00008914
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY11
#define GL_VERTEX_ATTRIB_ARRAY11                                     0x0000865B
#endif
#ifndef GL_CONVOLUTION_2D
#define GL_CONVOLUTION_2D                                            0x00008011
#endif
#ifndef GL_LO_SCALE
#define GL_LO_SCALE                                                  0x0000870F
#endif
#ifndef GL_FIELD_LOWER
#define GL_FIELD_LOWER                                               0x00009023
#endif
#ifndef GL_MATRIX28
#define GL_MATRIX28                                                  0x000088DC
#endif
#ifndef GL_TEXTURE_FLOAT_COMPONENTS
#define GL_TEXTURE_FLOAT_COMPONENTS                                  0x0000888C
#endif
#ifndef GL_DRAW_BUFFER10
#define GL_DRAW_BUFFER10                                             0x0000882F
#endif
#ifndef GL_FRAGMENT_LIGHT2
#define GL_FRAGMENT_LIGHT2                                           0x0000840E
#endif
#ifndef GL_DEBUG_TYPE_PUSH_GROUP
#define GL_DEBUG_TYPE_PUSH_GROUP                                     0x00008269
#endif
#ifndef GL_BACK_NORMALS_HINT
#define GL_BACK_NORMALS_HINT                                         0x0001A223
#endif
#ifndef GL_NEXT_BUFFER
#define GL_NEXT_BUFFER                                               0xFFFFFFFFFFFFFFFE
#endif
#ifndef GL_DOT_PRODUCT_TEXTURE_2D
#define GL_DOT_PRODUCT_TEXTURE_2D                                    0x000086EE
#endif
#ifndef GL_GREEN_BIT
#define GL_GREEN_BIT                                                 0x00000002
#endif
#ifndef GL_COLOR_FLOAT
#define GL_COLOR_FLOAT                                               0x00008A0F
#endif
#ifndef GL_PALETTE4_RGBA8
#define GL_PALETTE4_RGBA8                                            0x00008B91
#endif
#ifndef GL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS
#define GL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS                     0x00008868
#endif
#ifndef GL_SPRITE_MODE
#define GL_SPRITE_MODE                                               0x00008149
#endif
#ifndef GL_BEVEL
#define GL_BEVEL                                                     0x000090A6
#endif
#ifndef GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT                          0x00008E8E
#endif
#ifndef GL_VERTEX_SHADER_LOCALS
#define GL_VERTEX_SHADER_LOCALS                                      0x000087D3
#endif
#ifndef GL_CON_27
#define GL_CON_27                                                    0x0000895C
#endif
#ifndef GL_INDEX_ARRAY_LENGTH
#define GL_INDEX_ARRAY_LENGTH                                        0x00008F2E
#endif
#ifndef GL_FRAMEBUFFER_BINDING
#define GL_FRAMEBUFFER_BINDING                                       0x00008CA6
#endif
#ifndef GL_VERTEX_ATTRIB_MAP1_ORDER
#define GL_VERTEX_ATTRIB_MAP1_ORDER                                  0x00008A04
#endif
#ifndef GL_Z4Y12Z4CB12Z4Y12Z4CR12_422
#define GL_Z4Y12Z4CB12Z4Y12Z4CR12_422                                0x00009035
#endif
#ifndef GL_TEXTURE_FETCH_BARRIER_BIT
#define GL_TEXTURE_FETCH_BARRIER_BIT                                 0x00000008
#endif
#ifndef GL_MINMAX_FORMAT
#define GL_MINMAX_FORMAT                                             0x0000802F
#endif
#ifndef GL_STORAGE_CACHED
#define GL_STORAGE_CACHED                                            0x000085BE
#endif
#ifndef GL_SWIZZLE_STQ
#define GL_SWIZZLE_STQ                                               0x00008977
#endif
#ifndef GL_DECR_WRAP
#define GL_DECR_WRAP                                                 0x00008508
#endif
#ifndef GL_SHARED_TEXTURE_PALETTE
#define GL_SHARED_TEXTURE_PALETTE                                    0x000081FB
#endif
#ifndef GL_TEXTURE_BINDING_2D_ARRAY
#define GL_TEXTURE_BINDING_2D_ARRAY                                  0x00008C1D
#endif
#ifndef GL_WEIGHT_ARRAY_BUFFER_BINDING
#define GL_WEIGHT_ARRAY_BUFFER_BINDING                               0x0000889E
#endif
#ifndef GL_MATRIX_INDEX_ARRAY_POINTER
#define GL_MATRIX_INDEX_ARRAY_POINTER                                0x00008849
#endif
#ifndef GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH
#define GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH                     0x00008C76
#endif
#ifndef GL_REG_19
#define GL_REG_19                                                    0x00008934
#endif
#ifndef GL_MODELVIEW11
#define GL_MODELVIEW11                                               0x0000872B
#endif
#ifndef GL_TEXTURE_COLOR_SAMPLES
#define GL_TEXTURE_COLOR_SAMPLES                                     0x00009046
#endif
#ifndef GL_POST_COLOR_MATRIX_ALPHA_SCALE
#define GL_POST_COLOR_MATRIX_ALPHA_SCALE                             0x000080B7
#endif
#ifndef GL_LUMINANCE16_ALPHA16
#define GL_LUMINANCE16_ALPHA16                                       0x00008048
#endif
#ifndef GL_ARRAY_ELEMENT_LOCK_COUNT
#define GL_ARRAY_ELEMENT_LOCK_COUNT                                  0x000081A9
#endif
#ifndef GL_UNIFORM_BLOCK_NAME_LENGTH
#define GL_UNIFORM_BLOCK_NAME_LENGTH                                 0x00008A41
#endif
#ifndef GL_ALPHA_INTEGER
#define GL_ALPHA_INTEGER                                             0x00008D97
#endif
#ifndef GL_MODELVIEW9
#define GL_MODELVIEW9                                                0x00008729
#endif
#ifndef GL_YCBAYCR8A_4224
#define GL_YCBAYCR8A_4224                                            0x00009032
#endif
#ifndef GL_DISCARD
#define GL_DISCARD                                                   0x00008763
#endif
#ifndef GL_VIDEO_CAPTURE_SURFACE_ORIGIN
#define GL_VIDEO_CAPTURE_SURFACE_ORIGIN                              0x0000903C
#endif
#ifndef GL_VERTEX_ARRAY_LIST
#define GL_VERTEX_ARRAY_LIST                                         0x0001929E
#endif
#ifndef GL_MAX_CONVOLUTION_HEIGHT
#define GL_MAX_CONVOLUTION_HEIGHT                                    0x0000801B
#endif
#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER                                              0x00008892
#endif
#ifndef GL_TEXTURE_COORD_ARRAY_STRIDE
#define GL_TEXTURE_COORD_ARRAY_STRIDE                                0x0000808A
#endif
#ifndef GL_COMBINER_SUM_OUTPUT
#define GL_COMBINER_SUM_OUTPUT                                       0x0000854C
#endif
#ifndef GL_MAX_PIXEL_TRANSFORM_2D_STACK_DEPTH
#define GL_MAX_PIXEL_TRANSFORM_2D_STACK_DEPTH                        0x00008337
#endif
#ifndef GL_SAMPLER_EXTERNAL
#define GL_SAMPLER_EXTERNAL                                          0x00008D66
#endif
#ifndef GL_COMPRESSED_SIGNED_LUMINANCE_LATC1
#define GL_COMPRESSED_SIGNED_LUMINANCE_LATC1                         0x00008C71
#endif
#ifndef GL_FRAGMENT_LIGHT_MODEL_NORMAL_INTERPOLATION
#define GL_FRAGMENT_LIGHT_MODEL_NORMAL_INTERPOLATION                 0x0000840B
#endif
#ifndef GL_QUAD_MESH
#define GL_QUAD_MESH                                                 0x00008614
#endif
#ifndef GL_TEXTURE_MAX_CLAMP_S
#define GL_TEXTURE_MAX_CLAMP_S                                       0x00008369
#endif
#ifndef GL_SCALE_BY_FOUR
#define GL_SCALE_BY_FOUR                                             0x0000853F
#endif
#ifndef GL_MAX_CUBE_MAP_TEXTURE_SIZE
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE                                 0x0000851C
#endif
#ifndef GL_TEXTURE_WRAP_Q
#define GL_TEXTURE_WRAP_Q                                            0x00008137
#endif
#ifndef GL_NORMAL_ARRAY_BUFFER_BINDING
#define GL_NORMAL_ARRAY_BUFFER_BINDING                               0x00008897
#endif
#ifndef GL_VIDEO_COLOR_CONVERSION_MIN
#define GL_VIDEO_COLOR_CONVERSION_MIN                                0x0000902B
#endif
#ifndef GL_PROGRAM_FORMAT
#define GL_PROGRAM_FORMAT                                            0x00008876
#endif
#ifndef GL_SECONDARY_INTERPOLATOR
#define GL_SECONDARY_INTERPOLATOR                                    0x0000896D
#endif
#ifndef GL_RGB_SCALE
#define GL_RGB_SCALE                                                 0x00008573
#endif
#ifndef GL_RG16UI
#define GL_RG16UI                                                    0x0000823A
#endif
#ifndef GL_COUNT_DOWN
#define GL_COUNT_DOWN                                                0x00009089
#endif
#ifndef GL_COLOR_TABLE_WIDTH
#define GL_COLOR_TABLE_WIDTH                                         0x000080D9
#endif
#ifndef GL_HISTOGRAM_SINK
#define GL_HISTOGRAM_SINK                                            0x0000802D
#endif
#ifndef GL_R1UI_C3F_V3F
#define GL_R1UI_C3F_V3F                                              0x000085C6
#endif
#ifndef GL_MAX_ASYNC_READ_PIXELS
#define GL_MAX_ASYNC_READ_PIXELS                                     0x00008361
#endif
#ifndef GL_CLIP_NEAR_HINT
#define GL_CLIP_NEAR_HINT                                            0x0001A220
#endif
#ifndef GL_CON_9
#define GL_CON_9                                                     0x0000894A
#endif
#ifndef GL_SYNC_FLAGS
#define GL_SYNC_FLAGS                                                0x00009115
#endif
#ifndef GL_INTERLACE
#define GL_INTERLACE                                                 0x00008980
#endif
#ifndef GL_COMPARE_R_TO_TEXTURE
#define GL_COMPARE_R_TO_TEXTURE                                      0x0000884E
#endif
#ifndef GL_GREEN
#define GL_GREEN                                                     0x00001904
#endif
#ifndef GL_OPERAND1_RGB
#define GL_OPERAND1_RGB                                              0x00008591
#endif
#ifndef GL_MODELVIEW10
#define GL_MODELVIEW10                                               0x0000872A
#endif
#ifndef GL_VARIABLE_C
#define GL_VARIABLE_C                                                0x00008525
#endif
#ifndef GL_STRICT_SCISSOR_HINT
#define GL_STRICT_SCISSOR_HINT                                       0x0001A218
#endif
#ifndef GL_VARIANT_VALUE
#define GL_VARIANT_VALUE                                             0x000087E4
#endif
#ifndef GL_TRANSFORM_FEEDBACK_BARRIER_BIT
#define GL_TRANSFORM_FEEDBACK_BARRIER_BIT                            0x00000800
#endif
#ifndef GL_CUBIC_CURVE_TO
#define GL_CUBIC_CURVE_TO                                            0x0000000C
#endif
#ifndef GL_HSL_COLOR
#define GL_HSL_COLOR                                                 0x000092AF
#endif
#ifndef GL_PERFQUERY_COUNTER_THROUGHPUT
#define GL_PERFQUERY_COUNTER_THROUGHPUT                              0x000094F3
#endif
#ifndef GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE
#define GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE                          0x00008214
#endif
#ifndef GL_MODELVIEW1_MATRIX
#define GL_MODELVIEW1_MATRIX                                         0x00008506
#endif
#ifndef GL_SIGNED_LUMINANCE8_ALPHA8
#define GL_SIGNED_LUMINANCE8_ALPHA8                                  0x00008704
#endif
#ifndef GL_CONSTANT_ALPHA
#define GL_CONSTANT_ALPHA                                            0x00008003
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY6
#define GL_VERTEX_ATTRIB_ARRAY6                                      0x00008656
#endif
#ifndef GL_OP_EXP_BASE_2
#define GL_OP_EXP_BASE_2                                             0x00008791
#endif
#ifndef GL_MAX_PROGRAM_NATIVE_ATTRIBS
#define GL_MAX_PROGRAM_NATIVE_ATTRIBS                                0x000088AF
#endif
#ifndef GL_MAX_UNIFORM_BLOCK_SIZE
#define GL_MAX_UNIFORM_BLOCK_SIZE                                    0x00008A30
#endif
#ifndef GL_DOUBLE_MAT3x2
#define GL_DOUBLE_MAT3x2                                             0x00008F4B
#endif
#ifndef GL_STENCIL_BUFFER_BIT5
#define GL_STENCIL_BUFFER_BIT5                                       0x00200000
#endif
#ifndef GL_PATH_FILL_MASK
#define GL_PATH_FILL_MASK                                            0x00009081
#endif
#ifndef GL_FLOAT_RGBA_MODE
#define GL_FLOAT_RGBA_MODE                                           0x0000888E
#endif
#ifndef GL_DEBUG_SEVERITY_HIGH
#define GL_DEBUG_SEVERITY_HIGH                                       0x00009146
#endif
#ifndef GL_EDGE_FLAG_ARRAY_COUNT
#define GL_EDGE_FLAG_ARRAY_COUNT                                     0x0000808D
#endif
#ifndef GL_RGBA16UI
#define GL_RGBA16UI                                                  0x00008D76
#endif
#ifndef GL_PACK_ROW_LENGTH
#define GL_PACK_ROW_LENGTH                                           0x00000D02
#endif
#ifndef GL_DRAW_BUFFER5
#define GL_DRAW_BUFFER5                                              0x0000882A
#endif
#ifndef GL_TEXTURE_EXTERNAL
#define GL_TEXTURE_EXTERNAL                                          0x00008D65
#endif
#ifndef GL_SYNC_CL_EVENT
#define GL_SYNC_CL_EVENT                                             0x00008240
#endif
#ifndef GL_VERTEX_ELEMENT_SWIZZLE
#define GL_VERTEX_ELEMENT_SWIZZLE                                    0x000091A4
#endif
#ifndef GL_TEXTURE10
#define GL_TEXTURE10                                                 0x000084CA
#endif
#ifndef GL_TEXTURE7
#define GL_TEXTURE7                                                  0x000084C7
#endif
#ifndef GL_MAX_ASYNC_HISTOGRAM
#define GL_MAX_ASYNC_HISTOGRAM                                       0x0000832D
#endif
#ifndef GL_4PASS_2
#define GL_4PASS_2                                                   0x000080A6
#endif
#ifndef GL_MAP_ATTRIB_U_ORDER
#define GL_MAP_ATTRIB_U_ORDER                                        0x000086C3
#endif
#ifndef GL_SHARPEN_TEXTURE_FUNC_POINTS
#define GL_SHARPEN_TEXTURE_FUNC_POINTS                               0x000080B0
#endif
#ifndef GL_MAX_RATIONAL_EVAL_ORDER
#define GL_MAX_RATIONAL_EVAL_ORDER                                   0x000086D7
#endif
#ifndef GL_SYNC_CONDITION
#define GL_SYNC_CONDITION                                            0x00009113
#endif
#ifndef GL_EVAL_VERTEX_ATTRIB4
#define GL_EVAL_VERTEX_ATTRIB4                                       0x000086CA
#endif
#ifndef GL_MATRIX14
#define GL_MATRIX14                                                  0x000088CE
#endif
#ifndef GL_QUAD_ALPHA4
#define GL_QUAD_ALPHA4                                               0x0000811E
#endif
#ifndef GL_VERTEX_ARRAY_STRIDE
#define GL_VERTEX_ARRAY_STRIDE                                       0x0000807C
#endif
#ifndef GL_VERTEX_PRECLIP
#define GL_VERTEX_PRECLIP                                            0x000083EE
#endif
#ifndef GL_CONVOLUTION_HINT
#define GL_CONVOLUTION_HINT                                          0x00008316
#endif
#ifndef GL_COMPUTE_PROGRAM_PARAMETER_BUFFER
#define GL_COMPUTE_PROGRAM_PARAMETER_BUFFER                          0x000090FC
#endif
#ifndef GL_FUNC_REVERSE_SUBTRACT
#define GL_FUNC_REVERSE_SUBTRACT                                     0x0000800B
#endif
#ifndef GL_READ_FRAMEBUFFER
#define GL_READ_FRAMEBUFFER                                          0x00008CA8
#endif
#ifndef GL_REG_25
#define GL_REG_25                                                    0x0000893A
#endif
#ifndef GL_CURRENT_MATRIX_INDEX
#define GL_CURRENT_MATRIX_INDEX                                      0x00008845
#endif
#ifndef GL_TRANSFORM_FEEDBACK_BUFFER_MODE
#define GL_TRANSFORM_FEEDBACK_BUFFER_MODE                            0x00008C7F
#endif
#ifndef GL_DETAIL_TEXTURE_MODE
#define GL_DETAIL_TEXTURE_MODE                                       0x0000809B
#endif
#ifndef GL_COMBINER0
#define GL_COMBINER0                                                 0x00008550
#endif
#ifndef GL_IMAGE_2D
#define GL_IMAGE_2D                                                  0x0000904D
#endif
#ifndef GL_COVERAGE_COMPONENT4
#define GL_COVERAGE_COMPONENT4                                       0x00008ED1
#endif
#ifndef GL_INVERSE
#define GL_INVERSE                                                   0x0000862B
#endif
#ifndef GL_COORD_REPLACE
#define GL_COORD_REPLACE                                             0x00008862
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_4x4x4
#define GL_COMPRESSED_RGBA_ASTC_4x4x4                                0x000093C3
#endif
#ifndef GL_TESS_CONTROL_PROGRAM_PARAMETER_BUFFER
#define GL_TESS_CONTROL_PROGRAM_PARAMETER_BUFFER                     0x00008C74
#endif
#ifndef GL_PACK_SKIP_IMAGES
#define GL_PACK_SKIP_IMAGES                                          0x0000806B
#endif
#ifndef GL_NAMED_STRING_TYPE
#define GL_NAMED_STRING_TYPE                                         0x00008DEA
#endif
#ifndef GL_PATH_END_CAPS
#define GL_PATH_END_CAPS                                             0x00009076
#endif
#ifndef GL_PERFQUERY_QUERY_NAME_LENGTH_MAX
#define GL_PERFQUERY_QUERY_NAME_LENGTH_MAX                           0x000094FD
#endif
#ifndef GL_GEOMETRY_DEFORMATION_BIT
#define GL_GEOMETRY_DEFORMATION_BIT                                  0x00000002
#endif
#ifndef GL_UNSIGNED_SHORT_8_8_REV
#define GL_UNSIGNED_SHORT_8_8_REV                                    0x000085BB
#endif
#ifndef GL_RENDERBUFFER_STENCIL_SIZE
#define GL_RENDERBUFFER_STENCIL_SIZE                                 0x00008D55
#endif
#ifndef GL_RENDERBUFFER
#define GL_RENDERBUFFER                                              0x00008D41
#endif
#ifndef GL_LAYOUT_DEFAULT
#define GL_LAYOUT_DEFAULT                                            0x00000000
#endif
#ifndef GL_MODELVIEW20
#define GL_MODELVIEW20                                               0x00008734
#endif
#ifndef GL_MOVE_TO_CONTINUES
#define GL_MOVE_TO_CONTINUES                                         0x000090B6
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD20
#define GL_OUTPUT_TEXTURE_COORD20                                    0x000087B1
#endif
#ifndef GL_PIXEL_TILE_GRID_DEPTH
#define GL_PIXEL_TILE_GRID_DEPTH                                     0x00008144
#endif
#ifndef GL_UNIFORM_BARRIER_BIT
#define GL_UNIFORM_BARRIER_BIT                                       0x00000004
#endif
#ifndef GL_INT16_VEC3
#define GL_INT16_VEC3                                                0x00008FE6
#endif
#ifndef GL_TEXTURE_LOD_BIAS_R
#define GL_TEXTURE_LOD_BIAS_R                                        0x00008190
#endif
#ifndef GL_PROGRAM_NATIVE_TEX_INDIRECTIONS
#define GL_PROGRAM_NATIVE_TEX_INDIRECTIONS                           0x0000880A
#endif
#ifndef GL_RGB10_A2UI
#define GL_RGB10_A2UI                                                0x0000906F
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY_DIVISOR
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR                               0x000088FE
#endif
#ifndef GL_OFFSET_HILO_PROJECTIVE_TEXTURE_2D
#define GL_OFFSET_HILO_PROJECTIVE_TEXTURE_2D                         0x00008856
#endif
#ifndef GL_COLOR_BUFFER_BIT5
#define GL_COLOR_BUFFER_BIT5                                         0x00000020
#endif
#ifndef GL_TRANSFORM_FEEDBACK_BINDING
#define GL_TRANSFORM_FEEDBACK_BINDING                                0x00008E25
#endif
#ifndef GL_LARGE_CCW_ARC_TO
#define GL_LARGE_CCW_ARC_TO                                          0x00000016
#endif
#ifndef GL_UNSIGNED_INT_SAMPLER_1D
#define GL_UNSIGNED_INT_SAMPLER_1D                                   0x00008DD1
#endif
#ifndef GL_PIXEL_COUNT_AVAILABLE
#define GL_PIXEL_COUNT_AVAILABLE                                     0x00008867
#endif
#ifndef GL_DEPTH_TEXTURE_MODE
#define GL_DEPTH_TEXTURE_MODE                                        0x0000884B
#endif
#ifndef GL_MAX_SPARSE_TEXTURE_SIZE
#define GL_MAX_SPARSE_TEXTURE_SIZE                                   0x00009198
#endif
#ifndef GL_COLOR_ATTACHMENT
#define GL_COLOR_ATTACHMENT                                          0x000090F0
#endif
#ifndef GL_COMPARE_REF_TO_TEXTURE
#define GL_COMPARE_REF_TO_TEXTURE                                    0x0000884E
#endif
#ifndef GL_PARTIAL_SUCCESS
#define GL_PARTIAL_SUCCESS                                           0x0000902E
#endif
#ifndef GL_EMBOSS_MAP
#define GL_EMBOSS_MAP                                                0x0000855F
#endif
#ifndef GL_PRIMITIVE_RESTART_INDEX
#define GL_PRIMITIVE_RESTART_INDEX                                   0x00008559
#endif
#ifndef GL_LARGE_CW_ARC_TO
#define GL_LARGE_CW_ARC_TO                                           0x00000018
#endif
#ifndef GL_422
#define GL_422                                                       0x000080CC
#endif
#ifndef GL_CONVOLUTION_WIDTH
#define GL_CONVOLUTION_WIDTH                                         0x00008018
#endif
#ifndef GL_TRIANGLE_STRIP_ADJACENCY
#define GL_TRIANGLE_STRIP_ADJACENCY                                  0x0000000D
#endif
#ifndef GL_CONST_EYE
#define GL_CONST_EYE                                                 0x000086E5
#endif
#ifndef GL_INTENSITY_FLOAT32
#define GL_INTENSITY_FLOAT32                                         0x00008817
#endif
#ifndef GL_PROGRAM_FORMAT_ASCII
#define GL_PROGRAM_FORMAT_ASCII                                      0x00008875
#endif
#ifndef GL_VERTEX_ARRAY_RANGE_POINTER
#define GL_VERTEX_ARRAY_RANGE_POINTER                                0x00008521
#endif
#ifndef GL_COVERAGE_SAMPLES
#define GL_COVERAGE_SAMPLES                                          0x00008ED4
#endif
#ifndef GL_CND
#define GL_CND                                                       0x0000896A
#endif
#ifndef GL_PATH_INITIAL_DASH_CAP
#define GL_PATH_INITIAL_DASH_CAP                                     0x0000907C
#endif
#ifndef GL_REFERENCE_PLANE_EQUATION
#define GL_REFERENCE_PLANE_EQUATION                                  0x0000817E
#endif
#ifndef GL_INT_IMAGE_BUFFER
#define GL_INT_IMAGE_BUFFER                                          0x0000905C
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_4x4
#define GL_COMPRESSED_RGBA_ASTC_4x4                                  0x000093B0
#endif
#ifndef GL_SAMPLE_COVERAGE_INVERT
#define GL_SAMPLE_COVERAGE_INVERT                                    0x000080AB
#endif
#ifndef GL_STENCIL_BUFFER_BIT2
#define GL_STENCIL_BUFFER_BIT2                                       0x00040000
#endif
#ifndef GL_FLOAT_R
#define GL_FLOAT_R                                                   0x00008880
#endif
#ifndef GL_TEXTURE_DEPTH_SIZE
#define GL_TEXTURE_DEPTH_SIZE                                        0x0000884A
#endif
#ifndef GL_SAMPLE_ALPHA_TO_COVERAGE
#define GL_SAMPLE_ALPHA_TO_COVERAGE                                  0x0000809E
#endif
#ifndef GL_RASTERIZER_DISCARD
#define GL_RASTERIZER_DISCARD                                        0x00008C89
#endif
#ifndef GL_PROGRAM_SEPARABLE
#define GL_PROGRAM_SEPARABLE                                         0x00008258
#endif
#ifndef GL_INVERTED_SCREEN_W
#define GL_INVERTED_SCREEN_W                                         0x00008491
#endif
#ifndef GL_UNSIGNED_INT_S8_S8_8_8
#define GL_UNSIGNED_INT_S8_S8_8_8                                    0x000086DA
#endif
#ifndef GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT                         0x00008CD6
#endif
#ifndef GL_MATRIX20
#define GL_MATRIX20                                                  0x000088D4
#endif
#ifndef GL_TEXTURE_RESIDENT
#define GL_TEXTURE_RESIDENT                                          0x00008067
#endif
#ifndef GL_PATH_TERMINAL_END_CAP
#define GL_PATH_TERMINAL_END_CAP                                     0x00009078
#endif
#ifndef GL_BUFFER_UPDATE_BARRIER_BIT
#define GL_BUFFER_UPDATE_BARRIER_BIT                                 0x00000200
#endif
#ifndef GL_POST_COLOR_MATRIX_RED_BIAS
#define GL_POST_COLOR_MATRIX_RED_BIAS                                0x000080B8
#endif
#ifndef GL_TEXCOORD1_BIT
#define GL_TEXCOORD1_BIT                                             0x10000000
#endif
#ifndef GL_PATH_DASH_CAPS
#define GL_PATH_DASH_CAPS                                            0x0000907B
#endif
#ifndef GL_VERTEX_WEIGHTING
#define GL_VERTEX_WEIGHTING                                          0x00008509
#endif
#ifndef GL_PERFORMANCE_MONITOR
#define GL_PERFORMANCE_MONITOR                                       0x00009152
#endif
#ifndef GL_VIRTUAL_PAGE_SIZE_Z
#define GL_VIRTUAL_PAGE_SIZE_Z                                       0x00009197
#endif
#ifndef GL_CURRENT_FOG_COORDINATE
#define GL_CURRENT_FOG_COORDINATE                                    0x00008453
#endif
#ifndef GL_MODELVIEW0_STACK_DEPTH
#define GL_MODELVIEW0_STACK_DEPTH                                    0x00000BA3
#endif
#ifndef GL_SKIP_COMPONENTS3
#define GL_SKIP_COMPONENTS3                                          0xFFFFFFFFFFFFFFFC
#endif
#ifndef GL_ALL_BARRIER_BITS
#define GL_ALL_BARRIER_BITS                                          0xFFFFFFFF
#endif
#ifndef GL_COMBINE_RGB
#define GL_COMBINE_RGB                                               0x00008571
#endif
#ifndef GL_GENERIC_ATTRIB
#define GL_GENERIC_ATTRIB                                            0x00008C7D
#endif
#ifndef GL_FRAMEBUFFER_UNDEFINED
#define GL_FRAMEBUFFER_UNDEFINED                                     0x00008219
#endif
#ifndef GL_LUMINANCE_ALPHA16UI
#define GL_LUMINANCE_ALPHA16UI                                       0x00008D7B
#endif
#ifndef GL_LUMINANCE16I
#define GL_LUMINANCE16I                                              0x00008D8C
#endif
#ifndef GL_MAX_OPTIMIZED_VERTEX_SHADER_VARIANTS
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_VARIANTS                      0x000087CB
#endif
#ifndef GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET
#define GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET                         0x00008E5E
#endif
#ifndef GL_TEXTURE_COORD_ARRAY
#define GL_TEXTURE_COORD_ARRAY                                       0x00008078
#endif
#ifndef GL_OFFSET_TEXTURE_2D_MATRIX
#define GL_OFFSET_TEXTURE_2D_MATRIX                                  0x000086E1
#endif
#ifndef GL_T2F_IUI_V3F
#define GL_T2F_IUI_V3F                                               0x000081B2
#endif
#ifndef GL_TEXTURE_MULTI_BUFFER_HINT
#define GL_TEXTURE_MULTI_BUFFER_HINT                                 0x0000812E
#endif
#ifndef GL_MAP2_VERTEX_ATTRIB12_4
#define GL_MAP2_VERTEX_ATTRIB12_4                                    0x0000867C
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD22
#define GL_OUTPUT_TEXTURE_COORD22                                    0x000087B3
#endif
#ifndef GL_BINORMAL_ARRAY_STRIDE
#define GL_BINORMAL_ARRAY_STRIDE                                     0x00008441
#endif
#ifndef GL_PACK_REVERSE_ROW_ORDER
#define GL_PACK_REVERSE_ROW_ORDER                                    0x000093A4
#endif
#ifndef GL_BGR
#define GL_BGR                                                       0x000080E0
#endif
#ifndef GL_IUI_N3F_V3F
#define GL_IUI_N3F_V3F                                               0x000081B0
#endif
#ifndef GL_INT8_VEC2
#define GL_INT8_VEC2                                                 0x00008FE1
#endif
#ifndef GL_FLOAT16_VEC4
#define GL_FLOAT16_VEC4                                              0x00008FFB
#endif
#ifndef GL_PREVIOUS_TEXTURE_INPUT
#define GL_PREVIOUS_TEXTURE_INPUT                                    0x000086E4
#endif
#ifndef GL_DEPENDENT_AR_TEXTURE_2D
#define GL_DEPENDENT_AR_TEXTURE_2D                                   0x000086E9
#endif
#ifndef GL_SIGNED_HILO8
#define GL_SIGNED_HILO8                                              0x0000885F
#endif
#ifndef GL_MAX_FRAGMENT_BINDABLE_UNIFORMS
#define GL_MAX_FRAGMENT_BINDABLE_UNIFORMS                            0x00008DE3
#endif
#ifndef GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE
#define GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE                         0x00008215
#endif
#ifndef GL_ALWAYS_FAST_HINT
#define GL_ALWAYS_FAST_HINT                                          0x0001A20C
#endif
#ifndef GL_CON_1
#define GL_CON_1                                                     0x00008942
#endif
#ifndef GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS                            0x00008B4C
#endif
#ifndef GL_BUFFER_MAP_OFFSET
#define GL_BUFFER_MAP_OFFSET                                         0x00009121
#endif
#ifndef GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT
#define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT                        0x00008E8F
#endif
#ifndef GL_FOG_COORDINATE_ARRAY_LIST_STRIDE
#define GL_FOG_COORDINATE_ARRAY_LIST_STRIDE                          0x000192AE
#endif
#ifndef GL_MAX_PROGRAM_TOTAL_OUTPUT_COMPONENTS
#define GL_MAX_PROGRAM_TOTAL_OUTPUT_COMPONENTS                       0x00008C28
#endif
#ifndef GL_MITER_REVERT
#define GL_MITER_REVERT                                              0x000090A7
#endif
#ifndef GL_PROVOKING_VERTEX
#define GL_PROVOKING_VERTEX                                          0x00008E4F
#endif
#ifndef GL_QUAD_LUMINANCE4
#define GL_QUAD_LUMINANCE4                                           0x00008120
#endif
#ifndef GL_MAT_SPECULAR_BIT
#define GL_MAT_SPECULAR_BIT                                          0x04000000
#endif
#ifndef GL_RGB8UI
#define GL_RGB8UI                                                    0x00008D7D
#endif
#ifndef GL_MAP1_VERTEX_ATTRIB14_4
#define GL_MAP1_VERTEX_ATTRIB14_4                                    0x0000866E
#endif
#ifndef GL_MAX_PROGRAM_LOOP_COUNT
#define GL_MAX_PROGRAM_LOOP_COUNT                                    0x000088F8
#endif
#ifndef GL_R11F_G11F_B10F
#define GL_R11F_G11F_B10F                                            0x00008C3A
#endif
#ifndef GL_ASYNC_READ_PIXELS
#define GL_ASYNC_READ_PIXELS                                         0x0000835E
#endif
#ifndef GL_COMBINER_MUX_SUM
#define GL_COMBINER_MUX_SUM                                          0x00008547
#endif
#ifndef GL_UNIFORM_NAME_LENGTH
#define GL_UNIFORM_NAME_LENGTH                                       0x00008A39
#endif
#ifndef GL_COLOR_INDEX8
#define GL_COLOR_INDEX8                                              0x000080E5
#endif
#ifndef GL_NUM_FRAGMENT_CONSTANTS
#define GL_NUM_FRAGMENT_CONSTANTS                                    0x0000896F
#endif
#ifndef GL_TEXTURE_COORD_ARRAY_LIST
#define GL_TEXTURE_COORD_ARRAY_LIST                                  0x000192A2
#endif
#ifndef GL_ACTIVE_STENCIL_FACE
#define GL_ACTIVE_STENCIL_FACE                                       0x00008911
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_10x5
#define GL_COMPRESSED_RGBA_ASTC_10x5                                 0x000093B8
#endif
#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER                                             0x00008B31
#endif
#ifndef GL_PERFQUERY_COUNTER_DATA_UINT32
#define GL_PERFQUERY_COUNTER_DATA_UINT32                             0x000094F8
#endif
#ifndef GL_UNSIGNED_INT_8_8_8_8
#define GL_UNSIGNED_INT_8_8_8_8                                      0x00008035
#endif
#ifndef GL_PATH_FILL_MODE
#define GL_PATH_FILL_MODE                                            0x00009080
#endif
#ifndef GL_UNPACK_IMAGE_DEPTH
#define GL_UNPACK_IMAGE_DEPTH                                        0x00008133
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD0
#define GL_OUTPUT_TEXTURE_COORD0                                     0x0000879D
#endif
#ifndef GL_QUERY_BUFFER
#define GL_QUERY_BUFFER                                              0x00009192
#endif
#ifndef GL_SOURCE1_ALPHA
#define GL_SOURCE1_ALPHA                                             0x00008589
#endif
#ifndef GL_EYE_PLANE_ABSOLUTE
#define GL_EYE_PLANE_ABSOLUTE                                        0x0000855C
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_8x5
#define GL_COMPRESSED_RGBA_ASTC_8x5                                  0x000093B5
#endif
#ifndef GL_R32I
#define GL_R32I                                                      0x00008235
#endif
#ifndef GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS                          0x00008B4D
#endif
#ifndef GL_MAX_COMPUTE_FIXED_GROUP_INVOCATIONS
#define GL_MAX_COMPUTE_FIXED_GROUP_INVOCATIONS                       0x000090EB
#endif
#ifndef GL_TEXTURE_SWIZZLE_R
#define GL_TEXTURE_SWIZZLE_R                                         0x00008E42
#endif
#ifndef GL_DUAL_LUMINANCE_ALPHA4
#define GL_DUAL_LUMINANCE_ALPHA4                                     0x0000811C
#endif
#ifndef GL_TEXTURE_LIGHTING_MODE
#define GL_TEXTURE_LIGHTING_MODE                                     0x00008167
#endif
#ifndef GL_REQUIRED_TEXTURE_IMAGE_UNITS
#define GL_REQUIRED_TEXTURE_IMAGE_UNITS                              0x00008D68
#endif
#ifndef GL_ALPHA_TEST_FUNC
#define GL_ALPHA_TEST_FUNC                                           0x00000BC1
#endif
#ifndef GL_TEXTURE_RECTANGLE
#define GL_TEXTURE_RECTANGLE                                         0x000084F5
#endif
#ifndef GL_UNCORRELATED
#define GL_UNCORRELATED                                              0x00009282
#endif
#ifndef GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS                         0x00008CD9
#endif
#ifndef GL_MAX_FRAGMENT_INTERPOLATION_OFFSET
#define GL_MAX_FRAGMENT_INTERPOLATION_OFFSET                         0x00008E5C
#endif
#ifndef GL_QUERY_OBJECT
#define GL_QUERY_OBJECT                                              0x00009153
#endif
#ifndef GL_MODELVIEW30
#define GL_MODELVIEW30                                               0x0000873E
#endif
#ifndef GL_FIXED_ONLY
#define GL_FIXED_ONLY                                                0x0000891D
#endif
#ifndef GL_INT64_VEC2
#define GL_INT64_VEC2                                                0x00008FE9
#endif
#ifndef GL_SAMPLE_MASK
#define GL_SAMPLE_MASK                                               0x000080A0
#endif
#ifndef GL_HALF_BIAS_NORMAL
#define GL_HALF_BIAS_NORMAL                                          0x0000853A
#endif
#ifndef GL_TRIANGULAR
#define GL_TRIANGULAR                                                0x000090A5
#endif
#ifndef GL_RG32F
#define GL_RG32F                                                     0x00008230
#endif
#ifndef GL_MAGNITUDE_BIAS
#define GL_MAGNITUDE_BIAS                                            0x00008718
#endif
#ifndef GL_SIGNED_LUMINANCE
#define GL_SIGNED_LUMINANCE                                          0x00008701
#endif
#ifndef GL_UNSIGNED_INT16_VEC4
#define GL_UNSIGNED_INT16_VEC4                                       0x00008FF3
#endif
#ifndef GL_EVAL_VERTEX_ATTRIB12
#define GL_EVAL_VERTEX_ATTRIB12                                      0x000086D2
#endif
#ifndef GL_SPARE0
#define GL_SPARE0                                                    0x0000852E
#endif
#ifndef GL_UNIFORM_TYPE
#define GL_UNIFORM_TYPE                                              0x00008A37
#endif
#ifndef GL_CONVOLUTION_FILTER_SCALE
#define GL_CONVOLUTION_FILTER_SCALE                                  0x00008014
#endif
#ifndef GL_REG_1
#define GL_REG_1                                                     0x00008922
#endif
#ifndef GL_SINGLE_COLOR
#define GL_SINGLE_COLOR                                              0x000081F9
#endif
#ifndef GL_DOT_PRODUCT_TEXTURE_1D
#define GL_DOT_PRODUCT_TEXTURE_1D                                    0x0000885C
#endif
#ifndef GL_DS_BIAS
#define GL_DS_BIAS                                                   0x00008716
#endif
#ifndef GL_MODELVIEW19
#define GL_MODELVIEW19                                               0x00008733
#endif
#ifndef GL_MAX_PROGRAM_ENV_PARAMETERS
#define GL_MAX_PROGRAM_ENV_PARAMETERS                                0x000088B5
#endif
#ifndef GL_MAX_COLOR_MATRIX_STACK_DEPTH
#define GL_MAX_COLOR_MATRIX_STACK_DEPTH                              0x000080B3
#endif
#ifndef GL_EVAL_VERTEX_ATTRIB2
#define GL_EVAL_VERTEX_ATTRIB2                                       0x000086C8
#endif
#ifndef GL_UNSIGNED_INT64
#define GL_UNSIGNED_INT64                                            0x00008BC2
#endif
#ifndef GL_FACTOR_MAX
#define GL_FACTOR_MAX                                                0x0000901D
#endif
#ifndef GL_PROGRAM_NATIVE_TEMPORARIES
#define GL_PROGRAM_NATIVE_TEMPORARIES                                0x000088A6
#endif
#ifndef GL_MAT_AMBIENT_BIT
#define GL_MAT_AMBIENT_BIT                                           0x00100000
#endif
#ifndef GL_MAT_DIFFUSE_BIT
#define GL_MAT_DIFFUSE_BIT                                           0x00400000
#endif
#ifndef GL_NORMAL_ARRAY_ADDRESS
#define GL_NORMAL_ARRAY_ADDRESS                                      0x00008F22
#endif
#ifndef GL_SYNC_FENCE
#define GL_SYNC_FENCE                                                0x00009116
#endif
#ifndef GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR                             0x0000824E
#endif
#ifndef GL_TEXTURE_GREEN_TYPE
#define GL_TEXTURE_GREEN_TYPE                                        0x00008C11
#endif
#ifndef GL_DISTANCE_ATTENUATION
#define GL_DISTANCE_ATTENUATION                                      0x00008129
#endif
#ifndef GL_MAX_TRACK_MATRICES
#define GL_MAX_TRACK_MATRICES                                        0x0000862F
#endif
#ifndef GL_FLOAT_MAT4x2
#define GL_FLOAT_MAT4x2                                              0x00008B69
#endif
#ifndef GL_PIXEL_UNPACK_BUFFER_BINDING
#define GL_PIXEL_UNPACK_BUFFER_BINDING                               0x000088EF
#endif
#ifndef GL_INT_SAMPLER_2D
#define GL_INT_SAMPLER_2D                                            0x00008DCA
#endif
#ifndef GL_RG8
#define GL_RG8                                                       0x0000822B
#endif
#ifndef GL_UNSIGNED_INT_IMAGE_BUFFER
#define GL_UNSIGNED_INT_IMAGE_BUFFER                                 0x00009067
#endif
#ifndef GL_OP_DOT3
#define GL_OP_DOT3                                                   0x00008784
#endif
#ifndef GL_FRAGMENT_LIGHT_MODEL_AMBIENT
#define GL_FRAGMENT_LIGHT_MODEL_AMBIENT                              0x0000840A
#endif
#ifndef GL_RENDERBUFFER_COLOR_SAMPLES
#define GL_RENDERBUFFER_COLOR_SAMPLES                                0x00008E10
#endif
#ifndef GL_TEXTURE_INDEX_SIZE
#define GL_TEXTURE_INDEX_SIZE                                        0x000080ED
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD7
#define GL_OUTPUT_TEXTURE_COORD7                                     0x000087A4
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5                        0x000093E6
#endif
#ifndef GL_COLOR_ATTACHMENT13
#define GL_COLOR_ATTACHMENT13                                        0x00008CED
#endif
#ifndef GL_TEXTURE_LOD_BIAS
#define GL_TEXTURE_LOD_BIAS                                          0x00008501
#endif
#ifndef GL_NEGATIVE_X
#define GL_NEGATIVE_X                                                0x000087D9
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD16
#define GL_OUTPUT_TEXTURE_COORD16                                    0x000087AD
#endif
#ifndef GL_ELEMENT_ARRAY_POINTER
#define GL_ELEMENT_ARRAY_POINTER                                     0x00008A0E
#endif
#ifndef GL_IUI_V3F
#define GL_IUI_V3F                                                   0x000081AE
#endif
#ifndef GL_TEXTURE_POST_SPECULAR
#define GL_TEXTURE_POST_SPECULAR                                     0x00008168
#endif
#ifndef GL_TRANSLATE_Y
#define GL_TRANSLATE_Y                                               0x0000908F
#endif
#ifndef GL_SPRITE_OBJECT_ALIGNED
#define GL_SPRITE_OBJECT_ALIGNED                                     0x0000814D
#endif
#ifndef GL_PERFQUERY_COUNTER_RAW
#define GL_PERFQUERY_COUNTER_RAW                                     0x000094F4
#endif
#ifndef GL_COLOR_ARRAY_ADDRESS
#define GL_COLOR_ARRAY_ADDRESS                                       0x00008F23
#endif
#ifndef GL_HISTOGRAM
#define GL_HISTOGRAM                                                 0x00008024
#endif
#ifndef GL_LUMINANCE_INTEGER
#define GL_LUMINANCE_INTEGER                                         0x00008D9C
#endif
#ifndef GL_MODELVIEW17
#define GL_MODELVIEW17                                               0x00008731
#endif
#ifndef GL_DEBUG_SOURCE_THIRD_PARTY
#define GL_DEBUG_SOURCE_THIRD_PARTY                                  0x00008249
#endif
#ifndef GL_VECTOR
#define GL_VECTOR                                                    0x000087BF
#endif
#ifndef GL_VERTEX_SOURCE
#define GL_VERTEX_SOURCE                                             0x00008774
#endif
#ifndef GL_READ_WRITE
#define GL_READ_WRITE                                                0x000088BA
#endif
#ifndef GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM
#define GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM                          0x00009047
#endif
#ifndef GL_MINUS_CLAMPED
#define GL_MINUS_CLAMPED                                             0x000092B3
#endif
#ifndef GL_UNSIGNED_INT_SAMPLER_1D_ARRAY
#define GL_UNSIGNED_INT_SAMPLER_1D_ARRAY                             0x00008DD6
#endif
#ifndef GL_DST_OUT
#define GL_DST_OUT                                                   0x0000928D
#endif
#ifndef GL_TEXTURE_MAG_SIZE
#define GL_TEXTURE_MAG_SIZE                                          0x0000871F
#endif
#ifndef GL_EVAL_VERTEX_ATTRIB1
#define GL_EVAL_VERTEX_ATTRIB1                                       0x000086C7
#endif
#ifndef GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS                   0x00008C8B
#endif
#ifndef GL_POST_COLOR_MATRIX_BLUE_SCALE
#define GL_POST_COLOR_MATRIX_BLUE_SCALE                              0x000080B6
#endif
#ifndef GL_TEXTURE_MATERIAL_PARAMETER
#define GL_TEXTURE_MATERIAL_PARAMETER                                0x00008352
#endif
#ifndef GL_DYNAMIC
#define GL_DYNAMIC                                                   0x00008761
#endif
#ifndef GL_BOOL
#define GL_BOOL                                                      0x00008B56
#endif
#ifndef GL_UNSIGNED_INT8
#define GL_UNSIGNED_INT8                                             0x00008FEC
#endif
#ifndef GL_MAX_CLIPMAP_VIRTUAL_DEPTH
#define GL_MAX_CLIPMAP_VIRTUAL_DEPTH                                 0x00008178
#endif
#ifndef GL_OP_DOT4
#define GL_OP_DOT4                                                   0x00008785
#endif
#ifndef GL_SET
#define GL_SET                                                       0x0000874A
#endif
#ifndef GL_POST_COLOR_MATRIX_GREEN_SCALE
#define GL_POST_COLOR_MATRIX_GREEN_SCALE                             0x000080B5
#endif
#ifndef GL_SOURCE2_ALPHA
#define GL_SOURCE2_ALPHA                                             0x0000858A
#endif
#ifndef GL_DRAW_INDIRECT_LENGTH
#define GL_DRAW_INDIRECT_LENGTH                                      0x00008F42
#endif
#ifndef GL_UNIFORM_MATRIX_STRIDE
#define GL_UNIFORM_MATRIX_STRIDE                                     0x00008A3D
#endif
#ifndef GL_OBJECT_ATTACHED_OBJECTS
#define GL_OBJECT_ATTACHED_OBJECTS                                   0x00008B85
#endif
#ifndef GL_IGNORE_BORDER
#define GL_IGNORE_BORDER                                             0x00008150
#endif
#ifndef GL_TEXTURE8
#define GL_TEXTURE8                                                  0x000084C8
#endif
#ifndef GL_2X_BIT
#define GL_2X_BIT                                                    0x00000001
#endif
#ifndef GL_ITALIC_BIT
#define GL_ITALIC_BIT                                                0x00000002
#endif
#ifndef GL_REG_22
#define GL_REG_22                                                    0x00008937
#endif
#ifndef GL_GLOBAL_ALPHA_FACTOR
#define GL_GLOBAL_ALPHA_FACTOR                                       0x000081DA
#endif
#ifndef GL_SHADOW_AMBIENT
#define GL_SHADOW_AMBIENT                                            0x000080BF
#endif
#ifndef GL_SRGB8_ALPHA8
#define GL_SRGB8_ALPHA8                                              0x00008C43
#endif
#ifndef GL_VERTEX_PRECLIP_HINT
#define GL_VERTEX_PRECLIP_HINT                                       0x000083EF
#endif
#ifndef GL_MALI_PROGRAM_BINARY
#define GL_MALI_PROGRAM_BINARY                                       0x00008F61
#endif
#ifndef GL_RGBA32I
#define GL_RGBA32I                                                   0x00008D82
#endif
#ifndef GL_MULTIPLY
#define GL_MULTIPLY                                                  0x00009294
#endif
#ifndef GL_FRAMEBUFFER_ATTACHMENT_LAYERED
#define GL_FRAMEBUFFER_ATTACHMENT_LAYERED                            0x00008DA7
#endif
#ifndef GL_OP_MAX
#define GL_OP_MAX                                                    0x0000878A
#endif
#ifndef GL_FRAGMENT_MATERIAL
#define GL_FRAGMENT_MATERIAL                                         0x00008349
#endif
#ifndef GL_TEXTURE_BUFFER_DATA_STORE_BINDING
#define GL_TEXTURE_BUFFER_DATA_STORE_BINDING                         0x00008C2D
#endif
#ifndef GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING                       0x0000889D
#endif
#ifndef GL_TEXTURE_BINDING_1D_ARRAY
#define GL_TEXTURE_BINDING_1D_ARRAY                                  0x00008C1C
#endif
#ifndef GL_INDEX_MATERIAL_FACE
#define GL_INDEX_MATERIAL_FACE                                       0x000081BA
#endif
#ifndef GL_SWIZZLE_STRQ_DQ
#define GL_SWIZZLE_STRQ_DQ                                           0x0000897B
#endif
#ifndef GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN
#define GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN                     0x00008C88
#endif
#ifndef GL_CONSTANT
#define GL_CONSTANT                                                  0x00008576
#endif
#ifndef GL_SRC_OUT
#define GL_SRC_OUT                                                   0x0000928C
#endif
#ifndef GL_STENCIL_OP_VALUE
#define GL_STENCIL_OP_VALUE                                          0x0000874C
#endif
#ifndef GL_PROGRAM_ERROR_POSITION
#define GL_PROGRAM_ERROR_POSITION                                    0x0000864B
#endif
#ifndef GL_SURFACE_MAPPED
#define GL_SURFACE_MAPPED                                            0x00008700
#endif
#ifndef GL_PATH_GEN_COLOR_FORMAT
#define GL_PATH_GEN_COLOR_FORMAT                                     0x000090B2
#endif
#ifndef GL_BUFFER_SERIALIZED_MODIFY
#define GL_BUFFER_SERIALIZED_MODIFY                                  0x00008A12
#endif
#ifndef GL_STACK_UNDERFLOW
#define GL_STACK_UNDERFLOW                                           0x00000504
#endif
#ifndef GL_TEXTURE_4D_BINDING
#define GL_TEXTURE_4D_BINDING                                        0x0000814F
#endif
#ifndef GL_FONT_X_MAX_BOUNDS_BIT
#define GL_FONT_X_MAX_BOUNDS_BIT                                     0x00040000
#endif
#ifndef GL_STATIC_DRAW
#define GL_STATIC_DRAW                                               0x000088E4
#endif
#ifndef GL_MATRIX18
#define GL_MATRIX18                                                  0x000088D2
#endif
#ifndef GL_COMPRESSED_RGB
#define GL_COMPRESSED_RGB                                            0x000084ED
#endif
#ifndef GL_OUTPUT_FOG
#define GL_OUTPUT_FOG                                                0x000087BD
#endif
#ifndef GL_MAX_SPOT_EXPONENT
#define GL_MAX_SPOT_EXPONENT                                         0x00008505
#endif
#ifndef GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY
#define GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY                   0x0000906C
#endif
#ifndef GL_COMP_BIT
#define GL_COMP_BIT                                                  0x00000002
#endif
#ifndef GL_SAMPLER_CUBE_SHADOW
#define GL_SAMPLER_CUBE_SHADOW                                       0x00008DC5
#endif
#ifndef GL_DEBUG_CATEGORY_PERFORMANCE
#define GL_DEBUG_CATEGORY_PERFORMANCE                                0x0000914D
#endif
#ifndef GL_IMAGE_SCALE_X
#define GL_IMAGE_SCALE_X                                             0x00008155
#endif
#ifndef GL_MATRIX30
#define GL_MATRIX30                                                  0x000088DE
#endif
#ifndef GL_TEXTURE_INTENSITY_TYPE
#define GL_TEXTURE_INTENSITY_TYPE                                    0x00008C15
#endif
#ifndef GL_VERTEX_SHADER_INSTRUCTIONS
#define GL_VERTEX_SHADER_INSTRUCTIONS                                0x000087CF
#endif
#ifndef GL_MAX_PROGRAM_PARAMETERS
#define GL_MAX_PROGRAM_PARAMETERS                                    0x000088A9
#endif
#ifndef GL_MAX_FRAMEZOOM_FACTOR
#define GL_MAX_FRAMEZOOM_FACTOR                                      0x0000818D
#endif
#ifndef GL_DEPTH_BUFFER_BIT3
#define GL_DEPTH_BUFFER_BIT3                                         0x00000800
#endif
#ifndef GL_MAX_PROGRAM_SUBROUTINE_NUM
#define GL_MAX_PROGRAM_SUBROUTINE_NUM                                0x00008F45
#endif
#ifndef GL_RGB8
#define GL_RGB8                                                      0x00008051
#endif
#ifndef GL_TANGENT_ARRAY_POINTER
#define GL_TANGENT_ARRAY_POINTER                                     0x00008442
#endif
#ifndef GL_PROGRAM
#define GL_PROGRAM                                                   0x000082E2
#endif
#ifndef GL_SRGB
#define GL_SRGB                                                      0x00008C40
#endif
#ifndef GL_SMOOTH_CUBIC_CURVE_TO
#define GL_SMOOTH_CUBIC_CURVE_TO                                     0x00000010
#endif
#ifndef GL_FOG_SPECULAR_TEXTURE
#define GL_FOG_SPECULAR_TEXTURE                                      0x000080EC
#endif
#ifndef GL_GENERATE_MIPMAP_HINT
#define GL_GENERATE_MIPMAP_HINT                                      0x00008192
#endif
#ifndef GL_RGB16UI
#define GL_RGB16UI                                                   0x00008D77
#endif
#ifndef GL_MVP_MATRIX
#define GL_MVP_MATRIX                                                0x000087E3
#endif
#ifndef GL_CURRENT_VERTEX
#define GL_CURRENT_VERTEX                                            0x000087E2
#endif
#ifndef GL_BOUNDING_BOX
#define GL_BOUNDING_BOX                                              0x0000908D
#endif
#ifndef GL_LUMINANCE12_ALPHA4
#define GL_LUMINANCE12_ALPHA4                                        0x00008046
#endif
#ifndef GL_CON_31
#define GL_CON_31                                                    0x00008960
#endif
#ifndef GL_FIELD_UPPER
#define GL_FIELD_UPPER                                               0x00009022
#endif
#ifndef GL_TRANSLATED_SHADER_SOURCE_LENGTH
#define GL_TRANSLATED_SHADER_SOURCE_LENGTH                           0x000093A0
#endif
#ifndef GL_DEPTH_STENCIL
#define GL_DEPTH_STENCIL                                             0x000084F9
#endif
#ifndef GL_PALETTE8_RGBA4
#define GL_PALETTE8_RGBA4                                            0x00008B98
#endif
#ifndef GL_MIN_SPARSE_LEVEL
#define GL_MIN_SPARSE_LEVEL                                          0x0000919B
#endif
#ifndef GL_TRANSPOSE_MODELVIEW_MATRIX
#define GL_TRANSPOSE_MODELVIEW_MATRIX                                0x000084E3
#endif
#ifndef GL_TEXTURE_4D
#define GL_TEXTURE_4D                                                0x00008134
#endif
#ifndef GL_TEXTURE20
#define GL_TEXTURE20                                                 0x000084D4
#endif
#ifndef GL_REPLACE_OLDEST
#define GL_REPLACE_OLDEST                                            0x00000003
#endif
#ifndef GL_FRAMEZOOM
#define GL_FRAMEZOOM                                                 0x0000818B
#endif
#ifndef GL_GUILTY_CONTEXT_RESET
#define GL_GUILTY_CONTEXT_RESET                                      0x00008253
#endif
#ifndef GL_CURRENT_ATTRIB
#define GL_CURRENT_ATTRIB                                            0x00008626
#endif
#ifndef GL_REG_30
#define GL_REG_30                                                    0x0000893F
#endif
#ifndef GL_SMALL_CW_ARC_TO
#define GL_SMALL_CW_ARC_TO                                           0x00000014
#endif
#ifndef GL_FRAMEBUFFER_INCOMPLETE_FORMATS
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS                            0x00008CDA
#endif
#ifndef GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR
#define GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR                         0x0000914C
#endif
#ifndef GL_MATRIX_INDEX_ARRAY_STRIDE
#define GL_MATRIX_INDEX_ARRAY_STRIDE                                 0x00008848
#endif
#ifndef GL_INDEX_ARRAY_BUFFER_BINDING
#define GL_INDEX_ARRAY_BUFFER_BINDING                                0x00008899
#endif
#ifndef GL_UNSIGNED_INT_SAMPLER_RENDERBUFFER
#define GL_UNSIGNED_INT_SAMPLER_RENDERBUFFER                         0x00008E58
#endif
#ifndef GL_RETAINED
#define GL_RETAINED                                                  0x00008A1B
#endif
#ifndef GL_DRAW_BUFFER0
#define GL_DRAW_BUFFER0                                              0x00008825
#endif
#ifndef GL_VERTEX_ARRAY_RANGE_WITHOUT_FLUSH
#define GL_VERTEX_ARRAY_RANGE_WITHOUT_FLUSH                          0x00008533
#endif
#ifndef GL_EVAL_VERTEX_ATTRIB15
#define GL_EVAL_VERTEX_ATTRIB15                                      0x000086D5
#endif
#ifndef GL_IMAGE_BINDING_LAYERED
#define GL_IMAGE_BINDING_LAYERED                                     0x00008F3C
#endif
#ifndef GL_COLOR_TABLE_FORMAT
#define GL_COLOR_TABLE_FORMAT                                        0x000080D8
#endif
#ifndef GL_SYNC_GPU_COMMANDS_COMPLETE
#define GL_SYNC_GPU_COMMANDS_COMPLETE                                0x00009117
#endif
#ifndef GL_RGB8I
#define GL_RGB8I                                                     0x00008D8F
#endif
#ifndef GL_MODELVIEW2
#define GL_MODELVIEW2                                                0x00008722
#endif
#ifndef GL_SURFACE_STATE
#define GL_SURFACE_STATE                                             0x000086EB
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5                          0x000093D2
#endif
#ifndef GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5                           0x00008C4F
#endif
#ifndef GL_PERFQUERY_COUNTER_NAME_LENGTH_MAX
#define GL_PERFQUERY_COUNTER_NAME_LENGTH_MAX                         0x000094FE
#endif
#ifndef GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT
#define GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT                            0x00008520
#endif
#ifndef GL_NUM_PROGRAM_BINARY_FORMATS
#define GL_NUM_PROGRAM_BINARY_FORMATS                                0x000087FE
#endif
#ifndef GL_MODELVIEW5
#define GL_MODELVIEW5                                                0x00008725
#endif
#ifndef GL_PIXEL_FRAGMENT_RGB_SOURCE
#define GL_PIXEL_FRAGMENT_RGB_SOURCE                                 0x00008354
#endif
#ifndef GL_LIST_PRIORITY
#define GL_LIST_PRIORITY                                             0x00008182
#endif
#ifndef GL_RGB10_A2
#define GL_RGB10_A2                                                  0x00008059
#endif
#ifndef GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_SAMPLES
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_SAMPLES                    0x00008D6C
#endif
#ifndef GL_COUNT_UP
#define GL_COUNT_UP                                                  0x00009088
#endif
#ifndef GL_CONSTANT_COLOR
#define GL_CONSTANT_COLOR                                            0x00008001
#endif
#ifndef GL_LUMINANCE_FLOAT32
#define GL_LUMINANCE_FLOAT32                                         0x00008818
#endif
#ifndef GL_FRAGMENT_SHADER_BIT
#define GL_FRAGMENT_SHADER_BIT                                       0x00000002
#endif
#ifndef GL_MAP1_BINORMAL
#define GL_MAP1_BINORMAL                                             0x00008446
#endif
#ifndef GL_MAD
#define GL_MAD                                                       0x00008968
#endif
#ifndef GL_REG_18
#define GL_REG_18                                                    0x00008933
#endif
#ifndef GL_ARRAY_ELEMENT_LOCK_FIRST
#define GL_ARRAY_ELEMENT_LOCK_FIRST                                  0x000081A8
#endif
#ifndef GL_DUAL_LUMINANCE8
#define GL_DUAL_LUMINANCE8                                           0x00008115
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_5x4x4
#define GL_COMPRESSED_RGBA_ASTC_5x4x4                                0x000093C4
#endif
#ifndef GL_COLOR_ATTACHMENT6
#define GL_COLOR_ATTACHMENT6                                         0x00008CE6
#endif
#ifndef GL_EDGE_FLAG_ARRAY_POINTER
#define GL_EDGE_FLAG_ARRAY_POINTER                                   0x00008093
#endif
#ifndef GL_TEXTURE_COORD_ARRAY_POINTER
#define GL_TEXTURE_COORD_ARRAY_POINTER                               0x00008092
#endif
#ifndef GL_INT_SAMPLER_1D
#define GL_INT_SAMPLER_1D                                            0x00008DC9
#endif
#ifndef GL_EYE_DISTANCE_TO_POINT
#define GL_EYE_DISTANCE_TO_POINT                                     0x000081F0
#endif
#ifndef GL_DUAL_LUMINANCE16
#define GL_DUAL_LUMINANCE16                                          0x00008117
#endif
#ifndef GL_ALPHA32I
#define GL_ALPHA32I                                                  0x00008D84
#endif
#ifndef GL_SPARSE_TEXTURE_FULL_ARRAY_CUBE_MIPMAPS
#define GL_SPARSE_TEXTURE_FULL_ARRAY_CUBE_MIPMAPS                    0x000091A9
#endif
#ifndef GL_VERTEX_ARRAY_ADDRESS
#define GL_VERTEX_ARRAY_ADDRESS                                      0x00008F21
#endif
#ifndef GL_COMPRESSED_SRGB_ALPHA
#define GL_COMPRESSED_SRGB_ALPHA                                     0x00008C49
#endif
#ifndef GL_STENCIL_INDEX4
#define GL_STENCIL_INDEX4                                            0x00008D47
#endif
#ifndef GL_FLOAT_RG16
#define GL_FLOAT_RG16                                                0x00008886
#endif
#ifndef GL_TEXTURE17
#define GL_TEXTURE17                                                 0x000084D1
#endif
#ifndef GL_MAX_OPTIMIZED_VERTEX_SHADER_INSTRUCTIONS
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_INSTRUCTIONS                  0x000087CA
#endif
#ifndef GL_COMPRESSED_RED_RGTC1
#define GL_COMPRESSED_RED_RGTC1                                      0x00008DBB
#endif
#ifndef GL_RGBA_FLOAT32
#define GL_RGBA_FLOAT32                                              0x00008814
#endif
#ifndef GL_INT_SAMPLER_RENDERBUFFER
#define GL_INT_SAMPLER_RENDERBUFFER                                  0x00008E57
#endif
#ifndef GL_SAMPLER_2D_ARRAY
#define GL_SAMPLER_2D_ARRAY                                          0x00008DC1
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC
#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC                          0x00009279
#endif
#ifndef GL_VIDEO_CAPTURE_FRAME_HEIGHT
#define GL_VIDEO_CAPTURE_FRAME_HEIGHT                                0x00009039
#endif
#ifndef GL_DOT_PRODUCT
#define GL_DOT_PRODUCT                                               0x000086EC
#endif
#ifndef GL_DOUBLE_MAT4x2
#define GL_DOUBLE_MAT4x2                                             0x00008F4D
#endif
#ifndef GL_IMAGE_MAG_FILTER
#define GL_IMAGE_MAG_FILTER                                          0x0000815C
#endif
#ifndef GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS
#define GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS                       0x0000880E
#endif
#ifndef GL_MAP2_VERTEX_ATTRIB11_4
#define GL_MAP2_VERTEX_ATTRIB11_4                                    0x0000867B
#endif
#ifndef GL_PROXY_TEXTURE_3D
#define GL_PROXY_TEXTURE_3D                                          0x00008070
#endif
#ifndef GL_OFFSET_HILO_TEXTURE_2D
#define GL_OFFSET_HILO_TEXTURE_2D                                    0x00008854
#endif
#ifndef GL_ADJACENT_PAIRS
#define GL_ADJACENT_PAIRS                                            0x000090AE
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6                          0x000093D4
#endif
#ifndef GL_ATOMIC_COUNTER_BARRIER_BIT
#define GL_ATOMIC_COUNTER_BARRIER_BIT                                0x00001000
#endif
#ifndef GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW
#define GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW                             0x0000900D
#endif
#ifndef GL_PN_TRIANGLES
#define GL_PN_TRIANGLES                                              0x000087F0
#endif
#ifndef GL_LINEAR_DETAIL_ALPHA
#define GL_LINEAR_DETAIL_ALPHA                                       0x00008098
#endif
#ifndef GL_DEBUG_SEVERITY_MEDIUM
#define GL_DEBUG_SEVERITY_MEDIUM                                     0x00009147
#endif
#ifndef GL_FOG_OFFSET_VALUE
#define GL_FOG_OFFSET_VALUE                                          0x00008199
#endif
#ifndef GL_VIBRANCE_BIAS
#define GL_VIBRANCE_BIAS                                             0x00008719
#endif
#ifndef GL_COMBINER3
#define GL_COMBINER3                                                 0x00008553
#endif
#ifndef GL_RELATIVE_QUADRATIC_CURVE_TO
#define GL_RELATIVE_QUADRATIC_CURVE_TO                               0x0000000B
#endif
#ifndef GL_PRIMITIVE_RESTART
#define GL_PRIMITIVE_RESTART                                         0x00008558
#endif
#ifndef GL_OVERLAY
#define GL_OVERLAY                                                   0x00009296
#endif
#ifndef GL_FOG_FUNC_POINTS
#define GL_FOG_FUNC_POINTS                                           0x0000812B
#endif
#ifndef GL_TEXTURE_ALPHA_SIZE
#define GL_TEXTURE_ALPHA_SIZE                                        0x0000805F
#endif
#ifndef GL_FRAGMENT_COLOR_MATERIAL_PARAMETER
#define GL_FRAGMENT_COLOR_MATERIAL_PARAMETER                         0x00008403
#endif
#ifndef GL_PATH_JOIN_STYLE
#define GL_PATH_JOIN_STYLE                                           0x00009079
#endif
#ifndef GL_LOCAL_CONSTANT_DATATYPE
#define GL_LOCAL_CONSTANT_DATATYPE                                   0x000087ED
#endif
#ifndef GL_BACK_SECONDARY_COLOR
#define GL_BACK_SECONDARY_COLOR                                      0x00008C78
#endif
#ifndef GL_CND0
#define GL_CND0                                                      0x0000896B
#endif
#ifndef GL_FIXED
#define GL_FIXED                                                     0x0000140C
#endif
#ifndef GL_Z
#define GL_Z                                                         0x000087D7
#endif
#ifndef GL_REG_5
#define GL_REG_5                                                     0x00008926
#endif
#ifndef GL_PN_TRIANGLES_POINT_MODE_LINEAR
#define GL_PN_TRIANGLES_POINT_MODE_LINEAR                            0x000087F5
#endif
#ifndef GL_MAX_TEXTURE_LOD_BIAS
#define GL_MAX_TEXTURE_LOD_BIAS                                      0x000084FD
#endif
#ifndef GL_DEPTH_STENCIL_TO_BGRA
#define GL_DEPTH_STENCIL_TO_BGRA                                     0x0000886F
#endif
#ifndef GL_CURRENT_QUERY
#define GL_CURRENT_QUERY                                             0x00008865
#endif
#ifndef GL_BOOL_VEC4
#define GL_BOOL_VEC4                                                 0x00008B59
#endif
#ifndef GL_PACK_SUBSAMPLE_RATE
#define GL_PACK_SUBSAMPLE_RATE                                       0x000085A0
#endif
#ifndef GL_ELEMENT_ARRAY
#define GL_ELEMENT_ARRAY                                             0x00008A0C
#endif
#ifndef GL_GEOMETRY_SHADER
#define GL_GEOMETRY_SHADER                                           0x00008DD9
#endif
#ifndef GL_GLYPH_HORIZONTAL_BEARING_X_BIT
#define GL_GLYPH_HORIZONTAL_BEARING_X_BIT                            0x00000004
#endif
#ifndef GL_OBJECT_SHADER_SOURCE_LENGTH
#define GL_OBJECT_SHADER_SOURCE_LENGTH                               0x00008B88
#endif
#ifndef GL_REPLACE
#define GL_REPLACE                                                   0x00008062
#endif
#ifndef GL_MATRIX23
#define GL_MATRIX23                                                  0x000088D7
#endif
#ifndef GL_OP_MOV
#define GL_OP_MOV                                                    0x00008799
#endif
#ifndef GL_NUM_COMPRESSED_TEXTURE_FORMATS
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS                            0x000086A2
#endif
#ifndef GL_TIMEOUT_IGNORED
#define GL_TIMEOUT_IGNORED                                           0xFFFFFFFFFFFFFFFF
#endif
#ifndef GL_QUERY_STENCIL_FAIL_EVENT_BIT
#define GL_QUERY_STENCIL_FAIL_EVENT_BIT                              0x00000004
#endif
#ifndef GL_MATRIX5
#define GL_MATRIX5                                                   0x000088C5
#endif
#ifndef GL_MATERIAL_SIDE_HINT
#define GL_MATERIAL_SIDE_HINT                                        0x0001A22C
#endif
#ifndef GL_COLOR_ARRAY_BUFFER_BINDING
#define GL_COLOR_ARRAY_BUFFER_BINDING                                0x00008898
#endif
#ifndef GL_CONVOLUTION_FILTER_BIAS
#define GL_CONVOLUTION_FILTER_BIAS                                   0x00008015
#endif
#ifndef GL_ACTIVE_PROGRAM
#define GL_ACTIVE_PROGRAM                                            0x00008259
#endif
#ifndef GL_SAMPLER_2D_ARRAY_SHADOW
#define GL_SAMPLER_2D_ARRAY_SHADOW                                   0x00008DC4
#endif
#ifndef GL_TEXTURE_COMPRESSED_IMAGE_SIZE
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE                             0x000086A0
#endif
#ifndef GL_TEXTURE_3D_BINDING
#define GL_TEXTURE_3D_BINDING                                        0x0000806A
#endif
#ifndef GL_REG_12
#define GL_REG_12                                                    0x0000892D
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6                          0x000093D6
#endif
#ifndef GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCAL_CONSTANTS
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCAL_CONSTANTS               0x000087CC
#endif
#ifndef GL_CONVOLUTION_BORDER_COLOR
#define GL_CONVOLUTION_BORDER_COLOR                                  0x00008154
#endif
#ifndef GL_PROGRAM_MATRIX_STACK_DEPTH
#define GL_PROGRAM_MATRIX_STACK_DEPTH                                0x00008E2F
#endif
#ifndef GL_QUERY_COUNTER_BITS
#define GL_QUERY_COUNTER_BITS                                        0x00008864
#endif
#ifndef GL_INVERT_OVG
#define GL_INVERT_OVG                                                0x000092B4
#endif
#ifndef GL_MAX_GEOMETRY_PROGRAM_INVOCATIONS
#define GL_MAX_GEOMETRY_PROGRAM_INVOCATIONS                          0x00008E5A
#endif
#ifndef GL_NORMAL_ARRAY_PARALLEL_POINTERS
#define GL_NORMAL_ARRAY_PARALLEL_POINTERS                            0x000083F6
#endif
#ifndef GL_IMAGE_1D
#define GL_IMAGE_1D                                                  0x0000904C
#endif
#ifndef GL_MODELVIEW27
#define GL_MODELVIEW27                                               0x0000873B
#endif
#ifndef GL_TEXTURE_COORD_ARRAY_LIST_STRIDE
#define GL_TEXTURE_COORD_ARRAY_LIST_STRIDE                           0x000192AC
#endif
#ifndef GL_VERTEX_ARRAY
#define GL_VERTEX_ARRAY                                              0x00008074
#endif
#ifndef GL_HSL_LUMINOSITY
#define GL_HSL_LUMINOSITY                                            0x000092B0
#endif
#ifndef GL_TEXTURE15
#define GL_TEXTURE15                                                 0x000084CF
#endif
#ifndef GL_MATRIX
#define GL_MATRIX                                                    0x000087C0
#endif
#ifndef GL_VARIANT_ARRAY
#define GL_VARIANT_ARRAY                                             0x000087E8
#endif
#ifndef GL_DST
#define GL_DST                                                       0x00009287
#endif
#ifndef GL_EVAL_VERTEX_ATTRIB9
#define GL_EVAL_VERTEX_ATTRIB9                                       0x000086CF
#endif
#ifndef GL_INT_IMAGE_2D_ARRAY
#define GL_INT_IMAGE_2D_ARRAY                                        0x0000905E
#endif
#ifndef GL_MAX_VERTEX_UNIFORM_BLOCKS
#define GL_MAX_VERTEX_UNIFORM_BLOCKS                                 0x00008A2B
#endif
#ifndef GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS
#define GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS                       0x0000880F
#endif
#ifndef GL_FUNC_ADD
#define GL_FUNC_ADD                                                  0x00008006
#endif
#ifndef GL_MAX_GEOMETRY_UNIFORM_COMPONENTS
#define GL_MAX_GEOMETRY_UNIFORM_COMPONENTS                           0x00008DDF
#endif
#ifndef GL_R1UI_N3F_V3F
#define GL_R1UI_N3F_V3F                                              0x000085C7
#endif
#ifndef GL_UNSIGNED_NORMALIZED
#define GL_UNSIGNED_NORMALIZED                                       0x00008C17
#endif
#ifndef GL_DRAW_BUFFER12
#define GL_DRAW_BUFFER12                                             0x00008831
#endif
#ifndef GL_INT_VEC3
#define GL_INT_VEC3                                                  0x00008B54
#endif
#ifndef GL_HISTOGRAM_RED_SIZE
#define GL_HISTOGRAM_RED_SIZE                                        0x00008028
#endif
#ifndef GL_DUAL_ALPHA4
#define GL_DUAL_ALPHA4                                               0x00008110
#endif
#ifndef GL_CON_14
#define GL_CON_14                                                    0x0000894F
#endif
#ifndef GL_TEXTURE_BINDING_EXTERNAL
#define GL_TEXTURE_BINDING_EXTERNAL                                  0x00008D67
#endif
#ifndef GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING                        0x0000889A
#endif
#ifndef GL_PROXY_POST_IMAGE_TRANSFORM_COLOR_TABLE
#define GL_PROXY_POST_IMAGE_TRANSFORM_COLOR_TABLE                    0x00008163
#endif
#ifndef GL_NORMAL_ARRAY_LENGTH
#define GL_NORMAL_ARRAY_LENGTH                                       0x00008F2C
#endif
#ifndef GL_PALETTE8_RGB5_A1
#define GL_PALETTE8_RGB5_A1                                          0x00008B99
#endif
#ifndef GL_FLOAT_RGB
#define GL_FLOAT_RGB                                                 0x00008882
#endif
#ifndef GL_REG_26
#define GL_REG_26                                                    0x0000893B
#endif
#ifndef GL_OP_RECIP
#define GL_OP_RECIP                                                  0x00008794
#endif
#ifndef GL_VERTEX_ID_SWIZZLE
#define GL_VERTEX_ID_SWIZZLE                                         0x000091A5
#endif
#ifndef GL_RENDERBUFFER_HEIGHT
#define GL_RENDERBUFFER_HEIGHT                                       0x00008D43
#endif
#ifndef GL_NUM_FILL_STREAMS
#define GL_NUM_FILL_STREAMS                                          0x00008E29
#endif
#ifndef GL_INVARIANT_VALUE
#define GL_INVARIANT_VALUE                                           0x000087EA
#endif
#ifndef GL_UNIFORM_ARRAY_STRIDE
#define GL_UNIFORM_ARRAY_STRIDE                                      0x00008A3C
#endif
#ifndef GL_MAP1_VERTEX_ATTRIB2_4
#define GL_MAP1_VERTEX_ATTRIB2_4                                     0x00008662
#endif
#ifndef GL_UNKNOWN_CONTEXT_RESET
#define GL_UNKNOWN_CONTEXT_RESET                                     0x00008255
#endif
#ifndef GL_POST_CONVOLUTION_BLUE_SCALE
#define GL_POST_CONVOLUTION_BLUE_SCALE                               0x0000801E
#endif
#ifndef GL_QUERY_DEPTH_BOUNDS_FAIL_EVENT_BIT
#define GL_QUERY_DEPTH_BOUNDS_FAIL_EVENT_BIT                         0x00000008
#endif
#ifndef GL_TESSELLATION_FACTOR
#define GL_TESSELLATION_FACTOR                                       0x00009005
#endif
#ifndef GL_ATTRIB_ARRAY_SIZE
#define GL_ATTRIB_ARRAY_SIZE                                         0x00008623
#endif
#ifndef GL_DUP_FIRST_CUBIC_CURVE_TO
#define GL_DUP_FIRST_CUBIC_CURVE_TO                                  0x000000F2
#endif
#ifndef GL_OP_RECIP_SQRT
#define GL_OP_RECIP_SQRT                                             0x00008795
#endif
#ifndef GL_DEBUG_CATEGORY_SHADER_COMPILER
#define GL_DEBUG_CATEGORY_SHADER_COMPILER                            0x0000914E
#endif
#ifndef GL_SUBTRACT
#define GL_SUBTRACT                                                  0x000084E7
#endif
#ifndef GL_PROGRAM_RESIDENT
#define GL_PROGRAM_RESIDENT                                          0x00008647
#endif
#ifndef GL_MODELVIEW23
#define GL_MODELVIEW23                                               0x00008737
#endif
#ifndef GL_SAMPLER_CUBE_MAP_ARRAY
#define GL_SAMPLER_CUBE_MAP_ARRAY                                    0x0000900C
#endif
#ifndef GL_QUAD_ALPHA8
#define GL_QUAD_ALPHA8                                               0x0000811F
#endif
#ifndef GL_SAMPLER_2D_RECT
#define GL_SAMPLER_2D_RECT                                           0x00008B63
#endif
#ifndef GL_TEXTURE_1D_STACK
#define GL_TEXTURE_1D_STACK                                          0x00008759
#endif
#ifndef GL_YCBCR_422
#define GL_YCBCR_422                                                 0x000085B9
#endif
#ifndef GL_PROXY_COLOR_TABLE
#define GL_PROXY_COLOR_TABLE                                         0x000080D3
#endif
#ifndef GL_NUM_SAMPLE_COUNTS
#define GL_NUM_SAMPLE_COUNTS                                         0x00009380
#endif
#ifndef GL_SECONDARY_COLOR
#define GL_SECONDARY_COLOR                                           0x0000852D
#endif
#ifndef GL_REG_9
#define GL_REG_9                                                     0x0000892A
#endif
#ifndef GL_BLEND_ADVANCED_COHERENT
#define GL_BLEND_ADVANCED_COHERENT                                   0x00009285
#endif
#ifndef GL_COLOR_BUFFER_BIT0
#define GL_COLOR_BUFFER_BIT0                                         0x00000001
#endif
#ifndef GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM
#define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM                          0x00008E8D
#endif
#ifndef GL_MAX_COLOR_ATTACHMENTS
#define GL_MAX_COLOR_ATTACHMENTS                                     0x00008CDF
#endif
#ifndef GL_TRANSFORM_FEEDBACK_BUFFER_SIZE
#define GL_TRANSFORM_FEEDBACK_BUFFER_SIZE                            0x00008C85
#endif
#ifndef GL_OFFSET_TEXTURE_2D_SCALE
#define GL_OFFSET_TEXTURE_2D_SCALE                                   0x000086E2
#endif
#ifndef GL_PATH_STENCIL_REF
#define GL_PATH_STENCIL_REF                                          0x000090B8
#endif
#ifndef GL_TEXTURE_COMPRESSED
#define GL_TEXTURE_COMPRESSED                                        0x000086A1
#endif
#ifndef GL_MODELVIEW14
#define GL_MODELVIEW14                                               0x0000872E
#endif
#ifndef GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP
#define GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP                              0x000086F1
#endif
#ifndef GL_UNSIGNED_INVERT
#define GL_UNSIGNED_INVERT                                           0x00008537
#endif
#ifndef GL_FLOAT_RGB16
#define GL_FLOAT_RGB16                                               0x00008888
#endif
#ifndef GL_QUARTER_BIT
#define GL_QUARTER_BIT                                               0x00000010
#endif
#ifndef GL_HSL_SATURATION
#define GL_HSL_SATURATION                                            0x000092AE
#endif
#ifndef GL_DU8DV8
#define GL_DU8DV8                                                    0x0000877A
#endif
#ifndef GL_RGB_422
#define GL_RGB_422                                                   0x00008A1F
#endif
#ifndef GL_MAX_ARRAY_TEXTURE_LAYERS
#define GL_MAX_ARRAY_TEXTURE_LAYERS                                  0x000088FF
#endif
#ifndef GL_SCALE_BY_ONE_HALF
#define GL_SCALE_BY_ONE_HALF                                         0x00008540
#endif
#ifndef GL_VERTEX_WEIGHT_ARRAY
#define GL_VERTEX_WEIGHT_ARRAY                                       0x0000850C
#endif
#ifndef GL_FRAMEBUFFER_UNSUPPORTED
#define GL_FRAMEBUFFER_UNSUPPORTED                                   0x00008CDD
#endif
#ifndef GL_RG8I
#define GL_RG8I                                                      0x00008237
#endif
#ifndef GL_PIXEL_TILE_WIDTH
#define GL_PIXEL_TILE_WIDTH                                          0x00008140
#endif
#ifndef GL_TEXTURE27
#define GL_TEXTURE27                                                 0x000084DB
#endif
#ifndef GL_RGBA8
#define GL_RGBA8                                                     0x00008058
#endif
#ifndef GL_FRAGMENT_LIGHT3
#define GL_FRAGMENT_LIGHT3                                           0x0000840F
#endif
#ifndef GL_TIMEOUT_EXPIRED
#define GL_TIMEOUT_EXPIRED                                           0x0000911B
#endif
#ifndef GL_TRIANGLE_MESH
#define GL_TRIANGLE_MESH                                             0x00008615
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY14
#define GL_VERTEX_ATTRIB_ARRAY14                                     0x0000865E
#endif
#ifndef GL_CON_11
#define GL_CON_11                                                    0x0000894C
#endif
#ifndef GL_TEXTURE25
#define GL_TEXTURE25                                                 0x000084D9
#endif
#ifndef GL_DSDT
#define GL_DSDT                                                      0x000086F5
#endif
#ifndef GL_POST_CONVOLUTION_ALPHA_SCALE
#define GL_POST_CONVOLUTION_ALPHA_SCALE                              0x0000801F
#endif
#ifndef GL_EDGE_FLAG_ARRAY_ADDRESS
#define GL_EDGE_FLAG_ARRAY_ADDRESS                                   0x00008F26
#endif
#ifndef GL_INT_SAMPLER_2D_RECT
#define GL_INT_SAMPLER_2D_RECT                                       0x00008DCD
#endif
#ifndef GL_ARRAY_OBJECT_BUFFER
#define GL_ARRAY_OBJECT_BUFFER                                       0x00008766
#endif
#ifndef GL_VERTEX_ARRAY_LENGTH
#define GL_VERTEX_ARRAY_LENGTH                                       0x00008F2B
#endif
#ifndef GL_DRAW_BUFFER11
#define GL_DRAW_BUFFER11                                             0x00008830
#endif
#ifndef GL_SHADER_INCLUDE
#define GL_SHADER_INCLUDE                                            0x00008DAE
#endif
#ifndef GL_SKIP_COMPONENTS2
#define GL_SKIP_COMPONENTS2                                          0xFFFFFFFFFFFFFFFB
#endif
#ifndef GL_SECONDARY_COLOR_ARRAY
#define GL_SECONDARY_COLOR_ARRAY                                     0x0000845E
#endif
#ifndef GL_MAX_GENERAL_COMBINERS
#define GL_MAX_GENERAL_COMBINERS                                     0x0000854D
#endif
#ifndef GL_MAX_VERTEX_SHADER_INSTRUCTIONS
#define GL_MAX_VERTEX_SHADER_INSTRUCTIONS                            0x000087C5
#endif
#ifndef GL_LAST_VERTEX_CONVENTION
#define GL_LAST_VERTEX_CONVENTION                                    0x00008E4E
#endif
#ifndef GL_MULTISAMPLE_BUFFER_BIT6
#define GL_MULTISAMPLE_BUFFER_BIT6                                   0x40000000
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD3
#define GL_OUTPUT_TEXTURE_COORD3                                     0x000087A0
#endif
#ifndef GL_UTF8
#define GL_UTF8                                                      0x0000909A
#endif
#ifndef GL_IUI_V2F
#define GL_IUI_V2F                                                   0x000081AD
#endif
#ifndef GL_SPARE1
#define GL_SPARE1                                                    0x0000852F
#endif
#ifndef GL_SAMPLER_2D_RECT_SHADOW
#define GL_SAMPLER_2D_RECT_SHADOW                                    0x00008B64
#endif
#ifndef GL_SOURCE2_RGB
#define GL_SOURCE2_RGB                                               0x00008582
#endif
#ifndef GL_STORAGE_PRIVATE
#define GL_STORAGE_PRIVATE                                           0x000085BD
#endif
#ifndef GL_TEXTURE_TARGET
#define GL_TEXTURE_TARGET                                            0x00008BDA
#endif
#ifndef GL_LUMINANCE_ALPHA16I
#define GL_LUMINANCE_ALPHA16I                                        0x00008D8D
#endif
#ifndef GL_VERTEX_CONSISTENT_HINT
#define GL_VERTEX_CONSISTENT_HINT                                    0x0001A22B
#endif
#ifndef GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS
#define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS                      0x00008DE1
#endif
#ifndef GL_BINORMAL_ARRAY_POINTER
#define GL_BINORMAL_ARRAY_POINTER                                    0x00008443
#endif
#ifndef GL_VERTEX_STREAM0
#define GL_VERTEX_STREAM0                                            0x0000876C
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD12
#define GL_OUTPUT_TEXTURE_COORD12                                    0x000087A9
#endif
#ifndef GL_EVAL_TRIANGULAR_2D
#define GL_EVAL_TRIANGULAR_2D                                        0x000086C1
#endif
#ifndef GL_BLUE_MIN_CLAMP
#define GL_BLUE_MIN_CLAMP                                            0x00008562
#endif
#ifndef GL_RGBA8_SNORM
#define GL_RGBA8_SNORM                                               0x00008F97
#endif
#ifndef GL_COLOR_ATTACHMENT9
#define GL_COLOR_ATTACHMENT9                                         0x00008CE9
#endif
#ifndef GL_CUBIC
#define GL_CUBIC                                                     0x00008334
#endif
#ifndef GL_DUAL_LUMINANCE_ALPHA8
#define GL_DUAL_LUMINANCE_ALPHA8                                     0x0000811D
#endif
#ifndef GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM
#define GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM                  0x00009049
#endif
#ifndef GL_TEXTURE4
#define GL_TEXTURE4                                                  0x000084C4
#endif
#ifndef GL_MAT_EMISSION_BIT
#define GL_MAT_EMISSION_BIT                                          0x00800000
#endif
#ifndef GL_DEPENDENT_HILO_TEXTURE_2D
#define GL_DEPENDENT_HILO_TEXTURE_2D                                 0x00008858
#endif
#ifndef GL_TEXTURE_BORDER_VALUES
#define GL_TEXTURE_BORDER_VALUES                                     0x0000871A
#endif
#ifndef GL_PATH_FOG_GEN_MODE
#define GL_PATH_FOG_GEN_MODE                                         0x000090AC
#endif
#ifndef GL_INTENSITY8UI
#define GL_INTENSITY8UI                                              0x00008D7F
#endif
#ifndef GL_VERTEX_DATA_HINT
#define GL_VERTEX_DATA_HINT                                          0x0001A22A
#endif
#ifndef GL_ASYNC_MARKER
#define GL_ASYNC_MARKER                                              0x00008329
#endif
#ifndef GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS
#define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS                          0x00008C29
#endif
#ifndef GL_PARAMETER_BUFFER_BINDING
#define GL_PARAMETER_BUFFER_BINDING                                  0x000080EF
#endif
#ifndef GL_PIXEL_TRANSFORM_2D_STACK_DEPTH
#define GL_PIXEL_TRANSFORM_2D_STACK_DEPTH                            0x00008336
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY2
#define GL_VERTEX_ATTRIB_ARRAY2                                      0x00008652
#endif
#ifndef GL_TESS_CONTROL_PROGRAM
#define GL_TESS_CONTROL_PROGRAM                                      0x0000891E
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD26
#define GL_OUTPUT_TEXTURE_COORD26                                    0x000087B7
#endif
#ifndef GL_SPRITE_AXIS
#define GL_SPRITE_AXIS                                               0x0000814A
#endif
#ifndef GL_WEIGHT_ARRAY_STRIDE
#define GL_WEIGHT_ARRAY_STRIDE                                       0x000086AA
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_10x6
#define GL_COMPRESSED_RGBA_ASTC_10x6                                 0x000093B9
#endif
#ifndef GL_IMPLEMENTATION_COLOR_READ_FORMAT
#define GL_IMPLEMENTATION_COLOR_READ_FORMAT                          0x00008B9B
#endif
#ifndef GL_RELATIVE_LARGE_CW_ARC_TO
#define GL_RELATIVE_LARGE_CW_ARC_TO                                  0x00000019
#endif
#ifndef GL_BGRA
#define GL_BGRA                                                      0x000080E1
#endif
#ifndef GL_DEBUG_TYPE_PORTABILITY
#define GL_DEBUG_TYPE_PORTABILITY                                    0x0000824F
#endif
#ifndef GL_REFERENCE_PLANE
#define GL_REFERENCE_PLANE                                           0x0000817D
#endif
#ifndef GL_COMPRESSED_LUMINANCE_LATC1
#define GL_COMPRESSED_LUMINANCE_LATC1                                0x00008C70
#endif
#ifndef GL_COLOR_ATTACHMENT2
#define GL_COLOR_ATTACHMENT2                                         0x00008CE2
#endif
#ifndef GL_NORMAL_ARRAY_LIST_STRIDE
#define GL_NORMAL_ARRAY_LIST_STRIDE                                  0x000192A9
#endif
#ifndef GL_INDEX_MATERIAL_PARAMETER
#define GL_INDEX_MATERIAL_PARAMETER                                  0x000081B9
#endif
#ifndef GL_REG_28
#define GL_REG_28                                                    0x0000893D
#endif
#ifndef GL_BLUE
#define GL_BLUE                                                      0x00001905
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_5x5x4
#define GL_COMPRESSED_RGBA_ASTC_5x5x4                                0x000093C5
#endif
#ifndef GL_LUMINANCE4_ALPHA4
#define GL_LUMINANCE4_ALPHA4                                         0x00008043
#endif
#ifndef GL_MAX_PROGRAM_INSTRUCTIONS
#define GL_MAX_PROGRAM_INSTRUCTIONS                                  0x000088A1
#endif
#ifndef GL_PROGRAM_STRING
#define GL_PROGRAM_STRING                                            0x00008628
#endif
#ifndef GL_VERTEX_SHADER_OPTIMIZED
#define GL_VERTEX_SHADER_OPTIMIZED                                   0x000087D4
#endif
#ifndef GL_PIXEL_PACK_BUFFER_BINDING
#define GL_PIXEL_PACK_BUFFER_BINDING                                 0x000088ED
#endif
#ifndef GL_LO_BIAS
#define GL_LO_BIAS                                                   0x00008715
#endif
#ifndef GL_VERTEX_ATTRIB_MAP1_DOMAIN
#define GL_VERTEX_ATTRIB_MAP1_DOMAIN                                 0x00008A05
#endif
#ifndef GL_DRAW_BUFFER4
#define GL_DRAW_BUFFER4                                              0x00008829
#endif
#ifndef GL_SECONDARY_COLOR_ARRAY_SIZE
#define GL_SECONDARY_COLOR_ARRAY_SIZE                                0x0000845A
#endif
#ifndef GL_DRAW_FRAMEBUFFER
#define GL_DRAW_FRAMEBUFFER                                          0x00008CA9
#endif
#ifndef GL_OPERAND2_RGB
#define GL_OPERAND2_RGB                                              0x00008592
#endif
#ifndef GL_TRANSPOSE_CURRENT_MATRIX
#define GL_TRANSPOSE_CURRENT_MATRIX                                  0x000088B7
#endif
#ifndef GL_UNIFORM_OFFSET
#define GL_UNIFORM_OFFSET                                            0x00008A3B
#endif
#ifndef GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE                        0x00008CD0
#endif
#ifndef GL_IR_INSTRUMENT1
#define GL_IR_INSTRUMENT1                                            0x0000817F
#endif
#ifndef GL_ELEMENT_ARRAY_ADDRESS
#define GL_ELEMENT_ARRAY_ADDRESS                                     0x00008F29
#endif
#ifndef GL_CON_5
#define GL_CON_5                                                     0x00008946
#endif
#ifndef GL_MODELVIEW1
#define GL_MODELVIEW1                                                0x0000850A
#endif
#ifndef GL_TESS_EVALUATION_PROGRAM_PARAMETER_BUFFER
#define GL_TESS_EVALUATION_PROGRAM_PARAMETER_BUFFER                  0x00008C75
#endif
#ifndef GL_TEXTURE_SPARSE
#define GL_TEXTURE_SPARSE                                            0x000091A6
#endif
#ifndef GL_FRAGMENT_LIGHT_MODEL_LOCAL_VIEWER
#define GL_FRAGMENT_LIGHT_MODEL_LOCAL_VIEWER                         0x00008408
#endif
#ifndef GL_FRAMEZOOM_FACTOR
#define GL_FRAMEZOOM_FACTOR                                          0x0000818C
#endif
#ifndef GL_COMBINER6
#define GL_COMBINER6                                                 0x00008556
#endif
#ifndef GL_FRAME
#define GL_FRAME                                                     0x00008E26
#endif
#ifndef GL_Z4Y12Z4CB12Z4CR12_444
#define GL_Z4Y12Z4CB12Z4CR12_444                                     0x00009037
#endif
#ifndef GL_COLOR_TABLE_INTENSITY_SIZE
#define GL_COLOR_TABLE_INTENSITY_SIZE                                0x000080DF
#endif
#ifndef GL_DEBUG_SEVERITY_NOTIFICATION
#define GL_DEBUG_SEVERITY_NOTIFICATION                               0x0000826B
#endif
#ifndef GL_VARIANT
#define GL_VARIANT                                                   0x000087C1
#endif
#ifndef GL_TEXTURE_CUBE_MAP_POSITIVE_Z
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z                               0x00008519
#endif
#ifndef GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1                           0x00008C4D
#endif
#ifndef GL_OBJECT_DISTANCE_TO_LINE
#define GL_OBJECT_DISTANCE_TO_LINE                                   0x000081F3
#endif
#ifndef GL_MAX_PROGRAM_TEMPORARIES
#define GL_MAX_PROGRAM_TEMPORARIES                                   0x000088A5
#endif
#ifndef GL_OFFSET_TEXTURE_SCALE
#define GL_OFFSET_TEXTURE_SCALE                                      0x000086E2
#endif
#ifndef GL_INT_SAMPLER_1D_ARRAY
#define GL_INT_SAMPLER_1D_ARRAY                                      0x00008DCE
#endif
#ifndef GL_COLOR_ARRAY_LIST_STRIDE
#define GL_COLOR_ARRAY_LIST_STRIDE                                   0x000192AA
#endif
#ifndef GL_TEXTURE_4DSIZE
#define GL_TEXTURE_4DSIZE                                            0x00008136
#endif
#ifndef GL_COLOR_BUFFER_BIT4
#define GL_COLOR_BUFFER_BIT4                                         0x00000010
#endif
#ifndef GL_FENCE_STATUS
#define GL_FENCE_STATUS                                              0x000084F3
#endif
#ifndef GL_4PASS_0
#define GL_4PASS_0                                                   0x000080A4
#endif
#ifndef GL_VERTEX_ATTRIB_MAP2_DOMAIN
#define GL_VERTEX_ATTRIB_MAP2_DOMAIN                                 0x00008A09
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_3x3x3
#define GL_COMPRESSED_RGBA_ASTC_3x3x3                                0x000093C0
#endif
#ifndef GL_POINT_SIZE_MAX
#define GL_POINT_SIZE_MAX                                            0x00008127
#endif
#ifndef GL_MAX_MAP_TESSELLATION
#define GL_MAX_MAP_TESSELLATION                                      0x000086D6
#endif
#ifndef GL_PROGRAM_BINARY_LENGTH
#define GL_PROGRAM_BINARY_LENGTH                                     0x00008741
#endif
#ifndef GL_MAX_BINDABLE_UNIFORM_SIZE
#define GL_MAX_BINDABLE_UNIFORM_SIZE                                 0x00008DED
#endif
#ifndef GL_FULL_STIPPLE_HINT
#define GL_FULL_STIPPLE_HINT                                         0x0001A219
#endif
#ifndef GL_DYNAMIC_COPY
#define GL_DYNAMIC_COPY                                              0x000088EA
#endif
#ifndef GL_FRAGMENT_COLOR
#define GL_FRAGMENT_COLOR                                            0x0000834C
#endif
#ifndef GL_MODELVIEW8
#define GL_MODELVIEW8                                                0x00008728
#endif
#ifndef GL_OP_MIN
#define GL_OP_MIN                                                    0x0000878B
#endif
#ifndef GL_LINEARLIGHT
#define GL_LINEARLIGHT                                               0x000092A7
#endif
#ifndef GL_POST_IMAGE_TRANSFORM_COLOR_TABLE
#define GL_POST_IMAGE_TRANSFORM_COLOR_TABLE                          0x00008162
#endif
#ifndef GL_COMPRESSED_TEXTURE_FORMATS
#define GL_COMPRESSED_TEXTURE_FORMATS                                0x000086A3
#endif
#ifndef GL_INVARIANT_DATATYPE
#define GL_INVARIANT_DATATYPE                                        0x000087EB
#endif
#ifndef GL_SMALL_CCW_ARC_TO
#define GL_SMALL_CCW_ARC_TO                                          0x00000012
#endif
#ifndef GL_RENDERBUFFER_WIDTH
#define GL_RENDERBUFFER_WIDTH                                        0x00008D42
#endif
#ifndef GL_PALETTE4_RGB5_A1
#define GL_PALETTE4_RGB5_A1                                          0x00008B94
#endif
#ifndef GL_4X_BIT
#define GL_4X_BIT                                                    0x00000002
#endif
#ifndef GL_ETC1_RGB8
#define GL_ETC1_RGB8                                                 0x00008D64
#endif
#ifndef GL_CLAMP_TO_BORDER
#define GL_CLAMP_TO_BORDER                                           0x0000812D
#endif
#ifndef GL_COLOR_TABLE_SCALE
#define GL_COLOR_TABLE_SCALE                                         0x000080D6
#endif
#ifndef GL_EVAL_VERTEX_ATTRIB5
#define GL_EVAL_VERTEX_ATTRIB5                                       0x000086CB
#endif
#ifndef GL_TEXTURE_COORD
#define GL_TEXTURE_COORD                                             0x00008C79
#endif
#ifndef GL_RGB8_SNORM
#define GL_RGB8_SNORM                                                0x00008F96
#endif
#ifndef GL_MULTISAMPLE_BIT
#define GL_MULTISAMPLE_BIT                                           0x20000000
#endif
#ifndef GL_SAMPLE_MASK_INVERT
#define GL_SAMPLE_MASK_INVERT                                        0x000080AB
#endif
#ifndef GL_CON_24
#define GL_CON_24                                                    0x00008959
#endif
#ifndef GL_REG_2
#define GL_REG_2                                                     0x00008923
#endif
#ifndef GL_OBJECT_ACTIVE_UNIFORMS
#define GL_OBJECT_ACTIVE_UNIFORMS                                    0x00008B86
#endif
#ifndef GL_SIGNED_ALPHA8
#define GL_SIGNED_ALPHA8                                             0x00008706
#endif
#ifndef GL_422_AVERAGE
#define GL_422_AVERAGE                                               0x000080CE
#endif
#ifndef GL_TEXTURE_FILTER4_SIZE
#define GL_TEXTURE_FILTER4_SIZE                                      0x00008147
#endif
#ifndef GL_UNSIGNED_BYTE_3_3_2
#define GL_UNSIGNED_BYTE_3_3_2                                       0x00008032
#endif
#ifndef GL_SAMPLES
#define GL_SAMPLES                                                   0x000080A9
#endif
#ifndef GL_PROGRAM_TEMPORARIES
#define GL_PROGRAM_TEMPORARIES                                       0x000088A4
#endif
#ifndef GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS
#define GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS                  0x00008A33
#endif
#ifndef GL_NEGATIVE_ONE
#define GL_NEGATIVE_ONE                                              0x000087DF
#endif
#ifndef GL_ALPHA8
#define GL_ALPHA8                                                    0x0000803C
#endif
#ifndef GL_NORMAL_ARRAY_COUNT
#define GL_NORMAL_ARRAY_COUNT                                        0x00008080
#endif
#ifndef GL_MIN_FRAGMENT_INTERPOLATION_OFFSET
#define GL_MIN_FRAGMENT_INTERPOLATION_OFFSET                         0x00008E5B
#endif
#ifndef GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE              0x00008CD3
#endif
#ifndef GL_BIAS_BY_NEGATIVE_ONE_HALF
#define GL_BIAS_BY_NEGATIVE_ONE_HALF                                 0x00008541
#endif
#ifndef GL_UNSIGNED_INT8_VEC4
#define GL_UNSIGNED_INT8_VEC4                                        0x00008FEF
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT
#define GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT                           0x00000001
#endif
#ifndef GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING
#define GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING                         0x000086D9
#endif
#ifndef GL_INT_SAMPLER_2D_ARRAY
#define GL_INT_SAMPLER_2D_ARRAY                                      0x00008DCF
#endif
#ifndef GL_STENCIL_BUFFER_BIT1
#define GL_STENCIL_BUFFER_BIT1                                       0x00020000
#endif
#ifndef GL_UNSIGNED_INT_SAMPLER_BUFFER
#define GL_UNSIGNED_INT_SAMPLER_BUFFER                               0x00009003
#endif
#ifndef GL_PROGRAM_ERROR_STRING
#define GL_PROGRAM_ERROR_STRING                                      0x00008874
#endif
#ifndef GL_OBJECT_POINT
#define GL_OBJECT_POINT                                              0x000081F5
#endif
#ifndef GL_UNSIGNED_INT_IMAGE_2D_ARRAY
#define GL_UNSIGNED_INT_IMAGE_2D_ARRAY                               0x00009069
#endif
#ifndef GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY
#define GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY                            0x00009061
#endif
#ifndef GL_RELATIVE_MOVE_TO
#define GL_RELATIVE_MOVE_TO                                          0x00000003
#endif
#ifndef GL_READ_PIXEL_DATA_RANGE
#define GL_READ_PIXEL_DATA_RANGE                                     0x00008879
#endif
#ifndef GL_UNSIGNED_INT_SAMPLER_2D
#define GL_UNSIGNED_INT_SAMPLER_2D                                   0x00008DD2
#endif
#ifndef GL_MAJOR_VERSION
#define GL_MAJOR_VERSION                                             0x0000821B
#endif
#ifndef GL_PACK_SKIP_PIXELS
#define GL_PACK_SKIP_PIXELS                                          0x00000D04
#endif
#ifndef GL_TRANSPOSE
#define GL_TRANSPOSE                                                 0x0000862C
#endif
#ifndef GL_SRC_ATOP
#define GL_SRC_ATOP                                                  0x0000928E
#endif
#ifndef GL_PIXEL_UNPACK_BUFFER
#define GL_PIXEL_UNPACK_BUFFER                                       0x000088EC
#endif
#ifndef GL_RGB_FLOAT32
#define GL_RGB_FLOAT32                                               0x00008815
#endif
#ifndef GL_SGX_BINARY
#define GL_SGX_BINARY                                                0x00008C0A
#endif
#ifndef GL_BUMP_NUM_TEX_UNITS
#define GL_BUMP_NUM_TEX_UNITS                                        0x00008777
#endif
#ifndef GL_NATIVE_GRAPHICS_BEGIN_HINT
#define GL_NATIVE_GRAPHICS_BEGIN_HINT                                0x0001A203
#endif
#ifndef GL_PACK_IMAGE_HEIGHT
#define GL_PACK_IMAGE_HEIGHT                                         0x0000806C
#endif
#ifndef GL_BLEND_EQUATION
#define GL_BLEND_EQUATION                                            0x00008009
#endif
#ifndef GL_COLOR_ATTACHMENT12
#define GL_COLOR_ATTACHMENT12                                        0x00008CEC
#endif
#ifndef GL_OPERAND2_ALPHA
#define GL_OPERAND2_ALPHA                                            0x0000859A
#endif
#ifndef GL_MATRIX2
#define GL_MATRIX2                                                   0x000088C2
#endif
#ifndef GL_LUMINANCE32I
#define GL_LUMINANCE32I                                              0x00008D86
#endif
#ifndef GL_FOG_COORDINATE_ARRAY_POINTER
#define GL_FOG_COORDINATE_ARRAY_POINTER                              0x00008456
#endif
#ifndef GL_COPY_READ_BUFFER
#define GL_COPY_READ_BUFFER                                          0x00008F36
#endif
#ifndef GL_Z6Y10Z6CB10Z6A10Z6Y10Z6CR10Z6A10_4224
#define GL_Z6Y10Z6CB10Z6A10Z6Y10Z6CR10Z6A10_4224                     0x00009034
#endif
#ifndef GL_HILO16
#define GL_HILO16                                                    0x000086F8
#endif
#ifndef GL_CURRENT_PALETTE_MATRIX
#define GL_CURRENT_PALETTE_MATRIX                                    0x00008843
#endif
#ifndef GL_PREVIOUS
#define GL_PREVIOUS                                                  0x00008578
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6                         0x000093D9
#endif
#ifndef GL_ELEMENT_ARRAY_BUFFER
#define GL_ELEMENT_ARRAY_BUFFER                                      0x00008893
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY10
#define GL_VERTEX_ATTRIB_ARRAY10                                     0x0000865A
#endif
#ifndef GL_HSL_HUE
#define GL_HSL_HUE                                                   0x000092AD
#endif
#ifndef GL_BUMP_TEX_UNITS
#define GL_BUMP_TEX_UNITS                                            0x00008778
#endif
#ifndef GL_CURRENT_OCCLUSION_QUERY_ID
#define GL_CURRENT_OCCLUSION_QUERY_ID                                0x00008865
#endif
#ifndef GL_VERTEX_STREAM4
#define GL_VERTEX_STREAM4                                            0x00008770
#endif
#ifndef GL_INT16_VEC2
#define GL_INT16_VEC2                                                0x00008FE5
#endif
#ifndef GL_REG_13
#define GL_REG_13                                                    0x0000892E
#endif
#ifndef GL_FRAMEBUFFER_SRGB_CAPABLE
#define GL_FRAMEBUFFER_SRGB_CAPABLE                                  0x00008DBA
#endif
#ifndef GL_COMPRESSED_SLUMINANCE
#define GL_COMPRESSED_SLUMINANCE                                     0x00008C4A
#endif
#ifndef GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_SCALE
#define GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_SCALE                 0x00008853
#endif
#ifndef GL_PASS_THROUGH
#define GL_PASS_THROUGH                                              0x000086E6
#endif
#ifndef GL_FOG_OFFSET
#define GL_FOG_OFFSET                                                0x00008198
#endif
#ifndef GL_MAX_ELEMENTS_VERTICES
#define GL_MAX_ELEMENTS_VERTICES                                     0x000080E8
#endif
#ifndef GL_FACTOR_MIN
#define GL_FACTOR_MIN                                                0x0000901C
#endif
#ifndef GL_DEBUG_CATEGORY_OTHER
#define GL_DEBUG_CATEGORY_OTHER                                      0x00009150
#endif
#ifndef GL_TEXTURE_FORMAT
#define GL_TEXTURE_FORMAT                                            0x00008BD6
#endif
#ifndef GL_FLOAT_MAT3x2
#define GL_FLOAT_MAT3x2                                              0x00008B67
#endif
#ifndef GL_OP_ROUND
#define GL_OP_ROUND                                                  0x00008790
#endif
#ifndef GL_MATRIX22
#define GL_MATRIX22                                                  0x000088D6
#endif
#ifndef GL_SCALEBIAS_HINT
#define GL_SCALEBIAS_HINT                                            0x00008322
#endif
#ifndef GL_FORMAT_SUBSAMPLE_24_24
#define GL_FORMAT_SUBSAMPLE_24_24                                    0x00008982
#endif
#ifndef GL_VERTEX_ATTRIB_MAP1_SIZE
#define GL_VERTEX_ATTRIB_MAP1_SIZE                                   0x00008A02
#endif
#ifndef GL_MAX_RENDERBUFFER_SIZE
#define GL_MAX_RENDERBUFFER_SIZE                                     0x000084E8
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_6x5
#define GL_COMPRESSED_RGBA_ASTC_6x5                                  0x000093B3
#endif
#ifndef GL_EDGEFLAG_BIT
#define GL_EDGEFLAG_BIT                                              0x00040000
#endif
#ifndef GL_UNSIGNED_INT_IMAGE_2D
#define GL_UNSIGNED_INT_IMAGE_2D                                     0x00009063
#endif
#ifndef GL_TEXTURE_CLIPMAP_CENTER
#define GL_TEXTURE_CLIPMAP_CENTER                                    0x00008171
#endif
#ifndef GL_CURRENT_TIME
#define GL_CURRENT_TIME                                              0x00008E28
#endif
#ifndef GL_UNSIGNED_INT_IMAGE_1D_ARRAY
#define GL_UNSIGNED_INT_IMAGE_1D_ARRAY                               0x00009068
#endif
#ifndef GL_PN_TRIANGLES_POINT_MODE_CUBIC
#define GL_PN_TRIANGLES_POINT_MODE_CUBIC                             0x000087F6
#endif
#ifndef GL_SAMPLE_COVERAGE
#define GL_SAMPLE_COVERAGE                                           0x000080A0
#endif
#ifndef GL_INT_10_10_10_2
#define GL_INT_10_10_10_2                                            0x00008DF7
#endif
#ifndef GL_COMPRESSED_RGB_PVRTC_2BPPV1
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1                               0x00008C01
#endif
#ifndef GL_WIDE_LINE_HINT
#define GL_WIDE_LINE_HINT                                            0x0001A222
#endif
#ifndef GL_SAMPLER_2D_SHADOW
#define GL_SAMPLER_2D_SHADOW                                         0x00008B62
#endif
#ifndef GL_FLOAT16
#define GL_FLOAT16                                                   0x00008FF8
#endif
#ifndef GL_FLOAT_RG32
#define GL_FLOAT_RG32                                                0x00008887
#endif
#ifndef GL_IMAGE_MIN_FILTER
#define GL_IMAGE_MIN_FILTER                                          0x0000815D
#endif
#ifndef GL_CPU_OPTIMIZED
#define GL_CPU_OPTIMIZED                                             0x00008FB1
#endif
#ifndef GL_MIN
#define GL_MIN                                                       0x00008007
#endif
#ifndef GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER
#define GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER               0x00008A46
#endif
#ifndef GL_MAP1_VERTEX_ATTRIB15_4
#define GL_MAP1_VERTEX_ATTRIB15_4                                    0x0000866F
#endif
#ifndef GL_PATH_COORD_COUNT
#define GL_PATH_COORD_COUNT                                          0x0000909E
#endif
#ifndef GL_PATH_STROKE_MASK
#define GL_PATH_STROKE_MASK                                          0x00009084
#endif
#ifndef GL_HISTOGRAM_GREEN_SIZE
#define GL_HISTOGRAM_GREEN_SIZE                                      0x00008029
#endif
#ifndef GL_NUM_LOOPBACK_COMPONENTS
#define GL_NUM_LOOPBACK_COMPONENTS                                   0x00008974
#endif
#ifndef GL_RGB2
#define GL_RGB2                                                      0x0000804E
#endif
#ifndef GL_SIGNALED
#define GL_SIGNALED                                                  0x00009119
#endif
#ifndef GL_MIN_SAMPLE_SHADING_VALUE
#define GL_MIN_SAMPLE_SHADING_VALUE                                  0x00008C37
#endif
#ifndef GL_COMPRESSED_SRGB_PVRTC_2BPPV1
#define GL_COMPRESSED_SRGB_PVRTC_2BPPV1                              0x00008A54
#endif
#ifndef GL_ARRAY_BUFFER_BINDING
#define GL_ARRAY_BUFFER_BINDING                                      0x00008894
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY9
#define GL_VERTEX_ATTRIB_ARRAY9                                      0x00008659
#endif
#ifndef GL_RELATIVE_SMOOTH_CUBIC_CURVE_TO
#define GL_RELATIVE_SMOOTH_CUBIC_CURVE_TO                            0x00000011
#endif
#ifndef GL_TEXTURE_STENCIL_SIZE
#define GL_TEXTURE_STENCIL_SIZE                                      0x000088F1
#endif
#ifndef GL_DOT3
#define GL_DOT3                                                      0x00008966
#endif
#ifndef GL_ALLOW_DRAW_OBJ_HINT
#define GL_ALLOW_DRAW_OBJ_HINT                                       0x0001A20E
#endif
#ifndef GL_SIGNED_INTENSITY
#define GL_SIGNED_INTENSITY                                          0x00008707
#endif
#ifndef GL_ALPHA_TEST_REF
#define GL_ALPHA_TEST_REF                                            0x00000BC2
#endif
#ifndef GL_BUFFER_ACCESS
#define GL_BUFFER_ACCESS                                             0x000088BB
#endif
#ifndef GL_INT_IMAGE_1D
#define GL_INT_IMAGE_1D                                              0x00009057
#endif
#ifndef GL_ACTIVE_VARYING_MAX_LENGTH
#define GL_ACTIVE_VARYING_MAX_LENGTH                                 0x00008C82
#endif
#ifndef GL_UNSIGNED_INT_VEC2
#define GL_UNSIGNED_INT_VEC2                                         0x00008DC6
#endif
#ifndef GL_IMAGE_BINDING_LEVEL
#define GL_IMAGE_BINDING_LEVEL                                       0x00008F3B
#endif
#ifndef GL_VARIABLE_F
#define GL_VARIABLE_F                                                0x00008528
#endif
#ifndef GL_RGB16F
#define GL_RGB16F                                                    0x0000881B
#endif
#ifndef GL_COMPRESSED_SRGB
#define GL_COMPRESSED_SRGB                                           0x00008C48
#endif
#ifndef GL_TEXTURE_NUM_LEVELS
#define GL_TEXTURE_NUM_LEVELS                                        0x00008BD9
#endif
#ifndef GL_TEXTURE_MAX_CLAMP_R
#define GL_TEXTURE_MAX_CLAMP_R                                       0x0000836B
#endif
#ifndef GL_IMAGE_TRANSFORM_2D
#define GL_IMAGE_TRANSFORM_2D                                        0x00008161
#endif
#ifndef GL_LERP
#define GL_LERP                                                      0x00008969
#endif
#ifndef GL_SHADER_GLOBAL_ACCESS_BARRIER_BIT
#define GL_SHADER_GLOBAL_ACCESS_BARRIER_BIT                          0x00000010
#endif
#ifndef GL_BUFFER_OBJECT
#define GL_BUFFER_OBJECT                                             0x00009151
#endif
#ifndef GL_ALPHA16
#define GL_ALPHA16                                                   0x0000803E
#endif
#ifndef GL_COMPRESSED_SRGB8_ETC2
#define GL_COMPRESSED_SRGB8_ETC2                                     0x00009275
#endif
#ifndef GL_COLOR_ARRAY_COUNT
#define GL_COLOR_ARRAY_COUNT                                         0x00008084
#endif
#ifndef GL_INT_IMAGE_2D
#define GL_INT_IMAGE_2D                                              0x00009058
#endif
#ifndef GL_INT8_VEC4
#define GL_INT8_VEC4                                                 0x00008FE3
#endif
#ifndef GL_PIXEL_GROUP_COLOR
#define GL_PIXEL_GROUP_COLOR                                         0x00008356
#endif
#ifndef GL_CURRENT_SECONDARY_COLOR
#define GL_CURRENT_SECONDARY_COLOR                                   0x00008459
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_5x4
#define GL_COMPRESSED_RGBA_ASTC_5x4                                  0x000093B1
#endif
#ifndef GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE
#define GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE                         0x0000906B
#endif
#ifndef GL_TEXTURE11
#define GL_TEXTURE11                                                 0x000084CB
#endif
#ifndef GL_OFFSET_HILO_TEXTURE_RECTANGLE
#define GL_OFFSET_HILO_TEXTURE_RECTANGLE                             0x00008855
#endif
#ifndef GL_QUERY_WAIT
#define GL_QUERY_WAIT                                                0x00008E13
#endif
#ifndef GL_PIXEL_TRANSFORM_2D_MATRIX
#define GL_PIXEL_TRANSFORM_2D_MATRIX                                 0x00008338
#endif
#ifndef GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT                 0x00008CD7
#endif
#ifndef GL_STENCIL_BUFFER_BIT4
#define GL_STENCIL_BUFFER_BIT4                                       0x00100000
#endif
#ifndef GL_4PASS_1
#define GL_4PASS_1                                                   0x000080A5
#endif
#ifndef GL_TEXTURE_CUBE_MAP_NEGATIVE_X
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X                               0x00008516
#endif
#ifndef GL_FRAGMENT_COLOR_MATERIAL_FACE
#define GL_FRAGMENT_COLOR_MATERIAL_FACE                              0x00008402
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_6x6x6
#define GL_COMPRESSED_RGBA_ASTC_6x6x6                                0x000093C9
#endif
#ifndef GL_DEBUG_SOURCE_APPLICATION
#define GL_DEBUG_SOURCE_APPLICATION                                  0x0000824A
#endif
#ifndef GL_PERFQUERY_COUNTER_DATA_UINT64
#define GL_PERFQUERY_COUNTER_DATA_UINT64                             0x000094F9
#endif
#ifndef GL_IMAGE_BINDING_NAME
#define GL_IMAGE_BINDING_NAME                                        0x00008F3A
#endif
#ifndef GL_DEPTH_STENCIL_TO_RGBA
#define GL_DEPTH_STENCIL_TO_RGBA                                     0x0000886E
#endif
#ifndef GL_FONT_HAS_KERNING_BIT
#define GL_FONT_HAS_KERNING_BIT                                      0x10000000
#endif
#ifndef GL_UNSIGNED_INT_VEC3
#define GL_UNSIGNED_INT_VEC3                                         0x00008DC7
#endif
#ifndef GL_MAX_VERTEX_HINT
#define GL_MAX_VERTEX_HINT                                           0x0001A22D
#endif
#ifndef GL_COLOR_TABLE_BLUE_SIZE
#define GL_COLOR_TABLE_BLUE_SIZE                                     0x000080DC
#endif
#ifndef GL_GEOMETRY_PROGRAM_PARAMETER_BUFFER
#define GL_GEOMETRY_PROGRAM_PARAMETER_BUFFER                         0x00008DA3
#endif
#ifndef GL_VIBRANCE_SCALE
#define GL_VIBRANCE_SCALE                                            0x00008713
#endif
#ifndef GL_MOVE_TO
#define GL_MOVE_TO                                                   0x00000002
#endif
#ifndef GL_422_REV
#define GL_422_REV                                                   0x000080CD
#endif
#ifndef GL_REPLICATE_BORDER
#define GL_REPLICATE_BORDER                                          0x00008153
#endif
#ifndef GL_PALETTE8_R5_G6_B5
#define GL_PALETTE8_R5_G6_B5                                         0x00008B97
#endif
#ifndef GL_RGBA16F
#define GL_RGBA16F                                                   0x0000881A
#endif
#ifndef GL_POST_CONVOLUTION_ALPHA_BIAS
#define GL_POST_CONVOLUTION_ALPHA_BIAS                               0x00008023
#endif
#ifndef GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR                            0x0000824D
#endif
#ifndef GL_TEXTURE_BORDER_COLOR
#define GL_TEXTURE_BORDER_COLOR                                      0x00001004
#endif
#ifndef GL_REG_0
#define GL_REG_0                                                     0x00008921
#endif
#ifndef GL_UNPACK_CMYK_HINT
#define GL_UNPACK_CMYK_HINT                                          0x0000800F
#endif
#ifndef GL_SKIP_MISSING_GLYPH
#define GL_SKIP_MISSING_GLYPH                                        0x000090A9
#endif
#ifndef GL_YCRCB
#define GL_YCRCB                                                     0x00008318
#endif
#ifndef GL_RED_MIN_CLAMP
#define GL_RED_MIN_CLAMP                                             0x00008560
#endif
#ifndef GL_PROGRAM_UNDER_NATIVE_LIMITS
#define GL_PROGRAM_UNDER_NATIVE_LIMITS                               0x000088B6
#endif
#ifndef GL_REPLACEMENT_CODE_ARRAY_STRIDE
#define GL_REPLACEMENT_CODE_ARRAY_STRIDE                             0x000085C2
#endif
#ifndef GL_MAP1_VERTEX_ATTRIB11_4
#define GL_MAP1_VERTEX_ATTRIB11_4                                    0x0000866B
#endif
#ifndef GL_PROGRAM_OBJECT
#define GL_PROGRAM_OBJECT                                            0x00008B40
#endif
#ifndef GL_UNSIGNED_INT8_VEC2
#define GL_UNSIGNED_INT8_VEC2                                        0x00008FED
#endif
#ifndef GL_TEXTURE_COORD_ARRAY_LENGTH
#define GL_TEXTURE_COORD_ARRAY_LENGTH                                0x00008F2F
#endif
#ifndef GL_DRAW_BUFFER3
#define GL_DRAW_BUFFER3                                              0x00008828
#endif
#ifndef GL_OBJECT_BUFFER_SIZE
#define GL_OBJECT_BUFFER_SIZE                                        0x00008764
#endif
#ifndef GL_LIGHT_MODEL_SPECULAR_VECTOR
#define GL_LIGHT_MODEL_SPECULAR_VECTOR                               0x000085B0
#endif
#ifndef GL_TEXTURE_COMPARE_OPERATOR
#define GL_TEXTURE_COMPARE_OPERATOR                                  0x0000819B
#endif
#ifndef GL_REG_4
#define GL_REG_4                                                     0x00008925
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY8
#define GL_VERTEX_ATTRIB_ARRAY8                                      0x00008658
#endif
#ifndef GL_DEBUG_CALLBACK_USER_PARAM
#define GL_DEBUG_CALLBACK_USER_PARAM                                 0x00008245
#endif
#ifndef GL_OPERAND0_RGB
#define GL_OPERAND0_RGB                                              0x00008590
#endif
#ifndef GL_DRAW_BUFFER
#define GL_DRAW_BUFFER                                               0x00000C01
#endif
#ifndef GL_PATH_FORMAT_PS
#define GL_PATH_FORMAT_PS                                            0x00009071
#endif
#ifndef GL_TEXTURE_BINDING_CUBE_MAP
#define GL_TEXTURE_BINDING_CUBE_MAP                                  0x00008514
#endif
#ifndef GL_TEXTURE_OBJECT_VALID
#define GL_TEXTURE_OBJECT_VALID                                      0x00008BDB
#endif
#ifndef GL_FAILURE
#define GL_FAILURE                                                   0x00009030
#endif
#ifndef GL_PATH_STROKE_COVER_MODE
#define GL_PATH_STROKE_COVER_MODE                                    0x00009083
#endif
#ifndef GL_RGB_INTEGER
#define GL_RGB_INTEGER                                               0x00008D98
#endif
#ifndef GL_TEXTURE12
#define GL_TEXTURE12                                                 0x000084CC
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_6x6
#define GL_COMPRESSED_RGBA_ASTC_6x6                                  0x000093B4
#endif
#ifndef GL_PLUS
#define GL_PLUS                                                      0x00009291
#endif
#ifndef GL_TEXTURE_SRGB_DECODE
#define GL_TEXTURE_SRGB_DECODE                                       0x00008A48
#endif
#ifndef GL_OFFSET_HILO_PROJECTIVE_TEXTURE_RECTANGLE
#define GL_OFFSET_HILO_PROJECTIVE_TEXTURE_RECTANGLE                  0x00008857
#endif
#ifndef GL_PIXEL_CUBIC_WEIGHT
#define GL_PIXEL_CUBIC_WEIGHT                                        0x00008333
#endif
#ifndef GL_VIRTUAL_PAGE_SIZE_X
#define GL_VIRTUAL_PAGE_SIZE_X                                       0x00009195
#endif
#ifndef GL_EDGE_FLAG_ARRAY_LENGTH
#define GL_EDGE_FLAG_ARRAY_LENGTH                                    0x00008F30
#endif
#ifndef GL_ALLOW_DRAW_WIN_HINT
#define GL_ALLOW_DRAW_WIN_HINT                                       0x0001A20F
#endif
#ifndef GL_TEXTURE_TYPE
#define GL_TEXTURE_TYPE                                              0x00008BD7
#endif
#ifndef GL_YCRCB_444
#define GL_YCRCB_444                                                 0x000081BC
#endif
#ifndef GL_UNIFORM_BLOCK_INDEX
#define GL_UNIFORM_BLOCK_INDEX                                       0x00008A3A
#endif
#ifndef GL_COLOR_MATRIX
#define GL_COLOR_MATRIX                                              0x000080B1
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5                          0x000093D5
#endif
#ifndef GL_VERTEX_STATE_PROGRAM
#define GL_VERTEX_STATE_PROGRAM                                      0x00008621
#endif
#ifndef GL_NEXT_VIDEO_CAPTURE_BUFFER_STATUS
#define GL_NEXT_VIDEO_CAPTURE_BUFFER_STATUS                          0x00009025
#endif
#ifndef GL_TEXTURE_RED_SIZE
#define GL_TEXTURE_RED_SIZE                                          0x0000805C
#endif
#ifndef GL_LUMINANCE_ALPHA8I
#define GL_LUMINANCE_ALPHA8I                                         0x00008D93
#endif
#ifndef GL_REG_11
#define GL_REG_11                                                    0x0000892C
#endif
#ifndef GL_TRANSFORM_HINT
#define GL_TRANSFORM_HINT                                            0x000085B1
#endif
#ifndef GL_PIXEL_TEX_GEN_MODE
#define GL_PIXEL_TEX_GEN_MODE                                        0x0000832B
#endif
#ifndef GL_SWIZZLE_STR_DR
#define GL_SWIZZLE_STR_DR                                            0x00008978
#endif
#ifndef GL_MATRIX_PALETTE
#define GL_MATRIX_PALETTE                                            0x00008840
#endif
#ifndef GL_FRAMEBUFFER_SRGB
#define GL_FRAMEBUFFER_SRGB                                          0x00008DB9
#endif
#ifndef GL_MAX_PROGRAM_MATRICES
#define GL_MAX_PROGRAM_MATRICES                                      0x0000862F
#endif
#ifndef GL_BIAS_BIT
#define GL_BIAS_BIT                                                  0x00000008
#endif
#ifndef GL_UNSIGNED_INT_8_8_S8_S8_REV
#define GL_UNSIGNED_INT_8_8_S8_S8_REV                                0x000086DB
#endif
#ifndef GL_DEBUG_SOURCE_API
#define GL_DEBUG_SOURCE_API                                          0x00008246
#endif
#ifndef GL_LINE_STRIP_ADJACENCY
#define GL_LINE_STRIP_ADJACENCY                                      0x0000000B
#endif
#ifndef GL_AFFINE_3D
#define GL_AFFINE_3D                                                 0x00009094
#endif
#ifndef GL_CMYKA
#define GL_CMYKA                                                     0x0000800D
#endif
#ifndef GL_DUAL_INTENSITY16
#define GL_DUAL_INTENSITY16                                          0x0000811B
#endif
#ifndef GL_VERTEX_ARRAY_COUNT
#define GL_VERTEX_ARRAY_COUNT                                        0x0000807D
#endif
#ifndef GL_RENDERBUFFER_INTERNAL_FORMAT
#define GL_RENDERBUFFER_INTERNAL_FORMAT                              0x00008D44
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8                         0x000093DA
#endif
#ifndef GL_UNSIGNED_INT_5_9_9_9_REV
#define GL_UNSIGNED_INT_5_9_9_9_REV                                  0x00008C3E
#endif
#ifndef GL_CIRCULAR_CW_ARC_TO
#define GL_CIRCULAR_CW_ARC_TO                                        0x000000FA
#endif
#ifndef GL_PACK_IMAGE_DEPTH
#define GL_PACK_IMAGE_DEPTH                                          0x00008131
#endif
#ifndef GL_MULTISAMPLE_BUFFER_BIT0
#define GL_MULTISAMPLE_BUFFER_BIT0                                   0x01000000
#endif
#ifndef GL_CURRENT_MATRIX
#define GL_CURRENT_MATRIX                                            0x00008641
#endif
#ifndef GL_Z6Y10Z6CB10Z6Y10Z6CR10_422
#define GL_Z6Y10Z6CB10Z6Y10Z6CR10_422                                0x00009033
#endif
#ifndef GL_REG_17
#define GL_REG_17                                                    0x00008932
#endif
#ifndef GL_PRIMITIVE_RESTART_FIXED_INDEX
#define GL_PRIMITIVE_RESTART_FIXED_INDEX                             0x00008D69
#endif
#ifndef GL_VERTEX_ATTRIB_MAP2_COEFF
#define GL_VERTEX_ATTRIB_MAP2_COEFF                                  0x00008A07
#endif
#ifndef GL_INDEX_MATERIAL
#define GL_INDEX_MATERIAL                                            0x000081B8
#endif
#ifndef GL_DRAW_BUFFER13
#define GL_DRAW_BUFFER13                                             0x00008832
#endif
#ifndef GL_HILO
#define GL_HILO                                                      0x000086F4
#endif
#ifndef GL_PERFQUERY_COUNTER_TIMESTAMP
#define GL_PERFQUERY_COUNTER_TIMESTAMP                               0x000094F5
#endif
#ifndef GL_MAX_VERTEX_SHADER_LOCAL_CONSTANTS
#define GL_MAX_VERTEX_SHADER_LOCAL_CONSTANTS                         0x000087C8
#endif
#ifndef GL_PROGRAM_ALU_INSTRUCTIONS
#define GL_PROGRAM_ALU_INSTRUCTIONS                                  0x00008805
#endif
#ifndef GL_GEOMETRY_DEFORMATION
#define GL_GEOMETRY_DEFORMATION                                      0x00008194
#endif
#ifndef GL_UNPACK_SKIP_IMAGES
#define GL_UNPACK_SKIP_IMAGES                                        0x0000806D
#endif
#ifndef GL_COMPRESSED_RGBA_BPTC_UNORM
#define GL_COMPRESSED_RGBA_BPTC_UNORM                                0x00008E8C
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_10x10
#define GL_COMPRESSED_RGBA_ASTC_10x10                                0x000093BB
#endif
#ifndef GL_REG_23
#define GL_REG_23                                                    0x00008938
#endif
#ifndef GL_EDGE_FLAG_ARRAY_BUFFER_BINDING
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING                            0x0000889B
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3                        0x000093E1
#endif
#ifndef GL_NORMAL_ARRAY_TYPE
#define GL_NORMAL_ARRAY_TYPE                                         0x0000807E
#endif
#ifndef GL_LIGHT_ENV_MODE
#define GL_LIGHT_ENV_MODE                                            0x00008407
#endif
#ifndef GL_OPERAND3_RGB
#define GL_OPERAND3_RGB                                              0x00008593
#endif
#ifndef GL_MAP2_VERTEX_ATTRIB8_4
#define GL_MAP2_VERTEX_ATTRIB8_4                                     0x00008678
#endif
#ifndef GL_TRANSFORM_FEEDBACK_ATTRIBS
#define GL_TRANSFORM_FEEDBACK_ATTRIBS                                0x00008C7E
#endif
#ifndef GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1                        0x00008A57
#endif
#ifndef GL_COVERAGE_BUFFERS
#define GL_COVERAGE_BUFFERS                                          0x00008ED3
#endif
#ifndef GL_PARAMETER_BUFFER
#define GL_PARAMETER_BUFFER                                          0x000080EE
#endif
#ifndef GL_ALPHA_MIN_CLAMP
#define GL_ALPHA_MIN_CLAMP                                           0x00008563
#endif
#ifndef GL_LAYOUT_LINEAR
#define GL_LAYOUT_LINEAR                                             0x00000001
#endif
#ifndef GL_VERTEX_ARRAY_OBJECT
#define GL_VERTEX_ARRAY_OBJECT                                       0x00009154
#endif
#ifndef GL_TRANSFORM_FEEDBACK_BUFFER
#define GL_TRANSFORM_FEEDBACK_BUFFER                                 0x00008C8E
#endif
#ifndef GL_LUMINANCE32UI
#define GL_LUMINANCE32UI                                             0x00008D74
#endif
#ifndef GL_REG_31
#define GL_REG_31                                                    0x00008940
#endif
#ifndef GL_TANGENT_ARRAY_TYPE
#define GL_TANGENT_ARRAY_TYPE                                        0x0000843E
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6                        0x000093E9
#endif
#ifndef GL_CON_28
#define GL_CON_28                                                    0x0000895D
#endif
#ifndef GL_SKIP_COMPONENTS1
#define GL_SKIP_COMPONENTS1                                          0xFFFFFFFFFFFFFFFA
#endif
#ifndef GL_LUMINANCE8_ALPHA8
#define GL_LUMINANCE8_ALPHA8                                         0x00008045
#endif
#ifndef GL_MAX_GEOMETRY_BINDABLE_UNIFORMS
#define GL_MAX_GEOMETRY_BINDABLE_UNIFORMS                            0x00008DE4
#endif
#ifndef GL_PROXY_TEXTURE_2D
#define GL_PROXY_TEXTURE_2D                                          0x00008064
#endif
#ifndef GL_MODULATE_ADD
#define GL_MODULATE_ADD                                              0x00008744
#endif
#ifndef GL_VARIANT_ARRAY_STRIDE
#define GL_VARIANT_ARRAY_STRIDE                                      0x000087E6
#endif
#ifndef GL_FLOAT_R16
#define GL_FLOAT_R16                                                 0x00008884
#endif
#ifndef GL_DSDT8_MAG8
#define GL_DSDT8_MAG8                                                0x0000870A
#endif
#ifndef GL_RGB16I
#define GL_RGB16I                                                    0x00008D89
#endif
#ifndef GL_VERTEX_ARRAY_PARALLEL_POINTERS
#define GL_VERTEX_ARRAY_PARALLEL_POINTERS                            0x000083F5
#endif
#ifndef GL_UNSIGNED_IDENTITY
#define GL_UNSIGNED_IDENTITY                                         0x00008536
#endif
#ifndef GL_TIMESTAMP
#define GL_TIMESTAMP                                                 0x00008E28
#endif
#ifndef GL_DOT_PRODUCT_REFLECT_CUBE_MAP
#define GL_DOT_PRODUCT_REFLECT_CUBE_MAP                              0x000086F2
#endif
#ifndef GL_STENCIL_ATTACHMENT
#define GL_STENCIL_ATTACHMENT                                        0x00008D20
#endif
#ifndef GL_OBJECT_DISTANCE_TO_POINT
#define GL_OBJECT_DISTANCE_TO_POINT                                  0x000081F1
#endif
#ifndef GL_CONSERVE_MEMORY_HINT
#define GL_CONSERVE_MEMORY_HINT                                      0x0001A1FD
#endif
#ifndef GL_COVERAGE_ATTACHMENT
#define GL_COVERAGE_ATTACHMENT                                       0x00008ED2
#endif
#ifndef GL_UNPACK_IMAGE_HEIGHT
#define GL_UNPACK_IMAGE_HEIGHT                                       0x0000806E
#endif
#ifndef GL_COLOR_BUFFER_BIT3
#define GL_COLOR_BUFFER_BIT3                                         0x00000008
#endif
#ifndef GL_VIRTUAL_PAGE_SIZE_INDEX
#define GL_VIRTUAL_PAGE_SIZE_INDEX                                   0x000091A7
#endif
#ifndef GL_DOUBLE_MAT2x4
#define GL_DOUBLE_MAT2x4                                             0x00008F4A
#endif
#ifndef GL_INDEX_ARRAY_LIST
#define GL_INDEX_ARRAY_LIST                                          0x000192A1
#endif
#ifndef GL_MAP1_VERTEX_ATTRIB6_4
#define GL_MAP1_VERTEX_ATTRIB6_4                                     0x00008666
#endif
#ifndef GL_MAX_COMPUTE_VARIABLE_GROUP_SIZE
#define GL_MAX_COMPUTE_VARIABLE_GROUP_SIZE                           0x00009345
#endif
#ifndef GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS                             0x00008A42
#endif
#ifndef GL_INT_VEC2
#define GL_INT_VEC2                                                  0x00008B53
#endif
#ifndef GL_MAX_PROGRAM_ATTRIB_COMPONENTS
#define GL_MAX_PROGRAM_ATTRIB_COMPONENTS                             0x00008908
#endif
#ifndef GL_LINEARDODGE
#define GL_LINEARDODGE                                               0x000092A4
#endif
#ifndef GL_ATTRIB_ARRAY_STRIDE
#define GL_ATTRIB_ARRAY_STRIDE                                       0x00008624
#endif
#ifndef GL_PHONG
#define GL_PHONG                                                     0x000080EA
#endif
#ifndef GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS                0x00008C80
#endif
#ifndef GL_COLOR_ATTACHMENT5
#define GL_COLOR_ATTACHMENT5                                         0x00008CE5
#endif
#ifndef GL_REGISTER_COMBINERS
#define GL_REGISTER_COMBINERS                                        0x00008522
#endif
#ifndef GL_TRANSPOSE_AFFINE_2D
#define GL_TRANSPOSE_AFFINE_2D                                       0x00009096
#endif
#ifndef GL_UNSIGNED_SHORT_8_8
#define GL_UNSIGNED_SHORT_8_8                                        0x000085BA
#endif
#ifndef GL_INTENSITY32F
#define GL_INTENSITY32F                                              0x00008817
#endif
#ifndef GL_T2F_IUI_N3F_V3F
#define GL_T2F_IUI_N3F_V3F                                           0x000081B4
#endif
#ifndef GL_EVAL_VERTEX_ATTRIB3
#define GL_EVAL_VERTEX_ATTRIB3                                       0x000086C9
#endif
#ifndef GL_RG32UI
#define GL_RG32UI                                                    0x0000823C
#endif
#ifndef GL_VARIABLE_B
#define GL_VARIABLE_B                                                0x00008524
#endif
#ifndef GL_TEXTURE21
#define GL_TEXTURE21                                                 0x000084D5
#endif
#ifndef GL_POLYGON_OFFSET
#define GL_POLYGON_OFFSET                                            0x00008037
#endif
#ifndef GL_TEXTURE_1D_ARRAY
#define GL_TEXTURE_1D_ARRAY                                          0x00008C18
#endif
#ifndef GL_OBJECT_LINK_STATUS
#define GL_OBJECT_LINK_STATUS                                        0x00008B82
#endif
#ifndef GL_SRC
#define GL_SRC                                                       0x00009286
#endif
#ifndef GL_HISTOGRAM_ALPHA_SIZE
#define GL_HISTOGRAM_ALPHA_SIZE                                      0x0000802B
#endif
#ifndef GL_COLOR_ARRAY_PARALLEL_POINTERS
#define GL_COLOR_ARRAY_PARALLEL_POINTERS                             0x000083F7
#endif
#ifndef GL_MAX_SERVER_WAIT_TIMEOUT
#define GL_MAX_SERVER_WAIT_TIMEOUT                                   0x00009111
#endif
#ifndef GL_TEXTURE_DS_SIZE
#define GL_TEXTURE_DS_SIZE                                           0x0000871D
#endif
#ifndef GL_POLYGON_OFFSET_BIAS
#define GL_POLYGON_OFFSET_BIAS                                       0x00008039
#endif
#ifndef GL_SAMPLE_POSITION
#define GL_SAMPLE_POSITION                                           0x00008E50
#endif
#ifndef GL_ELEMENT_ARRAY_LENGTH
#define GL_ELEMENT_ARRAY_LENGTH                                      0x00008F33
#endif
#ifndef GL_RGB32F
#define GL_RGB32F                                                    0x00008815
#endif
#ifndef GL_STENCIL_BACK_PASS_DEPTH_PASS
#define GL_STENCIL_BACK_PASS_DEPTH_PASS                              0x00008803
#endif
#ifndef GL_DUAL_INTENSITY12
#define GL_DUAL_INTENSITY12                                          0x0000811A
#endif
#ifndef GL_FLOAT_VEC2
#define GL_FLOAT_VEC2                                                0x00008B50
#endif
#ifndef GL_DOT_PRODUCT_TEXTURE_3D
#define GL_DOT_PRODUCT_TEXTURE_3D                                    0x000086EF
#endif
#ifndef GL_QUAD_INTENSITY4
#define GL_QUAD_INTENSITY4                                           0x00008122
#endif
#ifndef GL_OBJECT_DELETE_STATUS
#define GL_OBJECT_DELETE_STATUS                                      0x00008B80
#endif
#ifndef GL_BLEND_SRC_ALPHA
#define GL_BLEND_SRC_ALPHA                                           0x000080CB
#endif
#ifndef GL_MULTISAMPLE_BUFFER_BIT3
#define GL_MULTISAMPLE_BUFFER_BIT3                                   0x08000000
#endif
#ifndef GL_PROGRAM_TARGET
#define GL_PROGRAM_TARGET                                            0x00008646
#endif
#ifndef GL_COMBINER_CD_OUTPUT
#define GL_COMBINER_CD_OUTPUT                                        0x0000854B
#endif
#ifndef GL_OUTPUT_COLOR1
#define GL_OUTPUT_COLOR1                                             0x0000879C
#endif
#ifndef GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS
#define GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS                           0x0000919A
#endif
#ifndef GL_HALF_BIT
#define GL_HALF_BIT                                                  0x00000008
#endif
#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY
#define GL_MAX_TEXTURE_MAX_ANISOTROPY                                0x000084FF
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD6
#define GL_OUTPUT_TEXTURE_COORD6                                     0x000087A3
#endif
#ifndef GL_MAX_PROGRAM_ADDRESS_REGISTERS
#define GL_MAX_PROGRAM_ADDRESS_REGISTERS                             0x000088B1
#endif
#ifndef GL_RGB32UI
#define GL_RGB32UI                                                   0x00008D71
#endif
#ifndef GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCALS
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCALS                        0x000087CE
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5
#define GL_COMPRESSED_RGBA_S3TC_DXT5                                 0x000083F3
#endif
#ifndef GL_CULL_VERTEX
#define GL_CULL_VERTEX                                               0x000081AA
#endif
#ifndef GL_OP_NEGATE
#define GL_OP_NEGATE                                                 0x00008783
#endif
#ifndef GL_LUMINANCE16UI
#define GL_LUMINANCE16UI                                             0x00008D7A
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD11
#define GL_OUTPUT_TEXTURE_COORD11                                    0x000087A8
#endif
#ifndef GL_COMBINER2
#define GL_COMBINER2                                                 0x00008552
#endif
#ifndef GL_DEBUG_OUTPUT_SYNCHRONOUS
#define GL_DEBUG_OUTPUT_SYNCHRONOUS                                  0x00008242
#endif
#ifndef GL_COVERAGE_AUTOMATIC
#define GL_COVERAGE_AUTOMATIC                                        0x00008ED7
#endif
#ifndef GL_PRIMITIVE_ID
#define GL_PRIMITIVE_ID                                              0x00008C7C
#endif
#ifndef GL_DEBUG_TYPE_PERFORMANCE
#define GL_DEBUG_TYPE_PERFORMANCE                                    0x00008250
#endif
#ifndef GL_NATIVE_GRAPHICS_HANDLE
#define GL_NATIVE_GRAPHICS_HANDLE                                    0x0001A202
#endif
#ifndef GL_POST_CONVOLUTION_COLOR_TABLE
#define GL_POST_CONVOLUTION_COLOR_TABLE                              0x000080D1
#endif
#ifndef GL_OFFSET_TEXTURE_BIAS
#define GL_OFFSET_TEXTURE_BIAS                                       0x000086E3
#endif
#ifndef GL_MATRIX27
#define GL_MATRIX27                                                  0x000088DB
#endif
#ifndef GL_SPARE0_PLUS_SECONDARY_COLOR
#define GL_SPARE0_PLUS_SECONDARY_COLOR                               0x00008532
#endif
#ifndef GL_UNSIGNED_INT_IMAGE_2D_RECT
#define GL_UNSIGNED_INT_IMAGE_2D_RECT                                0x00009065
#endif
#ifndef GL_PN_TRIANGLES_POINT_MODE
#define GL_PN_TRIANGLES_POINT_MODE                                   0x000087F2
#endif
#ifndef GL_INTERLEAVED_ATTRIBS
#define GL_INTERLEAVED_ATTRIBS                                       0x00008C8C
#endif
#ifndef GL_INTENSITY8
#define GL_INTENSITY8                                                0x0000804B
#endif
#ifndef GL_TEXTURE_COORD_ARRAY_PARALLEL_POINTERS
#define GL_TEXTURE_COORD_ARRAY_PARALLEL_POINTERS                     0x000083F8
#endif
#ifndef GL_PROXY_POST_CONVOLUTION_COLOR_TABLE
#define GL_PROXY_POST_CONVOLUTION_COLOR_TABLE                        0x000080D4
#endif
#ifndef GL_UNSIGNED_SHORT_1_5_5_5_REV
#define GL_UNSIGNED_SHORT_1_5_5_5_REV                                0x00008366
#endif
#ifndef GL_GLYPH_VERTICAL_BEARING_Y_BIT
#define GL_GLYPH_VERTICAL_BEARING_Y_BIT                              0x00000040
#endif
#ifndef GL_VERTEX_STREAM7
#define GL_VERTEX_STREAM7                                            0x00008773
#endif
#ifndef GL_MAP1_VERTEX_ATTRIB7_4
#define GL_MAP1_VERTEX_ATTRIB7_4                                     0x00008667
#endif
#ifndef GL_INDEX_ARRAY_ADDRESS
#define GL_INDEX_ARRAY_ADDRESS                                       0x00008F24
#endif
#ifndef GL_DEPTH_COMPONENT16_NONLINEAR
#define GL_DEPTH_COMPONENT16_NONLINEAR                               0x00008E2C
#endif
#ifndef GL_HARDMIX
#define GL_HARDMIX                                                   0x000092A9
#endif
#ifndef GL_COUNTER_TYPE
#define GL_COUNTER_TYPE                                              0x00008BC0
#endif
#ifndef GL_FIELDS
#define GL_FIELDS                                                    0x00008E27
#endif
#ifndef GL_RENDER_DIRECT_TO_FRAMEBUFFER
#define GL_RENDER_DIRECT_TO_FRAMEBUFFER                              0x00008FB3
#endif
#ifndef GL_TEXTURE_MAX_ANISOTROPY
#define GL_TEXTURE_MAX_ANISOTROPY                                    0x000084FE
#endif
#ifndef GL_VIDEO_BUFFER_PITCH
#define GL_VIDEO_BUFFER_PITCH                                        0x00009028
#endif
#ifndef GL_LINEAR_SHARPEN_ALPHA
#define GL_LINEAR_SHARPEN_ALPHA                                      0x000080AE
#endif
#ifndef GL_REDUCE
#define GL_REDUCE                                                    0x00008016
#endif
#ifndef GL_TEXTURE5
#define GL_TEXTURE5                                                  0x000084C5
#endif
#ifndef GL_CONSTANT_COLOR0
#define GL_CONSTANT_COLOR0                                           0x0000852A
#endif
#ifndef GL_REG_7
#define GL_REG_7                                                     0x00008928
#endif
#ifndef GL_PIXEL_TEXTURE
#define GL_PIXEL_TEXTURE                                             0x00008353
#endif
#ifndef GL_COLOR_ARRAY_STRIDE
#define GL_COLOR_ARRAY_STRIDE                                        0x00008083
#endif
#ifndef GL_PERFMON_RESULT_AVAILABLE
#define GL_PERFMON_RESULT_AVAILABLE                                  0x00008BC4
#endif
#ifndef GL_MODELVIEW16
#define GL_MODELVIEW16                                               0x00008730
#endif
#ifndef GL_MODELVIEW22
#define GL_MODELVIEW22                                               0x00008736
#endif
#ifndef GL_PROGRAM_POINT_SIZE
#define GL_PROGRAM_POINT_SIZE                                        0x00008642
#endif
#ifndef GL_STORAGE_CLIENT
#define GL_STORAGE_CLIENT                                            0x000085B4
#endif
#ifndef GL_ROUND
#define GL_ROUND                                                     0x000090A4
#endif
#ifndef GL_RENDERBUFFER_ALPHA_SIZE
#define GL_RENDERBUFFER_ALPHA_SIZE                                   0x00008D53
#endif
#ifndef GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER                        0x00008CDC
#endif
#ifndef GL_VERTEX_ARRAY_STORAGE_HINT
#define GL_VERTEX_ARRAY_STORAGE_HINT                                 0x0000851F
#endif
#ifndef GL_STANDARD_FONT_NAME
#define GL_STANDARD_FONT_NAME                                        0x00009072
#endif
#ifndef GL_MAP2_VERTEX_ATTRIB9_4
#define GL_MAP2_VERTEX_ATTRIB9_4                                     0x00008679
#endif
#ifndef GL_MATRIX10
#define GL_MATRIX10                                                  0x000088CA
#endif
#ifndef GL_TEXTURE_SWIZZLE_RGBA
#define GL_TEXTURE_SWIZZLE_RGBA                                      0x00008E46
#endif
#ifndef GL_CON_23
#define GL_CON_23                                                    0x00008958
#endif
#ifndef GL_EVAL_VERTEX_ATTRIB0
#define GL_EVAL_VERTEX_ATTRIB0                                       0x000086C6
#endif
#ifndef GL_DEPENDENT_RGB_TEXTURE_3D
#define GL_DEPENDENT_RGB_TEXTURE_3D                                  0x00008859
#endif
#ifndef GL_UNSIGNED_INT_24_8
#define GL_UNSIGNED_INT_24_8                                         0x000084FA
#endif
#ifndef GL_ALLOW_DRAW_FRG_HINT
#define GL_ALLOW_DRAW_FRG_HINT                                       0x0001A210
#endif
#ifndef GL_GREEN_MAX_CLAMP
#define GL_GREEN_MAX_CLAMP                                           0x00008565
#endif
#ifndef GL_SLICE_ACCUM
#define GL_SLICE_ACCUM                                               0x000085CC
#endif
#ifndef GL_INT_IMAGE_3D
#define GL_INT_IMAGE_3D                                              0x00009059
#endif
#ifndef GL_INT64_VEC3
#define GL_INT64_VEC3                                                0x00008FEA
#endif
#ifndef GL_TEXTURE_BASE_LEVEL
#define GL_TEXTURE_BASE_LEVEL                                        0x0000813C
#endif
#ifndef GL_COMBINER_INPUT
#define GL_COMBINER_INPUT                                            0x00008542
#endif
#ifndef GL_BOUNDING_BOX_OF_BOUNDING_BOXES
#define GL_BOUNDING_BOX_OF_BOUNDING_BOXES                            0x0000909C
#endif
#ifndef GL_IMAGE_TRANSLATE_X
#define GL_IMAGE_TRANSLATE_X                                         0x00008157
#endif
#ifndef GL_PATH_STENCIL_FUNC
#define GL_PATH_STENCIL_FUNC                                         0x000090B7
#endif
#ifndef GL_VERTEX_ARRAY_SIZE
#define GL_VERTEX_ARRAY_SIZE                                         0x0000807A
#endif
#ifndef GL_MODELVIEW26
#define GL_MODELVIEW26                                               0x0000873A
#endif
#ifndef GL_SIGNED_RGB8_UNSIGNED_ALPHA8
#define GL_SIGNED_RGB8_UNSIGNED_ALPHA8                               0x0000870D
#endif
#ifndef GL_DOUBLE_MAT3x4
#define GL_DOUBLE_MAT3x4                                             0x00008F4C
#endif
#ifndef GL_PERFQUERY_GLOBAL_CONTEXT
#define GL_PERFQUERY_GLOBAL_CONTEXT                                  0x00000001
#endif
#ifndef GL_COPY_WRITE_BUFFER_BINDING
#define GL_COPY_WRITE_BUFFER_BINDING                                 0x00008F37
#endif
#ifndef GL_GEOMETRY_PROGRAM
#define GL_GEOMETRY_PROGRAM                                          0x00008C26
#endif
#ifndef GL_R32UI
#define GL_R32UI                                                     0x00008236
#endif
#ifndef GL_POST_COLOR_MATRIX_COLOR_TABLE
#define GL_POST_COLOR_MATRIX_COLOR_TABLE                             0x000080D2
#endif
#ifndef GL_COMPRESSED_RGBA_PVRTC_2BPPV1
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1                              0x00008C03
#endif
#ifndef GL_OUTPUT_VERTEX
#define GL_OUTPUT_VERTEX                                             0x0000879A
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1
#define GL_COMPRESSED_RGBA_S3TC_DXT1                                 0x000083F1
#endif
#ifndef GL_NORMAL_MAP
#define GL_NORMAL_MAP                                                0x00008511
#endif
#ifndef GL_PRIMITIVES_GENERATED
#define GL_PRIMITIVES_GENERATED                                      0x00008C87
#endif
#ifndef GL_R1UI_C4F_N3F_V3F
#define GL_R1UI_C4F_N3F_V3F                                          0x000085C8
#endif
#ifndef GL_TEXTURE_USAGE
#define GL_TEXTURE_USAGE                                             0x000093A2
#endif
#ifndef GL_PERFQUERY_COUNTER_DESC_LENGTH_MAX
#define GL_PERFQUERY_COUNTER_DESC_LENGTH_MAX                         0x000094FF
#endif
#ifndef GL_MAX_PALETTE_MATRICES
#define GL_MAX_PALETTE_MATRICES                                      0x00008842
#endif
#ifndef GL_ACTIVE_TEXTURE
#define GL_ACTIVE_TEXTURE                                            0x000084E0
#endif
#ifndef GL_ACTIVE_UNIFORM_BLOCKS
#define GL_ACTIVE_UNIFORM_BLOCKS                                     0x00008A36
#endif
#ifndef GL_DEPTH_ATTACHMENT
#define GL_DEPTH_ATTACHMENT                                          0x00008D00
#endif
#ifndef GL_MAX_MULTIVIEW_BUFFERS
#define GL_MAX_MULTIVIEW_BUFFERS                                     0x000090F2
#endif
#ifndef GL_VERTEX_ATTRIB_MAP2_SIZE
#define GL_VERTEX_ATTRIB_MAP2_SIZE                                   0x00008A06
#endif
#ifndef GL_TEXTURE_HI_SIZE
#define GL_TEXTURE_HI_SIZE                                           0x0000871B
#endif
#ifndef GL_PRESENT_TIME
#define GL_PRESENT_TIME                                              0x00008E2A
#endif
#ifndef GL_MAP_INVALIDATE_RANGE_BIT
#define GL_MAP_INVALIDATE_RANGE_BIT                                  0x00000004
#endif
#ifndef GL_VERTEX_SHADER_LOCAL_CONSTANTS
#define GL_VERTEX_SHADER_LOCAL_CONSTANTS                             0x000087D2
#endif
#ifndef GL_TEXTURE_COORD_ARRAY_TYPE
#define GL_TEXTURE_COORD_ARRAY_TYPE                                  0x00008089
#endif
#ifndef GL_MATRIX15
#define GL_MATRIX15                                                  0x000088CF
#endif
#ifndef GL_DATA_BUFFER
#define GL_DATA_BUFFER                                               0x00009151
#endif
#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER                                           0x00008B30
#endif
#ifndef GL_NUM_FRAGMENT_REGISTERS
#define GL_NUM_FRAGMENT_REGISTERS                                    0x0000896E
#endif
#ifndef GL_VERTEX_WEIGHT_ARRAY_STRIDE
#define GL_VERTEX_WEIGHT_ARRAY_STRIDE                                0x0000850F
#endif
#ifndef GL_SGX_PROGRAM_BINARY
#define GL_SGX_PROGRAM_BINARY                                        0x00009130
#endif
#ifndef GL_TEXTURE_MATERIAL_FACE
#define GL_TEXTURE_MATERIAL_FACE                                     0x00008351
#endif
#ifndef GL_MAX_VERTEX_STREAMS
#define GL_MAX_VERTEX_STREAMS                                        0x0000876B
#endif
#ifndef GL_COMPARE_REF_DEPTH_TO_TEXTURE
#define GL_COMPARE_REF_DEPTH_TO_TEXTURE                              0x0000884E
#endif
#ifndef GL_MAP1_TANGENT
#define GL_MAP1_TANGENT                                              0x00008444
#endif
#ifndef GL_DEBUG_SOURCE_WINDOW_SYSTEM
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM                                0x00008247
#endif
#ifndef GL_FOG_COORD_ARRAY_ADDRESS
#define GL_FOG_COORD_ARRAY_ADDRESS                                   0x00008F28
#endif
#ifndef GL_TEXTURE28
#define GL_TEXTURE28                                                 0x000084DC
#endif
#ifndef GL_EDGE_FLAG_ARRAY_LIST_STRIDE
#define GL_EDGE_FLAG_ARRAY_LIST_STRIDE                               0x000192AD
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY_POINTER
#define GL_VERTEX_ATTRIB_ARRAY_POINTER                               0x00008645
#endif
#ifndef GL_PATH_MITER_LIMIT
#define GL_PATH_MITER_LIMIT                                          0x0000907A
#endif
#ifndef GL_COLORBURN
#define GL_COLORBURN                                                 0x0000929A
#endif
#ifndef GL_MAX_PROGRAM_NATIVE_PARAMETERS
#define GL_MAX_PROGRAM_NATIVE_PARAMETERS                             0x000088AB
#endif
#ifndef GL_TEXTURE_RED_TYPE
#define GL_TEXTURE_RED_TYPE                                          0x00008C10
#endif
#ifndef GL_EYE_LINE
#define GL_EYE_LINE                                                  0x000081F6
#endif
#ifndef GL_CONDITION_SATISFIED
#define GL_CONDITION_SATISFIED                                       0x0000911C
#endif
#ifndef GL_ALL_SHADER_BITS
#define GL_ALL_SHADER_BITS                                           0xFFFFFFFF
#endif
#ifndef GL_FRAGMENT_LIGHT_MODEL_TWO_SIDE
#define GL_FRAGMENT_LIGHT_MODEL_TWO_SIDE                             0x00008409
#endif
#ifndef GL_PROGRAM_ATTRIBS
#define GL_PROGRAM_ATTRIBS                                           0x000088AC
#endif
#ifndef GL_FILE_NAME
#define GL_FILE_NAME                                                 0x00009074
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD1
#define GL_OUTPUT_TEXTURE_COORD1                                     0x0000879E
#endif
#ifndef GL_MINOR_VERSION
#define GL_MINOR_VERSION                                             0x0000821C
#endif
#ifndef GL_PROXY_TEXTURE_CUBE_MAP_ARRAY
#define GL_PROXY_TEXTURE_CUBE_MAP_ARRAY                              0x0000900B
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_5x5x5
#define GL_COMPRESSED_RGBA_ASTC_5x5x5                                0x000093C6
#endif
#ifndef GL_FRAGMENT_LIGHT4
#define GL_FRAGMENT_LIGHT4                                           0x00008410
#endif
#ifndef GL_LOCAL
#define GL_LOCAL                                                     0x000087C4
#endif
#ifndef GL_SAMPLER_RENDERBUFFER
#define GL_SAMPLER_RENDERBUFFER                                      0x00008E56
#endif
#ifndef GL_Z400_BINARY
#define GL_Z400_BINARY                                               0x00008740
#endif
#ifndef GL_OFFSET_TEXTURE_2D_BIAS
#define GL_OFFSET_TEXTURE_2D_BIAS                                    0x000086E3
#endif
#ifndef GL_TRIANGLES_ADJACENCY
#define GL_TRIANGLES_ADJACENCY                                       0x0000000C
#endif
#ifndef GL_CON_2
#define GL_CON_2                                                     0x00008943
#endif
#ifndef GL_OBJECT_ACTIVE_ATTRIBUTES
#define GL_OBJECT_ACTIVE_ATTRIBUTES                                  0x00008B89
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY13
#define GL_VERTEX_ATTRIB_ARRAY13                                     0x0000865D
#endif
#ifndef GL_ADD_SIGNED
#define GL_ADD_SIGNED                                                0x00008574
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD10
#define GL_OUTPUT_TEXTURE_COORD10                                    0x000087A7
#endif
#ifndef GL_VARIANT_DATATYPE
#define GL_VARIANT_DATATYPE                                          0x000087E5
#endif
#ifndef GL_INVARIANT
#define GL_INVARIANT                                                 0x000087C2
#endif
#ifndef GL_FIRST_TO_REST
#define GL_FIRST_TO_REST                                             0x000090AF
#endif
#ifndef GL_OPERAND0_ALPHA
#define GL_OPERAND0_ALPHA                                            0x00008598
#endif
#ifndef GL_EVAL_VERTEX_ATTRIB13
#define GL_EVAL_VERTEX_ATTRIB13                                      0x000086D3
#endif
#ifndef GL_DETAIL_TEXTURE_LEVEL
#define GL_DETAIL_TEXTURE_LEVEL                                      0x0000809A
#endif
#ifndef GL_ETC1_SRGB8
#define GL_ETC1_SRGB8                                                0x000088EE
#endif
#ifndef GL_DUAL_INTENSITY8
#define GL_DUAL_INTENSITY8                                           0x00008119
#endif
#ifndef GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE
#define GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE                       0x000080D5
#endif
#ifndef GL_CON_17
#define GL_CON_17                                                    0x00008952
#endif
#ifndef GL_FRAMEBUFFER
#define GL_FRAMEBUFFER                                               0x00008D40
#endif
#ifndef GL_CULL_MODES
#define GL_CULL_MODES                                                0x000086E0
#endif
#ifndef GL_TESSELLATION_MODE
#define GL_TESSELLATION_MODE                                         0x00009004
#endif
#ifndef GL_TEXTURE_BINDING_RECTANGLE
#define GL_TEXTURE_BINDING_RECTANGLE                                 0x000084F6
#endif
#ifndef GL_PACK_ROW_BYTES
#define GL_PACK_ROW_BYTES                                            0x00008A15
#endif
#ifndef GL_BLEND_DST_ALPHA
#define GL_BLEND_DST_ALPHA                                           0x000080CA
#endif
#ifndef GL_MAX_DRAW_BUFFERS
#define GL_MAX_DRAW_BUFFERS                                          0x00008824
#endif
#ifndef GL_MATRIX4
#define GL_MATRIX4                                                   0x000088C4
#endif
#ifndef GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY
#define GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY                         0x0000906A
#endif
#ifndef GL_POST_CONVOLUTION_BLUE_BIAS
#define GL_POST_CONVOLUTION_BLUE_BIAS                                0x00008022
#endif
#ifndef GL_EMBOSS_CONSTANT
#define GL_EMBOSS_CONSTANT                                           0x0000855E
#endif
#ifndef GL_OBJECT_COMPILE_STATUS
#define GL_OBJECT_COMPILE_STATUS                                     0x00008B81
#endif
#ifndef GL_SOFTLIGHT
#define GL_SOFTLIGHT                                                 0x0000929C
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD30
#define GL_OUTPUT_TEXTURE_COORD30                                    0x000087BB
#endif
#ifndef GL_COLOR_INDEX2
#define GL_COLOR_INDEX2                                              0x000080E3
#endif
#ifndef GL_DETAIL_TEXTURE_2D_BINDING
#define GL_DETAIL_TEXTURE_2D_BINDING                                 0x00008096
#endif
#ifndef GL_VERTEX_ARRAY_POINTER
#define GL_VERTEX_ARRAY_POINTER                                      0x0000808E
#endif
#ifndef GL_2PASS_1
#define GL_2PASS_1                                                   0x000080A3
#endif
#ifndef GL_COMPRESSED_SRGB_S3TC_DXT1
#define GL_COMPRESSED_SRGB_S3TC_DXT1                                 0x00008C4C
#endif
#ifndef GL_EXPAND_NEGATE
#define GL_EXPAND_NEGATE                                             0x00008539
#endif
#ifndef GL_POST_TEXTURE_FILTER_BIAS_RANGE
#define GL_POST_TEXTURE_FILTER_BIAS_RANGE                            0x0000817B
#endif
#ifndef GL_R8
#define GL_R8                                                        0x00008229
#endif
#ifndef GL_STENCIL_BUFFER_BIT7
#define GL_STENCIL_BUFFER_BIT7                                       0x00800000
#endif
#ifndef GL_TEXTURE_SWIZZLE_G
#define GL_TEXTURE_SWIZZLE_G                                         0x00008E43
#endif
#ifndef GL_VERTEX_SHADER_INVARIANTS
#define GL_VERTEX_SHADER_INVARIANTS                                  0x000087D1
#endif
#ifndef GL_COLOR_ATTACHMENT8
#define GL_COLOR_ATTACHMENT8                                         0x00008CE8
#endif
#ifndef GL_FLOAT_MAT4
#define GL_FLOAT_MAT4                                                0x00008B5C
#endif
#ifndef GL_SAMPLER_3D
#define GL_SAMPLER_3D                                                0x00008B5F
#endif
#ifndef GL_DOT3_RGBA
#define GL_DOT3_RGBA                                                 0x000086AF
#endif
#ifndef GL_EDGE_FLAG_ARRAY_STRIDE
#define GL_EDGE_FLAG_ARRAY_STRIDE                                    0x0000808C
#endif
#ifndef GL_MODELVIEW1_STACK_DEPTH
#define GL_MODELVIEW1_STACK_DEPTH                                    0x00008502
#endif
#ifndef GL_DRAW_INDIRECT_ADDRESS
#define GL_DRAW_INDIRECT_ADDRESS                                     0x00008F41
#endif
#ifndef GL_MAX_DEBUG_MESSAGE_LENGTH
#define GL_MAX_DEBUG_MESSAGE_LENGTH                                  0x00009143
#endif
#ifndef GL_DOUBLE_VEC2
#define GL_DOUBLE_VEC2                                               0x00008FFC
#endif
#ifndef GL_PATH_DASH_OFFSET
#define GL_PATH_DASH_OFFSET                                          0x0000907E
#endif
#ifndef GL_SAMPLER_1D_ARRAY
#define GL_SAMPLER_1D_ARRAY                                          0x00008DC0
#endif
#ifndef GL_DEBUG_SOURCE_SHADER_COMPILER
#define GL_DEBUG_SOURCE_SHADER_COMPILER                              0x00008248
#endif
#ifndef GL_UNSIGNED_INT_IMAGE_1D
#define GL_UNSIGNED_INT_IMAGE_1D                                     0x00009062
#endif
#ifndef GL_MAX_ASYNC_TEX_IMAGE
#define GL_MAX_ASYNC_TEX_IMAGE                                       0x0000835F
#endif
#ifndef GL_EVAL_2D
#define GL_EVAL_2D                                                   0x000086C0
#endif
#ifndef GL_ACCUM_ADJACENT_PAIRS
#define GL_ACCUM_ADJACENT_PAIRS                                      0x000090AD
#endif
#ifndef GL_TEXTURE_STORAGE_SPARSE_BIT
#define GL_TEXTURE_STORAGE_SPARSE_BIT                                0x00000001
#endif
#ifndef GL_MAP_UNSYNCHRONIZED_BIT
#define GL_MAP_UNSYNCHRONIZED_BIT                                    0x00000020
#endif
#ifndef GL_FOG_FUNC
#define GL_FOG_FUNC                                                  0x0000812A
#endif
#ifndef GL_ATTRIB_ARRAY_POINTER
#define GL_ATTRIB_ARRAY_POINTER                                      0x00008645
#endif
#ifndef GL_RELATIVE_ARC_TO
#define GL_RELATIVE_ARC_TO                                           0x000000FF
#endif
#ifndef GL_ALL_COMPLETED
#define GL_ALL_COMPLETED                                             0x000084F2
#endif
#ifndef GL_PERFQUERY_WAIT
#define GL_PERFQUERY_WAIT                                            0x000083FB
#endif
#ifndef GL_SRC_IN
#define GL_SRC_IN                                                    0x0000928A
#endif
#ifndef GL_VBO_FREE_MEMORY
#define GL_VBO_FREE_MEMORY                                           0x000087FB
#endif
#ifndef GL_VERTEX_ARRAY_RANGE_LENGTH
#define GL_VERTEX_ARRAY_RANGE_LENGTH                                 0x0000851E
#endif
#ifndef GL_VERTEX_ARRAY_BINDING
#define GL_VERTEX_ARRAY_BINDING                                      0x000085B5
#endif
#ifndef GL_REG_20
#define GL_REG_20                                                    0x00008935
#endif
#ifndef GL_COVERAGE_ALL_FRAGMENTS
#define GL_COVERAGE_ALL_FRAGMENTS                                    0x00008ED5
#endif
#ifndef GL_MATRIX16
#define GL_MATRIX16                                                  0x000088D0
#endif
#ifndef GL_PROGRAM_PIPELINE_BINDING
#define GL_PROGRAM_PIPELINE_BINDING                                  0x0000825A
#endif
#ifndef GL_MULTIVIEW
#define GL_MULTIVIEW                                                 0x000090F1
#endif
#ifndef GL_VERTEX_ARRAY_TYPE
#define GL_VERTEX_ARRAY_TYPE                                         0x0000807B
#endif
#ifndef GL_DOUBLE_VEC3
#define GL_DOUBLE_VEC3                                               0x00008FFD
#endif
#ifndef GL_PRESENT_DURATION
#define GL_PRESENT_DURATION                                          0x00008E2B
#endif
#ifndef GL_FILTER4
#define GL_FILTER4                                                   0x00008146
#endif
#ifndef GL_COMPRESSED_LUMINANCE
#define GL_COMPRESSED_LUMINANCE                                      0x000084EA
#endif
#ifndef GL_PRIMARY_COLOR
#define GL_PRIMARY_COLOR                                             0x00008577
#endif
#ifndef GL_HALF_BIAS_NEGATE
#define GL_HALF_BIAS_NEGATE                                          0x0000853B
#endif
#ifndef GL_RGB5
#define GL_RGB5                                                      0x00008050
#endif
#ifndef GL_MINUS
#define GL_MINUS                                                     0x0000929F
#endif
#ifndef GL_PIXEL_FRAGMENT_ALPHA_SOURCE
#define GL_PIXEL_FRAGMENT_ALPHA_SOURCE                               0x00008355
#endif
#ifndef GL_CON_6
#define GL_CON_6                                                     0x00008947
#endif
#ifndef GL_PIXEL_MAG_FILTER
#define GL_PIXEL_MAG_FILTER                                          0x00008331
#endif
#ifndef GL_LUMINANCE8UI
#define GL_LUMINANCE8UI                                              0x00008D80
#endif
#ifndef GL_INT_IMAGE_1D_ARRAY
#define GL_INT_IMAGE_1D_ARRAY                                        0x0000905D
#endif
#ifndef GL_USE_MISSING_GLYPH
#define GL_USE_MISSING_GLYPH                                         0x000090AA
#endif
#ifndef GL_ALPHA32F
#define GL_ALPHA32F                                                  0x00008816
#endif
#ifndef GL_FLOAT_MAT2
#define GL_FLOAT_MAT2                                                0x00008B5A
#endif
#ifndef GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y                               0x00008518
#endif
#ifndef GL_OFFSET_TEXTURE_2D
#define GL_OFFSET_TEXTURE_2D                                         0x000086E8
#endif
#ifndef GL_MAX_PROGRAM_TEXEL_OFFSET
#define GL_MAX_PROGRAM_TEXEL_OFFSET                                  0x00008905
#endif
#ifndef GL_422_REV_AVERAGE
#define GL_422_REV_AVERAGE                                           0x000080CF
#endif
#ifndef GL_PROXY_TEXTURE_2D_STACK
#define GL_PROXY_TEXTURE_2D_STACK                                    0x0000875C
#endif
#ifndef GL_VARIABLE_E
#define GL_VARIABLE_E                                                0x00008527
#endif
#ifndef GL_MAP2_VERTEX_ATTRIB5_4
#define GL_MAP2_VERTEX_ATTRIB5_4                                     0x00008675
#endif
#ifndef GL_MAT_SHININESS_BIT
#define GL_MAT_SHININESS_BIT                                         0x02000000
#endif
#ifndef GL_NORMAL_ARRAY_LIST
#define GL_NORMAL_ARRAY_LIST                                         0x0001929F
#endif
#ifndef GL_PIXEL_TILE_GRID_WIDTH
#define GL_PIXEL_TILE_GRID_WIDTH                                     0x00008142
#endif
#ifndef GL_INDEX_ARRAY_LIST_STRIDE
#define GL_INDEX_ARRAY_LIST_STRIDE                                   0x000192AB
#endif
#ifndef GL_COMBINER5
#define GL_COMBINER5                                                 0x00008555
#endif
#ifndef GL_OFFSET_TEXTURE_RECTANGLE
#define GL_OFFSET_TEXTURE_RECTANGLE                                  0x0000864C
#endif
#ifndef GL_RGBA_INTEGER_MODE
#define GL_RGBA_INTEGER_MODE                                         0x00008D9E
#endif
#ifndef GL_FULL_RANGE
#define GL_FULL_RANGE                                                0x000087E1
#endif
#ifndef GL_PATH_FILL_COVER_MODE
#define GL_PATH_FILL_COVER_MODE                                      0x00009082
#endif
#ifndef GL_FONT_X_MIN_BOUNDS_BIT
#define GL_FONT_X_MIN_BOUNDS_BIT                                     0x00010000
#endif
#ifndef GL_DT_SCALE
#define GL_DT_SCALE                                                  0x00008711
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY_STRIDE
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE                                0x00008624
#endif
#ifndef GL_UNSIGNED_INT_SAMPLER_3D
#define GL_UNSIGNED_INT_SAMPLER_3D                                   0x00008DD3
#endif
#ifndef GL_R16I
#define GL_R16I                                                      0x00008233
#endif
#ifndef GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2
#define GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2                         0x00008DBE
#endif
#ifndef GL_COLOR_TABLE_BIAS
#define GL_COLOR_TABLE_BIAS                                          0x000080D7
#endif
#ifndef GL_SOURCE0_RGB
#define GL_SOURCE0_RGB                                               0x00008580
#endif
#ifndef GL_QUERY
#define GL_QUERY                                                     0x000082E3
#endif
#ifndef GL_READ_PIXEL_DATA_RANGE_LENGTH
#define GL_READ_PIXEL_DATA_RANGE_LENGTH                              0x0000887B
#endif
#ifndef GL_CURRENT_TANGENT
#define GL_CURRENT_TANGENT                                           0x0000843B
#endif
#ifndef GL_TEXTURE_CUBE_MAP
#define GL_TEXTURE_CUBE_MAP                                          0x00008513
#endif
#ifndef GL_RGB16
#define GL_RGB16                                                     0x00008054
#endif
#ifndef GL_MITER_TRUNCATE
#define GL_MITER_TRUNCATE                                            0x000090A8
#endif
#ifndef GL_BOLD_BIT
#define GL_BOLD_BIT                                                  0x00000001
#endif
#ifndef GL_ATC_RGBA_INTERPOLATED_ALPHA
#define GL_ATC_RGBA_INTERPOLATED_ALPHA                               0x000087EE
#endif
#ifndef GL_MUL
#define GL_MUL                                                       0x00008964
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY5
#define GL_VERTEX_ATTRIB_ARRAY5                                      0x00008655
#endif
#ifndef GL_TRANSFORM_FEEDBACK
#define GL_TRANSFORM_FEEDBACK                                        0x00008E22
#endif
#ifndef GL_CON_13
#define GL_CON_13                                                    0x0000894E
#endif
#ifndef GL_IMAGE_SCALE_Y
#define GL_IMAGE_SCALE_Y                                             0x00008156
#endif
#ifndef GL_3DC_X
#define GL_3DC_X                                                     0x000087F9
#endif
#ifndef GL_NORMALIZED_RANGE
#define GL_NORMALIZED_RANGE                                          0x000087E0
#endif
#ifndef GL_ATTRIB_ARRAY_TYPE
#define GL_ATTRIB_ARRAY_TYPE                                         0x00008625
#endif
#ifndef GL_SIGNED_RGBA
#define GL_SIGNED_RGBA                                               0x000086FB
#endif
#ifndef GL_TEXTURE_BLUE_SIZE
#define GL_TEXTURE_BLUE_SIZE                                         0x0000805E
#endif
#ifndef GL_SYNC_CL_EVENT_COMPLETE
#define GL_SYNC_CL_EVENT_COMPLETE                                    0x00008241
#endif
#ifndef GL_COMBINER_AB_DOT_PRODUCT
#define GL_COMBINER_AB_DOT_PRODUCT                                   0x00008545
#endif
#ifndef GL_TEXTURE_STORAGE_HINT
#define GL_TEXTURE_STORAGE_HINT                                      0x000085BC
#endif
#ifndef GL_MAX_TEXTURE_UNITS
#define GL_MAX_TEXTURE_UNITS                                         0x000084E2
#endif
#ifndef GL_TEXTURE_IMMUTABLE_LEVELS
#define GL_TEXTURE_IMMUTABLE_LEVELS                                  0x000082DF
#endif
#ifndef GL_FLOAT_VEC3
#define GL_FLOAT_VEC3                                                0x00008B51
#endif
#ifndef GL_COMPRESSED_RGB_S3TC_DXT1
#define GL_COMPRESSED_RGB_S3TC_DXT1                                  0x000083F0
#endif
#ifndef GL_FLOAT_MAT2x4
#define GL_FLOAT_MAT2x4                                              0x00008B66
#endif
#ifndef GL_PATH_GEN_COMPONENTS
#define GL_PATH_GEN_COMPONENTS                                       0x000090B3
#endif
#ifndef GL_MAT_COLOR_INDEXES_BIT
#define GL_MAT_COLOR_INDEXES_BIT                                     0x01000000
#endif
#ifndef GL_BOOL_VEC3
#define GL_BOOL_VEC3                                                 0x00008B58
#endif
#ifndef GL_NUM_VIDEO_CAPTURE_STREAMS
#define GL_NUM_VIDEO_CAPTURE_STREAMS                                 0x00009024
#endif
#ifndef GL_MAX_PROGRAM_ALU_INSTRUCTIONS
#define GL_MAX_PROGRAM_ALU_INSTRUCTIONS                              0x0000880B
#endif
#ifndef GL_REG_8
#define GL_REG_8                                                     0x00008929
#endif
#ifndef GL_BINNING_CONTROL_HINT
#define GL_BINNING_CONTROL_HINT                                      0x00008FB0
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5                         0x000093D8
#endif
#ifndef GL_TEXTURE_ALPHA_TYPE
#define GL_TEXTURE_ALPHA_TYPE                                        0x00008C13
#endif
#ifndef GL_IMAGE_ROTATE_ORIGIN_Y
#define GL_IMAGE_ROTATE_ORIGIN_Y                                     0x0000815B
#endif
#ifndef GL_PATH_FILL_BOUNDING_BOX
#define GL_PATH_FILL_BOUNDING_BOX                                    0x000090A1
#endif
#ifndef GL_PALETTE8_RGBA8
#define GL_PALETTE8_RGBA8                                            0x00008B96
#endif
#ifndef GL_EXPAND_NORMAL
#define GL_EXPAND_NORMAL                                             0x00008538
#endif
#ifndef GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS
#define GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS             0x00008F39
#endif
#ifndef GL_DRAW_BUFFER7
#define GL_DRAW_BUFFER7                                              0x0000882C
#endif
#ifndef GL_RG8_SNORM
#define GL_RG8_SNORM                                                 0x00008F95
#endif
#ifndef GL_GLYPH_HORIZONTAL_BEARING_ADVANCE_BIT
#define GL_GLYPH_HORIZONTAL_BEARING_ADVANCE_BIT                      0x00000010
#endif
#ifndef GL_UNIFORM_BLOCK_BINDING
#define GL_UNIFORM_BLOCK_BINDING                                     0x00008A3F
#endif
#ifndef GL_COLOR_CLEAR_UNCLAMPED_VALUE
#define GL_COLOR_CLEAR_UNCLAMPED_VALUE                               0x00008835
#endif
#ifndef GL_R1UI_T2F_N3F_V3F
#define GL_R1UI_T2F_N3F_V3F                                          0x000085CA
#endif
#ifndef GL_SIGNED_RGB8
#define GL_SIGNED_RGB8                                               0x000086FF
#endif
#ifndef GL_STENCIL_BUFFER_BIT0
#define GL_STENCIL_BUFFER_BIT0                                       0x00010000
#endif
#ifndef GL_SAMPLER_BUFFER
#define GL_SAMPLER_BUFFER                                            0x00009001
#endif
#ifndef GL_DOT4
#define GL_DOT4                                                      0x00008967
#endif
#ifndef GL_OFFSET_TEXTURE_MATRIX
#define GL_OFFSET_TEXTURE_MATRIX                                     0x000086E1
#endif
#ifndef GL_PERFMON_RESULT
#define GL_PERFMON_RESULT                                            0x00008BC6
#endif
#ifndef GL_COLOR_INDEX12
#define GL_COLOR_INDEX12                                             0x000080E6
#endif
#ifndef GL_FONT_Y_MAX_BOUNDS_BIT
#define GL_FONT_Y_MAX_BOUNDS_BIT                                     0x00080000
#endif
#ifndef GL_MAP1_VERTEX_ATTRIB0_4
#define GL_MAP1_VERTEX_ATTRIB0_4                                     0x00008660
#endif
#ifndef GL_MAP2_BINORMAL
#define GL_MAP2_BINORMAL                                             0x00008447
#endif
#ifndef GL_MODELVIEW13
#define GL_MODELVIEW13                                               0x0000872D
#endif
#ifndef GL_MAX_VERTEX_SHADER_LOCALS
#define GL_MAX_VERTEX_SHADER_LOCALS                                  0x000087C9
#endif
#ifndef GL_COMBINER_CD_DOT_PRODUCT
#define GL_COMBINER_CD_DOT_PRODUCT                                   0x00008546
#endif
#ifndef GL_BLEND_EQUATION_RGB
#define GL_BLEND_EQUATION_RGB                                        0x00008009
#endif
#ifndef GL_STENCIL_INDEX1
#define GL_STENCIL_INDEX1                                            0x00008D46
#endif
#ifndef GL_DEPTH_CLAMP_FAR
#define GL_DEPTH_CLAMP_FAR                                           0x0000901F
#endif
#ifndef GL_LUMINANCE_FLOAT16
#define GL_LUMINANCE_FLOAT16                                         0x0000881E
#endif
#ifndef GL_UNSIGNED_SHORT_4_4_4_4_REV
#define GL_UNSIGNED_SHORT_4_4_4_4_REV                                0x00008365
#endif
#ifndef GL_CULL_VERTEX_EYE_POSITION
#define GL_CULL_VERTEX_EYE_POSITION                                  0x000081AB
#endif
#ifndef GL_MAP1_VERTEX_ATTRIB13_4
#define GL_MAP1_VERTEX_ATTRIB13_4                                    0x0000866D
#endif
#ifndef GL_INT_SAMPLER_CUBE
#define GL_INT_SAMPLER_CUBE                                          0x00008DCC
#endif
#ifndef GL_PROGRAM_BINDING
#define GL_PROGRAM_BINDING                                           0x00008677
#endif
#ifndef GL_SECONDARY_COLOR_ARRAY_TYPE
#define GL_SECONDARY_COLOR_ARRAY_TYPE                                0x0000845B
#endif
#ifndef GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS
#define GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS                           0x000088A3
#endif
#ifndef GL_ELEMENT_ARRAY_BUFFER_BINDING
#define GL_ELEMENT_ARRAY_BUFFER_BINDING                              0x00008895
#endif
#ifndef GL_BGRA8
#define GL_BGRA8                                                     0x000093A1
#endif
#ifndef GL_STENCIL_BACK_OP_VALUE
#define GL_STENCIL_BACK_OP_VALUE                                     0x0000874D
#endif
#ifndef GL_INT8_VEC3
#define GL_INT8_VEC3                                                 0x00008FE2
#endif
#ifndef GL_RENDERBUFFER_SAMPLES
#define GL_RENDERBUFFER_SAMPLES                                      0x00008CAB
#endif
#ifndef GL_MIRROR_CLAMP_TO_BORDER
#define GL_MIRROR_CLAMP_TO_BORDER                                    0x00008912
#endif
#ifndef GL_SYNC_X11_FENCE
#define GL_SYNC_X11_FENCE                                            0x000090E1
#endif
#ifndef GL_EDGE_FLAG_ARRAY
#define GL_EDGE_FLAG_ARRAY                                           0x00008079
#endif
#ifndef GL_CLIP_DISTANCE
#define GL_CLIP_DISTANCE                                             0x00008C7A
#endif
#ifndef GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING                     0x00008210
#endif
#ifndef GL_DEBUG_CATEGORY_API_ERROR
#define GL_DEBUG_CATEGORY_API_ERROR                                  0x00009149
#endif
#ifndef GL_NUM_GENERAL_COMBINERS
#define GL_NUM_GENERAL_COMBINERS                                     0x0000854E
#endif
#ifndef GL_COVERAGE_COMPONENT
#define GL_COVERAGE_COMPONENT                                        0x00008ED0
#endif
#ifndef GL_MATRIX1
#define GL_MATRIX1                                                   0x000088C1
#endif
#ifndef GL_INTERLACE_READ
#define GL_INTERLACE_READ                                            0x00008568
#endif
#ifndef GL_MAP1_VERTEX_ATTRIB9_4
#define GL_MAP1_VERTEX_ATTRIB9_4                                     0x00008669
#endif
#ifndef GL_DEPTH_BUFFER_BIT7
#define GL_DEPTH_BUFFER_BIT7                                         0x00008000
#endif
#ifndef GL_IMAGE_2D_ARRAY
#define GL_IMAGE_2D_ARRAY                                            0x00009053
#endif
#ifndef GL_QUERY_NO_WAIT
#define GL_QUERY_NO_WAIT                                             0x00008E14
#endif
#ifndef GL_MAP2_TANGENT
#define GL_MAP2_TANGENT                                              0x00008445
#endif
#ifndef GL_HISTOGRAM_WIDTH
#define GL_HISTOGRAM_WIDTH                                           0x00008026
#endif
#ifndef GL_INTENSITY16I
#define GL_INTENSITY16I                                              0x00008D8B
#endif
#ifndef GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL                      0x00008CD2
#endif
#ifndef GL_MAX_IMAGE_UNITS
#define GL_MAX_IMAGE_UNITS                                           0x00008F38
#endif
#ifndef GL_DOUBLE_MAT3
#define GL_DOUBLE_MAT3                                               0x00008F47
#endif
#ifndef GL_TEXTURE_3D
#define GL_TEXTURE_3D                                                0x0000806F
#endif
#ifndef GL_PIXEL_TILE_BEST_ALIGNMENT
#define GL_PIXEL_TILE_BEST_ALIGNMENT                                 0x0000813E
#endif
#ifndef GL_DUAL_INTENSITY4
#define GL_DUAL_INTENSITY4                                           0x00008118
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD27
#define GL_OUTPUT_TEXTURE_COORD27                                    0x000087B8
#endif
#ifndef GL_DUAL_LUMINANCE12
#define GL_DUAL_LUMINANCE12                                          0x00008116
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_8x6
#define GL_COMPRESSED_RGBA_ASTC_8x6                                  0x000093B6
#endif
#ifndef GL_LAYOUT_LINEAR_CPU_CACHED
#define GL_LAYOUT_LINEAR_CPU_CACHED                                  0x00000002
#endif
#ifndef GL_LINEAR_CLIPMAP_LINEAR
#define GL_LINEAR_CLIPMAP_LINEAR                                     0x00008170
#endif
#ifndef GL_STACK_OVERFLOW
#define GL_STACK_OVERFLOW                                            0x00000503
#endif
#ifndef GL_TEXTURE_COMPARE_FAIL_VALUE
#define GL_TEXTURE_COMPARE_FAIL_VALUE                                0x000080BF
#endif
#ifndef GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT                        0x00008DA9
#endif
#ifndef GL_MAX_ELEMENTS_INDICES
#define GL_MAX_ELEMENTS_INDICES                                      0x000080E9
#endif
#ifndef GL_DUAL_LUMINANCE4
#define GL_DUAL_LUMINANCE4                                           0x00008114
#endif
#ifndef GL_DOT2_ADD
#define GL_DOT2_ADD                                                  0x0000896C
#endif
#ifndef GL_PACK_SKIP_VOLUMES
#define GL_PACK_SKIP_VOLUMES                                         0x00008130
#endif
#ifndef GL_RGBA16
#define GL_RGBA16                                                    0x0000805B
#endif
#ifndef GL_TEXTURE1
#define GL_TEXTURE1                                                  0x000084C1
#endif
#ifndef GL_SWIZZLE_STR
#define GL_SWIZZLE_STR                                               0x00008976
#endif
#ifndef GL_MODELVIEW4
#define GL_MODELVIEW4                                                0x00008724
#endif
#ifndef GL_QUERY_BUFFER_BINDING
#define GL_QUERY_BUFFER_BINDING                                      0x00009193
#endif
#ifndef GL_BUFFER_USAGE
#define GL_BUFFER_USAGE                                              0x00008765
#endif
#ifndef GL_FENCE
#define GL_FENCE                                                     0x00008A0B
#endif
#ifndef GL_MATRIX19
#define GL_MATRIX19                                                  0x000088D3
#endif
#ifndef GL_SAMPLER_OBJECT
#define GL_SAMPLER_OBJECT                                            0x00009155
#endif
#ifndef GL_STREAM_READ
#define GL_STREAM_READ                                               0x000088E1
#endif
#ifndef GL_LUMINANCE_ALPHA16F
#define GL_LUMINANCE_ALPHA16F                                        0x0000881F
#endif
#ifndef GL_INVERT_RGB
#define GL_INVERT_RGB                                                0x000092A3
#endif
#ifndef GL_PIXEL_SUBSAMPLE_4242
#define GL_PIXEL_SUBSAMPLE_4242                                      0x000085A4
#endif
#ifndef GL_MAX_MATRIX_PALETTE_STACK_DEPTH
#define GL_MAX_MATRIX_PALETTE_STACK_DEPTH                            0x00008841
#endif
#ifndef GL_CON_20
#define GL_CON_20                                                    0x00008955
#endif
#ifndef GL_TEXTURE_DEFORMATION_BIT
#define GL_TEXTURE_DEFORMATION_BIT                                   0x00000001
#endif
#ifndef GL_POST_COLOR_MATRIX_RED_SCALE
#define GL_POST_COLOR_MATRIX_RED_SCALE                               0x000080B4
#endif
#ifndef GL_PATH_COMPUTED_LENGTH
#define GL_PATH_COMPUTED_LENGTH                                      0x000090A0
#endif
#ifndef GL_QUAD_INTENSITY8
#define GL_QUAD_INTENSITY8                                           0x00008123
#endif
#ifndef GL_STATIC_VERTEX_ARRAY
#define GL_STATIC_VERTEX_ARRAY                                       0x00019295
#endif
#ifndef GL_PROGRAM_PARAMETER
#define GL_PROGRAM_PARAMETER                                         0x00008644
#endif
#ifndef GL_DEPTH_BUFFER_BIT4
#define GL_DEPTH_BUFFER_BIT4                                         0x00001000
#endif
#ifndef GL_COMMAND_BARRIER_BIT
#define GL_COMMAND_BARRIER_BIT                                       0x00000040
#endif
#ifndef GL_DEPTH_COMPONENT16
#define GL_DEPTH_COMPONENT16                                         0x000081A5
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3                        0x000093E0
#endif
#ifndef GL_RGBA8UI
#define GL_RGBA8UI                                                   0x00008D7C
#endif
#ifndef GL_NORMAL_ARRAY_POINTER
#define GL_NORMAL_ARRAY_POINTER                                      0x0000808F
#endif
#ifndef GL_STENCIL_TAG_BITS
#define GL_STENCIL_TAG_BITS                                          0x000088F2
#endif
#ifndef GL_PROGRAM_BINARY_FORMATS
#define GL_PROGRAM_BINARY_FORMATS                                    0x000087FF
#endif
#ifndef GL_PROXY_TEXTURE_COLOR_TABLE
#define GL_PROXY_TEXTURE_COLOR_TABLE                                 0x000080BD
#endif
#ifndef GL_UNIFORM_BUFFER_SIZE
#define GL_UNIFORM_BUFFER_SIZE                                       0x00008A2A
#endif
#ifndef GL_PIXEL_COUNTER_BITS
#define GL_PIXEL_COUNTER_BITS                                        0x00008864
#endif
#ifndef GL_COLOR_ATTACHMENT1
#define GL_COLOR_ATTACHMENT1                                         0x00008CE1
#endif
#ifndef GL_UNIFORM_BUFFER
#define GL_UNIFORM_BUFFER                                            0x00008A11
#endif
#ifndef GL_TEXTURE0
#define GL_TEXTURE0                                                  0x000084C0
#endif
#ifndef GL_DOT_PRODUCT_PASS_THROUGH
#define GL_DOT_PRODUCT_PASS_THROUGH                                  0x0000885B
#endif
#ifndef GL_TEXTURE_MEMORY_LAYOUT
#define GL_TEXTURE_MEMORY_LAYOUT                                     0x000083FF
#endif
#ifndef GL_RGBA4
#define GL_RGBA4                                                     0x00008056
#endif
#ifndef GL_SRGB_ALPHA
#define GL_SRGB_ALPHA                                                0x00008C42
#endif
#ifndef GL_GENERATE_MIPMAP
#define GL_GENERATE_MIPMAP                                           0x00008191
#endif
#ifndef GL_STATIC_READ
#define GL_STATIC_READ                                               0x000088E5
#endif
#ifndef GL_CONTINUOUS
#define GL_CONTINUOUS                                                0x00009007
#endif
#ifndef GL_COLOR_ARRAY_POINTER
#define GL_COLOR_ARRAY_POINTER                                       0x00008090
#endif
#ifndef GL_EVAL_VERTEX_ATTRIB6
#define GL_EVAL_VERTEX_ATTRIB6                                       0x000086CC
#endif
#ifndef GL_FRAGMENT_PROGRAM_INTERPOLATION_OFFSET_BITS
#define GL_FRAGMENT_PROGRAM_INTERPOLATION_OFFSET_BITS                0x00008E5D
#endif
#ifndef GL_MAX_LABEL_LENGTH
#define GL_MAX_LABEL_LENGTH                                          0x000082E8
#endif
#ifndef GL_VERTEX_SHADER_VARIANTS
#define GL_VERTEX_SHADER_VARIANTS                                    0x000087D0
#endif
#ifndef GL_COLOR_TABLE_ALPHA_SIZE
#define GL_COLOR_TABLE_ALPHA_SIZE                                    0x000080DD
#endif
#ifndef GL_FIRST_VERTEX_CONVENTION
#define GL_FIRST_VERTEX_CONVENTION                                   0x00008E4D
#endif
#ifndef GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE
#define GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE                          0x00008E24
#endif
#ifndef GL_TEXTURE_SHADER
#define GL_TEXTURE_SHADER                                            0x000086DE
#endif
#ifndef GL_BUFFER
#define GL_BUFFER                                                    0x000082E0
#endif
#ifndef GL_UNPACK_CLIENT_STORAGE
#define GL_UNPACK_CLIENT_STORAGE                                     0x000085B2
#endif
#ifndef GL_BUFFER_MAP_POINTER
#define GL_BUFFER_MAP_POINTER                                        0x000088BD
#endif
#ifndef GL_TEXTURE_IMAGE_VALID
#define GL_TEXTURE_IMAGE_VALID                                       0x00008BD8
#endif
#ifndef GL_MAX_DEEP_3D_TEXTURE_WIDTH_HEIGHT
#define GL_MAX_DEEP_3D_TEXTURE_WIDTH_HEIGHT                          0x000090D0
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3                        0x000093E2
#endif
#ifndef GL_LINE_TO
#define GL_LINE_TO                                                   0x00000004
#endif
#ifndef GL_SRC_OVER
#define GL_SRC_OVER                                                  0x00009288
#endif
#ifndef GL_CLAMP_READ_COLOR
#define GL_CLAMP_READ_COLOR                                          0x0000891C
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY_INTEGER
#define GL_VERTEX_ATTRIB_ARRAY_INTEGER                               0x000088FD
#endif
#ifndef GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS                      0x00008DA8
#endif
#ifndef GL_SECONDARY_COLOR_ARRAY_ADDRESS
#define GL_SECONDARY_COLOR_ARRAY_ADDRESS                             0x00008F27
#endif
#ifndef GL_UNPACK_SKIP_ROWS
#define GL_UNPACK_SKIP_ROWS                                          0x00000CF3
#endif
#ifndef GL_BLEND_COLOR
#define GL_BLEND_COLOR                                               0x00008005
#endif
#ifndef GL_TRACK_MATRIX
#define GL_TRACK_MATRIX                                              0x00008648
#endif
#ifndef GL_UNIFORM_BUFFER_START
#define GL_UNIFORM_BUFFER_START                                      0x00008A29
#endif
#ifndef GL_LUMINANCE6_ALPHA2
#define GL_LUMINANCE6_ALPHA2                                         0x00008044
#endif
#ifndef GL_MODULATE_SUBTRACT
#define GL_MODULATE_SUBTRACT                                         0x00008746
#endif
#ifndef GL_FUNC_SUBTRACT
#define GL_FUNC_SUBTRACT                                             0x0000800A
#endif
#ifndef GL_UNPACK_SUBSAMPLE_RATE
#define GL_UNPACK_SUBSAMPLE_RATE                                     0x000085A1
#endif
#ifndef GL_DEBUG_CATEGORY_DEPRECATION
#define GL_DEBUG_CATEGORY_DEPRECATION                                0x0000914B
#endif
#ifndef GL_TRANSFORM_FEEDBACK_BUFFER_BINDING
#define GL_TRANSFORM_FEEDBACK_BUFFER_BINDING                         0x00008C8F
#endif
#ifndef GL_VIDEO_BUFFER
#define GL_VIDEO_BUFFER                                              0x00009020
#endif
#ifndef GL_RGB5_A1
#define GL_RGB5_A1                                                   0x00008057
#endif
#ifndef GL_READ_FRAMEBUFFER_BINDING
#define GL_READ_FRAMEBUFFER_BINDING                                  0x00008CAA
#endif
#ifndef GL_SAMPLE_ALPHA_TO_MASK
#define GL_SAMPLE_ALPHA_TO_MASK                                      0x0000809E
#endif
#ifndef GL_MODELVIEW0_MATRIX
#define GL_MODELVIEW0_MATRIX                                         0x00000BA6
#endif
#ifndef GL_CULL_FRAGMENT
#define GL_CULL_FRAGMENT                                             0x000086E7
#endif
#ifndef GL_CON_18
#define GL_CON_18                                                    0x00008953
#endif
#ifndef GL_Y
#define GL_Y                                                         0x000087D6
#endif
#ifndef GL_DEBUG_SOURCE_OTHER
#define GL_DEBUG_SOURCE_OTHER                                        0x0000824B
#endif
#ifndef GL_RENDERBUFFER_BLUE_SIZE
#define GL_RENDERBUFFER_BLUE_SIZE                                    0x00008D52
#endif
#ifndef GL_CLIENT_ACTIVE_TEXTURE
#define GL_CLIENT_ACTIVE_TEXTURE                                     0x000084E1
#endif
#ifndef GL_FLOAT16_VEC3
#define GL_FLOAT16_VEC3                                              0x00008FFA
#endif
#ifndef GL_SRGB8
#define GL_SRGB8                                                     0x00008C41
#endif
#ifndef GL_PROGRAM_RESULT_COMPONENTS
#define GL_PROGRAM_RESULT_COMPONENTS                                 0x00008907
#endif
#ifndef GL_COLOR_ARRAY_LENGTH
#define GL_COLOR_ARRAY_LENGTH                                        0x00008F2D
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_12x10
#define GL_COMPRESSED_RGBA_ASTC_12x10                                0x000093BC
#endif
#ifndef GL_BUFFER_MAP_LENGTH
#define GL_BUFFER_MAP_LENGTH                                         0x00009120
#endif
#ifndef GL_LUMINANCE12_ALPHA12
#define GL_LUMINANCE12_ALPHA12                                       0x00008047
#endif
#ifndef GL_OP_SUB
#define GL_OP_SUB                                                    0x00008796
#endif
#ifndef GL_UNSIGNED_INT64_VEC4
#define GL_UNSIGNED_INT64_VEC4                                       0x00008FF7
#endif
#ifndef GL_BUFFER_FLUSHING_UNMAP
#define GL_BUFFER_FLUSHING_UNMAP                                     0x00008A13
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD9
#define GL_OUTPUT_TEXTURE_COORD9                                     0x000087A6
#endif
#ifndef GL_VERTEX_STREAM3
#define GL_VERTEX_STREAM3                                            0x0000876F
#endif
#ifndef GL_FRAGMENT_LIGHT0
#define GL_FRAGMENT_LIGHT0                                           0x0000840C
#endif
#ifndef GL_INT_2_10_10_10_REV
#define GL_INT_2_10_10_10_REV                                        0x00008D9F
#endif
#ifndef GL_UNSIGNED_INT16
#define GL_UNSIGNED_INT16                                            0x00008FF0
#endif
#ifndef GL_NORMAL_ARRAY
#define GL_NORMAL_ARRAY                                              0x00008075
#endif
#ifndef GL_TEXTURE_LUMINANCE_TYPE
#define GL_TEXTURE_LUMINANCE_TYPE                                    0x00008C14
#endif
#ifndef GL_INT_IMAGE_CUBE_MAP_ARRAY
#define GL_INT_IMAGE_CUBE_MAP_ARRAY                                  0x0000905F
#endif
#ifndef GL_PERCENTAGE
#define GL_PERCENTAGE                                                0x00008BC3
#endif
#ifndef GL_FRAGMENT_LIGHTING
#define GL_FRAGMENT_LIGHTING                                         0x00008400
#endif
#ifndef GL_INTENSITY16UI
#define GL_INTENSITY16UI                                             0x00008D79
#endif
#ifndef GL_COLOR_ATTACHMENT11
#define GL_COLOR_ATTACHMENT11                                        0x00008CEB
#endif
#ifndef GL_PINLIGHT
#define GL_PINLIGHT                                                  0x000092A8
#endif
#ifndef GL_TEXTURE_FILTER_CONTROL
#define GL_TEXTURE_FILTER_CONTROL                                    0x00008500
#endif
#ifndef GL_VERTEX23_BIT
#define GL_VERTEX23_BIT                                              0x00000004
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY1
#define GL_VERTEX_ATTRIB_ARRAY1                                      0x00008651
#endif
#ifndef GL_R8_SNORM
#define GL_R8_SNORM                                                  0x00008F94
#endif
#ifndef GL_POINT_SPRITE_R_MODE
#define GL_POINT_SPRITE_R_MODE                                       0x00008863
#endif
#ifndef GL_MATRIX26
#define GL_MATRIX26                                                  0x000088DA
#endif
#ifndef GL_READ_ONLY
#define GL_READ_ONLY                                                 0x000088B8
#endif
#ifndef GL_ATC_RGBA_EXPLICIT_ALPHA
#define GL_ATC_RGBA_EXPLICIT_ALPHA                                   0x00008C93
#endif
#ifndef GL_SWIZZLE_STRQ
#define GL_SWIZZLE_STRQ                                              0x0000897A
#endif
#ifndef GL_DOT_PRODUCT_TEXTURE_CUBE_MAP
#define GL_DOT_PRODUCT_TEXTURE_CUBE_MAP                              0x000086F0
#endif
#ifndef GL_ALPHA_TEST
#define GL_ALPHA_TEST                                                0x00000BC0
#endif
#ifndef GL_PLUS_CLAMPED_ALPHA
#define GL_PLUS_CLAMPED_ALPHA                                        0x000092B2
#endif
#ifndef GL_SHADING_LANGUAGE_VERSION
#define GL_SHADING_LANGUAGE_VERSION                                  0x00008B8C
#endif
#ifndef GL_RELATIVE_LINE_TO
#define GL_RELATIVE_LINE_TO                                          0x00000005
#endif
#ifndef GL_GEOMETRY_VERTICES_OUT
#define GL_GEOMETRY_VERTICES_OUT                                     0x00008DDA
#endif
#ifndef GL_MAP2_VERTEX_ATTRIB10_4
#define GL_MAP2_VERTEX_ATTRIB10_4                                    0x0000867A
#endif
#ifndef GL_CONVEX_HULL
#define GL_CONVEX_HULL                                               0x0000908B
#endif
#ifndef GL_FOG_COORDINATE
#define GL_FOG_COORDINATE                                            0x00008451
#endif
#ifndef GL_MATRIX21
#define GL_MATRIX21                                                  0x000088D5
#endif
#ifndef GL_VERTEX_ATTRIB_MAP1_COEFF
#define GL_VERTEX_ATTRIB_MAP1_COEFF                                  0x00008A03
#endif
#ifndef GL_TEXTURE_2D_BINDING
#define GL_TEXTURE_2D_BINDING                                        0x00008069
#endif
#ifndef GL_COMBINER_COMPONENT_USAGE
#define GL_COMBINER_COMPONENT_USAGE                                  0x00008544
#endif
#ifndef GL_UNSIGNED_INT64_VEC2
#define GL_UNSIGNED_INT64_VEC2                                       0x00008FF5
#endif
#ifndef GL_ARC_TO
#define GL_ARC_TO                                                    0x000000FE
#endif
#ifndef GL_EYE_POINT
#define GL_EYE_POINT                                                 0x000081F4
#endif
#ifndef GL_SECONDARY_COLOR_ARRAY_STRIDE
#define GL_SECONDARY_COLOR_ARRAY_STRIDE                              0x0000845C
#endif
#ifndef GL_RELATIVE_HORIZONTAL_LINE_TO
#define GL_RELATIVE_HORIZONTAL_LINE_TO                               0x00000007
#endif
#ifndef GL_SPRITE_TRANSLATION
#define GL_SPRITE_TRANSLATION                                        0x0000814B
#endif
#ifndef GL_VIRTUAL_PAGE_SIZE_Y
#define GL_VIRTUAL_PAGE_SIZE_Y                                       0x00009196
#endif
#ifndef GL_SUBSAMPLE_DISTANCE
#define GL_SUBSAMPLE_DISTANCE                                        0x0000883F
#endif
#ifndef GL_FRAMEBUFFER_BARRIER_BIT
#define GL_FRAMEBUFFER_BARRIER_BIT                                   0x00000400
#endif
#ifndef GL_SPRITE
#define GL_SPRITE                                                    0x00008148
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD31
#define GL_OUTPUT_TEXTURE_COORD31                                    0x000087BC
#endif
#ifndef GL_COMPRESSED_SIGNED_RG11_EAC
#define GL_COMPRESSED_SIGNED_RG11_EAC                                0x00009273
#endif
#ifndef GL_CON_29
#define GL_CON_29                                                    0x0000895E
#endif
#ifndef GL_RGB_FLOAT16
#define GL_RGB_FLOAT16                                               0x0000881B
#endif
#ifndef GL_MODELVIEW21
#define GL_MODELVIEW21                                               0x00008735
#endif
#ifndef GL_WEIGHT_ARRAY_POINTER
#define GL_WEIGHT_ARRAY_POINTER                                      0x000086AC
#endif
#ifndef GL_CONTEXT_ROBUST_ACCESS
#define GL_CONTEXT_ROBUST_ACCESS                                     0x000090F3
#endif
#ifndef GL_DUAL_TEXTURE_SELECT
#define GL_DUAL_TEXTURE_SELECT                                       0x00008124
#endif
#ifndef GL_COLOR_ATTACHMENT4
#define GL_COLOR_ATTACHMENT4                                         0x00008CE4
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5                        0x000093E8
#endif
#ifndef GL_R1UI_V3F
#define GL_R1UI_V3F                                                  0x000085C4
#endif
#ifndef GL_LINES_ADJACENCY
#define GL_LINES_ADJACENCY                                           0x0000000A
#endif
#ifndef GL_PERTURB
#define GL_PERTURB                                                   0x000085AE
#endif
#ifndef GL_PATH_OBJECT_BOUNDING_BOX
#define GL_PATH_OBJECT_BOUNDING_BOX                                  0x0000908A
#endif
#ifndef GL_TEXTURE_BINDING_3D
#define GL_TEXTURE_BINDING_3D                                        0x0000806A
#endif
#ifndef GL_8X_BIT
#define GL_8X_BIT                                                    0x00000004
#endif
#ifndef GL_MAX_DEFORMATION_ORDER
#define GL_MAX_DEFORMATION_ORDER                                     0x00008197
#endif
#ifndef GL_CURRENT_MATRIX_STACK_DEPTH
#define GL_CURRENT_MATRIX_STACK_DEPTH                                0x00008640
#endif
#ifndef GL_COMPRESSED_RGBA
#define GL_COMPRESSED_RGBA                                           0x000084EE
#endif
#ifndef GL_MIRROR_CLAMP
#define GL_MIRROR_CLAMP                                              0x00008742
#endif
#ifndef GL_TEXTURE_UPDATE_BARRIER_BIT
#define GL_TEXTURE_UPDATE_BARRIER_BIT                                0x00000100
#endif
#ifndef GL_FLOAT_R32
#define GL_FLOAT_R32                                                 0x00008885
#endif
#ifndef GL_COLOR_BUFFER_BIT2
#define GL_COLOR_BUFFER_BIT2                                         0x00000004
#endif
#ifndef GL_MODELVIEW_PROJECTION
#define GL_MODELVIEW_PROJECTION                                      0x00008629
#endif
#ifndef GL_FRAGMENT_LIGHT7
#define GL_FRAGMENT_LIGHT7                                           0x00008413
#endif
#ifndef GL_VIDEO_BUFFER_INTERNAL_FORMAT
#define GL_VIDEO_BUFFER_INTERNAL_FORMAT                              0x0000902D
#endif
#ifndef GL_ONE_MINUS_CONSTANT_COLOR
#define GL_ONE_MINUS_CONSTANT_COLOR                                  0x00008002
#endif
#ifndef GL_COMBINER_SCALE
#define GL_COMBINER_SCALE                                            0x00008548
#endif
#ifndef GL_COMBINE_ALPHA
#define GL_COMBINE_ALPHA                                             0x00008572
#endif
#ifndef GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE                     0x00008211
#endif
#ifndef GL_ABGR
#define GL_ABGR                                                      0x00008000
#endif
#ifndef GL_POST_CONVOLUTION_GREEN_SCALE
#define GL_POST_CONVOLUTION_GREEN_SCALE                              0x0000801D
#endif
#ifndef GL_PERFQUERY_COUNTER_EVENT
#define GL_PERFQUERY_COUNTER_EVENT                                   0x000094F0
#endif
#ifndef GL_TEXTURE22
#define GL_TEXTURE22                                                 0x000084D6
#endif
#ifndef GL_CONTRAST
#define GL_CONTRAST                                                  0x000092A1
#endif
#ifndef GL_ADD
#define GL_ADD                                                       0x00008963
#endif
#ifndef GL_INTENSITY16
#define GL_INTENSITY16                                               0x0000804D
#endif
#ifndef GL_DEPTH_BUFFER_BIT0
#define GL_DEPTH_BUFFER_BIT0                                         0x00000100
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD18
#define GL_OUTPUT_TEXTURE_COORD18                                    0x000087AF
#endif
#ifndef GL_MATRIX_INDEX_ARRAY_TYPE
#define GL_MATRIX_INDEX_ARRAY_TYPE                                   0x00008847
#endif
#ifndef GL_TIME_ELAPSED
#define GL_TIME_ELAPSED                                              0x000088BF
#endif
#ifndef GL_OCCLUSION_TEST
#define GL_OCCLUSION_TEST                                            0x00008165
#endif
#ifndef GL_TEXTURE_RENDERBUFFER_DATA_STORE_BINDING
#define GL_TEXTURE_RENDERBUFFER_DATA_STORE_BINDING                   0x00008E54
#endif
#ifndef GL_MAP1_VERTEX_ATTRIB3_4
#define GL_MAP1_VERTEX_ATTRIB3_4                                     0x00008663
#endif
#ifndef GL_TEXTURE_RANGE_LENGTH
#define GL_TEXTURE_RANGE_LENGTH                                      0x000085B7
#endif
#ifndef GL_TEXTURE_COVERAGE_SAMPLES
#define GL_TEXTURE_COVERAGE_SAMPLES                                  0x00009045
#endif
#ifndef GL_DETAIL_TEXTURE_FUNC_POINTS
#define GL_DETAIL_TEXTURE_FUNC_POINTS                                0x0000809C
#endif
#ifndef GL_IMAGE_ROTATE_ANGLE
#define GL_IMAGE_ROTATE_ANGLE                                        0x00008159
#endif
#ifndef GL_MAX_PROGRAM_NATIVE_TEMPORARIES
#define GL_MAX_PROGRAM_NATIVE_TEMPORARIES                            0x000088A7
#endif
#ifndef GL_PERFMON_RESULT_SIZE
#define GL_PERFMON_RESULT_SIZE                                       0x00008BC5
#endif
#ifndef GL_TEXTURE_CLIPMAP_LOD_OFFSET
#define GL_TEXTURE_CLIPMAP_LOD_OFFSET                                0x00008175
#endif
#ifndef GL_RECLAIM_MEMORY_HINT
#define GL_RECLAIM_MEMORY_HINT                                       0x0001A1FE
#endif
#ifndef GL_STENCIL
#define GL_STENCIL                                                   0x00001802
#endif
#ifndef GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE
#define GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE                       0x00008852
#endif
#ifndef GL_COMPRESSED_RGBA8_ETC2_EAC
#define GL_COMPRESSED_RGBA8_ETC2_EAC                                 0x00009278
#endif
#ifndef GL_VIDEO_CAPTURE_FRAME_WIDTH
#define GL_VIDEO_CAPTURE_FRAME_WIDTH                                 0x00009038
#endif
#ifndef GL_RELATIVE_SMALL_CW_ARC_TO
#define GL_RELATIVE_SMALL_CW_ARC_TO                                  0x00000015
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_4x4x3
#define GL_COMPRESSED_RGBA_ASTC_4x4x3                                0x000093C2
#endif
#ifndef GL_TEXTURE_BUFFER_FORMAT
#define GL_TEXTURE_BUFFER_FORMAT                                     0x00008C2E
#endif
#ifndef GL_VARIANT_ARRAY_TYPE
#define GL_VARIANT_ARRAY_TYPE                                        0x000087E7
#endif
#ifndef GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION
#define GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION                  0x00008E4C
#endif
#ifndef GL_DT_BIAS
#define GL_DT_BIAS                                                   0x00008717
#endif
#ifndef GL_ANY_SAMPLES_PASSED_CONSERVATIVE
#define GL_ANY_SAMPLES_PASSED_CONSERVATIVE                           0x00008D6A
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_5x5
#define GL_COMPRESSED_RGBA_ASTC_5x5                                  0x000093B2
#endif
#ifndef GL_UNPACK_ROW_LENGTH
#define GL_UNPACK_ROW_LENGTH                                         0x00000CF2
#endif
#ifndef GL_TEXTURE_CLIPMAP_FRAME
#define GL_TEXTURE_CLIPMAP_FRAME                                     0x00008172
#endif
#ifndef GL_GEOMETRY_OUTPUT_TYPE
#define GL_GEOMETRY_OUTPUT_TYPE                                      0x00008DDC
#endif
#ifndef GL_BGR_INTEGER
#define GL_BGR_INTEGER                                               0x00008D9A
#endif
#ifndef GL_VERTEX_ATTRIB_MAP1
#define GL_VERTEX_ATTRIB_MAP1                                        0x00008A00
#endif
#ifndef GL_TRANSFORM_FEEDBACK_PAUSED
#define GL_TRANSFORM_FEEDBACK_PAUSED                                 0x00008E23
#endif
#ifndef GL_COLOR_SUM_CLAMP
#define GL_COLOR_SUM_CLAMP                                           0x0000854F
#endif
#ifndef GL_MOV
#define GL_MOV                                                       0x00008961
#endif
#ifndef GL_VARIABLE_A
#define GL_VARIABLE_A                                                0x00008523
#endif
#ifndef GL_STATIC_COPY
#define GL_STATIC_COPY                                               0x000088E6
#endif
#ifndef GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC
#define GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC                        0x000087F8
#endif
#ifndef GL_IMAGE_2D_MULTISAMPLE_ARRAY
#define GL_IMAGE_2D_MULTISAMPLE_ARRAY                                0x00009056
#endif
#ifndef GL_TEXTURE_CLIPMAP_VIRTUAL_DEPTH
#define GL_TEXTURE_CLIPMAP_VIRTUAL_DEPTH                             0x00008174
#endif
#ifndef GL_GCCSO_SHADER_BINARY
#define GL_GCCSO_SHADER_BINARY                                       0x00009260
#endif
#ifndef GL_PIXEL_TRANSFORM_2D
#define GL_PIXEL_TRANSFORM_2D                                        0x00008330
#endif
#ifndef GL_COLOR_TABLE_RED_SIZE
#define GL_COLOR_TABLE_RED_SIZE                                      0x000080DA
#endif
#ifndef GL_COLOR_BUFFER_BIT7
#define GL_COLOR_BUFFER_BIT7                                         0x00000080
#endif
#ifndef GL_OP_SET_GE
#define GL_OP_SET_GE                                                 0x0000878C
#endif
#ifndef GL_UNSIGNED_INT_VEC4
#define GL_UNSIGNED_INT_VEC4                                         0x00008DC8
#endif
#ifndef GL_SUCCESS
#define GL_SUCCESS                                                   0x0000902F
#endif
#ifndef GL_VARIABLE_D
#define GL_VARIABLE_D                                                0x00008526
#endif
#ifndef GL_TEXTURE29
#define GL_TEXTURE29                                                 0x000084DD
#endif
#ifndef GL_DUAL_ALPHA12
#define GL_DUAL_ALPHA12                                              0x00008112
#endif
#ifndef GL_CONTEXT_FLAG_DEBUG_BIT
#define GL_CONTEXT_FLAG_DEBUG_BIT                                    0x00000002
#endif
#ifndef GL_COLOR_SUM
#define GL_COLOR_SUM                                                 0x00008458
#endif
#ifndef GL_POST_CONVOLUTION_RED_SCALE
#define GL_POST_CONVOLUTION_RED_SCALE                                0x0000801C
#endif
#ifndef GL_BACK_PRIMARY_COLOR
#define GL_BACK_PRIMARY_COLOR                                        0x00008C77
#endif
#ifndef GL_PERFQUERY_COUNTER_DURATION_RAW
#define GL_PERFQUERY_COUNTER_DURATION_RAW                            0x000094F2
#endif
#ifndef GL_INDEX_TEST
#define GL_INDEX_TEST                                                0x000081B5
#endif
#ifndef GL_ALL_STATIC_DATA
#define GL_ALL_STATIC_DATA                                           0x00019294
#endif
#ifndef GL_VERTEX_PROGRAM
#define GL_VERTEX_PROGRAM                                            0x00008620
#endif
#ifndef GL_RG16F
#define GL_RG16F                                                     0x0000822F
#endif
#ifndef GL_MODELVIEW12
#define GL_MODELVIEW12                                               0x0000872C
#endif
#ifndef GL_TEXTURE_COMPARE_FUNC
#define GL_TEXTURE_COMPARE_FUNC                                      0x0000884D
#endif
#ifndef GL_MAX_TEXTURE_IMAGE_UNITS
#define GL_MAX_TEXTURE_IMAGE_UNITS                                   0x00008872
#endif
#ifndef GL_LUMINANCE4
#define GL_LUMINANCE4                                                0x0000803F
#endif
#ifndef GL_PLUS_DARKER
#define GL_PLUS_DARKER                                               0x00009292
#endif
#ifndef GL_OBJECT_INFO_LOG_LENGTH
#define GL_OBJECT_INFO_LOG_LENGTH                                    0x00008B84
#endif
#ifndef GL_DEPTH_COMPONENT32
#define GL_DEPTH_COMPONENT32                                         0x000081A7
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY_ENABLED
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED                               0x00008622
#endif
#ifndef GL_RGBA32UI
#define GL_RGBA32UI                                                  0x00008D70
#endif
#ifndef GL_TEXTURE_DEPTH
#define GL_TEXTURE_DEPTH                                             0x00008071
#endif
#ifndef GL_COMBINER1
#define GL_COMBINER1                                                 0x00008551
#endif
#ifndef GL_DEBUG_TYPE_OTHER
#define GL_DEBUG_TYPE_OTHER                                          0x00008251
#endif
#ifndef GL_CURRENT_BINORMAL
#define GL_CURRENT_BINORMAL                                          0x0000843C
#endif
#ifndef GL_VERTEX_SHADER_BINDING
#define GL_VERTEX_SHADER_BINDING                                     0x00008781
#endif
#ifndef GL_MAX_VARYING_FLOATS
#define GL_MAX_VARYING_FLOATS                                        0x00008B4B
#endif
#ifndef GL_UNSIGNED_INT8_VEC3
#define GL_UNSIGNED_INT8_VEC3                                        0x00008FEE
#endif
#ifndef GL_COLOR_INDEX16
#define GL_COLOR_INDEX16                                             0x000080E7
#endif
#ifndef GL_TANGENT_ARRAY_STRIDE
#define GL_TANGENT_ARRAY_STRIDE                                      0x0000843F
#endif
#ifndef GL_MAX_PROGRAM_IF_DEPTH
#define GL_MAX_PROGRAM_IF_DEPTH                                      0x000088F6
#endif
#ifndef GL_REG_3
#define GL_REG_3                                                     0x00008924
#endif
#ifndef GL_FRAGMENT_DEPTH
#define GL_FRAGMENT_DEPTH                                            0x00008452
#endif
#ifndef GL_WEIGHT_SUM_UNITY
#define GL_WEIGHT_SUM_UNITY                                          0x000086A6
#endif
#ifndef GL_MAX_DEBUG_GROUP_STACK_DEPTH
#define GL_MAX_DEBUG_GROUP_STACK_DEPTH                               0x0000826C
#endif
#ifndef GL_INTENSITY32UI
#define GL_INTENSITY32UI                                             0x00008D73
#endif
#ifndef GL_R16F
#define GL_R16F                                                      0x0000822D
#endif
#ifndef GL_COMPRESSED_SRGB_PVRTC_4BPPV1
#define GL_COMPRESSED_SRGB_PVRTC_4BPPV1                              0x00008A55
#endif
#ifndef GL_PIXEL_TILE_CACHE_INCREMENT
#define GL_PIXEL_TILE_CACHE_INCREMENT                                0x0000813F
#endif
#ifndef GL_POLYGON_OFFSET_FACTOR
#define GL_POLYGON_OFFSET_FACTOR                                     0x00008038
#endif
#ifndef GL_TEXTURE_PRIORITY
#define GL_TEXTURE_PRIORITY                                          0x00008066
#endif
#ifndef GL_UNSIGNED_INT_SAMPLER_CUBE
#define GL_UNSIGNED_INT_SAMPLER_CUBE                                 0x00008DD4
#endif
#ifndef GL_MAX_OPTIMIZED_VERTEX_SHADER_INVARIANTS
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_INVARIANTS                    0x000087CD
#endif
#ifndef GL_MATRIX7
#define GL_MATRIX7                                                   0x000088C7
#endif
#ifndef GL_TEXTURE_1D_STACK_BINDING
#define GL_TEXTURE_1D_STACK_BINDING                                  0x0000875D
#endif
#ifndef GL_PROGRAM_PIPELINE_OBJECT
#define GL_PROGRAM_PIPELINE_OBJECT                                   0x00008A4F
#endif
#ifndef GL_MAX_VERTEX_SHADER_INVARIANTS
#define GL_MAX_VERTEX_SHADER_INVARIANTS                              0x000087C7
#endif
#ifndef GL_INT_IMAGE_2D_MULTISAMPLE
#define GL_INT_IMAGE_2D_MULTISAMPLE                                  0x00009060
#endif
#ifndef GL_OP_CROSS_PRODUCT
#define GL_OP_CROSS_PRODUCT                                          0x00008797
#endif
#ifndef GL_SECONDARY_COLOR_ARRAY_LENGTH
#define GL_SECONDARY_COLOR_ARRAY_LENGTH                              0x00008F31
#endif
#ifndef GL_PERFMON_GLOBAL_MODE
#define GL_PERFMON_GLOBAL_MODE                                       0x00008FA0
#endif
#ifndef GL_RGB12
#define GL_RGB12                                                     0x00008053
#endif
#ifndef GL_FRAGMENT_LIGHT5
#define GL_FRAGMENT_LIGHT5                                           0x00008411
#endif
#ifndef GL_DEPTH_BOUNDS_TEST
#define GL_DEPTH_BOUNDS_TEST                                         0x00008890
#endif
#ifndef GL_BOOL_VEC2
#define GL_BOOL_VEC2                                                 0x00008B57
#endif
#ifndef GL_PERFQUERY_COUNTER_DATA_DOUBLE
#define GL_PERFQUERY_COUNTER_DATA_DOUBLE                             0x000094FB
#endif
#ifndef GL_PROGRAM_BINARY
#define GL_PROGRAM_BINARY                                            0x000093A6
#endif
#ifndef GL_REG_14
#define GL_REG_14                                                    0x0000892F
#endif
#ifndef GL_MULTISAMPLE_BUFFER_BIT1
#define GL_MULTISAMPLE_BUFFER_BIT1                                   0x02000000
#endif
#ifndef GL_MODELVIEW31
#define GL_MODELVIEW31                                               0x0000873F
#endif
#ifndef GL_DOT_PRODUCT_DEPTH_REPLACE
#define GL_DOT_PRODUCT_DEPTH_REPLACE                                 0x000086ED
#endif
#ifndef GL_FRAMEBUFFER_ATTACHMENT
#define GL_FRAMEBUFFER_ATTACHMENT                                    0x000093A3
#endif
#ifndef GL_TEXTURE13
#define GL_TEXTURE13                                                 0x000084CD
#endif
#ifndef GL_PALETTE8_RGB8
#define GL_PALETTE8_RGB8                                             0x00008B95
#endif
#ifndef GL_COMPUTE_PROGRAM
#define GL_COMPUTE_PROGRAM                                           0x000090FB
#endif
#ifndef GL_OPERAND3_ALPHA
#define GL_OPERAND3_ALPHA                                            0x0000859B
#endif
#ifndef GL_TEXTURE_WRAP_R
#define GL_TEXTURE_WRAP_R                                            0x00008072
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD2
#define GL_OUTPUT_TEXTURE_COORD2                                     0x0000879F
#endif
#ifndef GL_R8UI
#define GL_R8UI                                                      0x00008232
#endif
#ifndef GL_STORAGE_SHARED
#define GL_STORAGE_SHARED                                            0x000085BF
#endif
#ifndef GL_PATH_TERMINAL_DASH_CAP
#define GL_PATH_TERMINAL_DASH_CAP                                    0x0000907D
#endif
#ifndef GL_RGB565
#define GL_RGB565                                                    0x00008D62
#endif
#ifndef GL_MODULATE_SIGNED_ADD
#define GL_MODULATE_SIGNED_ADD                                       0x00008745
#endif
#ifndef GL_COLOR_SAMPLES
#define GL_COLOR_SAMPLES                                             0x00008E20
#endif
#ifndef GL_MAP_FLUSH_EXPLICIT_BIT
#define GL_MAP_FLUSH_EXPLICIT_BIT                                    0x00000010
#endif
#ifndef GL_RG32I
#define GL_RG32I                                                     0x0000823B
#endif
#ifndef GL_DEBUG_CATEGORY_APPLICATION
#define GL_DEBUG_CATEGORY_APPLICATION                                0x0000914F
#endif
#ifndef GL_FRAGMENT_NORMAL
#define GL_FRAGMENT_NORMAL                                           0x0000834A
#endif
#ifndef GL_TEXTURE31
#define GL_TEXTURE31                                                 0x000084DF
#endif
#ifndef GL_TRANSPOSE_PROGRAM_MATRIX
#define GL_TRANSPOSE_PROGRAM_MATRIX                                  0x00008E2E
#endif
#ifndef GL_MAX_VERTEX_OUTPUT_COMPONENTS
#define GL_MAX_VERTEX_OUTPUT_COMPONENTS                              0x00009122
#endif
#ifndef GL_DRAW_BUFFER9
#define GL_DRAW_BUFFER9                                              0x0000882E
#endif
#ifndef GL_PROGRAM_NATIVE_TEX_INSTRUCTIONS
#define GL_PROGRAM_NATIVE_TEX_INSTRUCTIONS                           0x00008809
#endif
#ifndef GL_COLOR_ALPHA_PAIRING
#define GL_COLOR_ALPHA_PAIRING                                       0x00008975
#endif
#ifndef GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP
#define GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP                    0x000086F3
#endif
#ifndef GL_RG_INTEGER
#define GL_RG_INTEGER                                                0x00008228
#endif
#ifndef GL_TEXTURE_BINDING_RENDERBUFFER
#define GL_TEXTURE_BINDING_RENDERBUFFER                              0x00008E53
#endif
#ifndef GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1                        0x00008A56
#endif
#ifndef GL_OBJECT_TYPE
#define GL_OBJECT_TYPE                                               0x00009112
#endif
#ifndef GL_INDEX_TEST_REF
#define GL_INDEX_TEST_REF                                            0x000081B7
#endif
#ifndef GL_MAP2_VERTEX_ATTRIB2_4
#define GL_MAP2_VERTEX_ATTRIB2_4                                     0x00008672
#endif
#ifndef GL_COMPRESSED_RGBA_PVRTC_2BPPV2
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV2                              0x00009137
#endif
#ifndef GL_MODELVIEW25
#define GL_MODELVIEW25                                               0x00008739
#endif
#ifndef GL_UNSIGNED_INT16_VEC2
#define GL_UNSIGNED_INT16_VEC2                                       0x00008FF1
#endif
#ifndef GL_COMPRESSED_ALPHA
#define GL_COMPRESSED_ALPHA                                          0x000084E9
#endif
#ifndef GL_DEPTH
#define GL_DEPTH                                                     0x00001801
#endif
#ifndef GL_MAP2_VERTEX_ATTRIB4_4
#define GL_MAP2_VERTEX_ATTRIB4_4                                     0x00008674
#endif
#ifndef GL_TEXTURE_CLIPMAP_OFFSET
#define GL_TEXTURE_CLIPMAP_OFFSET                                    0x00008173
#endif
#ifndef GL_SLUMINANCE8_ALPHA8
#define GL_SLUMINANCE8_ALPHA8                                        0x00008C45
#endif
#ifndef GL_VERTEX_ARRAY_RANGE_VALID
#define GL_VERTEX_ARRAY_RANGE_VALID                                  0x0000851F
#endif
#ifndef GL_TEXTURE_LEQUAL_R
#define GL_TEXTURE_LEQUAL_R                                          0x0000819C
#endif
#ifndef GL_POST_TEXTURE_FILTER_SCALE_RANGE
#define GL_POST_TEXTURE_FILTER_SCALE_RANGE                           0x0000817C
#endif
#ifndef GL_UNPACK_RESAMPLE
#define GL_UNPACK_RESAMPLE                                           0x00008985
#endif
#ifndef GL_RESET_NOTIFICATION_STRATEGY
#define GL_RESET_NOTIFICATION_STRATEGY                               0x00008256
#endif
#ifndef GL_INNOCENT_CONTEXT_RESET
#define GL_INNOCENT_CONTEXT_RESET                                    0x00008254
#endif
#ifndef GL_AUX_DEPTH_STENCIL
#define GL_AUX_DEPTH_STENCIL                                         0x00008A14
#endif
#ifndef GL_RED_BIT
#define GL_RED_BIT                                                   0x00000001
#endif
#ifndef GL_QUERY_BY_REGION_NO_WAIT
#define GL_QUERY_BY_REGION_NO_WAIT                                   0x00008E16
#endif
#ifndef GL_LUMINANCE_ALPHA_FLOAT32
#define GL_LUMINANCE_ALPHA_FLOAT32                                   0x00008819
#endif
#ifndef GL_T2F_IUI_N3F_V2F
#define GL_T2F_IUI_N3F_V2F                                           0x000081B3
#endif
#ifndef GL_PRESERVE
#define GL_PRESERVE                                                  0x00008762
#endif
#ifndef GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING                      0x0000889C
#endif
#ifndef GL_DETAIL_TEXTURE_2D
#define GL_DETAIL_TEXTURE_2D                                         0x00008095
#endif
#ifndef GL_DEPTH24_STENCIL8
#define GL_DEPTH24_STENCIL8                                          0x000088F0
#endif
#ifndef GL_MAX_TEXTURE_COORDS
#define GL_MAX_TEXTURE_COORDS                                        0x00008871
#endif
#ifndef GL_R8I
#define GL_R8I                                                       0x00008231
#endif
#ifndef GL_UNSIGNALED
#define GL_UNSIGNALED                                                0x00009118
#endif
#ifndef GL_VOLATILE
#define GL_VOLATILE                                                  0x00008A1A
#endif
#ifndef GL_INT64_VEC4
#define GL_INT64_VEC4                                                0x00008FEB
#endif
#ifndef GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH
#define GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH                        0x00008B8A
#endif
#ifndef GL_CON_16
#define GL_CON_16                                                    0x00008951
#endif
#ifndef GL_VIDEO_CAPTURE_TO_422_SUPPORTED
#define GL_VIDEO_CAPTURE_TO_422_SUPPORTED                            0x00009026
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4                          0x000093D0
#endif
#ifndef GL_SEPARABLE_2D
#define GL_SEPARABLE_2D                                              0x00008012
#endif
#ifndef GL_INSTRUMENT_MEASUREMENTS
#define GL_INSTRUMENT_MEASUREMENTS                                   0x00008181
#endif
#ifndef GL_GLYPH_HAS_KERNING_BIT
#define GL_GLYPH_HAS_KERNING_BIT                                     0x00000100
#endif
#ifndef GL_SHADOW_ATTENUATION
#define GL_SHADOW_ATTENUATION                                        0x0000834E
#endif
#ifndef GL_SOURCE3_RGB
#define GL_SOURCE3_RGB                                               0x00008583
#endif
#ifndef GL_SEPARATE_SPECULAR_COLOR
#define GL_SEPARATE_SPECULAR_COLOR                                   0x000081FA
#endif
#ifndef GL_INDEX_ARRAY_POINTER
#define GL_INDEX_ARRAY_POINTER                                       0x00008091
#endif
#ifndef GL_COLOR_ATTACHMENT7
#define GL_COLOR_ATTACHMENT7                                         0x00008CE7
#endif
#ifndef GL_CON_3
#define GL_CON_3                                                     0x00008944
#endif
#ifndef GL_DEBUG_SEVERITY_LOW
#define GL_DEBUG_SEVERITY_LOW                                        0x00009148
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD24
#define GL_OUTPUT_TEXTURE_COORD24                                    0x000087B5
#endif
#ifndef GL_SYNC_STATUS
#define GL_SYNC_STATUS                                               0x00009114
#endif
#ifndef GL_SAMPLER_1D_ARRAY_SHADOW
#define GL_SAMPLER_1D_ARRAY_SHADOW                                   0x00008DC3
#endif
#ifndef GL_FRAGMENT_COLOR_MATERIAL
#define GL_FRAGMENT_COLOR_MATERIAL                                   0x00008401
#endif
#ifndef GL_GLOBAL_ALPHA
#define GL_GLOBAL_ALPHA                                              0x000081D9
#endif
#ifndef GL_Z4Y12Z4CB12Z4A12Z4Y12Z4CR12Z4A12_4224
#define GL_Z4Y12Z4CB12Z4A12Z4Y12Z4CR12Z4A12_4224                     0x00009036
#endif
#ifndef GL_POINT_SIZE_MIN
#define GL_POINT_SIZE_MIN                                            0x00008126
#endif
#ifndef GL_TEXTURE_ENV_BIAS
#define GL_TEXTURE_ENV_BIAS                                          0x000080BE
#endif
#ifndef GL_MAX_PROGRAM_PARAMETER_BUFFER_SIZE
#define GL_MAX_PROGRAM_PARAMETER_BUFFER_SIZE                         0x00008DA1
#endif
#ifndef GL_ALPHA_MIN
#define GL_ALPHA_MIN                                                 0x00008320
#endif
#ifndef GL_PATH_STENCIL_DEPTH_OFFSET_UNITS
#define GL_PATH_STENCIL_DEPTH_OFFSET_UNITS                           0x000090BE
#endif
#ifndef GL_BUMP_ROT_MATRIX
#define GL_BUMP_ROT_MATRIX                                           0x00008775
#endif
#ifndef GL_RED
#define GL_RED                                                       0x00001903
#endif
#ifndef GL_NUM_INSTRUCTIONS_TOTAL
#define GL_NUM_INSTRUCTIONS_TOTAL                                    0x00008972
#endif
#ifndef GL_DEFORMATIONS_MASK
#define GL_DEFORMATIONS_MASK                                         0x00008196
#endif
#ifndef GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE
#define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE                         0x00008216
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY3
#define GL_VERTEX_ATTRIB_ARRAY3                                      0x00008653
#endif
#ifndef GL_CON_4
#define GL_CON_4                                                     0x00008945
#endif
#ifndef GL_MAP2_VERTEX_ATTRIB6_4
#define GL_MAP2_VERTEX_ATTRIB6_4                                     0x00008676
#endif
#ifndef GL_LAYER
#define GL_LAYER                                                     0x00008DAA
#endif
#ifndef GL_DOT_PRODUCT_AFFINE_DEPTH_REPLACE
#define GL_DOT_PRODUCT_AFFINE_DEPTH_REPLACE                          0x0000885D
#endif
#ifndef GL_POINT_SPRITE
#define GL_POINT_SPRITE                                              0x00008861
#endif
#ifndef GL_SCALED_RESOLVE_NICEST
#define GL_SCALED_RESOLVE_NICEST                                     0x000090BB
#endif
#ifndef GL_BLUE_BIT
#define GL_BLUE_BIT                                                  0x00000004
#endif
#ifndef GL_DEPTH_STENCIL_ATTACHMENT
#define GL_DEPTH_STENCIL_ATTACHMENT                                  0x0000821A
#endif
#ifndef GL_BLEND_DST_RGB
#define GL_BLEND_DST_RGB                                             0x000080C8
#endif
#ifndef GL_EVAL_FRACTIONAL_TESSELLATION
#define GL_EVAL_FRACTIONAL_TESSELLATION                              0x000086C5
#endif
#ifndef GL_COLOR_ATTACHMENT15
#define GL_COLOR_ATTACHMENT15                                        0x00008CEF
#endif
#ifndef GL_FOG_COORDINATE_ARRAY_STRIDE
#define GL_FOG_COORDINATE_ARRAY_STRIDE                               0x00008455
#endif
#ifndef GL_PIXEL_SUBSAMPLE_4444
#define GL_PIXEL_SUBSAMPLE_4444                                      0x000085A2
#endif
#ifndef GL_VERTEX_PROGRAM_BINDING
#define GL_VERTEX_PROGRAM_BINDING                                    0x0000864A
#endif
#ifndef GL_TRACK_MATRIX_TRANSFORM
#define GL_TRACK_MATRIX_TRANSFORM                                    0x00008649
#endif
#ifndef GL_COLOR_ARRAY_TYPE
#define GL_COLOR_ARRAY_TYPE                                          0x00008082
#endif
#ifndef GL_PREFER_DOUBLEBUFFER_HINT
#define GL_PREFER_DOUBLEBUFFER_HINT                                  0x0001A1F8
#endif
#ifndef GL_TEXTURE_DT_SIZE
#define GL_TEXTURE_DT_SIZE                                           0x0000871E
#endif
#ifndef GL_MAP1_VERTEX_ATTRIB8_4
#define GL_MAP1_VERTEX_ATTRIB8_4                                     0x00008668
#endif
#ifndef GL_EMBOSS_LIGHT
#define GL_EMBOSS_LIGHT                                              0x0000855D
#endif
#ifndef GL_DISJOINT
#define GL_DISJOINT                                                  0x00009283
#endif
#ifndef GL_GLYPH_HORIZONTAL_BEARING_Y_BIT
#define GL_GLYPH_HORIZONTAL_BEARING_Y_BIT                            0x00000008
#endif
#ifndef GL_SAMPLE_COVERAGE_VALUE
#define GL_SAMPLE_COVERAGE_VALUE                                     0x000080AA
#endif
#ifndef GL_MAP1_VERTEX_ATTRIB4_4
#define GL_MAP1_VERTEX_ATTRIB4_4                                     0x00008664
#endif
#ifndef GL_OP_POWER
#define GL_OP_POWER                                                  0x00008793
#endif
#ifndef GL_VERTEX_STREAM6
#define GL_VERTEX_STREAM6                                            0x00008772
#endif
#ifndef GL_TEXTURE_CUBE_MAP_POSITIVE_Y
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y                               0x00008517
#endif
#ifndef GL_REFLECTION_MAP
#define GL_REFLECTION_MAP                                            0x00008512
#endif
#ifndef GL_PATH_DASH_ARRAY_COUNT
#define GL_PATH_DASH_ARRAY_COUNT                                     0x0000909F
#endif
#ifndef GL_COVERAGE_EDGE_FRAGMENTS
#define GL_COVERAGE_EDGE_FRAGMENTS                                   0x00008ED6
#endif
#ifndef GL_SAMPLE_PATTERN
#define GL_SAMPLE_PATTERN                                            0x000080AC
#endif
#ifndef GL_HALF_FLOAT
#define GL_HALF_FLOAT                                                0x0000140B
#endif
#ifndef GL_NEGATIVE_Y
#define GL_NEGATIVE_Y                                                0x000087DA
#endif
#ifndef GL_DEBUG_OUTPUT
#define GL_DEBUG_OUTPUT                                              0x000092E0
#endif
#ifndef GL_DRAW_FRAMEBUFFER_BINDING
#define GL_DRAW_FRAMEBUFFER_BINDING                                  0x00008CA6
#endif
#ifndef GL_MULTISAMPLE_BUFFER_BIT4
#define GL_MULTISAMPLE_BUFFER_BIT4                                   0x10000000
#endif
#ifndef GL_COMBINE4
#define GL_COMBINE4                                                  0x00008503
#endif
#ifndef GL_ELEMENT_ARRAY_TYPE
#define GL_ELEMENT_ARRAY_TYPE                                        0x00008A0D
#endif
#ifndef GL_COMPRESSED_RG11_EAC
#define GL_COMPRESSED_RG11_EAC                                       0x00009272
#endif
#ifndef GL_CMYK
#define GL_CMYK                                                      0x0000800C
#endif
#ifndef GL_TEXTURE_INTENSITY_SIZE
#define GL_TEXTURE_INTENSITY_SIZE                                    0x00008061
#endif
#ifndef GL_DST_IN
#define GL_DST_IN                                                    0x0000928B
#endif
#ifndef GL_TEXTURE_CUBE_MAP_POSITIVE_X
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X                               0x00008515
#endif
#ifndef GL_FRAGMENT_SHADER_DISCARDS_SAMPLES
#define GL_FRAGMENT_SHADER_DISCARDS_SAMPLES                          0x00008A52
#endif
#ifndef GL_ALPHA16I
#define GL_ALPHA16I                                                  0x00008D8A
#endif
#ifndef GL_SKIP_COMPONENTS4
#define GL_SKIP_COMPONENTS4                                          0xFFFFFFFFFFFFFFFD
#endif
#ifndef GL_SIGNED_NORMALIZED
#define GL_SIGNED_NORMALIZED                                         0x00008F9C
#endif
#ifndef GL_SAMPLER_1D_SHADOW
#define GL_SAMPLER_1D_SHADOW                                         0x00008B61
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4                        0x000093E3
#endif
#ifndef GL_INDEX_BIT
#define GL_INDEX_BIT                                                 0x00080000
#endif
#ifndef GL_SYNC_FLUSH_COMMANDS_BIT
#define GL_SYNC_FLUSH_COMMANDS_BIT                                   0x00000001
#endif
#ifndef GL_TEXTURE_2D_STACK_BINDING
#define GL_TEXTURE_2D_STACK_BINDING                                  0x0000875E
#endif
#ifndef GL_MAX_GEOMETRY_OUTPUT_VERTICES
#define GL_MAX_GEOMETRY_OUTPUT_VERTICES                              0x00008DE0
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD4
#define GL_OUTPUT_TEXTURE_COORD4                                     0x000087A1
#endif
#ifndef GL_MODELVIEW15
#define GL_MODELVIEW15                                               0x0000872F
#endif
#ifndef GL_DST_ATOP
#define GL_DST_ATOP                                                  0x0000928F
#endif
#ifndef GL_ALPHA8I
#define GL_ALPHA8I                                                   0x00008D90
#endif
#ifndef GL_ALWAYS_SOFT_HINT
#define GL_ALWAYS_SOFT_HINT                                          0x0001A20D
#endif
#ifndef GL_MAX_FRAGMENT_UNIFORM_COMPONENTS
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS                           0x00008B49
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10                        0x000093DB
#endif
#ifndef GL_MODELVIEW0
#define GL_MODELVIEW0                                                0x00001700
#endif
#ifndef GL_DOUBLE_MAT4
#define GL_DOUBLE_MAT4                                               0x00008F48
#endif
#ifndef GL_MATRIX29
#define GL_MATRIX29                                                  0x000088DD
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD5
#define GL_OUTPUT_TEXTURE_COORD5                                     0x000087A2
#endif
#ifndef GL_ASYNC_DRAW_PIXELS
#define GL_ASYNC_DRAW_PIXELS                                         0x0000835D
#endif
#ifndef GL_REG_29
#define GL_REG_29                                                    0x0000893E
#endif
#ifndef GL_COMPRESSED_SIGNED_RED_RGTC1
#define GL_COMPRESSED_SIGNED_RED_RGTC1                               0x00008DBC
#endif
#ifndef GL_COLOR_ATTACHMENT0
#define GL_COLOR_ATTACHMENT0                                         0x00008CE0
#endif
#ifndef GL_COLOR_ARRAY_LIST
#define GL_COLOR_ARRAY_LIST                                          0x000192A0
#endif
#ifndef GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE
#define GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE                           0x00008212
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY_SIZE
#define GL_VERTEX_ATTRIB_ARRAY_SIZE                                  0x00008623
#endif
#ifndef GL_CLIP_FAR_HINT
#define GL_CLIP_FAR_HINT                                             0x0001A221
#endif
#ifndef GL_EDGE_FLAG_ARRAY_LIST
#define GL_EDGE_FLAG_ARRAY_LIST                                      0x000192A3
#endif
#ifndef GL_MAX_FRAGMENT_INPUT_COMPONENTS
#define GL_MAX_FRAGMENT_INPUT_COMPONENTS                             0x00009125
#endif
#ifndef GL_EYE_DISTANCE_TO_LINE
#define GL_EYE_DISTANCE_TO_LINE                                      0x000081F2
#endif
#ifndef GL_TEXTURE_DEPTH_TYPE
#define GL_TEXTURE_DEPTH_TYPE                                        0x00008C16
#endif
#ifndef GL_TEXTURE6
#define GL_TEXTURE6                                                  0x000084C6
#endif
#ifndef GL_TEXCOORD2_BIT
#define GL_TEXCOORD2_BIT                                             0x20000000
#endif
#ifndef GL_QUADRATIC_CURVE_TO
#define GL_QUADRATIC_CURVE_TO                                        0x0000000A
#endif
#ifndef GL_1PASS
#define GL_1PASS                                                     0x000080A1
#endif
#ifndef GL_TEXTURE_SWIZZLE_B
#define GL_TEXTURE_SWIZZLE_B                                         0x00008E44
#endif
#ifndef GL_NEGATIVE_Z
#define GL_NEGATIVE_Z                                                0x000087DB
#endif
#ifndef GL_VERTEX_SHADER_BIT
#define GL_VERTEX_SHADER_BIT                                         0x00000001
#endif
#ifndef GL_MAX_VARYING_COMPONENTS
#define GL_MAX_VARYING_COMPONENTS                                    0x00008B4B
#endif
#ifndef GL_GLYPH_HEIGHT_BIT
#define GL_GLYPH_HEIGHT_BIT                                          0x00000002
#endif
#ifndef GL_DEPENDENT_GB_TEXTURE_2D
#define GL_DEPENDENT_GB_TEXTURE_2D                                   0x000086EA
#endif
#ifndef GL_DOUBLE_VEC4
#define GL_DOUBLE_VEC4                                               0x00008FFE
#endif
#ifndef GL_RG16I
#define GL_RG16I                                                     0x00008239
#endif
#ifndef GL_RG8UI
#define GL_RG8UI                                                     0x00008238
#endif
#ifndef GL_DRAW_INDIRECT_UNIFIED
#define GL_DRAW_INDIRECT_UNIFIED                                     0x00008F40
#endif
#ifndef GL_SURFACE_REGISTERED
#define GL_SURFACE_REGISTERED                                        0x000086FD
#endif
#ifndef GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS
#define GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS                       0x00008810
#endif
#ifndef GL_QUERY_RESULT_NO_WAIT
#define GL_QUERY_RESULT_NO_WAIT                                      0x00009194
#endif
#ifndef GL_RGBA_SIGNED_COMPONENTS
#define GL_RGBA_SIGNED_COMPONENTS                                    0x00008C3C
#endif
#ifndef GL_COLOR_BUFFER_BIT1
#define GL_COLOR_BUFFER_BIT1                                         0x00000002
#endif
#ifndef GL_TEXTURE_COORD_ARRAY_SIZE
#define GL_TEXTURE_COORD_ARRAY_SIZE                                  0x00008088
#endif
#ifndef GL_MAX_PROGRAM_SUBROUTINE_PARAMETERS
#define GL_MAX_PROGRAM_SUBROUTINE_PARAMETERS                         0x00008F44
#endif
#ifndef GL_TEXTURE_MIN_LOD
#define GL_TEXTURE_MIN_LOD                                           0x0000813A
#endif
#ifndef GL_MAX_VERTEX_ATTRIBS
#define GL_MAX_VERTEX_ATTRIBS                                        0x00008869
#endif
#ifndef GL_VERTEX4_BIT
#define GL_VERTEX4_BIT                                               0x00000008
#endif
#ifndef GL_ALPHA8UI
#define GL_ALPHA8UI                                                  0x00008D7E
#endif
#ifndef GL_LUMINANCE_ALPHA32I
#define GL_LUMINANCE_ALPHA32I                                        0x00008D87
#endif
#ifndef GL_UNSIGNED_INT_SAMPLER_2D_ARRAY
#define GL_UNSIGNED_INT_SAMPLER_2D_ARRAY                             0x00008DD7
#endif
#ifndef GL_RELATIVE_CUBIC_CURVE_TO
#define GL_RELATIVE_CUBIC_CURVE_TO                                   0x0000000D
#endif
#ifndef GL_PROGRAM_TEX_INDIRECTIONS
#define GL_PROGRAM_TEX_INDIRECTIONS                                  0x00008807
#endif
#ifndef GL_CON_22
#define GL_CON_22                                                    0x00008957
#endif
#ifndef GL_LUMINANCE_ALPHA8UI
#define GL_LUMINANCE_ALPHA8UI                                        0x00008D81
#endif
#ifndef GL_MATRIX25
#define GL_MATRIX25                                                  0x000088D9
#endif
#ifndef GL_UNSIGNED_SHORT_4_4_4_4
#define GL_UNSIGNED_SHORT_4_4_4_4                                    0x00008033
#endif
#ifndef GL_SECONDARY_COLOR_ARRAY_POINTER
#define GL_SECONDARY_COLOR_ARRAY_POINTER                             0x0000845D
#endif
#ifndef GL_QUERY_BY_REGION_WAIT
#define GL_QUERY_BY_REGION_WAIT                                      0x00008E15
#endif
#ifndef GL_ELEMENT_ARRAY_UNIFIED
#define GL_ELEMENT_ARRAY_UNIFIED                                     0x00008F1F
#endif
#ifndef GL_FLOAT_VEC4
#define GL_FLOAT_VEC4                                                0x00008B52
#endif
#ifndef GL_COMPRESSED_SIGNED_R11_EAC
#define GL_COMPRESSED_SIGNED_R11_EAC                                 0x00009271
#endif
#ifndef GL_TEXTURE_LUMINANCE_SIZE
#define GL_TEXTURE_LUMINANCE_SIZE                                    0x00008060
#endif
#ifndef GL_TEXTURE_RANGE_POINTER
#define GL_TEXTURE_RANGE_POINTER                                     0x000085B8
#endif
#ifndef GL_DEPTH_BUFFER_BIT5
#define GL_DEPTH_BUFFER_BIT5                                         0x00002000
#endif
#ifndef GL_MAX_FOG_FUNC_POINTS
#define GL_MAX_FOG_FUNC_POINTS                                       0x0000812C
#endif
#ifndef GL_DEBUG_TYPE_ERROR
#define GL_DEBUG_TYPE_ERROR                                          0x0000824C
#endif
#ifndef GL_DEBUG_TYPE_MARKER
#define GL_DEBUG_TYPE_MARKER                                         0x00008268
#endif
#ifndef GL_DSDT8_MAG8_INTENSITY8
#define GL_DSDT8_MAG8_INTENSITY8                                     0x0000870B
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING                        0x0000889F
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY4
#define GL_VERTEX_ATTRIB_ARRAY4                                      0x00008654
#endif
#ifndef GL_VERTEX_STREAM2
#define GL_VERTEX_STREAM2                                            0x0000876E
#endif
#ifndef GL_WEIGHT_ARRAY
#define GL_WEIGHT_ARRAY                                              0x000086AD
#endif
#ifndef GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS
#define GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS             0x00008C8A
#endif
#ifndef GL_ONE
#define GL_ONE                                                       0x000087DE
#endif
#ifndef GL_MAX_RECTANGLE_TEXTURE_SIZE
#define GL_MAX_RECTANGLE_TEXTURE_SIZE                                0x000084F8
#endif
#ifndef GL_REG_16
#define GL_REG_16                                                    0x00008931
#endif
#ifndef GL_REPLACE_VALUE
#define GL_REPLACE_VALUE                                             0x0000874B
#endif
#ifndef GL_CONSTANT_BORDER
#define GL_CONSTANT_BORDER                                           0x00008151
#endif
#ifndef GL_NORMAL_ARRAY_STRIDE
#define GL_NORMAL_ARRAY_STRIDE                                       0x0000807F
#endif
#ifndef GL_QUERY_RESULT_AVAILABLE
#define GL_QUERY_RESULT_AVAILABLE                                    0x00008867
#endif
#ifndef GL_INT16_VEC4
#define GL_INT16_VEC4                                                0x00008FE7
#endif
#ifndef GL_RESAMPLE_ZERO_FILL
#define GL_RESAMPLE_ZERO_FILL                                        0x00008987
#endif
#ifndef GL_PN_TRIANGLES_TESSELATION_LEVEL
#define GL_PN_TRIANGLES_TESSELATION_LEVEL                            0x000087F4
#endif
#ifndef GL_DST_OVER
#define GL_DST_OVER                                                  0x00009289
#endif
#ifndef GL_PROXY_TEXTURE_4D
#define GL_PROXY_TEXTURE_4D                                          0x00008135
#endif
#ifndef GL_COMBINER_MAPPING
#define GL_COMBINER_MAPPING                                          0x00008543
#endif
#ifndef GL_DEBUG_TYPE_POP_GROUP
#define GL_DEBUG_TYPE_POP_GROUP                                      0x0000826A
#endif
#ifndef GL_MAX_SPARSE_3D_TEXTURE_SIZE
#define GL_MAX_SPARSE_3D_TEXTURE_SIZE                                0x00009199
#endif
#ifndef GL_PROXY_TEXTURE_1D_ARRAY
#define GL_PROXY_TEXTURE_1D_ARRAY                                    0x00008C19
#endif
#ifndef GL_MAX_IMAGE_SAMPLES
#define GL_MAX_IMAGE_SAMPLES                                         0x0000906D
#endif
#ifndef GL_SLUMINANCE
#define GL_SLUMINANCE                                                0x00008C46
#endif
#ifndef GL_MODELVIEW29
#define GL_MODELVIEW29                                               0x0000873D
#endif
#ifndef GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY
#define GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY                    0x00009048
#endif
#ifndef GL_EVAL_VERTEX_ATTRIB7
#define GL_EVAL_VERTEX_ATTRIB7                                       0x000086CD
#endif
#ifndef GL_INT8
#define GL_INT8                                                      0x00008FE0
#endif
#ifndef GL_RESCALE_NORMAL
#define GL_RESCALE_NORMAL                                            0x0000803A
#endif
#ifndef GL_OP_MULTIPLY_MATRIX
#define GL_OP_MULTIPLY_MATRIX                                        0x00008798
#endif
#ifndef GL_TEXTURE9
#define GL_TEXTURE9                                                  0x000084C9
#endif
#ifndef GL_RG
#define GL_RG                                                        0x00008227
#endif
#ifndef GL_CURRENT_VERTEX_ATTRIB
#define GL_CURRENT_VERTEX_ATTRIB                                     0x00008626
#endif
#ifndef GL_ALPHA12
#define GL_ALPHA12                                                   0x0000803D
#endif
#ifndef GL_TEXTURE_CLIPMAP_DEPTH
#define GL_TEXTURE_CLIPMAP_DEPTH                                     0x00008176
#endif
#ifndef GL_MULTISAMPLE_BUFFER_BIT7
#define GL_MULTISAMPLE_BUFFER_BIT7                                   0x80000000
#endif
#ifndef GL_COLOR_ATTACHMENT10
#define GL_COLOR_ATTACHMENT10                                        0x00008CEA
#endif
#ifndef GL_INT_IMAGE_CUBE
#define GL_INT_IMAGE_CUBE                                            0x0000905B
#endif
#ifndef GL_LUMINANCE8
#define GL_LUMINANCE8                                                0x00008040
#endif
#ifndef GL_MAX_SAMPLES
#define GL_MAX_SAMPLES                                               0x00008D57
#endif
#ifndef GL_BLEND_PREMULTIPLIED_SRC
#define GL_BLEND_PREMULTIPLIED_SRC                                   0x00009280
#endif
#ifndef GL_4PASS_3
#define GL_4PASS_3                                                   0x000080A7
#endif
#ifndef GL_IMAGE_BUFFER
#define GL_IMAGE_BUFFER                                              0x00009051
#endif
#ifndef GL_COLOR_ARRAY
#define GL_COLOR_ARRAY                                               0x00008076
#endif
#ifndef GL_DSDT_MAG
#define GL_DSDT_MAG                                                  0x000086F6
#endif
#ifndef GL_QUERY_DEPTH_FAIL_EVENT_BIT
#define GL_QUERY_DEPTH_FAIL_EVENT_BIT                                0x00000002
#endif
#ifndef GL_OUTPUT_COLOR0
#define GL_OUTPUT_COLOR0                                             0x0000879B
#endif
#ifndef GL_LINEAR_SHARPEN
#define GL_LINEAR_SHARPEN                                            0x000080AD
#endif
#ifndef GL_PATH_STROKE_BOUNDING_BOX
#define GL_PATH_STROKE_BOUNDING_BOX                                  0x000090A2
#endif
#ifndef GL_TABLE_TOO_LARGE
#define GL_TABLE_TOO_LARGE                                           0x00008031
#endif
#ifndef GL_WRITE_DISCARD
#define GL_WRITE_DISCARD                                             0x000088BE
#endif
#ifndef GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER                      0x00008CD4
#endif
#ifndef GL_DRAW_BUFFER6
#define GL_DRAW_BUFFER6                                              0x0000882B
#endif
#ifndef GL_CON_25
#define GL_CON_25                                                    0x0000895A
#endif
#ifndef GL_TANGENT_ARRAY
#define GL_TANGENT_ARRAY                                             0x00008439
#endif
#ifndef GL_MAX_VERTEX_UNIFORM_COMPONENTS
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS                             0x00008B4A
#endif
#ifndef GL_OCCLUSION_TEST_RESULT
#define GL_OCCLUSION_TEST_RESULT                                     0x00008166
#endif
#ifndef GL_MINMAX_SINK
#define GL_MINMAX_SINK                                               0x00008030
#endif
#ifndef GL_COMBINER4
#define GL_COMBINER4                                                 0x00008554
#endif
#ifndef GL_SYNC_OBJECT
#define GL_SYNC_OBJECT                                               0x00008A53
#endif
#ifndef GL_STENCIL_BACK_PASS_DEPTH_FAIL
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL                              0x00008802
#endif
#ifndef GL_MAX_PROGRAM_LOCAL_PARAMETERS
#define GL_MAX_PROGRAM_LOCAL_PARAMETERS                              0x000088B4
#endif
#ifndef GL_MIN_LOD_WARNING
#define GL_MIN_LOD_WARNING                                           0x0000919C
#endif
#ifndef GL_SCREEN_COORDINATES
#define GL_SCREEN_COORDINATES                                        0x00008490
#endif
#ifndef GL_FOG_COORDINATE_ARRAY_TYPE
#define GL_FOG_COORDINATE_ARRAY_TYPE                                 0x00008454
#endif
#ifndef GL_PROXY_HISTOGRAM
#define GL_PROXY_HISTOGRAM                                           0x00008025
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4                        0x000093E4
#endif
#ifndef GL_TEXTURE18
#define GL_TEXTURE18                                                 0x000084D2
#endif
#ifndef GL_WRAP_BORDER
#define GL_WRAP_BORDER                                               0x000081D4
#endif
#ifndef GL_GEOMETRY_INPUT_TYPE
#define GL_GEOMETRY_INPUT_TYPE                                       0x00008DDB
#endif
#ifndef GL_GPU_DISJOINT
#define GL_GPU_DISJOINT                                              0x00008FBB
#endif
#ifndef GL_DYNAMIC_DRAW
#define GL_DYNAMIC_DRAW                                              0x000088E8
#endif
#ifndef GL_GLYPH_VERTICAL_BEARING_ADVANCE_BIT
#define GL_GLYPH_VERTICAL_BEARING_ADVANCE_BIT                        0x00000080
#endif
#ifndef GL_FONT_ASCENDER_BIT
#define GL_FONT_ASCENDER_BIT                                         0x00200000
#endif
#ifndef GL_MATRIX13
#define GL_MATRIX13                                                  0x000088CD
#endif
#ifndef GL_EYE_RADIAL
#define GL_EYE_RADIAL                                                0x0000855B
#endif
#ifndef GL_T2F_IUI_V2F
#define GL_T2F_IUI_V2F                                               0x000081B1
#endif
#ifndef GL_FONT_Y_MIN_BOUNDS_BIT
#define GL_FONT_Y_MIN_BOUNDS_BIT                                     0x00020000
#endif
#ifndef GL_WEIGHT_ARRAY_TYPE
#define GL_WEIGHT_ARRAY_TYPE                                         0x000086A9
#endif
#ifndef GL_PERFQUERY_SINGLE_CONTEXT
#define GL_PERFQUERY_SINGLE_CONTEXT                                  0x00000000
#endif
#ifndef GL_COLOR_INDEX1
#define GL_COLOR_INDEX1                                              0x000080E2
#endif
#ifndef GL_RGB10
#define GL_RGB10                                                     0x00008052
#endif
#ifndef GL_FENCE_CONDITION
#define GL_FENCE_CONDITION                                           0x000084F4
#endif
#ifndef GL_PIXEL_TILE_CACHE_SIZE
#define GL_PIXEL_TILE_CACHE_SIZE                                     0x00008145
#endif
#ifndef GL_POINT_FADE_THRESHOLD_SIZE
#define GL_POINT_FADE_THRESHOLD_SIZE                                 0x00008128
#endif
#ifndef GL_DARKEN
#define GL_DARKEN                                                    0x00009297
#endif
#ifndef GL_VIDEO_CAPTURE_FIELD_UPPER_HEIGHT
#define GL_VIDEO_CAPTURE_FIELD_UPPER_HEIGHT                          0x0000903A
#endif
#ifndef GL_BUFFER_GPU_ADDRESS
#define GL_BUFFER_GPU_ADDRESS                                        0x00008F1D
#endif
#ifndef GL_SMOOTH_QUADRATIC_CURVE_TO
#define GL_SMOOTH_QUADRATIC_CURVE_TO                                 0x0000000E
#endif
#ifndef GL_PIXEL_PACK_BUFFER
#define GL_PIXEL_PACK_BUFFER                                         0x000088EB
#endif
#ifndef GL_YCBYCR8_422
#define GL_YCBYCR8_422                                               0x00009031
#endif
#ifndef GL_NUM_INPUT_INTERPOLATOR_COMPONENTS
#define GL_NUM_INPUT_INTERPOLATOR_COMPONENTS                         0x00008973
#endif
#ifndef GL_MAX_DEEP_3D_TEXTURE_DEPTH
#define GL_MAX_DEEP_3D_TEXTURE_DEPTH                                 0x000090D1
#endif
#ifndef GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH
#define GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH                          0x00008B87
#endif
#ifndef GL_TRANSFORM_FEEDBACK_RECORD
#define GL_TRANSFORM_FEEDBACK_RECORD                                 0x00008C86
#endif
#ifndef GL_DOUBLE_MAT2
#define GL_DOUBLE_MAT2                                               0x00008F46
#endif
#ifndef GL_UNIFORM_IS_ROW_MAJOR
#define GL_UNIFORM_IS_ROW_MAJOR                                      0x00008A3E
#endif
#ifndef GL_AVERAGE
#define GL_AVERAGE                                                   0x00008335
#endif
#ifndef GL_IMAGE_ROTATE_ORIGIN_X
#define GL_IMAGE_ROTATE_ORIGIN_X                                     0x0000815A
#endif
#ifndef GL_MAX_PROGRAM_MATRIX_STACK_DEPTH
#define GL_MAX_PROGRAM_MATRIX_STACK_DEPTH                            0x0000862E
#endif
#ifndef GL_FLOAT_RGBA
#define GL_FLOAT_RGBA                                                0x00008883
#endif
#ifndef GL_TEXTURE_GEQUAL_R
#define GL_TEXTURE_GEQUAL_R                                          0x0000819D
#endif
#ifndef GL_REPLACEMENT_CODE_ARRAY_POINTER
#define GL_REPLACEMENT_CODE_ARRAY_POINTER                            0x000085C3
#endif
#ifndef GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME                        0x00008CD1
#endif
#ifndef GL_SLUMINANCE8
#define GL_SLUMINANCE8                                               0x00008C47
#endif
#ifndef GL_DRAW_PIXELS
#define GL_DRAW_PIXELS                                               0x00008A0A
#endif
#ifndef GL_PN_TRIANGLES_NORMAL_MODE
#define GL_PN_TRIANGLES_NORMAL_MODE                                  0x000087F3
#endif
#ifndef GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER                        0x00008CDB
#endif
#ifndef GL_POST_TEXTURE_FILTER_SCALE
#define GL_POST_TEXTURE_FILTER_SCALE                                 0x0000817A
#endif
#ifndef GL_GREEN_INTEGER
#define GL_GREEN_INTEGER                                             0x00008D95
#endif
#ifndef GL_IDENTITY
#define GL_IDENTITY                                                  0x0000862A
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_6x6x5
#define GL_COMPRESSED_RGBA_ASTC_6x6x5                                0x000093C8
#endif
#ifndef GL_DEPTH_BUFFER_FLOAT_MODE
#define GL_DEPTH_BUFFER_FLOAT_MODE                                   0x00008DAF
#endif
#ifndef GL_FLOAT_MAT3
#define GL_FLOAT_MAT3                                                0x00008B5B
#endif
#ifndef GL_DYNAMIC_READ
#define GL_DYNAMIC_READ                                              0x000088E9
#endif
#ifndef GL_TESS_EVALUATION_PROGRAM
#define GL_TESS_EVALUATION_PROGRAM                                   0x0000891F
#endif
#ifndef GL_STENCIL_BUFFER_BIT6
#define GL_STENCIL_BUFFER_BIT6                                       0x00400000
#endif
#ifndef GL_OP_INDEX
#define GL_OP_INDEX                                                  0x00008782
#endif
#ifndef GL_MAX_PROGRAM_TEX_INSTRUCTIONS
#define GL_MAX_PROGRAM_TEX_INSTRUCTIONS                              0x0000880C
#endif
#ifndef GL_DUDV
#define GL_DUDV                                                      0x00008779
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_6x5x5
#define GL_COMPRESSED_RGBA_ASTC_6x5x5                                0x000093C7
#endif
#ifndef GL_ALPHA16F
#define GL_ALPHA16F                                                  0x0000881C
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD23
#define GL_OUTPUT_TEXTURE_COORD23                                    0x000087B4
#endif
#ifndef GL_FLOAT_RGB32
#define GL_FLOAT_RGB32                                               0x00008889
#endif
#ifndef GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z                               0x0000851A
#endif
#ifndef GL_DOT_PRODUCT_TEXTURE_RECTANGLE
#define GL_DOT_PRODUCT_TEXTURE_RECTANGLE                             0x0000864E
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY7
#define GL_VERTEX_ATTRIB_ARRAY7                                      0x00008657
#endif
#ifndef GL_ACTIVE_VARYINGS
#define GL_ACTIVE_VARYINGS                                           0x00008C81
#endif
#ifndef GL_FONT_MAX_ADVANCE_HEIGHT_BIT
#define GL_FONT_MAX_ADVANCE_HEIGHT_BIT                               0x02000000
#endif
#ifndef GL_SYSTEM_FONT_NAME
#define GL_SYSTEM_FONT_NAME                                          0x00009073
#endif
#ifndef GL_REPLACEMENT_CODE_ARRAY
#define GL_REPLACEMENT_CODE_ARRAY                                    0x000085C0
#endif
#ifndef GL_INDEX_TEST_FUNC
#define GL_INDEX_TEST_FUNC                                           0x000081B6
#endif
#ifndef GL_SAMPLER_2D
#define GL_SAMPLER_2D                                                0x00008B5E
#endif
#ifndef GL_W
#define GL_W                                                         0x000087D8
#endif
#ifndef GL_PATH_DASH_OFFSET_RESET
#define GL_PATH_DASH_OFFSET_RESET                                    0x000090B4
#endif
#ifndef GL_TEXTURE_LOD_BIAS_T
#define GL_TEXTURE_LOD_BIAS_T                                        0x0000818F
#endif
#ifndef GL_OBJECT_SUBTYPE
#define GL_OBJECT_SUBTYPE                                            0x00008B4F
#endif
#ifndef GL_FOG_COORDINATE_SOURCE
#define GL_FOG_COORDINATE_SOURCE                                     0x00008450
#endif
#ifndef GL_FONT_DESCENDER_BIT
#define GL_FONT_DESCENDER_BIT                                        0x00400000
#endif
#ifndef GL_TEXTURE_LIGHT
#define GL_TEXTURE_LIGHT                                             0x00008350
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12                        0x000093DD
#endif
#ifndef GL_TEXTURE_LOD_BIAS_S
#define GL_TEXTURE_LOD_BIAS_S                                        0x0000818E
#endif
#ifndef GL_COPY_WRITE_BUFFER
#define GL_COPY_WRITE_BUFFER                                         0x00008F37
#endif
#ifndef GL_TEXTURE_APPLICATION_MODE
#define GL_TEXTURE_APPLICATION_MODE                                  0x0000834F
#endif
#ifndef GL_MAX_SAMPLE_MASK_WORDS
#define GL_MAX_SAMPLE_MASK_WORDS                                     0x00008E59
#endif
#ifndef GL_POST_CONVOLUTION_RED_BIAS
#define GL_POST_CONVOLUTION_RED_BIAS                                 0x00008020
#endif
#ifndef GL_MAP2_VERTEX_ATTRIB7_4
#define GL_MAP2_VERTEX_ATTRIB7_4                                     0x00008677
#endif
#ifndef GL_MAX_PROGRAM_GENERIC_ATTRIBS
#define GL_MAX_PROGRAM_GENERIC_ATTRIBS                               0x00008DA5
#endif
#ifndef GL_SIGNED_HILO16
#define GL_SIGNED_HILO16                                             0x000086FA
#endif
#ifndef GL_WAIT_FAILED
#define GL_WAIT_FAILED                                               0x0000911D
#endif
#ifndef GL_VIDEO_BUFFER_BINDING
#define GL_VIDEO_BUFFER_BINDING                                      0x00009021
#endif
#ifndef GL_BLEND_EQUATION_ALPHA
#define GL_BLEND_EQUATION_ALPHA                                      0x0000883D
#endif
#ifndef GL_UNSIGNED_SHORT_5_5_5_1
#define GL_UNSIGNED_SHORT_5_5_5_1                                    0x00008034
#endif
#ifndef GL_TEXTURE_TOO_LARGE
#define GL_TEXTURE_TOO_LARGE                                         0x00008065
#endif
#ifndef GL_EIGHTH_BIT
#define GL_EIGHTH_BIT                                                0x00000020
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4                          0x000093D1
#endif
#ifndef GL_DEPTH_CLAMP
#define GL_DEPTH_CLAMP                                               0x0000864F
#endif
#ifndef GL_DEPTH_CLAMP_NEAR
#define GL_DEPTH_CLAMP_NEAR                                          0x0000901E
#endif
#ifndef GL_RGB4
#define GL_RGB4                                                      0x0000804F
#endif
#ifndef GL_COLOR_ARRAY_SIZE
#define GL_COLOR_ARRAY_SIZE                                          0x00008081
#endif
#ifndef GL_AFFINE_2D
#define GL_AFFINE_2D                                                 0x00009092
#endif
#ifndef GL_MAX_PROGRAM_TEXTURE_GATHER_COMPONENTS
#define GL_MAX_PROGRAM_TEXTURE_GATHER_COMPONENTS                     0x00008F9F
#endif
#ifndef GL_STRICT_DEPTHFUNC_HINT
#define GL_STRICT_DEPTHFUNC_HINT                                     0x0001A216
#endif
#ifndef GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS
#define GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS                    0x00008A31
#endif
#ifndef GL_UNSIGNED_INT_IMAGE_CUBE
#define GL_UNSIGNED_INT_IMAGE_CUBE                                   0x00009066
#endif
#ifndef GL_RASTER_POSITION_UNCLIPPED
#define GL_RASTER_POSITION_UNCLIPPED                                 0x00019262
#endif
#ifndef GL_PROGRAM_NATIVE_PARAMETERS
#define GL_PROGRAM_NATIVE_PARAMETERS                                 0x000088AA
#endif
#ifndef GL_MAX_PROGRAM_CALL_DEPTH
#define GL_MAX_PROGRAM_CALL_DEPTH                                    0x000088F5
#endif
#ifndef GL_TEXCOORD3_BIT
#define GL_TEXCOORD3_BIT                                             0x40000000
#endif
#ifndef GL_X
#define GL_X                                                         0x000087D5
#endif
#ifndef GL_COLORDODGE
#define GL_COLORDODGE                                                0x00009299
#endif
#ifndef GL_GLYPH_WIDTH_BIT
#define GL_GLYPH_WIDTH_BIT                                           0x00000001
#endif
#ifndef GL_NUM_INSTRUCTIONS_PER_PASS
#define GL_NUM_INSTRUCTIONS_PER_PASS                                 0x00008971
#endif
#ifndef GL_SHADER_CONSISTENT
#define GL_SHADER_CONSISTENT                                         0x000086DD
#endif
#ifndef GL_MAP_INVALIDATE_BUFFER_BIT
#define GL_MAP_INVALIDATE_BUFFER_BIT                                 0x00000008
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY_NORMALIZED
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED                            0x0000886A
#endif
#ifndef GL_OBJECT_VALIDATE_STATUS
#define GL_OBJECT_VALIDATE_STATUS                                    0x00008B83
#endif
#ifndef GL_DSDT_MAG_INTENSITY
#define GL_DSDT_MAG_INTENSITY                                        0x000086DC
#endif
#ifndef GL_VERTEX_WEIGHT_ARRAY_SIZE
#define GL_VERTEX_WEIGHT_ARRAY_SIZE                                  0x0000850D
#endif
#ifndef GL_OP_MUL
#define GL_OP_MUL                                                    0x00008786
#endif
#ifndef GL_UNSIGNED_INT_IMAGE_3D
#define GL_UNSIGNED_INT_IMAGE_3D                                     0x00009064
#endif
#ifndef GL_PROXY_TEXTURE_RECTANGLE
#define GL_PROXY_TEXTURE_RECTANGLE                                   0x000084F7
#endif
#ifndef GL_PERFQUERY_COUNTER_DURATION_NORM
#define GL_PERFQUERY_COUNTER_DURATION_NORM                           0x000094F1
#endif
#ifndef GL_RESAMPLE_REPLICATE
#define GL_RESAMPLE_REPLICATE                                        0x00008986
#endif
#ifndef GL_COMPRESSED_RGB_PVRTC_4BPPV1
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1                               0x00008C00
#endif
#ifndef GL_IMAGE_CUBE
#define GL_IMAGE_CUBE                                                0x00009050
#endif
#ifndef GL_INT_SAMPLER_3D
#define GL_INT_SAMPLER_3D                                            0x00008DCB
#endif
#ifndef GL_POST_COLOR_MATRIX_BLUE_BIAS
#define GL_POST_COLOR_MATRIX_BLUE_BIAS                               0x000080BA
#endif
#ifndef GL_LINEARBURN
#define GL_LINEARBURN                                                0x000092A5
#endif
#ifndef GL_POINT_DISTANCE_ATTENUATION
#define GL_POINT_DISTANCE_ATTENUATION                                0x00008129
#endif
#ifndef GL_QUERY_ALL_EVENT_BITS
#define GL_QUERY_ALL_EVENT_BITS                                      0xFFFFFFFF
#endif
#ifndef GL_SCALED_RESOLVE_FASTEST
#define GL_SCALED_RESOLVE_FASTEST                                    0x000090BA
#endif
#ifndef GL_PATH_GEN_MODE
#define GL_PATH_GEN_MODE                                             0x000090B0
#endif
#ifndef GL_TRANSFORM_FEEDBACK_VARYINGS
#define GL_TRANSFORM_FEEDBACK_VARYINGS                               0x00008C83
#endif
#ifndef GL_COMPRESSED_RGBA_PVRTC_4BPPV1
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1                              0x00008C02
#endif
#ifndef GL_FLOAT_RGBA16
#define GL_FLOAT_RGBA16                                              0x0000888A
#endif
#ifndef GL_SHADER_OPERATION
#define GL_SHADER_OPERATION                                          0x000086DF
#endif
#ifndef GL_POST_CONVOLUTION_GREEN_BIAS
#define GL_POST_CONVOLUTION_GREEN_BIAS                               0x00008021
#endif
#ifndef GL_IMAGE_BINDING_ACCESS
#define GL_IMAGE_BINDING_ACCESS                                      0x00008F3E
#endif
#ifndef GL_INTENSITY_FLOAT16
#define GL_INTENSITY_FLOAT16                                         0x0000881D
#endif
#ifndef GL_FRAGMENT_SHADER_DERIVATIVE_HINT
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT                           0x00008B8B
#endif
#ifndef GL_LUMINANCE_ALPHA_FLOAT16
#define GL_LUMINANCE_ALPHA_FLOAT16                                   0x0000881F
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD8
#define GL_OUTPUT_TEXTURE_COORD8                                     0x000087A5
#endif
#ifndef GL_RGBA_FLOAT_MODE
#define GL_RGBA_FLOAT_MODE                                           0x00008820
#endif
#ifndef GL_TEXTURE_COMPRESSION_HINT
#define GL_TEXTURE_COMPRESSION_HINT                                  0x000084EF
#endif
#ifndef GL_DEPTH_COMPONENT32F
#define GL_DEPTH_COMPONENT32F                                        0x00008CAC
#endif
#ifndef GL_PATH_COMMAND_COUNT
#define GL_PATH_COMMAND_COUNT                                        0x0000909D
#endif
#ifndef GL_READ_BUFFER
#define GL_READ_BUFFER                                               0x00000C02
#endif
#ifndef GL_DSDT_MAG_VIB
#define GL_DSDT_MAG_VIB                                              0x000086F7
#endif
#ifndef GL_OP_LOG_BASE_2
#define GL_OP_LOG_BASE_2                                             0x00008792
#endif
#ifndef GL_SAMPLER_CUBE
#define GL_SAMPLER_CUBE                                              0x00008B60
#endif
#ifndef GL_CURRENT_WEIGHT
#define GL_CURRENT_WEIGHT                                            0x000086A8
#endif
#ifndef GL_CLIP_VOLUME_CLIPPING_HINT
#define GL_CLIP_VOLUME_CLIPPING_HINT                                 0x000080F0
#endif
#ifndef GL_INVERSE_TRANSPOSE
#define GL_INVERSE_TRANSPOSE                                         0x0000862D
#endif
#ifndef GL_BINORMAL_ARRAY
#define GL_BINORMAL_ARRAY                                            0x0000843A
#endif
#ifndef GL_GPU_ADDRESS
#define GL_GPU_ADDRESS                                               0x00008F34
#endif
#ifndef GL_MAX_FRAGMENT_LIGHTS
#define GL_MAX_FRAGMENT_LIGHTS                                       0x00008404
#endif
#ifndef GL_LUMINANCE_ALPHA32UI
#define GL_LUMINANCE_ALPHA32UI                                       0x00008D75
#endif
#ifndef GL_FORMAT_SUBSAMPLE_244_244
#define GL_FORMAT_SUBSAMPLE_244_244                                  0x00008983
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY0
#define GL_VERTEX_ATTRIB_ARRAY0                                      0x00008650
#endif
#ifndef GL_WRITE_PIXEL_DATA_RANGE
#define GL_WRITE_PIXEL_DATA_RANGE                                    0x00008878
#endif
#ifndef GL_BUMP_ROT_MATRIX_SIZE
#define GL_BUMP_ROT_MATRIX_SIZE                                      0x00008776
#endif
#ifndef GL_TEXTURE_RENDERBUFFER
#define GL_TEXTURE_RENDERBUFFER                                      0x00008E55
#endif
#ifndef GL_HALF
#define GL_HALF                                                      0x0000140B
#endif
#ifndef GL_RESAMPLE_AVERAGE
#define GL_RESAMPLE_AVERAGE                                          0x00008988
#endif
#ifndef GL_OPERAND1_ALPHA
#define GL_OPERAND1_ALPHA                                            0x00008599
#endif
#ifndef GL_CURRENT_VERTEX_WEIGHT
#define GL_CURRENT_VERTEX_WEIGHT                                     0x0000850B
#endif
#ifndef GL_COMBINER_AB_OUTPUT
#define GL_COMBINER_AB_OUTPUT                                        0x0000854A
#endif
#ifndef GL_UNSIGNED_INT_2_10_10_10_REV
#define GL_UNSIGNED_INT_2_10_10_10_REV                               0x00008368
#endif
#ifndef GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER
#define GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER                 0x00008A44
#endif
#ifndef GL_TEXTURE_MAX_LEVEL
#define GL_TEXTURE_MAX_LEVEL                                         0x0000813D
#endif
#ifndef GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES                      0x00008A43
#endif
#ifndef GL_BUFFER_MAPPED
#define GL_BUFFER_MAPPED                                             0x000088BC
#endif
#ifndef GL_SATURATE_BIT
#define GL_SATURATE_BIT                                              0x00000040
#endif
#ifndef GL_MAX_PROGRAM_PARAMETER_BUFFER_BINDINGS
#define GL_MAX_PROGRAM_PARAMETER_BUFFER_BINDINGS                     0x00008DA0
#endif
#ifndef GL_MALI_SHADER_BINARY
#define GL_MALI_SHADER_BINARY                                        0x00008F60
#endif
#ifndef GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE
#define GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE                       0x00008217
#endif
#ifndef GL_INDEX_ARRAY_STRIDE
#define GL_INDEX_ARRAY_STRIDE                                        0x00008086
#endif
#ifndef GL_FRAGMENT_LIGHT1
#define GL_FRAGMENT_LIGHT1                                           0x0000840D
#endif
#ifndef GL_INT64
#define GL_INT64                                                     0x0000140E
#endif
#ifndef GL_FOG_COORDINATE_ARRAY
#define GL_FOG_COORDINATE_ARRAY                                      0x00008457
#endif
#ifndef GL_SAMPLER_BINDING
#define GL_SAMPLER_BINDING                                           0x00008919
#endif
#ifndef GL_INT16
#define GL_INT16                                                     0x00008FE4
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY_ADDRESS
#define GL_VERTEX_ATTRIB_ARRAY_ADDRESS                               0x00008F20
#endif
#ifndef GL_TEXTURE2
#define GL_TEXTURE2                                                  0x000084C2
#endif
#ifndef GL_ALREADY_SIGNALED
#define GL_ALREADY_SIGNALED                                          0x0000911A
#endif
#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5                        0x000093E7
#endif
#ifndef GL_ALPHA_MAX_CLAMP
#define GL_ALPHA_MAX_CLAMP                                           0x00008567
#endif
#ifndef GL_PIXEL_COUNT
#define GL_PIXEL_COUNT                                               0x00008866
#endif
#ifndef GL_PLUS_CLAMPED
#define GL_PLUS_CLAMPED                                              0x000092B1
#endif
#ifndef GL_2PASS_0
#define GL_2PASS_0                                                   0x000080A2
#endif
#ifndef GL_TEXTURE_DEFORMATION
#define GL_TEXTURE_DEFORMATION                                       0x00008195
#endif
#ifndef GL_HISTOGRAM_FORMAT
#define GL_HISTOGRAM_FORMAT                                          0x00008027
#endif
#ifndef GL_DIFFERENCE
#define GL_DIFFERENCE                                                0x0000929E
#endif
#ifndef GL_CON_26
#define GL_CON_26                                                    0x0000895B
#endif
#ifndef GL_ARRAY_OBJECT_OFFSET
#define GL_ARRAY_OBJECT_OFFSET                                       0x00008767
#endif
#ifndef GL_UNSIGNED_INT16_VEC3
#define GL_UNSIGNED_INT16_VEC3                                       0x00008FF2
#endif
#ifndef GL_MAX_PROGRAM_RESULT_COMPONENTS
#define GL_MAX_PROGRAM_RESULT_COMPONENTS                             0x00008909
#endif
#ifndef GL_VERTEX_ATTRIB_ARRAY12
#define GL_VERTEX_ATTRIB_ARRAY12                                     0x0000865C
#endif
#ifndef GL_UNIFORM_BLOCK_DATA_SIZE
#define GL_UNIFORM_BLOCK_DATA_SIZE                                   0x00008A40
#endif
#ifndef GL_ATC_RGB
#define GL_ATC_RGB                                                   0x00008C92
#endif
#ifndef GL_VARIANT_ARRAY_POINTER
#define GL_VARIANT_ARRAY_POINTER                                     0x000087E9
#endif
#ifndef GL_PIXEL_MIN_FILTER
#define GL_PIXEL_MIN_FILTER                                          0x00008332
#endif
#ifndef GL_EVAL_VERTEX_ATTRIB10
#define GL_EVAL_VERTEX_ATTRIB10                                      0x000086D0
#endif
#ifndef GL_PROXY_TEXTURE_2D_ARRAY
#define GL_PROXY_TEXTURE_2D_ARRAY                                    0x00008C1B
#endif
#ifndef GL_MAP2_VERTEX_ATTRIB14_4
#define GL_MAP2_VERTEX_ATTRIB14_4                                    0x0000867E
#endif
#ifndef GL_PROGRAM_INSTRUCTIONS
#define GL_PROGRAM_INSTRUCTIONS                                      0x000088A0
#endif
#ifndef GL_RED_INTEGER
#define GL_RED_INTEGER                                               0x00008D94
#endif
#ifndef GL_SCREEN
#define GL_SCREEN                                                    0x00009295
#endif
#ifndef GL_MATRIX11
#define GL_MATRIX11                                                  0x000088CB
#endif
#ifndef GL_WRITE_ONLY
#define GL_WRITE_ONLY                                                0x000088B9
#endif
#ifndef GL_LINEAR_DETAIL
#define GL_LINEAR_DETAIL                                             0x00008097
#endif
#ifndef GL_COMPRESSED_LUMINANCE_ALPHA
#define GL_COMPRESSED_LUMINANCE_ALPHA                                0x000084EB
#endif
#ifndef GL_INTENSITY12
#define GL_INTENSITY12                                               0x0000804C
#endif
#ifndef GL_MATRIX0
#define GL_MATRIX0                                                   0x000088C0
#endif
#ifndef GL_CULL_VERTEX_OBJECT_POSITION
#define GL_CULL_VERTEX_OBJECT_POSITION                               0x000081AC
#endif
#ifndef GL_PROGRAM_ADDRESS_REGISTERS
#define GL_PROGRAM_ADDRESS_REGISTERS                                 0x000088B0
#endif
#ifndef GL_R1UI_C4UB_V3F
#define GL_R1UI_C4UB_V3F                                             0x000085C5
#endif
#ifndef GL_PALETTE4_RGB8
#define GL_PALETTE4_RGB8                                             0x00008B90
#endif
#ifndef GL_CURRENT_RASTER_NORMAL
#define GL_CURRENT_RASTER_NORMAL                                     0x00008406
#endif
#ifndef GL_MATRIX_INDEX_ARRAY
#define GL_MATRIX_INDEX_ARRAY                                        0x00008844
#endif
#ifndef GL_SEPARATE_ATTRIBS
#define GL_SEPARATE_ATTRIBS                                          0x00008C8D
#endif
#ifndef GL_DRAW_BUFFER2
#define GL_DRAW_BUFFER2                                              0x00008827
#endif
#ifndef GL_SUB
#define GL_SUB                                                       0x00008965
#endif
#ifndef GL_EXCLUSION
#define GL_EXCLUSION                                                 0x000092A0
#endif
#ifndef GL_ALPHA32UI
#define GL_ALPHA32UI                                                 0x00008D72
#endif
#ifndef GL_MODELVIEW7
#define GL_MODELVIEW7                                                0x00008727
#endif
#ifndef GL_LUMINANCE_ALPHA32F
#define GL_LUMINANCE_ALPHA32F                                        0x00008819
#endif
#ifndef GL_YCRCB_422
#define GL_YCRCB_422                                                 0x000081BB
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD28
#define GL_OUTPUT_TEXTURE_COORD28                                    0x000087B9
#endif
#ifndef GL_GPU_MEMORY_INFO_EVICTION_COUNT
#define GL_GPU_MEMORY_INFO_EVICTION_COUNT                            0x0000904A
#endif
#ifndef GL_BUFFER_ACCESS_FLAGS
#define GL_BUFFER_ACCESS_FLAGS                                       0x0000911F
#endif
#ifndef GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3                           0x00008C4E
#endif
#ifndef GL_PATH_CLIENT_LENGTH
#define GL_PATH_CLIENT_LENGTH                                        0x0000907F
#endif
#ifndef GL_SAMPLE_BUFFERS
#define GL_SAMPLE_BUFFERS                                            0x000080A8
#endif
#ifndef GL_PROGRAM_NATIVE_ATTRIBS
#define GL_PROGRAM_NATIVE_ATTRIBS                                    0x000088AE
#endif
#ifndef GL_CON_12
#define GL_CON_12                                                    0x0000894D
#endif
#ifndef GL_RGB32I
#define GL_RGB32I                                                    0x00008D83
#endif
#ifndef GL_LOCAL_CONSTANT
#define GL_LOCAL_CONSTANT                                            0x000087C3
#endif
#ifndef GL_MAP_READ_BIT
#define GL_MAP_READ_BIT                                              0x00000001
#endif
#ifndef GL_VERTEX_ARRAY_BUFFER_BINDING
#define GL_VERTEX_ARRAY_BUFFER_BINDING                               0x00008896
#endif
#ifndef GL_UNPACK_SKIP_VOLUMES
#define GL_UNPACK_SKIP_VOLUMES                                       0x00008132
#endif
#ifndef GL_COMPRESSED_RGB8_ETC2
#define GL_COMPRESSED_RGB8_ETC2                                      0x00009274
#endif
#ifndef GL_CON_21
#define GL_CON_21                                                    0x00008956
#endif
#ifndef GL_CON_8
#define GL_CON_8                                                     0x00008949
#endif
#ifndef GL_OFFSET_TEXTURE_RECTANGLE_SCALE
#define GL_OFFSET_TEXTURE_RECTANGLE_SCALE                            0x0000864D
#endif
#ifndef GL_QUERY_RESULT
#define GL_QUERY_RESULT                                              0x00008866
#endif
#ifndef GL_COLOR
#define GL_COLOR                                                     0x00001800
#endif
#ifndef GL_STENCIL_INDEX8
#define GL_STENCIL_INDEX8                                            0x00008D48
#endif
#ifndef GL_HILO8
#define GL_HILO8                                                     0x0000885E
#endif
#ifndef GL_COVERAGE_BUFFER_BIT
#define GL_COVERAGE_BUFFER_BIT                                       0x00008000
#endif
#ifndef GL_TEXTURE23
#define GL_TEXTURE23                                                 0x000084D7
#endif
#ifndef GL_COMPRESSED_SLUMINANCE_ALPHA
#define GL_COMPRESSED_SLUMINANCE_ALPHA                               0x00008C4B
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD14
#define GL_OUTPUT_TEXTURE_COORD14                                    0x000087AB
#endif
#ifndef GL_TEXTURE_MAX_CLAMP_T
#define GL_TEXTURE_MAX_CLAMP_T                                       0x0000836A
#endif
#ifndef GL_FLOAT_MAT2x3
#define GL_FLOAT_MAT2x3                                              0x00008B65
#endif
#ifndef GL_RELATIVE_VERTICAL_LINE_TO
#define GL_RELATIVE_VERTICAL_LINE_TO                                 0x00000009
#endif
#ifndef GL_COLOR_ATTACHMENT3
#define GL_COLOR_ATTACHMENT3                                         0x00008CE3
#endif
#ifndef GL_CON_7
#define GL_CON_7                                                     0x00008948
#endif
#ifndef GL_OUTPUT_TEXTURE_COORD15
#define GL_OUTPUT_TEXTURE_COORD15                                    0x000087AC
#endif
#ifndef GL_BLUE_INTEGER
#define GL_BLUE_INTEGER                                              0x00008D96
#endif
#ifndef GL_PIXEL_SUBSAMPLE_2424
#define GL_PIXEL_SUBSAMPLE_2424                                      0x000085A3
#endif
#ifndef GL_ANY_SAMPLES_PASSED
#define GL_ANY_SAMPLES_PASSED                                        0x00008C2F
#endif
#ifndef GL_RGBA12
#define GL_RGBA12                                                    0x0000805A
#endif
#ifndef GL_GREEN_MIN_CLAMP
#define GL_GREEN_MIN_CLAMP                                           0x00008561
#endif
#ifndef GL_CLOSE_PATH
#define GL_CLOSE_PATH                                                0x00000000
#endif
#ifndef GL_ACTIVE_VERTEX_UNITS
#define GL_ACTIVE_VERTEX_UNITS                                       0x000086A5
#endif
#ifndef GL_MATRIX9
#define GL_MATRIX9                                                   0x000088C9
#endif
#ifndef GL_SECONDARY_COLOR_ARRAY_LIST_STRIDE
#define GL_SECONDARY_COLOR_ARRAY_LIST_STRIDE                         0x000192AF
#endif
#ifndef GL_R32F
#define GL_R32F                                                      0x0000822E
#endif
#ifndef GL_MAX_PROGRAM_PATCH_ATTRIBS
#define GL_MAX_PROGRAM_PATCH_ATTRIBS                                 0x000086D8
#endif
#ifndef GL_REG_24
#define GL_REG_24                                                    0x00008939
#endif
#ifndef GL_VERTEX_ATTRIB_MAP2
#define GL_VERTEX_ATTRIB_MAP2                                        0x00008A01
#endif
#ifndef GL_PROGRAM_TEX_INSTRUCTIONS
#define GL_PROGRAM_TEX_INSTRUCTIONS                                  0x00008806
#endif

namespace Scaleform { namespace Render { namespace GL {

class GraphicsDeviceImmediate;

enum GLExtensionType
{
    SF_GL_NONE,
    SF_GL_3DFX_multisample,
    SF_GL_3DFX_tbuffer,
    SF_GL_3DFX_texture_compression_FXT1,
    SF_GL_AMD_blend_minmax_factor,
    SF_GL_AMD_compressed_3DC_texture,
    SF_GL_AMD_compressed_ATC_texture,
    SF_GL_AMD_conservative_depth,
    SF_GL_AMD_debug_output,
    SF_GL_AMD_depth_clamp_separate,
    SF_GL_AMD_draw_buffers_blend,
    SF_GL_AMD_interleaved_elements,
    SF_GL_AMD_multi_draw_indirect,
    SF_GL_AMD_name_gen_delete,
    SF_GL_AMD_occlusion_query_event,
    SF_GL_AMD_performance_monitor,
    SF_GL_AMD_pinned_memory,
    SF_GL_AMD_program_binary_Z400,
    SF_GL_AMD_query_buffer_object,
    SF_GL_AMD_sample_positions,
    SF_GL_AMD_seamless_cubemap_per_texture,
    SF_GL_AMD_shader_atomic_counter_ops,
    SF_GL_AMD_shader_stencil_export,
    SF_GL_AMD_shader_trinary_minmax,
    SF_GL_AMD_sparse_texture,
    SF_GL_AMD_stencil_operation_extended,
    SF_GL_AMD_texture_texture4,
    SF_GL_AMD_transform_feedback3_lines_triangles,
    SF_GL_AMD_vertex_shader_layer,
    SF_GL_AMD_vertex_shader_tessellator,
    SF_GL_AMD_vertex_shader_viewport_index,
    SF_GL_ANGLE_depth_texture,
    SF_GL_ANGLE_framebuffer_blit,
    SF_GL_ANGLE_framebuffer_multisample,
    SF_GL_ANGLE_instanced_arrays,
    SF_GL_ANGLE_pack_reverse_row_order,
    SF_GL_ANGLE_program_binary,
    SF_GL_ANGLE_texture_compression_dxt3,
    SF_GL_ANGLE_texture_compression_dxt5,
    SF_GL_ANGLE_texture_usage,
    SF_GL_ANGLE_translated_shader_source,
    SF_GL_APPLE_aux_depth_stencil,
    SF_GL_APPLE_client_storage,
    SF_GL_APPLE_copy_texture_levels,
    SF_GL_APPLE_element_array,
    SF_GL_APPLE_fence,
    SF_GL_APPLE_float_pixels,
    SF_GL_APPLE_flush_buffer_range,
    SF_GL_APPLE_framebuffer_multisample,
    SF_GL_APPLE_object_purgeable,
    SF_GL_APPLE_rgb_422,
    SF_GL_APPLE_row_bytes,
    SF_GL_APPLE_specular_vector,
    SF_GL_APPLE_sync,
    SF_GL_APPLE_texture_format_BGRA8888,
    SF_GL_APPLE_texture_max_level,
    SF_GL_APPLE_texture_range,
    SF_GL_APPLE_transform_hint,
    SF_GL_APPLE_vertex_array_object,
    SF_GL_APPLE_vertex_array_range,
    SF_GL_APPLE_vertex_program_evaluators,
    SF_GL_APPLE_ycbcr_422,
    SF_GL_ARB_ES2_compatibility,
    SF_GL_ARB_ES3_compatibility,
    SF_GL_ARB_arrays_of_arrays,
    SF_GL_ARB_base_instance,
    SF_GL_ARB_bindless_texture,
    SF_GL_ARB_blend_func_extended,
    SF_GL_ARB_buffer_storage,
    SF_GL_ARB_cl_event,
    SF_GL_ARB_clear_buffer_object,
    SF_GL_ARB_clear_texture,
    SF_GL_ARB_color_buffer_float,
    SF_GL_ARB_compatibility,
    SF_GL_ARB_compressed_texture_pixel_storage,
    SF_GL_ARB_compute_shader,
    SF_GL_ARB_compute_variable_group_size,
    SF_GL_ARB_conservative_depth,
    SF_GL_ARB_copy_buffer,
    SF_GL_ARB_copy_image,
    SF_GL_ARB_debug_output,
    SF_GL_ARB_depth_buffer_float,
    SF_GL_ARB_depth_clamp,
    SF_GL_ARB_depth_texture,
    SF_GL_ARB_draw_buffers,
    SF_GL_ARB_draw_buffers_blend,
    SF_GL_ARB_draw_elements_base_vertex,
    SF_GL_ARB_draw_indirect,
    SF_GL_ARB_draw_instanced,
    SF_GL_ARB_enhanced_layouts,
    SF_GL_ARB_explicit_attrib_location,
    SF_GL_ARB_explicit_uniform_location,
    SF_GL_ARB_fragment_coord_conventions,
    SF_GL_ARB_fragment_layer_viewport,
    SF_GL_ARB_fragment_program,
    SF_GL_ARB_fragment_program_shadow,
    SF_GL_ARB_fragment_shader,
    SF_GL_ARB_framebuffer_no_attachments,
    SF_GL_ARB_framebuffer_object,
    SF_GL_ARB_framebuffer_sRGB,
    SF_GL_ARB_geometry_shader4,
    SF_GL_ARB_get_program_binary,
    SF_GL_ARB_gpu_shader5,
    SF_GL_ARB_gpu_shader_fp64,
    SF_GL_ARB_half_float_pixel,
    SF_GL_ARB_half_float_vertex,
    SF_GL_ARB_imaging,
    SF_GL_ARB_indirect_parameters,
    SF_GL_ARB_instanced_arrays,
    SF_GL_ARB_internalformat_query,
    SF_GL_ARB_internalformat_query2,
    SF_GL_ARB_invalidate_subdata,
    SF_GL_ARB_map_buffer_alignment,
    SF_GL_ARB_map_buffer_range,
    SF_GL_ARB_matrix_palette,
    SF_GL_ARB_multi_bind,
    SF_GL_ARB_multi_draw_indirect,
    SF_GL_ARB_multisample,
    SF_GL_ARB_multitexture,
    SF_GL_ARB_occlusion_query,
    SF_GL_ARB_occlusion_query2,
    SF_GL_ARB_pixel_buffer_object,
    SF_GL_ARB_point_parameters,
    SF_GL_ARB_point_sprite,
    SF_GL_ARB_program_interface_query,
    SF_GL_ARB_provoking_vertex,
    SF_GL_ARB_query_buffer_object,
    SF_GL_ARB_robust_buffer_access_behavior,
    SF_GL_ARB_robustness,
    SF_GL_ARB_robustness_isolation,
    SF_GL_ARB_sample_shading,
    SF_GL_ARB_sampler_objects,
    SF_GL_ARB_seamless_cube_map,
    SF_GL_ARB_seamless_cubemap_per_texture,
    SF_GL_ARB_separate_shader_objects,
    SF_GL_ARB_shader_atomic_counters,
    SF_GL_ARB_shader_bit_encoding,
    SF_GL_ARB_shader_draw_parameters,
    SF_GL_ARB_shader_group_vote,
    SF_GL_ARB_shader_image_load_store,
    SF_GL_ARB_shader_image_size,
    SF_GL_ARB_shader_objects,
    SF_GL_ARB_shader_precision,
    SF_GL_ARB_shader_stencil_export,
    SF_GL_ARB_shader_storage_buffer_object,
    SF_GL_ARB_shader_subroutine,
    SF_GL_ARB_shader_texture_lod,
    SF_GL_ARB_shading_language_100,
    SF_GL_ARB_shading_language_420pack,
    SF_GL_ARB_shading_language_include,
    SF_GL_ARB_shading_language_packing,
    SF_GL_ARB_shadow,
    SF_GL_ARB_shadow_ambient,
    SF_GL_ARB_sparse_texture,
    SF_GL_ARB_stencil_texturing,
    SF_GL_ARB_sync,
    SF_GL_ARB_tessellation_shader,
    SF_GL_ARB_texture_border_clamp,
    SF_GL_ARB_texture_buffer_object,
    SF_GL_ARB_texture_buffer_object_rgb32,
    SF_GL_ARB_texture_buffer_range,
    SF_GL_ARB_texture_compression,
    SF_GL_ARB_texture_compression_bptc,
    SF_GL_ARB_texture_compression_rgtc,
    SF_GL_ARB_texture_cube_map,
    SF_GL_ARB_texture_cube_map_array,
    SF_GL_ARB_texture_env_add,
    SF_GL_ARB_texture_env_combine,
    SF_GL_ARB_texture_env_crossbar,
    SF_GL_ARB_texture_env_dot3,
    SF_GL_ARB_texture_float,
    SF_GL_ARB_texture_gather,
    SF_GL_ARB_texture_mirror_clamp_to_edge,
    SF_GL_ARB_texture_mirrored_repeat,
    SF_GL_ARB_texture_multisample,
    SF_GL_ARB_texture_non_power_of_two,
    SF_GL_ARB_texture_query_levels,
    SF_GL_ARB_texture_query_lod,
    SF_GL_ARB_texture_rectangle,
    SF_GL_ARB_texture_rg,
    SF_GL_ARB_texture_rgb10_a2ui,
    SF_GL_ARB_texture_stencil8,
    SF_GL_ARB_texture_storage,
    SF_GL_ARB_texture_storage_multisample,
    SF_GL_ARB_texture_swizzle,
    SF_GL_ARB_texture_view,
    SF_GL_ARB_timer_query,
    SF_GL_ARB_transform_feedback2,
    SF_GL_ARB_transform_feedback3,
    SF_GL_ARB_transform_feedback_instanced,
    SF_GL_ARB_transpose_matrix,
    SF_GL_ARB_uniform_buffer_object,
    SF_GL_ARB_vertex_array_bgra,
    SF_GL_ARB_vertex_array_object,
    SF_GL_ARB_vertex_attrib_64bit,
    SF_GL_ARB_vertex_attrib_binding,
    SF_GL_ARB_vertex_blend,
    SF_GL_ARB_vertex_buffer_object,
    SF_GL_ARB_vertex_program,
    SF_GL_ARB_vertex_shader,
    SF_GL_ARB_vertex_type_10f_11f_11f_rev,
    SF_GL_ARB_vertex_type_2_10_10_10_rev,
    SF_GL_ARB_viewport_array,
    SF_GL_ARB_window_pos,
    SF_GL_ARM_mali_program_binary,
    SF_GL_ARM_mali_shader_binary,
    SF_GL_ARM_rgba8,
    SF_GL_ATI_draw_buffers,
    SF_GL_ATI_element_array,
    SF_GL_ATI_envmap_bumpmap,
    SF_GL_ATI_fragment_shader,
    SF_GL_ATI_map_object_buffer,
    SF_GL_ATI_meminfo,
    SF_GL_ATI_pixel_format_float,
    SF_GL_ATI_pn_triangles,
    SF_GL_ATI_separate_stencil,
    SF_GL_ATI_text_fragment_shader,
    SF_GL_ATI_texture_env_combine3,
    SF_GL_ATI_texture_float,
    SF_GL_ATI_texture_mirror_once,
    SF_GL_ATI_vertex_array_object,
    SF_GL_ATI_vertex_attrib_array_object,
    SF_GL_ATI_vertex_streams,
    SF_GL_DMP_shader_binary,
    SF_GL_ES_VERSION_2_0,
    SF_GL_ES_VERSION_3_0,
    SF_GL_EXT_422_pixels,
    SF_GL_EXT_abgr,
    SF_GL_EXT_bgra,
    SF_GL_EXT_bindable_uniform,
    SF_GL_EXT_blend_color,
    SF_GL_EXT_blend_equation_separate,
    SF_GL_EXT_blend_func_separate,
    SF_GL_EXT_blend_logic_op,
    SF_GL_EXT_blend_minmax,
    SF_GL_EXT_blend_subtract,
    SF_GL_EXT_clip_volume_hint,
    SF_GL_EXT_cmyka,
    SF_GL_EXT_color_buffer_half_float,
    SF_GL_EXT_color_subtable,
    SF_GL_EXT_compiled_vertex_array,
    SF_GL_EXT_convolution,
    SF_GL_EXT_coordinate_frame,
    SF_GL_EXT_copy_texture,
    SF_GL_EXT_cull_vertex,
    SF_GL_EXT_debug_label,
    SF_GL_EXT_debug_marker,
    SF_GL_EXT_depth_bounds_test,
    SF_GL_EXT_direct_state_access,
    SF_GL_EXT_discard_framebuffer,
    SF_GL_EXT_disjoint_timer_query,
    SF_GL_EXT_draw_buffers,
    SF_GL_EXT_draw_buffers2,
    SF_GL_EXT_draw_instanced,
    SF_GL_EXT_draw_range_elements,
    SF_GL_EXT_fog_coord,
    SF_GL_EXT_framebuffer_blit,
    SF_GL_EXT_framebuffer_multisample,
    SF_GL_EXT_framebuffer_multisample_blit_scaled,
    SF_GL_EXT_framebuffer_object,
    SF_GL_EXT_framebuffer_sRGB,
    SF_GL_EXT_geometry_shader4,
    SF_GL_EXT_gpu_program_parameters,
    SF_GL_EXT_gpu_shader4,
    SF_GL_EXT_histogram,
    SF_GL_EXT_index_array_formats,
    SF_GL_EXT_index_func,
    SF_GL_EXT_index_material,
    SF_GL_EXT_index_texture,
    SF_GL_EXT_instanced_arrays,
    SF_GL_EXT_light_texture,
    SF_GL_EXT_map_buffer_range,
    SF_GL_EXT_misc_attribute,
    SF_GL_EXT_multi_draw_arrays,
    SF_GL_EXT_multisample,
    SF_GL_EXT_multisampled_render_to_texture,
    SF_GL_EXT_multiview_draw_buffers,
    SF_GL_EXT_occlusion_query_boolean,
    SF_GL_EXT_packed_depth_stencil,
    SF_GL_EXT_packed_float,
    SF_GL_EXT_packed_pixels,
    SF_GL_EXT_paletted_texture,
    SF_GL_EXT_pixel_buffer_object,
    SF_GL_EXT_pixel_transform,
    SF_GL_EXT_pixel_transform_color_table,
    SF_GL_EXT_point_parameters,
    SF_GL_EXT_polygon_offset,
    SF_GL_EXT_provoking_vertex,
    SF_GL_EXT_pvrtc_sRGB,
    SF_GL_EXT_read_format_bgra,
    SF_GL_EXT_rescale_normal,
    SF_GL_EXT_robustness,
    SF_GL_EXT_sRGB,
    SF_GL_EXT_sRGB_write_control,
    SF_GL_EXT_secondary_color,
    SF_GL_EXT_separate_shader_objects,
    SF_GL_EXT_separate_specular_color,
    SF_GL_EXT_shader_framebuffer_fetch,
    SF_GL_EXT_shader_image_load_store,
    SF_GL_EXT_shader_integer_mix,
    SF_GL_EXT_shader_texture_lod,
    SF_GL_EXT_shadow_funcs,
    SF_GL_EXT_shadow_samplers,
    SF_GL_EXT_shared_texture_palette,
    SF_GL_EXT_stencil_clear_tag,
    SF_GL_EXT_stencil_two_side,
    SF_GL_EXT_stencil_wrap,
    SF_GL_EXT_subtexture,
    SF_GL_EXT_texture,
    SF_GL_EXT_texture3D,
    SF_GL_EXT_texture_array,
    SF_GL_EXT_texture_buffer_object,
    SF_GL_EXT_texture_compression_dxt1,
    SF_GL_EXT_texture_compression_latc,
    SF_GL_EXT_texture_compression_rgtc,
    SF_GL_EXT_texture_compression_s3tc,
    SF_GL_EXT_texture_cube_map,
    SF_GL_EXT_texture_env_add,
    SF_GL_EXT_texture_env_combine,
    SF_GL_EXT_texture_env_dot3,
    SF_GL_EXT_texture_filter_anisotropic,
    SF_GL_EXT_texture_format_BGRA8888,
    SF_GL_EXT_texture_integer,
    SF_GL_EXT_texture_lod_bias,
    SF_GL_EXT_texture_mirror_clamp,
    SF_GL_EXT_texture_object,
    SF_GL_EXT_texture_perturb_normal,
    SF_GL_EXT_texture_rg,
    SF_GL_EXT_texture_sRGB,
    SF_GL_EXT_texture_sRGB_decode,
    SF_GL_EXT_texture_shared_exponent,
    SF_GL_EXT_texture_snorm,
    SF_GL_EXT_texture_storage,
    SF_GL_EXT_texture_swizzle,
    SF_GL_EXT_texture_type_2_10_10_10_REV,
    SF_GL_EXT_timer_query,
    SF_GL_EXT_transform_feedback,
    SF_GL_EXT_unpack_subimage,
    SF_GL_EXT_vertex_array,
    SF_GL_EXT_vertex_array_bgra,
    SF_GL_EXT_vertex_attrib_64bit,
    SF_GL_EXT_vertex_shader,
    SF_GL_EXT_vertex_weighting,
    SF_GL_EXT_x11_sync_object,
    SF_GL_FJ_shader_binary_GCCSO,
    SF_GL_GREMEDY_frame_terminator,
    SF_GL_GREMEDY_string_marker,
    SF_GL_HP_convolution_border_modes,
    SF_GL_HP_image_transform,
    SF_GL_HP_occlusion_test,
    SF_GL_HP_texture_lighting,
    SF_GL_IBM_cull_vertex,
    SF_GL_IBM_multimode_draw_arrays,
    SF_GL_IBM_rasterpos_clip,
    SF_GL_IBM_static_data,
    SF_GL_IBM_texture_mirrored_repeat,
    SF_GL_IBM_vertex_array_lists,
    SF_GL_IMG_multisampled_render_to_texture,
    SF_GL_IMG_program_binary,
    SF_GL_IMG_read_format,
    SF_GL_IMG_shader_binary,
    SF_GL_IMG_texture_compression_pvrtc,
    SF_GL_IMG_texture_compression_pvrtc2,
    SF_GL_INGR_blend_func_separate,
    SF_GL_INGR_color_clamp,
    SF_GL_INGR_interlace_read,
    SF_GL_INTEL_fragment_shader_ordering,
    SF_GL_INTEL_map_texture,
    SF_GL_INTEL_parallel_arrays,
    SF_GL_INTEL_performance_query,
    SF_GL_KHR_debug,
    SF_GL_KHR_texture_compression_astc_hdr,
    SF_GL_KHR_texture_compression_astc_ldr,
    SF_GL_MESAX_texture_stack,
    SF_GL_MESA_pack_invert,
    SF_GL_MESA_resize_buffers,
    SF_GL_MESA_window_pos,
    SF_GL_MESA_ycbcr_texture,
    SF_GL_NVX_conditional_render,
    SF_GL_NVX_gpu_memory_info,
    SF_GL_NV_bindless_multi_draw_indirect,
    SF_GL_NV_bindless_texture,
    SF_GL_NV_blend_equation_advanced,
    SF_GL_NV_blend_equation_advanced_coherent,
    SF_GL_NV_blend_square,
    SF_GL_NV_compute_program5,
    SF_GL_NV_conditional_render,
    SF_GL_NV_copy_buffer,
    SF_GL_NV_copy_depth_to_color,
    SF_GL_NV_copy_image,
    SF_GL_NV_coverage_sample,
    SF_GL_NV_deep_texture3D,
    SF_GL_NV_depth_buffer_float,
    SF_GL_NV_depth_clamp,
    SF_GL_NV_depth_nonlinear,
    SF_GL_NV_draw_buffers,
    SF_GL_NV_draw_instanced,
    SF_GL_NV_draw_texture,
    SF_GL_NV_evaluators,
    SF_GL_NV_explicit_attrib_location,
    SF_GL_NV_explicit_multisample,
    SF_GL_NV_fbo_color_attachments,
    SF_GL_NV_fence,
    SF_GL_NV_float_buffer,
    SF_GL_NV_fog_distance,
    SF_GL_NV_fragment_program,
    SF_GL_NV_fragment_program2,
    SF_GL_NV_fragment_program4,
    SF_GL_NV_fragment_program_option,
    SF_GL_NV_framebuffer_blit,
    SF_GL_NV_framebuffer_multisample,
    SF_GL_NV_framebuffer_multisample_coverage,
    SF_GL_NV_generate_mipmap_sRGB,
    SF_GL_NV_geometry_program4,
    SF_GL_NV_geometry_shader4,
    SF_GL_NV_gpu_program4,
    SF_GL_NV_gpu_program5,
    SF_GL_NV_gpu_program5_mem_extended,
    SF_GL_NV_gpu_shader5,
    SF_GL_NV_half_float,
    SF_GL_NV_instanced_arrays,
    SF_GL_NV_light_max_exponent,
    SF_GL_NV_multisample_coverage,
    SF_GL_NV_multisample_filter_hint,
    SF_GL_NV_non_square_matrices,
    SF_GL_NV_occlusion_query,
    SF_GL_NV_packed_depth_stencil,
    SF_GL_NV_parameter_buffer_object,
    SF_GL_NV_parameter_buffer_object2,
    SF_GL_NV_path_rendering,
    SF_GL_NV_pixel_data_range,
    SF_GL_NV_point_sprite,
    SF_GL_NV_present_video,
    SF_GL_NV_primitive_restart,
    SF_GL_NV_read_buffer,
    SF_GL_NV_read_buffer_front,
    SF_GL_NV_read_depth,
    SF_GL_NV_read_depth_stencil,
    SF_GL_NV_read_stencil,
    SF_GL_NV_register_combiners,
    SF_GL_NV_register_combiners2,
    SF_GL_NV_sRGB_formats,
    SF_GL_NV_shader_atomic_counters,
    SF_GL_NV_shader_atomic_float,
    SF_GL_NV_shader_buffer_load,
    SF_GL_NV_shader_buffer_store,
    SF_GL_NV_shader_storage_buffer_object,
    SF_GL_NV_shadow_samplers_array,
    SF_GL_NV_shadow_samplers_cube,
    SF_GL_NV_tessellation_program5,
    SF_GL_NV_texgen_emboss,
    SF_GL_NV_texgen_reflection,
    SF_GL_NV_texture_barrier,
    SF_GL_NV_texture_border_clamp,
    SF_GL_NV_texture_compression_s3tc_update,
    SF_GL_NV_texture_compression_vtc,
    SF_GL_NV_texture_env_combine4,
    SF_GL_NV_texture_expand_normal,
    SF_GL_NV_texture_multisample,
    SF_GL_NV_texture_npot_2D_mipmap,
    SF_GL_NV_texture_rectangle,
    SF_GL_NV_texture_shader,
    SF_GL_NV_texture_shader2,
    SF_GL_NV_texture_shader3,
    SF_GL_NV_transform_feedback,
    SF_GL_NV_transform_feedback2,
    SF_GL_NV_vdpau_interop,
    SF_GL_NV_vertex_array_range,
    SF_GL_NV_vertex_array_range2,
    SF_GL_NV_vertex_attrib_integer_64bit,
    SF_GL_NV_vertex_buffer_unified_memory,
    SF_GL_NV_vertex_program,
    SF_GL_NV_vertex_program1_1,
    SF_GL_NV_vertex_program2,
    SF_GL_NV_vertex_program2_option,
    SF_GL_NV_vertex_program3,
    SF_GL_NV_vertex_program4,
    SF_GL_NV_video_capture,
    SF_GL_OES_EGL_image,
    SF_GL_OES_EGL_image_external,
    SF_GL_OES_byte_coordinates,
    SF_GL_OES_compressed_ETC1_RGB8_texture,
    SF_GL_OES_compressed_paletted_texture,
    SF_GL_OES_depth24,
    SF_GL_OES_depth32,
    SF_GL_OES_depth_texture,
    SF_GL_OES_element_index_uint,
    SF_GL_OES_fbo_render_mipmap,
    SF_GL_OES_fixed_point,
    SF_GL_OES_fragment_precision_high,
    SF_GL_OES_get_program_binary,
    SF_GL_OES_mapbuffer,
    SF_GL_OES_packed_depth_stencil,
    SF_GL_OES_query_matrix,
    SF_GL_OES_read_format,
    SF_GL_OES_required_internalformat,
    SF_GL_OES_rgb8_rgba8,
    SF_GL_OES_single_precision,
    SF_GL_OES_standard_derivatives,
    SF_GL_OES_stencil1,
    SF_GL_OES_stencil4,
    SF_GL_OES_surfaceless_context,
    SF_GL_OES_texture_3D,
    SF_GL_OES_texture_compression_astc,
    SF_GL_OES_texture_float,
    SF_GL_OES_texture_float_linear,
    SF_GL_OES_texture_half_float,
    SF_GL_OES_texture_half_float_linear,
    SF_GL_OES_texture_npot,
    SF_GL_OES_vertex_array_object,
    SF_GL_OES_vertex_half_float,
    SF_GL_OES_vertex_type_10_10_10_2,
    SF_GL_OML_interlace,
    SF_GL_OML_resample,
    SF_GL_OML_subsample,
    SF_GL_PGI_misc_hints,
    SF_GL_PGI_vertex_hints,
    SF_GL_QCOM_alpha_test,
    SF_GL_QCOM_binning_control,
    SF_GL_QCOM_driver_control,
    SF_GL_QCOM_extended_get,
    SF_GL_QCOM_extended_get2,
    SF_GL_QCOM_perfmon_global_mode,
    SF_GL_QCOM_tiled_rendering,
    SF_GL_QCOM_writeonly_rendering,
    SF_GL_REND_screen_coordinates,
    SF_GL_S3_s3tc,
    SF_GL_SGIS_detail_texture,
    SF_GL_SGIS_fog_function,
    SF_GL_SGIS_generate_mipmap,
    SF_GL_SGIS_multisample,
    SF_GL_SGIS_pixel_texture,
    SF_GL_SGIS_point_line_texgen,
    SF_GL_SGIS_point_parameters,
    SF_GL_SGIS_sharpen_texture,
    SF_GL_SGIS_texture4D,
    SF_GL_SGIS_texture_border_clamp,
    SF_GL_SGIS_texture_color_mask,
    SF_GL_SGIS_texture_edge_clamp,
    SF_GL_SGIS_texture_filter4,
    SF_GL_SGIS_texture_lod,
    SF_GL_SGIS_texture_select,
    SF_GL_SGIX_async,
    SF_GL_SGIX_async_histogram,
    SF_GL_SGIX_async_pixel,
    SF_GL_SGIX_blend_alpha_minmax,
    SF_GL_SGIX_calligraphic_fragment,
    SF_GL_SGIX_clipmap,
    SF_GL_SGIX_convolution_accuracy,
    SF_GL_SGIX_depth_pass_instrument,
    SF_GL_SGIX_depth_texture,
    SF_GL_SGIX_flush_raster,
    SF_GL_SGIX_fog_offset,
    SF_GL_SGIX_fragment_lighting,
    SF_GL_SGIX_framezoom,
    SF_GL_SGIX_igloo_interface,
    SF_GL_SGIX_instruments,
    SF_GL_SGIX_interlace,
    SF_GL_SGIX_ir_instrument1,
    SF_GL_SGIX_list_priority,
    SF_GL_SGIX_pixel_texture,
    SF_GL_SGIX_pixel_tiles,
    SF_GL_SGIX_polynomial_ffd,
    SF_GL_SGIX_reference_plane,
    SF_GL_SGIX_resample,
    SF_GL_SGIX_scalebias_hint,
    SF_GL_SGIX_shadow,
    SF_GL_SGIX_shadow_ambient,
    SF_GL_SGIX_sprite,
    SF_GL_SGIX_subsample,
    SF_GL_SGIX_tag_sample_buffer,
    SF_GL_SGIX_texture_add_env,
    SF_GL_SGIX_texture_coordinate_clamp,
    SF_GL_SGIX_texture_lod_bias,
    SF_GL_SGIX_texture_multi_buffer,
    SF_GL_SGIX_texture_scale_bias,
    SF_GL_SGIX_vertex_preclip,
    SF_GL_SGIX_ycrcb,
    SF_GL_SGIX_ycrcb_subsample,
    SF_GL_SGIX_ycrcba,
    SF_GL_SGI_color_matrix,
    SF_GL_SGI_color_table,
    SF_GL_SGI_texture_color_table,
    SF_GL_SUNX_constant_data,
    SF_GL_SUN_convolution_border_modes,
    SF_GL_SUN_global_alpha,
    SF_GL_SUN_mesh_array,
    SF_GL_SUN_slice_accum,
    SF_GL_SUN_triangle_list,
    SF_GL_SUN_vertex,
    SF_GL_VERSION_1_1,
    SF_GL_VERSION_1_2,
    SF_GL_VERSION_1_3,
    SF_GL_VERSION_1_4,
    SF_GL_VERSION_1_5,
    SF_GL_VERSION_2_0,
    SF_GL_VERSION_2_1,
    SF_GL_VERSION_3_0,
    SF_GL_VERSION_3_1,
    SF_GL_VERSION_3_2,
    SF_GL_VERSION_3_3,
    SF_GL_VERSION_4_0,
    SF_GL_VERSION_4_1,
    SF_GL_VERSION_4_2,
    SF_GL_VERSION_4_3,
    SF_GL_VERSION_4_4,
    SF_GL_VIV_shader_binary,
    SF_GL_WIN_phong_shading,
    SF_GL_WIN_specular_fog,
    SF_GL_Extension_Count
};

class GLInterface
{
    friend class GL::GraphicsDeviceImmediate;
public:
    GLInterface();
    GLInterface(const GLInterface& other);
    
    virtual bool CheckExtension(GLExtensionType ext);
    virtual bool CheckGLVersion(unsigned reqMajor, unsigned reqMinor);

    virtual void glActiveTexture(GLenum  a0)= 0;
    virtual void glAttachShader(GLuint  a0, GLuint  a1)= 0;
    virtual void glBindAttribLocation(GLuint  a0, GLuint  a1, const GLchar * a2)= 0;
    virtual void glBindBuffer(GLenum  a0, GLuint  a1)= 0;
    virtual  void glBindFragDataLocation(GLuint  a0, GLuint  a1, const GLchar * a2)= 0;
    virtual void glBindFramebuffer(GLenum  a0, GLuint  a1)= 0;
    virtual void glBindProgramPipeline(GLuint  a0)= 0;
    virtual void glBindRenderbuffer(GLenum  a0, GLuint  a1)= 0;
    virtual void glBindTexture(GLenum  a0, GLuint  a1)= 0;
    virtual void glBindVertexArray(GLuint  a0)= 0;
    virtual void glBlendEquation(GLenum  a0)= 0;
    virtual void glBlendEquationSeparate(GLenum  a0, GLenum  a1)= 0;
    virtual void glBlendFunc(GLenum  a0, GLenum  a1)= 0;
    virtual void glBlendFuncSeparate(GLenum  a0, GLenum  a1, GLenum  a2, GLenum  a3)= 0;
    virtual void glBufferData(GLenum  a0, GLsizeiptr  a1, const void * a2, GLenum  a3)= 0;
    virtual void glBufferSubData(GLenum  a0, GLintptr  a1, GLsizeiptr  a2, const void * a3)= 0;
    virtual GLenum glCheckFramebufferStatus(GLenum  a0)= 0;
    virtual void glClear(GLbitfield  a0)= 0;
    virtual void glClearColor(GLclampf  a0, GLclampf  a1, GLclampf  a2, GLclampf  a3)= 0;
    virtual GLenum glClientWaitSync(GLsync  a0, GLbitfield  a1, GLuint64  a2)= 0;
    virtual void glColorMask(GLboolean  a0, GLboolean  a1, GLboolean  a2, GLboolean  a3)= 0;
    virtual void glCompileShader(GLuint  a0)= 0;
    virtual void glCompressedTexImage2D(GLenum  a0, GLint  a1, GLenum  a2, GLsizei  a3, GLsizei  a4, GLint  a5, GLsizei  a6, const void * a7)= 0;
    virtual GLuint glCreateProgram()= 0;
    virtual GLuint glCreateShader(GLenum  a0)= 0;
    virtual void glDebugMessageCallback(GLDEBUGPROCKHR  a0, const void * a1)= 0;
    virtual void glDebugMessageControl(GLenum  a0, GLenum  a1, GLenum  a2, GLsizei  a3, const GLuint * a4, GLboolean  a5)= 0;
    virtual void glDeleteBuffers(GLsizei  a0, const GLuint * a1)= 0;
    virtual void glDeleteFramebuffers(GLsizei  a0, const GLuint * a1)= 0;
    virtual void glDeleteProgram(GLuint  a0)= 0;
    virtual void glDeleteProgramPipelines(GLsizei  a0, const GLuint * a1)= 0;
    virtual void glDeleteRenderbuffers(GLsizei  a0, const GLuint * a1)= 0;
    virtual void glDeleteShader(GLuint  a0)= 0;
    virtual void glDeleteSync(GLsync  a0)= 0;
    virtual void glDeleteTextures(GLsizei  a0, const GLuint * a1)= 0;
    virtual void glDeleteVertexArrays(GLsizei  a0, const GLuint * a1)= 0;
    virtual void glDepthFunc(GLenum  a0)= 0;
    virtual void glDepthMask(GLboolean  a0)= 0;
    virtual void glDisable(GLenum  a0)= 0;
    virtual void glDisableVertexAttribArray(GLuint  a0)= 0;
    virtual void glDrawArrays(GLenum  a0, GLint  a1, GLsizei  a2)= 0;
    virtual void glDrawElements(GLenum  a0, GLsizei  a1, GLenum  a2, const GLvoid * a3)= 0;
    virtual void glDrawElementsInstanced(GLenum  a0, GLsizei  a1, GLenum  a2, const void * a3, GLsizei  a4)= 0;
    virtual void glEnable(GLenum  a0)= 0;
    virtual void glEnableVertexAttribArray(GLuint  a0)= 0;
    virtual GLsync glFenceSync(GLenum  a0, GLbitfield  a1)= 0;
    virtual void glFlush()= 0;
    virtual void glFlushMappedBufferRange(GLenum  a0, GLintptr  a1, GLsizeiptr  a2)= 0;
    virtual void glFramebufferRenderbuffer(GLenum  a0, GLenum  a1, GLenum  a2, GLuint  a3)= 0;
    virtual void glFramebufferTexture2D(GLenum  a0, GLenum  a1, GLenum  a2, GLuint  a3, GLint  a4)= 0;
    virtual void glGenBuffers(GLsizei  a0, GLuint * a1)= 0;
    virtual void glGenFramebuffers(GLsizei  a0, GLuint * a1)= 0;
    virtual void glGenProgramPipelines(GLsizei  a0, GLuint * a1)= 0;
    virtual void glGenRenderbuffers(GLsizei  a0, GLuint * a1)= 0;
    virtual void glGenTextures(GLsizei  a0, GLuint * a1)= 0;
    virtual void glGenVertexArrays(GLsizei  a0, GLuint * a1)= 0;
    virtual void glGenerateMipmap(GLenum  a0)= 0;
    virtual void glGetActiveUniform(GLuint  a0, GLuint  a1, GLsizei  a2, GLsizei * a3, GLint * a4, GLenum * a5, GLchar * a6)= 0;
    virtual GLenum glGetError()= 0;
    virtual void glGetFloatv(GLenum  a0, GLfloat * a1)= 0;
    virtual GLint glGetFragDataLocation(GLuint  a0, const GLchar * a1)= 0;
    virtual void glGetFramebufferAttachmentParameteriv(GLenum  a0, GLenum  a1, GLenum  a2, GLint * a3)= 0;
    virtual void glGetIntegerv(GLenum  a0, GLint * a1)= 0;
    virtual void glGetProgramBinary(GLuint  a0, GLsizei  a1, GLsizei * a2, GLenum * a3, void * a4)= 0;
    virtual void glGetProgramInfoLog(GLuint  a0, GLsizei  a1, GLsizei * a2, GLchar * a3)= 0;
    virtual void glGetProgramiv(GLuint  a0, GLenum  a1, GLint * a2)= 0;
    virtual void glGetRenderbufferParameteriv(GLenum  a0, GLenum  a1, GLint * a2)= 0;
    virtual void glGetShaderInfoLog(GLuint  a0, GLsizei  a1, GLsizei * a2, GLchar * a3)= 0;
    virtual void glGetShaderiv(GLuint  a0, GLenum  a1, GLint * a2)= 0;
    virtual const GLubyte * glGetString(GLenum  a0)= 0;
    virtual const GLubyte * glGetStringi(GLenum  a0, GLuint  a1)= 0;
    virtual void glGetSynciv(GLsync  a0, GLenum  a1, GLsizei  a2, GLsizei * a3, GLint * a4)= 0;
    virtual void glGetTexImage(GLenum  a0, GLint  a1, GLenum  a2, GLenum  a3, GLvoid * a4)= 0;
    virtual void glGetTexLevelParameteriv(GLenum  a0, GLint  a1, GLenum  a2, GLint * a3)= 0;
    virtual void glGetTexParameteriv(GLenum  a0, GLenum  a1, GLint * a2)= 0;
    virtual GLint glGetUniformLocation(GLuint  a0, const GLchar * a1)= 0;
    virtual GLboolean glIsFramebuffer(GLuint  a0)= 0;
    virtual GLboolean glIsProgram(GLuint  a0)= 0;
    virtual GLboolean glIsRenderbuffer(GLuint  a0)= 0;
    virtual void glLinkProgram(GLuint  a0)= 0;
    virtual void * glMapBuffer(GLenum  a0, GLenum  a1)= 0;
    virtual void * glMapBufferRange(GLenum  a0, GLintptr  a1, GLsizeiptr  a2, GLbitfield  a3)= 0;
    virtual void glPixelStorei(GLenum  a0, GLint  a1)= 0;
    virtual void glPolygonMode(GLenum  a0, GLenum  a1)= 0;
    virtual void glPopGroupMarker()= 0;
    virtual void glProgramBinary(GLuint  a0, GLenum  a1, const void * a2, GLint  a3)= 0;
    virtual void glProgramParameteri(GLuint  a0, GLenum  a1, GLint  a2)= 0;
    virtual void glProgramUniform1fv(GLuint  a0, GLint  a1, GLsizei  a2, const GLfloat * a3)= 0;
    virtual void glProgramUniform1iv(GLuint  a0, GLint  a1, GLsizei  a2, const GLint * a3)= 0;
    virtual void glProgramUniform2fv(GLuint  a0, GLint  a1, GLsizei  a2, const GLfloat * a3)= 0;
    virtual void glProgramUniform3fv(GLuint  a0, GLint  a1, GLsizei  a2, const GLfloat * a3)= 0;
    virtual void glProgramUniform4fv(GLuint  a0, GLint  a1, GLsizei  a2, const GLfloat * a3)= 0;
    virtual void glProgramUniformMatrix4fv(GLuint  a0, GLint  a1, GLsizei  a2, GLboolean  a3, const GLfloat * a4)= 0;
    virtual void glPushGroupMarker(GLsizei  a0, const GLchar * a1)= 0;
    virtual void glReadPixels(GLint  a0, GLint  a1, GLsizei  a2, GLsizei  a3, GLenum  a4, GLenum  a5, GLvoid * a6)= 0;
    virtual void glReleaseShaderCompiler()= 0;
    virtual void glRenderbufferStorage(GLenum  a0, GLenum  a1, GLsizei  a2, GLsizei  a3)= 0;
    virtual void glScissor(GLint  a0, GLint  a1, GLsizei  a2, GLsizei  a3)= 0;
    virtual void glShaderSource(GLuint  a0, GLsizei  a1, GLSHADERSOURCEA2TYPE a2, const GLint * a3)= 0;
    virtual void glStencilFunc(GLenum  a0, GLint  a1, GLuint  a2)= 0;
    virtual void glStencilMask(GLuint  a0)= 0;
    virtual void glStencilOp(GLenum  a0, GLenum  a1, GLenum  a2)= 0;
    virtual  void glStringMarker(GLsizei  a0, const void * a1)= 0;
    virtual void glTexImage2D(GLenum  a0, GLint  a1, GLint  a2, GLsizei  a3, GLsizei  a4, GLint  a5, GLenum  a6, GLenum  a7, const GLvoid * a8)= 0;
    virtual void glTexParameteri(GLenum  a0, GLenum  a1, GLint  a2)= 0;
    virtual void glTexSubImage2D(GLenum  a0, GLint  a1, GLint  a2, GLint  a3, GLsizei  a4, GLsizei  a5, GLenum  a6, GLenum  a7, const GLvoid * a8)= 0;
    virtual void glUniform1f(GLint  a0, GLfloat  a1)= 0;
    virtual void glUniform1fv(GLint  a0, GLsizei  a1, const GLfloat * a2)= 0;
    virtual void glUniform1i(GLint  a0, GLint  a1)= 0;
    virtual void glUniform1iv(GLint  a0, GLsizei  a1, const GLint * a2)= 0;
    virtual void glUniform2f(GLint  a0, GLfloat  a1, GLfloat  a2)= 0;
    virtual void glUniform2fv(GLint  a0, GLsizei  a1, const GLfloat * a2)= 0;
    virtual void glUniform3fv(GLint  a0, GLsizei  a1, const GLfloat * a2)= 0;
    virtual void glUniform4fv(GLint  a0, GLsizei  a1, const GLfloat * a2)= 0;
    virtual void glUniformMatrix4fv(GLint  a0, GLsizei  a1, GLboolean  a2, const GLfloat * a3)= 0;
    virtual GLboolean glUnmapBuffer(GLenum  a0)= 0;
    virtual void glUseProgram(GLuint  a0)= 0;
    virtual void glUseProgramStages(GLuint  a0, GLbitfield  a1, GLuint  a2)= 0;
    virtual void glVertexAttribPointer(GLuint  a0, GLint  a1, GLenum  a2, GLboolean  a3, GLsizei  a4, const void * a5)= 0;
    virtual void glViewport(GLint  a0, GLint  a1, GLsizei  a2, GLsizei  a3)= 0;

protected:
    unsigned MajorVersion, MinorVersion;
    bool ExtensionSupported[SF_GL_Extension_Count];
};

class GLImmediate : public GLInterface
{
public:   
    GLImmediate();
    void glActiveTexture(GLenum  a0);
    void glAttachShader(GLuint  a0, GLuint  a1);
    void glBindAttribLocation(GLuint  a0, GLuint  a1, const GLchar * a2);
    void glBindBuffer(GLenum  a0, GLuint  a1);
     void glBindFragDataLocation(GLuint  a0, GLuint  a1, const GLchar * a2);
    void glBindFramebuffer(GLenum  a0, GLuint  a1);
    void glBindProgramPipeline(GLuint  a0);
    void glBindRenderbuffer(GLenum  a0, GLuint  a1);
    void glBindTexture(GLenum  a0, GLuint  a1);
    void glBindVertexArray(GLuint  a0);
    void glBlendEquation(GLenum  a0);
    void glBlendEquationSeparate(GLenum  a0, GLenum  a1);
    void glBlendFunc(GLenum  a0, GLenum  a1);
    void glBlendFuncSeparate(GLenum  a0, GLenum  a1, GLenum  a2, GLenum  a3);
    void glBufferData(GLenum  a0, GLsizeiptr  a1, const void * a2, GLenum  a3);
    void glBufferSubData(GLenum  a0, GLintptr  a1, GLsizeiptr  a2, const void * a3);
    GLenum glCheckFramebufferStatus(GLenum  a0);
    void glClear(GLbitfield  a0);
    void glClearColor(GLclampf  a0, GLclampf  a1, GLclampf  a2, GLclampf  a3);
    GLenum glClientWaitSync(GLsync  a0, GLbitfield  a1, GLuint64  a2);
    void glColorMask(GLboolean  a0, GLboolean  a1, GLboolean  a2, GLboolean  a3);
    void glCompileShader(GLuint  a0);
    void glCompressedTexImage2D(GLenum  a0, GLint  a1, GLenum  a2, GLsizei  a3, GLsizei  a4, GLint  a5, GLsizei  a6, const void * a7);
    GLuint glCreateProgram();
    GLuint glCreateShader(GLenum  a0);
    void glDebugMessageCallback(GLDEBUGPROCKHR  a0, const void * a1);
    void glDebugMessageControl(GLenum  a0, GLenum  a1, GLenum  a2, GLsizei  a3, const GLuint * a4, GLboolean  a5);
    void glDeleteBuffers(GLsizei  a0, const GLuint * a1);
    void glDeleteFramebuffers(GLsizei  a0, const GLuint * a1);
    void glDeleteProgram(GLuint  a0);
    void glDeleteProgramPipelines(GLsizei  a0, const GLuint * a1);
    void glDeleteRenderbuffers(GLsizei  a0, const GLuint * a1);
    void glDeleteShader(GLuint  a0);
    void glDeleteSync(GLsync  a0);
    void glDeleteTextures(GLsizei  a0, const GLuint * a1);
    void glDeleteVertexArrays(GLsizei  a0, const GLuint * a1);
    void glDepthFunc(GLenum  a0);
    void glDepthMask(GLboolean  a0);
    void glDisable(GLenum  a0);
    void glDisableVertexAttribArray(GLuint  a0);
    void glDrawArrays(GLenum  a0, GLint  a1, GLsizei  a2);
    void glDrawElements(GLenum  a0, GLsizei  a1, GLenum  a2, const GLvoid * a3);
    void glDrawElementsInstanced(GLenum  a0, GLsizei  a1, GLenum  a2, const void * a3, GLsizei  a4);
    void glEnable(GLenum  a0);
    void glEnableVertexAttribArray(GLuint  a0);
    GLsync glFenceSync(GLenum  a0, GLbitfield  a1);
    void glFlush();
    void glFlushMappedBufferRange(GLenum  a0, GLintptr  a1, GLsizeiptr  a2);
    void glFramebufferRenderbuffer(GLenum  a0, GLenum  a1, GLenum  a2, GLuint  a3);
    void glFramebufferTexture2D(GLenum  a0, GLenum  a1, GLenum  a2, GLuint  a3, GLint  a4);
    void glGenBuffers(GLsizei  a0, GLuint * a1);
    void glGenFramebuffers(GLsizei  a0, GLuint * a1);
    void glGenProgramPipelines(GLsizei  a0, GLuint * a1);
    void glGenRenderbuffers(GLsizei  a0, GLuint * a1);
    void glGenTextures(GLsizei  a0, GLuint * a1);
    void glGenVertexArrays(GLsizei  a0, GLuint * a1);
    void glGenerateMipmap(GLenum  a0);
    void glGetActiveUniform(GLuint  a0, GLuint  a1, GLsizei  a2, GLsizei * a3, GLint * a4, GLenum * a5, GLchar * a6);
    GLenum glGetError();
    void glGetFloatv(GLenum  a0, GLfloat * a1);
    GLint glGetFragDataLocation(GLuint  a0, const GLchar * a1);
    void glGetFramebufferAttachmentParameteriv(GLenum  a0, GLenum  a1, GLenum  a2, GLint * a3);
    void glGetIntegerv(GLenum  a0, GLint * a1);
    void glGetProgramBinary(GLuint  a0, GLsizei  a1, GLsizei * a2, GLenum * a3, void * a4);
    void glGetProgramInfoLog(GLuint  a0, GLsizei  a1, GLsizei * a2, GLchar * a3);
    void glGetProgramiv(GLuint  a0, GLenum  a1, GLint * a2);
    void glGetRenderbufferParameteriv(GLenum  a0, GLenum  a1, GLint * a2);
    void glGetShaderInfoLog(GLuint  a0, GLsizei  a1, GLsizei * a2, GLchar * a3);
    void glGetShaderiv(GLuint  a0, GLenum  a1, GLint * a2);
    const GLubyte * glGetString(GLenum  a0);
    const GLubyte * glGetStringi(GLenum  a0, GLuint  a1);
    void glGetSynciv(GLsync  a0, GLenum  a1, GLsizei  a2, GLsizei * a3, GLint * a4);
    void glGetTexImage(GLenum  a0, GLint  a1, GLenum  a2, GLenum  a3, GLvoid * a4);
    void glGetTexLevelParameteriv(GLenum  a0, GLint  a1, GLenum  a2, GLint * a3);
    void glGetTexParameteriv(GLenum  a0, GLenum  a1, GLint * a2);
    GLint glGetUniformLocation(GLuint  a0, const GLchar * a1);
    GLboolean glIsFramebuffer(GLuint  a0);
    GLboolean glIsProgram(GLuint  a0);
    GLboolean glIsRenderbuffer(GLuint  a0);
    void glLinkProgram(GLuint  a0);
    void * glMapBuffer(GLenum  a0, GLenum  a1);
    void * glMapBufferRange(GLenum  a0, GLintptr  a1, GLsizeiptr  a2, GLbitfield  a3);
    void glPixelStorei(GLenum  a0, GLint  a1);
    void glPolygonMode(GLenum  a0, GLenum  a1);
    void glPopGroupMarker();
    void glProgramBinary(GLuint  a0, GLenum  a1, const void * a2, GLint  a3);
    void glProgramParameteri(GLuint  a0, GLenum  a1, GLint  a2);
    void glProgramUniform1fv(GLuint  a0, GLint  a1, GLsizei  a2, const GLfloat * a3);
    void glProgramUniform1iv(GLuint  a0, GLint  a1, GLsizei  a2, const GLint * a3);
    void glProgramUniform2fv(GLuint  a0, GLint  a1, GLsizei  a2, const GLfloat * a3);
    void glProgramUniform3fv(GLuint  a0, GLint  a1, GLsizei  a2, const GLfloat * a3);
    void glProgramUniform4fv(GLuint  a0, GLint  a1, GLsizei  a2, const GLfloat * a3);
    void glProgramUniformMatrix4fv(GLuint  a0, GLint  a1, GLsizei  a2, GLboolean  a3, const GLfloat * a4);
    void glPushGroupMarker(GLsizei  a0, const GLchar * a1);
    void glReadPixels(GLint  a0, GLint  a1, GLsizei  a2, GLsizei  a3, GLenum  a4, GLenum  a5, GLvoid * a6);
    void glReleaseShaderCompiler();
    void glRenderbufferStorage(GLenum  a0, GLenum  a1, GLsizei  a2, GLsizei  a3);
    void glScissor(GLint  a0, GLint  a1, GLsizei  a2, GLsizei  a3);
    void glShaderSource(GLuint  a0, GLsizei  a1, GLSHADERSOURCEA2TYPE a2, const GLint * a3);
    void glStencilFunc(GLenum  a0, GLint  a1, GLuint  a2);
    void glStencilMask(GLuint  a0);
    void glStencilOp(GLenum  a0, GLenum  a1, GLenum  a2);
     void glStringMarker(GLsizei  a0, const void * a1);
    void glTexImage2D(GLenum  a0, GLint  a1, GLint  a2, GLsizei  a3, GLsizei  a4, GLint  a5, GLenum  a6, GLenum  a7, const GLvoid * a8);
    void glTexParameteri(GLenum  a0, GLenum  a1, GLint  a2);
    void glTexSubImage2D(GLenum  a0, GLint  a1, GLint  a2, GLint  a3, GLsizei  a4, GLsizei  a5, GLenum  a6, GLenum  a7, const GLvoid * a8);
    void glUniform1f(GLint  a0, GLfloat  a1);
    void glUniform1fv(GLint  a0, GLsizei  a1, const GLfloat * a2);
    void glUniform1i(GLint  a0, GLint  a1);
    void glUniform1iv(GLint  a0, GLsizei  a1, const GLint * a2);
    void glUniform2f(GLint  a0, GLfloat  a1, GLfloat  a2);
    void glUniform2fv(GLint  a0, GLsizei  a1, const GLfloat * a2);
    void glUniform3fv(GLint  a0, GLsizei  a1, const GLfloat * a2);
    void glUniform4fv(GLint  a0, GLsizei  a1, const GLfloat * a2);
    void glUniformMatrix4fv(GLint  a0, GLsizei  a1, GLboolean  a2, const GLfloat * a3);
    GLboolean glUnmapBuffer(GLenum  a0);
    void glUseProgram(GLuint  a0);
    void glUseProgramStages(GLuint  a0, GLbitfield  a1, GLuint  a2);
    void glVertexAttribPointer(GLuint  a0, GLint  a1, GLenum  a2, GLboolean  a3, GLsizei  a4, const void * a5);
    void glViewport(GLint  a0, GLint  a1, GLsizei  a2, GLsizei  a3);

public:
    enum VerbosityType
    {
        Verbosity_Errors,
        Verbosity_Warnings,
        Verbosity_Info,
        Verbosity_DumpCalls,
    };
    void Init(unsigned verbosity=Verbosity_Errors);
    
    class ScopedGLErrorCheck
    {
    public:
        ScopedGLErrorCheck(const char* func) : FuncName(func)
        {
            SF_UNUSED(FuncName); // Release warning
            performErrorCheck("before");
        }
        ~ScopedGLErrorCheck()
        {
            performErrorCheck("after");
        }
    private:
        void performErrorCheck(const char* timing)
        {
            SF_UNUSED(timing);
            #ifdef SF_BUILD_DEBUG
                GLenum error = ::glGetError(); SF_UNUSED(error);
                static const char* errorMessages[] = {
                    "GL_INVALID_ENUM",
                    "GL_INVALID_VALUE",
                    "GL_INVALID_OPERATION",
                    "GL_STACK_OVERFLOW",
                    "GL_STACK_UNDERFLOW",
                    "GL_OUT_OF_MEMORY" };
                if (error >= GL_INVALID_ENUM && error <= GL_OUT_OF_MEMORY)
                {
                    SF_DEBUG_ASSERT3(!error, "GL error %s function %s (error code = %s)\n", timing, FuncName, errorMessages[error-GL_INVALID_ENUM] );
                }
                else
                {
                    SF_DEBUG_ASSERT3(!error, "GL error %s function %s (non-standard error code = %d)\n", timing, FuncName, error );
                }
            #endif
        }
        const char * FuncName;
    };

private:
    VerbosityType Verbosity;
    void initExtensions(unsigned verbosity);
    
#if defined(SF_GL_RUNTIME_LINK)
    PFNGLACTIVETEXTUREPROC p_glActiveTexture;
    PFNGLATTACHSHADERPROC p_glAttachShader;
    PFNGLBINDATTRIBLOCATIONPROC p_glBindAttribLocation;
    PFNGLBINDBUFFERPROC p_glBindBuffer;
    PFNGLBINDFRAGDATALOCATIONPROC p_glBindFragDataLocation;
    PFNGLBINDFRAMEBUFFERPROC p_glBindFramebuffer;
    PFNGLBINDPROGRAMPIPELINEPROC p_glBindProgramPipeline;
    PFNGLBINDRENDERBUFFERPROC p_glBindRenderbuffer;
    PFNGLBINDVERTEXARRAYPROC p_glBindVertexArray;
    PFNGLBLENDEQUATIONPROC p_glBlendEquation;
    PFNGLBLENDEQUATIONSEPARATEPROC p_glBlendEquationSeparate;
    PFNGLBLENDFUNCSEPARATEPROC p_glBlendFuncSeparate;
    PFNGLBUFFERDATAPROC p_glBufferData;
    PFNGLBUFFERSUBDATAPROC p_glBufferSubData;
    PFNGLCHECKFRAMEBUFFERSTATUSPROC p_glCheckFramebufferStatus;
    PFNGLCLIENTWAITSYNCPROC p_glClientWaitSync;
    PFNGLCOMPILESHADERPROC p_glCompileShader;
    PFNGLCOMPRESSEDTEXIMAGE2DPROC p_glCompressedTexImage2D;
    PFNGLCREATEPROGRAMPROC p_glCreateProgram;
    PFNGLCREATESHADERPROC p_glCreateShader;
    PFNGLDEBUGMESSAGECALLBACKPROC p_glDebugMessageCallback;
    PFNGLDEBUGMESSAGECONTROLPROC p_glDebugMessageControl;
    PFNGLDELETEBUFFERSPROC p_glDeleteBuffers;
    PFNGLDELETEFRAMEBUFFERSPROC p_glDeleteFramebuffers;
    PFNGLDELETEPROGRAMPROC p_glDeleteProgram;
    PFNGLDELETEPROGRAMPIPELINESPROC p_glDeleteProgramPipelines;
    PFNGLDELETERENDERBUFFERSPROC p_glDeleteRenderbuffers;
    PFNGLDELETESHADERPROC p_glDeleteShader;
    PFNGLDELETESYNCPROC p_glDeleteSync;
    PFNGLDELETEVERTEXARRAYSPROC p_glDeleteVertexArrays;
    PFNGLDISABLEVERTEXATTRIBARRAYPROC p_glDisableVertexAttribArray;
    PFNGLDRAWELEMENTSINSTANCEDPROC p_glDrawElementsInstanced;
    PFNGLENABLEVERTEXATTRIBARRAYPROC p_glEnableVertexAttribArray;
    PFNGLFENCESYNCPROC p_glFenceSync;
    PFNGLFLUSHMAPPEDBUFFERRANGEPROC p_glFlushMappedBufferRange;
    PFNGLFRAMEBUFFERRENDERBUFFERPROC p_glFramebufferRenderbuffer;
    PFNGLFRAMEBUFFERTEXTURE2DPROC p_glFramebufferTexture2D;
    PFNGLGENBUFFERSPROC p_glGenBuffers;
    PFNGLGENFRAMEBUFFERSPROC p_glGenFramebuffers;
    PFNGLGENPROGRAMPIPELINESPROC p_glGenProgramPipelines;
    PFNGLGENRENDERBUFFERSPROC p_glGenRenderbuffers;
    PFNGLGENVERTEXARRAYSPROC p_glGenVertexArrays;
    PFNGLGENERATEMIPMAPPROC p_glGenerateMipmap;
    PFNGLGETACTIVEUNIFORMPROC p_glGetActiveUniform;
    PFNGLGETFRAGDATALOCATIONPROC p_glGetFragDataLocation;
    PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC p_glGetFramebufferAttachmentParameteriv;
    PFNGLGETPROGRAMBINARYPROC p_glGetProgramBinary;
    PFNGLGETPROGRAMINFOLOGPROC p_glGetProgramInfoLog;
    PFNGLGETPROGRAMIVPROC p_glGetProgramiv;
    PFNGLGETRENDERBUFFERPARAMETERIVPROC p_glGetRenderbufferParameteriv;
    PFNGLGETSHADERINFOLOGPROC p_glGetShaderInfoLog;
    PFNGLGETSHADERIVPROC p_glGetShaderiv;
    PFNGLGETSTRINGIPROC p_glGetStringi;
    PFNGLGETSYNCIVPROC p_glGetSynciv;
    PFNGLGETTEXIMAGEPROC p_glGetTexImage;
    PFNGLGETTEXLEVELPARAMETERIVPROC p_glGetTexLevelParameteriv;
    PFNGLGETUNIFORMLOCATIONPROC p_glGetUniformLocation;
    PFNGLISFRAMEBUFFERPROC p_glIsFramebuffer;
    PFNGLISPROGRAMPROC p_glIsProgram;
    PFNGLISRENDERBUFFERPROC p_glIsRenderbuffer;
    PFNGLLINKPROGRAMPROC p_glLinkProgram;
    PFNGLMAPBUFFERPROC p_glMapBuffer;
    PFNGLMAPBUFFERRANGEPROC p_glMapBufferRange;
    PFNGLPOLYGONMODEPROC p_glPolygonMode;
    PFNGLPOPGROUPMARKERPROC p_glPopGroupMarker;
    PFNGLPROGRAMBINARYPROC p_glProgramBinary;
    PFNGLPROGRAMPARAMETERIPROC p_glProgramParameteri;
    PFNGLPROGRAMUNIFORM1FVPROC p_glProgramUniform1fv;
    PFNGLPROGRAMUNIFORM1IVPROC p_glProgramUniform1iv;
    PFNGLPROGRAMUNIFORM2FVPROC p_glProgramUniform2fv;
    PFNGLPROGRAMUNIFORM3FVPROC p_glProgramUniform3fv;
    PFNGLPROGRAMUNIFORM4FVPROC p_glProgramUniform4fv;
    PFNGLPROGRAMUNIFORMMATRIX4FVPROC p_glProgramUniformMatrix4fv;
    PFNGLPUSHGROUPMARKERPROC p_glPushGroupMarker;
    PFNGLRELEASESHADERCOMPILERPROC p_glReleaseShaderCompiler;
    PFNGLRENDERBUFFERSTORAGEPROC p_glRenderbufferStorage;
    PFNGLSHADERSOURCEPROC p_glShaderSource;
    PFNGLSTRINGMARKERPROC p_glStringMarker;
    PFNGLUNIFORM1FPROC p_glUniform1f;
    PFNGLUNIFORM1FVPROC p_glUniform1fv;
    PFNGLUNIFORM1IPROC p_glUniform1i;
    PFNGLUNIFORM1IVPROC p_glUniform1iv;
    PFNGLUNIFORM2FPROC p_glUniform2f;
    PFNGLUNIFORM2FVPROC p_glUniform2fv;
    PFNGLUNIFORM3FVPROC p_glUniform3fv;
    PFNGLUNIFORM4FVPROC p_glUniform4fv;
    PFNGLUNIFORMMATRIX4FVPROC p_glUniformMatrix4fv;
    PFNGLUNMAPBUFFERPROC p_glUnmapBuffer;
    PFNGLUSEPROGRAMPROC p_glUseProgram;
    PFNGLUSEPROGRAMSTAGESPROC p_glUseProgramStages;
    PFNGLVERTEXATTRIBPOINTERPROC p_glVertexAttribPointer;

#endif // SF_GL_RUNTIME_LINK
};
}}}
#endif
