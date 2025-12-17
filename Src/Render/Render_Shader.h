/**************************************************************************

PublicHeader:   Render
Filename    :   Render_Shader.h
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_Shader_H
#define INC_SF_Render_Shader_H

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_Hash.h"
#include "Kernel/SF_Random.h"

#include "Render/Render_Matrix2x4.h"
#include "Render/Render_Matrix4x4.h"
#include "Render/Render_Color.h"
#include "Render/Render_CxForm.h"
#include "Render/Render_HAL.h"
#include "Render/Render_MatrixPool.h"
#include "Render/Render_Stats.h"
#include "Render/Render_Containers.h"
#include "Render/Render_Primitive.h"
#include "Render/Render_Gradients.h"
#include "Render/Render_Vertex.h"
#include "Render/Render_Types2D.h"
#include "Render/Render_Math2D.h"
#include "Render/Render_Twips.h"
#include "Render/Render_Profiler.h"
#include "Render/Render_FiltersLE.h"

namespace Scaleform { namespace Render {

enum StandardShaderFlags
{
    SS_Batch      = 1,
    SS_Position3d = 2,
    SS_Mul        = 4,
    SS_Cxform     = 8,
};

enum VertexFormatFlags
{
    // Aligns vertex strides to the size of the largest vertex component (but uses a minimum of 4 bytes)
    MVF_AlignVertexStride     = 0x01,

    // Aligns the start of vertex attributes on 4 bytes boundaries.
    MVF_AlignVertexAttrs      = 0x02,

    // Aligns both stride, and individual vertex components
    MVF_Align                 = MVF_AlignVertexStride | MVF_AlignVertexAttrs,   

    // Makes RGBA colors store as ARGB colors (and vice-versa). This may be required depending on the rendering API's expected color component ordering.
    MVF_ReverseColor          = 0x04,

    // Indicates that the rendering API can support instancing, so these vertex formats should be generated.
    MVF_HasInstancing         = 0x08,
    
    // Indicates that the render API cannot support batching, so these vertex formats should not be generated.
    MVF_NoBatching            = 0x10,

    // Endian swaps 'factor' parameters. This is required on X360, the factors are written at given offsets, but
    // because they are stored in colors, they must be reversed for the GPU to read them correctly.
    MVF_EndianSwapFactors     = 0x20,
};

template<typename Uniforms, typename Shader>
class ShaderInterfaceBase
{
protected:
    float    UniformData[Uniforms::SU_TotalSize];
    bool     UniformSet[Uniforms::SU_Count];
    Texture* Textures[4];
    bool     PrimitiveOpen; // true, if inside a BeginPrimitive/Finish block, false otherwise. Used for consistency checking.

public:

    ShaderInterfaceBase() :
        PrimitiveOpen(false)
    {
        memset(Textures, 0, sizeof(Textures));
        memset(UniformSet, 0, sizeof(UniformSet));
    }

    unsigned GetParameterStage(const Shader& sd, unsigned var, int index = 0) const
    {
        if (sd->pVDesc->BatchUniforms[var].Offset >= 0 && index < sd->pVDesc->BatchUniforms[var].Size)
            return 0x10000 | sd->pVDesc->BatchUniforms[var].Size;
        else if (sd->pVDesc->Uniforms[var].Location >= 0 && index < sd->pVDesc->Uniforms[var].Size)
            return 0x10000 | sd->pVDesc->Uniforms[var].Size / sd->pVDesc->Uniforms[var].ElementSize;
        else if (sd->pFDesc->BatchUniforms[var].Offset >= 0 && index < sd->pFDesc->BatchUniforms[var].Size)
            return 0x20000 | sd->pFDesc->BatchUniforms[var].Size;
        else if (sd->pFDesc->Uniforms[var].Location >= 0 && index < sd->pFDesc->Uniforms[var].Size)
            return 0x20000 | sd->pFDesc->Uniforms[var].Size / sd->pVDesc->Uniforms[var].ElementSize;
        else
            return 0;
    }

    void BeginPrimitive()
    {
        SF_DEBUG_ASSERT(!PrimitiveOpen, "BeginPrimitive called multiple times, without Finish.");
        if (PrimitiveOpen)
            return;
        PrimitiveOpen = true;

#ifdef SF_BUILD_DEBUG
        // If we are beginning a primitive, no uniforms should be currently set.
        for (int i = 0; i < Uniforms::SU_Count; i++)
        {
            SF_DEBUG_ASSERT1(!UniformSet[i], "Unexpected uniform set during BeginPrimitive (%d).", i);
        }
        memset(UniformData, 0, sizeof(UniformData));
#endif // SF_BUILD_DEBUG
    	memset(UniformSet, 0, sizeof(UniformSet));
        memset(Textures, 0, sizeof(Textures));
    }

    void Finish(unsigned meshCount)
    {
        SF_UNUSED(meshCount);
        SF_DEBUG_ASSERT(PrimitiveOpen, "Finish called without BeginPrimitive.");
        PrimitiveOpen = false;
    }

    void SetUniform0(const Shader& sd, unsigned var, const float* v, unsigned n, unsigned index = 0, unsigned batch = 0)
    {
	    SF_ASSERT(batch == 0); SF_UNUSED(batch);
        // Relax this restriction. If we are using debug profile modes, we might be tricking the system to use another shader,
        // which doesn't contain the same parameters as the original.
	    //SF_DEBUG_ASSERT(sd->pFDesc->Uniforms[var].Size > 0 || sd->pVDesc->Uniforms[var].Size > 0, "Setting uniform not used by current shader.");

        if (sd->pVDesc->Uniforms[var].Size)
        {
            SF_ASSERT(sd->pVDesc->Uniforms[var].ShadowOffset + sd->pVDesc->Uniforms[var].ElementSize * 
                      index + n <= Uniforms::SU_TotalSize);
    	    memcpy(UniformData + sd->pVDesc->Uniforms[var].ShadowOffset + 
                   sd->pVDesc->Uniforms[var].ElementSize * index, v, n * sizeof(float));
        }
        if (sd->pFDesc->Uniforms[var].Size)
        {
            SF_ASSERT(sd->pFDesc->Uniforms[var].ShadowOffset + sd->pFDesc->Uniforms[var].ElementSize * 
                      index + n <= Uniforms::SU_TotalSize);
            memcpy(UniformData + sd->pFDesc->Uniforms[var].ShadowOffset + 
                   sd->pFDesc->Uniforms[var].ElementSize * index, v, n * sizeof(float));
        }
	    UniformSet[var] = 1;
    }

    void SetUniform(const Shader& sd, unsigned var, const float* v, unsigned n, unsigned index = 0, unsigned batch = 0)
    {
        if (sd->pVDesc->BatchUniforms[var].Offset >= 0)
	    {
	        unsigned bvar = sd->pVDesc->BatchUniforms[var].Array;
	        SetUniform0(sd, sd->pVDesc->BatchUniforms[var].Array, v, n,
			    batch * sd->pVDesc->Uniforms[bvar].BatchSize + sd->pVDesc->BatchUniforms[var].Offset + index);
	    }
        else if (sd->pFDesc->BatchUniforms[var].Offset >= 0)
        {
            unsigned bvar = sd->pFDesc->BatchUniforms[var].Array;
            SetUniform0(sd, sd->pFDesc->BatchUniforms[var].Array, v, n,
                batch * sd->pFDesc->Uniforms[bvar].BatchSize + sd->pFDesc->BatchUniforms[var].Offset + index);
        }
	    else
	        SetUniform0(sd, var, v, n, index, batch);
    }

    void SetColor(const Shader& sd, unsigned var, Color c, unsigned index = 0, unsigned batch = 0)
    {
	    const float mult  = 1.0f / 255.0f;
	    float rgba[4] = 
	    {
	        c.GetRed() * mult,  c.GetGreen() * mult,
	        c.GetBlue() * mult, c.GetAlpha() * mult
	    };
	    SetUniform(sd, var, rgba, 4, index, batch);
    }

    void SetMatrix(const Shader& sd, unsigned var, const Matrix2F &m, unsigned index = 0, unsigned batch = 0)
    {
        const float *rows = &m.M[0][0];
	    SetUniform(sd, var, rows, 8, index*2, batch);
    }

    void SetMatrix(const Shader& sd, unsigned var, const Matrix2F &m1, const HMatrix &m2, const MatrixState* Matrices, unsigned index = 0, unsigned batch = 0)
    {
        if (m2.Has3D())
        {
            Matrix4F m(Matrices->GetUVP(), Matrix3F(m2.GetMatrix3D(), m1));
            const float *rows = m.Data();
            SetUniform(sd, var, rows, 16, index, batch);
        }
        else
        {
            Matrix2F  m(m1, m2.GetMatrix2D(), Matrices->UserView);
            const float *rows = &m.M[0][0];

            SetUniform(sd, var, rows, 8, index*2, batch);
        }
    }

    void SetCxform(const Shader& sd, const Cxform & cx, unsigned index = 0, unsigned batch = 0)
    {
        const float (*rows)[4] = &cx.M[0];

        SetUniform(sd, Uniforms::SU_cxmul, rows[0], 4, index, batch);
        SetUniform(sd, Uniforms::SU_cxadd, rows[1], 4, index, batch);
    }

    // Returns 0 if uniform not present in shader
    unsigned GetUniformSize(const Shader& sd, unsigned i)
    {
        unsigned size = 0;
        if (sd->pVDesc->BatchUniforms[i].Array <= Uniforms::SU_Count-1)
            size = sd->pVDesc->BatchUniforms[i].Size * sd->pVDesc->Uniforms[(int)sd->pVDesc->BatchUniforms[i].Array].ElementSize;
        else if (sd->pVDesc->Uniforms[i].Location >= 0)
            size = sd->pVDesc->Uniforms[i].Size;
        else if (sd->pFDesc->BatchUniforms[i].Array <= Uniforms::SU_Count-1)
            size = sd->pFDesc->BatchUniforms[i].Size * sd->pFDesc->Uniforms[(int)sd->pFDesc->BatchUniforms[i].Array].ElementSize;
        else if (sd->pFDesc->Uniforms[i].Location >= 0)
            size = sd->pFDesc->Uniforms[i].Size;
        return size;
    }

    static bool IsBuiltinUniform(unsigned var)
    {
        return Uniforms::UniformFlags[var] & Uniforms::Uniform_Builtin;
    }

    void SetUserUniforms(const Shader& sd, const HMatrix &m, int batch = 0)
    {
        float* pdata = (float*)m.GetUserData();
        for (int i = 0; i < Uniforms::SU_Count; i++)
            if (!IsBuiltinUniform(i))
            {
                unsigned size = GetUniformSize(sd, i);

                if (size)
                {
                    if ((Uniforms::UniformFlags[i] & Uniforms::Uniform_TexScale) && size <= 4 && Textures[0])
                    {
                        ImageSize tsize = Textures[0]->GetSize();
                        float uscale = 1.0f/tsize.Width;
                        float vscale = 1.0f/tsize.Height;
                        float scaled[4];
                        scaled[0] = pdata[0] * uscale;
                        scaled[1] = pdata[1] * vscale;
                        scaled[2] = pdata[2];
                        scaled[3] = pdata[3];
                        SetUniform(sd, i, scaled, size, 0, batch);
                    }
                    else
                        SetUniform(sd, i, pdata, size, 0, batch);
                    pdata += size;
                }
            }
    }

    // BeginScene stub. ShaderInterface derived classes can implement this method, to perform operations
    // that should happen at the beginning of a scene.
    void BeginScene()
    {

    }

};

// This is used by the StaticShaderManager to determine hash VertexFormats for different fill types.
struct SourceFormatHash
{
    SourceFormatHash(PrimitiveFillType  fill,
        const VertexFormat* sourceFormat,
        unsigned           flags) :
    FillType(fill),
        FormatFlags(flags),
        SourceFormat(sourceFormat) { }

    bool operator==(const SourceFormatHash& other) const
    {
        return other.FillType == FillType &&
            other.SourceFormat == SourceFormat &&
            other.FormatFlags == FormatFlags;
    }
    // Member variables need to be aligned correctly so that there is no padding, even on 64-bit
    PrimitiveFillType       FillType;
    unsigned                FormatFlags;
    const VertexFormat*     SourceFormat;
};

// This is used by the StaticShaderManager to store VertexFormat hash results.
struct ResultFormat
{
    const VertexFormat*     single;
    const VertexFormat*     batch;
    const VertexFormat*     instanced;
};

template<typename ShaderDesc, typename VShaderDesc,
         class Uniforms, class ShaderInterface, class NativeTexture>
class StaticShaderManager
{
public:
    typedef typename VShaderDesc::VertexAttrDesc    VertexAttrDesc;
    typedef typename ShaderDesc::ShaderType         ShaderType;
    typedef typename ShaderInterface::Shader        Shader;
    typedef typename ShaderDesc::ShaderVersion      ShaderVersion;

protected:
    MultiKeyCollection<VertexElement, VertexFormat, 32>    VFormats;
    ProfileViews*                                          Profiler;

public:
    StaticShaderManager(ProfileViews* prof) : Profiler(prof) {}
    virtual ~StaticShaderManager() { }

    virtual ShaderVersion GetShaderVersion() const 
    {
        return ShaderDesc::ShaderVersion_Default;
    }

    virtual unsigned GetNumberOfUniforms() const
    {
        // Assume that there are at least 256 uniforms. The number of batches is generally 
        // limited to (SF_RENDER_MAX_BATCHES x maximum 10 uniforms per batch), and SF_RENDER_MAX_BATCHES is generally 24,
        // meaning exceeding 256 is not useful. However, some platforms (GLES) don't always support this many uniforms. 
        return 256;
    }

    ShaderType StaticShaderForFill (PrimitiveFill* pfill, unsigned& fillflags, unsigned batchType)
    {
        PrimitiveFillType type = Profiler->GetFillType(pfill->GetType());
        switch(type)
        {
        // Override these specifically for video textures. It needs to set the frag shader as a video compatible one.
        case PrimFill_Texture:
        case PrimFill_Texture_EAlpha:
            if ( pfill->GetTextureCount() == 1 && pfill->GetTexture(0)->GetFormat() >= Image_VideoFormat_Start &&
                 pfill->GetTexture(0)->GetFormat() <= Image_VideoFormat_End)
            {

                unsigned shader = 0;
                switch(pfill->GetTexture(0)->GetFormat())
                {
                    case Image_Y8_U2_V2: 
                        SF_DEBUG_WARNONCE1(pfill->GetTexture(0)->GetPlaneCount() != 3, 
                            "Image_Y8_U2_V2 should have 3 planes, but it has %d", pfill->GetTexture(0)->GetPlaneCount());
                        shader = ShaderDesc::ST_YUV; 
                        break;
                    case Image_Y8_U2_V2_A8: 
                        SF_DEBUG_WARNONCE1(pfill->GetTexture(0)->GetPlaneCount() != 4, 
                            "Image_Y8_U2_V2_A8 should have 4 planes, but it has %d", pfill->GetTexture(0)->GetPlaneCount());
                        shader = ShaderDesc::ST_YUVA; 
                        break;
                    case Image_Y4_U2_V2:
                        SF_DEBUG_WARNONCE1(pfill->GetTexture(0)->GetPlaneCount() != 1, 
                            "Image_Y4_U2_V2 should have 1 plane, but it has %d", pfill->GetTexture(0)->GetPlaneCount());
                        shader = ShaderDesc::ST_YUY2;
                        break;
                    default:
                        SF_DEBUG_ASSERT1(0, "Unknown video texture format: %d\n", pfill->GetTexture(0)->GetFormat());
                        shader = ShaderDesc::ST_YUV;
                        break;
                }

                if (type == PrimFill_Texture_EAlpha)
                    shader += ShaderDesc::ST_base_video_EAlpha;

                if (shader != ShaderDesc::ST_Text)
                {
                    if ((fillflags & (FF_AlphaWrite|FF_Cxform)) == (FF_AlphaWrite|FF_Cxform))
                        shader += ShaderDesc::ST_base_video_CxformAc;
                    else if ((fillflags & FF_Cxform))
                        shader += ShaderDesc::ST_base_video_Cxform;
                }

                switch(batchType)
                {
                case PrimitiveBatch::DP_Batch:
                    shader += ShaderDesc::ST_base_video_Batch;
                    break;

                case PrimitiveBatch::DP_Instanced:
                    shader += ShaderDesc::ST_base_video_Instanced;
                    break;
                default: break;
                }

                if (fillflags & FF_Multiply)
                    shader += ShaderDesc::ST_base_video_Mul;

                if (fillflags & FF_3DProjection)
                    shader += ShaderDesc::ST_base_video_Position3d;


                return (ShaderType)shader;
            }
            break;

        default: break;
        }
        return StaticShaderForFill(type, fillflags, batchType);
    }

    ShaderType StaticShaderForFill (PrimitiveFillType fill, unsigned& fillflags, unsigned batchType)
    {
        unsigned shader = ShaderDesc::ST_None;

        switch (fill)
        {
        default:
        case PrimFill_None:
        case PrimFill_Mask:
        case PrimFill_SolidColor:
            shader = ShaderDesc::ST_Solid;
            fillflags &= ~FF_Cxform;
            break;

        case PrimFill_VColor:
            shader = ShaderDesc::ST_Vertex;
            break;

        case PrimFill_VColor_EAlpha:
            shader = ShaderDesc::ST_VertexEAlpha;
            break;

        case PrimFill_Texture:
            shader = ShaderDesc::ST_TexTG;
            break;

        case PrimFill_Texture_EAlpha:
            shader = ShaderDesc::ST_TexTGEAlpha;
            break;
    		
        case PrimFill_Texture_VColor:
            shader = ShaderDesc::ST_TexTGVertex;
            break;

        case PrimFill_Texture_VColor_EAlpha:
            shader = ShaderDesc::ST_TexTGVertexEAlpha;
            break;

        case PrimFill_2Texture:
            shader = ShaderDesc::ST_TexTGTexTG;
            break;

        case PrimFill_2Texture_EAlpha:
            shader = ShaderDesc::ST_TexTGTexTGEAlpha;
            break;

        case PrimFill_UVTexture:
            shader = ShaderDesc::ST_TexUV;
            break;

        case PrimFill_UVTextureAlpha_VColor:
            shader = ShaderDesc::ST_Text;
            fillflags |= FF_Cxform;
            break;

        //case PrimFill_UVTextureDFAlpha_VColor:
        //    fillflags |= FF_Cxform;
        //    return FShaderDesc::FindStaticShader("TextDFA", (batchType == PrimitiveBatch::DP_Batch ? SS_Batch : 0) | (fillflags & FF_Multiply ? SS_Mul : 0));
        //    break;
        }

        // Texture density profiler mode implies no Cxform, or special blendmode shader
        if (Profiler->GetProfileMode() == Profile_TextureDensity)
        {
            if (PrimitiveFill::HasTexture(fill))
            {
                fillflags &= ~(FF_Cxform|FF_BlendMask);
                shader += ShaderDesc::ST_base_TexDensity;
            }
        }

        if (shader != ShaderDesc::ST_Text)
        {
            if ((fillflags & (FF_AlphaWrite|FF_Cxform)) == (FF_AlphaWrite|FF_Cxform))
                shader += ShaderDesc::ST_base_CxformAc;
            else if ((fillflags & FF_Cxform))
                shader += ShaderDesc::ST_base_Cxform;
        }



        switch(batchType)
        {
        case PrimitiveBatch::DP_Batch:
            shader += ShaderDesc::ST_base_Batch;
            break;

        case PrimitiveBatch::DP_Instanced:
            shader += ShaderDesc::ST_base_Instanced;
            break;

        default: break;
        }

        if (fillflags & FF_Multiply)
            shader += ShaderDesc::ST_base_Mul;
        if (fillflags & FF_Invert)
            shader += ShaderDesc::ST_base_Inv;

        if (fillflags & FF_3DProjection)
            shader += ShaderDesc::ST_base_Position3d;

        return (ShaderType)shader;
    }

    const VertexFormat* GetVertexFormat(VertexElement* pelements, unsigned count, unsigned size, unsigned alignment = 1)
    {
        VertexFormat  *pformat   = VFormats.Find(pelements, count);
        if (pformat)
            return pformat;

        pformat = VFormats.Add(&pelements, pelements, count);
        if (!pformat)
            return 0;

        // Align vertex size to if requested
        size = (size+(alignment-1)) & ~(alignment-1);

        pformat->Size      = size;
        pformat->pElements = pelements;
        return pformat;
    }

    HashLH<SourceFormatHash, ResultFormat>  VertexFormatComputedHash;

    void    MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
			    const VertexFormat** single,
			    const VertexFormat** batch, const VertexFormat** instanced,
                unsigned flags = 0)
    {
        // Check the profiler, and see if batching/instancing are disabled.
        if (Profiler->GetProfileFlag(ProfileFlag_NoBatching))
            flags |= MVF_NoBatching;
        if (Profiler->GetProfileFlag(ProfileFlag_NoInstancing))
            flags &= ~MVF_HasInstancing;

        // Translate the fill based on the profile mode. Different shaders may be used, and thus
        // require different vertex formats.
        fill = Profiler->GetFillType(fill);

        // Hashed lookup. If we have done this source format/fill and flags combination before, simply return it.
        ResultFormat result;
        SourceFormatHash sourceKey(fill, sourceFormat, flags);
        if (VertexFormatComputedHash.Get(sourceKey, &result))
        {
            *single = result.single;
            *batch = result.batch;
            *instanced = result.instanced;
            return;
        }
        
        unsigned             fillflags = 0;
        ShaderType           shader  = StaticShaderForFill(fill, fillflags, PrimitiveBatch::DP_Single);
        const VShaderDesc*   pshader = VShaderDesc::GetDesc(shader, GetShaderVersion());

        // If the desc could not be found, it means that this fill is not supported by the current shader system.
        if (pshader == 0)
        {
            *batch = *single = *instanced = 0;
            return;
        }

        const VertexAttrDesc* psvf = pshader->Attributes;
        const unsigned       maxVertexElements = 8;
        VertexElement        outf[maxVertexElements];
        unsigned             size = 0, maxalign = 4;
        int                  batchOffset = -1, batchElement = -1;
        int                  j = 0;

        for (int i = 0; i < pshader->NumAttribs; i++)
        {
            if ((psvf[i].Attr & (VET_Usage_Mask | VET_Index_Mask | VET_Components_Mask)) == (VET_Color | (1 << VET_Index_Shift) | 4))
            {
                // XXX - change shaders to use .rg instead of .ra for these
                if ( (flags & MVF_EndianSwapFactors) == 0 )
                {
                outf[j].Offset = size;
                outf[j].Attribute = VET_T0Weight8;
                j++;
                outf[j].Offset = size+3;
                outf[j].Attribute = VET_FactorAlpha8;
                batchOffset = size+2;
                batchElement = j;
                }
                else
                {
                    outf[j].Offset = size;
                    outf[j].Attribute = VET_FactorAlpha8;
                j++;
                    outf[j].Offset = size+3;
                    outf[j].Attribute = VET_T0Weight8;
                    batchElement = j;
                    batchOffset = size+1;
                }
                j++;
                size += 4;
                continue;
            }

            const VertexElement* pv = sourceFormat->GetElement(psvf[i].Attr & (VET_Usage_Mask|VET_Index_Mask), VET_Usage_Mask|VET_Index_Mask);
            if (!pv)
            {
                *batch = *single = *instanced = nullptr;
                return;
            }
            outf[j] = *pv;
            outf[j].Offset = size;

            if (flags & MVF_ReverseColor)
            {
                if ((pv->Attribute & (VET_Usage_Mask|VET_CompType_Mask|VET_Components_Mask)) == VET_ColorRGBA8)
                    outf[j].Attribute = VET_ColorARGB8 | (pv->Attribute & ~(VET_Usage_Mask|VET_CompType_Mask|VET_Components_Mask));
            }
            else
            {
                if ((pv->Attribute & (VET_Usage_Mask|VET_CompType_Mask|VET_Components_Mask)) == VET_ColorARGB8)
                    outf[j].Attribute = VET_ColorRGBA8 | (pv->Attribute & ~(VET_Usage_Mask|VET_CompType_Mask|VET_Components_Mask));
            }

            SF_ASSERT((outf[j].Attribute & VET_Components_Mask) > 0 && (outf[j].Attribute & VET_Components_Mask) <= 4);
            maxalign = Alg::Max(maxalign, pv->CompSize());
            size += outf[j].Size();
            if (flags & MVF_AlignVertexAttrs)
                size = (size + 3) & ~3;

            j++;
        }
        outf[j].Attribute = VET_None;
        outf[j].Offset = 0;
        *single = GetVertexFormat(outf, j+1, size, flags & MVF_AlignVertexStride ? maxalign : 1);

        // If we have instancing support, add an I8 vertex element to indicate this.
        if ( flags & MVF_HasInstancing )
        {
            outf[j].Attribute = VET_Instance | VET_I8  | 1 | VET_Argument_Flag;
            outf[j].Offset = 0;
            outf[j+1].Attribute = VET_None;
            outf[j+1].Offset = 0;
            *instanced = GetVertexFormat(outf, j+2, size, flags & MVF_AlignVertexStride ? maxalign : 1);
        }
        else
        {
            *instanced = 0;
        }

        // If we have batching support, either add the batch index to the packed factors, or add another attribute.
        if ((flags & MVF_NoBatching) == 0)
        {
            if (batchOffset >= 0)
            {
                for (int i = j-1; i >= batchElement; i--)
                    outf[i+1] = outf[i];

                outf[batchElement].Attribute = VET_Instance8;
                outf[batchElement].Offset = batchOffset;
            }
            else
            {
                outf[j].Attribute = VET_Instance8;
                outf[j].Offset = (flags & MVF_EndianSwapFactors) == 0 ? size : size+3;
                size += outf[j].Size();
                if (flags & MVF_AlignVertexAttrs)
                    size = Alg::Align<4>(size);
            }
            outf[j+1].Attribute = VET_None;
            outf[j+1].Offset = 0;
            *batch = GetVertexFormat(outf, j+2, size, flags & MVF_AlignVertexStride ? maxalign : 1);
        }
        else
        {
            *batch = 0;
        }

        // Set the results in the hash.
        result.single    = *single;
        result.batch     = *batch;
        result.instanced = *instanced;
        VertexFormatComputedHash.Set(sourceKey, result);
    }

    const Shader& SetPrimitiveFill(PrimitiveFill* pfill, unsigned& fillFlags, unsigned batchType, const VertexFormat* pvf, unsigned meshCount,
                                   const MatrixState* Matrices, const Primitive::MeshEntry* pmeshes, ShaderInterface* psi)
    {
        PrimitiveFillType fillType = Profiler->GetFillType(pfill->GetType());

        if ((fillFlags & FF_Blending) == 0 && pfill->RequiresBlend())
            fillFlags |= FF_Blending;

        // If we do not have CxForms, or blending, check the color transforms of the matrices to determine if we will need to apply them.
        if ((fillFlags & (FF_Blending|FF_Cxform)) != (FF_Blending|FF_Cxform))
        {
            for (unsigned i = 0; i < meshCount; i++)
            {
                Cxform finalCx = Profiler->GetCxform(pmeshes[i].M.GetCxform());
                if (finalCx != Cxform::Identity)
                {
                    fillFlags |= FF_Cxform;
                    if (finalCx.RequiresBlend())
                        fillFlags |= FF_Blending;
                    break;
                }
            }
        }

        ShaderType shader = StaticShaderForFill(pfill, fillFlags, batchType);

        Profiler->SetFillFlags(fillFlags);

        psi->SetStaticShader(shader, pvf);
        const Shader& pso = psi->GetCurrentShaders();
        bool solid = PrimitiveFill::IsSolid(fillType);

        if (solid)
            psi->SetColor(pso, Uniforms::SU_cxmul, Profiler->GetColor(pfill->GetSolidColor()));
        else if (fillType >= PrimFill_Texture)
        {     
            NativeTexture*  pt0 = (NativeTexture*)pfill->GetTexture(0);
            SF_ASSERT(pt0);

            ImageFillMode fm0 = pfill->GetFillMode(0);


            psi->SetTexture(pso, Uniforms::SU_tex, pt0, fm0);
            if ((fillType == PrimFill_2Texture) || (fillType == PrimFill_2Texture_EAlpha))
            {
                NativeTexture* pt1 = (NativeTexture*)pfill->GetTexture(1);
                ImageFillMode  fm1 = pfill->GetFillMode(1);

                psi->SetTexture(pso, Uniforms::SU_tex, pt1, fm1, pt0->GetTextureStageCount());
            }
        }

        if (fillType == PrimFill_UVTextureDFAlpha_VColor)
        {
            for (unsigned i = 0; i < meshCount; i++)
            {
                psi->SetUserUniforms(pso, pmeshes[i].M, i);
            }
        }

        unsigned tmCount = (psi->GetParameterStage(pso, Uniforms::SU_texgen) & 0xffff) >> 1;

        for (unsigned i = 0; i < meshCount; i++)
        {
            psi->SetMatrix(pso, Uniforms::SU_mvp, pmeshes[i].pMesh->VertexMatrix, pmeshes[i].M, Matrices, 0, i);

            if (fillType == PrimFill_Mask)
                psi->SetColor(pso, Uniforms::SU_cxmul, Profiler->GetColor(Color(128,0,0,128)));
            else
            if (fillFlags & FF_Cxform)
                psi->SetCxform(pso, Profiler->GetCxform(pmeshes[i].M.GetCxform()), 0, i);

            for (unsigned tm = 0; tm < tmCount; tm++)
            {
                Matrix2F m(pmeshes[i].pMesh->VertexMatrix);
                m.Append(pmeshes[i].M.GetTextureMatrix(tm));
                psi->SetMatrix(pso, Uniforms::SU_texgen, m, tm, i);
            }
        }

        return pso;
    }

    const Shader& SetFill(PrimitiveFillType fillType, unsigned& fillFlags, unsigned batchType, const VertexFormat* pvf, ShaderInterface* psi)
    {
        ShaderType shader = StaticShaderForFill(fillType, fillFlags, batchType);
        psi->SetStaticShader(shader, pvf);
        psi->BeginPrimitive();
        return psi->GetCurrentShaders();
    }

    // Applies the given fill pass for the given filter. The first target in the list is the texture to be filtered.
    bool SetFilterFill(const Matrix2F& mvp, const Cxform & cx, const Filter* filter, Ptr<RenderTarget> * targets, 
                       unsigned* shaders, unsigned pass, unsigned passCount, const VertexFormat* pvf, 
                       ShaderInterface* psi)
    { 
        if ( !psi->SetStaticShader( (ShaderType)shaders[pass], pvf ) )
            return false;
        psi->BeginPrimitive();
        const Shader& pso = psi->GetCurrentShaders();

        // Apply the mvp
        psi->SetMatrix(pso, Uniforms::SU_mvp, mvp );

        // Apply the source texture.
        bool shadowPass = shaders[pass] >= ShaderDesc::ST_start_shadows && shaders[pass] <= ShaderDesc::ST_end_shadows;
        NativeTexture* ptexture = (NativeTexture*)targets[Target_Source]->GetTexture();
        psi->SetTexture(pso, Uniforms::SU_tex, ptexture, ImageFillMode(Wrap_Clamp, Sample_Linear));

        // Apply the texture matrix.
        Matrix2F texgen;
        const Rect<int>& srect = targets[Target_Source]->GetRect();
        texgen.AppendTranslation((float)srect.x1, (float)srect.y1);
        texgen.AppendScaling((float)srect.Width() / ptexture->GetSize().Width, (float)srect.Height() / ptexture->GetSize().Height);
        psi->SetUniform(pso, Uniforms::SU_texgen, &(texgen.M[0][0]), 8);

        // Apply the remainder of the uniforms.
        if ( filter->GetFilterType() <= Filter_Blur_End )
        {
            BlurFilter* blurFilter = (BlurFilter*)filter;
            const BlurFilterParams& params = blurFilter->GetParams();

            // Common parameters.
            psi->SetCxform(pso, cx);

            float texscale[2];
            texscale[0] = 1.0f / ptexture->GetSize().Width;
            texscale[1] = 1.0f / ptexture->GetSize().Height;

            float fsize[4];
            float blurx = Alg::Max( 1.0f, floorf(TwipsToPixels(params.BlurX)));
            float blury = Alg::Max( 1.0f, floorf(TwipsToPixels(params.BlurY)));
            if ( shaders[pass] == ShaderDesc::ST_Box1Blur || shaders[pass] == ShaderDesc::ST_Box1BlurMul)
            {
                // On even passes, do the X blur, on odd passes, do the Y blur.
                if ( (pass & 1) == 0 )
                {
                    fsize[0] = (blurx-1)*0.5f;
                    fsize[1] = 0;
                    fsize[3] = 1.0f / (blurx);
                    texscale[1] = 0;
                }
                else
                {
                    fsize[0] = (blury-1)*0.5f;
                    fsize[1] = 0;
                    fsize[3] = 1.0f / (blury);
                    texscale[0] = 0;
                }
            }
            else if ( pass == passCount-1 && pass != 0)
            {
                fsize[0] = 0.0f;
                fsize[1] = (blury-1)*0.5f;
                fsize[3] = 1.0f / (blury);
            }
            else
            {
                fsize[0] = (blurx-1)*0.5f;
                fsize[1] = (blury-1)*0.5f;
                fsize[3] = 1.0f / (blurx * blury);
            }

            // fsize.z = strength, only used on the last pass.
            fsize[2] = (pass == passCount-1) ? params.Strength : 1.0f;

            // Make sure that fsize.x/.y are not zero, but instead small values.
            fsize[0] = Alg::Max<float>((float)SF_MATH_EPSILON, fsize[0]);
            fsize[1] = Alg::Max<float>((float)SF_MATH_EPSILON, fsize[1]);

            psi->SetUniform(pso, Uniforms::SU_fsize, fsize, 4 );
            psi->SetUniform(pso, Uniforms::SU_texscale, texscale, 2 );

            if ( shadowPass )
            {
                float scolors[2][4];
                float srctexscale[2];
                float offset[2];
                params.Colors[0].GetRGBAFloat(scolors[0]);
                params.Colors[1].GetRGBAFloat(scolors[1]);
                offset[0] = -TwipsToPixels(params.Offset.x);
                offset[1] = -TwipsToPixels(params.Offset.y);

                if ( targets[Target_Original] )
                {
                    NativeTexture* psrctex = (NativeTexture*)targets[Target_Original]->GetTexture();
                    srctexscale[0] = 1.0f / (psrctex->GetSize().Width * texscale[0] );
                    srctexscale[1] = 1.0f / (psrctex->GetSize().Height * texscale[1] );
                    psi->SetUniform(pso, Uniforms::SU_srctexscale, srctexscale, 2 );
                    psi->SetTexture(pso, Uniforms::SU_srctex, psrctex, ImageFillMode(Wrap_Clamp, Sample_Linear));
                }
        
                psi->SetUniform(pso, Uniforms::SU_offset, offset, 2 );

                if (filter->GetFilterType() == Filter_Bevel)
                    psi->SetUniform(pso, Uniforms::SU_scolor2, scolors[1], 4 );

                if (filter->GetFilterType() == Filter_GradientBevel || 
                    filter->GetFilterType() == Filter_GradientGlow)
                {
                    const GradientFilter* gradientFilter = reinterpret_cast<const GradientFilter*>(filter);
                    Image* img = gradientFilter->GetGradientImage();
                    Texture* gradientTexture = img->GetTexture(ptexture->GetManager());
                    psi->SetTexture(pso, Uniforms::SU_gradtex, gradientTexture, ImageFillMode(Wrap_Clamp, Sample_Linear) );
                }
                else
                {
                    psi->SetUniform(pso, Uniforms::SU_scolor, scolors[0], 4 );
                }
            }
        }
        else if (filter->GetFilterType() == Filter_ColorMatrix)
        {
            ColorMatrixFilter& matrixFilter = *(ColorMatrixFilter*)filter;

            // If there is a color transform, in addition to the filter, embed that directly into the matrix.
            float matrixData[ColorMatrixFilter::ColorMatrixEntries];
            memcpy(matrixData, &matrixFilter[0], sizeof(matrixData));
            SF_COMPILER_ASSERT(ColorMatrixFilter::ColorMatrixEntries == 20);
            for (unsigned i = 0; i < 5; i++)
            {
                matrixData[i*4+0] = matrixData[i*4+0] * cx.M[0][0] * cx.M[0][3];
                matrixData[i*4+1] = matrixData[i*4+1] * cx.M[0][1] * cx.M[0][3];
                matrixData[i*4+2] = matrixData[i*4+2] * cx.M[0][2] * cx.M[0][3];
                matrixData[i*4+3] = matrixData[i*4+3] * cx.M[0][3];
            }
            matrixData[16] = (matrixData[16] + cx.M[1][0] * 1.f/255.f) * cx.M[0][3];
            matrixData[17] = (matrixData[17] + cx.M[1][1] * 1.f/255.f) * cx.M[0][3];
            matrixData[18] = (matrixData[18] + cx.M[1][2] * 1.f/255.f) * cx.M[0][3];
            matrixData[19] = (matrixData[19] + cx.M[1][3] * 1.f/255.f) * cx.M[0][3];

            // Now apply the data.
            psi->SetUniform(pso, Uniforms::SU_cxadd, &(matrixData[16]), 4 );
            psi->SetUniform(pso, Uniforms::SU_cxmul, &(matrixData[0]), 16 );
        }
        else if (filter->GetFilterType() == Filter_DisplacementMap)
        {
            const DisplacementMapFilter* dmFilter = reinterpret_cast<const DisplacementMapFilter*>(filter);

            // DisplacementMap may specify a different fill mode to achieve its effect.
            ImageFillMode fillMode(Wrap_Clamp, Sample_Linear);
            if (dmFilter->Mode == DisplacementMapFilter::DisplacementMode_Wrap)
                fillMode = ImageFillMode(Wrap_Repeat, Sample_Linear);

            NativeTexture* pmaptex = (NativeTexture*)dmFilter->DisplacementMap->GetTexture(0);
            psi->SetTexture(pso, Uniforms::SU_maptex, pmaptex, fillMode);

            float compx[4], compy[4], mapScale[4], scale[2];
            memset(compx, 0, sizeof(compx));
            memset(compy, 0, sizeof(compy));
            memset(mapScale, 0, sizeof(mapScale));
            memset(scale, 0, sizeof(scale));

            switch(dmFilter->ComponentX)
            {
            default: SF_DEBUG_WARNING1(1, "Unknown ComponentX in DisplacementMapFilter (%d). Default to Red channel.", dmFilter->ComponentX);
            case DrawableImage::Channel_Red:    compx[0] = 1; break;
            case DrawableImage::Channel_Green:  compx[1] = 1; break;
            case DrawableImage::Channel_Blue:   compx[2] = 1; break;
            case DrawableImage::Channel_Alpha:  compx[3] = 1; break;
            }
            switch(dmFilter->ComponentY)
            {
            default: SF_DEBUG_WARNING1(1, "Unknown ComponentY in DisplacementMapFilter (%d). Default to Red channel.", dmFilter->ComponentY);
            case DrawableImage::Channel_Red:    compy[0] = 1; break;
            case DrawableImage::Channel_Green:  compy[1] = 1; break;
            case DrawableImage::Channel_Blue:   compy[2] = 1; break;
            case DrawableImage::Channel_Alpha:  compy[3] = 1; break;
            }
            mapScale[0] = ((float)ptexture->GetSize().Width / pmaptex->GetSize().Width);
            mapScale[1] = ((float)ptexture->GetSize().Height / pmaptex->GetSize().Height);
            mapScale[2] = dmFilter->MapPoint.x / pmaptex->GetSize().Width;
            mapScale[3] = dmFilter->MapPoint.y / pmaptex->GetSize().Height;

            scale[0] = dmFilter->ScaleX / ptexture->GetSize().Width;
            scale[1] = dmFilter->ScaleY / ptexture->GetSize().Height;

            psi->SetUniform(pso, Uniforms::SU_compx, compx, 4);
            psi->SetUniform(pso, Uniforms::SU_compy, compy, 4);
            psi->SetUniform(pso, Uniforms::SU_mapScale, mapScale, 4);
            psi->SetUniform(pso, Uniforms::SU_scale, scale, 2);

            if (dmFilter->Mode == DisplacementMapFilter::DisplacementMode_Color)
            {
                float boundColor[4];
                dmFilter->ColorValue.GetRGBAFloat(boundColor);
                psi->SetUniform(pso, Uniforms::SU_boundColor, boundColor, 4);
            }
        }

        psi->Finish(1);

        return true;
    }

    static const int MaximumBlurKernel = 32 * 20 * 20; // in twips^2
    static const int MaximumQuality      = 15;
    static const int MaximumFilterPasses = 8 * MaximumQuality;

    // Returns the number of passes required to render the given filter from scratch. Fills out the passes
    // array (which must contain at least MaximumPasses elements), with the shaders used per-pass.
    unsigned GetFilterPasses(const Filter* filter, unsigned fillFlags, unsigned* passes ) const
    {
        unsigned passCount;
        if ( filter->GetFilterType() <= Filter_Blur_End )
        {
            BlurFilter* bfilter = (BlurFilter*)filter;
            const BlurFilterParams& params = bfilter->GetParams();
            bool box1 = false;

            // If the blur is too large, do two 1D blurs, instead of one 2D blur.
            if ( (params.BlurX * params.BlurY) >= (float)MaximumBlurKernel )
            {
                passCount = 2 * params.Passes;
                box1 = true;
            }
            else
                passCount = params.Passes;

            // Every pass except the last is simply a blur
            unsigned pass;
            for ( pass = 0; pass < Alg::Max<unsigned>(0,passCount-1); ++pass )
                passes[pass] = box1 ? ShaderDesc::ST_Box1Blur : ShaderDesc::ST_Box2Blur;

            FilterType type = (FilterType)(params.Mode & BlurFilterParams::Mode_FilterMask);
            switch( type )
            {
                default:
                case Filter_Blur:
                    passes[pass] = ShaderDesc::ST_start_blurs;

                    // Extra flags.
                    passes[pass] += box1 ? 0 : ShaderDesc::ST_blurs_Box2;
                    if ( fillFlags & FF_Multiply )
                        passes[pass] += (unsigned)ShaderDesc::ST_blurs_Mul;
                    break;

                case Filter_Glow:
                case Filter_Shadow:
                case Filter_Bevel:
                case Filter_GradientGlow:
                case Filter_GradientBevel:
                    switch(type)
                    {
                    case Filter_Glow:
                    case Filter_Shadow:
                        passes[pass] = ShaderDesc::ST_start_shadows + ShaderDesc::ST_shadows_ShadowBase + ShaderDesc::ST_shadows_SColor;
                        break;
                    case Filter_Bevel:
                        passes[pass] = ShaderDesc::ST_start_shadows + ShaderDesc::ST_shadows_BevelBase + ShaderDesc::ST_shadows_SColor2;
                        break;
                    case Filter_GradientGlow:
                        passes[pass] = ShaderDesc::ST_start_shadows + ShaderDesc::ST_shadows_ShadowBase + ShaderDesc::ST_shadows_SGrad;
                        break;
                    case Filter_GradientBevel:
                        passes[pass] = ShaderDesc::ST_start_shadows + ShaderDesc::ST_shadows_BevelBase + ShaderDesc::ST_shadows_SGrad2;
                        break;
                    default:
                        break;
                    }

                    // Choose the type of shadow (inner/outer/full).
                    if (params.Mode & BlurFilterParams::Mode_Inner)
                    {
                        // 'Inner' is different between bevel and shadow.
                        if ((passes[pass] & ShaderDesc::ST_shadows_BevelBase) || type == Filter_GradientGlow)
                            passes[pass] += ShaderDesc::ST_shadows_InnerBevel;
                        else
                            passes[pass] += ShaderDesc::ST_shadows_InnerShadow;
                    }
                    else if (params.Mode & BlurFilterParams::Mode_Highlight)
                        passes[pass] += ShaderDesc::ST_shadows_FullBevel;
                    else
                    {
                        // Special case: shadow+hide (and not knockout) = full + hide.
                        if (type == Filter_Shadow && 
                            (params.Mode & BlurFilterParams::Mode_HideObject) &&
                            (params.Mode & BlurFilterParams::Mode_Knockout) == 0)
                        {
                            passes[pass] += ShaderDesc::ST_shadows_FullBevel;
                        }
                        else
                        {
                            passes[pass] += ShaderDesc::ST_shadows_OuterBevel;
                            }
                    }

                    // Whether it has the original overlaid.
                    if ((params.Mode & BlurFilterParams::Mode_HideObject) ||
                        (params.Mode & BlurFilterParams::Mode_Knockout))
                        passes[pass] += ShaderDesc::ST_shadows_HideBase;
                    else
                        passes[pass] += ShaderDesc::ST_shadows_Base;

                    // Extra flags.
                    if ( fillFlags & FF_Multiply )
                        passes[pass] += (unsigned)ShaderDesc::ST_shadows_Mul;
                    break;
            }
        }
        else if (filter->GetFilterType() == Filter_ColorMatrix)
        {
            passCount = 1;
            ColorMatrixFilter* matrixFilter = (ColorMatrixFilter*)filter;
            float* params = &((float&)matrixFilter[0]);
            SF_UNUSED(params);
            passes[0] = ShaderDesc::ST_start_cmatrix;
            if ( fillFlags & FF_Multiply )
                passes[0] += (unsigned)ShaderDesc::ST_cmatrix_Mul;
        }
        else if (filter->GetFilterType() == Filter_DisplacementMap)
        {
            passCount = 1;
            passes[0] = ShaderDesc::ST_start_DisplacementMap;
            const DisplacementMapFilter* dmFilter = reinterpret_cast<const DisplacementMapFilter*>(filter);
            switch(dmFilter->Mode)
            {
            default: SF_DEBUG_WARNING1(1,"Unknown DisplacementMap mode (%d). Deafulting to Wrap.", dmFilter->Mode);
            case DisplacementMapFilter::DisplacementMode_Wrap:
            case DisplacementMapFilter::DisplacementMode_Clamp:
                break;
            case DisplacementMapFilter::DisplacementMode_Ignore:
                passes[0] += (unsigned)ShaderDesc::ST_DisplacementMap_DMIgnore;
                break;
            case DisplacementMapFilter::DisplacementMode_Color:
                passes[0] += (unsigned)ShaderDesc::ST_DisplacementMap_DMColor;
                break;
            }
        }
        else
        {
            // Report 0 passes. This should cause the HAL::drawUncachedFilter code to just use the
            // source texture as the destination.
            SF_DEBUG_ASSERT(filter->GetFilterType() == Filter_CacheAsBitmap, "Expected filter type to be CacheAsBitmap.");
            passCount = 0;
        }
        return passCount;
    }

    // Sets up the filter to be rendered. If there is a possibility that the platform does not
    // support dynamic loops in shaders, the BlurFilterState should be used to override the default
    // number of passes, and shader array in StaticShaderManagerType::GetFilterPasses.
    unsigned SetupFilter(const Filter* filter, unsigned fillFlags, unsigned* passes, BlurFilterState&) const
    {
        return GetFilterPasses(filter, fillFlags, passes);
    }

    // Set of flags that may be passed to various DrawableImage functions.
    enum DrawableImageFlags
    {
        CPF_HalfPixelOffset     = 0x1,      // Some platforms require a half-pixel offset to have pixel-centres match exactly (D3D9).
        CPF_InvertedViewport    = 0x2,      // Some platforms require their render target output to be flipped (GL).
    };

    // Sets a fill based on a CopyPixels BitmapData command.
    bool SetDrawableCopyPixelsFill( Render::Texture** tex, const Matrix2F* texgen, const Size<int> texsize, 
                                    const Matrix2F& mvp, bool mergeAlpha, bool destAlpha, const VertexFormat* pvf,
									ShaderInterface* psi, unsigned flags = 0 )
    {
        // Determine which shader to use, based on parameters.
		unsigned type = ShaderDesc::ST_start_DrawableCopyPixels;
        if ( tex[2] )
		{
            type += ShaderDesc::ST_DrawableCopyPixels_DrawableCopyPixelsAlpha;
		}
        if ( !destAlpha )
            type += ShaderDesc::ST_DrawableCopyPixels_NoDestAlpha;
		else if ( mergeAlpha )
			type += ShaderDesc::ST_DrawableCopyPixels_MergeAlpha;

        if (!psi->SetStaticShader((ShaderType)type, pvf ))
            return false;

        psi->BeginPrimitive();
        return DrawableFinish(2 + (tex[2] ? 1 : 0), tex, texgen, texsize, mvp, psi, flags);
    }

    // Sets a fill based on a CopyChannel or Merge BitmapData command.
    bool SetDrawableMergeFill(  Render::Texture** tex, const Matrix2F* texgen, const Size<int> texsize, 
                                const Matrix4F* cxmul,
                                const VertexFormat* pvf, ShaderInterface* psi, unsigned flags = 0 )
    {
        if (!psi->SetStaticShader(ShaderDesc::ST_DrawableMerge, pvf ))
            return false;
        psi->BeginPrimitive();
        const Shader& pso = psi->GetCurrentShaders();

        // Set the color matrices which will perform the operation.
        psi->SetUniform(pso, Uniforms::SU_cxmul, cxmul[0], 16);
        psi->SetUniform(pso, Uniforms::SU_cxmul1, cxmul[1], 16);

        // Make the mvp cover the entire viewport.
        Matrix2F mvp = Matrix2F::Scaling(2,-2) * Matrix2F::Translation(-0.5f, -0.5f);
        return DrawableFinish(2, tex, texgen, texsize, mvp, psi, flags);
    }

    bool SetDrawableCxform( Render::Texture** tex, const Matrix2F* texgen, const Size<int> texsize, 
                            const Cxform* cx,
                            const VertexFormat* pvf, ShaderInterface* psi, unsigned flags = 0 )
    {
        if (!psi->SetStaticShader(ShaderDesc::ST_TexTGCxform, pvf ))
            return false;
        psi->BeginPrimitive();
        const Shader& pso = psi->GetCurrentShaders();

        psi->SetCxform(pso, *cx);

        // Make the mvp cover the entire viewport.
        Matrix2F mvp = Matrix2F::Scaling(2,-2) * Matrix2F::Translation(-0.5f, -0.5f);
        return DrawableFinish(1, tex, texgen, texsize, mvp, psi, flags);
    }

    bool SetDrawableCompare( Render::Texture** tex, const Matrix2F* texgen, const Size<int> texsize, 
                             const VertexFormat* pvf, ShaderInterface* psi, unsigned flags = 0 )
    {
        if (!psi->SetStaticShader(ShaderDesc::ST_DrawableCompare, pvf ))
            return false;
        psi->BeginPrimitive();

        // Make the mvp cover the entire viewport.
        Matrix2F mvp = Matrix2F::Scaling(2,-2) * Matrix2F::Translation(-0.5f, -0.5f);
        return DrawableFinish(2, tex, texgen, texsize, mvp, psi, flags);
    }

    bool SetDrawablePaletteMap( Render::Texture** tex, const Matrix2F* texgen, const Size<int> texsize, 
                                const Matrix2F& mvp, Render::Texture* paletteMap,
                                const VertexFormat* pvf, ShaderInterface* psi, unsigned flags = 0 )
    {
        if (!psi->SetStaticShader(ShaderDesc::ST_DrawablePaletteMap, pvf ))
            return false;
        psi->BeginPrimitive();
        const Shader& pso = psi->GetCurrentShaders();

        // Set the palette map texture.
        psi->SetTexture(pso, Uniforms::SU_srctex, paletteMap, ImageFillMode(Wrap_Clamp, Sample_Point));

        return DrawableFinish(1, tex, texgen, texsize, mvp, psi, flags);
    }

    // This function is used by many BitmapData commands. It assumes that one of the SetDrawable* functions
    // has already been called.
    bool DrawableFinish    (  unsigned srcCount, 
                              Render::Texture** tex, const Matrix2F* texgen, const Size<int> texsize, 
                              const Matrix2F& mvpOriginal,
                              ShaderInterface* psi, unsigned flags = 0 )
    {
        const Shader& pso = psi->GetCurrentShaders();

        // Caculate the MVP, should be the whole viewport.
        Matrix2F mvp = mvpOriginal;

        if ( flags & CPF_InvertedViewport )
        {
            mvp.PrependTranslation(0.0f, 1.0f);
            mvp.PrependScaling(1.0f, -1.0f);
        }
        if ( flags & CPF_HalfPixelOffset )
        {
            mvp.Tx() -= 1.0f/texsize.Width;   // D3D9 1/2 pixel center offset
            mvp.Ty() += 1.0f/texsize.Height;
        }

        psi->SetMatrix (pso, Uniforms::SU_mvp,      mvp );

        for ( unsigned i =0; i < srcCount; ++i )
        {
            psi->SetTexture(pso, Uniforms::SU_tex, tex[i], ImageFillMode(Wrap_Clamp, Sample_Point), i);
            psi->SetMatrix(pso, Uniforms::SU_texgen, texgen[i], i);
        }
        psi->Finish(1);
        return true;
    }

    void SetBlendFill(BlendMode mode, const Matrix2F& mvp, const Cxform& cx, Render::Texture** ptextures, const Matrix2F* texgen, 
        const VertexFormat* pvf, ShaderInterface* psi)
    {
        ShaderType shaderType = ShaderDesc::ST_Solid;
        unsigned srcTexUniform = Uniforms::SU_srctex;
        switch(mode)
        {
            default:                SF_DEBUG_WARNONCE1(1, "Unexpected blend mode (%d) in SetBlendFill (should be done with GPU blending)", mode); break;
            case Blend_Layer:       
                // NOTE: If an alpha target is not included, it means that this was a layer blend mode, which didn't use any
                // alpha or erase operations, but it still rendering to a temporary target. This may happen if they layer has
                // an alpha Cxform applied to it. In this case, the alpha layer should be full alpha, so use a different shader
                // to accomplish this.
                if (ptextures[Target_Alpha] == 0)
                {
                    SF_DEBUG_WARNONCE(ptextures[Target_Destination] != 0, "Unexpected 'destination' texture provided.");
                    shaderType = ShaderDesc::ST_TexTGCxformAc;
                    srcTexUniform = Uniforms::SU_tex;
                }
                else
                {
                    shaderType = ShaderDesc::ST_BlendLayer; 
                }
                break;
            case Blend_Lighten:     shaderType = ShaderDesc::ST_BlendLighten; break;
            case Blend_Darken:      shaderType = ShaderDesc::ST_BlendDarken; break;
            case Blend_Difference:  shaderType = ShaderDesc::ST_BlendDifference; break;
            case Blend_Overlay:     shaderType = ShaderDesc::ST_BlendOverlay; break;
            case Blend_HardLight:   shaderType = ShaderDesc::ST_BlendHardlight; break;
        }

        psi->SetStaticShader(shaderType, pvf);
        psi->BeginPrimitive();
        const Shader& pso = psi->GetCurrentShaders();

        // Set the palette map texture.
        psi->SetMatrix(pso, Uniforms::SU_mvp, mvp);
        psi->SetMatrix(pso, Uniforms::SU_texgen, texgen[Target_Source], 0);
        psi->SetTexture(pso, srcTexUniform, ptextures[Target_Source], ImageFillMode(Wrap_Clamp, Sample_Point));

        if (ptextures[Target_Destination])
        {
            psi->SetMatrix(pso, Uniforms::SU_texgen, texgen[Target_Destination], 1);
            psi->SetTexture(pso, Uniforms::SU_dsttex, ptextures[Target_Destination], ImageFillMode(Wrap_Clamp, Sample_Point));
        }

        if (ptextures[Target_Alpha])
        {
            psi->SetMatrix(pso, Uniforms::SU_texgen, texgen[Target_Alpha], 1);
            psi->SetTexture(pso, Uniforms::SU_alphatex, ptextures[Target_Alpha], ImageFillMode(Wrap_Clamp, Sample_Point));
        }

        if (mode == Blend_Layer)
        {
            psi->SetCxform(pso, cx);
        }
        psi->Finish(1);
    }

    // BeginScene/EndScene stub. ShaderManager derived classes can implement these methods, to perform operations
    // that should happen at the beginning/end of a scene.
    void BeginScene() { }
    void EndScene()   { }

};

template<class Builder>
void BuildVertexArray(const VertexFormat* pfmt, Builder& output)
{
    const VertexElement* pve = pfmt->pElements;
    int vi = 0;
    for (; pve->Attribute != VET_None; pve++, vi++)
    {
        unsigned offset = pve->Offset;
        int ac = (pve->Attribute & VET_Components_Mask);

        // Packed factors
        if (((pve[0].Attribute | pve[1].Attribute) & (VET_Usage_Mask|VET_Index_Mask)) == (VET_Color | (3 << VET_Index_Shift)))
        {
            ac = 4;
            SF_ASSERT(pve[1].Offset == offset+3);
            pve++;
        }
        // Factors with Batch index
        else if (((pve[1].Attribute & VET_Usage_Mask) == VET_Instance) &&
            ((pve[0].Attribute | pve[2].Attribute) & (VET_Usage_Mask|VET_Index_Mask)) == (VET_Color | (3 << VET_Index_Shift)))
        {
            ac = 4;
#ifndef SF_OS_XBOX360
            SF_ASSERT(pve[1].Offset == offset+2);
            SF_ASSERT(pve[2].Offset == offset+3);
#endif
            pve+=2;
        }

        output.Add(vi, pve->Attribute, ac, offset);
    }

    output.Finish(vi);
}

}}

#endif
