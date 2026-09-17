/**************************************************************************

    PublicHeader:   Render
    Filename    :   PS3_ShaderDescs.cpp
    Content     :   PS3 Shader descriptors
    Created     :   12/12/2012
    Authors     :   Automatically generated.

    Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

    Use of this software is subject to the terms of the Autodesk license
    agreement provided at the time of installation or download, or which
    otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/PS3/PS3_Shader.h"
#include "Render/PS3/PS3_ShaderDescs.h"

namespace Scaleform { namespace Render { namespace PS3 {

const char* ShaderUniformNames[Uniform::SU_Count] = 
{
    "cxadd",
    "cxmul",
    "cxmul1",
    "fsize",
    "mvp",
    "offset",
    "scolor",
    "scolor2",
    "srctex",
    "srctexscale",
    "tex",
    "texgen",
    "texscale",
    "vfmuniforms",
    "vfuniforms",
};

const unsigned char Uniform::UniformFlags[Uniform::SU_Count] = 
{
    0,  // cxadd
    0,  // cxmul
    0,  // cxmul1
    0,  // fsize
    0,  // mvp
    0,  // offset
    0,  // scolor
    0,  // scolor2
    0,  // srctex
    0,  // srctexscale
    0,  // tex
    0,  // texgen
    0,  // texscale
    0,  // vfmuniforms
    0,  // vfuniforms
};

bool ShaderDesc::IsShaderVersionSupported(ShaderVersion ver)
{
    switch(ver)
    {
        case ShaderVersion_PS3: return true;
        default: return false;
    }
};

ShaderDesc::ShaderType ShaderDesc::GetShaderTypeForComboIndex(unsigned comboIndex, ShaderVersion ver)
{
    switch(ver)
    {
        case ShaderDesc::ShaderVersion_PS3:
        {
            SF_UNUSED(comboIndex);
            SF_DEBUG_ASSERT(1, "PS3 indicated that ShaderCombo information was not required.");
            return ST_None;
        }
        default: SF_DEBUG_ASSERT1(1, "Invalid shader platform provided (%d)", ver); return ST_None;
    }
};

ShaderDesc::ShaderIndex ShaderDesc::GetShaderIndex(ShaderType type)
{
    switch(type)
    {
        case ST_TexTG:                                           return STI_TexTG;
        case ST_TexTGMul:                                        return STI_TexTGMul;
        case ST_BatchTexTG:                                      return STI_BatchTexTG;
        case ST_BatchTexTGMul:                                   return STI_BatchTexTGMul;
        case ST_InstancedTexTG:                                  return STI_InstancedTexTG;
        case ST_InstancedTexTGMul:                               return STI_InstancedTexTGMul;
        case ST_Position3dTexTG:                                 return STI_Position3dTexTG;
        case ST_Position3dTexTGMul:                              return STI_Position3dTexTGMul;
        case ST_BatchPosition3dTexTG:                            return STI_BatchPosition3dTexTG;
        case ST_BatchPosition3dTexTGMul:                         return STI_BatchPosition3dTexTGMul;
        case ST_InstancedPosition3dTexTG:                        return STI_InstancedPosition3dTexTG;
        case ST_InstancedPosition3dTexTGMul:                     return STI_InstancedPosition3dTexTGMul;
        case ST_TexTGCxform:                                     return STI_TexTGCxform;
        case ST_TexTGCxformMul:                                  return STI_TexTGCxformMul;
        case ST_BatchTexTGCxform:                                return STI_BatchTexTGCxform;
        case ST_BatchTexTGCxformMul:                             return STI_BatchTexTGCxformMul;
        case ST_InstancedTexTGCxform:                            return STI_InstancedTexTGCxform;
        case ST_InstancedTexTGCxformMul:                         return STI_InstancedTexTGCxformMul;
        case ST_Position3dTexTGCxform:                           return STI_Position3dTexTGCxform;
        case ST_Position3dTexTGCxformMul:                        return STI_Position3dTexTGCxformMul;
        case ST_BatchPosition3dTexTGCxform:                      return STI_BatchPosition3dTexTGCxform;
        case ST_BatchPosition3dTexTGCxformMul:                   return STI_BatchPosition3dTexTGCxformMul;
        case ST_InstancedPosition3dTexTGCxform:                  return STI_InstancedPosition3dTexTGCxform;
        case ST_InstancedPosition3dTexTGCxformMul:               return STI_InstancedPosition3dTexTGCxformMul;
        case ST_TexTGEAlpha:                                     return STI_TexTGEAlpha;
        case ST_TexTGEAlphaMul:                                  return STI_TexTGEAlphaMul;
        case ST_BatchTexTGEAlpha:                                return STI_BatchTexTGEAlpha;
        case ST_BatchTexTGEAlphaMul:                             return STI_BatchTexTGEAlphaMul;
        case ST_InstancedTexTGEAlpha:                            return STI_InstancedTexTGEAlpha;
        case ST_InstancedTexTGEAlphaMul:                         return STI_InstancedTexTGEAlphaMul;
        case ST_Position3dTexTGEAlpha:                           return STI_Position3dTexTGEAlpha;
        case ST_Position3dTexTGEAlphaMul:                        return STI_Position3dTexTGEAlphaMul;
        case ST_BatchPosition3dTexTGEAlpha:                      return STI_BatchPosition3dTexTGEAlpha;
        case ST_BatchPosition3dTexTGEAlphaMul:                   return STI_BatchPosition3dTexTGEAlphaMul;
        case ST_InstancedPosition3dTexTGEAlpha:                  return STI_InstancedPosition3dTexTGEAlpha;
        case ST_InstancedPosition3dTexTGEAlphaMul:               return STI_InstancedPosition3dTexTGEAlphaMul;
        case ST_TexTGCxformEAlpha:                               return STI_TexTGCxformEAlpha;
        case ST_TexTGCxformEAlphaMul:                            return STI_TexTGCxformEAlphaMul;
        case ST_BatchTexTGCxformEAlpha:                          return STI_BatchTexTGCxformEAlpha;
        case ST_BatchTexTGCxformEAlphaMul:                       return STI_BatchTexTGCxformEAlphaMul;
        case ST_InstancedTexTGCxformEAlpha:                      return STI_InstancedTexTGCxformEAlpha;
        case ST_InstancedTexTGCxformEAlphaMul:                   return STI_InstancedTexTGCxformEAlphaMul;
        case ST_Position3dTexTGCxformEAlpha:                     return STI_Position3dTexTGCxformEAlpha;
        case ST_Position3dTexTGCxformEAlphaMul:                  return STI_Position3dTexTGCxformEAlphaMul;
        case ST_BatchPosition3dTexTGCxformEAlpha:                return STI_BatchPosition3dTexTGCxformEAlpha;
        case ST_BatchPosition3dTexTGCxformEAlphaMul:             return STI_BatchPosition3dTexTGCxformEAlphaMul;
        case ST_InstancedPosition3dTexTGCxformEAlpha:            return STI_InstancedPosition3dTexTGCxformEAlpha;
        case ST_InstancedPosition3dTexTGCxformEAlphaMul:         return STI_InstancedPosition3dTexTGCxformEAlphaMul;
        case ST_TexTGInv:                                        return STI_TexTGInv;
        case ST_BatchTexTGInv:                                   return STI_BatchTexTGInv;
        case ST_InstancedTexTGInv:                               return STI_InstancedTexTGInv;
        case ST_Position3dTexTGInv:                              return STI_Position3dTexTGInv;
        case ST_BatchPosition3dTexTGInv:                         return STI_BatchPosition3dTexTGInv;
        case ST_InstancedPosition3dTexTGInv:                     return STI_InstancedPosition3dTexTGInv;
        case ST_TexTGCxformInv:                                  return STI_TexTGCxformInv;
        case ST_BatchTexTGCxformInv:                             return STI_BatchTexTGCxformInv;
        case ST_InstancedTexTGCxformInv:                         return STI_InstancedTexTGCxformInv;
        case ST_Position3dTexTGCxformInv:                        return STI_Position3dTexTGCxformInv;
        case ST_BatchPosition3dTexTGCxformInv:                   return STI_BatchPosition3dTexTGCxformInv;
        case ST_InstancedPosition3dTexTGCxformInv:               return STI_InstancedPosition3dTexTGCxformInv;
        case ST_TexTGEAlphaInv:                                  return STI_TexTGEAlphaInv;
        case ST_BatchTexTGEAlphaInv:                             return STI_BatchTexTGEAlphaInv;
        case ST_InstancedTexTGEAlphaInv:                         return STI_InstancedTexTGEAlphaInv;
        case ST_Position3dTexTGEAlphaInv:                        return STI_Position3dTexTGEAlphaInv;
        case ST_BatchPosition3dTexTGEAlphaInv:                   return STI_BatchPosition3dTexTGEAlphaInv;
        case ST_InstancedPosition3dTexTGEAlphaInv:               return STI_InstancedPosition3dTexTGEAlphaInv;
        case ST_TexTGCxformEAlphaInv:                            return STI_TexTGCxformEAlphaInv;
        case ST_BatchTexTGCxformEAlphaInv:                       return STI_BatchTexTGCxformEAlphaInv;
        case ST_InstancedTexTGCxformEAlphaInv:                   return STI_InstancedTexTGCxformEAlphaInv;
        case ST_Position3dTexTGCxformEAlphaInv:                  return STI_Position3dTexTGCxformEAlphaInv;
        case ST_BatchPosition3dTexTGCxformEAlphaInv:             return STI_BatchPosition3dTexTGCxformEAlphaInv;
        case ST_InstancedPosition3dTexTGCxformEAlphaInv:         return STI_InstancedPosition3dTexTGCxformEAlphaInv;
        case ST_TexTGCxformAc:                                   return STI_TexTGCxformAc;
        case ST_TexTGCxformAcMul:                                return STI_TexTGCxformAcMul;
        case ST_BatchTexTGCxformAc:                              return STI_BatchTexTGCxformAc;
        case ST_BatchTexTGCxformAcMul:                           return STI_BatchTexTGCxformAcMul;
        case ST_InstancedTexTGCxformAc:                          return STI_InstancedTexTGCxformAc;
        case ST_InstancedTexTGCxformAcMul:                       return STI_InstancedTexTGCxformAcMul;
        case ST_Position3dTexTGCxformAc:                         return STI_Position3dTexTGCxformAc;
        case ST_Position3dTexTGCxformAcMul:                      return STI_Position3dTexTGCxformAcMul;
        case ST_BatchPosition3dTexTGCxformAc:                    return STI_BatchPosition3dTexTGCxformAc;
        case ST_BatchPosition3dTexTGCxformAcMul:                 return STI_BatchPosition3dTexTGCxformAcMul;
        case ST_InstancedPosition3dTexTGCxformAc:                return STI_InstancedPosition3dTexTGCxformAc;
        case ST_InstancedPosition3dTexTGCxformAcMul:             return STI_InstancedPosition3dTexTGCxformAcMul;
        case ST_TexTGCxformAcEAlpha:                             return STI_TexTGCxformAcEAlpha;
        case ST_TexTGCxformAcEAlphaMul:                          return STI_TexTGCxformAcEAlphaMul;
        case ST_BatchTexTGCxformAcEAlpha:                        return STI_BatchTexTGCxformAcEAlpha;
        case ST_BatchTexTGCxformAcEAlphaMul:                     return STI_BatchTexTGCxformAcEAlphaMul;
        case ST_InstancedTexTGCxformAcEAlpha:                    return STI_InstancedTexTGCxformAcEAlpha;
        case ST_InstancedTexTGCxformAcEAlphaMul:                 return STI_InstancedTexTGCxformAcEAlphaMul;
        case ST_Position3dTexTGCxformAcEAlpha:                   return STI_Position3dTexTGCxformAcEAlpha;
        case ST_Position3dTexTGCxformAcEAlphaMul:                return STI_Position3dTexTGCxformAcEAlphaMul;
        case ST_BatchPosition3dTexTGCxformAcEAlpha:              return STI_BatchPosition3dTexTGCxformAcEAlpha;
        case ST_BatchPosition3dTexTGCxformAcEAlphaMul:           return STI_BatchPosition3dTexTGCxformAcEAlphaMul;
        case ST_InstancedPosition3dTexTGCxformAcEAlpha:          return STI_InstancedPosition3dTexTGCxformAcEAlpha;
        case ST_InstancedPosition3dTexTGCxformAcEAlphaMul:       return STI_InstancedPosition3dTexTGCxformAcEAlphaMul;
        case ST_TexTGCxformAcInv:                                return STI_TexTGCxformAcInv;
        case ST_BatchTexTGCxformAcInv:                           return STI_BatchTexTGCxformAcInv;
        case ST_InstancedTexTGCxformAcInv:                       return STI_InstancedTexTGCxformAcInv;
        case ST_Position3dTexTGCxformAcInv:                      return STI_Position3dTexTGCxformAcInv;
        case ST_BatchPosition3dTexTGCxformAcInv:                 return STI_BatchPosition3dTexTGCxformAcInv;
        case ST_InstancedPosition3dTexTGCxformAcInv:             return STI_InstancedPosition3dTexTGCxformAcInv;
        case ST_TexTGCxformAcEAlphaInv:                          return STI_TexTGCxformAcEAlphaInv;
        case ST_BatchTexTGCxformAcEAlphaInv:                     return STI_BatchTexTGCxformAcEAlphaInv;
        case ST_InstancedTexTGCxformAcEAlphaInv:                 return STI_InstancedTexTGCxformAcEAlphaInv;
        case ST_Position3dTexTGCxformAcEAlphaInv:                return STI_Position3dTexTGCxformAcEAlphaInv;
        case ST_BatchPosition3dTexTGCxformAcEAlphaInv:           return STI_BatchPosition3dTexTGCxformAcEAlphaInv;
        case ST_InstancedPosition3dTexTGCxformAcEAlphaInv:       return STI_InstancedPosition3dTexTGCxformAcEAlphaInv;
        case ST_Vertex:                                          return STI_Vertex;
        case ST_VertexMul:                                       return STI_VertexMul;
        case ST_BatchVertex:                                     return STI_BatchVertex;
        case ST_BatchVertexMul:                                  return STI_BatchVertexMul;
        case ST_InstancedVertex:                                 return STI_InstancedVertex;
        case ST_InstancedVertexMul:                              return STI_InstancedVertexMul;
        case ST_Position3dVertex:                                return STI_Position3dVertex;
        case ST_Position3dVertexMul:                             return STI_Position3dVertexMul;
        case ST_BatchPosition3dVertex:                           return STI_BatchPosition3dVertex;
        case ST_BatchPosition3dVertexMul:                        return STI_BatchPosition3dVertexMul;
        case ST_InstancedPosition3dVertex:                       return STI_InstancedPosition3dVertex;
        case ST_InstancedPosition3dVertexMul:                    return STI_InstancedPosition3dVertexMul;
        case ST_VertexCxform:                                    return STI_VertexCxform;
        case ST_VertexCxformMul:                                 return STI_VertexCxformMul;
        case ST_BatchVertexCxform:                               return STI_BatchVertexCxform;
        case ST_BatchVertexCxformMul:                            return STI_BatchVertexCxformMul;
        case ST_InstancedVertexCxform:                           return STI_InstancedVertexCxform;
        case ST_InstancedVertexCxformMul:                        return STI_InstancedVertexCxformMul;
        case ST_Position3dVertexCxform:                          return STI_Position3dVertexCxform;
        case ST_Position3dVertexCxformMul:                       return STI_Position3dVertexCxformMul;
        case ST_BatchPosition3dVertexCxform:                     return STI_BatchPosition3dVertexCxform;
        case ST_BatchPosition3dVertexCxformMul:                  return STI_BatchPosition3dVertexCxformMul;
        case ST_InstancedPosition3dVertexCxform:                 return STI_InstancedPosition3dVertexCxform;
        case ST_InstancedPosition3dVertexCxformMul:              return STI_InstancedPosition3dVertexCxformMul;
        case ST_VertexEAlpha:                                    return STI_VertexEAlpha;
        case ST_VertexEAlphaMul:                                 return STI_VertexEAlphaMul;
        case ST_BatchVertexEAlpha:                               return STI_BatchVertexEAlpha;
        case ST_BatchVertexEAlphaMul:                            return STI_BatchVertexEAlphaMul;
        case ST_InstancedVertexEAlpha:                           return STI_InstancedVertexEAlpha;
        case ST_InstancedVertexEAlphaMul:                        return STI_InstancedVertexEAlphaMul;
        case ST_Position3dVertexEAlpha:                          return STI_Position3dVertexEAlpha;
        case ST_Position3dVertexEAlphaMul:                       return STI_Position3dVertexEAlphaMul;
        case ST_BatchPosition3dVertexEAlpha:                     return STI_BatchPosition3dVertexEAlpha;
        case ST_BatchPosition3dVertexEAlphaMul:                  return STI_BatchPosition3dVertexEAlphaMul;
        case ST_InstancedPosition3dVertexEAlpha:                 return STI_InstancedPosition3dVertexEAlpha;
        case ST_InstancedPosition3dVertexEAlphaMul:              return STI_InstancedPosition3dVertexEAlphaMul;
        case ST_VertexCxformEAlpha:                              return STI_VertexCxformEAlpha;
        case ST_VertexCxformEAlphaMul:                           return STI_VertexCxformEAlphaMul;
        case ST_BatchVertexCxformEAlpha:                         return STI_BatchVertexCxformEAlpha;
        case ST_BatchVertexCxformEAlphaMul:                      return STI_BatchVertexCxformEAlphaMul;
        case ST_InstancedVertexCxformEAlpha:                     return STI_InstancedVertexCxformEAlpha;
        case ST_InstancedVertexCxformEAlphaMul:                  return STI_InstancedVertexCxformEAlphaMul;
        case ST_Position3dVertexCxformEAlpha:                    return STI_Position3dVertexCxformEAlpha;
        case ST_Position3dVertexCxformEAlphaMul:                 return STI_Position3dVertexCxformEAlphaMul;
        case ST_BatchPosition3dVertexCxformEAlpha:               return STI_BatchPosition3dVertexCxformEAlpha;
        case ST_BatchPosition3dVertexCxformEAlphaMul:            return STI_BatchPosition3dVertexCxformEAlphaMul;
        case ST_InstancedPosition3dVertexCxformEAlpha:           return STI_InstancedPosition3dVertexCxformEAlpha;
        case ST_InstancedPosition3dVertexCxformEAlphaMul:        return STI_InstancedPosition3dVertexCxformEAlphaMul;
        case ST_VertexInv:                                       return STI_VertexInv;
        case ST_BatchVertexInv:                                  return STI_BatchVertexInv;
        case ST_InstancedVertexInv:                              return STI_InstancedVertexInv;
        case ST_Position3dVertexInv:                             return STI_Position3dVertexInv;
        case ST_BatchPosition3dVertexInv:                        return STI_BatchPosition3dVertexInv;
        case ST_InstancedPosition3dVertexInv:                    return STI_InstancedPosition3dVertexInv;
        case ST_VertexCxformInv:                                 return STI_VertexCxformInv;
        case ST_BatchVertexCxformInv:                            return STI_BatchVertexCxformInv;
        case ST_InstancedVertexCxformInv:                        return STI_InstancedVertexCxformInv;
        case ST_Position3dVertexCxformInv:                       return STI_Position3dVertexCxformInv;
        case ST_BatchPosition3dVertexCxformInv:                  return STI_BatchPosition3dVertexCxformInv;
        case ST_InstancedPosition3dVertexCxformInv:              return STI_InstancedPosition3dVertexCxformInv;
        case ST_VertexEAlphaInv:                                 return STI_VertexEAlphaInv;
        case ST_BatchVertexEAlphaInv:                            return STI_BatchVertexEAlphaInv;
        case ST_InstancedVertexEAlphaInv:                        return STI_InstancedVertexEAlphaInv;
        case ST_Position3dVertexEAlphaInv:                       return STI_Position3dVertexEAlphaInv;
        case ST_BatchPosition3dVertexEAlphaInv:                  return STI_BatchPosition3dVertexEAlphaInv;
        case ST_InstancedPosition3dVertexEAlphaInv:              return STI_InstancedPosition3dVertexEAlphaInv;
        case ST_VertexCxformEAlphaInv:                           return STI_VertexCxformEAlphaInv;
        case ST_BatchVertexCxformEAlphaInv:                      return STI_BatchVertexCxformEAlphaInv;
        case ST_InstancedVertexCxformEAlphaInv:                  return STI_InstancedVertexCxformEAlphaInv;
        case ST_Position3dVertexCxformEAlphaInv:                 return STI_Position3dVertexCxformEAlphaInv;
        case ST_BatchPosition3dVertexCxformEAlphaInv:            return STI_BatchPosition3dVertexCxformEAlphaInv;
        case ST_InstancedPosition3dVertexCxformEAlphaInv:        return STI_InstancedPosition3dVertexCxformEAlphaInv;
        case ST_VertexCxformAc:                                  return STI_VertexCxformAc;
        case ST_VertexCxformAcMul:                               return STI_VertexCxformAcMul;
        case ST_BatchVertexCxformAc:                             return STI_BatchVertexCxformAc;
        case ST_BatchVertexCxformAcMul:                          return STI_BatchVertexCxformAcMul;
        case ST_InstancedVertexCxformAc:                         return STI_InstancedVertexCxformAc;
        case ST_InstancedVertexCxformAcMul:                      return STI_InstancedVertexCxformAcMul;
        case ST_Position3dVertexCxformAc:                        return STI_Position3dVertexCxformAc;
        case ST_Position3dVertexCxformAcMul:                     return STI_Position3dVertexCxformAcMul;
        case ST_BatchPosition3dVertexCxformAc:                   return STI_BatchPosition3dVertexCxformAc;
        case ST_BatchPosition3dVertexCxformAcMul:                return STI_BatchPosition3dVertexCxformAcMul;
        case ST_InstancedPosition3dVertexCxformAc:               return STI_InstancedPosition3dVertexCxformAc;
        case ST_InstancedPosition3dVertexCxformAcMul:            return STI_InstancedPosition3dVertexCxformAcMul;
        case ST_VertexCxformAcEAlpha:                            return STI_VertexCxformAcEAlpha;
        case ST_VertexCxformAcEAlphaMul:                         return STI_VertexCxformAcEAlphaMul;
        case ST_BatchVertexCxformAcEAlpha:                       return STI_BatchVertexCxformAcEAlpha;
        case ST_BatchVertexCxformAcEAlphaMul:                    return STI_BatchVertexCxformAcEAlphaMul;
        case ST_InstancedVertexCxformAcEAlpha:                   return STI_InstancedVertexCxformAcEAlpha;
        case ST_InstancedVertexCxformAcEAlphaMul:                return STI_InstancedVertexCxformAcEAlphaMul;
        case ST_Position3dVertexCxformAcEAlpha:                  return STI_Position3dVertexCxformAcEAlpha;
        case ST_Position3dVertexCxformAcEAlphaMul:               return STI_Position3dVertexCxformAcEAlphaMul;
        case ST_BatchPosition3dVertexCxformAcEAlpha:             return STI_BatchPosition3dVertexCxformAcEAlpha;
        case ST_BatchPosition3dVertexCxformAcEAlphaMul:          return STI_BatchPosition3dVertexCxformAcEAlphaMul;
        case ST_InstancedPosition3dVertexCxformAcEAlpha:         return STI_InstancedPosition3dVertexCxformAcEAlpha;
        case ST_InstancedPosition3dVertexCxformAcEAlphaMul:      return STI_InstancedPosition3dVertexCxformAcEAlphaMul;
        case ST_VertexCxformAcInv:                               return STI_VertexCxformAcInv;
        case ST_BatchVertexCxformAcInv:                          return STI_BatchVertexCxformAcInv;
        case ST_InstancedVertexCxformAcInv:                      return STI_InstancedVertexCxformAcInv;
        case ST_Position3dVertexCxformAcInv:                     return STI_Position3dVertexCxformAcInv;
        case ST_BatchPosition3dVertexCxformAcInv:                return STI_BatchPosition3dVertexCxformAcInv;
        case ST_InstancedPosition3dVertexCxformAcInv:            return STI_InstancedPosition3dVertexCxformAcInv;
        case ST_VertexCxformAcEAlphaInv:                         return STI_VertexCxformAcEAlphaInv;
        case ST_BatchVertexCxformAcEAlphaInv:                    return STI_BatchVertexCxformAcEAlphaInv;
        case ST_InstancedVertexCxformAcEAlphaInv:                return STI_InstancedVertexCxformAcEAlphaInv;
        case ST_Position3dVertexCxformAcEAlphaInv:               return STI_Position3dVertexCxformAcEAlphaInv;
        case ST_BatchPosition3dVertexCxformAcEAlphaInv:          return STI_BatchPosition3dVertexCxformAcEAlphaInv;
        case ST_InstancedPosition3dVertexCxformAcEAlphaInv:      return STI_InstancedPosition3dVertexCxformAcEAlphaInv;
        case ST_TexTGTexTG:                                      return STI_TexTGTexTG;
        case ST_TexTGTexTGMul:                                   return STI_TexTGTexTGMul;
        case ST_BatchTexTGTexTG:                                 return STI_BatchTexTGTexTG;
        case ST_BatchTexTGTexTGMul:                              return STI_BatchTexTGTexTGMul;
        case ST_InstancedTexTGTexTG:                             return STI_InstancedTexTGTexTG;
        case ST_InstancedTexTGTexTGMul:                          return STI_InstancedTexTGTexTGMul;
        case ST_Position3dTexTGTexTG:                            return STI_Position3dTexTGTexTG;
        case ST_Position3dTexTGTexTGMul:                         return STI_Position3dTexTGTexTGMul;
        case ST_BatchPosition3dTexTGTexTG:                       return STI_BatchPosition3dTexTGTexTG;
        case ST_BatchPosition3dTexTGTexTGMul:                    return STI_BatchPosition3dTexTGTexTGMul;
        case ST_InstancedPosition3dTexTGTexTG:                   return STI_InstancedPosition3dTexTGTexTG;
        case ST_InstancedPosition3dTexTGTexTGMul:                return STI_InstancedPosition3dTexTGTexTGMul;
        case ST_TexTGTexTGCxform:                                return STI_TexTGTexTGCxform;
        case ST_TexTGTexTGCxformMul:                             return STI_TexTGTexTGCxformMul;
        case ST_BatchTexTGTexTGCxform:                           return STI_BatchTexTGTexTGCxform;
        case ST_BatchTexTGTexTGCxformMul:                        return STI_BatchTexTGTexTGCxformMul;
        case ST_InstancedTexTGTexTGCxform:                       return STI_InstancedTexTGTexTGCxform;
        case ST_InstancedTexTGTexTGCxformMul:                    return STI_InstancedTexTGTexTGCxformMul;
        case ST_Position3dTexTGTexTGCxform:                      return STI_Position3dTexTGTexTGCxform;
        case ST_Position3dTexTGTexTGCxformMul:                   return STI_Position3dTexTGTexTGCxformMul;
        case ST_BatchPosition3dTexTGTexTGCxform:                 return STI_BatchPosition3dTexTGTexTGCxform;
        case ST_BatchPosition3dTexTGTexTGCxformMul:              return STI_BatchPosition3dTexTGTexTGCxformMul;
        case ST_InstancedPosition3dTexTGTexTGCxform:             return STI_InstancedPosition3dTexTGTexTGCxform;
        case ST_InstancedPosition3dTexTGTexTGCxformMul:          return STI_InstancedPosition3dTexTGTexTGCxformMul;
        case ST_TexTGTexTGEAlpha:                                return STI_TexTGTexTGEAlpha;
        case ST_TexTGTexTGEAlphaMul:                             return STI_TexTGTexTGEAlphaMul;
        case ST_BatchTexTGTexTGEAlpha:                           return STI_BatchTexTGTexTGEAlpha;
        case ST_BatchTexTGTexTGEAlphaMul:                        return STI_BatchTexTGTexTGEAlphaMul;
        case ST_InstancedTexTGTexTGEAlpha:                       return STI_InstancedTexTGTexTGEAlpha;
        case ST_InstancedTexTGTexTGEAlphaMul:                    return STI_InstancedTexTGTexTGEAlphaMul;
        case ST_Position3dTexTGTexTGEAlpha:                      return STI_Position3dTexTGTexTGEAlpha;
        case ST_Position3dTexTGTexTGEAlphaMul:                   return STI_Position3dTexTGTexTGEAlphaMul;
        case ST_BatchPosition3dTexTGTexTGEAlpha:                 return STI_BatchPosition3dTexTGTexTGEAlpha;
        case ST_BatchPosition3dTexTGTexTGEAlphaMul:              return STI_BatchPosition3dTexTGTexTGEAlphaMul;
        case ST_InstancedPosition3dTexTGTexTGEAlpha:             return STI_InstancedPosition3dTexTGTexTGEAlpha;
        case ST_InstancedPosition3dTexTGTexTGEAlphaMul:          return STI_InstancedPosition3dTexTGTexTGEAlphaMul;
        case ST_TexTGTexTGCxformEAlpha:                          return STI_TexTGTexTGCxformEAlpha;
        case ST_TexTGTexTGCxformEAlphaMul:                       return STI_TexTGTexTGCxformEAlphaMul;
        case ST_BatchTexTGTexTGCxformEAlpha:                     return STI_BatchTexTGTexTGCxformEAlpha;
        case ST_BatchTexTGTexTGCxformEAlphaMul:                  return STI_BatchTexTGTexTGCxformEAlphaMul;
        case ST_InstancedTexTGTexTGCxformEAlpha:                 return STI_InstancedTexTGTexTGCxformEAlpha;
        case ST_InstancedTexTGTexTGCxformEAlphaMul:              return STI_InstancedTexTGTexTGCxformEAlphaMul;
        case ST_Position3dTexTGTexTGCxformEAlpha:                return STI_Position3dTexTGTexTGCxformEAlpha;
        case ST_Position3dTexTGTexTGCxformEAlphaMul:             return STI_Position3dTexTGTexTGCxformEAlphaMul;
        case ST_BatchPosition3dTexTGTexTGCxformEAlpha:           return STI_BatchPosition3dTexTGTexTGCxformEAlpha;
        case ST_BatchPosition3dTexTGTexTGCxformEAlphaMul:        return STI_BatchPosition3dTexTGTexTGCxformEAlphaMul;
        case ST_InstancedPosition3dTexTGTexTGCxformEAlpha:       return STI_InstancedPosition3dTexTGTexTGCxformEAlpha;
        case ST_InstancedPosition3dTexTGTexTGCxformEAlphaMul:    return STI_InstancedPosition3dTexTGTexTGCxformEAlphaMul;
        case ST_TexTGTexTGInv:                                   return STI_TexTGTexTGInv;
        case ST_BatchTexTGTexTGInv:                              return STI_BatchTexTGTexTGInv;
        case ST_InstancedTexTGTexTGInv:                          return STI_InstancedTexTGTexTGInv;
        case ST_Position3dTexTGTexTGInv:                         return STI_Position3dTexTGTexTGInv;
        case ST_BatchPosition3dTexTGTexTGInv:                    return STI_BatchPosition3dTexTGTexTGInv;
        case ST_InstancedPosition3dTexTGTexTGInv:                return STI_InstancedPosition3dTexTGTexTGInv;
        case ST_TexTGTexTGCxformInv:                             return STI_TexTGTexTGCxformInv;
        case ST_BatchTexTGTexTGCxformInv:                        return STI_BatchTexTGTexTGCxformInv;
        case ST_InstancedTexTGTexTGCxformInv:                    return STI_InstancedTexTGTexTGCxformInv;
        case ST_Position3dTexTGTexTGCxformInv:                   return STI_Position3dTexTGTexTGCxformInv;
        case ST_BatchPosition3dTexTGTexTGCxformInv:              return STI_BatchPosition3dTexTGTexTGCxformInv;
        case ST_InstancedPosition3dTexTGTexTGCxformInv:          return STI_InstancedPosition3dTexTGTexTGCxformInv;
        case ST_TexTGTexTGEAlphaInv:                             return STI_TexTGTexTGEAlphaInv;
        case ST_BatchTexTGTexTGEAlphaInv:                        return STI_BatchTexTGTexTGEAlphaInv;
        case ST_InstancedTexTGTexTGEAlphaInv:                    return STI_InstancedTexTGTexTGEAlphaInv;
        case ST_Position3dTexTGTexTGEAlphaInv:                   return STI_Position3dTexTGTexTGEAlphaInv;
        case ST_BatchPosition3dTexTGTexTGEAlphaInv:              return STI_BatchPosition3dTexTGTexTGEAlphaInv;
        case ST_InstancedPosition3dTexTGTexTGEAlphaInv:          return STI_InstancedPosition3dTexTGTexTGEAlphaInv;
        case ST_TexTGTexTGCxformEAlphaInv:                       return STI_TexTGTexTGCxformEAlphaInv;
        case ST_BatchTexTGTexTGCxformEAlphaInv:                  return STI_BatchTexTGTexTGCxformEAlphaInv;
        case ST_InstancedTexTGTexTGCxformEAlphaInv:              return STI_InstancedTexTGTexTGCxformEAlphaInv;
        case ST_Position3dTexTGTexTGCxformEAlphaInv:             return STI_Position3dTexTGTexTGCxformEAlphaInv;
        case ST_BatchPosition3dTexTGTexTGCxformEAlphaInv:        return STI_BatchPosition3dTexTGTexTGCxformEAlphaInv;
        case ST_InstancedPosition3dTexTGTexTGCxformEAlphaInv:    return STI_InstancedPosition3dTexTGTexTGCxformEAlphaInv;
        case ST_TexTGTexTGCxformAc:                              return STI_TexTGTexTGCxformAc;
        case ST_TexTGTexTGCxformAcMul:                           return STI_TexTGTexTGCxformAcMul;
        case ST_BatchTexTGTexTGCxformAc:                         return STI_BatchTexTGTexTGCxformAc;
        case ST_BatchTexTGTexTGCxformAcMul:                      return STI_BatchTexTGTexTGCxformAcMul;
        case ST_InstancedTexTGTexTGCxformAc:                     return STI_InstancedTexTGTexTGCxformAc;
        case ST_InstancedTexTGTexTGCxformAcMul:                  return STI_InstancedTexTGTexTGCxformAcMul;
        case ST_Position3dTexTGTexTGCxformAc:                    return STI_Position3dTexTGTexTGCxformAc;
        case ST_Position3dTexTGTexTGCxformAcMul:                 return STI_Position3dTexTGTexTGCxformAcMul;
        case ST_BatchPosition3dTexTGTexTGCxformAc:               return STI_BatchPosition3dTexTGTexTGCxformAc;
        case ST_BatchPosition3dTexTGTexTGCxformAcMul:            return STI_BatchPosition3dTexTGTexTGCxformAcMul;
        case ST_InstancedPosition3dTexTGTexTGCxformAc:           return STI_InstancedPosition3dTexTGTexTGCxformAc;
        case ST_InstancedPosition3dTexTGTexTGCxformAcMul:        return STI_InstancedPosition3dTexTGTexTGCxformAcMul;
        case ST_TexTGTexTGCxformAcEAlpha:                        return STI_TexTGTexTGCxformAcEAlpha;
        case ST_TexTGTexTGCxformAcEAlphaMul:                     return STI_TexTGTexTGCxformAcEAlphaMul;
        case ST_BatchTexTGTexTGCxformAcEAlpha:                   return STI_BatchTexTGTexTGCxformAcEAlpha;
        case ST_BatchTexTGTexTGCxformAcEAlphaMul:                return STI_BatchTexTGTexTGCxformAcEAlphaMul;
        case ST_InstancedTexTGTexTGCxformAcEAlpha:               return STI_InstancedTexTGTexTGCxformAcEAlpha;
        case ST_InstancedTexTGTexTGCxformAcEAlphaMul:            return STI_InstancedTexTGTexTGCxformAcEAlphaMul;
        case ST_Position3dTexTGTexTGCxformAcEAlpha:              return STI_Position3dTexTGTexTGCxformAcEAlpha;
        case ST_Position3dTexTGTexTGCxformAcEAlphaMul:           return STI_Position3dTexTGTexTGCxformAcEAlphaMul;
        case ST_BatchPosition3dTexTGTexTGCxformAcEAlpha:         return STI_BatchPosition3dTexTGTexTGCxformAcEAlpha;
        case ST_BatchPosition3dTexTGTexTGCxformAcEAlphaMul:      return STI_BatchPosition3dTexTGTexTGCxformAcEAlphaMul;
        case ST_InstancedPosition3dTexTGTexTGCxformAcEAlpha:     return STI_InstancedPosition3dTexTGTexTGCxformAcEAlpha;
        case ST_InstancedPosition3dTexTGTexTGCxformAcEAlphaMul:  return STI_InstancedPosition3dTexTGTexTGCxformAcEAlphaMul;
        case ST_TexTGTexTGCxformAcInv:                           return STI_TexTGTexTGCxformAcInv;
        case ST_BatchTexTGTexTGCxformAcInv:                      return STI_BatchTexTGTexTGCxformAcInv;
        case ST_InstancedTexTGTexTGCxformAcInv:                  return STI_InstancedTexTGTexTGCxformAcInv;
        case ST_Position3dTexTGTexTGCxformAcInv:                 return STI_Position3dTexTGTexTGCxformAcInv;
        case ST_BatchPosition3dTexTGTexTGCxformAcInv:            return STI_BatchPosition3dTexTGTexTGCxformAcInv;
        case ST_InstancedPosition3dTexTGTexTGCxformAcInv:        return STI_InstancedPosition3dTexTGTexTGCxformAcInv;
        case ST_TexTGTexTGCxformAcEAlphaInv:                     return STI_TexTGTexTGCxformAcEAlphaInv;
        case ST_BatchTexTGTexTGCxformAcEAlphaInv:                return STI_BatchTexTGTexTGCxformAcEAlphaInv;
        case ST_InstancedTexTGTexTGCxformAcEAlphaInv:            return STI_InstancedTexTGTexTGCxformAcEAlphaInv;
        case ST_Position3dTexTGTexTGCxformAcEAlphaInv:           return STI_Position3dTexTGTexTGCxformAcEAlphaInv;
        case ST_BatchPosition3dTexTGTexTGCxformAcEAlphaInv:      return STI_BatchPosition3dTexTGTexTGCxformAcEAlphaInv;
        case ST_InstancedPosition3dTexTGTexTGCxformAcEAlphaInv:  return STI_InstancedPosition3dTexTGTexTGCxformAcEAlphaInv;
        case ST_TexTGVertex:                                     return STI_TexTGVertex;
        case ST_TexTGVertexMul:                                  return STI_TexTGVertexMul;
        case ST_BatchTexTGVertex:                                return STI_BatchTexTGVertex;
        case ST_BatchTexTGVertexMul:                             return STI_BatchTexTGVertexMul;
        case ST_InstancedTexTGVertex:                            return STI_InstancedTexTGVertex;
        case ST_InstancedTexTGVertexMul:                         return STI_InstancedTexTGVertexMul;
        case ST_Position3dTexTGVertex:                           return STI_Position3dTexTGVertex;
        case ST_Position3dTexTGVertexMul:                        return STI_Position3dTexTGVertexMul;
        case ST_BatchPosition3dTexTGVertex:                      return STI_BatchPosition3dTexTGVertex;
        case ST_BatchPosition3dTexTGVertexMul:                   return STI_BatchPosition3dTexTGVertexMul;
        case ST_InstancedPosition3dTexTGVertex:                  return STI_InstancedPosition3dTexTGVertex;
        case ST_InstancedPosition3dTexTGVertexMul:               return STI_InstancedPosition3dTexTGVertexMul;
        case ST_TexTGVertexCxform:                               return STI_TexTGVertexCxform;
        case ST_TexTGVertexCxformMul:                            return STI_TexTGVertexCxformMul;
        case ST_BatchTexTGVertexCxform:                          return STI_BatchTexTGVertexCxform;
        case ST_BatchTexTGVertexCxformMul:                       return STI_BatchTexTGVertexCxformMul;
        case ST_InstancedTexTGVertexCxform:                      return STI_InstancedTexTGVertexCxform;
        case ST_InstancedTexTGVertexCxformMul:                   return STI_InstancedTexTGVertexCxformMul;
        case ST_Position3dTexTGVertexCxform:                     return STI_Position3dTexTGVertexCxform;
        case ST_Position3dTexTGVertexCxformMul:                  return STI_Position3dTexTGVertexCxformMul;
        case ST_BatchPosition3dTexTGVertexCxform:                return STI_BatchPosition3dTexTGVertexCxform;
        case ST_BatchPosition3dTexTGVertexCxformMul:             return STI_BatchPosition3dTexTGVertexCxformMul;
        case ST_InstancedPosition3dTexTGVertexCxform:            return STI_InstancedPosition3dTexTGVertexCxform;
        case ST_InstancedPosition3dTexTGVertexCxformMul:         return STI_InstancedPosition3dTexTGVertexCxformMul;
        case ST_TexTGVertexEAlpha:                               return STI_TexTGVertexEAlpha;
        case ST_TexTGVertexEAlphaMul:                            return STI_TexTGVertexEAlphaMul;
        case ST_BatchTexTGVertexEAlpha:                          return STI_BatchTexTGVertexEAlpha;
        case ST_BatchTexTGVertexEAlphaMul:                       return STI_BatchTexTGVertexEAlphaMul;
        case ST_InstancedTexTGVertexEAlpha:                      return STI_InstancedTexTGVertexEAlpha;
        case ST_InstancedTexTGVertexEAlphaMul:                   return STI_InstancedTexTGVertexEAlphaMul;
        case ST_Position3dTexTGVertexEAlpha:                     return STI_Position3dTexTGVertexEAlpha;
        case ST_Position3dTexTGVertexEAlphaMul:                  return STI_Position3dTexTGVertexEAlphaMul;
        case ST_BatchPosition3dTexTGVertexEAlpha:                return STI_BatchPosition3dTexTGVertexEAlpha;
        case ST_BatchPosition3dTexTGVertexEAlphaMul:             return STI_BatchPosition3dTexTGVertexEAlphaMul;
        case ST_InstancedPosition3dTexTGVertexEAlpha:            return STI_InstancedPosition3dTexTGVertexEAlpha;
        case ST_InstancedPosition3dTexTGVertexEAlphaMul:         return STI_InstancedPosition3dTexTGVertexEAlphaMul;
        case ST_TexTGVertexCxformEAlpha:                         return STI_TexTGVertexCxformEAlpha;
        case ST_TexTGVertexCxformEAlphaMul:                      return STI_TexTGVertexCxformEAlphaMul;
        case ST_BatchTexTGVertexCxformEAlpha:                    return STI_BatchTexTGVertexCxformEAlpha;
        case ST_BatchTexTGVertexCxformEAlphaMul:                 return STI_BatchTexTGVertexCxformEAlphaMul;
        case ST_InstancedTexTGVertexCxformEAlpha:                return STI_InstancedTexTGVertexCxformEAlpha;
        case ST_InstancedTexTGVertexCxformEAlphaMul:             return STI_InstancedTexTGVertexCxformEAlphaMul;
        case ST_Position3dTexTGVertexCxformEAlpha:               return STI_Position3dTexTGVertexCxformEAlpha;
        case ST_Position3dTexTGVertexCxformEAlphaMul:            return STI_Position3dTexTGVertexCxformEAlphaMul;
        case ST_BatchPosition3dTexTGVertexCxformEAlpha:          return STI_BatchPosition3dTexTGVertexCxformEAlpha;
        case ST_BatchPosition3dTexTGVertexCxformEAlphaMul:       return STI_BatchPosition3dTexTGVertexCxformEAlphaMul;
        case ST_InstancedPosition3dTexTGVertexCxformEAlpha:      return STI_InstancedPosition3dTexTGVertexCxformEAlpha;
        case ST_InstancedPosition3dTexTGVertexCxformEAlphaMul:   return STI_InstancedPosition3dTexTGVertexCxformEAlphaMul;
        case ST_TexTGVertexInv:                                  return STI_TexTGVertexInv;
        case ST_BatchTexTGVertexInv:                             return STI_BatchTexTGVertexInv;
        case ST_InstancedTexTGVertexInv:                         return STI_InstancedTexTGVertexInv;
        case ST_Position3dTexTGVertexInv:                        return STI_Position3dTexTGVertexInv;
        case ST_BatchPosition3dTexTGVertexInv:                   return STI_BatchPosition3dTexTGVertexInv;
        case ST_InstancedPosition3dTexTGVertexInv:               return STI_InstancedPosition3dTexTGVertexInv;
        case ST_TexTGVertexCxformInv:                            return STI_TexTGVertexCxformInv;
        case ST_BatchTexTGVertexCxformInv:                       return STI_BatchTexTGVertexCxformInv;
        case ST_InstancedTexTGVertexCxformInv:                   return STI_InstancedTexTGVertexCxformInv;
        case ST_Position3dTexTGVertexCxformInv:                  return STI_Position3dTexTGVertexCxformInv;
        case ST_BatchPosition3dTexTGVertexCxformInv:             return STI_BatchPosition3dTexTGVertexCxformInv;
        case ST_InstancedPosition3dTexTGVertexCxformInv:         return STI_InstancedPosition3dTexTGVertexCxformInv;
        case ST_TexTGVertexEAlphaInv:                            return STI_TexTGVertexEAlphaInv;
        case ST_BatchTexTGVertexEAlphaInv:                       return STI_BatchTexTGVertexEAlphaInv;
        case ST_InstancedTexTGVertexEAlphaInv:                   return STI_InstancedTexTGVertexEAlphaInv;
        case ST_Position3dTexTGVertexEAlphaInv:                  return STI_Position3dTexTGVertexEAlphaInv;
        case ST_BatchPosition3dTexTGVertexEAlphaInv:             return STI_BatchPosition3dTexTGVertexEAlphaInv;
        case ST_InstancedPosition3dTexTGVertexEAlphaInv:         return STI_InstancedPosition3dTexTGVertexEAlphaInv;
        case ST_TexTGVertexCxformEAlphaInv:                      return STI_TexTGVertexCxformEAlphaInv;
        case ST_BatchTexTGVertexCxformEAlphaInv:                 return STI_BatchTexTGVertexCxformEAlphaInv;
        case ST_InstancedTexTGVertexCxformEAlphaInv:             return STI_InstancedTexTGVertexCxformEAlphaInv;
        case ST_Position3dTexTGVertexCxformEAlphaInv:            return STI_Position3dTexTGVertexCxformEAlphaInv;
        case ST_BatchPosition3dTexTGVertexCxformEAlphaInv:       return STI_BatchPosition3dTexTGVertexCxformEAlphaInv;
        case ST_InstancedPosition3dTexTGVertexCxformEAlphaInv:   return STI_InstancedPosition3dTexTGVertexCxformEAlphaInv;
        case ST_TexTGVertexCxformAc:                             return STI_TexTGVertexCxformAc;
        case ST_TexTGVertexCxformAcMul:                          return STI_TexTGVertexCxformAcMul;
        case ST_BatchTexTGVertexCxformAc:                        return STI_BatchTexTGVertexCxformAc;
        case ST_BatchTexTGVertexCxformAcMul:                     return STI_BatchTexTGVertexCxformAcMul;
        case ST_InstancedTexTGVertexCxformAc:                    return STI_InstancedTexTGVertexCxformAc;
        case ST_InstancedTexTGVertexCxformAcMul:                 return STI_InstancedTexTGVertexCxformAcMul;
        case ST_Position3dTexTGVertexCxformAc:                   return STI_Position3dTexTGVertexCxformAc;
        case ST_Position3dTexTGVertexCxformAcMul:                return STI_Position3dTexTGVertexCxformAcMul;
        case ST_BatchPosition3dTexTGVertexCxformAc:              return STI_BatchPosition3dTexTGVertexCxformAc;
        case ST_BatchPosition3dTexTGVertexCxformAcMul:           return STI_BatchPosition3dTexTGVertexCxformAcMul;
        case ST_InstancedPosition3dTexTGVertexCxformAc:          return STI_InstancedPosition3dTexTGVertexCxformAc;
        case ST_InstancedPosition3dTexTGVertexCxformAcMul:       return STI_InstancedPosition3dTexTGVertexCxformAcMul;
        case ST_TexTGVertexCxformAcEAlpha:                       return STI_TexTGVertexCxformAcEAlpha;
        case ST_TexTGVertexCxformAcEAlphaMul:                    return STI_TexTGVertexCxformAcEAlphaMul;
        case ST_BatchTexTGVertexCxformAcEAlpha:                  return STI_BatchTexTGVertexCxformAcEAlpha;
        case ST_BatchTexTGVertexCxformAcEAlphaMul:               return STI_BatchTexTGVertexCxformAcEAlphaMul;
        case ST_InstancedTexTGVertexCxformAcEAlpha:              return STI_InstancedTexTGVertexCxformAcEAlpha;
        case ST_InstancedTexTGVertexCxformAcEAlphaMul:           return STI_InstancedTexTGVertexCxformAcEAlphaMul;
        case ST_Position3dTexTGVertexCxformAcEAlpha:             return STI_Position3dTexTGVertexCxformAcEAlpha;
        case ST_Position3dTexTGVertexCxformAcEAlphaMul:          return STI_Position3dTexTGVertexCxformAcEAlphaMul;
        case ST_BatchPosition3dTexTGVertexCxformAcEAlpha:        return STI_BatchPosition3dTexTGVertexCxformAcEAlpha;
        case ST_BatchPosition3dTexTGVertexCxformAcEAlphaMul:     return STI_BatchPosition3dTexTGVertexCxformAcEAlphaMul;
        case ST_InstancedPosition3dTexTGVertexCxformAcEAlpha:    return STI_InstancedPosition3dTexTGVertexCxformAcEAlpha;
        case ST_InstancedPosition3dTexTGVertexCxformAcEAlphaMul: return STI_InstancedPosition3dTexTGVertexCxformAcEAlphaMul;
        case ST_TexTGVertexCxformAcInv:                          return STI_TexTGVertexCxformAcInv;
        case ST_BatchTexTGVertexCxformAcInv:                     return STI_BatchTexTGVertexCxformAcInv;
        case ST_InstancedTexTGVertexCxformAcInv:                 return STI_InstancedTexTGVertexCxformAcInv;
        case ST_Position3dTexTGVertexCxformAcInv:                return STI_Position3dTexTGVertexCxformAcInv;
        case ST_BatchPosition3dTexTGVertexCxformAcInv:           return STI_BatchPosition3dTexTGVertexCxformAcInv;
        case ST_InstancedPosition3dTexTGVertexCxformAcInv:       return STI_InstancedPosition3dTexTGVertexCxformAcInv;
        case ST_TexTGVertexCxformAcEAlphaInv:                    return STI_TexTGVertexCxformAcEAlphaInv;
        case ST_BatchTexTGVertexCxformAcEAlphaInv:               return STI_BatchTexTGVertexCxformAcEAlphaInv;
        case ST_InstancedTexTGVertexCxformAcEAlphaInv:           return STI_InstancedTexTGVertexCxformAcEAlphaInv;
        case ST_Position3dTexTGVertexCxformAcEAlphaInv:          return STI_Position3dTexTGVertexCxformAcEAlphaInv;
        case ST_BatchPosition3dTexTGVertexCxformAcEAlphaInv:     return STI_BatchPosition3dTexTGVertexCxformAcEAlphaInv;
        case ST_InstancedPosition3dTexTGVertexCxformAcEAlphaInv: return STI_InstancedPosition3dTexTGVertexCxformAcEAlphaInv;
        case ST_Solid:                                           return STI_Solid;
        case ST_SolidMul:                                        return STI_SolidMul;
        case ST_BatchSolid:                                      return STI_BatchSolid;
        case ST_BatchSolidMul:                                   return STI_BatchSolidMul;
        case ST_InstancedSolid:                                  return STI_InstancedSolid;
        case ST_InstancedSolidMul:                               return STI_InstancedSolidMul;
        case ST_Position3dSolid:                                 return STI_Position3dSolid;
        case ST_Position3dSolidMul:                              return STI_Position3dSolidMul;
        case ST_BatchPosition3dSolid:                            return STI_BatchPosition3dSolid;
        case ST_BatchPosition3dSolidMul:                         return STI_BatchPosition3dSolidMul;
        case ST_InstancedPosition3dSolid:                        return STI_InstancedPosition3dSolid;
        case ST_InstancedPosition3dSolidMul:                     return STI_InstancedPosition3dSolidMul;
        case ST_SolidInv:                                        return STI_SolidInv;
        case ST_BatchSolidInv:                                   return STI_BatchSolidInv;
        case ST_InstancedSolidInv:                               return STI_InstancedSolidInv;
        case ST_Position3dSolidInv:                              return STI_Position3dSolidInv;
        case ST_BatchPosition3dSolidInv:                         return STI_BatchPosition3dSolidInv;
        case ST_InstancedPosition3dSolidInv:                     return STI_InstancedPosition3dSolidInv;
        case ST_Text:                                            return STI_Text;
        case ST_TextMul:                                         return STI_TextMul;
        case ST_BatchText:                                       return STI_BatchText;
        case ST_BatchTextMul:                                    return STI_BatchTextMul;
        case ST_InstancedText:                                   return STI_InstancedText;
        case ST_InstancedTextMul:                                return STI_InstancedTextMul;
        case ST_Position3dText:                                  return STI_Position3dText;
        case ST_Position3dTextMul:                               return STI_Position3dTextMul;
        case ST_BatchPosition3dText:                             return STI_BatchPosition3dText;
        case ST_BatchPosition3dTextMul:                          return STI_BatchPosition3dTextMul;
        case ST_InstancedPosition3dText:                         return STI_InstancedPosition3dText;
        case ST_InstancedPosition3dTextMul:                      return STI_InstancedPosition3dTextMul;
        case ST_TextInv:                                         return STI_TextInv;
        case ST_BatchTextInv:                                    return STI_BatchTextInv;
        case ST_InstancedTextInv:                                return STI_InstancedTextInv;
        case ST_Position3dTextInv:                               return STI_Position3dTextInv;
        case ST_BatchPosition3dTextInv:                          return STI_BatchPosition3dTextInv;
        case ST_InstancedPosition3dTextInv:                      return STI_InstancedPosition3dTextInv;
        case ST_TextColor:                                       return STI_TextColor;
        case ST_TextColorMul:                                    return STI_TextColorMul;
        case ST_BatchTextColor:                                  return STI_BatchTextColor;
        case ST_BatchTextColorMul:                               return STI_BatchTextColorMul;
        case ST_InstancedTextColor:                              return STI_InstancedTextColor;
        case ST_InstancedTextColorMul:                           return STI_InstancedTextColorMul;
        case ST_Position3dTextColor:                             return STI_Position3dTextColor;
        case ST_Position3dTextColorMul:                          return STI_Position3dTextColorMul;
        case ST_BatchPosition3dTextColor:                        return STI_BatchPosition3dTextColor;
        case ST_BatchPosition3dTextColorMul:                     return STI_BatchPosition3dTextColorMul;
        case ST_InstancedPosition3dTextColor:                    return STI_InstancedPosition3dTextColor;
        case ST_InstancedPosition3dTextColorMul:                 return STI_InstancedPosition3dTextColorMul;
        case ST_TextColorCxform:                                 return STI_TextColorCxform;
        case ST_TextColorCxformMul:                              return STI_TextColorCxformMul;
        case ST_BatchTextColorCxform:                            return STI_BatchTextColorCxform;
        case ST_BatchTextColorCxformMul:                         return STI_BatchTextColorCxformMul;
        case ST_InstancedTextColorCxform:                        return STI_InstancedTextColorCxform;
        case ST_InstancedTextColorCxformMul:                     return STI_InstancedTextColorCxformMul;
        case ST_Position3dTextColorCxform:                       return STI_Position3dTextColorCxform;
        case ST_Position3dTextColorCxformMul:                    return STI_Position3dTextColorCxformMul;
        case ST_BatchPosition3dTextColorCxform:                  return STI_BatchPosition3dTextColorCxform;
        case ST_BatchPosition3dTextColorCxformMul:               return STI_BatchPosition3dTextColorCxformMul;
        case ST_InstancedPosition3dTextColorCxform:              return STI_InstancedPosition3dTextColorCxform;
        case ST_InstancedPosition3dTextColorCxformMul:           return STI_InstancedPosition3dTextColorCxformMul;
        case ST_TextColorInv:                                    return STI_TextColorInv;
        case ST_BatchTextColorInv:                               return STI_BatchTextColorInv;
        case ST_InstancedTextColorInv:                           return STI_InstancedTextColorInv;
        case ST_Position3dTextColorInv:                          return STI_Position3dTextColorInv;
        case ST_BatchPosition3dTextColorInv:                     return STI_BatchPosition3dTextColorInv;
        case ST_InstancedPosition3dTextColorInv:                 return STI_InstancedPosition3dTextColorInv;
        case ST_TextColorCxformInv:                              return STI_TextColorCxformInv;
        case ST_BatchTextColorCxformInv:                         return STI_BatchTextColorCxformInv;
        case ST_InstancedTextColorCxformInv:                     return STI_InstancedTextColorCxformInv;
        case ST_Position3dTextColorCxformInv:                    return STI_Position3dTextColorCxformInv;
        case ST_BatchPosition3dTextColorCxformInv:               return STI_BatchPosition3dTextColorCxformInv;
        case ST_InstancedPosition3dTextColorCxformInv:           return STI_InstancedPosition3dTextColorCxformInv;
        case ST_YUV:                                             return STI_YUV;
        case ST_YUVMul:                                          return STI_YUVMul;
        case ST_BatchYUV:                                        return STI_BatchYUV;
        case ST_BatchYUVMul:                                     return STI_BatchYUVMul;
        case ST_InstancedYUV:                                    return STI_InstancedYUV;
        case ST_InstancedYUVMul:                                 return STI_InstancedYUVMul;
        case ST_Position3dYUV:                                   return STI_Position3dYUV;
        case ST_Position3dYUVMul:                                return STI_Position3dYUVMul;
        case ST_BatchPosition3dYUV:                              return STI_BatchPosition3dYUV;
        case ST_BatchPosition3dYUVMul:                           return STI_BatchPosition3dYUVMul;
        case ST_InstancedPosition3dYUV:                          return STI_InstancedPosition3dYUV;
        case ST_InstancedPosition3dYUVMul:                       return STI_InstancedPosition3dYUVMul;
        case ST_YUVCxform:                                       return STI_YUVCxform;
        case ST_YUVCxformMul:                                    return STI_YUVCxformMul;
        case ST_BatchYUVCxform:                                  return STI_BatchYUVCxform;
        case ST_BatchYUVCxformMul:                               return STI_BatchYUVCxformMul;
        case ST_InstancedYUVCxform:                              return STI_InstancedYUVCxform;
        case ST_InstancedYUVCxformMul:                           return STI_InstancedYUVCxformMul;
        case ST_Position3dYUVCxform:                             return STI_Position3dYUVCxform;
        case ST_Position3dYUVCxformMul:                          return STI_Position3dYUVCxformMul;
        case ST_BatchPosition3dYUVCxform:                        return STI_BatchPosition3dYUVCxform;
        case ST_BatchPosition3dYUVCxformMul:                     return STI_BatchPosition3dYUVCxformMul;
        case ST_InstancedPosition3dYUVCxform:                    return STI_InstancedPosition3dYUVCxform;
        case ST_InstancedPosition3dYUVCxformMul:                 return STI_InstancedPosition3dYUVCxformMul;
        case ST_YUVEAlpha:                                       return STI_YUVEAlpha;
        case ST_YUVEAlphaMul:                                    return STI_YUVEAlphaMul;
        case ST_BatchYUVEAlpha:                                  return STI_BatchYUVEAlpha;
        case ST_BatchYUVEAlphaMul:                               return STI_BatchYUVEAlphaMul;
        case ST_InstancedYUVEAlpha:                              return STI_InstancedYUVEAlpha;
        case ST_InstancedYUVEAlphaMul:                           return STI_InstancedYUVEAlphaMul;
        case ST_Position3dYUVEAlpha:                             return STI_Position3dYUVEAlpha;
        case ST_Position3dYUVEAlphaMul:                          return STI_Position3dYUVEAlphaMul;
        case ST_BatchPosition3dYUVEAlpha:                        return STI_BatchPosition3dYUVEAlpha;
        case ST_BatchPosition3dYUVEAlphaMul:                     return STI_BatchPosition3dYUVEAlphaMul;
        case ST_InstancedPosition3dYUVEAlpha:                    return STI_InstancedPosition3dYUVEAlpha;
        case ST_InstancedPosition3dYUVEAlphaMul:                 return STI_InstancedPosition3dYUVEAlphaMul;
        case ST_YUVCxformEAlpha:                                 return STI_YUVCxformEAlpha;
        case ST_YUVCxformEAlphaMul:                              return STI_YUVCxformEAlphaMul;
        case ST_BatchYUVCxformEAlpha:                            return STI_BatchYUVCxformEAlpha;
        case ST_BatchYUVCxformEAlphaMul:                         return STI_BatchYUVCxformEAlphaMul;
        case ST_InstancedYUVCxformEAlpha:                        return STI_InstancedYUVCxformEAlpha;
        case ST_InstancedYUVCxformEAlphaMul:                     return STI_InstancedYUVCxformEAlphaMul;
        case ST_Position3dYUVCxformEAlpha:                       return STI_Position3dYUVCxformEAlpha;
        case ST_Position3dYUVCxformEAlphaMul:                    return STI_Position3dYUVCxformEAlphaMul;
        case ST_BatchPosition3dYUVCxformEAlpha:                  return STI_BatchPosition3dYUVCxformEAlpha;
        case ST_BatchPosition3dYUVCxformEAlphaMul:               return STI_BatchPosition3dYUVCxformEAlphaMul;
        case ST_InstancedPosition3dYUVCxformEAlpha:              return STI_InstancedPosition3dYUVCxformEAlpha;
        case ST_InstancedPosition3dYUVCxformEAlphaMul:           return STI_InstancedPosition3dYUVCxformEAlphaMul;
        case ST_YUVCxformAc:                                     return STI_YUVCxformAc;
        case ST_YUVCxformAcMul:                                  return STI_YUVCxformAcMul;
        case ST_BatchYUVCxformAc:                                return STI_BatchYUVCxformAc;
        case ST_BatchYUVCxformAcMul:                             return STI_BatchYUVCxformAcMul;
        case ST_InstancedYUVCxformAc:                            return STI_InstancedYUVCxformAc;
        case ST_InstancedYUVCxformAcMul:                         return STI_InstancedYUVCxformAcMul;
        case ST_Position3dYUVCxformAc:                           return STI_Position3dYUVCxformAc;
        case ST_Position3dYUVCxformAcMul:                        return STI_Position3dYUVCxformAcMul;
        case ST_BatchPosition3dYUVCxformAc:                      return STI_BatchPosition3dYUVCxformAc;
        case ST_BatchPosition3dYUVCxformAcMul:                   return STI_BatchPosition3dYUVCxformAcMul;
        case ST_InstancedPosition3dYUVCxformAc:                  return STI_InstancedPosition3dYUVCxformAc;
        case ST_InstancedPosition3dYUVCxformAcMul:               return STI_InstancedPosition3dYUVCxformAcMul;
        case ST_YUVCxformAcEAlpha:                               return STI_YUVCxformAcEAlpha;
        case ST_YUVCxformAcEAlphaMul:                            return STI_YUVCxformAcEAlphaMul;
        case ST_BatchYUVCxformAcEAlpha:                          return STI_BatchYUVCxformAcEAlpha;
        case ST_BatchYUVCxformAcEAlphaMul:                       return STI_BatchYUVCxformAcEAlphaMul;
        case ST_InstancedYUVCxformAcEAlpha:                      return STI_InstancedYUVCxformAcEAlpha;
        case ST_InstancedYUVCxformAcEAlphaMul:                   return STI_InstancedYUVCxformAcEAlphaMul;
        case ST_Position3dYUVCxformAcEAlpha:                     return STI_Position3dYUVCxformAcEAlpha;
        case ST_Position3dYUVCxformAcEAlphaMul:                  return STI_Position3dYUVCxformAcEAlphaMul;
        case ST_BatchPosition3dYUVCxformAcEAlpha:                return STI_BatchPosition3dYUVCxformAcEAlpha;
        case ST_BatchPosition3dYUVCxformAcEAlphaMul:             return STI_BatchPosition3dYUVCxformAcEAlphaMul;
        case ST_InstancedPosition3dYUVCxformAcEAlpha:            return STI_InstancedPosition3dYUVCxformAcEAlpha;
        case ST_InstancedPosition3dYUVCxformAcEAlphaMul:         return STI_InstancedPosition3dYUVCxformAcEAlphaMul;
        case ST_YUVA:                                            return STI_YUVA;
        case ST_YUVAMul:                                         return STI_YUVAMul;
        case ST_BatchYUVA:                                       return STI_BatchYUVA;
        case ST_BatchYUVAMul:                                    return STI_BatchYUVAMul;
        case ST_InstancedYUVA:                                   return STI_InstancedYUVA;
        case ST_InstancedYUVAMul:                                return STI_InstancedYUVAMul;
        case ST_Position3dYUVA:                                  return STI_Position3dYUVA;
        case ST_Position3dYUVAMul:                               return STI_Position3dYUVAMul;
        case ST_BatchPosition3dYUVA:                             return STI_BatchPosition3dYUVA;
        case ST_BatchPosition3dYUVAMul:                          return STI_BatchPosition3dYUVAMul;
        case ST_InstancedPosition3dYUVA:                         return STI_InstancedPosition3dYUVA;
        case ST_InstancedPosition3dYUVAMul:                      return STI_InstancedPosition3dYUVAMul;
        case ST_YUVACxform:                                      return STI_YUVACxform;
        case ST_YUVACxformMul:                                   return STI_YUVACxformMul;
        case ST_BatchYUVACxform:                                 return STI_BatchYUVACxform;
        case ST_BatchYUVACxformMul:                              return STI_BatchYUVACxformMul;
        case ST_InstancedYUVACxform:                             return STI_InstancedYUVACxform;
        case ST_InstancedYUVACxformMul:                          return STI_InstancedYUVACxformMul;
        case ST_Position3dYUVACxform:                            return STI_Position3dYUVACxform;
        case ST_Position3dYUVACxformMul:                         return STI_Position3dYUVACxformMul;
        case ST_BatchPosition3dYUVACxform:                       return STI_BatchPosition3dYUVACxform;
        case ST_BatchPosition3dYUVACxformMul:                    return STI_BatchPosition3dYUVACxformMul;
        case ST_InstancedPosition3dYUVACxform:                   return STI_InstancedPosition3dYUVACxform;
        case ST_InstancedPosition3dYUVACxformMul:                return STI_InstancedPosition3dYUVACxformMul;
        case ST_YUVAEAlpha:                                      return STI_YUVAEAlpha;
        case ST_YUVAEAlphaMul:                                   return STI_YUVAEAlphaMul;
        case ST_BatchYUVAEAlpha:                                 return STI_BatchYUVAEAlpha;
        case ST_BatchYUVAEAlphaMul:                              return STI_BatchYUVAEAlphaMul;
        case ST_InstancedYUVAEAlpha:                             return STI_InstancedYUVAEAlpha;
        case ST_InstancedYUVAEAlphaMul:                          return STI_InstancedYUVAEAlphaMul;
        case ST_Position3dYUVAEAlpha:                            return STI_Position3dYUVAEAlpha;
        case ST_Position3dYUVAEAlphaMul:                         return STI_Position3dYUVAEAlphaMul;
        case ST_BatchPosition3dYUVAEAlpha:                       return STI_BatchPosition3dYUVAEAlpha;
        case ST_BatchPosition3dYUVAEAlphaMul:                    return STI_BatchPosition3dYUVAEAlphaMul;
        case ST_InstancedPosition3dYUVAEAlpha:                   return STI_InstancedPosition3dYUVAEAlpha;
        case ST_InstancedPosition3dYUVAEAlphaMul:                return STI_InstancedPosition3dYUVAEAlphaMul;
        case ST_YUVACxformEAlpha:                                return STI_YUVACxformEAlpha;
        case ST_YUVACxformEAlphaMul:                             return STI_YUVACxformEAlphaMul;
        case ST_BatchYUVACxformEAlpha:                           return STI_BatchYUVACxformEAlpha;
        case ST_BatchYUVACxformEAlphaMul:                        return STI_BatchYUVACxformEAlphaMul;
        case ST_InstancedYUVACxformEAlpha:                       return STI_InstancedYUVACxformEAlpha;
        case ST_InstancedYUVACxformEAlphaMul:                    return STI_InstancedYUVACxformEAlphaMul;
        case ST_Position3dYUVACxformEAlpha:                      return STI_Position3dYUVACxformEAlpha;
        case ST_Position3dYUVACxformEAlphaMul:                   return STI_Position3dYUVACxformEAlphaMul;
        case ST_BatchPosition3dYUVACxformEAlpha:                 return STI_BatchPosition3dYUVACxformEAlpha;
        case ST_BatchPosition3dYUVACxformEAlphaMul:              return STI_BatchPosition3dYUVACxformEAlphaMul;
        case ST_InstancedPosition3dYUVACxformEAlpha:             return STI_InstancedPosition3dYUVACxformEAlpha;
        case ST_InstancedPosition3dYUVACxformEAlphaMul:          return STI_InstancedPosition3dYUVACxformEAlphaMul;
        case ST_YUVAInv:                                         return STI_YUVAInv;
        case ST_BatchYUVAInv:                                    return STI_BatchYUVAInv;
        case ST_InstancedYUVAInv:                                return STI_InstancedYUVAInv;
        case ST_Position3dYUVAInv:                               return STI_Position3dYUVAInv;
        case ST_BatchPosition3dYUVAInv:                          return STI_BatchPosition3dYUVAInv;
        case ST_InstancedPosition3dYUVAInv:                      return STI_InstancedPosition3dYUVAInv;
        case ST_YUVACxformInv:                                   return STI_YUVACxformInv;
        case ST_BatchYUVACxformInv:                              return STI_BatchYUVACxformInv;
        case ST_InstancedYUVACxformInv:                          return STI_InstancedYUVACxformInv;
        case ST_Position3dYUVACxformInv:                         return STI_Position3dYUVACxformInv;
        case ST_BatchPosition3dYUVACxformInv:                    return STI_BatchPosition3dYUVACxformInv;
        case ST_InstancedPosition3dYUVACxformInv:                return STI_InstancedPosition3dYUVACxformInv;
        case ST_YUVAEAlphaInv:                                   return STI_YUVAEAlphaInv;
        case ST_BatchYUVAEAlphaInv:                              return STI_BatchYUVAEAlphaInv;
        case ST_InstancedYUVAEAlphaInv:                          return STI_InstancedYUVAEAlphaInv;
        case ST_Position3dYUVAEAlphaInv:                         return STI_Position3dYUVAEAlphaInv;
        case ST_BatchPosition3dYUVAEAlphaInv:                    return STI_BatchPosition3dYUVAEAlphaInv;
        case ST_InstancedPosition3dYUVAEAlphaInv:                return STI_InstancedPosition3dYUVAEAlphaInv;
        case ST_YUVACxformEAlphaInv:                             return STI_YUVACxformEAlphaInv;
        case ST_BatchYUVACxformEAlphaInv:                        return STI_BatchYUVACxformEAlphaInv;
        case ST_InstancedYUVACxformEAlphaInv:                    return STI_InstancedYUVACxformEAlphaInv;
        case ST_Position3dYUVACxformEAlphaInv:                   return STI_Position3dYUVACxformEAlphaInv;
        case ST_BatchPosition3dYUVACxformEAlphaInv:              return STI_BatchPosition3dYUVACxformEAlphaInv;
        case ST_InstancedPosition3dYUVACxformEAlphaInv:          return STI_InstancedPosition3dYUVACxformEAlphaInv;
        case ST_YUVACxformAc:                                    return STI_YUVACxformAc;
        case ST_YUVACxformAcMul:                                 return STI_YUVACxformAcMul;
        case ST_BatchYUVACxformAc:                               return STI_BatchYUVACxformAc;
        case ST_BatchYUVACxformAcMul:                            return STI_BatchYUVACxformAcMul;
        case ST_InstancedYUVACxformAc:                           return STI_InstancedYUVACxformAc;
        case ST_InstancedYUVACxformAcMul:                        return STI_InstancedYUVACxformAcMul;
        case ST_Position3dYUVACxformAc:                          return STI_Position3dYUVACxformAc;
        case ST_Position3dYUVACxformAcMul:                       return STI_Position3dYUVACxformAcMul;
        case ST_BatchPosition3dYUVACxformAc:                     return STI_BatchPosition3dYUVACxformAc;
        case ST_BatchPosition3dYUVACxformAcMul:                  return STI_BatchPosition3dYUVACxformAcMul;
        case ST_InstancedPosition3dYUVACxformAc:                 return STI_InstancedPosition3dYUVACxformAc;
        case ST_InstancedPosition3dYUVACxformAcMul:              return STI_InstancedPosition3dYUVACxformAcMul;
        case ST_YUVACxformAcEAlpha:                              return STI_YUVACxformAcEAlpha;
        case ST_YUVACxformAcEAlphaMul:                           return STI_YUVACxformAcEAlphaMul;
        case ST_BatchYUVACxformAcEAlpha:                         return STI_BatchYUVACxformAcEAlpha;
        case ST_BatchYUVACxformAcEAlphaMul:                      return STI_BatchYUVACxformAcEAlphaMul;
        case ST_InstancedYUVACxformAcEAlpha:                     return STI_InstancedYUVACxformAcEAlpha;
        case ST_InstancedYUVACxformAcEAlphaMul:                  return STI_InstancedYUVACxformAcEAlphaMul;
        case ST_Position3dYUVACxformAcEAlpha:                    return STI_Position3dYUVACxformAcEAlpha;
        case ST_Position3dYUVACxformAcEAlphaMul:                 return STI_Position3dYUVACxformAcEAlphaMul;
        case ST_BatchPosition3dYUVACxformAcEAlpha:               return STI_BatchPosition3dYUVACxformAcEAlpha;
        case ST_BatchPosition3dYUVACxformAcEAlphaMul:            return STI_BatchPosition3dYUVACxformAcEAlphaMul;
        case ST_InstancedPosition3dYUVACxformAcEAlpha:           return STI_InstancedPosition3dYUVACxformAcEAlpha;
        case ST_InstancedPosition3dYUVACxformAcEAlphaMul:        return STI_InstancedPosition3dYUVACxformAcEAlphaMul;
        case ST_YUVACxformAcInv:                                 return STI_YUVACxformAcInv;
        case ST_BatchYUVACxformAcInv:                            return STI_BatchYUVACxformAcInv;
        case ST_InstancedYUVACxformAcInv:                        return STI_InstancedYUVACxformAcInv;
        case ST_Position3dYUVACxformAcInv:                       return STI_Position3dYUVACxformAcInv;
        case ST_BatchPosition3dYUVACxformAcInv:                  return STI_BatchPosition3dYUVACxformAcInv;
        case ST_InstancedPosition3dYUVACxformAcInv:              return STI_InstancedPosition3dYUVACxformAcInv;
        case ST_YUVACxformAcEAlphaInv:                           return STI_YUVACxformAcEAlphaInv;
        case ST_BatchYUVACxformAcEAlphaInv:                      return STI_BatchYUVACxformAcEAlphaInv;
        case ST_InstancedYUVACxformAcEAlphaInv:                  return STI_InstancedYUVACxformAcEAlphaInv;
        case ST_Position3dYUVACxformAcEAlphaInv:                 return STI_Position3dYUVACxformAcEAlphaInv;
        case ST_BatchPosition3dYUVACxformAcEAlphaInv:            return STI_BatchPosition3dYUVACxformAcEAlphaInv;
        case ST_InstancedPosition3dYUVACxformAcEAlphaInv:        return STI_InstancedPosition3dYUVACxformAcEAlphaInv;
        case ST_TexTGCMatrixAc:                                  return STI_TexTGCMatrixAc;
        case ST_TexTGCMatrixAcMul:                               return STI_TexTGCMatrixAcMul;
        case ST_TexTGCMatrixAcEAlpha:                            return STI_TexTGCMatrixAcEAlpha;
        case ST_TexTGCMatrixAcEAlphaMul:                         return STI_TexTGCMatrixAcEAlphaMul;
        case ST_Box1Blur:                                        return STI_Box1Blur;
        case ST_Box1BlurMul:                                     return STI_Box1BlurMul;
        case ST_Box2Blur:                                        return STI_Box2Blur;
        case ST_Box2BlurMul:                                     return STI_Box2BlurMul;
        case ST_Box2Shadow:                                      return STI_Box2Shadow;
        case ST_Box2ShadowMul:                                   return STI_Box2ShadowMul;
        case ST_Box2ShadowKnockout:                              return STI_Box2ShadowKnockout;
        case ST_Box2ShadowKnockoutMul:                           return STI_Box2ShadowKnockoutMul;
        case ST_Box2Shadowonly:                                  return STI_Box2Shadowonly;
        case ST_Box2ShadowonlyMul:                               return STI_Box2ShadowonlyMul;
        case ST_Box2InnerShadow:                                 return STI_Box2InnerShadow;
        case ST_Box2InnerShadowMul:                              return STI_Box2InnerShadowMul;
        case ST_Box2InnerShadowKnockout:                         return STI_Box2InnerShadowKnockout;
        case ST_Box2InnerShadowKnockoutMul:                      return STI_Box2InnerShadowKnockoutMul;
        case ST_Box2ShadowonlyHighlight:                         return STI_Box2ShadowonlyHighlight;
        case ST_Box2ShadowonlyHighlightMul:                      return STI_Box2ShadowonlyHighlightMul;
        case ST_Box2FullShadowHighlight:                         return STI_Box2FullShadowHighlight;
        case ST_Box2FullShadowHighlightMul:                      return STI_Box2FullShadowHighlightMul;
        case ST_Box2InnerShadowHighlight:                        return STI_Box2InnerShadowHighlight;
        case ST_Box2InnerShadowHighlightMul:                     return STI_Box2InnerShadowHighlightMul;
        case ST_Box2InnerShadowHighlightKnockout:                return STI_Box2InnerShadowHighlightKnockout;
        case ST_Box2InnerShadowHighlightKnockoutMul:             return STI_Box2InnerShadowHighlightKnockoutMul;
        case ST_Box2ShadowHighlight:                             return STI_Box2ShadowHighlight;
        case ST_Box2ShadowHighlightMul:                          return STI_Box2ShadowHighlightMul;
        case ST_Box2ShadowHighlightKnockout:                     return STI_Box2ShadowHighlightKnockout;
        case ST_Box2ShadowHighlightKnockoutMul:                  return STI_Box2ShadowHighlightKnockoutMul;
        case ST_DrawableCopyPixels:                              return STI_DrawableCopyPixels;
        case ST_DrawableCopyPixelsNoDestAlpha:                   return STI_DrawableCopyPixelsNoDestAlpha;
        case ST_DrawableCopyPixelsMergeAlpha:                    return STI_DrawableCopyPixelsMergeAlpha;
        case ST_DrawableCopyPixelsAlpha:                         return STI_DrawableCopyPixelsAlpha;
        case ST_DrawableCopyPixelsAlphaNoDestAlpha:              return STI_DrawableCopyPixelsAlphaNoDestAlpha;
        case ST_DrawableCopyPixelsAlphaMergeAlpha:               return STI_DrawableCopyPixelsAlphaMergeAlpha;
        case ST_DrawableMerge:                                   return STI_DrawableMerge;
        case ST_DrawableCompare:                                 return STI_DrawableCompare;
        case ST_DrawablePaletteMap:                              return STI_DrawablePaletteMap;
        default: SF_DEBUG_ASSERT1(1, "Invalid ShaderType (%d)", type);
    }
    return STI_None;
}

struct ShaderIndexEntry
{
    VertexShaderDesc::ShaderIndex VertexIndex;
    FragShaderDesc::ShaderIndex FragIndex;
};

static const ShaderIndexEntry ShaderIndexingData[ShaderDesc::STI_Count][ShaderDesc::ShaderVersion_Count] = 
{
    { // ST_None
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
    },
    { // ST_TexTG
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTG,
            FragShaderDesc::FSI_PS3_FTexTG
        }
    },
    { // ST_TexTGMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGMul,
            FragShaderDesc::FSI_PS3_FTexTGMul
        }
    },
    { // ST_BatchTexTG
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTG,
            FragShaderDesc::FSI_PS3_FBatchTexTG
        }
    },
    { // ST_BatchTexTGMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGMul,
            FragShaderDesc::FSI_PS3_FBatchTexTGMul
        }
    },
    { // ST_InstancedTexTG
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTG,
            FragShaderDesc::FSI_PS3_FInstancedTexTG
        }
    },
    { // ST_InstancedTexTGMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGMul,
            FragShaderDesc::FSI_PS3_FInstancedTexTGMul
        }
    },
    { // ST_Position3dTexTG
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTG,
            FragShaderDesc::FSI_PS3_FPosition3dTexTG
        }
    },
    { // ST_Position3dTexTGMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGMul,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGMul
        }
    },
    { // ST_BatchPosition3dTexTG
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTG,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTG
        }
    },
    { // ST_BatchPosition3dTexTGMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGMul
        }
    },
    { // ST_InstancedPosition3dTexTG
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTG,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTG
        }
    },
    { // ST_InstancedPosition3dTexTGMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGMul
        }
    },
    { // ST_TexTGCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGCxform,
            FragShaderDesc::FSI_PS3_FTexTGCxform
        }
    },
    { // ST_TexTGCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGCxformMul,
            FragShaderDesc::FSI_PS3_FTexTGCxformMul
        }
    },
    { // ST_BatchTexTGCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGCxform,
            FragShaderDesc::FSI_PS3_FBatchTexTGCxform
        }
    },
    { // ST_BatchTexTGCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGCxformMul,
            FragShaderDesc::FSI_PS3_FBatchTexTGCxformMul
        }
    },
    { // ST_InstancedTexTGCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGCxform,
            FragShaderDesc::FSI_PS3_FInstancedTexTGCxform
        }
    },
    { // ST_InstancedTexTGCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGCxformMul,
            FragShaderDesc::FSI_PS3_FInstancedTexTGCxformMul
        }
    },
    { // ST_Position3dTexTGCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGCxform,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGCxform
        }
    },
    { // ST_Position3dTexTGCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGCxformMul,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGCxformMul
        }
    },
    { // ST_BatchPosition3dTexTGCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGCxform,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGCxform
        }
    },
    { // ST_BatchPosition3dTexTGCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGCxformMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGCxformMul
        }
    },
    { // ST_InstancedPosition3dTexTGCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGCxform,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGCxform
        }
    },
    { // ST_InstancedPosition3dTexTGCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGCxformMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGCxformMul
        }
    },
    { // ST_TexTGEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGEAlpha,
            FragShaderDesc::FSI_PS3_FTexTGEAlpha
        }
    },
    { // ST_TexTGEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGEAlphaMul,
            FragShaderDesc::FSI_PS3_FTexTGEAlphaMul
        }
    },
    { // ST_BatchTexTGEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGEAlpha,
            FragShaderDesc::FSI_PS3_FBatchTexTGEAlpha
        }
    },
    { // ST_BatchTexTGEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchTexTGEAlphaMul
        }
    },
    { // ST_InstancedTexTGEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedTexTGEAlpha
        }
    },
    { // ST_InstancedTexTGEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedTexTGEAlphaMul
        }
    },
    { // ST_Position3dTexTGEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGEAlpha,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGEAlpha
        }
    },
    { // ST_Position3dTexTGEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGEAlphaMul,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGEAlphaMul
        }
    },
    { // ST_BatchPosition3dTexTGEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGEAlpha,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGEAlpha
        }
    },
    { // ST_BatchPosition3dTexTGEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGEAlphaMul
        }
    },
    { // ST_InstancedPosition3dTexTGEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGEAlpha
        }
    },
    { // ST_InstancedPosition3dTexTGEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGEAlphaMul
        }
    },
    { // ST_TexTGCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGCxformEAlpha,
            FragShaderDesc::FSI_PS3_FTexTGCxformEAlpha
        }
    },
    { // ST_TexTGCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FTexTGCxformEAlphaMul
        }
    },
    { // ST_BatchTexTGCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGCxformEAlpha,
            FragShaderDesc::FSI_PS3_FBatchTexTGCxformEAlpha
        }
    },
    { // ST_BatchTexTGCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchTexTGCxformEAlphaMul
        }
    },
    { // ST_InstancedTexTGCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGCxformEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedTexTGCxformEAlpha
        }
    },
    { // ST_InstancedTexTGCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedTexTGCxformEAlphaMul
        }
    },
    { // ST_Position3dTexTGCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGCxformEAlpha,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGCxformEAlpha
        }
    },
    { // ST_Position3dTexTGCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGCxformEAlphaMul
        }
    },
    { // ST_BatchPosition3dTexTGCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGCxformEAlpha,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGCxformEAlpha
        }
    },
    { // ST_BatchPosition3dTexTGCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGCxformEAlphaMul
        }
    },
    { // ST_InstancedPosition3dTexTGCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGCxformEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGCxformEAlpha
        }
    },
    { // ST_InstancedPosition3dTexTGCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGCxformEAlphaMul
        }
    },
    { // ST_TexTGInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGInv,
            FragShaderDesc::FSI_PS3_FTexTGInv
        }
    },
    { // ST_BatchTexTGInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGInv,
            FragShaderDesc::FSI_PS3_FBatchTexTGInv
        }
    },
    { // ST_InstancedTexTGInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGInv,
            FragShaderDesc::FSI_PS3_FInstancedTexTGInv
        }
    },
    { // ST_Position3dTexTGInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGInv,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGInv
        }
    },
    { // ST_BatchPosition3dTexTGInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGInv
        }
    },
    { // ST_InstancedPosition3dTexTGInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGInv
        }
    },
    { // ST_TexTGCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGCxformInv,
            FragShaderDesc::FSI_PS3_FTexTGCxformInv
        }
    },
    { // ST_BatchTexTGCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGCxformInv,
            FragShaderDesc::FSI_PS3_FBatchTexTGCxformInv
        }
    },
    { // ST_InstancedTexTGCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGCxformInv,
            FragShaderDesc::FSI_PS3_FInstancedTexTGCxformInv
        }
    },
    { // ST_Position3dTexTGCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGCxformInv,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGCxformInv
        }
    },
    { // ST_BatchPosition3dTexTGCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGCxformInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGCxformInv
        }
    },
    { // ST_InstancedPosition3dTexTGCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGCxformInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGCxformInv
        }
    },
    { // ST_TexTGEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGEAlphaInv,
            FragShaderDesc::FSI_PS3_FTexTGEAlphaInv
        }
    },
    { // ST_BatchTexTGEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchTexTGEAlphaInv
        }
    },
    { // ST_InstancedTexTGEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedTexTGEAlphaInv
        }
    },
    { // ST_Position3dTexTGEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGEAlphaInv,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGEAlphaInv
        }
    },
    { // ST_BatchPosition3dTexTGEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGEAlphaInv
        }
    },
    { // ST_InstancedPosition3dTexTGEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGEAlphaInv
        }
    },
    { // ST_TexTGCxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FTexTGCxformEAlphaInv
        }
    },
    { // ST_BatchTexTGCxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchTexTGCxformEAlphaInv
        }
    },
    { // ST_InstancedTexTGCxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedTexTGCxformEAlphaInv
        }
    },
    { // ST_Position3dTexTGCxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGCxformEAlphaInv
        }
    },
    { // ST_BatchPosition3dTexTGCxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGCxformEAlphaInv
        }
    },
    { // ST_InstancedPosition3dTexTGCxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGCxformEAlphaInv
        }
    },
    { // ST_TexTGCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGCxformAc,
            FragShaderDesc::FSI_PS3_FTexTGCxformAc
        }
    },
    { // ST_TexTGCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGCxformAcMul,
            FragShaderDesc::FSI_PS3_FTexTGCxformAcMul
        }
    },
    { // ST_BatchTexTGCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGCxformAc,
            FragShaderDesc::FSI_PS3_FBatchTexTGCxformAc
        }
    },
    { // ST_BatchTexTGCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGCxformAcMul,
            FragShaderDesc::FSI_PS3_FBatchTexTGCxformAcMul
        }
    },
    { // ST_InstancedTexTGCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGCxformAc,
            FragShaderDesc::FSI_PS3_FInstancedTexTGCxformAc
        }
    },
    { // ST_InstancedTexTGCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGCxformAcMul,
            FragShaderDesc::FSI_PS3_FInstancedTexTGCxformAcMul
        }
    },
    { // ST_Position3dTexTGCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGCxformAc,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGCxformAc
        }
    },
    { // ST_Position3dTexTGCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGCxformAcMul,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGCxformAcMul
        }
    },
    { // ST_BatchPosition3dTexTGCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGCxformAc,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGCxformAc
        }
    },
    { // ST_BatchPosition3dTexTGCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGCxformAcMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGCxformAcMul
        }
    },
    { // ST_InstancedPosition3dTexTGCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGCxformAc,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGCxformAc
        }
    },
    { // ST_InstancedPosition3dTexTGCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGCxformAcMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGCxformAcMul
        }
    },
    { // ST_TexTGCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FTexTGCxformAcEAlpha
        }
    },
    { // ST_TexTGCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FTexTGCxformAcEAlphaMul
        }
    },
    { // ST_BatchTexTGCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FBatchTexTGCxformAcEAlpha
        }
    },
    { // ST_BatchTexTGCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchTexTGCxformAcEAlphaMul
        }
    },
    { // ST_InstancedTexTGCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedTexTGCxformAcEAlpha
        }
    },
    { // ST_InstancedTexTGCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedTexTGCxformAcEAlphaMul
        }
    },
    { // ST_Position3dTexTGCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGCxformAcEAlpha
        }
    },
    { // ST_Position3dTexTGCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGCxformAcEAlphaMul
        }
    },
    { // ST_BatchPosition3dTexTGCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGCxformAcEAlpha
        }
    },
    { // ST_BatchPosition3dTexTGCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGCxformAcEAlphaMul
        }
    },
    { // ST_InstancedPosition3dTexTGCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGCxformAcEAlpha
        }
    },
    { // ST_InstancedPosition3dTexTGCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGCxformAcEAlphaMul
        }
    },
    { // ST_TexTGCxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGCxformAcInv,
            FragShaderDesc::FSI_PS3_FTexTGCxformAcInv
        }
    },
    { // ST_BatchTexTGCxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGCxformAcInv,
            FragShaderDesc::FSI_PS3_FBatchTexTGCxformAcInv
        }
    },
    { // ST_InstancedTexTGCxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGCxformAcInv,
            FragShaderDesc::FSI_PS3_FInstancedTexTGCxformAcInv
        }
    },
    { // ST_Position3dTexTGCxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGCxformAcInv,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGCxformAcInv
        }
    },
    { // ST_BatchPosition3dTexTGCxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGCxformAcInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGCxformAcInv
        }
    },
    { // ST_InstancedPosition3dTexTGCxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGCxformAcInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGCxformAcInv
        }
    },
    { // ST_TexTGCxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FTexTGCxformAcEAlphaInv
        }
    },
    { // ST_BatchTexTGCxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchTexTGCxformAcEAlphaInv
        }
    },
    { // ST_InstancedTexTGCxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedTexTGCxformAcEAlphaInv
        }
    },
    { // ST_Position3dTexTGCxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGCxformAcEAlphaInv
        }
    },
    { // ST_BatchPosition3dTexTGCxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGCxformAcEAlphaInv
        }
    },
    { // ST_InstancedPosition3dTexTGCxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGCxformAcEAlphaInv
        }
    },
    { // ST_Vertex
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VVertex,
            FragShaderDesc::FSI_PS3_FVertex
        }
    },
    { // ST_VertexMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VVertexMul,
            FragShaderDesc::FSI_PS3_FVertexMul
        }
    },
    { // ST_BatchVertex
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchVertex,
            FragShaderDesc::FSI_PS3_FBatchVertex
        }
    },
    { // ST_BatchVertexMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchVertexMul,
            FragShaderDesc::FSI_PS3_FBatchVertexMul
        }
    },
    { // ST_InstancedVertex
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedVertex,
            FragShaderDesc::FSI_PS3_FInstancedVertex
        }
    },
    { // ST_InstancedVertexMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedVertexMul,
            FragShaderDesc::FSI_PS3_FInstancedVertexMul
        }
    },
    { // ST_Position3dVertex
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dVertex,
            FragShaderDesc::FSI_PS3_FPosition3dVertex
        }
    },
    { // ST_Position3dVertexMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dVertexMul,
            FragShaderDesc::FSI_PS3_FPosition3dVertexMul
        }
    },
    { // ST_BatchPosition3dVertex
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dVertex,
            FragShaderDesc::FSI_PS3_FBatchPosition3dVertex
        }
    },
    { // ST_BatchPosition3dVertexMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dVertexMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dVertexMul
        }
    },
    { // ST_InstancedPosition3dVertex
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dVertex,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dVertex
        }
    },
    { // ST_InstancedPosition3dVertexMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dVertexMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dVertexMul
        }
    },
    { // ST_VertexCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VVertexCxform,
            FragShaderDesc::FSI_PS3_FVertexCxform
        }
    },
    { // ST_VertexCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VVertexCxformMul,
            FragShaderDesc::FSI_PS3_FVertexCxformMul
        }
    },
    { // ST_BatchVertexCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchVertexCxform,
            FragShaderDesc::FSI_PS3_FBatchVertexCxform
        }
    },
    { // ST_BatchVertexCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchVertexCxformMul,
            FragShaderDesc::FSI_PS3_FBatchVertexCxformMul
        }
    },
    { // ST_InstancedVertexCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedVertexCxform,
            FragShaderDesc::FSI_PS3_FInstancedVertexCxform
        }
    },
    { // ST_InstancedVertexCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedVertexCxformMul,
            FragShaderDesc::FSI_PS3_FInstancedVertexCxformMul
        }
    },
    { // ST_Position3dVertexCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dVertexCxform,
            FragShaderDesc::FSI_PS3_FPosition3dVertexCxform
        }
    },
    { // ST_Position3dVertexCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dVertexCxformMul,
            FragShaderDesc::FSI_PS3_FPosition3dVertexCxformMul
        }
    },
    { // ST_BatchPosition3dVertexCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dVertexCxform,
            FragShaderDesc::FSI_PS3_FBatchPosition3dVertexCxform
        }
    },
    { // ST_BatchPosition3dVertexCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dVertexCxformMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dVertexCxformMul
        }
    },
    { // ST_InstancedPosition3dVertexCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dVertexCxform,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dVertexCxform
        }
    },
    { // ST_InstancedPosition3dVertexCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dVertexCxformMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dVertexCxformMul
        }
    },
    { // ST_VertexEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VVertexEAlpha,
            FragShaderDesc::FSI_PS3_FVertexEAlpha
        }
    },
    { // ST_VertexEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VVertexEAlphaMul,
            FragShaderDesc::FSI_PS3_FVertexEAlphaMul
        }
    },
    { // ST_BatchVertexEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchVertexEAlpha,
            FragShaderDesc::FSI_PS3_FBatchVertexEAlpha
        }
    },
    { // ST_BatchVertexEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchVertexEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchVertexEAlphaMul
        }
    },
    { // ST_InstancedVertexEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedVertexEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedVertexEAlpha
        }
    },
    { // ST_InstancedVertexEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedVertexEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedVertexEAlphaMul
        }
    },
    { // ST_Position3dVertexEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dVertexEAlpha,
            FragShaderDesc::FSI_PS3_FPosition3dVertexEAlpha
        }
    },
    { // ST_Position3dVertexEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dVertexEAlphaMul,
            FragShaderDesc::FSI_PS3_FPosition3dVertexEAlphaMul
        }
    },
    { // ST_BatchPosition3dVertexEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dVertexEAlpha,
            FragShaderDesc::FSI_PS3_FBatchPosition3dVertexEAlpha
        }
    },
    { // ST_BatchPosition3dVertexEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dVertexEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dVertexEAlphaMul
        }
    },
    { // ST_InstancedPosition3dVertexEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dVertexEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dVertexEAlpha
        }
    },
    { // ST_InstancedPosition3dVertexEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dVertexEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dVertexEAlphaMul
        }
    },
    { // ST_VertexCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VVertexCxformEAlpha,
            FragShaderDesc::FSI_PS3_FVertexCxformEAlpha
        }
    },
    { // ST_VertexCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VVertexCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FVertexCxformEAlphaMul
        }
    },
    { // ST_BatchVertexCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchVertexCxformEAlpha,
            FragShaderDesc::FSI_PS3_FBatchVertexCxformEAlpha
        }
    },
    { // ST_BatchVertexCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchVertexCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchVertexCxformEAlphaMul
        }
    },
    { // ST_InstancedVertexCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedVertexCxformEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedVertexCxformEAlpha
        }
    },
    { // ST_InstancedVertexCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedVertexCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedVertexCxformEAlphaMul
        }
    },
    { // ST_Position3dVertexCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dVertexCxformEAlpha,
            FragShaderDesc::FSI_PS3_FPosition3dVertexCxformEAlpha
        }
    },
    { // ST_Position3dVertexCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dVertexCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FPosition3dVertexCxformEAlphaMul
        }
    },
    { // ST_BatchPosition3dVertexCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dVertexCxformEAlpha,
            FragShaderDesc::FSI_PS3_FBatchPosition3dVertexCxformEAlpha
        }
    },
    { // ST_BatchPosition3dVertexCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dVertexCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dVertexCxformEAlphaMul
        }
    },
    { // ST_InstancedPosition3dVertexCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dVertexCxformEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dVertexCxformEAlpha
        }
    },
    { // ST_InstancedPosition3dVertexCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dVertexCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dVertexCxformEAlphaMul
        }
    },
    { // ST_VertexInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VVertexInv,
            FragShaderDesc::FSI_PS3_FVertexInv
        }
    },
    { // ST_BatchVertexInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchVertexInv,
            FragShaderDesc::FSI_PS3_FBatchVertexInv
        }
    },
    { // ST_InstancedVertexInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedVertexInv,
            FragShaderDesc::FSI_PS3_FInstancedVertexInv
        }
    },
    { // ST_Position3dVertexInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dVertexInv,
            FragShaderDesc::FSI_PS3_FPosition3dVertexInv
        }
    },
    { // ST_BatchPosition3dVertexInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dVertexInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dVertexInv
        }
    },
    { // ST_InstancedPosition3dVertexInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dVertexInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dVertexInv
        }
    },
    { // ST_VertexCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VVertexCxformInv,
            FragShaderDesc::FSI_PS3_FVertexCxformInv
        }
    },
    { // ST_BatchVertexCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchVertexCxformInv,
            FragShaderDesc::FSI_PS3_FBatchVertexCxformInv
        }
    },
    { // ST_InstancedVertexCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedVertexCxformInv,
            FragShaderDesc::FSI_PS3_FInstancedVertexCxformInv
        }
    },
    { // ST_Position3dVertexCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dVertexCxformInv,
            FragShaderDesc::FSI_PS3_FPosition3dVertexCxformInv
        }
    },
    { // ST_BatchPosition3dVertexCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dVertexCxformInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dVertexCxformInv
        }
    },
    { // ST_InstancedPosition3dVertexCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dVertexCxformInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dVertexCxformInv
        }
    },
    { // ST_VertexEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VVertexEAlphaInv,
            FragShaderDesc::FSI_PS3_FVertexEAlphaInv
        }
    },
    { // ST_BatchVertexEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchVertexEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchVertexEAlphaInv
        }
    },
    { // ST_InstancedVertexEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedVertexEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedVertexEAlphaInv
        }
    },
    { // ST_Position3dVertexEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dVertexEAlphaInv,
            FragShaderDesc::FSI_PS3_FPosition3dVertexEAlphaInv
        }
    },
    { // ST_BatchPosition3dVertexEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dVertexEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dVertexEAlphaInv
        }
    },
    { // ST_InstancedPosition3dVertexEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dVertexEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dVertexEAlphaInv
        }
    },
    { // ST_VertexCxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VVertexCxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FVertexCxformEAlphaInv
        }
    },
    { // ST_BatchVertexCxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchVertexCxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchVertexCxformEAlphaInv
        }
    },
    { // ST_InstancedVertexCxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedVertexCxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedVertexCxformEAlphaInv
        }
    },
    { // ST_Position3dVertexCxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dVertexCxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FPosition3dVertexCxformEAlphaInv
        }
    },
    { // ST_BatchPosition3dVertexCxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dVertexCxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dVertexCxformEAlphaInv
        }
    },
    { // ST_InstancedPosition3dVertexCxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dVertexCxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dVertexCxformEAlphaInv
        }
    },
    { // ST_VertexCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VVertexCxformAc,
            FragShaderDesc::FSI_PS3_FVertexCxformAc
        }
    },
    { // ST_VertexCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VVertexCxformAcMul,
            FragShaderDesc::FSI_PS3_FVertexCxformAcMul
        }
    },
    { // ST_BatchVertexCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchVertexCxformAc,
            FragShaderDesc::FSI_PS3_FBatchVertexCxformAc
        }
    },
    { // ST_BatchVertexCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchVertexCxformAcMul,
            FragShaderDesc::FSI_PS3_FBatchVertexCxformAcMul
        }
    },
    { // ST_InstancedVertexCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedVertexCxformAc,
            FragShaderDesc::FSI_PS3_FInstancedVertexCxformAc
        }
    },
    { // ST_InstancedVertexCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedVertexCxformAcMul,
            FragShaderDesc::FSI_PS3_FInstancedVertexCxformAcMul
        }
    },
    { // ST_Position3dVertexCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dVertexCxformAc,
            FragShaderDesc::FSI_PS3_FPosition3dVertexCxformAc
        }
    },
    { // ST_Position3dVertexCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dVertexCxformAcMul,
            FragShaderDesc::FSI_PS3_FPosition3dVertexCxformAcMul
        }
    },
    { // ST_BatchPosition3dVertexCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dVertexCxformAc,
            FragShaderDesc::FSI_PS3_FBatchPosition3dVertexCxformAc
        }
    },
    { // ST_BatchPosition3dVertexCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dVertexCxformAcMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dVertexCxformAcMul
        }
    },
    { // ST_InstancedPosition3dVertexCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dVertexCxformAc,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dVertexCxformAc
        }
    },
    { // ST_InstancedPosition3dVertexCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dVertexCxformAcMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dVertexCxformAcMul
        }
    },
    { // ST_VertexCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VVertexCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FVertexCxformAcEAlpha
        }
    },
    { // ST_VertexCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FVertexCxformAcEAlphaMul
        }
    },
    { // ST_BatchVertexCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchVertexCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FBatchVertexCxformAcEAlpha
        }
    },
    { // ST_BatchVertexCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchVertexCxformAcEAlphaMul
        }
    },
    { // ST_InstancedVertexCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedVertexCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedVertexCxformAcEAlpha
        }
    },
    { // ST_InstancedVertexCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedVertexCxformAcEAlphaMul
        }
    },
    { // ST_Position3dVertexCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dVertexCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FPosition3dVertexCxformAcEAlpha
        }
    },
    { // ST_Position3dVertexCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FPosition3dVertexCxformAcEAlphaMul
        }
    },
    { // ST_BatchPosition3dVertexCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dVertexCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FBatchPosition3dVertexCxformAcEAlpha
        }
    },
    { // ST_BatchPosition3dVertexCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dVertexCxformAcEAlphaMul
        }
    },
    { // ST_InstancedPosition3dVertexCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dVertexCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dVertexCxformAcEAlpha
        }
    },
    { // ST_InstancedPosition3dVertexCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dVertexCxformAcEAlphaMul
        }
    },
    { // ST_VertexCxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VVertexCxformAcInv,
            FragShaderDesc::FSI_PS3_FVertexCxformAcInv
        }
    },
    { // ST_BatchVertexCxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchVertexCxformAcInv,
            FragShaderDesc::FSI_PS3_FBatchVertexCxformAcInv
        }
    },
    { // ST_InstancedVertexCxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedVertexCxformAcInv,
            FragShaderDesc::FSI_PS3_FInstancedVertexCxformAcInv
        }
    },
    { // ST_Position3dVertexCxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dVertexCxformAcInv,
            FragShaderDesc::FSI_PS3_FPosition3dVertexCxformAcInv
        }
    },
    { // ST_BatchPosition3dVertexCxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dVertexCxformAcInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dVertexCxformAcInv
        }
    },
    { // ST_InstancedPosition3dVertexCxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dVertexCxformAcInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dVertexCxformAcInv
        }
    },
    { // ST_VertexCxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FVertexCxformAcEAlphaInv
        }
    },
    { // ST_BatchVertexCxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchVertexCxformAcEAlphaInv
        }
    },
    { // ST_InstancedVertexCxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedVertexCxformAcEAlphaInv
        }
    },
    { // ST_Position3dVertexCxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FPosition3dVertexCxformAcEAlphaInv
        }
    },
    { // ST_BatchPosition3dVertexCxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dVertexCxformAcEAlphaInv
        }
    },
    { // ST_InstancedPosition3dVertexCxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dVertexCxformAcEAlphaInv
        }
    },
    { // ST_TexTGTexTG
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGTexTG,
            FragShaderDesc::FSI_PS3_FTexTGTexTG
        }
    },
    { // ST_TexTGTexTGMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGTexTGMul,
            FragShaderDesc::FSI_PS3_FTexTGTexTGMul
        }
    },
    { // ST_BatchTexTGTexTG
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGTexTG,
            FragShaderDesc::FSI_PS3_FBatchTexTGTexTG
        }
    },
    { // ST_BatchTexTGTexTGMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGTexTGMul,
            FragShaderDesc::FSI_PS3_FBatchTexTGTexTGMul
        }
    },
    { // ST_InstancedTexTGTexTG
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGTexTG,
            FragShaderDesc::FSI_PS3_FInstancedTexTGTexTG
        }
    },
    { // ST_InstancedTexTGTexTGMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGTexTGMul,
            FragShaderDesc::FSI_PS3_FInstancedTexTGTexTGMul
        }
    },
    { // ST_Position3dTexTGTexTG
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGTexTG,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGTexTG
        }
    },
    { // ST_Position3dTexTGTexTGMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGTexTGMul,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGTexTGMul
        }
    },
    { // ST_BatchPosition3dTexTGTexTG
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGTexTG,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGTexTG
        }
    },
    { // ST_BatchPosition3dTexTGTexTGMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGTexTGMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGTexTGMul
        }
    },
    { // ST_InstancedPosition3dTexTGTexTG
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGTexTG,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGTexTG
        }
    },
    { // ST_InstancedPosition3dTexTGTexTGMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGTexTGMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGTexTGMul
        }
    },
    { // ST_TexTGTexTGCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGTexTGCxform,
            FragShaderDesc::FSI_PS3_FTexTGTexTGCxform
        }
    },
    { // ST_TexTGTexTGCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGTexTGCxformMul,
            FragShaderDesc::FSI_PS3_FTexTGTexTGCxformMul
        }
    },
    { // ST_BatchTexTGTexTGCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGTexTGCxform,
            FragShaderDesc::FSI_PS3_FBatchTexTGTexTGCxform
        }
    },
    { // ST_BatchTexTGTexTGCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGTexTGCxformMul,
            FragShaderDesc::FSI_PS3_FBatchTexTGTexTGCxformMul
        }
    },
    { // ST_InstancedTexTGTexTGCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGTexTGCxform,
            FragShaderDesc::FSI_PS3_FInstancedTexTGTexTGCxform
        }
    },
    { // ST_InstancedTexTGTexTGCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGTexTGCxformMul,
            FragShaderDesc::FSI_PS3_FInstancedTexTGTexTGCxformMul
        }
    },
    { // ST_Position3dTexTGTexTGCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGTexTGCxform,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGTexTGCxform
        }
    },
    { // ST_Position3dTexTGTexTGCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGTexTGCxformMul,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGTexTGCxformMul
        }
    },
    { // ST_BatchPosition3dTexTGTexTGCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGTexTGCxform,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGTexTGCxform
        }
    },
    { // ST_BatchPosition3dTexTGTexTGCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGTexTGCxformMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGTexTGCxformMul
        }
    },
    { // ST_InstancedPosition3dTexTGTexTGCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGTexTGCxform,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGTexTGCxform
        }
    },
    { // ST_InstancedPosition3dTexTGTexTGCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGTexTGCxformMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGTexTGCxformMul
        }
    },
    { // ST_TexTGTexTGEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGTexTGEAlpha,
            FragShaderDesc::FSI_PS3_FTexTGTexTGEAlpha
        }
    },
    { // ST_TexTGTexTGEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGTexTGEAlphaMul,
            FragShaderDesc::FSI_PS3_FTexTGTexTGEAlphaMul
        }
    },
    { // ST_BatchTexTGTexTGEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGTexTGEAlpha,
            FragShaderDesc::FSI_PS3_FBatchTexTGTexTGEAlpha
        }
    },
    { // ST_BatchTexTGTexTGEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGTexTGEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchTexTGTexTGEAlphaMul
        }
    },
    { // ST_InstancedTexTGTexTGEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGTexTGEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedTexTGTexTGEAlpha
        }
    },
    { // ST_InstancedTexTGTexTGEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGTexTGEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedTexTGTexTGEAlphaMul
        }
    },
    { // ST_Position3dTexTGTexTGEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGTexTGEAlpha,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGTexTGEAlpha
        }
    },
    { // ST_Position3dTexTGTexTGEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGTexTGEAlphaMul,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGTexTGEAlphaMul
        }
    },
    { // ST_BatchPosition3dTexTGTexTGEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGTexTGEAlpha,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGTexTGEAlpha
        }
    },
    { // ST_BatchPosition3dTexTGTexTGEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGTexTGEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGTexTGEAlphaMul
        }
    },
    { // ST_InstancedPosition3dTexTGTexTGEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGTexTGEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGTexTGEAlpha
        }
    },
    { // ST_InstancedPosition3dTexTGTexTGEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGTexTGEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGTexTGEAlphaMul
        }
    },
    { // ST_TexTGTexTGCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGTexTGCxformEAlpha,
            FragShaderDesc::FSI_PS3_FTexTGTexTGCxformEAlpha
        }
    },
    { // ST_TexTGTexTGCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FTexTGTexTGCxformEAlphaMul
        }
    },
    { // ST_BatchTexTGTexTGCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGTexTGCxformEAlpha,
            FragShaderDesc::FSI_PS3_FBatchTexTGTexTGCxformEAlpha
        }
    },
    { // ST_BatchTexTGTexTGCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchTexTGTexTGCxformEAlphaMul
        }
    },
    { // ST_InstancedTexTGTexTGCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGTexTGCxformEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedTexTGTexTGCxformEAlpha
        }
    },
    { // ST_InstancedTexTGTexTGCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedTexTGTexTGCxformEAlphaMul
        }
    },
    { // ST_Position3dTexTGTexTGCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGTexTGCxformEAlpha,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGTexTGCxformEAlpha
        }
    },
    { // ST_Position3dTexTGTexTGCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGTexTGCxformEAlphaMul
        }
    },
    { // ST_BatchPosition3dTexTGTexTGCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGTexTGCxformEAlpha,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGTexTGCxformEAlpha
        }
    },
    { // ST_BatchPosition3dTexTGTexTGCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGTexTGCxformEAlphaMul
        }
    },
    { // ST_InstancedPosition3dTexTGTexTGCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGTexTGCxformEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGTexTGCxformEAlpha
        }
    },
    { // ST_InstancedPosition3dTexTGTexTGCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGTexTGCxformEAlphaMul
        }
    },
    { // ST_TexTGTexTGInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGTexTGInv,
            FragShaderDesc::FSI_PS3_FTexTGTexTGInv
        }
    },
    { // ST_BatchTexTGTexTGInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGTexTGInv,
            FragShaderDesc::FSI_PS3_FBatchTexTGTexTGInv
        }
    },
    { // ST_InstancedTexTGTexTGInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGTexTGInv,
            FragShaderDesc::FSI_PS3_FInstancedTexTGTexTGInv
        }
    },
    { // ST_Position3dTexTGTexTGInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGTexTGInv,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGTexTGInv
        }
    },
    { // ST_BatchPosition3dTexTGTexTGInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGTexTGInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGTexTGInv
        }
    },
    { // ST_InstancedPosition3dTexTGTexTGInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGTexTGInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGTexTGInv
        }
    },
    { // ST_TexTGTexTGCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGTexTGCxformInv,
            FragShaderDesc::FSI_PS3_FTexTGTexTGCxformInv
        }
    },
    { // ST_BatchTexTGTexTGCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGTexTGCxformInv,
            FragShaderDesc::FSI_PS3_FBatchTexTGTexTGCxformInv
        }
    },
    { // ST_InstancedTexTGTexTGCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGTexTGCxformInv,
            FragShaderDesc::FSI_PS3_FInstancedTexTGTexTGCxformInv
        }
    },
    { // ST_Position3dTexTGTexTGCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGTexTGCxformInv,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGTexTGCxformInv
        }
    },
    { // ST_BatchPosition3dTexTGTexTGCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGTexTGCxformInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGTexTGCxformInv
        }
    },
    { // ST_InstancedPosition3dTexTGTexTGCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGTexTGCxformInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGTexTGCxformInv
        }
    },
    { // ST_TexTGTexTGEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGTexTGEAlphaInv,
            FragShaderDesc::FSI_PS3_FTexTGTexTGEAlphaInv
        }
    },
    { // ST_BatchTexTGTexTGEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGTexTGEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchTexTGTexTGEAlphaInv
        }
    },
    { // ST_InstancedTexTGTexTGEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGTexTGEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedTexTGTexTGEAlphaInv
        }
    },
    { // ST_Position3dTexTGTexTGEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGTexTGEAlphaInv,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGTexTGEAlphaInv
        }
    },
    { // ST_BatchPosition3dTexTGTexTGEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGTexTGEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGTexTGEAlphaInv
        }
    },
    { // ST_InstancedPosition3dTexTGTexTGEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGTexTGEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGTexTGEAlphaInv
        }
    },
    { // ST_TexTGTexTGCxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FTexTGTexTGCxformEAlphaInv
        }
    },
    { // ST_BatchTexTGTexTGCxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchTexTGTexTGCxformEAlphaInv
        }
    },
    { // ST_InstancedTexTGTexTGCxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedTexTGTexTGCxformEAlphaInv
        }
    },
    { // ST_Position3dTexTGTexTGCxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGTexTGCxformEAlphaInv
        }
    },
    { // ST_BatchPosition3dTexTGTexTGCxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGTexTGCxformEAlphaInv
        }
    },
    { // ST_InstancedPosition3dTexTGTexTGCxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGTexTGCxformEAlphaInv
        }
    },
    { // ST_TexTGTexTGCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGTexTGCxformAc,
            FragShaderDesc::FSI_PS3_FTexTGTexTGCxformAc
        }
    },
    { // ST_TexTGTexTGCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGTexTGCxformAcMul,
            FragShaderDesc::FSI_PS3_FTexTGTexTGCxformAcMul
        }
    },
    { // ST_BatchTexTGTexTGCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGTexTGCxformAc,
            FragShaderDesc::FSI_PS3_FBatchTexTGTexTGCxformAc
        }
    },
    { // ST_BatchTexTGTexTGCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGTexTGCxformAcMul,
            FragShaderDesc::FSI_PS3_FBatchTexTGTexTGCxformAcMul
        }
    },
    { // ST_InstancedTexTGTexTGCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGTexTGCxformAc,
            FragShaderDesc::FSI_PS3_FInstancedTexTGTexTGCxformAc
        }
    },
    { // ST_InstancedTexTGTexTGCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGTexTGCxformAcMul,
            FragShaderDesc::FSI_PS3_FInstancedTexTGTexTGCxformAcMul
        }
    },
    { // ST_Position3dTexTGTexTGCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGTexTGCxformAc,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGTexTGCxformAc
        }
    },
    { // ST_Position3dTexTGTexTGCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGTexTGCxformAcMul,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGTexTGCxformAcMul
        }
    },
    { // ST_BatchPosition3dTexTGTexTGCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGTexTGCxformAc,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGTexTGCxformAc
        }
    },
    { // ST_BatchPosition3dTexTGTexTGCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGTexTGCxformAcMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGTexTGCxformAcMul
        }
    },
    { // ST_InstancedPosition3dTexTGTexTGCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGTexTGCxformAc,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGTexTGCxformAc
        }
    },
    { // ST_InstancedPosition3dTexTGTexTGCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGTexTGCxformAcMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGTexTGCxformAcMul
        }
    },
    { // ST_TexTGTexTGCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FTexTGTexTGCxformAcEAlpha
        }
    },
    { // ST_TexTGTexTGCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FTexTGTexTGCxformAcEAlphaMul
        }
    },
    { // ST_BatchTexTGTexTGCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FBatchTexTGTexTGCxformAcEAlpha
        }
    },
    { // ST_BatchTexTGTexTGCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchTexTGTexTGCxformAcEAlphaMul
        }
    },
    { // ST_InstancedTexTGTexTGCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedTexTGTexTGCxformAcEAlpha
        }
    },
    { // ST_InstancedTexTGTexTGCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedTexTGTexTGCxformAcEAlphaMul
        }
    },
    { // ST_Position3dTexTGTexTGCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGTexTGCxformAcEAlpha
        }
    },
    { // ST_Position3dTexTGTexTGCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGTexTGCxformAcEAlphaMul
        }
    },
    { // ST_BatchPosition3dTexTGTexTGCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGTexTGCxformAcEAlpha
        }
    },
    { // ST_BatchPosition3dTexTGTexTGCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGTexTGCxformAcEAlphaMul
        }
    },
    { // ST_InstancedPosition3dTexTGTexTGCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGTexTGCxformAcEAlpha
        }
    },
    { // ST_InstancedPosition3dTexTGTexTGCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGTexTGCxformAcEAlphaMul
        }
    },
    { // ST_TexTGTexTGCxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGTexTGCxformAcInv,
            FragShaderDesc::FSI_PS3_FTexTGTexTGCxformAcInv
        }
    },
    { // ST_BatchTexTGTexTGCxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGTexTGCxformAcInv,
            FragShaderDesc::FSI_PS3_FBatchTexTGTexTGCxformAcInv
        }
    },
    { // ST_InstancedTexTGTexTGCxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGTexTGCxformAcInv,
            FragShaderDesc::FSI_PS3_FInstancedTexTGTexTGCxformAcInv
        }
    },
    { // ST_Position3dTexTGTexTGCxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGTexTGCxformAcInv,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGTexTGCxformAcInv
        }
    },
    { // ST_BatchPosition3dTexTGTexTGCxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGTexTGCxformAcInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGTexTGCxformAcInv
        }
    },
    { // ST_InstancedPosition3dTexTGTexTGCxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGTexTGCxformAcInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGTexTGCxformAcInv
        }
    },
    { // ST_TexTGTexTGCxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FTexTGTexTGCxformAcEAlphaInv
        }
    },
    { // ST_BatchTexTGTexTGCxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchTexTGTexTGCxformAcEAlphaInv
        }
    },
    { // ST_InstancedTexTGTexTGCxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedTexTGTexTGCxformAcEAlphaInv
        }
    },
    { // ST_Position3dTexTGTexTGCxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGTexTGCxformAcEAlphaInv
        }
    },
    { // ST_BatchPosition3dTexTGTexTGCxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGTexTGCxformAcEAlphaInv
        }
    },
    { // ST_InstancedPosition3dTexTGTexTGCxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGTexTGCxformAcEAlphaInv
        }
    },
    { // ST_TexTGVertex
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGVertex,
            FragShaderDesc::FSI_PS3_FTexTGVertex
        }
    },
    { // ST_TexTGVertexMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGVertexMul,
            FragShaderDesc::FSI_PS3_FTexTGVertexMul
        }
    },
    { // ST_BatchTexTGVertex
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGVertex,
            FragShaderDesc::FSI_PS3_FBatchTexTGVertex
        }
    },
    { // ST_BatchTexTGVertexMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGVertexMul,
            FragShaderDesc::FSI_PS3_FBatchTexTGVertexMul
        }
    },
    { // ST_InstancedTexTGVertex
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGVertex,
            FragShaderDesc::FSI_PS3_FInstancedTexTGVertex
        }
    },
    { // ST_InstancedTexTGVertexMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGVertexMul,
            FragShaderDesc::FSI_PS3_FInstancedTexTGVertexMul
        }
    },
    { // ST_Position3dTexTGVertex
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGVertex,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGVertex
        }
    },
    { // ST_Position3dTexTGVertexMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGVertexMul,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGVertexMul
        }
    },
    { // ST_BatchPosition3dTexTGVertex
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGVertex,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGVertex
        }
    },
    { // ST_BatchPosition3dTexTGVertexMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGVertexMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGVertexMul
        }
    },
    { // ST_InstancedPosition3dTexTGVertex
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGVertex,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGVertex
        }
    },
    { // ST_InstancedPosition3dTexTGVertexMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGVertexMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGVertexMul
        }
    },
    { // ST_TexTGVertexCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGVertexCxform,
            FragShaderDesc::FSI_PS3_FTexTGVertexCxform
        }
    },
    { // ST_TexTGVertexCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGVertexCxformMul,
            FragShaderDesc::FSI_PS3_FTexTGVertexCxformMul
        }
    },
    { // ST_BatchTexTGVertexCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGVertexCxform,
            FragShaderDesc::FSI_PS3_FBatchTexTGVertexCxform
        }
    },
    { // ST_BatchTexTGVertexCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGVertexCxformMul,
            FragShaderDesc::FSI_PS3_FBatchTexTGVertexCxformMul
        }
    },
    { // ST_InstancedTexTGVertexCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGVertexCxform,
            FragShaderDesc::FSI_PS3_FInstancedTexTGVertexCxform
        }
    },
    { // ST_InstancedTexTGVertexCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGVertexCxformMul,
            FragShaderDesc::FSI_PS3_FInstancedTexTGVertexCxformMul
        }
    },
    { // ST_Position3dTexTGVertexCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGVertexCxform,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGVertexCxform
        }
    },
    { // ST_Position3dTexTGVertexCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGVertexCxformMul,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGVertexCxformMul
        }
    },
    { // ST_BatchPosition3dTexTGVertexCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGVertexCxform,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGVertexCxform
        }
    },
    { // ST_BatchPosition3dTexTGVertexCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGVertexCxformMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGVertexCxformMul
        }
    },
    { // ST_InstancedPosition3dTexTGVertexCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGVertexCxform,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGVertexCxform
        }
    },
    { // ST_InstancedPosition3dTexTGVertexCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGVertexCxformMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGVertexCxformMul
        }
    },
    { // ST_TexTGVertexEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGVertexEAlpha,
            FragShaderDesc::FSI_PS3_FTexTGVertexEAlpha
        }
    },
    { // ST_TexTGVertexEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGVertexEAlphaMul,
            FragShaderDesc::FSI_PS3_FTexTGVertexEAlphaMul
        }
    },
    { // ST_BatchTexTGVertexEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGVertexEAlpha,
            FragShaderDesc::FSI_PS3_FBatchTexTGVertexEAlpha
        }
    },
    { // ST_BatchTexTGVertexEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGVertexEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchTexTGVertexEAlphaMul
        }
    },
    { // ST_InstancedTexTGVertexEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGVertexEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedTexTGVertexEAlpha
        }
    },
    { // ST_InstancedTexTGVertexEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGVertexEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedTexTGVertexEAlphaMul
        }
    },
    { // ST_Position3dTexTGVertexEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGVertexEAlpha,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGVertexEAlpha
        }
    },
    { // ST_Position3dTexTGVertexEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGVertexEAlphaMul,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGVertexEAlphaMul
        }
    },
    { // ST_BatchPosition3dTexTGVertexEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGVertexEAlpha,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGVertexEAlpha
        }
    },
    { // ST_BatchPosition3dTexTGVertexEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGVertexEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGVertexEAlphaMul
        }
    },
    { // ST_InstancedPosition3dTexTGVertexEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGVertexEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGVertexEAlpha
        }
    },
    { // ST_InstancedPosition3dTexTGVertexEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGVertexEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGVertexEAlphaMul
        }
    },
    { // ST_TexTGVertexCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGVertexCxformEAlpha,
            FragShaderDesc::FSI_PS3_FTexTGVertexCxformEAlpha
        }
    },
    { // ST_TexTGVertexCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGVertexCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FTexTGVertexCxformEAlphaMul
        }
    },
    { // ST_BatchTexTGVertexCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGVertexCxformEAlpha,
            FragShaderDesc::FSI_PS3_FBatchTexTGVertexCxformEAlpha
        }
    },
    { // ST_BatchTexTGVertexCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGVertexCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchTexTGVertexCxformEAlphaMul
        }
    },
    { // ST_InstancedTexTGVertexCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGVertexCxformEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedTexTGVertexCxformEAlpha
        }
    },
    { // ST_InstancedTexTGVertexCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGVertexCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedTexTGVertexCxformEAlphaMul
        }
    },
    { // ST_Position3dTexTGVertexCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGVertexCxformEAlpha,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGVertexCxformEAlpha
        }
    },
    { // ST_Position3dTexTGVertexCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGVertexCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGVertexCxformEAlphaMul
        }
    },
    { // ST_BatchPosition3dTexTGVertexCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGVertexCxformEAlpha,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGVertexCxformEAlpha
        }
    },
    { // ST_BatchPosition3dTexTGVertexCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGVertexCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGVertexCxformEAlphaMul
        }
    },
    { // ST_InstancedPosition3dTexTGVertexCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGVertexCxformEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGVertexCxformEAlpha
        }
    },
    { // ST_InstancedPosition3dTexTGVertexCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGVertexCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGVertexCxformEAlphaMul
        }
    },
    { // ST_TexTGVertexInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGVertexInv,
            FragShaderDesc::FSI_PS3_FTexTGVertexInv
        }
    },
    { // ST_BatchTexTGVertexInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGVertexInv,
            FragShaderDesc::FSI_PS3_FBatchTexTGVertexInv
        }
    },
    { // ST_InstancedTexTGVertexInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGVertexInv,
            FragShaderDesc::FSI_PS3_FInstancedTexTGVertexInv
        }
    },
    { // ST_Position3dTexTGVertexInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGVertexInv,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGVertexInv
        }
    },
    { // ST_BatchPosition3dTexTGVertexInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGVertexInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGVertexInv
        }
    },
    { // ST_InstancedPosition3dTexTGVertexInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGVertexInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGVertexInv
        }
    },
    { // ST_TexTGVertexCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGVertexCxformInv,
            FragShaderDesc::FSI_PS3_FTexTGVertexCxformInv
        }
    },
    { // ST_BatchTexTGVertexCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGVertexCxformInv,
            FragShaderDesc::FSI_PS3_FBatchTexTGVertexCxformInv
        }
    },
    { // ST_InstancedTexTGVertexCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGVertexCxformInv,
            FragShaderDesc::FSI_PS3_FInstancedTexTGVertexCxformInv
        }
    },
    { // ST_Position3dTexTGVertexCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGVertexCxformInv,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGVertexCxformInv
        }
    },
    { // ST_BatchPosition3dTexTGVertexCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGVertexCxformInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGVertexCxformInv
        }
    },
    { // ST_InstancedPosition3dTexTGVertexCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGVertexCxformInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGVertexCxformInv
        }
    },
    { // ST_TexTGVertexEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGVertexEAlphaInv,
            FragShaderDesc::FSI_PS3_FTexTGVertexEAlphaInv
        }
    },
    { // ST_BatchTexTGVertexEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGVertexEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchTexTGVertexEAlphaInv
        }
    },
    { // ST_InstancedTexTGVertexEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGVertexEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedTexTGVertexEAlphaInv
        }
    },
    { // ST_Position3dTexTGVertexEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGVertexEAlphaInv,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGVertexEAlphaInv
        }
    },
    { // ST_BatchPosition3dTexTGVertexEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGVertexEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGVertexEAlphaInv
        }
    },
    { // ST_InstancedPosition3dTexTGVertexEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGVertexEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGVertexEAlphaInv
        }
    },
    { // ST_TexTGVertexCxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGVertexCxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FTexTGVertexCxformEAlphaInv
        }
    },
    { // ST_BatchTexTGVertexCxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGVertexCxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchTexTGVertexCxformEAlphaInv
        }
    },
    { // ST_InstancedTexTGVertexCxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGVertexCxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedTexTGVertexCxformEAlphaInv
        }
    },
    { // ST_Position3dTexTGVertexCxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGVertexCxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGVertexCxformEAlphaInv
        }
    },
    { // ST_BatchPosition3dTexTGVertexCxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGVertexCxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGVertexCxformEAlphaInv
        }
    },
    { // ST_InstancedPosition3dTexTGVertexCxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGVertexCxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGVertexCxformEAlphaInv
        }
    },
    { // ST_TexTGVertexCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGVertexCxformAc,
            FragShaderDesc::FSI_PS3_FTexTGVertexCxformAc
        }
    },
    { // ST_TexTGVertexCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGVertexCxformAcMul,
            FragShaderDesc::FSI_PS3_FTexTGVertexCxformAcMul
        }
    },
    { // ST_BatchTexTGVertexCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGVertexCxformAc,
            FragShaderDesc::FSI_PS3_FBatchTexTGVertexCxformAc
        }
    },
    { // ST_BatchTexTGVertexCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGVertexCxformAcMul,
            FragShaderDesc::FSI_PS3_FBatchTexTGVertexCxformAcMul
        }
    },
    { // ST_InstancedTexTGVertexCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGVertexCxformAc,
            FragShaderDesc::FSI_PS3_FInstancedTexTGVertexCxformAc
        }
    },
    { // ST_InstancedTexTGVertexCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGVertexCxformAcMul,
            FragShaderDesc::FSI_PS3_FInstancedTexTGVertexCxformAcMul
        }
    },
    { // ST_Position3dTexTGVertexCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGVertexCxformAc,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGVertexCxformAc
        }
    },
    { // ST_Position3dTexTGVertexCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGVertexCxformAcMul,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGVertexCxformAcMul
        }
    },
    { // ST_BatchPosition3dTexTGVertexCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGVertexCxformAc,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGVertexCxformAc
        }
    },
    { // ST_BatchPosition3dTexTGVertexCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGVertexCxformAcMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGVertexCxformAcMul
        }
    },
    { // ST_InstancedPosition3dTexTGVertexCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGVertexCxformAc,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGVertexCxformAc
        }
    },
    { // ST_InstancedPosition3dTexTGVertexCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGVertexCxformAcMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGVertexCxformAcMul
        }
    },
    { // ST_TexTGVertexCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGVertexCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FTexTGVertexCxformAcEAlpha
        }
    },
    { // ST_TexTGVertexCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FTexTGVertexCxformAcEAlphaMul
        }
    },
    { // ST_BatchTexTGVertexCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGVertexCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FBatchTexTGVertexCxformAcEAlpha
        }
    },
    { // ST_BatchTexTGVertexCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchTexTGVertexCxformAcEAlphaMul
        }
    },
    { // ST_InstancedTexTGVertexCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGVertexCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedTexTGVertexCxformAcEAlpha
        }
    },
    { // ST_InstancedTexTGVertexCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedTexTGVertexCxformAcEAlphaMul
        }
    },
    { // ST_Position3dTexTGVertexCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGVertexCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGVertexCxformAcEAlpha
        }
    },
    { // ST_Position3dTexTGVertexCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGVertexCxformAcEAlphaMul
        }
    },
    { // ST_BatchPosition3dTexTGVertexCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGVertexCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGVertexCxformAcEAlpha
        }
    },
    { // ST_BatchPosition3dTexTGVertexCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGVertexCxformAcEAlphaMul
        }
    },
    { // ST_InstancedPosition3dTexTGVertexCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGVertexCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGVertexCxformAcEAlpha
        }
    },
    { // ST_InstancedPosition3dTexTGVertexCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGVertexCxformAcEAlphaMul
        }
    },
    { // ST_TexTGVertexCxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGVertexCxformAcInv,
            FragShaderDesc::FSI_PS3_FTexTGVertexCxformAcInv
        }
    },
    { // ST_BatchTexTGVertexCxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGVertexCxformAcInv,
            FragShaderDesc::FSI_PS3_FBatchTexTGVertexCxformAcInv
        }
    },
    { // ST_InstancedTexTGVertexCxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGVertexCxformAcInv,
            FragShaderDesc::FSI_PS3_FInstancedTexTGVertexCxformAcInv
        }
    },
    { // ST_Position3dTexTGVertexCxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGVertexCxformAcInv,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGVertexCxformAcInv
        }
    },
    { // ST_BatchPosition3dTexTGVertexCxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGVertexCxformAcInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGVertexCxformAcInv
        }
    },
    { // ST_InstancedPosition3dTexTGVertexCxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGVertexCxformAcInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGVertexCxformAcInv
        }
    },
    { // ST_TexTGVertexCxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FTexTGVertexCxformAcEAlphaInv
        }
    },
    { // ST_BatchTexTGVertexCxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTexTGVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchTexTGVertexCxformAcEAlphaInv
        }
    },
    { // ST_InstancedTexTGVertexCxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTexTGVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedTexTGVertexCxformAcEAlphaInv
        }
    },
    { // ST_Position3dTexTGVertexCxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTexTGVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FPosition3dTexTGVertexCxformAcEAlphaInv
        }
    },
    { // ST_BatchPosition3dTexTGVertexCxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTexTGVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTexTGVertexCxformAcEAlphaInv
        }
    },
    { // ST_InstancedPosition3dTexTGVertexCxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTexTGVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTexTGVertexCxformAcEAlphaInv
        }
    },
    { // ST_Solid
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VSolid,
            FragShaderDesc::FSI_PS3_FSolid
        }
    },
    { // ST_SolidMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VSolidMul,
            FragShaderDesc::FSI_PS3_FSolidMul
        }
    },
    { // ST_BatchSolid
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchSolid,
            FragShaderDesc::FSI_PS3_FBatchSolid
        }
    },
    { // ST_BatchSolidMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchSolidMul,
            FragShaderDesc::FSI_PS3_FBatchSolidMul
        }
    },
    { // ST_InstancedSolid
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedSolid,
            FragShaderDesc::FSI_PS3_FInstancedSolid
        }
    },
    { // ST_InstancedSolidMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedSolidMul,
            FragShaderDesc::FSI_PS3_FInstancedSolidMul
        }
    },
    { // ST_Position3dSolid
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dSolid,
            FragShaderDesc::FSI_PS3_FPosition3dSolid
        }
    },
    { // ST_Position3dSolidMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dSolidMul,
            FragShaderDesc::FSI_PS3_FPosition3dSolidMul
        }
    },
    { // ST_BatchPosition3dSolid
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dSolid,
            FragShaderDesc::FSI_PS3_FBatchPosition3dSolid
        }
    },
    { // ST_BatchPosition3dSolidMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dSolidMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dSolidMul
        }
    },
    { // ST_InstancedPosition3dSolid
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dSolid,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dSolid
        }
    },
    { // ST_InstancedPosition3dSolidMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dSolidMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dSolidMul
        }
    },
    { // ST_SolidInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VSolidInv,
            FragShaderDesc::FSI_PS3_FSolidInv
        }
    },
    { // ST_BatchSolidInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchSolidInv,
            FragShaderDesc::FSI_PS3_FBatchSolidInv
        }
    },
    { // ST_InstancedSolidInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedSolidInv,
            FragShaderDesc::FSI_PS3_FInstancedSolidInv
        }
    },
    { // ST_Position3dSolidInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dSolidInv,
            FragShaderDesc::FSI_PS3_FPosition3dSolidInv
        }
    },
    { // ST_BatchPosition3dSolidInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dSolidInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dSolidInv
        }
    },
    { // ST_InstancedPosition3dSolidInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dSolidInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dSolidInv
        }
    },
    { // ST_Text
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VText,
            FragShaderDesc::FSI_PS3_FText
        }
    },
    { // ST_TextMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTextMul,
            FragShaderDesc::FSI_PS3_FTextMul
        }
    },
    { // ST_BatchText
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchText,
            FragShaderDesc::FSI_PS3_FBatchText
        }
    },
    { // ST_BatchTextMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTextMul,
            FragShaderDesc::FSI_PS3_FBatchTextMul
        }
    },
    { // ST_InstancedText
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedText,
            FragShaderDesc::FSI_PS3_FInstancedText
        }
    },
    { // ST_InstancedTextMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTextMul,
            FragShaderDesc::FSI_PS3_FInstancedTextMul
        }
    },
    { // ST_Position3dText
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dText,
            FragShaderDesc::FSI_PS3_FPosition3dText
        }
    },
    { // ST_Position3dTextMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTextMul,
            FragShaderDesc::FSI_PS3_FPosition3dTextMul
        }
    },
    { // ST_BatchPosition3dText
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dText,
            FragShaderDesc::FSI_PS3_FBatchPosition3dText
        }
    },
    { // ST_BatchPosition3dTextMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTextMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTextMul
        }
    },
    { // ST_InstancedPosition3dText
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dText,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dText
        }
    },
    { // ST_InstancedPosition3dTextMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTextMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTextMul
        }
    },
    { // ST_TextInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTextInv,
            FragShaderDesc::FSI_PS3_FTextInv
        }
    },
    { // ST_BatchTextInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTextInv,
            FragShaderDesc::FSI_PS3_FBatchTextInv
        }
    },
    { // ST_InstancedTextInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTextInv,
            FragShaderDesc::FSI_PS3_FInstancedTextInv
        }
    },
    { // ST_Position3dTextInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTextInv,
            FragShaderDesc::FSI_PS3_FPosition3dTextInv
        }
    },
    { // ST_BatchPosition3dTextInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTextInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTextInv
        }
    },
    { // ST_InstancedPosition3dTextInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTextInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTextInv
        }
    },
    { // ST_TextColor
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTextColor,
            FragShaderDesc::FSI_PS3_FTextColor
        }
    },
    { // ST_TextColorMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTextColorMul,
            FragShaderDesc::FSI_PS3_FTextColorMul
        }
    },
    { // ST_BatchTextColor
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTextColor,
            FragShaderDesc::FSI_PS3_FBatchTextColor
        }
    },
    { // ST_BatchTextColorMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTextColorMul,
            FragShaderDesc::FSI_PS3_FBatchTextColorMul
        }
    },
    { // ST_InstancedTextColor
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTextColor,
            FragShaderDesc::FSI_PS3_FInstancedTextColor
        }
    },
    { // ST_InstancedTextColorMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTextColorMul,
            FragShaderDesc::FSI_PS3_FInstancedTextColorMul
        }
    },
    { // ST_Position3dTextColor
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTextColor,
            FragShaderDesc::FSI_PS3_FPosition3dTextColor
        }
    },
    { // ST_Position3dTextColorMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTextColorMul,
            FragShaderDesc::FSI_PS3_FPosition3dTextColorMul
        }
    },
    { // ST_BatchPosition3dTextColor
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTextColor,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTextColor
        }
    },
    { // ST_BatchPosition3dTextColorMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTextColorMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTextColorMul
        }
    },
    { // ST_InstancedPosition3dTextColor
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTextColor,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTextColor
        }
    },
    { // ST_InstancedPosition3dTextColorMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTextColorMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTextColorMul
        }
    },
    { // ST_TextColorCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTextColorCxform,
            FragShaderDesc::FSI_PS3_FTextColorCxform
        }
    },
    { // ST_TextColorCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTextColorCxformMul,
            FragShaderDesc::FSI_PS3_FTextColorCxformMul
        }
    },
    { // ST_BatchTextColorCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTextColorCxform,
            FragShaderDesc::FSI_PS3_FBatchTextColorCxform
        }
    },
    { // ST_BatchTextColorCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTextColorCxformMul,
            FragShaderDesc::FSI_PS3_FBatchTextColorCxformMul
        }
    },
    { // ST_InstancedTextColorCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTextColorCxform,
            FragShaderDesc::FSI_PS3_FInstancedTextColorCxform
        }
    },
    { // ST_InstancedTextColorCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTextColorCxformMul,
            FragShaderDesc::FSI_PS3_FInstancedTextColorCxformMul
        }
    },
    { // ST_Position3dTextColorCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTextColorCxform,
            FragShaderDesc::FSI_PS3_FPosition3dTextColorCxform
        }
    },
    { // ST_Position3dTextColorCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTextColorCxformMul,
            FragShaderDesc::FSI_PS3_FPosition3dTextColorCxformMul
        }
    },
    { // ST_BatchPosition3dTextColorCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTextColorCxform,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTextColorCxform
        }
    },
    { // ST_BatchPosition3dTextColorCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTextColorCxformMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTextColorCxformMul
        }
    },
    { // ST_InstancedPosition3dTextColorCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTextColorCxform,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTextColorCxform
        }
    },
    { // ST_InstancedPosition3dTextColorCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTextColorCxformMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTextColorCxformMul
        }
    },
    { // ST_TextColorInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTextColorInv,
            FragShaderDesc::FSI_PS3_FTextColorInv
        }
    },
    { // ST_BatchTextColorInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTextColorInv,
            FragShaderDesc::FSI_PS3_FBatchTextColorInv
        }
    },
    { // ST_InstancedTextColorInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTextColorInv,
            FragShaderDesc::FSI_PS3_FInstancedTextColorInv
        }
    },
    { // ST_Position3dTextColorInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTextColorInv,
            FragShaderDesc::FSI_PS3_FPosition3dTextColorInv
        }
    },
    { // ST_BatchPosition3dTextColorInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTextColorInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTextColorInv
        }
    },
    { // ST_InstancedPosition3dTextColorInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTextColorInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTextColorInv
        }
    },
    { // ST_TextColorCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTextColorCxformInv,
            FragShaderDesc::FSI_PS3_FTextColorCxformInv
        }
    },
    { // ST_BatchTextColorCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchTextColorCxformInv,
            FragShaderDesc::FSI_PS3_FBatchTextColorCxformInv
        }
    },
    { // ST_InstancedTextColorCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedTextColorCxformInv,
            FragShaderDesc::FSI_PS3_FInstancedTextColorCxformInv
        }
    },
    { // ST_Position3dTextColorCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dTextColorCxformInv,
            FragShaderDesc::FSI_PS3_FPosition3dTextColorCxformInv
        }
    },
    { // ST_BatchPosition3dTextColorCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dTextColorCxformInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dTextColorCxformInv
        }
    },
    { // ST_InstancedPosition3dTextColorCxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dTextColorCxformInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dTextColorCxformInv
        }
    },
    { // ST_YUV
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUV,
            FragShaderDesc::FSI_PS3_FYUV
        }
    },
    { // ST_YUVMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVMul,
            FragShaderDesc::FSI_PS3_FYUVMul
        }
    },
    { // ST_BatchYUV
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUV,
            FragShaderDesc::FSI_PS3_FBatchYUV
        }
    },
    { // ST_BatchYUVMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVMul,
            FragShaderDesc::FSI_PS3_FBatchYUVMul
        }
    },
    { // ST_InstancedYUV
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUV,
            FragShaderDesc::FSI_PS3_FInstancedYUV
        }
    },
    { // ST_InstancedYUVMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVMul,
            FragShaderDesc::FSI_PS3_FInstancedYUVMul
        }
    },
    { // ST_Position3dYUV
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUV,
            FragShaderDesc::FSI_PS3_FPosition3dYUV
        }
    },
    { // ST_Position3dYUVMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVMul,
            FragShaderDesc::FSI_PS3_FPosition3dYUVMul
        }
    },
    { // ST_BatchPosition3dYUV
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUV,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUV
        }
    },
    { // ST_BatchPosition3dYUVMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVMul
        }
    },
    { // ST_InstancedPosition3dYUV
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUV,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUV
        }
    },
    { // ST_InstancedPosition3dYUVMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVMul
        }
    },
    { // ST_YUVCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVCxform,
            FragShaderDesc::FSI_PS3_FYUVCxform
        }
    },
    { // ST_YUVCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVCxformMul,
            FragShaderDesc::FSI_PS3_FYUVCxformMul
        }
    },
    { // ST_BatchYUVCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVCxform,
            FragShaderDesc::FSI_PS3_FBatchYUVCxform
        }
    },
    { // ST_BatchYUVCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVCxformMul,
            FragShaderDesc::FSI_PS3_FBatchYUVCxformMul
        }
    },
    { // ST_InstancedYUVCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVCxform,
            FragShaderDesc::FSI_PS3_FInstancedYUVCxform
        }
    },
    { // ST_InstancedYUVCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVCxformMul,
            FragShaderDesc::FSI_PS3_FInstancedYUVCxformMul
        }
    },
    { // ST_Position3dYUVCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVCxform,
            FragShaderDesc::FSI_PS3_FPosition3dYUVCxform
        }
    },
    { // ST_Position3dYUVCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVCxformMul,
            FragShaderDesc::FSI_PS3_FPosition3dYUVCxformMul
        }
    },
    { // ST_BatchPosition3dYUVCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVCxform,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVCxform
        }
    },
    { // ST_BatchPosition3dYUVCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVCxformMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVCxformMul
        }
    },
    { // ST_InstancedPosition3dYUVCxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVCxform,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVCxform
        }
    },
    { // ST_InstancedPosition3dYUVCxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVCxformMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVCxformMul
        }
    },
    { // ST_YUVEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVEAlpha,
            FragShaderDesc::FSI_PS3_FYUVEAlpha
        }
    },
    { // ST_YUVEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVEAlphaMul,
            FragShaderDesc::FSI_PS3_FYUVEAlphaMul
        }
    },
    { // ST_BatchYUVEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVEAlpha,
            FragShaderDesc::FSI_PS3_FBatchYUVEAlpha
        }
    },
    { // ST_BatchYUVEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchYUVEAlphaMul
        }
    },
    { // ST_InstancedYUVEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedYUVEAlpha
        }
    },
    { // ST_InstancedYUVEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedYUVEAlphaMul
        }
    },
    { // ST_Position3dYUVEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVEAlpha,
            FragShaderDesc::FSI_PS3_FPosition3dYUVEAlpha
        }
    },
    { // ST_Position3dYUVEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVEAlphaMul,
            FragShaderDesc::FSI_PS3_FPosition3dYUVEAlphaMul
        }
    },
    { // ST_BatchPosition3dYUVEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVEAlpha,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVEAlpha
        }
    },
    { // ST_BatchPosition3dYUVEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVEAlphaMul
        }
    },
    { // ST_InstancedPosition3dYUVEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVEAlpha
        }
    },
    { // ST_InstancedPosition3dYUVEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVEAlphaMul
        }
    },
    { // ST_YUVCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVCxformEAlpha,
            FragShaderDesc::FSI_PS3_FYUVCxformEAlpha
        }
    },
    { // ST_YUVCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FYUVCxformEAlphaMul
        }
    },
    { // ST_BatchYUVCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVCxformEAlpha,
            FragShaderDesc::FSI_PS3_FBatchYUVCxformEAlpha
        }
    },
    { // ST_BatchYUVCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchYUVCxformEAlphaMul
        }
    },
    { // ST_InstancedYUVCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVCxformEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedYUVCxformEAlpha
        }
    },
    { // ST_InstancedYUVCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedYUVCxformEAlphaMul
        }
    },
    { // ST_Position3dYUVCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVCxformEAlpha,
            FragShaderDesc::FSI_PS3_FPosition3dYUVCxformEAlpha
        }
    },
    { // ST_Position3dYUVCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FPosition3dYUVCxformEAlphaMul
        }
    },
    { // ST_BatchPosition3dYUVCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVCxformEAlpha,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVCxformEAlpha
        }
    },
    { // ST_BatchPosition3dYUVCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVCxformEAlphaMul
        }
    },
    { // ST_InstancedPosition3dYUVCxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVCxformEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVCxformEAlpha
        }
    },
    { // ST_InstancedPosition3dYUVCxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVCxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVCxformEAlphaMul
        }
    },
    { // ST_YUVCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVCxformAc,
            FragShaderDesc::FSI_PS3_FYUVCxformAc
        }
    },
    { // ST_YUVCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVCxformAcMul,
            FragShaderDesc::FSI_PS3_FYUVCxformAcMul
        }
    },
    { // ST_BatchYUVCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVCxformAc,
            FragShaderDesc::FSI_PS3_FBatchYUVCxformAc
        }
    },
    { // ST_BatchYUVCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVCxformAcMul,
            FragShaderDesc::FSI_PS3_FBatchYUVCxformAcMul
        }
    },
    { // ST_InstancedYUVCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVCxformAc,
            FragShaderDesc::FSI_PS3_FInstancedYUVCxformAc
        }
    },
    { // ST_InstancedYUVCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVCxformAcMul,
            FragShaderDesc::FSI_PS3_FInstancedYUVCxformAcMul
        }
    },
    { // ST_Position3dYUVCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVCxformAc,
            FragShaderDesc::FSI_PS3_FPosition3dYUVCxformAc
        }
    },
    { // ST_Position3dYUVCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVCxformAcMul,
            FragShaderDesc::FSI_PS3_FPosition3dYUVCxformAcMul
        }
    },
    { // ST_BatchPosition3dYUVCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVCxformAc,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVCxformAc
        }
    },
    { // ST_BatchPosition3dYUVCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVCxformAcMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVCxformAcMul
        }
    },
    { // ST_InstancedPosition3dYUVCxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVCxformAc,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVCxformAc
        }
    },
    { // ST_InstancedPosition3dYUVCxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVCxformAcMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVCxformAcMul
        }
    },
    { // ST_YUVCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FYUVCxformAcEAlpha
        }
    },
    { // ST_YUVCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FYUVCxformAcEAlphaMul
        }
    },
    { // ST_BatchYUVCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FBatchYUVCxformAcEAlpha
        }
    },
    { // ST_BatchYUVCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchYUVCxformAcEAlphaMul
        }
    },
    { // ST_InstancedYUVCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedYUVCxformAcEAlpha
        }
    },
    { // ST_InstancedYUVCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedYUVCxformAcEAlphaMul
        }
    },
    { // ST_Position3dYUVCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FPosition3dYUVCxformAcEAlpha
        }
    },
    { // ST_Position3dYUVCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FPosition3dYUVCxformAcEAlphaMul
        }
    },
    { // ST_BatchPosition3dYUVCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVCxformAcEAlpha
        }
    },
    { // ST_BatchPosition3dYUVCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVCxformAcEAlphaMul
        }
    },
    { // ST_InstancedPosition3dYUVCxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVCxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVCxformAcEAlpha
        }
    },
    { // ST_InstancedPosition3dYUVCxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVCxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVCxformAcEAlphaMul
        }
    },
    { // ST_YUVA
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVA,
            FragShaderDesc::FSI_PS3_FYUVA
        }
    },
    { // ST_YUVAMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVAMul,
            FragShaderDesc::FSI_PS3_FYUVAMul
        }
    },
    { // ST_BatchYUVA
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVA,
            FragShaderDesc::FSI_PS3_FBatchYUVA
        }
    },
    { // ST_BatchYUVAMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVAMul,
            FragShaderDesc::FSI_PS3_FBatchYUVAMul
        }
    },
    { // ST_InstancedYUVA
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVA,
            FragShaderDesc::FSI_PS3_FInstancedYUVA
        }
    },
    { // ST_InstancedYUVAMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVAMul,
            FragShaderDesc::FSI_PS3_FInstancedYUVAMul
        }
    },
    { // ST_Position3dYUVA
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVA,
            FragShaderDesc::FSI_PS3_FPosition3dYUVA
        }
    },
    { // ST_Position3dYUVAMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVAMul,
            FragShaderDesc::FSI_PS3_FPosition3dYUVAMul
        }
    },
    { // ST_BatchPosition3dYUVA
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVA,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVA
        }
    },
    { // ST_BatchPosition3dYUVAMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVAMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVAMul
        }
    },
    { // ST_InstancedPosition3dYUVA
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVA,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVA
        }
    },
    { // ST_InstancedPosition3dYUVAMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVAMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVAMul
        }
    },
    { // ST_YUVACxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVACxform,
            FragShaderDesc::FSI_PS3_FYUVACxform
        }
    },
    { // ST_YUVACxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVACxformMul,
            FragShaderDesc::FSI_PS3_FYUVACxformMul
        }
    },
    { // ST_BatchYUVACxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVACxform,
            FragShaderDesc::FSI_PS3_FBatchYUVACxform
        }
    },
    { // ST_BatchYUVACxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVACxformMul,
            FragShaderDesc::FSI_PS3_FBatchYUVACxformMul
        }
    },
    { // ST_InstancedYUVACxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVACxform,
            FragShaderDesc::FSI_PS3_FInstancedYUVACxform
        }
    },
    { // ST_InstancedYUVACxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVACxformMul,
            FragShaderDesc::FSI_PS3_FInstancedYUVACxformMul
        }
    },
    { // ST_Position3dYUVACxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVACxform,
            FragShaderDesc::FSI_PS3_FPosition3dYUVACxform
        }
    },
    { // ST_Position3dYUVACxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVACxformMul,
            FragShaderDesc::FSI_PS3_FPosition3dYUVACxformMul
        }
    },
    { // ST_BatchPosition3dYUVACxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVACxform,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVACxform
        }
    },
    { // ST_BatchPosition3dYUVACxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVACxformMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVACxformMul
        }
    },
    { // ST_InstancedPosition3dYUVACxform
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVACxform,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVACxform
        }
    },
    { // ST_InstancedPosition3dYUVACxformMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVACxformMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVACxformMul
        }
    },
    { // ST_YUVAEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVAEAlpha,
            FragShaderDesc::FSI_PS3_FYUVAEAlpha
        }
    },
    { // ST_YUVAEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVAEAlphaMul,
            FragShaderDesc::FSI_PS3_FYUVAEAlphaMul
        }
    },
    { // ST_BatchYUVAEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVAEAlpha,
            FragShaderDesc::FSI_PS3_FBatchYUVAEAlpha
        }
    },
    { // ST_BatchYUVAEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVAEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchYUVAEAlphaMul
        }
    },
    { // ST_InstancedYUVAEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVAEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedYUVAEAlpha
        }
    },
    { // ST_InstancedYUVAEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVAEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedYUVAEAlphaMul
        }
    },
    { // ST_Position3dYUVAEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVAEAlpha,
            FragShaderDesc::FSI_PS3_FPosition3dYUVAEAlpha
        }
    },
    { // ST_Position3dYUVAEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVAEAlphaMul,
            FragShaderDesc::FSI_PS3_FPosition3dYUVAEAlphaMul
        }
    },
    { // ST_BatchPosition3dYUVAEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVAEAlpha,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVAEAlpha
        }
    },
    { // ST_BatchPosition3dYUVAEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVAEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVAEAlphaMul
        }
    },
    { // ST_InstancedPosition3dYUVAEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVAEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVAEAlpha
        }
    },
    { // ST_InstancedPosition3dYUVAEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVAEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVAEAlphaMul
        }
    },
    { // ST_YUVACxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVACxformEAlpha,
            FragShaderDesc::FSI_PS3_FYUVACxformEAlpha
        }
    },
    { // ST_YUVACxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVACxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FYUVACxformEAlphaMul
        }
    },
    { // ST_BatchYUVACxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVACxformEAlpha,
            FragShaderDesc::FSI_PS3_FBatchYUVACxformEAlpha
        }
    },
    { // ST_BatchYUVACxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVACxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchYUVACxformEAlphaMul
        }
    },
    { // ST_InstancedYUVACxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVACxformEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedYUVACxformEAlpha
        }
    },
    { // ST_InstancedYUVACxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVACxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedYUVACxformEAlphaMul
        }
    },
    { // ST_Position3dYUVACxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVACxformEAlpha,
            FragShaderDesc::FSI_PS3_FPosition3dYUVACxformEAlpha
        }
    },
    { // ST_Position3dYUVACxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVACxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FPosition3dYUVACxformEAlphaMul
        }
    },
    { // ST_BatchPosition3dYUVACxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVACxformEAlpha,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVACxformEAlpha
        }
    },
    { // ST_BatchPosition3dYUVACxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVACxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVACxformEAlphaMul
        }
    },
    { // ST_InstancedPosition3dYUVACxformEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVACxformEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVACxformEAlpha
        }
    },
    { // ST_InstancedPosition3dYUVACxformEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVACxformEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVACxformEAlphaMul
        }
    },
    { // ST_YUVAInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVAInv,
            FragShaderDesc::FSI_PS3_FYUVAInv
        }
    },
    { // ST_BatchYUVAInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVAInv,
            FragShaderDesc::FSI_PS3_FBatchYUVAInv
        }
    },
    { // ST_InstancedYUVAInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVAInv,
            FragShaderDesc::FSI_PS3_FInstancedYUVAInv
        }
    },
    { // ST_Position3dYUVAInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVAInv,
            FragShaderDesc::FSI_PS3_FPosition3dYUVAInv
        }
    },
    { // ST_BatchPosition3dYUVAInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVAInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVAInv
        }
    },
    { // ST_InstancedPosition3dYUVAInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVAInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVAInv
        }
    },
    { // ST_YUVACxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVACxformInv,
            FragShaderDesc::FSI_PS3_FYUVACxformInv
        }
    },
    { // ST_BatchYUVACxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVACxformInv,
            FragShaderDesc::FSI_PS3_FBatchYUVACxformInv
        }
    },
    { // ST_InstancedYUVACxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVACxformInv,
            FragShaderDesc::FSI_PS3_FInstancedYUVACxformInv
        }
    },
    { // ST_Position3dYUVACxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVACxformInv,
            FragShaderDesc::FSI_PS3_FPosition3dYUVACxformInv
        }
    },
    { // ST_BatchPosition3dYUVACxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVACxformInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVACxformInv
        }
    },
    { // ST_InstancedPosition3dYUVACxformInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVACxformInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVACxformInv
        }
    },
    { // ST_YUVAEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVAEAlphaInv,
            FragShaderDesc::FSI_PS3_FYUVAEAlphaInv
        }
    },
    { // ST_BatchYUVAEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVAEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchYUVAEAlphaInv
        }
    },
    { // ST_InstancedYUVAEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVAEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedYUVAEAlphaInv
        }
    },
    { // ST_Position3dYUVAEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVAEAlphaInv,
            FragShaderDesc::FSI_PS3_FPosition3dYUVAEAlphaInv
        }
    },
    { // ST_BatchPosition3dYUVAEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVAEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVAEAlphaInv
        }
    },
    { // ST_InstancedPosition3dYUVAEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVAEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVAEAlphaInv
        }
    },
    { // ST_YUVACxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVACxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FYUVACxformEAlphaInv
        }
    },
    { // ST_BatchYUVACxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVACxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchYUVACxformEAlphaInv
        }
    },
    { // ST_InstancedYUVACxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVACxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedYUVACxformEAlphaInv
        }
    },
    { // ST_Position3dYUVACxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVACxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FPosition3dYUVACxformEAlphaInv
        }
    },
    { // ST_BatchPosition3dYUVACxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVACxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVACxformEAlphaInv
        }
    },
    { // ST_InstancedPosition3dYUVACxformEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVACxformEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVACxformEAlphaInv
        }
    },
    { // ST_YUVACxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVACxformAc,
            FragShaderDesc::FSI_PS3_FYUVACxformAc
        }
    },
    { // ST_YUVACxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVACxformAcMul,
            FragShaderDesc::FSI_PS3_FYUVACxformAcMul
        }
    },
    { // ST_BatchYUVACxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVACxformAc,
            FragShaderDesc::FSI_PS3_FBatchYUVACxformAc
        }
    },
    { // ST_BatchYUVACxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVACxformAcMul,
            FragShaderDesc::FSI_PS3_FBatchYUVACxformAcMul
        }
    },
    { // ST_InstancedYUVACxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVACxformAc,
            FragShaderDesc::FSI_PS3_FInstancedYUVACxformAc
        }
    },
    { // ST_InstancedYUVACxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVACxformAcMul,
            FragShaderDesc::FSI_PS3_FInstancedYUVACxformAcMul
        }
    },
    { // ST_Position3dYUVACxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVACxformAc,
            FragShaderDesc::FSI_PS3_FPosition3dYUVACxformAc
        }
    },
    { // ST_Position3dYUVACxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVACxformAcMul,
            FragShaderDesc::FSI_PS3_FPosition3dYUVACxformAcMul
        }
    },
    { // ST_BatchPosition3dYUVACxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVACxformAc,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVACxformAc
        }
    },
    { // ST_BatchPosition3dYUVACxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVACxformAcMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVACxformAcMul
        }
    },
    { // ST_InstancedPosition3dYUVACxformAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVACxformAc,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVACxformAc
        }
    },
    { // ST_InstancedPosition3dYUVACxformAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVACxformAcMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVACxformAcMul
        }
    },
    { // ST_YUVACxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVACxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FYUVACxformAcEAlpha
        }
    },
    { // ST_YUVACxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVACxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FYUVACxformAcEAlphaMul
        }
    },
    { // ST_BatchYUVACxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVACxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FBatchYUVACxformAcEAlpha
        }
    },
    { // ST_BatchYUVACxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVACxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchYUVACxformAcEAlphaMul
        }
    },
    { // ST_InstancedYUVACxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVACxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedYUVACxformAcEAlpha
        }
    },
    { // ST_InstancedYUVACxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVACxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedYUVACxformAcEAlphaMul
        }
    },
    { // ST_Position3dYUVACxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVACxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FPosition3dYUVACxformAcEAlpha
        }
    },
    { // ST_Position3dYUVACxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVACxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FPosition3dYUVACxformAcEAlphaMul
        }
    },
    { // ST_BatchPosition3dYUVACxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVACxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVACxformAcEAlpha
        }
    },
    { // ST_BatchPosition3dYUVACxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVACxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVACxformAcEAlphaMul
        }
    },
    { // ST_InstancedPosition3dYUVACxformAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVACxformAcEAlpha,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVACxformAcEAlpha
        }
    },
    { // ST_InstancedPosition3dYUVACxformAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVACxformAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVACxformAcEAlphaMul
        }
    },
    { // ST_YUVACxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVACxformAcInv,
            FragShaderDesc::FSI_PS3_FYUVACxformAcInv
        }
    },
    { // ST_BatchYUVACxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVACxformAcInv,
            FragShaderDesc::FSI_PS3_FBatchYUVACxformAcInv
        }
    },
    { // ST_InstancedYUVACxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVACxformAcInv,
            FragShaderDesc::FSI_PS3_FInstancedYUVACxformAcInv
        }
    },
    { // ST_Position3dYUVACxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVACxformAcInv,
            FragShaderDesc::FSI_PS3_FPosition3dYUVACxformAcInv
        }
    },
    { // ST_BatchPosition3dYUVACxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVACxformAcInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVACxformAcInv
        }
    },
    { // ST_InstancedPosition3dYUVACxformAcInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVACxformAcInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVACxformAcInv
        }
    },
    { // ST_YUVACxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VYUVACxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FYUVACxformAcEAlphaInv
        }
    },
    { // ST_BatchYUVACxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchYUVACxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchYUVACxformAcEAlphaInv
        }
    },
    { // ST_InstancedYUVACxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedYUVACxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedYUVACxformAcEAlphaInv
        }
    },
    { // ST_Position3dYUVACxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VPosition3dYUVACxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FPosition3dYUVACxformAcEAlphaInv
        }
    },
    { // ST_BatchPosition3dYUVACxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBatchPosition3dYUVACxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FBatchPosition3dYUVACxformAcEAlphaInv
        }
    },
    { // ST_InstancedPosition3dYUVACxformAcEAlphaInv
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VInstancedPosition3dYUVACxformAcEAlphaInv,
            FragShaderDesc::FSI_PS3_FInstancedPosition3dYUVACxformAcEAlphaInv
        }
    },
    { // ST_TexTGCMatrixAc
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGCMatrixAc,
            FragShaderDesc::FSI_PS3_FTexTGCMatrixAc
        }
    },
    { // ST_TexTGCMatrixAcMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGCMatrixAcMul,
            FragShaderDesc::FSI_PS3_FTexTGCMatrixAcMul
        }
    },
    { // ST_TexTGCMatrixAcEAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGCMatrixAcEAlpha,
            FragShaderDesc::FSI_PS3_FTexTGCMatrixAcEAlpha
        }
    },
    { // ST_TexTGCMatrixAcEAlphaMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VTexTGCMatrixAcEAlphaMul,
            FragShaderDesc::FSI_PS3_FTexTGCMatrixAcEAlphaMul
        }
    },
    { // ST_Box1Blur
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox1Blur,
            FragShaderDesc::FSI_PS3_FBox1Blur
        }
    },
    { // ST_Box1BlurMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox1BlurMul,
            FragShaderDesc::FSI_PS3_FBox1BlurMul
        }
    },
    { // ST_Box2Blur
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox2Blur,
            FragShaderDesc::FSI_PS3_FBox2Blur
        }
    },
    { // ST_Box2BlurMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox2BlurMul,
            FragShaderDesc::FSI_PS3_FBox2BlurMul
        }
    },
    { // ST_Box2Shadow
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox2Shadow,
            FragShaderDesc::FSI_PS3_FBox2Shadow
        }
    },
    { // ST_Box2ShadowMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox2ShadowMul,
            FragShaderDesc::FSI_PS3_FBox2ShadowMul
        }
    },
    { // ST_Box2ShadowKnockout
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox2ShadowKnockout,
            FragShaderDesc::FSI_PS3_FBox2ShadowKnockout
        }
    },
    { // ST_Box2ShadowKnockoutMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox2ShadowKnockoutMul,
            FragShaderDesc::FSI_PS3_FBox2ShadowKnockoutMul
        }
    },
    { // ST_Box2Shadowonly
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox2Shadowonly,
            FragShaderDesc::FSI_PS3_FBox2Shadowonly
        }
    },
    { // ST_Box2ShadowonlyMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox2ShadowonlyMul,
            FragShaderDesc::FSI_PS3_FBox2ShadowonlyMul
        }
    },
    { // ST_Box2InnerShadow
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox2InnerShadow,
            FragShaderDesc::FSI_PS3_FBox2InnerShadow
        }
    },
    { // ST_Box2InnerShadowMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox2InnerShadowMul,
            FragShaderDesc::FSI_PS3_FBox2InnerShadowMul
        }
    },
    { // ST_Box2InnerShadowKnockout
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox2InnerShadowKnockout,
            FragShaderDesc::FSI_PS3_FBox2InnerShadowKnockout
        }
    },
    { // ST_Box2InnerShadowKnockoutMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox2InnerShadowKnockoutMul,
            FragShaderDesc::FSI_PS3_FBox2InnerShadowKnockoutMul
        }
    },
    { // ST_Box2ShadowonlyHighlight
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox2ShadowonlyHighlight,
            FragShaderDesc::FSI_PS3_FBox2ShadowonlyHighlight
        }
    },
    { // ST_Box2ShadowonlyHighlightMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox2ShadowonlyHighlightMul,
            FragShaderDesc::FSI_PS3_FBox2ShadowonlyHighlightMul
        }
    },
    { // ST_Box2FullShadowHighlight
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox2FullShadowHighlight,
            FragShaderDesc::FSI_PS3_FBox2FullShadowHighlight
        }
    },
    { // ST_Box2FullShadowHighlightMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox2FullShadowHighlightMul,
            FragShaderDesc::FSI_PS3_FBox2FullShadowHighlightMul
        }
    },
    { // ST_Box2InnerShadowHighlight
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox2InnerShadowHighlight,
            FragShaderDesc::FSI_PS3_FBox2InnerShadowHighlight
        }
    },
    { // ST_Box2InnerShadowHighlightMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox2InnerShadowHighlightMul,
            FragShaderDesc::FSI_PS3_FBox2InnerShadowHighlightMul
        }
    },
    { // ST_Box2InnerShadowHighlightKnockout
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox2InnerShadowHighlightKnockout,
            FragShaderDesc::FSI_PS3_FBox2InnerShadowHighlightKnockout
        }
    },
    { // ST_Box2InnerShadowHighlightKnockoutMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox2InnerShadowHighlightKnockoutMul,
            FragShaderDesc::FSI_PS3_FBox2InnerShadowHighlightKnockoutMul
        }
    },
    { // ST_Box2ShadowHighlight
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox2ShadowHighlight,
            FragShaderDesc::FSI_PS3_FBox2ShadowHighlight
        }
    },
    { // ST_Box2ShadowHighlightMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox2ShadowHighlightMul,
            FragShaderDesc::FSI_PS3_FBox2ShadowHighlightMul
        }
    },
    { // ST_Box2ShadowHighlightKnockout
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox2ShadowHighlightKnockout,
            FragShaderDesc::FSI_PS3_FBox2ShadowHighlightKnockout
        }
    },
    { // ST_Box2ShadowHighlightKnockoutMul
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VBox2ShadowHighlightKnockoutMul,
            FragShaderDesc::FSI_PS3_FBox2ShadowHighlightKnockoutMul
        }
    },
    { // ST_DrawableCopyPixels
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VDrawableCopyPixels,
            FragShaderDesc::FSI_PS3_FDrawableCopyPixels
        }
    },
    { // ST_DrawableCopyPixelsNoDestAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VDrawableCopyPixelsNoDestAlpha,
            FragShaderDesc::FSI_PS3_FDrawableCopyPixelsNoDestAlpha
        }
    },
    { // ST_DrawableCopyPixelsMergeAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VDrawableCopyPixelsMergeAlpha,
            FragShaderDesc::FSI_PS3_FDrawableCopyPixelsMergeAlpha
        }
    },
    { // ST_DrawableCopyPixelsAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VDrawableCopyPixelsAlpha,
            FragShaderDesc::FSI_PS3_FDrawableCopyPixelsAlpha
        }
    },
    { // ST_DrawableCopyPixelsAlphaNoDestAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VDrawableCopyPixelsAlphaNoDestAlpha,
            FragShaderDesc::FSI_PS3_FDrawableCopyPixelsAlphaNoDestAlpha
        }
    },
    { // ST_DrawableCopyPixelsAlphaMergeAlpha
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VDrawableCopyPixelsAlphaMergeAlpha,
            FragShaderDesc::FSI_PS3_FDrawableCopyPixelsAlphaMergeAlpha
        }
    },
    { // ST_DrawableMerge
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VDrawableMerge,
            FragShaderDesc::FSI_PS3_FDrawableMerge
        }
    },
    { // ST_DrawableCompare
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VDrawableCompare,
            FragShaderDesc::FSI_PS3_FDrawableCompare
        }
    },
    { // ST_DrawablePaletteMap
        { // ShaderVersion_PS3
            VertexShaderDesc::VSI_PS3_VDrawablePaletteMap,
            FragShaderDesc::FSI_PS3_FDrawablePaletteMap
        }
    },
};
extern "C" const int _binary_PS3_VTexTG_po_start[];
extern "C" const int _binary_PS3_VTexTGEAlpha_po_start[];
extern "C" const int _binary_PS3_VTexTGCxform_po_start[];
extern "C" const int _binary_PS3_VTexTGCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_VVertex_po_start[];
extern "C" const int _binary_PS3_VVertexEAlpha_po_start[];
extern "C" const int _binary_PS3_VVertexCxform_po_start[];
extern "C" const int _binary_PS3_VVertexCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_VTexTGTexTG_po_start[];
extern "C" const int _binary_PS3_VTexTGTexTGCxform_po_start[];
extern "C" const int _binary_PS3_VTexTGVertex_po_start[];
extern "C" const int _binary_PS3_VTexTGVertexCxform_po_start[];
extern "C" const int _binary_PS3_VPosition3dTexTG_po_start[];
extern "C" const int _binary_PS3_VPosition3dTexTGEAlpha_po_start[];
extern "C" const int _binary_PS3_VPosition3dTexTGCxform_po_start[];
extern "C" const int _binary_PS3_VPosition3dTexTGCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_VPosition3dVertex_po_start[];
extern "C" const int _binary_PS3_VPosition3dVertexEAlpha_po_start[];
extern "C" const int _binary_PS3_VPosition3dVertexCxform_po_start[];
extern "C" const int _binary_PS3_VPosition3dVertexCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_VPosition3dTexTGTexTG_po_start[];
extern "C" const int _binary_PS3_VPosition3dTexTGTexTGCxform_po_start[];
extern "C" const int _binary_PS3_VPosition3dTexTGVertex_po_start[];
extern "C" const int _binary_PS3_VPosition3dTexTGVertexCxform_po_start[];
extern "C" const int _binary_PS3_VBatchTexTG_po_start[];
extern "C" const int _binary_PS3_VBatchTexTGEAlpha_po_start[];
extern "C" const int _binary_PS3_VBatchTexTGCxform_po_start[];
extern "C" const int _binary_PS3_VBatchTexTGCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_VBatchVertex_po_start[];
extern "C" const int _binary_PS3_VBatchVertexEAlpha_po_start[];
extern "C" const int _binary_PS3_VBatchVertexCxform_po_start[];
extern "C" const int _binary_PS3_VBatchVertexCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_VBatchTexTGTexTG_po_start[];
extern "C" const int _binary_PS3_VBatchTexTGTexTGCxform_po_start[];
extern "C" const int _binary_PS3_VBatchTexTGVertex_po_start[];
extern "C" const int _binary_PS3_VBatchTexTGVertexCxform_po_start[];
extern "C" const int _binary_PS3_VBatchPosition3dTexTG_po_start[];
extern "C" const int _binary_PS3_VBatchPosition3dTexTGEAlpha_po_start[];
extern "C" const int _binary_PS3_VBatchPosition3dTexTGCxform_po_start[];
extern "C" const int _binary_PS3_VBatchPosition3dTexTGCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_VBatchPosition3dVertex_po_start[];
extern "C" const int _binary_PS3_VBatchPosition3dVertexEAlpha_po_start[];
extern "C" const int _binary_PS3_VBatchPosition3dVertexCxform_po_start[];
extern "C" const int _binary_PS3_VBatchPosition3dVertexCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_VBatchPosition3dTexTGTexTG_po_start[];
extern "C" const int _binary_PS3_VBatchPosition3dTexTGTexTGCxform_po_start[];
extern "C" const int _binary_PS3_VBatchPosition3dTexTGVertex_po_start[];
extern "C" const int _binary_PS3_VBatchPosition3dTexTGVertexCxform_po_start[];
extern "C" const int _binary_PS3_VTexTG_po_start[];
extern "C" const int _binary_PS3_VTexTGEAlpha_po_start[];
extern "C" const int _binary_PS3_VTexTGCxform_po_start[];
extern "C" const int _binary_PS3_VTexTGCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_VVertex_po_start[];
extern "C" const int _binary_PS3_VVertexEAlpha_po_start[];
extern "C" const int _binary_PS3_VVertexCxform_po_start[];
extern "C" const int _binary_PS3_VVertexCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_VTexTGTexTG_po_start[];
extern "C" const int _binary_PS3_VTexTGTexTGCxform_po_start[];
extern "C" const int _binary_PS3_VTexTGVertex_po_start[];
extern "C" const int _binary_PS3_VTexTGVertexCxform_po_start[];
extern "C" const int _binary_PS3_VPosition3dTexTG_po_start[];
extern "C" const int _binary_PS3_VPosition3dTexTGEAlpha_po_start[];
extern "C" const int _binary_PS3_VPosition3dTexTGCxform_po_start[];
extern "C" const int _binary_PS3_VPosition3dTexTGCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_VPosition3dVertex_po_start[];
extern "C" const int _binary_PS3_VPosition3dVertexEAlpha_po_start[];
extern "C" const int _binary_PS3_VPosition3dVertexCxform_po_start[];
extern "C" const int _binary_PS3_VPosition3dVertexCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_VPosition3dTexTGTexTG_po_start[];
extern "C" const int _binary_PS3_VPosition3dTexTGTexTGCxform_po_start[];
extern "C" const int _binary_PS3_VPosition3dTexTGVertex_po_start[];
extern "C" const int _binary_PS3_VPosition3dTexTGVertexCxform_po_start[];
extern "C" const int _binary_PS3_VSolid_po_start[];
extern "C" const int _binary_PS3_VText_po_start[];
extern "C" const int _binary_PS3_VTextColor_po_start[];
extern "C" const int _binary_PS3_VTextColorCxform_po_start[];
extern "C" const int _binary_PS3_VPosition3dSolid_po_start[];
extern "C" const int _binary_PS3_VPosition3dText_po_start[];
extern "C" const int _binary_PS3_VPosition3dTextColor_po_start[];
extern "C" const int _binary_PS3_VPosition3dTextColorCxform_po_start[];
extern "C" const int _binary_PS3_VBatchSolid_po_start[];
extern "C" const int _binary_PS3_VBatchText_po_start[];
extern "C" const int _binary_PS3_VBatchTextColor_po_start[];
extern "C" const int _binary_PS3_VBatchTextColorCxform_po_start[];
extern "C" const int _binary_PS3_VBatchPosition3dSolid_po_start[];
extern "C" const int _binary_PS3_VBatchPosition3dText_po_start[];
extern "C" const int _binary_PS3_VBatchPosition3dTextColor_po_start[];
extern "C" const int _binary_PS3_VBatchPosition3dTextColorCxform_po_start[];
extern "C" const int _binary_PS3_VSolid_po_start[];
extern "C" const int _binary_PS3_VText_po_start[];
extern "C" const int _binary_PS3_VTextColor_po_start[];
extern "C" const int _binary_PS3_VTextColorCxform_po_start[];
extern "C" const int _binary_PS3_VPosition3dSolid_po_start[];
extern "C" const int _binary_PS3_VPosition3dText_po_start[];
extern "C" const int _binary_PS3_VPosition3dTextColor_po_start[];
extern "C" const int _binary_PS3_VPosition3dTextColorCxform_po_start[];
extern "C" const int _binary_PS3_VTexTGCxform_po_start[];
extern "C" const int _binary_PS3_VDrawableCopyPixels_po_start[];
extern "C" const int _binary_PS3_VDrawableCopyPixelsAlpha_po_start[];

UniformVar Uniforms_2402505279[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {0, 0, 4, 8, 4, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     {2, 8, 4, 8, 4, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

BatchVar BatchUniforms_6184305142[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_Count, -1, 0},
    /* cxmul*/       {Uniform::SU_Count, -1, 0},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_Count, -1, 0},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_Count, -1, 0},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

