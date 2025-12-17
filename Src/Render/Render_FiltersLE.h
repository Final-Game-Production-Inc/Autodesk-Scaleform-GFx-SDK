/**************************************************************************

Filename    :   Render_FilterLE.h
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_FilterLE_H
#define INC_SF_Render_FilterLE_H

#include "Render/Render_Filters.h"

namespace Scaleform { namespace Render {

struct BlurFilterState
{
    int                     MaxTexOps;
    int                     Passes;
    bool                    Prims, UsesOriginal;
    BlurFilterParams        PassParams[3];

    // updated for each pass
    float                   SizeX, SizeY;
    int                     Samples, Quads, SamplesPerQuad;
    const BlurFilterParams* CurPass;
    int                     BoxTCs, BaseTCs, TotalTCs, VertexAttrs;

    BlurFilterState(int maxTex = 8) : MaxTexOps(maxTex), Passes(0), Prims(0), UsesOriginal(0) {}

    bool Setup(const Filter* filter)
    {
        int filterType = filter->GetFilterType();

        if (filterType < Filter_Blur || filterType > Filter_Blur_End)
            return false;

        BlurFilter* bfilter = (BlurFilter*)filter;
        const BlurFilterParams& params = bfilter->GetParams();

        UsesOriginal = (filterType != Filter_Blur);
        unsigned n = params.Passes;

        PassParams[0] = params;
        if ((params.Mode & BlurFilterParams::Mode_FilterMask) == Filter_Glow)
            PassParams[0].Mode = (PassParams[0].Mode & BlurFilterParams::Mode_FlagsMask) | Filter_Shadow;
        PassParams[0].BlurX = TwipsToPixels(params.BlurX);
        PassParams[0].BlurY = TwipsToPixels(params.BlurY);
        PassParams[0].Offset.x = TwipsToPixels(params.Offset.x);
        PassParams[0].Offset.y = TwipsToPixels(params.Offset.y);
        PassParams[1] = PassParams[0];
        PassParams[2] = PassParams[0];

        if (PassParams[0].BlurX * PassParams[0].BlurY > MaxTexOps)
        {
            n *= 2;
            PassParams[0].Mode &= BlurFilterParams::Mode_FlagsMask;
            PassParams[0].Mode |= Filter_Blur;
            PassParams[1].Mode &= BlurFilterParams::Mode_FlagsMask;
            PassParams[1].Mode |= Filter_Blur;
            PassParams[0].BlurY = 1;
            PassParams[1].BlurX = 1;
            PassParams[2].BlurX = 1;
        }
        else if ((params.Mode & BlurFilterParams::Mode_FilterMask) != Filter_Blur)
        {
            PassParams[0].Mode &= BlurFilterParams::Mode_FlagsMask;
            PassParams[0].Mode |= Filter_Blur;
            PassParams[1].Mode &= BlurFilterParams::Mode_FlagsMask;
            PassParams[1].Mode |= Filter_Blur;
        }

        Prims = 0;
        if (PassParams[2].BlurX*PassParams[2].BlurY > (UsesOriginal ? MaxTexOps - 1 : MaxTexOps))
        {
            n++;
            Prims = 1;
            PassParams[2].BlurX = PassParams[2].BlurY = 1;
        }
        PassParams[0].Offset.x = PassParams[0].Offset.y = PassParams[1].Offset.x = PassParams[1].Offset.y = 0;
        Passes = n;

        return true;
    }

    void SetPass(int pass)
    {
        unsigned passi = (pass == Passes-1) ? 2 : (pass&1);
        const BlurFilterParams& pparams = PassParams[passi];

        CurPass = &pparams;
        SizeX = unsigned(pparams.BlurX-1) * 0.5f;
        SizeY = unsigned(pparams.BlurY-1) * 0.5f;
        Samples = unsigned((SizeX*2+1)*(SizeY*2+1));
        Quads = (Samples + MaxTexOps - 1) / MaxTexOps;
        SamplesPerQuad = (Samples > MaxTexOps) ? MaxTexOps : Samples;

        TotalTCs = MaxTexOps;
        BaseTCs = 0;
        if (pparams.Mode & Filter_Shadow)
            BaseTCs++;
        BoxTCs = TotalTCs - BaseTCs;
        if (pparams.Mode & BlurFilterParams::Mode_Highlight)
        {
            BoxTCs >>= 1;
            TotalTCs = BoxTCs*2 + BaseTCs;
        }
        if (BoxTCs > Samples)
        {
            BoxTCs = Samples;
            TotalTCs = BoxTCs * ((pparams.Mode & BlurFilterParams::Mode_Highlight) ? 2 : 1) + BaseTCs;
        }
        VertexAttrs = 1 + TotalTCs;
    }

    int GetVertexBufferSize() const
    {
        int vc = 1 + TotalTCs;

        return vc * 2 * 6 * Quads;
    }

    template<class VertexFunc>
    void GetVertices(const Rect<int>& srcrect, const Rect<int>& destrect, VertexFunc& vf) const
    {
        SF_ASSERT(Quads > 0);
        SF_ASSERT(SamplesPerQuad > 0 && SamplesPerQuad <= MaxTexOps);
        SF_ASSERT(Quads == 1 || Prims);

        const float vertices[] =   {(float)destrect.x1, (float)destrect.y1, (float)srcrect.x1, (float)srcrect.y1,
                                    (float)destrect.x2, (float)destrect.y1, (float)srcrect.x2, (float)srcrect.y1,   
                                    (float)destrect.x1, (float)destrect.y2, (float)srcrect.x1, (float)srcrect.y2,
                                    (float)destrect.x2, (float)destrect.y2, (float)srcrect.x2, (float)srcrect.y2,
                                    (float)destrect.x1, (float)destrect.y2, (float)srcrect.x1, (float)srcrect.y2,
                                    (float)destrect.x2, (float)destrect.y1, (float)srcrect.x2, (float)srcrect.y1};

        float x = -SizeX, y = -SizeY;
        for (int p = 0; p < Quads; p++)
        {
            float xv[16], yv[16];
            for (int tc = 0; tc < SamplesPerQuad; tc++)
                if (y <= SizeY)
                {
                    xv[tc] = x - CurPass->Offset.x;
                    yv[tc] = y - CurPass->Offset.y;
                    x++;
                    if (x > SizeX)
                    {
                        x = -SizeX;
                        y++;
                    }
                }
                else
                    xv[tc] = yv[tc] = 1e10;

            for (unsigned j = 0; j < 6; j++)
            {
                vf.Position(vertices[j*4], vertices[j*4+1]);

                int tci = 0;
                for (int tc = 0; tc < BoxTCs; tc++)
                {
                    vf.TexCoord(tci++, (xv[tc] < 1e9 ? vertices[j*4+2] + xv[tc] : 0), (yv[tc] < 1e9 ? vertices[j*4+3] + yv[tc] : 0));

                    if (CurPass->Mode & BlurFilterParams::Mode_Highlight)
                        vf.TexCoord(tci++, (xv[tc] < 1e9 ? vertices[j*4+2] - xv[tc] : 0), (yv[tc] < 1e9 ? vertices[j*4+3] - yv[tc] : 0));
                }
                if (BaseTCs)
                    vf.TexCoord(tci++, vertices[j*4+2], vertices[j*4+3]);
            }
        }
    }
};

struct BlurFilterShaderKey
{
    // make sure these member variables continue to stay without padding
    // Padding breaks the Hash function that uses this struct as a key
    unsigned Mode;
    int      BoxTCs, BaseTCs, TotalTCs;

    BlurFilterShaderKey(const BlurFilterState& state)
        : Mode(state.CurPass->Mode), BoxTCs(state.BoxTCs), BaseTCs(state.BaseTCs), TotalTCs(state.TotalTCs) {}

    bool operator==(const BlurFilterShaderKey& other) const
    {
        return Mode == other.Mode && BoxTCs == other.BoxTCs && BaseTCs == other.BaseTCs && TotalTCs == other.TotalTCs;
    }
};

template<typename ShaderType>
struct BlurFilterShaderDesc
{
    int         pos, tc[16];
    int         mvp, cxadd, cxmul, offset, samples, scolor[2], tex[2], texscale[2];
    ShaderType  Shader;
};

struct VertexFunc_Buffer
{
    float*      pvertices;

    VertexFunc_Buffer(float* pv) : pvertices(pv) {}

    void Position(float x, float y)
    {
        (*pvertices++) = x;
        (*pvertices++) = y;
    }

    void TexCoord(int, float x, float y)
    {
        (*pvertices++) = x;
        (*pvertices++) = y;
    }
};

}}
#endif
