/**************************************************************************

Filename    :   Wii_Shader.h
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_WII_Fill_H
#define INC_SF_WII_Fill_H

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_Hash.h"
#include "Kernel/SF_Debug.h"

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

#include <revolution/gx.h>
#include <revolution/mtx.h>

namespace Scaleform { namespace Render { namespace Wii {

class HAL;
class Texture;

struct Shader
{
    unsigned   NumTex, NumTexMatrix, NumMatrices, NumStages;

    unsigned   BatchMatrixSize;
    bool       IsComplexMesh;

    void BeginPrimitive() { NumStages = NumTex = NumTexMatrix = 0; BatchMatrixSize = 0; NumMatrices = 0; IsComplexMesh = 0; }
};

struct Uniform
{
    enum Uniforms
    {
        SU_mvp = 0,
        SU_texgen = 1,
        SU_cxmul,
    };
};

enum
{
    VE_Pos,
    VE_Dummy1,
    VE_Color,
    VE_Factor,
    VE_TexCoord
};

struct SysVertexFormat : public Render::SystemVertexFormat
{
    int      vfn;
    unsigned NumMatrices, IndexSize, ExtraSize;
    bool     IsComplexMesh;

    SysVertexFormat(int n, unsigned m, unsigned i, bool cm, unsigned ex)
        : vfn(n), NumMatrices(m), IndexSize(i), ExtraSize(ex), IsComplexMesh(cm) {}
};

struct StaticVertexFormat
{
    unsigned Formats[8];
    bool     IsSet;
};

class ShaderManager
{
public:
    HAL*                                                   pHal;
    MultiKeyCollection<VertexElement, VertexFormat, 32>    VFormats;
    StaticVertexFormat                                     SFormats[7];
    Shader                                                 CurShader;
    const ProfileViews*                                    Profiler;


    ShaderManager();
    void Initialize(HAL* phal, const ProfileViews* pProfiler);

    void SetColor(const Shader& sd, Color c)
    {
        GXColor gc;
        gc.r = c.GetRed();
        gc.g = c.GetGreen();
        gc.b = c.GetBlue();
        gc.a = c.GetAlpha();

        GXSetChanMatColor(GX_COLOR0A0, gc);
    }

    void SetCxform(const Shader& sd, const Cxform& cx) const
    {
        GXColor cm;
        cm.r = cx.M[0][0] * 255;
        cm.g = cx.M[0][1] * 255;
        cm.b = cx.M[0][2] * 255;
        cm.a = cx.M[0][3] * 255;
        
        GXColorS10 ca;
        ca.r = cx.M[1][0] * 255;
        ca.g = cx.M[1][1] * 255;
        ca.b = cx.M[1][2] * 255;
        ca.a = cx.M[1][3] * 255;

        GXSetTevColorS10(GX_TEVREG0, ca);
        GXSetTevColor(GX_TEVREG1, cm);
    }

    inline void SetMatrix(const Shader& sd, unsigned var, Matrix2F m, unsigned index = 0, unsigned batch = 0);
    inline void SetMatrix(const Shader& sd, unsigned var, Matrix2F m1, HMatrix m2, const Render::MatrixState* matrices,
                   unsigned index = 0, unsigned batch = 0);

    VertexFormat* GetVertexFormat(VertexElement* pelements, unsigned count, unsigned size)
    {
        VertexFormat  *pformat   = VFormats.Find(pelements, count);
        if (pformat)
            return pformat;

        pformat = VFormats.Add(&pelements, pelements, count);
        if (!pformat)
            return 0;

        pformat->Size      = size;
        pformat->pElements = pelements;
        return pformat;
    }

    void                BeginScene();
    void                BeginPrimitive() { CurShader.BeginPrimitive(); }
    void                Finish(unsigned meshCount) { }

    Shader              GetCurrentShaders() const { return CurShader; }

    void                SetTexture(Shader& sp, unsigned stage, Texture* ptexture, ImageFillMode fm);

    void                SetStreamedBatch(const Shader& shader, int batchIndex)
    {
        UInt32    basematrix = batchIndex * shader.BatchMatrixSize;
        UInt32    mtx0 = basematrix;

        if (shader.NumTexMatrix)
            mtx0 |= ((basematrix+3) << 6) + ((basematrix+5) << 12);
        else
            mtx0 |= (60 << 6) + (60 << 12);

        GXColor1x16(0x0830);
        GXColor1u32(mtx0);
        GXColor1x8(0x10);
        GXColor1u32(0x00011018);
        GXColor1u32(mtx0);
        GXColor1u32(0);
    }

    void    SetVertexAttr(int n, int Attr, bool hasColor);

    int     SetVertexArray(GXVtxFmt vf, const VertexFormat* pvf, UByte* pvertexData);

    void    MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                            const VertexFormat** single, const VertexFormat** batch, const VertexFormat** instanced, unsigned meshType);

    void    SetSysVertexFormat(VertexFormat* pvf, unsigned indexsize, unsigned numMatrices, bool iscomplex, unsigned extras);

    Shader  SetPrimitiveFill(PrimitiveFill* pfill, unsigned& fillFlags, unsigned batchType, const VertexFormat* pFormat,
                             unsigned meshCount, const Render::MatrixState* halMatrices, const Primitive::MeshEntry* pmeshes);

    Shader  SetFill(PrimitiveFill* pfill, PrimitiveFillType fillType, unsigned fillFlags, unsigned batchType,
                    const VertexFormat* pFormat);

    // Filters
    unsigned GetFilterPasses(const Filter* filter, unsigned fillFlags, unsigned* passes ) const;
};

}}}

#endif