UniformVar Uniforms_2315053743[Uniform::SU_Count] = 
{
    /* cxadd*/      {0, 0, 4, 4, 4, 0 },
    /* cxmul*/      {1, 4, 4, 4, 4, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {2, 8, 4, 8, 4, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     {4, 16, 4, 8, 4, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_3962131312[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {0, 0, 4, 8, 4, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_4660718830[Uniform::SU_Count] = 
{
    /* cxadd*/      {0, 0, 4, 4, 4, 0 },
    /* cxmul*/      {1, 4, 4, 4, 4, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {2, 8, 4, 8, 4, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_2666373766[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {0, 0, 4, 8, 4, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     {2, 8, 4, 16, 4, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_3200611686[Uniform::SU_Count] = 
{
    /* cxadd*/      {0, 0, 4, 4, 4, 0 },
    /* cxmul*/      {1, 4, 4, 4, 4, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {2, 8, 4, 8, 4, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     {4, 16, 4, 16, 4, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_6395323925[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {0, 0, 16, 16, 16, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     {4, 16, 4, 8, 4, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_4496992820[Uniform::SU_Count] = 
{
    /* cxadd*/      {0, 0, 4, 4, 4, 0 },
    /* cxmul*/      {1, 4, 4, 4, 4, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {2, 8, 16, 16, 16, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     {6, 24, 4, 8, 4, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_4480697281[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {0, 0, 16, 16, 16, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_3978807109[Uniform::SU_Count] = 
{
    /* cxadd*/      {0, 0, 4, 4, 4, 0 },
    /* cxmul*/      {1, 4, 4, 4, 4, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {2, 8, 16, 16, 16, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_2590809056[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {0, 0, 16, 16, 16, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     {4, 16, 4, 16, 4, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_6124559549[Uniform::SU_Count] = 
{
    /* cxadd*/      {0, 0, 4, 4, 4, 0 },
    /* cxmul*/      {1, 4, 4, 4, 4, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {2, 8, 16, 16, 16, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     {6, 24, 4, 16, 4, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_4025051752[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ {0, 0, 4, 384, 4, 4 },
};

BatchVar BatchUniforms_3478612506[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_Count, -1, 0},
    /* cxmul*/       {Uniform::SU_Count, -1, 0},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_vfuniforms, 0, 2},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_vfuniforms, 2, 2},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

UniformVar Uniforms_4705861522[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ {0, 0, 4, 576, 4, 6 },
};

BatchVar BatchUniforms_6232566636[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_vfuniforms, 0, 1},
    /* cxmul*/       {Uniform::SU_vfuniforms, 1, 1},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_vfuniforms, 2, 2},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_vfuniforms, 4, 2},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

UniformVar Uniforms_2647200773[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ {0, 0, 4, 192, 4, 2 },
};

BatchVar BatchUniforms_2375807902[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_Count, -1, 0},
    /* cxmul*/       {Uniform::SU_Count, -1, 0},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_vfuniforms, 0, 2},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_Count, -1, 0},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

BatchVar BatchUniforms_5188561028[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_vfuniforms, 0, 1},
    /* cxmul*/       {Uniform::SU_vfuniforms, 1, 1},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_vfuniforms, 2, 2},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_Count, -1, 0},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

BatchVar BatchUniforms_3422124174[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_Count, -1, 0},
    /* cxmul*/       {Uniform::SU_Count, -1, 0},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_vfuniforms, 0, 2},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_vfuniforms, 2, 4},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

UniformVar Uniforms_2393121201[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ {0, 0, 4, 768, 4, 8 },
};

BatchVar BatchUniforms_4076936984[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_vfuniforms, 0, 1},
    /* cxmul*/       {Uniform::SU_vfuniforms, 1, 1},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_vfuniforms, 2, 2},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_vfuniforms, 4, 4},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

UniformVar Uniforms_3965965598[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{0, 0, 16, 384, 16, 1 },
    /* vfuniforms*/ {96, 384, 4, 192, 4, 2 },
};

BatchVar BatchUniforms_2889592211[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_Count, -1, 0},
    /* cxmul*/       {Uniform::SU_Count, -1, 0},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_vfmuniforms, 0, 1},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_vfuniforms, 0, 2},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

UniformVar Uniforms_3892239162[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{0, 0, 16, 384, 16, 1 },
    /* vfuniforms*/ {96, 384, 4, 384, 4, 4 },
};

BatchVar BatchUniforms_4272630808[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_vfuniforms, 0, 1},
    /* cxmul*/       {Uniform::SU_vfuniforms, 1, 1},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_vfmuniforms, 0, 1},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_vfuniforms, 2, 2},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

UniformVar Uniforms_5257564337[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{0, 0, 16, 384, 16, 1 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

BatchVar BatchUniforms_4952385266[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_Count, -1, 0},
    /* cxmul*/       {Uniform::SU_Count, -1, 0},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_vfmuniforms, 0, 1},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_Count, -1, 0},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

BatchVar BatchUniforms_5592595562[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_vfuniforms, 0, 1},
    /* cxmul*/       {Uniform::SU_vfuniforms, 1, 1},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_vfmuniforms, 0, 1},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_Count, -1, 0},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

BatchVar BatchUniforms_4749803573[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_Count, -1, 0},
    /* cxmul*/       {Uniform::SU_Count, -1, 0},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_vfmuniforms, 0, 1},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_vfuniforms, 0, 4},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

UniformVar Uniforms_6117254102[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{0, 0, 16, 384, 16, 1 },
    /* vfuniforms*/ {96, 384, 4, 576, 4, 6 },
};

BatchVar BatchUniforms_3953522647[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_vfuniforms, 0, 1},
    /* cxmul*/       {Uniform::SU_vfuniforms, 1, 1},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_vfmuniforms, 0, 1},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_vfuniforms, 2, 4},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

UniformVar Uniforms_2267270599[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {0, 0, 4, 8, 4, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     {2, 8, 4, 24, 4, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

static VertexShaderDesc ShaderDesc_VS_PS3_VTexTG = 
{
    /* Type */          ShaderDesc::ST_TexTG,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VTexTG_po_start,
    /* Uniforms */      Uniforms_2402505279,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VTexTGEAlpha_po_start,
    /* Uniforms */      Uniforms_2402505279,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_TexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VTexTGCxform_po_start,
    /* Uniforms */      Uniforms_2315053743,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VTexTGCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_2315053743,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VVertex = 
{
    /* Type */          ShaderDesc::ST_Vertex,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VVertex_po_start,
    /* Uniforms */      Uniforms_3962131312,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_VertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VVertexEAlpha_po_start,
    /* Uniforms */      Uniforms_3962131312,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VVertexCxform = 
{
    /* Type */          ShaderDesc::ST_VertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VVertexCxform_po_start,
    /* Uniforms */      Uniforms_4660718830,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_VertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VVertexCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_4660718830,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VTexTGTexTG = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTG,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VTexTGTexTG_po_start,
    /* Uniforms */      Uniforms_2666373766,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VTexTGTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VTexTGTexTGCxform_po_start,
    /* Uniforms */      Uniforms_3200611686,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VTexTGVertex = 
{
    /* Type */          ShaderDesc::ST_TexTGVertex,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VTexTGVertex_po_start,
    /* Uniforms */      Uniforms_2402505279,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VTexTGVertexCxform = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VTexTGVertexCxform_po_start,
    /* Uniforms */      Uniforms_2315053743,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VPosition3dTexTG = 
{
    /* Type */          ShaderDesc::ST_Position3dTexTG,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VPosition3dTexTG_po_start,
    /* Uniforms */      Uniforms_6395323925,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VPosition3dTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_Position3dTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VPosition3dTexTGEAlpha_po_start,
    /* Uniforms */      Uniforms_6395323925,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VPosition3dTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_Position3dTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VPosition3dTexTGCxform_po_start,
    /* Uniforms */      Uniforms_4496992820,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VPosition3dTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_Position3dTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VPosition3dTexTGCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_4496992820,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VPosition3dVertex = 
{
    /* Type */          ShaderDesc::ST_Position3dVertex,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VPosition3dVertex_po_start,
    /* Uniforms */      Uniforms_4480697281,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VPosition3dVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_Position3dVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VPosition3dVertexEAlpha_po_start,
    /* Uniforms */      Uniforms_4480697281,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VPosition3dVertexCxform = 
{
    /* Type */          ShaderDesc::ST_Position3dVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VPosition3dVertexCxform_po_start,
    /* Uniforms */      Uniforms_3978807109,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VPosition3dVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_Position3dVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VPosition3dVertexCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_3978807109,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VPosition3dTexTGTexTG = 
{
    /* Type */          ShaderDesc::ST_Position3dTexTGTexTG,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VPosition3dTexTGTexTG_po_start,
    /* Uniforms */      Uniforms_2590809056,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VPosition3dTexTGTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_Position3dTexTGTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VPosition3dTexTGTexTGCxform_po_start,
    /* Uniforms */      Uniforms_6124559549,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VPosition3dTexTGVertex = 
{
    /* Type */          ShaderDesc::ST_Position3dTexTGVertex,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VPosition3dTexTGVertex_po_start,
    /* Uniforms */      Uniforms_6395323925,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VPosition3dTexTGVertexCxform = 
{
    /* Type */          ShaderDesc::ST_Position3dTexTGVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VPosition3dTexTGVertexCxform_po_start,
    /* Uniforms */      Uniforms_4496992820,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchTexTG = 
{
    /* Type */          ShaderDesc::ST_BatchTexTG,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchTexTG_po_start,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_3478612506,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "pos",        4 | VET_Pos},
        { "vbatch",     1 | VET_Instance8},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchTexTGEAlpha_po_start,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_3478612506,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchTexTGCxform_po_start,
    /* Uniforms */      Uniforms_4705861522,
    /* BatchUniforms */ BatchUniforms_6232566636,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "pos",        4 | VET_Pos},
        { "vbatch",     1 | VET_Instance8},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchTexTGCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_4705861522,
    /* BatchUniforms */ BatchUniforms_6232566636,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchVertex = 
{
    /* Type */          ShaderDesc::ST_BatchVertex,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchVertex_po_start,
    /* Uniforms */      Uniforms_2647200773,
    /* BatchUniforms */ BatchUniforms_2375807902,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "pos",        4 | VET_Pos},
        { "vbatch",     1 | VET_Instance8},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchVertexEAlpha_po_start,
    /* Uniforms */      Uniforms_2647200773,
    /* BatchUniforms */ BatchUniforms_2375807902,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchVertexCxform = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchVertexCxform_po_start,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_5188561028,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "pos",        4 | VET_Pos},
        { "vbatch",     1 | VET_Instance8},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchVertexCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_5188561028,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchTexTGTexTG = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTG,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchTexTGTexTG_po_start,
    /* Uniforms */      Uniforms_4705861522,
    /* BatchUniforms */ BatchUniforms_3422124174,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchTexTGTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchTexTGTexTGCxform_po_start,
    /* Uniforms */      Uniforms_2393121201,
    /* BatchUniforms */ BatchUniforms_4076936984,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchTexTGVertex = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertex,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchTexTGVertex_po_start,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_3478612506,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchTexTGVertexCxform = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchTexTGVertexCxform_po_start,
    /* Uniforms */      Uniforms_4705861522,
    /* BatchUniforms */ BatchUniforms_6232566636,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchPosition3dTexTG = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTexTG,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchPosition3dTexTG_po_start,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_2889592211,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "pos",        4 | VET_Pos},
        { "vbatch",     1 | VET_Instance8},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchPosition3dTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchPosition3dTexTGEAlpha_po_start,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_2889592211,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchPosition3dTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchPosition3dTexTGCxform_po_start,
    /* Uniforms */      Uniforms_3892239162,
    /* BatchUniforms */ BatchUniforms_4272630808,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "pos",        4 | VET_Pos},
        { "vbatch",     1 | VET_Instance8},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchPosition3dTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchPosition3dTexTGCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_3892239162,
    /* BatchUniforms */ BatchUniforms_4272630808,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchPosition3dVertex = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dVertex,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchPosition3dVertex_po_start,
    /* Uniforms */      Uniforms_5257564337,
    /* BatchUniforms */ BatchUniforms_4952385266,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "pos",        4 | VET_Pos},
        { "vbatch",     1 | VET_Instance8},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchPosition3dVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchPosition3dVertexEAlpha_po_start,
    /* Uniforms */      Uniforms_5257564337,
    /* BatchUniforms */ BatchUniforms_4952385266,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchPosition3dVertexCxform = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchPosition3dVertexCxform_po_start,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_5592595562,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "pos",        4 | VET_Pos},
        { "vbatch",     1 | VET_Instance8},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchPosition3dVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchPosition3dVertexCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_5592595562,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchPosition3dTexTGTexTG = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTexTGTexTG,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchPosition3dTexTGTexTG_po_start,
    /* Uniforms */      Uniforms_3892239162,
    /* BatchUniforms */ BatchUniforms_4749803573,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchPosition3dTexTGTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTexTGTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchPosition3dTexTGTexTGCxform_po_start,
    /* Uniforms */      Uniforms_6117254102,
    /* BatchUniforms */ BatchUniforms_3953522647,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchPosition3dTexTGVertex = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTexTGVertex,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchPosition3dTexTGVertex_po_start,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_2889592211,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchPosition3dTexTGVertexCxform = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTexTGVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchPosition3dTexTGVertexCxform_po_start,
    /* Uniforms */      Uniforms_3892239162,
    /* BatchUniforms */ BatchUniforms_4272630808,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedTexTG = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTG,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VTexTG_po_start,
    /* Uniforms */      Uniforms_2402505279,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VTexTGEAlpha_po_start,
    /* Uniforms */      Uniforms_2402505279,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VTexTGCxform_po_start,
    /* Uniforms */      Uniforms_2315053743,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VTexTGCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_2315053743,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedVertex = 
{
    /* Type */          ShaderDesc::ST_InstancedVertex,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VVertex_po_start,
    /* Uniforms */      Uniforms_3962131312,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VVertexEAlpha_po_start,
    /* Uniforms */      Uniforms_3962131312,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedVertexCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VVertexCxform_po_start,
    /* Uniforms */      Uniforms_4660718830,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VVertexCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_4660718830,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedTexTGTexTG = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTG,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VTexTGTexTG_po_start,
    /* Uniforms */      Uniforms_2666373766,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedTexTGTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VTexTGTexTGCxform_po_start,
    /* Uniforms */      Uniforms_3200611686,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedTexTGVertex = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertex,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VTexTGVertex_po_start,
    /* Uniforms */      Uniforms_2402505279,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedTexTGVertexCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VTexTGVertexCxform_po_start,
    /* Uniforms */      Uniforms_2315053743,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedPosition3dTexTG = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dTexTG,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VPosition3dTexTG_po_start,
    /* Uniforms */      Uniforms_6395323925,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedPosition3dTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VPosition3dTexTGEAlpha_po_start,
    /* Uniforms */      Uniforms_6395323925,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedPosition3dTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VPosition3dTexTGCxform_po_start,
    /* Uniforms */      Uniforms_4496992820,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedPosition3dTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VPosition3dTexTGCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_4496992820,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedPosition3dVertex = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dVertex,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VPosition3dVertex_po_start,
    /* Uniforms */      Uniforms_4480697281,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedPosition3dVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VPosition3dVertexEAlpha_po_start,
    /* Uniforms */      Uniforms_4480697281,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedPosition3dVertexCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VPosition3dVertexCxform_po_start,
    /* Uniforms */      Uniforms_3978807109,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedPosition3dVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VPosition3dVertexCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_3978807109,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedPosition3dTexTGTexTG = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dTexTGTexTG,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VPosition3dTexTGTexTG_po_start,
    /* Uniforms */      Uniforms_2590809056,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedPosition3dTexTGTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dTexTGTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VPosition3dTexTGTexTGCxform_po_start,
    /* Uniforms */      Uniforms_6124559549,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedPosition3dTexTGVertex = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dTexTGVertex,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VPosition3dTexTGVertex_po_start,
    /* Uniforms */      Uniforms_6395323925,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedPosition3dTexTGVertexCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dTexTGVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VPosition3dTexTGVertexCxform_po_start,
    /* Uniforms */      Uniforms_4496992820,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "afactor",    4 | VET_Color | (1 << VET_Index_Shift)},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VSolid = 
{
    /* Type */          ShaderDesc::ST_Solid,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VSolid_po_start,
    /* Uniforms */      Uniforms_3962131312,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VText = 
{
    /* Type */          ShaderDesc::ST_Text,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VText_po_start,
    /* Uniforms */      Uniforms_4660718830,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "atc",        2 | VET_TexCoord},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VTextColor = 
{
    /* Type */          ShaderDesc::ST_TextColor,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VTextColor_po_start,
    /* Uniforms */      Uniforms_3962131312,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "atc",        2 | VET_TexCoord},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VTextColorCxform = 
{
    /* Type */          ShaderDesc::ST_TextColorCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VTextColorCxform_po_start,
    /* Uniforms */      Uniforms_4660718830,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "atc",        2 | VET_TexCoord},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VPosition3dSolid = 
{
    /* Type */          ShaderDesc::ST_Position3dSolid,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VPosition3dSolid_po_start,
    /* Uniforms */      Uniforms_4480697281,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VPosition3dText = 
{
    /* Type */          ShaderDesc::ST_Position3dText,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VPosition3dText_po_start,
    /* Uniforms */      Uniforms_3978807109,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "atc",        2 | VET_TexCoord},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VPosition3dTextColor = 
{
    /* Type */          ShaderDesc::ST_Position3dTextColor,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VPosition3dTextColor_po_start,
    /* Uniforms */      Uniforms_4480697281,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "atc",        2 | VET_TexCoord},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VPosition3dTextColorCxform = 
{
    /* Type */          ShaderDesc::ST_Position3dTextColorCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VPosition3dTextColorCxform_po_start,
    /* Uniforms */      Uniforms_3978807109,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "atc",        2 | VET_TexCoord},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchSolid = 
{
    /* Type */          ShaderDesc::ST_BatchSolid,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchSolid_po_start,
    /* Uniforms */      Uniforms_2647200773,
    /* BatchUniforms */ BatchUniforms_2375807902,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "pos",        4 | VET_Pos},
        { "vbatch",     1 | VET_Instance8},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchText = 
{
    /* Type */          ShaderDesc::ST_BatchText,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchText_po_start,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_5188561028,
    /* NumAttribs */    4,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "atc",        2 | VET_TexCoord},
        { "pos",        4 | VET_Pos},
        { "vbatch",     1 | VET_Instance8},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchTextColor = 
{
    /* Type */          ShaderDesc::ST_BatchTextColor,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchTextColor_po_start,
    /* Uniforms */      Uniforms_2647200773,
    /* BatchUniforms */ BatchUniforms_2375807902,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "atc",        2 | VET_TexCoord},
        { "pos",        4 | VET_Pos},
        { "vbatch",     1 | VET_Instance8},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchTextColorCxform = 
{
    /* Type */          ShaderDesc::ST_BatchTextColorCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchTextColorCxform_po_start,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_5188561028,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "atc",        2 | VET_TexCoord},
        { "pos",        4 | VET_Pos},
        { "vbatch",     1 | VET_Instance8},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchPosition3dSolid = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dSolid,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchPosition3dSolid_po_start,
    /* Uniforms */      Uniforms_5257564337,
    /* BatchUniforms */ BatchUniforms_4952385266,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "pos",        4 | VET_Pos},
        { "vbatch",     1 | VET_Instance8},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchPosition3dText = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dText,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchPosition3dText_po_start,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_5592595562,
    /* NumAttribs */    4,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "atc",        2 | VET_TexCoord},
        { "pos",        4 | VET_Pos},
        { "vbatch",     1 | VET_Instance8},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchPosition3dTextColor = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTextColor,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchPosition3dTextColor_po_start,
    /* Uniforms */      Uniforms_5257564337,
    /* BatchUniforms */ BatchUniforms_4952385266,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "atc",        2 | VET_TexCoord},
        { "pos",        4 | VET_Pos},
        { "vbatch",     1 | VET_Instance8},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBatchPosition3dTextColorCxform = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTextColorCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_VBatchPosition3dTextColorCxform_po_start,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_5592595562,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "atc",        2 | VET_TexCoord},
        { "pos",        4 | VET_Pos},
        { "vbatch",     1 | VET_Instance8},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedSolid = 
{
    /* Type */          ShaderDesc::ST_InstancedSolid,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VSolid_po_start,
    /* Uniforms */      Uniforms_3962131312,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedText = 
{
    /* Type */          ShaderDesc::ST_InstancedText,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VText_po_start,
    /* Uniforms */      Uniforms_4660718830,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "atc",        2 | VET_TexCoord},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedTextColor = 
{
    /* Type */          ShaderDesc::ST_InstancedTextColor,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VTextColor_po_start,
    /* Uniforms */      Uniforms_3962131312,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "atc",        2 | VET_TexCoord},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedTextColorCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedTextColorCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VTextColorCxform_po_start,
    /* Uniforms */      Uniforms_4660718830,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "atc",        2 | VET_TexCoord},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedPosition3dSolid = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dSolid,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VPosition3dSolid_po_start,
    /* Uniforms */      Uniforms_4480697281,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedPosition3dText = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dText,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VPosition3dText_po_start,
    /* Uniforms */      Uniforms_3978807109,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
        { "acolor",     4 | VET_Color},
        { "atc",        2 | VET_TexCoord},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedPosition3dTextColor = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dTextColor,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VPosition3dTextColor_po_start,
    /* Uniforms */      Uniforms_4480697281,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "atc",        2 | VET_TexCoord},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VInstancedPosition3dTextColorCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dTextColorCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_VPosition3dTextColorCxform_po_start,
    /* Uniforms */      Uniforms_3978807109,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
        { "atc",        2 | VET_TexCoord},
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VBox1Blur = 
{
    /* Type */          ShaderDesc::ST_Box1Blur,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_VTexTGCxform_po_start,
    /* Uniforms */      Uniforms_2315053743,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VDrawableCopyPixels = 
{
    /* Type */          ShaderDesc::ST_DrawableCopyPixels,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VDrawableCopyPixels_po_start,
    /* Uniforms */      Uniforms_2666373766,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
        { "pos",        4 | VET_Pos},
    },
    
};

static VertexShaderDesc ShaderDesc_VS_PS3_VDrawableCopyPixelsAlpha = 
{
    /* Type */          ShaderDesc::ST_DrawableCopyPixelsAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_VDrawableCopyPixelsAlpha_po_start,
    /* Uniforms */      Uniforms_2267270599,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
        { "pos",        4 | VET_Pos},
    },
    
};

const VertexShaderDesc* VertexShaderDesc::Descs[VSI_Count] = {
    0,
    &ShaderDesc_VS_PS3_VTexTG,                                   // 1
    &ShaderDesc_VS_PS3_VTexTGEAlpha,                             // 2
    &ShaderDesc_VS_PS3_VTexTGCxform,                             // 3
    &ShaderDesc_VS_PS3_VTexTGCxformEAlpha,                       // 4
    &ShaderDesc_VS_PS3_VVertex,                                  // 5
    &ShaderDesc_VS_PS3_VVertexEAlpha,                            // 6
    &ShaderDesc_VS_PS3_VVertexCxform,                            // 7
    &ShaderDesc_VS_PS3_VVertexCxformEAlpha,                      // 8
    &ShaderDesc_VS_PS3_VTexTGTexTG,                              // 9
    &ShaderDesc_VS_PS3_VTexTGTexTGCxform,                        // 10
    &ShaderDesc_VS_PS3_VTexTGVertex,                             // 11
    &ShaderDesc_VS_PS3_VTexTGVertexCxform,                       // 12
    &ShaderDesc_VS_PS3_VPosition3dTexTG,                         // 13
    &ShaderDesc_VS_PS3_VPosition3dTexTGEAlpha,                   // 14
    &ShaderDesc_VS_PS3_VPosition3dTexTGCxform,                   // 15
    &ShaderDesc_VS_PS3_VPosition3dTexTGCxformEAlpha,             // 16
    &ShaderDesc_VS_PS3_VPosition3dVertex,                        // 17
    &ShaderDesc_VS_PS3_VPosition3dVertexEAlpha,                  // 18
    &ShaderDesc_VS_PS3_VPosition3dVertexCxform,                  // 19
    &ShaderDesc_VS_PS3_VPosition3dVertexCxformEAlpha,            // 20
    &ShaderDesc_VS_PS3_VPosition3dTexTGTexTG,                    // 21
    &ShaderDesc_VS_PS3_VPosition3dTexTGTexTGCxform,              // 22
    &ShaderDesc_VS_PS3_VPosition3dTexTGVertex,                   // 23
    &ShaderDesc_VS_PS3_VPosition3dTexTGVertexCxform,             // 24
    &ShaderDesc_VS_PS3_VBatchTexTG,                              // 25
    &ShaderDesc_VS_PS3_VBatchTexTGEAlpha,                        // 26
    &ShaderDesc_VS_PS3_VBatchTexTGCxform,                        // 27
    &ShaderDesc_VS_PS3_VBatchTexTGCxformEAlpha,                  // 28
    &ShaderDesc_VS_PS3_VBatchVertex,                             // 29
    &ShaderDesc_VS_PS3_VBatchVertexEAlpha,                       // 30
    &ShaderDesc_VS_PS3_VBatchVertexCxform,                       // 31
    &ShaderDesc_VS_PS3_VBatchVertexCxformEAlpha,                 // 32
    &ShaderDesc_VS_PS3_VBatchTexTGTexTG,                         // 33
    &ShaderDesc_VS_PS3_VBatchTexTGTexTGCxform,                   // 34
    &ShaderDesc_VS_PS3_VBatchTexTGVertex,                        // 35
    &ShaderDesc_VS_PS3_VBatchTexTGVertexCxform,                  // 36
    &ShaderDesc_VS_PS3_VBatchPosition3dTexTG,                    // 37
    &ShaderDesc_VS_PS3_VBatchPosition3dTexTGEAlpha,              // 38
    &ShaderDesc_VS_PS3_VBatchPosition3dTexTGCxform,              // 39
    &ShaderDesc_VS_PS3_VBatchPosition3dTexTGCxformEAlpha,        // 40
    &ShaderDesc_VS_PS3_VBatchPosition3dVertex,                   // 41
    &ShaderDesc_VS_PS3_VBatchPosition3dVertexEAlpha,             // 42
    &ShaderDesc_VS_PS3_VBatchPosition3dVertexCxform,             // 43
    &ShaderDesc_VS_PS3_VBatchPosition3dVertexCxformEAlpha,       // 44
    &ShaderDesc_VS_PS3_VBatchPosition3dTexTGTexTG,               // 45
    &ShaderDesc_VS_PS3_VBatchPosition3dTexTGTexTGCxform,         // 46
    &ShaderDesc_VS_PS3_VBatchPosition3dTexTGVertex,              // 47
    &ShaderDesc_VS_PS3_VBatchPosition3dTexTGVertexCxform,        // 48
    &ShaderDesc_VS_PS3_VInstancedTexTG,                          // 49
    &ShaderDesc_VS_PS3_VInstancedTexTGEAlpha,                    // 50
    &ShaderDesc_VS_PS3_VInstancedTexTGCxform,                    // 51
    &ShaderDesc_VS_PS3_VInstancedTexTGCxformEAlpha,              // 52
    &ShaderDesc_VS_PS3_VInstancedVertex,                         // 53
    &ShaderDesc_VS_PS3_VInstancedVertexEAlpha,                   // 54
    &ShaderDesc_VS_PS3_VInstancedVertexCxform,                   // 55
    &ShaderDesc_VS_PS3_VInstancedVertexCxformEAlpha,             // 56
    &ShaderDesc_VS_PS3_VInstancedTexTGTexTG,                     // 57
    &ShaderDesc_VS_PS3_VInstancedTexTGTexTGCxform,               // 58
    &ShaderDesc_VS_PS3_VInstancedTexTGVertex,                    // 59
    &ShaderDesc_VS_PS3_VInstancedTexTGVertexCxform,              // 60
    &ShaderDesc_VS_PS3_VInstancedPosition3dTexTG,                // 61
    &ShaderDesc_VS_PS3_VInstancedPosition3dTexTGEAlpha,          // 62
    &ShaderDesc_VS_PS3_VInstancedPosition3dTexTGCxform,          // 63
    &ShaderDesc_VS_PS3_VInstancedPosition3dTexTGCxformEAlpha,    // 64
    &ShaderDesc_VS_PS3_VInstancedPosition3dVertex,               // 65
    &ShaderDesc_VS_PS3_VInstancedPosition3dVertexEAlpha,         // 66
    &ShaderDesc_VS_PS3_VInstancedPosition3dVertexCxform,         // 67
    &ShaderDesc_VS_PS3_VInstancedPosition3dVertexCxformEAlpha,   // 68
    &ShaderDesc_VS_PS3_VInstancedPosition3dTexTGTexTG,           // 69
    &ShaderDesc_VS_PS3_VInstancedPosition3dTexTGTexTGCxform,     // 70
    &ShaderDesc_VS_PS3_VInstancedPosition3dTexTGVertex,          // 71
    &ShaderDesc_VS_PS3_VInstancedPosition3dTexTGVertexCxform,    // 72
    &ShaderDesc_VS_PS3_VSolid,                                   // 73
    &ShaderDesc_VS_PS3_VText,                                    // 74
    &ShaderDesc_VS_PS3_VTextColor,                               // 75
    &ShaderDesc_VS_PS3_VTextColorCxform,                         // 76
    &ShaderDesc_VS_PS3_VPosition3dSolid,                         // 77
    &ShaderDesc_VS_PS3_VPosition3dText,                          // 78
    &ShaderDesc_VS_PS3_VPosition3dTextColor,                     // 79
    &ShaderDesc_VS_PS3_VPosition3dTextColorCxform,               // 80
    &ShaderDesc_VS_PS3_VBatchSolid,                              // 81
    &ShaderDesc_VS_PS3_VBatchText,                               // 82
    &ShaderDesc_VS_PS3_VBatchTextColor,                          // 83
    &ShaderDesc_VS_PS3_VBatchTextColorCxform,                    // 84
    &ShaderDesc_VS_PS3_VBatchPosition3dSolid,                    // 85
    &ShaderDesc_VS_PS3_VBatchPosition3dText,                     // 86
    &ShaderDesc_VS_PS3_VBatchPosition3dTextColor,                // 87
    &ShaderDesc_VS_PS3_VBatchPosition3dTextColorCxform,          // 88
    &ShaderDesc_VS_PS3_VInstancedSolid,                          // 89
    &ShaderDesc_VS_PS3_VInstancedText,                           // 90
    &ShaderDesc_VS_PS3_VInstancedTextColor,                      // 91
    &ShaderDesc_VS_PS3_VInstancedTextColorCxform,                // 92
    &ShaderDesc_VS_PS3_VInstancedPosition3dSolid,                // 93
    &ShaderDesc_VS_PS3_VInstancedPosition3dText,                 // 94
    &ShaderDesc_VS_PS3_VInstancedPosition3dTextColor,            // 95
    &ShaderDesc_VS_PS3_VInstancedPosition3dTextColorCxform,      // 96
    &ShaderDesc_VS_PS3_VBox1Blur,                                // 97
    &ShaderDesc_VS_PS3_VDrawableCopyPixels,                      // 98
    &ShaderDesc_VS_PS3_VDrawableCopyPixelsAlpha,                 // 99
};

const VertexShaderDesc* VertexShaderDesc::GetDesc(ShaderDesc::ShaderType shader, ShaderDesc::ShaderVersion ver)
{
    return Descs[GetShaderIndex(shader, ver)];
};

VertexShaderDesc::ShaderIndex VertexShaderDesc::GetShaderIndex(ShaderDesc::ShaderType shader, ShaderDesc::ShaderVersion ver)
{
    ShaderDesc::ShaderIndex index = ShaderDesc::GetShaderIndex(shader);
    return ShaderIndexingData[index][ver]. VertexIndex;
};

VertexShaderDesc::ShaderIndex VertexShaderDesc::GetShaderIndexForComboIndex(unsigned index, ShaderDesc::ShaderVersion ver) 
{
    switch(ver)
    {
        case ShaderDesc::ShaderVersion_PS3:
        {
            SF_UNUSED(index);
            SF_DEBUG_ASSERT(1, "PS3 indicated that ShaderCombo information was not required.");
            return VSI_None;
        }
        default: SF_ASSERT(0); return VSI_None;
    }
}

unsigned VertexShaderDesc::GetShaderComboIndex(ShaderDesc::ShaderType shader, ShaderDesc::ShaderVersion ver)
{
    switch(ver)
    {
        case ShaderDesc::ShaderVersion_PS3:
        {
            SF_UNUSED(shader);
            SF_DEBUG_ASSERT(1, "PS3 indicated that ShaderCombo information was not required.");
            return 0;
        }
        default: SF_ASSERT(0); return 0;
    }
}

extern "C" const int _binary_PS3_FTexTG_po_start[];
extern "C" const int _binary_PS3_FTexTGMul_po_start[];
extern "C" const int _binary_PS3_FTexTGInv_po_start[];
extern "C" const int _binary_PS3_FTexTGEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTGCxform_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformMul_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformInv_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformAc_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformAcMul_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformAcInv_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformAcEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformAcEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformAcEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FVertex_po_start[];
extern "C" const int _binary_PS3_FVertexMul_po_start[];
extern "C" const int _binary_PS3_FVertexInv_po_start[];
extern "C" const int _binary_PS3_FVertexEAlpha_po_start[];
extern "C" const int _binary_PS3_FVertexEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FVertexEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FVertexCxform_po_start[];
extern "C" const int _binary_PS3_FVertexCxformMul_po_start[];
extern "C" const int _binary_PS3_FVertexCxformInv_po_start[];
extern "C" const int _binary_PS3_FVertexCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_FVertexCxformEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FVertexCxformEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FVertexCxformAc_po_start[];
extern "C" const int _binary_PS3_FVertexCxformAcMul_po_start[];
extern "C" const int _binary_PS3_FVertexCxformAcInv_po_start[];
extern "C" const int _binary_PS3_FVertexCxformAcEAlpha_po_start[];
extern "C" const int _binary_PS3_FVertexCxformAcEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FVertexCxformAcEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTG_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGMul_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGInv_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxform_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformMul_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformInv_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformAc_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformAcMul_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformAcInv_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformAcEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformAcEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformAcEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTGVertex_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexMul_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexInv_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxform_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformMul_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformInv_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformAc_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformAcMul_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformAcInv_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformAcEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformAcEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformAcEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTG_po_start[];
extern "C" const int _binary_PS3_FTexTGMul_po_start[];
extern "C" const int _binary_PS3_FTexTGInv_po_start[];
extern "C" const int _binary_PS3_FTexTGEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTGCxform_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformMul_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformInv_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformAc_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformAcMul_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformAcInv_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformAcEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformAcEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformAcEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FVertex_po_start[];
extern "C" const int _binary_PS3_FVertexMul_po_start[];
extern "C" const int _binary_PS3_FVertexInv_po_start[];
extern "C" const int _binary_PS3_FVertexEAlpha_po_start[];
extern "C" const int _binary_PS3_FVertexEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FVertexEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FVertexCxform_po_start[];
extern "C" const int _binary_PS3_FVertexCxformMul_po_start[];
extern "C" const int _binary_PS3_FVertexCxformInv_po_start[];
extern "C" const int _binary_PS3_FVertexCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_FVertexCxformEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FVertexCxformEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FVertexCxformAc_po_start[];
extern "C" const int _binary_PS3_FVertexCxformAcMul_po_start[];
extern "C" const int _binary_PS3_FVertexCxformAcInv_po_start[];
extern "C" const int _binary_PS3_FVertexCxformAcEAlpha_po_start[];
extern "C" const int _binary_PS3_FVertexCxformAcEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FVertexCxformAcEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTG_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGMul_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGInv_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxform_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformMul_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformInv_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformAc_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformAcMul_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformAcInv_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformAcEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformAcEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformAcEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTGVertex_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexMul_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexInv_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxform_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformMul_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformInv_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformAc_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformAcMul_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformAcInv_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformAcEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformAcEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformAcEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTG_po_start[];
extern "C" const int _binary_PS3_FTexTGMul_po_start[];
extern "C" const int _binary_PS3_FTexTGInv_po_start[];
extern "C" const int _binary_PS3_FTexTGEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTGCxform_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformMul_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformInv_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformAc_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformAcMul_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformAcInv_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformAcEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformAcEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGCxformAcEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FVertex_po_start[];
extern "C" const int _binary_PS3_FVertexMul_po_start[];
extern "C" const int _binary_PS3_FVertexInv_po_start[];
extern "C" const int _binary_PS3_FVertexEAlpha_po_start[];
extern "C" const int _binary_PS3_FVertexEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FVertexEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FVertexCxform_po_start[];
extern "C" const int _binary_PS3_FVertexCxformMul_po_start[];
extern "C" const int _binary_PS3_FVertexCxformInv_po_start[];
extern "C" const int _binary_PS3_FVertexCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_FVertexCxformEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FVertexCxformEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FVertexCxformAc_po_start[];
extern "C" const int _binary_PS3_FVertexCxformAcMul_po_start[];
extern "C" const int _binary_PS3_FVertexCxformAcInv_po_start[];
extern "C" const int _binary_PS3_FVertexCxformAcEAlpha_po_start[];
extern "C" const int _binary_PS3_FVertexCxformAcEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FVertexCxformAcEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTG_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGMul_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGInv_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxform_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformMul_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformInv_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformAc_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformAcMul_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformAcInv_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformAcEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformAcEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGTexTGCxformAcEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTGVertex_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexMul_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexInv_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxform_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformMul_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformInv_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformAc_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformAcMul_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformAcInv_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformAcEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformAcEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FTexTGVertexCxformAcEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FSolid_po_start[];
extern "C" const int _binary_PS3_FSolidMul_po_start[];
extern "C" const int _binary_PS3_FSolidInv_po_start[];
extern "C" const int _binary_PS3_FText_po_start[];
extern "C" const int _binary_PS3_FTextMul_po_start[];
extern "C" const int _binary_PS3_FTextInv_po_start[];
extern "C" const int _binary_PS3_FSolid_po_start[];
extern "C" const int _binary_PS3_FSolidMul_po_start[];
extern "C" const int _binary_PS3_FSolidInv_po_start[];
extern "C" const int _binary_PS3_FText_po_start[];
extern "C" const int _binary_PS3_FTextMul_po_start[];
extern "C" const int _binary_PS3_FTextInv_po_start[];
extern "C" const int _binary_PS3_FSolid_po_start[];
extern "C" const int _binary_PS3_FSolidMul_po_start[];
extern "C" const int _binary_PS3_FSolidInv_po_start[];
extern "C" const int _binary_PS3_FText_po_start[];
extern "C" const int _binary_PS3_FTextMul_po_start[];
extern "C" const int _binary_PS3_FTextInv_po_start[];
extern "C" const int _binary_PS3_FYUV_po_start[];
extern "C" const int _binary_PS3_FYUVMul_po_start[];
extern "C" const int _binary_PS3_FYUVEAlpha_po_start[];
extern "C" const int _binary_PS3_FYUVEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FYUVCxform_po_start[];
extern "C" const int _binary_PS3_FYUVCxformMul_po_start[];
extern "C" const int _binary_PS3_FYUVCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_FYUVCxformEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FYUVCxformAc_po_start[];
extern "C" const int _binary_PS3_FYUVCxformAcMul_po_start[];
extern "C" const int _binary_PS3_FYUVCxformAcEAlpha_po_start[];
extern "C" const int _binary_PS3_FYUVCxformAcEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FYUVA_po_start[];
extern "C" const int _binary_PS3_FYUVAMul_po_start[];
extern "C" const int _binary_PS3_FYUVAInv_po_start[];
extern "C" const int _binary_PS3_FYUVAEAlpha_po_start[];
extern "C" const int _binary_PS3_FYUVAEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FYUVAEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FYUVACxform_po_start[];
extern "C" const int _binary_PS3_FYUVACxformMul_po_start[];
extern "C" const int _binary_PS3_FYUVACxformInv_po_start[];
extern "C" const int _binary_PS3_FYUVACxformEAlpha_po_start[];
extern "C" const int _binary_PS3_FYUVACxformEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FYUVACxformEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FYUVACxformAc_po_start[];
extern "C" const int _binary_PS3_FYUVACxformAcMul_po_start[];
extern "C" const int _binary_PS3_FYUVACxformAcInv_po_start[];
extern "C" const int _binary_PS3_FYUVACxformAcEAlpha_po_start[];
extern "C" const int _binary_PS3_FYUVACxformAcEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FYUVACxformAcEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FYUV_po_start[];
extern "C" const int _binary_PS3_FYUVMul_po_start[];
extern "C" const int _binary_PS3_FYUVEAlpha_po_start[];
extern "C" const int _binary_PS3_FYUVEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FYUVCxform_po_start[];
extern "C" const int _binary_PS3_FYUVCxformMul_po_start[];
extern "C" const int _binary_PS3_FYUVCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_FYUVCxformEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FYUVCxformAc_po_start[];
extern "C" const int _binary_PS3_FYUVCxformAcMul_po_start[];
extern "C" const int _binary_PS3_FYUVCxformAcEAlpha_po_start[];
extern "C" const int _binary_PS3_FYUVCxformAcEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FYUVA_po_start[];
extern "C" const int _binary_PS3_FYUVAMul_po_start[];
extern "C" const int _binary_PS3_FYUVAInv_po_start[];
extern "C" const int _binary_PS3_FYUVAEAlpha_po_start[];
extern "C" const int _binary_PS3_FYUVAEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FYUVAEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FYUVACxform_po_start[];
extern "C" const int _binary_PS3_FYUVACxformMul_po_start[];
extern "C" const int _binary_PS3_FYUVACxformInv_po_start[];
extern "C" const int _binary_PS3_FYUVACxformEAlpha_po_start[];
extern "C" const int _binary_PS3_FYUVACxformEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FYUVACxformEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FYUVACxformAc_po_start[];
extern "C" const int _binary_PS3_FYUVACxformAcMul_po_start[];
extern "C" const int _binary_PS3_FYUVACxformAcInv_po_start[];
extern "C" const int _binary_PS3_FYUVACxformAcEAlpha_po_start[];
extern "C" const int _binary_PS3_FYUVACxformAcEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FYUVACxformAcEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FYUV_po_start[];
extern "C" const int _binary_PS3_FYUVMul_po_start[];
extern "C" const int _binary_PS3_FYUVEAlpha_po_start[];
extern "C" const int _binary_PS3_FYUVEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FYUVCxform_po_start[];
extern "C" const int _binary_PS3_FYUVCxformMul_po_start[];
extern "C" const int _binary_PS3_FYUVCxformEAlpha_po_start[];
extern "C" const int _binary_PS3_FYUVCxformEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FYUVCxformAc_po_start[];
extern "C" const int _binary_PS3_FYUVCxformAcMul_po_start[];
extern "C" const int _binary_PS3_FYUVCxformAcEAlpha_po_start[];
extern "C" const int _binary_PS3_FYUVCxformAcEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FYUVA_po_start[];
extern "C" const int _binary_PS3_FYUVAMul_po_start[];
extern "C" const int _binary_PS3_FYUVAInv_po_start[];
extern "C" const int _binary_PS3_FYUVAEAlpha_po_start[];
extern "C" const int _binary_PS3_FYUVAEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FYUVAEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FYUVACxform_po_start[];
extern "C" const int _binary_PS3_FYUVACxformMul_po_start[];
extern "C" const int _binary_PS3_FYUVACxformInv_po_start[];
extern "C" const int _binary_PS3_FYUVACxformEAlpha_po_start[];
extern "C" const int _binary_PS3_FYUVACxformEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FYUVACxformEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FYUVACxformAc_po_start[];
extern "C" const int _binary_PS3_FYUVACxformAcMul_po_start[];
extern "C" const int _binary_PS3_FYUVACxformAcInv_po_start[];
extern "C" const int _binary_PS3_FYUVACxformAcEAlpha_po_start[];
extern "C" const int _binary_PS3_FYUVACxformAcEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FYUVACxformAcEAlphaInv_po_start[];
extern "C" const int _binary_PS3_FTexTGCMatrixAc_po_start[];
extern "C" const int _binary_PS3_FTexTGCMatrixAcMul_po_start[];
extern "C" const int _binary_PS3_FTexTGCMatrixAcEAlpha_po_start[];
extern "C" const int _binary_PS3_FTexTGCMatrixAcEAlphaMul_po_start[];
extern "C" const int _binary_PS3_FBox1Blur_po_start[];
extern "C" const int _binary_PS3_FBox1BlurMul_po_start[];
extern "C" const int _binary_PS3_FBox2Blur_po_start[];
extern "C" const int _binary_PS3_FBox2BlurMul_po_start[];
extern "C" const int _binary_PS3_FBox2Shadow_po_start[];
extern "C" const int _binary_PS3_FBox2ShadowMul_po_start[];
extern "C" const int _binary_PS3_FBox2ShadowKnockout_po_start[];
extern "C" const int _binary_PS3_FBox2ShadowKnockoutMul_po_start[];
extern "C" const int _binary_PS3_FBox2InnerShadow_po_start[];
extern "C" const int _binary_PS3_FBox2InnerShadowMul_po_start[];
extern "C" const int _binary_PS3_FBox2InnerShadowKnockout_po_start[];
extern "C" const int _binary_PS3_FBox2InnerShadowKnockoutMul_po_start[];
extern "C" const int _binary_PS3_FBox2Shadowonly_po_start[];
extern "C" const int _binary_PS3_FBox2ShadowonlyMul_po_start[];
extern "C" const int _binary_PS3_FBox2ShadowHighlight_po_start[];
extern "C" const int _binary_PS3_FBox2ShadowHighlightMul_po_start[];
extern "C" const int _binary_PS3_FBox2ShadowHighlightKnockout_po_start[];
extern "C" const int _binary_PS3_FBox2ShadowHighlightKnockoutMul_po_start[];
extern "C" const int _binary_PS3_FBox2InnerShadowHighlight_po_start[];
extern "C" const int _binary_PS3_FBox2InnerShadowHighlightMul_po_start[];
extern "C" const int _binary_PS3_FBox2InnerShadowHighlightKnockout_po_start[];
extern "C" const int _binary_PS3_FBox2InnerShadowHighlightKnockoutMul_po_start[];
extern "C" const int _binary_PS3_FBox2FullShadowHighlight_po_start[];
extern "C" const int _binary_PS3_FBox2FullShadowHighlightMul_po_start[];
extern "C" const int _binary_PS3_FBox2ShadowonlyHighlight_po_start[];
extern "C" const int _binary_PS3_FBox2ShadowonlyHighlightMul_po_start[];
extern "C" const int _binary_PS3_FDrawableCopyPixels_po_start[];
extern "C" const int _binary_PS3_FDrawableCopyPixelsMergeAlpha_po_start[];
extern "C" const int _binary_PS3_FDrawableCopyPixelsNoDestAlpha_po_start[];
extern "C" const int _binary_PS3_FDrawableCopyPixelsAlpha_po_start[];
extern "C" const int _binary_PS3_FDrawableCopyPixelsAlphaMergeAlpha_po_start[];
extern "C" const int _binary_PS3_FDrawableCopyPixelsAlphaNoDestAlpha_po_start[];
extern "C" const int _binary_PS3_FDrawableMerge_po_start[];
extern "C" const int _binary_PS3_FDrawableCompare_po_start[];
extern "C" const int _binary_PS3_FDrawablePaletteMap_po_start[];

UniformVar Uniforms_2381729308[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        {0, 1032, 1, 1, 1, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_2601942360[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_6280085138[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        {0, 1032, 1, 2, 1, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_3135047186[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      {0, 1032, 4, 4, 4, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_2732175232[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        {0, 1032, 1, 3, 1, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_2565513755[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        {0, 1032, 1, 4, 1, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_5271643974[Uniform::SU_Count] = 
{
    /* cxadd*/      {0, 1032, 4, 4, 4, 0 },
    /* cxmul*/      {1, 1036, 16, 16, 16, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        {5, 1052, 1, 1, 1, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_3499811502[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      {0, 1032, 4, 4, 4, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        {1, 1036, 1, 1, 1, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   {2, 1040, 4, 4, 4, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_3194986030[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      {0, 1032, 4, 4, 4, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     {1, 1036, 4, 4, 4, 0 },
    /* scolor*/     {2, 1040, 4, 4, 4, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     {3, 1044, 1, 1, 1, 0 },
    /* srctexscale*/{4, 1048, 4, 4, 4, 0 },
    /* tex*/        {5, 1052, 1, 1, 1, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   {6, 1056, 4, 4, 4, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_3141144559[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      {0, 1032, 4, 4, 4, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     {1, 1036, 4, 4, 4, 0 },
    /* scolor*/     {2, 1040, 4, 4, 4, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        {3, 1044, 1, 1, 1, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   {4, 1048, 4, 4, 4, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_6199549546[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      {0, 1032, 4, 4, 4, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     {1, 1036, 4, 4, 4, 0 },
    /* scolor*/     {2, 1040, 4, 4, 4, 0 },
    /* scolor2*/    {3, 1044, 4, 4, 4, 0 },
    /* srctex*/     {4, 1048, 1, 1, 1, 0 },
    /* srctexscale*/{5, 1052, 4, 4, 4, 0 },
    /* tex*/        {6, 1056, 1, 1, 1, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   {7, 1060, 4, 4, 4, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_3929338965[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      {0, 1032, 4, 4, 4, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     {1, 1036, 4, 4, 4, 0 },
    /* scolor*/     {2, 1040, 4, 4, 4, 0 },
    /* scolor2*/    {3, 1044, 4, 4, 4, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        {4, 1048, 1, 1, 1, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   {5, 1052, 4, 4, 4, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_2456628487[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      {0, 1032, 16, 16, 16, 0 },
    /* cxmul1*/     {4, 1048, 16, 16, 16, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        {8, 1064, 1, 2, 1, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_2486161926[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     {0, 1032, 1, 1, 1, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        {1, 1036, 1, 1, 1, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTG = 
{
    /* Type */          ShaderDesc::ST_TexTG,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTG_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGMul = 
{
    /* Type */          ShaderDesc::ST_TexTGMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGInv = 
{
    /* Type */          ShaderDesc::ST_TexTGInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGEAlpha_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_TexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGCxform_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGCxformMul = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGCxformMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGCxformInv = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGCxformInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGCxformEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGCxformEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGCxformAc = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGCxformAc_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGCxformAcMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGCxformAcInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGCxformAcEAlpha_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGCxformAcEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGCxformAcEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FVertex = 
{
    /* Type */          ShaderDesc::ST_Vertex,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FVertex_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FVertexMul = 
{
    /* Type */          ShaderDesc::ST_VertexMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FVertexMul_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FVertexInv = 
{
    /* Type */          ShaderDesc::ST_VertexInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FVertexInv_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_VertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FVertexEAlpha_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FVertexEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_VertexEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FVertexEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FVertexEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_VertexEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FVertexEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FVertexCxform = 
{
    /* Type */          ShaderDesc::ST_VertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FVertexCxform_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FVertexCxformMul = 
{
    /* Type */          ShaderDesc::ST_VertexCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FVertexCxformMul_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FVertexCxformInv = 
{
    /* Type */          ShaderDesc::ST_VertexCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FVertexCxformInv_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_VertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FVertexCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FVertexCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_VertexCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FVertexCxformEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FVertexCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_VertexCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FVertexCxformEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FVertexCxformAc = 
{
    /* Type */          ShaderDesc::ST_VertexCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FVertexCxformAc_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FVertexCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_VertexCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FVertexCxformAcMul_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FVertexCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_VertexCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FVertexCxformAcInv_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FVertexCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_VertexCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FVertexCxformAcEAlpha_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FVertexCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_VertexCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FVertexCxformAcEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FVertexCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_VertexCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FVertexCxformAcEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGTexTG = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTG,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGTexTG_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGTexTGMul = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGTexTGMul_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGTexTGInv = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGTexTGInv_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGTexTGEAlpha_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGTexTGEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGTexTGEAlphaMul_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGTexTGEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGTexTGEAlphaInv_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxform_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGTexTGCxformMul = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformMul_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGTexTGCxformInv = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformInv_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGTexTGCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformEAlphaMul_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGTexTGCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformEAlphaInv_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGTexTGCxformAc = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformAc_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGTexTGCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformAcMul_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGTexTGCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformAcInv_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGTexTGCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformAcEAlpha_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGTexTGCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformAcEAlphaMul_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGTexTGCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformAcEAlphaInv_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGVertex = 
{
    /* Type */          ShaderDesc::ST_TexTGVertex,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGVertex_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGVertexMul = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGVertexMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGVertexInv = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGVertexInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGVertexEAlpha_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGVertexEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGVertexEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGVertexEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGVertexEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGVertexCxform = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGVertexCxform_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGVertexCxformMul = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGVertexCxformInv = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGVertexCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGVertexCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGVertexCxformAc = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformAc_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGVertexCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformAcMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGVertexCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformAcInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGVertexCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformAcEAlpha_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGVertexCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformAcEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGVertexCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformAcEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTG = 
{
    /* Type */          ShaderDesc::ST_BatchTexTG,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTG_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGEAlpha_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGCxform_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGCxformMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGCxformMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGCxformInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGCxformInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGCxformEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGCxformEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGCxformAc = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGCxformAc_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGCxformAcMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGCxformAcInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGCxformAcEAlpha_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGCxformAcEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGCxformAcEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchVertex = 
{
    /* Type */          ShaderDesc::ST_BatchVertex,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FVertex_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchVertexMul = 
{
    /* Type */          ShaderDesc::ST_BatchVertexMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FVertexMul_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchVertexInv = 
{
    /* Type */          ShaderDesc::ST_BatchVertexInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FVertexInv_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FVertexEAlpha_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchVertexEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchVertexEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FVertexEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchVertexEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchVertexEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FVertexEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchVertexCxform = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FVertexCxform_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchVertexCxformMul = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FVertexCxformMul_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchVertexCxformInv = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FVertexCxformInv_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FVertexCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchVertexCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FVertexCxformEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchVertexCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FVertexCxformEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchVertexCxformAc = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FVertexCxformAc_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchVertexCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FVertexCxformAcMul_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchVertexCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FVertexCxformAcInv_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchVertexCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FVertexCxformAcEAlpha_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchVertexCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FVertexCxformAcEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchVertexCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FVertexCxformAcEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGTexTG = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTG,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGTexTG_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGTexTGMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGTexTGMul_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGTexTGInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGTexTGInv_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGTexTGEAlpha_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGTexTGEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGTexTGEAlphaMul_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGTexTGEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGTexTGEAlphaInv_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxform_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGTexTGCxformMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformMul_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGTexTGCxformInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformInv_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGTexTGCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformEAlphaMul_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGTexTGCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformEAlphaInv_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGTexTGCxformAc = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformAc_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGTexTGCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformAcMul_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGTexTGCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformAcInv_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGTexTGCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformAcEAlpha_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGTexTGCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformAcEAlphaMul_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGTexTGCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformAcEAlphaInv_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGVertex = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertex,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGVertex_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGVertexMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGVertexMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGVertexInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGVertexInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGVertexEAlpha_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGVertexEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGVertexEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGVertexEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGVertexEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGVertexCxform = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGVertexCxform_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGVertexCxformMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGVertexCxformInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGVertexCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGVertexCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGVertexCxformAc = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformAc_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGVertexCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformAcMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGVertexCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformAcInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGVertexCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformAcEAlpha_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGVertexCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformAcEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTexTGVertexCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformAcEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTG = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTG,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTG_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGEAlpha_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGCxform_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGCxformMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGCxformMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGCxformInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGCxformInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGCxformEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGCxformEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGCxformAc = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGCxformAc_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGCxformAcMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGCxformAcInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGCxformAcEAlpha_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGCxformAcEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGCxformAcEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedVertex = 
{
    /* Type */          ShaderDesc::ST_InstancedVertex,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FVertex_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedVertexMul = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FVertexMul_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedVertexInv = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FVertexInv_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FVertexEAlpha_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedVertexEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FVertexEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedVertexEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FVertexEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedVertexCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FVertexCxform_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedVertexCxformMul = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FVertexCxformMul_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedVertexCxformInv = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FVertexCxformInv_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FVertexCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedVertexCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FVertexCxformEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedVertexCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FVertexCxformEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedVertexCxformAc = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FVertexCxformAc_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedVertexCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FVertexCxformAcMul_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedVertexCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FVertexCxformAcInv_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedVertexCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FVertexCxformAcEAlpha_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedVertexCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FVertexCxformAcEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedVertexCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FVertexCxformAcEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGTexTG = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTG,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGTexTG_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGTexTGMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGTexTGMul_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGTexTGInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGTexTGInv_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGTexTGEAlpha_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGTexTGEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGTexTGEAlphaMul_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGTexTGEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGTexTGEAlphaInv_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxform_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGTexTGCxformMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformMul_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGTexTGCxformInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformInv_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGTexTGCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformEAlphaMul_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGTexTGCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformEAlphaInv_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGTexTGCxformAc = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformAc_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGTexTGCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformAcMul_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGTexTGCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformAcInv_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGTexTGCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformAcEAlpha_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGTexTGCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformAcEAlphaMul_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGTexTGCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGTexTGCxformAcEAlphaInv_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGVertex = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertex,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGVertex_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGVertexMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGVertexMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGVertexInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGVertexInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGVertexEAlpha_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGVertexEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGVertexEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGVertexEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGVertexEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGVertexCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGVertexCxform_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGVertexCxformMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGVertexCxformInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGVertexCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGVertexCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGVertexCxformAc = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformAc_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGVertexCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformAcMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGVertexCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformAcInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGVertexCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformAcEAlpha_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGVertexCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformAcEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTexTGVertexCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTexTGVertexCxformAcEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FSolid = 
{
    /* Type */          ShaderDesc::ST_Solid,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FSolid_po_start,
    /* Uniforms */      Uniforms_3135047186,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FSolidMul = 
{
    /* Type */          ShaderDesc::ST_SolidMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FSolidMul_po_start,
    /* Uniforms */      Uniforms_3135047186,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FSolidInv = 
{
    /* Type */          ShaderDesc::ST_SolidInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FSolidInv_po_start,
    /* Uniforms */      Uniforms_3135047186,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FText = 
{
    /* Type */          ShaderDesc::ST_Text,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FText_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTextMul = 
{
    /* Type */          ShaderDesc::ST_TextMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTextMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTextInv = 
{
    /* Type */          ShaderDesc::ST_TextInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTextInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchSolid = 
{
    /* Type */          ShaderDesc::ST_BatchSolid,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FSolid_po_start,
    /* Uniforms */      Uniforms_3135047186,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchSolidMul = 
{
    /* Type */          ShaderDesc::ST_BatchSolidMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FSolidMul_po_start,
    /* Uniforms */      Uniforms_3135047186,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchSolidInv = 
{
    /* Type */          ShaderDesc::ST_BatchSolidInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FSolidInv_po_start,
    /* Uniforms */      Uniforms_3135047186,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchText = 
{
    /* Type */          ShaderDesc::ST_BatchText,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FText_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTextMul = 
{
    /* Type */          ShaderDesc::ST_BatchTextMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTextMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchTextInv = 
{
    /* Type */          ShaderDesc::ST_BatchTextInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FTextInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedSolid = 
{
    /* Type */          ShaderDesc::ST_InstancedSolid,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FSolid_po_start,
    /* Uniforms */      Uniforms_3135047186,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedSolidMul = 
{
    /* Type */          ShaderDesc::ST_InstancedSolidMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FSolidMul_po_start,
    /* Uniforms */      Uniforms_3135047186,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedSolidInv = 
{
    /* Type */          ShaderDesc::ST_InstancedSolidInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FSolidInv_po_start,
    /* Uniforms */      Uniforms_3135047186,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedText = 
{
    /* Type */          ShaderDesc::ST_InstancedText,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FText_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTextMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTextMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTextMul_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedTextInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTextInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FTextInv_po_start,
    /* Uniforms */      Uniforms_2381729308,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUV = 
{
    /* Type */          ShaderDesc::ST_YUV,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUV_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVMul = 
{
    /* Type */          ShaderDesc::ST_YUVMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVMul_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVEAlpha = 
{
    /* Type */          ShaderDesc::ST_YUVEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVEAlpha_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_YUVEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVCxform = 
{
    /* Type */          ShaderDesc::ST_YUVCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVCxform_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVCxformMul = 
{
    /* Type */          ShaderDesc::ST_YUVCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVCxformMul_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_YUVCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_YUVCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVCxformEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVCxformAc = 
{
    /* Type */          ShaderDesc::ST_YUVCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVCxformAc_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_YUVCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVCxformAcMul_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_YUVCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVCxformAcEAlpha_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_YUVCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVCxformAcEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVA = 
{
    /* Type */          ShaderDesc::ST_YUVA,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVA_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVAMul = 
{
    /* Type */          ShaderDesc::ST_YUVAMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVAMul_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVAInv = 
{
    /* Type */          ShaderDesc::ST_YUVAInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVAInv_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVAEAlpha = 
{
    /* Type */          ShaderDesc::ST_YUVAEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVAEAlpha_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVAEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_YUVAEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVAEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVAEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_YUVAEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVAEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVACxform = 
{
    /* Type */          ShaderDesc::ST_YUVACxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVACxform_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVACxformMul = 
{
    /* Type */          ShaderDesc::ST_YUVACxformMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVACxformMul_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVACxformInv = 
{
    /* Type */          ShaderDesc::ST_YUVACxformInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVACxformInv_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVACxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_YUVACxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVACxformEAlpha_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVACxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_YUVACxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVACxformEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVACxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_YUVACxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVACxformEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVACxformAc = 
{
    /* Type */          ShaderDesc::ST_YUVACxformAc,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVACxformAc_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVACxformAcMul = 
{
    /* Type */          ShaderDesc::ST_YUVACxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVACxformAcMul_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVACxformAcInv = 
{
    /* Type */          ShaderDesc::ST_YUVACxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVACxformAcInv_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVACxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_YUVACxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVACxformAcEAlpha_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVACxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_YUVACxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVACxformAcEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FYUVACxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_YUVACxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FYUVACxformAcEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUV = 
{
    /* Type */          ShaderDesc::ST_BatchYUV,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUV_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVMul_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchYUVEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVEAlpha_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVCxform = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVCxform_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVCxformMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVCxformMul_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVCxformEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVCxformAc = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVCxformAc_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVCxformAcMul_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVCxformAcEAlpha_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVCxformAcEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVA = 
{
    /* Type */          ShaderDesc::ST_BatchYUVA,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVA_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVAMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVAMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVAMul_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVAInv = 
{
    /* Type */          ShaderDesc::ST_BatchYUVAInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVAInv_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVAEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchYUVAEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVAEAlpha_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVAEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVAEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVAEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVAEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchYUVAEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVAEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVACxform = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVACxform_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVACxformMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVACxformMul_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVACxformInv = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVACxformInv_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVACxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVACxformEAlpha_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVACxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVACxformEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVACxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVACxformEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVACxformAc = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformAc,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVACxformAc_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVACxformAcMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVACxformAcMul_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVACxformAcInv = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVACxformAcInv_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVACxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVACxformAcEAlpha_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVACxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVACxformAcEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBatchYUVACxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Batch,
    /* pBinary */       _binary_PS3_FYUVACxformAcEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUV = 
{
    /* Type */          ShaderDesc::ST_InstancedYUV,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUV_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVMul = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVMul_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVEAlpha_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVCxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVCxform_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVCxformMul = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVCxformMul_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVCxformEAlpha_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVCxformEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVCxformAc = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVCxformAc_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVCxformAcMul_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVCxformAcEAlpha_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVCxformAcEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVA = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVA,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVA_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVAMul = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVAMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVAMul_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVAInv = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVAInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVAInv_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVAEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVAEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVAEAlpha_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVAEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVAEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVAEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVAEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVAEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVAEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVACxform = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVACxform,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVACxform_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVACxformMul = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVACxformMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVACxformMul_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVACxformInv = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVACxformInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVACxformInv_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVACxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVACxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVACxformEAlpha_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVACxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVACxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVACxformEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVACxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVACxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVACxformEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVACxformAc = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVACxformAc,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVACxformAc_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVACxformAcMul = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVACxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVACxformAcMul_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVACxformAcInv = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVACxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVACxformAcInv_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVACxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVACxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVACxformAcEAlpha_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVACxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVACxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVACxformAcEAlphaMul_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FInstancedYUVACxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVACxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_Instanced,
    /* pBinary */       _binary_PS3_FYUVACxformAcEAlphaInv_po_start,
    /* Uniforms */      Uniforms_2565513755,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGCMatrixAc = 
{
    /* Type */          ShaderDesc::ST_TexTGCMatrixAc,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGCMatrixAc_po_start,
    /* Uniforms */      Uniforms_5271643974,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGCMatrixAcMul = 
{
    /* Type */          ShaderDesc::ST_TexTGCMatrixAcMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGCMatrixAcMul_po_start,
    /* Uniforms */      Uniforms_5271643974,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGCMatrixAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGCMatrixAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGCMatrixAcEAlpha_po_start,
    /* Uniforms */      Uniforms_5271643974,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FTexTGCMatrixAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGCMatrixAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FTexTGCMatrixAcEAlphaMul_po_start,
    /* Uniforms */      Uniforms_5271643974,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox1Blur = 
{
    /* Type */          ShaderDesc::ST_Box1Blur,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox1Blur_po_start,
    /* Uniforms */      Uniforms_3499811502,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox1BlurMul = 
{
    /* Type */          ShaderDesc::ST_Box1BlurMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox1BlurMul_po_start,
    /* Uniforms */      Uniforms_3499811502,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox2Blur = 
{
    /* Type */          ShaderDesc::ST_Box2Blur,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox2Blur_po_start,
    /* Uniforms */      Uniforms_3499811502,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox2BlurMul = 
{
    /* Type */          ShaderDesc::ST_Box2BlurMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox2BlurMul_po_start,
    /* Uniforms */      Uniforms_3499811502,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox2Shadow = 
{
    /* Type */          ShaderDesc::ST_Box2Shadow,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox2Shadow_po_start,
    /* Uniforms */      Uniforms_3194986030,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox2ShadowMul = 
{
    /* Type */          ShaderDesc::ST_Box2ShadowMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox2ShadowMul_po_start,
    /* Uniforms */      Uniforms_3194986030,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox2ShadowKnockout = 
{
    /* Type */          ShaderDesc::ST_Box2ShadowKnockout,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox2ShadowKnockout_po_start,
    /* Uniforms */      Uniforms_3194986030,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox2ShadowKnockoutMul = 
{
    /* Type */          ShaderDesc::ST_Box2ShadowKnockoutMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox2ShadowKnockoutMul_po_start,
    /* Uniforms */      Uniforms_3194986030,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox2InnerShadow = 
{
    /* Type */          ShaderDesc::ST_Box2InnerShadow,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox2InnerShadow_po_start,
    /* Uniforms */      Uniforms_3194986030,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox2InnerShadowMul = 
{
    /* Type */          ShaderDesc::ST_Box2InnerShadowMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox2InnerShadowMul_po_start,
    /* Uniforms */      Uniforms_3194986030,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox2InnerShadowKnockout = 
{
    /* Type */          ShaderDesc::ST_Box2InnerShadowKnockout,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox2InnerShadowKnockout_po_start,
    /* Uniforms */      Uniforms_3194986030,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox2InnerShadowKnockoutMul = 
{
    /* Type */          ShaderDesc::ST_Box2InnerShadowKnockoutMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox2InnerShadowKnockoutMul_po_start,
    /* Uniforms */      Uniforms_3194986030,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox2Shadowonly = 
{
    /* Type */          ShaderDesc::ST_Box2Shadowonly,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox2Shadowonly_po_start,
    /* Uniforms */      Uniforms_3141144559,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox2ShadowonlyMul = 
{
    /* Type */          ShaderDesc::ST_Box2ShadowonlyMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox2ShadowonlyMul_po_start,
    /* Uniforms */      Uniforms_3141144559,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox2ShadowHighlight = 
{
    /* Type */          ShaderDesc::ST_Box2ShadowHighlight,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox2ShadowHighlight_po_start,
    /* Uniforms */      Uniforms_6199549546,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox2ShadowHighlightMul = 
{
    /* Type */          ShaderDesc::ST_Box2ShadowHighlightMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox2ShadowHighlightMul_po_start,
    /* Uniforms */      Uniforms_6199549546,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox2ShadowHighlightKnockout = 
{
    /* Type */          ShaderDesc::ST_Box2ShadowHighlightKnockout,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox2ShadowHighlightKnockout_po_start,
    /* Uniforms */      Uniforms_6199549546,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox2ShadowHighlightKnockoutMul = 
{
    /* Type */          ShaderDesc::ST_Box2ShadowHighlightKnockoutMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox2ShadowHighlightKnockoutMul_po_start,
    /* Uniforms */      Uniforms_6199549546,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox2InnerShadowHighlight = 
{
    /* Type */          ShaderDesc::ST_Box2InnerShadowHighlight,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox2InnerShadowHighlight_po_start,
    /* Uniforms */      Uniforms_6199549546,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox2InnerShadowHighlightMul = 
{
    /* Type */          ShaderDesc::ST_Box2InnerShadowHighlightMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox2InnerShadowHighlightMul_po_start,
    /* Uniforms */      Uniforms_6199549546,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox2InnerShadowHighlightKnockout = 
{
    /* Type */          ShaderDesc::ST_Box2InnerShadowHighlightKnockout,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox2InnerShadowHighlightKnockout_po_start,
    /* Uniforms */      Uniforms_6199549546,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox2InnerShadowHighlightKnockoutMul = 
{
    /* Type */          ShaderDesc::ST_Box2InnerShadowHighlightKnockoutMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox2InnerShadowHighlightKnockoutMul_po_start,
    /* Uniforms */      Uniforms_6199549546,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox2FullShadowHighlight = 
{
    /* Type */          ShaderDesc::ST_Box2FullShadowHighlight,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox2FullShadowHighlight_po_start,
    /* Uniforms */      Uniforms_6199549546,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox2FullShadowHighlightMul = 
{
    /* Type */          ShaderDesc::ST_Box2FullShadowHighlightMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox2FullShadowHighlightMul_po_start,
    /* Uniforms */      Uniforms_6199549546,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox2ShadowonlyHighlight = 
{
    /* Type */          ShaderDesc::ST_Box2ShadowonlyHighlight,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox2ShadowonlyHighlight_po_start,
    /* Uniforms */      Uniforms_3929338965,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FBox2ShadowonlyHighlightMul = 
{
    /* Type */          ShaderDesc::ST_Box2ShadowonlyHighlightMul,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       _binary_PS3_FBox2ShadowonlyHighlightMul_po_start,
    /* Uniforms */      Uniforms_3929338965,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FDrawableCopyPixels = 
{
    /* Type */          ShaderDesc::ST_DrawableCopyPixels,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FDrawableCopyPixels_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FDrawableCopyPixelsMergeAlpha = 
{
    /* Type */          ShaderDesc::ST_DrawableCopyPixelsMergeAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FDrawableCopyPixelsMergeAlpha_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FDrawableCopyPixelsNoDestAlpha = 
{
    /* Type */          ShaderDesc::ST_DrawableCopyPixelsNoDestAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FDrawableCopyPixelsNoDestAlpha_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FDrawableCopyPixelsAlpha = 
{
    /* Type */          ShaderDesc::ST_DrawableCopyPixelsAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FDrawableCopyPixelsAlpha_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FDrawableCopyPixelsAlphaMergeAlpha = 
{
    /* Type */          ShaderDesc::ST_DrawableCopyPixelsAlphaMergeAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FDrawableCopyPixelsAlphaMergeAlpha_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FDrawableCopyPixelsAlphaNoDestAlpha = 
{
    /* Type */          ShaderDesc::ST_DrawableCopyPixelsAlphaNoDestAlpha,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FDrawableCopyPixelsAlphaNoDestAlpha_po_start,
    /* Uniforms */      Uniforms_2732175232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FDrawableMerge = 
{
    /* Type */          ShaderDesc::ST_DrawableMerge,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FDrawableMerge_po_start,
    /* Uniforms */      Uniforms_2456628487,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FDrawableCompare = 
{
    /* Type */          ShaderDesc::ST_DrawableCompare,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FDrawableCompare_po_start,
    /* Uniforms */      Uniforms_6280085138,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_PS3_FDrawablePaletteMap = 
{
    /* Type */          ShaderDesc::ST_DrawablePaletteMap,
    /* Version */       ShaderDesc::ShaderVersion_PS3,
    /* Flags */         0,
    /* pBinary */       _binary_PS3_FDrawablePaletteMap_po_start,
    /* Uniforms */      Uniforms_2486161926,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

const FragShaderDesc* FragShaderDesc::Descs[FSI_Count] = {
    0,
    &ShaderDesc_FS_PS3_FTexTG,                                   // 1
    &ShaderDesc_FS_PS3_FTexTGMul,                                // 2
    &ShaderDesc_FS_PS3_FTexTGInv,                                // 3
    &ShaderDesc_FS_PS3_FTexTGEAlpha,                             // 4
    &ShaderDesc_FS_PS3_FTexTGEAlphaMul,                          // 5
    &ShaderDesc_FS_PS3_FTexTGEAlphaInv,                          // 6
    &ShaderDesc_FS_PS3_FTexTGCxform,                             // 7
    &ShaderDesc_FS_PS3_FTexTGCxformMul,                          // 8
    &ShaderDesc_FS_PS3_FTexTGCxformInv,                          // 9
    &ShaderDesc_FS_PS3_FTexTGCxformEAlpha,                       // 10
    &ShaderDesc_FS_PS3_FTexTGCxformEAlphaMul,                    // 11
    &ShaderDesc_FS_PS3_FTexTGCxformEAlphaInv,                    // 12
    &ShaderDesc_FS_PS3_FTexTGCxformAc,                           // 13
    &ShaderDesc_FS_PS3_FTexTGCxformAcMul,                        // 14
    &ShaderDesc_FS_PS3_FTexTGCxformAcInv,                        // 15
    &ShaderDesc_FS_PS3_FTexTGCxformAcEAlpha,                     // 16
    &ShaderDesc_FS_PS3_FTexTGCxformAcEAlphaMul,                  // 17
    &ShaderDesc_FS_PS3_FTexTGCxformAcEAlphaInv,                  // 18
    &ShaderDesc_FS_PS3_FVertex,                                  // 19
    &ShaderDesc_FS_PS3_FVertexMul,                               // 20
    &ShaderDesc_FS_PS3_FVertexInv,                               // 21
    &ShaderDesc_FS_PS3_FVertexEAlpha,                            // 22
    &ShaderDesc_FS_PS3_FVertexEAlphaMul,                         // 23
    &ShaderDesc_FS_PS3_FVertexEAlphaInv,                         // 24
    &ShaderDesc_FS_PS3_FVertexCxform,                            // 25
    &ShaderDesc_FS_PS3_FVertexCxformMul,                         // 26
    &ShaderDesc_FS_PS3_FVertexCxformInv,                         // 27
    &ShaderDesc_FS_PS3_FVertexCxformEAlpha,                      // 28
    &ShaderDesc_FS_PS3_FVertexCxformEAlphaMul,                   // 29
    &ShaderDesc_FS_PS3_FVertexCxformEAlphaInv,                   // 30
    &ShaderDesc_FS_PS3_FVertexCxformAc,                          // 31
    &ShaderDesc_FS_PS3_FVertexCxformAcMul,                       // 32
    &ShaderDesc_FS_PS3_FVertexCxformAcInv,                       // 33
    &ShaderDesc_FS_PS3_FVertexCxformAcEAlpha,                    // 34
    &ShaderDesc_FS_PS3_FVertexCxformAcEAlphaMul,                 // 35
    &ShaderDesc_FS_PS3_FVertexCxformAcEAlphaInv,                 // 36
    &ShaderDesc_FS_PS3_FTexTGTexTG,                              // 37
    &ShaderDesc_FS_PS3_FTexTGTexTGMul,                           // 38
    &ShaderDesc_FS_PS3_FTexTGTexTGInv,                           // 39
    &ShaderDesc_FS_PS3_FTexTGTexTGEAlpha,                        // 40
    &ShaderDesc_FS_PS3_FTexTGTexTGEAlphaMul,                     // 41
    &ShaderDesc_FS_PS3_FTexTGTexTGEAlphaInv,                     // 42
    &ShaderDesc_FS_PS3_FTexTGTexTGCxform,                        // 43
    &ShaderDesc_FS_PS3_FTexTGTexTGCxformMul,                     // 44
    &ShaderDesc_FS_PS3_FTexTGTexTGCxformInv,                     // 45
    &ShaderDesc_FS_PS3_FTexTGTexTGCxformEAlpha,                  // 46
    &ShaderDesc_FS_PS3_FTexTGTexTGCxformEAlphaMul,               // 47
    &ShaderDesc_FS_PS3_FTexTGTexTGCxformEAlphaInv,               // 48
    &ShaderDesc_FS_PS3_FTexTGTexTGCxformAc,                      // 49
    &ShaderDesc_FS_PS3_FTexTGTexTGCxformAcMul,                   // 50
    &ShaderDesc_FS_PS3_FTexTGTexTGCxformAcInv,                   // 51
    &ShaderDesc_FS_PS3_FTexTGTexTGCxformAcEAlpha,                // 52
    &ShaderDesc_FS_PS3_FTexTGTexTGCxformAcEAlphaMul,             // 53
    &ShaderDesc_FS_PS3_FTexTGTexTGCxformAcEAlphaInv,             // 54
    &ShaderDesc_FS_PS3_FTexTGVertex,                             // 55
    &ShaderDesc_FS_PS3_FTexTGVertexMul,                          // 56
    &ShaderDesc_FS_PS3_FTexTGVertexInv,                          // 57
    &ShaderDesc_FS_PS3_FTexTGVertexEAlpha,                       // 58
    &ShaderDesc_FS_PS3_FTexTGVertexEAlphaMul,                    // 59
    &ShaderDesc_FS_PS3_FTexTGVertexEAlphaInv,                    // 60
    &ShaderDesc_FS_PS3_FTexTGVertexCxform,                       // 61
    &ShaderDesc_FS_PS3_FTexTGVertexCxformMul,                    // 62
    &ShaderDesc_FS_PS3_FTexTGVertexCxformInv,                    // 63
    &ShaderDesc_FS_PS3_FTexTGVertexCxformEAlpha,                 // 64
    &ShaderDesc_FS_PS3_FTexTGVertexCxformEAlphaMul,              // 65
    &ShaderDesc_FS_PS3_FTexTGVertexCxformEAlphaInv,              // 66
    &ShaderDesc_FS_PS3_FTexTGVertexCxformAc,                     // 67
    &ShaderDesc_FS_PS3_FTexTGVertexCxformAcMul,                  // 68
    &ShaderDesc_FS_PS3_FTexTGVertexCxformAcInv,                  // 69
    &ShaderDesc_FS_PS3_FTexTGVertexCxformAcEAlpha,               // 70
    &ShaderDesc_FS_PS3_FTexTGVertexCxformAcEAlphaMul,            // 71
    &ShaderDesc_FS_PS3_FTexTGVertexCxformAcEAlphaInv,            // 72
    &ShaderDesc_FS_PS3_FBatchTexTG,                              // 73
    &ShaderDesc_FS_PS3_FBatchTexTGMul,                           // 74
    &ShaderDesc_FS_PS3_FBatchTexTGInv,                           // 75
    &ShaderDesc_FS_PS3_FBatchTexTGEAlpha,                        // 76
    &ShaderDesc_FS_PS3_FBatchTexTGEAlphaMul,                     // 77
    &ShaderDesc_FS_PS3_FBatchTexTGEAlphaInv,                     // 78
    &ShaderDesc_FS_PS3_FBatchTexTGCxform,                        // 79
    &ShaderDesc_FS_PS3_FBatchTexTGCxformMul,                     // 80
    &ShaderDesc_FS_PS3_FBatchTexTGCxformInv,                     // 81
    &ShaderDesc_FS_PS3_FBatchTexTGCxformEAlpha,                  // 82
    &ShaderDesc_FS_PS3_FBatchTexTGCxformEAlphaMul,               // 83
    &ShaderDesc_FS_PS3_FBatchTexTGCxformEAlphaInv,               // 84
    &ShaderDesc_FS_PS3_FBatchTexTGCxformAc,                      // 85
    &ShaderDesc_FS_PS3_FBatchTexTGCxformAcMul,                   // 86
    &ShaderDesc_FS_PS3_FBatchTexTGCxformAcInv,                   // 87
    &ShaderDesc_FS_PS3_FBatchTexTGCxformAcEAlpha,                // 88
    &ShaderDesc_FS_PS3_FBatchTexTGCxformAcEAlphaMul,             // 89
    &ShaderDesc_FS_PS3_FBatchTexTGCxformAcEAlphaInv,             // 90
    &ShaderDesc_FS_PS3_FBatchVertex,                             // 91
    &ShaderDesc_FS_PS3_FBatchVertexMul,                          // 92
    &ShaderDesc_FS_PS3_FBatchVertexInv,                          // 93
    &ShaderDesc_FS_PS3_FBatchVertexEAlpha,                       // 94
    &ShaderDesc_FS_PS3_FBatchVertexEAlphaMul,                    // 95
    &ShaderDesc_FS_PS3_FBatchVertexEAlphaInv,                    // 96
    &ShaderDesc_FS_PS3_FBatchVertexCxform,                       // 97
    &ShaderDesc_FS_PS3_FBatchVertexCxformMul,                    // 98
    &ShaderDesc_FS_PS3_FBatchVertexCxformInv,                    // 99
    &ShaderDesc_FS_PS3_FBatchVertexCxformEAlpha,                 // 100
    &ShaderDesc_FS_PS3_FBatchVertexCxformEAlphaMul,              // 101
    &ShaderDesc_FS_PS3_FBatchVertexCxformEAlphaInv,              // 102
    &ShaderDesc_FS_PS3_FBatchVertexCxformAc,                     // 103
    &ShaderDesc_FS_PS3_FBatchVertexCxformAcMul,                  // 104
    &ShaderDesc_FS_PS3_FBatchVertexCxformAcInv,                  // 105
    &ShaderDesc_FS_PS3_FBatchVertexCxformAcEAlpha,               // 106
    &ShaderDesc_FS_PS3_FBatchVertexCxformAcEAlphaMul,            // 107
    &ShaderDesc_FS_PS3_FBatchVertexCxformAcEAlphaInv,            // 108
    &ShaderDesc_FS_PS3_FBatchTexTGTexTG,                         // 109
    &ShaderDesc_FS_PS3_FBatchTexTGTexTGMul,                      // 110
    &ShaderDesc_FS_PS3_FBatchTexTGTexTGInv,                      // 111
    &ShaderDesc_FS_PS3_FBatchTexTGTexTGEAlpha,                   // 112
    &ShaderDesc_FS_PS3_FBatchTexTGTexTGEAlphaMul,                // 113
    &ShaderDesc_FS_PS3_FBatchTexTGTexTGEAlphaInv,                // 114
    &ShaderDesc_FS_PS3_FBatchTexTGTexTGCxform,                   // 115
    &ShaderDesc_FS_PS3_FBatchTexTGTexTGCxformMul,                // 116
    &ShaderDesc_FS_PS3_FBatchTexTGTexTGCxformInv,                // 117
    &ShaderDesc_FS_PS3_FBatchTexTGTexTGCxformEAlpha,             // 118
    &ShaderDesc_FS_PS3_FBatchTexTGTexTGCxformEAlphaMul,          // 119
    &ShaderDesc_FS_PS3_FBatchTexTGTexTGCxformEAlphaInv,          // 120
    &ShaderDesc_FS_PS3_FBatchTexTGTexTGCxformAc,                 // 121
    &ShaderDesc_FS_PS3_FBatchTexTGTexTGCxformAcMul,              // 122
    &ShaderDesc_FS_PS3_FBatchTexTGTexTGCxformAcInv,              // 123
    &ShaderDesc_FS_PS3_FBatchTexTGTexTGCxformAcEAlpha,           // 124
    &ShaderDesc_FS_PS3_FBatchTexTGTexTGCxformAcEAlphaMul,        // 125
    &ShaderDesc_FS_PS3_FBatchTexTGTexTGCxformAcEAlphaInv,        // 126
    &ShaderDesc_FS_PS3_FBatchTexTGVertex,                        // 127
    &ShaderDesc_FS_PS3_FBatchTexTGVertexMul,                     // 128
    &ShaderDesc_FS_PS3_FBatchTexTGVertexInv,                     // 129
    &ShaderDesc_FS_PS3_FBatchTexTGVertexEAlpha,                  // 130
    &ShaderDesc_FS_PS3_FBatchTexTGVertexEAlphaMul,               // 131
    &ShaderDesc_FS_PS3_FBatchTexTGVertexEAlphaInv,               // 132
    &ShaderDesc_FS_PS3_FBatchTexTGVertexCxform,                  // 133
    &ShaderDesc_FS_PS3_FBatchTexTGVertexCxformMul,               // 134
    &ShaderDesc_FS_PS3_FBatchTexTGVertexCxformInv,               // 135
    &ShaderDesc_FS_PS3_FBatchTexTGVertexCxformEAlpha,            // 136
    &ShaderDesc_FS_PS3_FBatchTexTGVertexCxformEAlphaMul,         // 137
    &ShaderDesc_FS_PS3_FBatchTexTGVertexCxformEAlphaInv,         // 138
    &ShaderDesc_FS_PS3_FBatchTexTGVertexCxformAc,                // 139
    &ShaderDesc_FS_PS3_FBatchTexTGVertexCxformAcMul,             // 140
    &ShaderDesc_FS_PS3_FBatchTexTGVertexCxformAcInv,             // 141
    &ShaderDesc_FS_PS3_FBatchTexTGVertexCxformAcEAlpha,          // 142
    &ShaderDesc_FS_PS3_FBatchTexTGVertexCxformAcEAlphaMul,       // 143
    &ShaderDesc_FS_PS3_FBatchTexTGVertexCxformAcEAlphaInv,       // 144
    &ShaderDesc_FS_PS3_FInstancedTexTG,                          // 145
    &ShaderDesc_FS_PS3_FInstancedTexTGMul,                       // 146
    &ShaderDesc_FS_PS3_FInstancedTexTGInv,                       // 147
    &ShaderDesc_FS_PS3_FInstancedTexTGEAlpha,                    // 148
    &ShaderDesc_FS_PS3_FInstancedTexTGEAlphaMul,                 // 149
    &ShaderDesc_FS_PS3_FInstancedTexTGEAlphaInv,                 // 150
    &ShaderDesc_FS_PS3_FInstancedTexTGCxform,                    // 151
    &ShaderDesc_FS_PS3_FInstancedTexTGCxformMul,                 // 152
    &ShaderDesc_FS_PS3_FInstancedTexTGCxformInv,                 // 153
    &ShaderDesc_FS_PS3_FInstancedTexTGCxformEAlpha,              // 154
    &ShaderDesc_FS_PS3_FInstancedTexTGCxformEAlphaMul,           // 155
    &ShaderDesc_FS_PS3_FInstancedTexTGCxformEAlphaInv,           // 156
    &ShaderDesc_FS_PS3_FInstancedTexTGCxformAc,                  // 157
    &ShaderDesc_FS_PS3_FInstancedTexTGCxformAcMul,               // 158
    &ShaderDesc_FS_PS3_FInstancedTexTGCxformAcInv,               // 159
    &ShaderDesc_FS_PS3_FInstancedTexTGCxformAcEAlpha,            // 160
    &ShaderDesc_FS_PS3_FInstancedTexTGCxformAcEAlphaMul,         // 161
    &ShaderDesc_FS_PS3_FInstancedTexTGCxformAcEAlphaInv,         // 162
    &ShaderDesc_FS_PS3_FInstancedVertex,                         // 163
    &ShaderDesc_FS_PS3_FInstancedVertexMul,                      // 164
    &ShaderDesc_FS_PS3_FInstancedVertexInv,                      // 165
    &ShaderDesc_FS_PS3_FInstancedVertexEAlpha,                   // 166
    &ShaderDesc_FS_PS3_FInstancedVertexEAlphaMul,                // 167
    &ShaderDesc_FS_PS3_FInstancedVertexEAlphaInv,                // 168
    &ShaderDesc_FS_PS3_FInstancedVertexCxform,                   // 169
    &ShaderDesc_FS_PS3_FInstancedVertexCxformMul,                // 170
    &ShaderDesc_FS_PS3_FInstancedVertexCxformInv,                // 171
    &ShaderDesc_FS_PS3_FInstancedVertexCxformEAlpha,             // 172
    &ShaderDesc_FS_PS3_FInstancedVertexCxformEAlphaMul,          // 173
    &ShaderDesc_FS_PS3_FInstancedVertexCxformEAlphaInv,          // 174
    &ShaderDesc_FS_PS3_FInstancedVertexCxformAc,                 // 175
    &ShaderDesc_FS_PS3_FInstancedVertexCxformAcMul,              // 176
    &ShaderDesc_FS_PS3_FInstancedVertexCxformAcInv,              // 177
    &ShaderDesc_FS_PS3_FInstancedVertexCxformAcEAlpha,           // 178
    &ShaderDesc_FS_PS3_FInstancedVertexCxformAcEAlphaMul,        // 179
    &ShaderDesc_FS_PS3_FInstancedVertexCxformAcEAlphaInv,        // 180
    &ShaderDesc_FS_PS3_FInstancedTexTGTexTG,                     // 181
    &ShaderDesc_FS_PS3_FInstancedTexTGTexTGMul,                  // 182
    &ShaderDesc_FS_PS3_FInstancedTexTGTexTGInv,                  // 183
    &ShaderDesc_FS_PS3_FInstancedTexTGTexTGEAlpha,               // 184
    &ShaderDesc_FS_PS3_FInstancedTexTGTexTGEAlphaMul,            // 185
    &ShaderDesc_FS_PS3_FInstancedTexTGTexTGEAlphaInv,            // 186
    &ShaderDesc_FS_PS3_FInstancedTexTGTexTGCxform,               // 187
    &ShaderDesc_FS_PS3_FInstancedTexTGTexTGCxformMul,            // 188
    &ShaderDesc_FS_PS3_FInstancedTexTGTexTGCxformInv,            // 189
    &ShaderDesc_FS_PS3_FInstancedTexTGTexTGCxformEAlpha,         // 190
    &ShaderDesc_FS_PS3_FInstancedTexTGTexTGCxformEAlphaMul,      // 191
    &ShaderDesc_FS_PS3_FInstancedTexTGTexTGCxformEAlphaInv,      // 192
    &ShaderDesc_FS_PS3_FInstancedTexTGTexTGCxformAc,             // 193
    &ShaderDesc_FS_PS3_FInstancedTexTGTexTGCxformAcMul,          // 194
    &ShaderDesc_FS_PS3_FInstancedTexTGTexTGCxformAcInv,          // 195
    &ShaderDesc_FS_PS3_FInstancedTexTGTexTGCxformAcEAlpha,       // 196
    &ShaderDesc_FS_PS3_FInstancedTexTGTexTGCxformAcEAlphaMul,    // 197
    &ShaderDesc_FS_PS3_FInstancedTexTGTexTGCxformAcEAlphaInv,    // 198
    &ShaderDesc_FS_PS3_FInstancedTexTGVertex,                    // 199
    &ShaderDesc_FS_PS3_FInstancedTexTGVertexMul,                 // 200
    &ShaderDesc_FS_PS3_FInstancedTexTGVertexInv,                 // 201
    &ShaderDesc_FS_PS3_FInstancedTexTGVertexEAlpha,              // 202
    &ShaderDesc_FS_PS3_FInstancedTexTGVertexEAlphaMul,           // 203
    &ShaderDesc_FS_PS3_FInstancedTexTGVertexEAlphaInv,           // 204
    &ShaderDesc_FS_PS3_FInstancedTexTGVertexCxform,              // 205
    &ShaderDesc_FS_PS3_FInstancedTexTGVertexCxformMul,           // 206
    &ShaderDesc_FS_PS3_FInstancedTexTGVertexCxformInv,           // 207
    &ShaderDesc_FS_PS3_FInstancedTexTGVertexCxformEAlpha,        // 208
    &ShaderDesc_FS_PS3_FInstancedTexTGVertexCxformEAlphaMul,     // 209
    &ShaderDesc_FS_PS3_FInstancedTexTGVertexCxformEAlphaInv,     // 210
    &ShaderDesc_FS_PS3_FInstancedTexTGVertexCxformAc,            // 211
    &ShaderDesc_FS_PS3_FInstancedTexTGVertexCxformAcMul,         // 212
    &ShaderDesc_FS_PS3_FInstancedTexTGVertexCxformAcInv,         // 213
    &ShaderDesc_FS_PS3_FInstancedTexTGVertexCxformAcEAlpha,      // 214
    &ShaderDesc_FS_PS3_FInstancedTexTGVertexCxformAcEAlphaMul,   // 215
    &ShaderDesc_FS_PS3_FInstancedTexTGVertexCxformAcEAlphaInv,   // 216
    &ShaderDesc_FS_PS3_FSolid,                                   // 217
    &ShaderDesc_FS_PS3_FSolidMul,                                // 218
    &ShaderDesc_FS_PS3_FSolidInv,                                // 219
    &ShaderDesc_FS_PS3_FText,                                    // 220
    &ShaderDesc_FS_PS3_FTextMul,                                 // 221
    &ShaderDesc_FS_PS3_FTextInv,                                 // 222
    &ShaderDesc_FS_PS3_FBatchSolid,                              // 223
    &ShaderDesc_FS_PS3_FBatchSolidMul,                           // 224
    &ShaderDesc_FS_PS3_FBatchSolidInv,                           // 225
    &ShaderDesc_FS_PS3_FBatchText,                               // 226
    &ShaderDesc_FS_PS3_FBatchTextMul,                            // 227
    &ShaderDesc_FS_PS3_FBatchTextInv,                            // 228
    &ShaderDesc_FS_PS3_FInstancedSolid,                          // 229
    &ShaderDesc_FS_PS3_FInstancedSolidMul,                       // 230
    &ShaderDesc_FS_PS3_FInstancedSolidInv,                       // 231
    &ShaderDesc_FS_PS3_FInstancedText,                           // 232
    &ShaderDesc_FS_PS3_FInstancedTextMul,                        // 233
    &ShaderDesc_FS_PS3_FInstancedTextInv,                        // 234
    &ShaderDesc_FS_PS3_FYUV,                                     // 235
    &ShaderDesc_FS_PS3_FYUVMul,                                  // 236
    &ShaderDesc_FS_PS3_FYUVEAlpha,                               // 237
    &ShaderDesc_FS_PS3_FYUVEAlphaMul,                            // 238
    &ShaderDesc_FS_PS3_FYUVCxform,                               // 239
    &ShaderDesc_FS_PS3_FYUVCxformMul,                            // 240
    &ShaderDesc_FS_PS3_FYUVCxformEAlpha,                         // 241
    &ShaderDesc_FS_PS3_FYUVCxformEAlphaMul,                      // 242
    &ShaderDesc_FS_PS3_FYUVCxformAc,                             // 243
    &ShaderDesc_FS_PS3_FYUVCxformAcMul,                          // 244
    &ShaderDesc_FS_PS3_FYUVCxformAcEAlpha,                       // 245
    &ShaderDesc_FS_PS3_FYUVCxformAcEAlphaMul,                    // 246
    &ShaderDesc_FS_PS3_FYUVA,                                    // 247
    &ShaderDesc_FS_PS3_FYUVAMul,                                 // 248
    &ShaderDesc_FS_PS3_FYUVAInv,                                 // 249
    &ShaderDesc_FS_PS3_FYUVAEAlpha,                              // 250
    &ShaderDesc_FS_PS3_FYUVAEAlphaMul,                           // 251
    &ShaderDesc_FS_PS3_FYUVAEAlphaInv,                           // 252
    &ShaderDesc_FS_PS3_FYUVACxform,                              // 253
    &ShaderDesc_FS_PS3_FYUVACxformMul,                           // 254
    &ShaderDesc_FS_PS3_FYUVACxformInv,                           // 255
    &ShaderDesc_FS_PS3_FYUVACxformEAlpha,                        // 256
    &ShaderDesc_FS_PS3_FYUVACxformEAlphaMul,                     // 257
    &ShaderDesc_FS_PS3_FYUVACxformEAlphaInv,                     // 258
    &ShaderDesc_FS_PS3_FYUVACxformAc,                            // 259
    &ShaderDesc_FS_PS3_FYUVACxformAcMul,                         // 260
    &ShaderDesc_FS_PS3_FYUVACxformAcInv,                         // 261
    &ShaderDesc_FS_PS3_FYUVACxformAcEAlpha,                      // 262
    &ShaderDesc_FS_PS3_FYUVACxformAcEAlphaMul,                   // 263
    &ShaderDesc_FS_PS3_FYUVACxformAcEAlphaInv,                   // 264
    &ShaderDesc_FS_PS3_FBatchYUV,                                // 265
    &ShaderDesc_FS_PS3_FBatchYUVMul,                             // 266
    &ShaderDesc_FS_PS3_FBatchYUVEAlpha,                          // 267
    &ShaderDesc_FS_PS3_FBatchYUVEAlphaMul,                       // 268
    &ShaderDesc_FS_PS3_FBatchYUVCxform,                          // 269
    &ShaderDesc_FS_PS3_FBatchYUVCxformMul,                       // 270
    &ShaderDesc_FS_PS3_FBatchYUVCxformEAlpha,                    // 271
    &ShaderDesc_FS_PS3_FBatchYUVCxformEAlphaMul,                 // 272
    &ShaderDesc_FS_PS3_FBatchYUVCxformAc,                        // 273
    &ShaderDesc_FS_PS3_FBatchYUVCxformAcMul,                     // 274
    &ShaderDesc_FS_PS3_FBatchYUVCxformAcEAlpha,                  // 275
    &ShaderDesc_FS_PS3_FBatchYUVCxformAcEAlphaMul,               // 276
    &ShaderDesc_FS_PS3_FBatchYUVA,                               // 277
    &ShaderDesc_FS_PS3_FBatchYUVAMul,                            // 278
    &ShaderDesc_FS_PS3_FBatchYUVAInv,                            // 279
    &ShaderDesc_FS_PS3_FBatchYUVAEAlpha,                         // 280
    &ShaderDesc_FS_PS3_FBatchYUVAEAlphaMul,                      // 281
    &ShaderDesc_FS_PS3_FBatchYUVAEAlphaInv,                      // 282
    &ShaderDesc_FS_PS3_FBatchYUVACxform,                         // 283
    &ShaderDesc_FS_PS3_FBatchYUVACxformMul,                      // 284
    &ShaderDesc_FS_PS3_FBatchYUVACxformInv,                      // 285
    &ShaderDesc_FS_PS3_FBatchYUVACxformEAlpha,                   // 286
    &ShaderDesc_FS_PS3_FBatchYUVACxformEAlphaMul,                // 287
    &ShaderDesc_FS_PS3_FBatchYUVACxformEAlphaInv,                // 288
    &ShaderDesc_FS_PS3_FBatchYUVACxformAc,                       // 289
    &ShaderDesc_FS_PS3_FBatchYUVACxformAcMul,                    // 290
    &ShaderDesc_FS_PS3_FBatchYUVACxformAcInv,                    // 291
    &ShaderDesc_FS_PS3_FBatchYUVACxformAcEAlpha,                 // 292
    &ShaderDesc_FS_PS3_FBatchYUVACxformAcEAlphaMul,              // 293
    &ShaderDesc_FS_PS3_FBatchYUVACxformAcEAlphaInv,              // 294
    &ShaderDesc_FS_PS3_FInstancedYUV,                            // 295
    &ShaderDesc_FS_PS3_FInstancedYUVMul,                         // 296
    &ShaderDesc_FS_PS3_FInstancedYUVEAlpha,                      // 297
    &ShaderDesc_FS_PS3_FInstancedYUVEAlphaMul,                   // 298
    &ShaderDesc_FS_PS3_FInstancedYUVCxform,                      // 299
    &ShaderDesc_FS_PS3_FInstancedYUVCxformMul,                   // 300
    &ShaderDesc_FS_PS3_FInstancedYUVCxformEAlpha,                // 301
    &ShaderDesc_FS_PS3_FInstancedYUVCxformEAlphaMul,             // 302
    &ShaderDesc_FS_PS3_FInstancedYUVCxformAc,                    // 303
    &ShaderDesc_FS_PS3_FInstancedYUVCxformAcMul,                 // 304
    &ShaderDesc_FS_PS3_FInstancedYUVCxformAcEAlpha,              // 305
    &ShaderDesc_FS_PS3_FInstancedYUVCxformAcEAlphaMul,           // 306
    &ShaderDesc_FS_PS3_FInstancedYUVA,                           // 307
    &ShaderDesc_FS_PS3_FInstancedYUVAMul,                        // 308
    &ShaderDesc_FS_PS3_FInstancedYUVAInv,                        // 309
    &ShaderDesc_FS_PS3_FInstancedYUVAEAlpha,                     // 310
    &ShaderDesc_FS_PS3_FInstancedYUVAEAlphaMul,                  // 311
    &ShaderDesc_FS_PS3_FInstancedYUVAEAlphaInv,                  // 312
    &ShaderDesc_FS_PS3_FInstancedYUVACxform,                     // 313
    &ShaderDesc_FS_PS3_FInstancedYUVACxformMul,                  // 314
    &ShaderDesc_FS_PS3_FInstancedYUVACxformInv,                  // 315
    &ShaderDesc_FS_PS3_FInstancedYUVACxformEAlpha,               // 316
    &ShaderDesc_FS_PS3_FInstancedYUVACxformEAlphaMul,            // 317
    &ShaderDesc_FS_PS3_FInstancedYUVACxformEAlphaInv,            // 318
    &ShaderDesc_FS_PS3_FInstancedYUVACxformAc,                   // 319
    &ShaderDesc_FS_PS3_FInstancedYUVACxformAcMul,                // 320
    &ShaderDesc_FS_PS3_FInstancedYUVACxformAcInv,                // 321
    &ShaderDesc_FS_PS3_FInstancedYUVACxformAcEAlpha,             // 322
    &ShaderDesc_FS_PS3_FInstancedYUVACxformAcEAlphaMul,          // 323
    &ShaderDesc_FS_PS3_FInstancedYUVACxformAcEAlphaInv,          // 324
    &ShaderDesc_FS_PS3_FTexTGCMatrixAc,                          // 325
    &ShaderDesc_FS_PS3_FTexTGCMatrixAcMul,                       // 326
    &ShaderDesc_FS_PS3_FTexTGCMatrixAcEAlpha,                    // 327
    &ShaderDesc_FS_PS3_FTexTGCMatrixAcEAlphaMul,                 // 328
    &ShaderDesc_FS_PS3_FBox1Blur,                                // 329
    &ShaderDesc_FS_PS3_FBox1BlurMul,                             // 330
    &ShaderDesc_FS_PS3_FBox2Blur,                                // 331
    &ShaderDesc_FS_PS3_FBox2BlurMul,                             // 332
    &ShaderDesc_FS_PS3_FBox2Shadow,                              // 333
    &ShaderDesc_FS_PS3_FBox2ShadowMul,                           // 334
    &ShaderDesc_FS_PS3_FBox2ShadowKnockout,                      // 335
    &ShaderDesc_FS_PS3_FBox2ShadowKnockoutMul,                   // 336
    &ShaderDesc_FS_PS3_FBox2InnerShadow,                         // 337
    &ShaderDesc_FS_PS3_FBox2InnerShadowMul,                      // 338
    &ShaderDesc_FS_PS3_FBox2InnerShadowKnockout,                 // 339
    &ShaderDesc_FS_PS3_FBox2InnerShadowKnockoutMul,              // 340
    &ShaderDesc_FS_PS3_FBox2Shadowonly,                          // 341
    &ShaderDesc_FS_PS3_FBox2ShadowonlyMul,                       // 342
    &ShaderDesc_FS_PS3_FBox2ShadowHighlight,                     // 343
    &ShaderDesc_FS_PS3_FBox2ShadowHighlightMul,                  // 344
    &ShaderDesc_FS_PS3_FBox2ShadowHighlightKnockout,             // 345
    &ShaderDesc_FS_PS3_FBox2ShadowHighlightKnockoutMul,          // 346
    &ShaderDesc_FS_PS3_FBox2InnerShadowHighlight,                // 347
    &ShaderDesc_FS_PS3_FBox2InnerShadowHighlightMul,             // 348
    &ShaderDesc_FS_PS3_FBox2InnerShadowHighlightKnockout,        // 349
    &ShaderDesc_FS_PS3_FBox2InnerShadowHighlightKnockoutMul,     // 350
    &ShaderDesc_FS_PS3_FBox2FullShadowHighlight,                 // 351
    &ShaderDesc_FS_PS3_FBox2FullShadowHighlightMul,              // 352
    &ShaderDesc_FS_PS3_FBox2ShadowonlyHighlight,                 // 353
    &ShaderDesc_FS_PS3_FBox2ShadowonlyHighlightMul,              // 354
    &ShaderDesc_FS_PS3_FDrawableCopyPixels,                      // 355
    &ShaderDesc_FS_PS3_FDrawableCopyPixelsMergeAlpha,            // 356
    &ShaderDesc_FS_PS3_FDrawableCopyPixelsNoDestAlpha,           // 357
    &ShaderDesc_FS_PS3_FDrawableCopyPixelsAlpha,                 // 358
    &ShaderDesc_FS_PS3_FDrawableCopyPixelsAlphaMergeAlpha,       // 359
    &ShaderDesc_FS_PS3_FDrawableCopyPixelsAlphaNoDestAlpha,      // 360
    &ShaderDesc_FS_PS3_FDrawableMerge,                           // 361
    &ShaderDesc_FS_PS3_FDrawableCompare,                         // 362
    &ShaderDesc_FS_PS3_FDrawablePaletteMap,                      // 363
};

const FragShaderDesc* FragShaderDesc::GetDesc(ShaderDesc::ShaderType shader, ShaderDesc::ShaderVersion ver)
{
    return Descs[GetShaderIndex(shader, ver)];
};

FragShaderDesc::ShaderIndex FragShaderDesc::GetShaderIndex(ShaderDesc::ShaderType shader, ShaderDesc::ShaderVersion ver)
{
    ShaderDesc::ShaderIndex index = ShaderDesc::GetShaderIndex(shader);
    return ShaderIndexingData[index][ver]. FragIndex;
};

FragShaderDesc::ShaderIndex FragShaderDesc::GetShaderIndexForComboIndex(unsigned index, ShaderDesc::ShaderVersion ver) 
{
    switch(ver)
    {
        case ShaderDesc::ShaderVersion_PS3:
        {
            SF_UNUSED(index);
            SF_DEBUG_ASSERT(1, "PS3 indicated that ShaderCombo information was not required.");
            return FSI_None;
        }
        default: SF_ASSERT(0); return FSI_None;
    }
}

unsigned FragShaderDesc::GetShaderComboIndex(ShaderDesc::ShaderType shader, ShaderDesc::ShaderVersion ver)
{
    switch(ver)
    {
        case ShaderDesc::ShaderVersion_PS3:
        {
            SF_UNUSED(shader);
            SF_DEBUG_ASSERT(1, "PS3 indicated that ShaderCombo information was not required.");
            return 0;
        }
        default: SF_ASSERT(0); return 0;
    }
}

}}} // Scaleform::Render::PS3

