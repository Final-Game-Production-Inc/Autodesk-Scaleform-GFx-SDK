/**********************************************************************

PublicHeader:   Render
Filename    :   Render_Profiler.h
Content     :   
Created     :   2012/08/16
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_Render_Profiler_H
#define INC_SF_Render_Profiler_H

#include "Render/Render_Color.h"
#include "Render/Render_States.h"
#include "Render/Render_Primitive.h"

namespace Scaleform { namespace Render {

class BeginDisplayData;
class Primitive;

// The 'types' of things that can be drawn 
enum DrawModeType
{
    DrawMode_Fill,    // Regular primitives.
    DrawMode_Mask,    // Masking primitives
    DrawMode_CachedPrim,  // Filter primitives
    DrawMode_Clear,   // Clearing draw calls.
    DrawMode_Count
};

enum ProfilerModes
{
    Profile_None,                      // Renders normally.
    Profile_Overdraw,                  // Renders all primitives with low alpha and solid shapes, so that frequently overdrawn areas appear brightly.
    Profile_Batch,                     // Colors all primitives rendered in the same batch as the same color.
    Profile_Blending,                  // Renders primitives with blending distinctly from non-blending primitives
    Profile_TextureDensity,            // Renders all shapes with a relative measure of texture sampling density (non-textured shapes are invisible).
    Profile_Count,                     // The number of profile modes available.

    Profile_All = 0xFFFFFFFF,          // Bitwise mask for all modes (used with availability functions).
};

enum ProfilerFlags
{
    ProfileFlag_NoFilterCaching    = 0x1,           // Filter caching does not occur; useful for debugging filter errors.
    ProfileFlag_NoBlendCaching     = 0x2,           // Blendmode caching does not occur; useful for debugging blendmode errors.
    ProfileFlag_NoBatching         = 0x4,           // Disable batching (also clears MeshCache, so that any previously batched items will become individual DPs)
    ProfileFlag_NoInstancing       = 0x8,           // Disable instancing (also clears MeshCache, so that any previously instanced items will become individual DPs)
    ProfileFlag_All                = 0xFFFFFFFF,    // Bitwise mask for all flags (used with availability functions).
};

enum ProfilerUniform
{
    ProfileUniform_TextureSize,         // float2, size of a texture.
    ProfileUniform_TextureMips,         // float, number of mips in a texture.
};


// Determines behavior of certain operations while executing drawing. Within the ProfilerHAL, there is
// an active ProfileModifier. The implementation of an overridden ProfilerModifier determines how the
// scene is rendered, differently from the regular method. The base implementation performs normal rendering.
class ProfileModifier : public NewOverrideBase<StatRender_Mem>
{
public:
                                ProfileModifier() : FillFlags(0)                                     { }
    virtual                     ~ProfileModifier()                                                   { }
    virtual Color               GetClearColor(DrawModeType, Color c) const                           { return c; }
    virtual BlendMode           GetBlendMode(DrawModeType, BlendMode mode) const                     { return mode; }
    virtual PrimitiveFillType   GetFillType(DrawModeType, PrimitiveFillType fill) const              { return fill; }
    virtual Color               GetColor(DrawModeType, Color color) const                            { return color; }
    virtual bool                IsCxformChanged(DrawModeType) const                                  { return true; }
    virtual Cxform              GetCxform(DrawModeType, const Cxform& cx) const                      { return cx; }
    virtual void                SetFillFlags(DrawModeType, unsigned flags)                           { FillFlags = flags; }
    virtual void                SetBatch(DrawModeType, HAL* phal, Primitive* prim, unsigned index)   { SF_UNUSED3(phal, prim, index); }
    virtual void                SetBatch(DrawModeType, HAL* phal, ComplexMesh* prim, unsigned index) { SF_UNUSED3(phal, prim, index); }
    virtual bool                ShouldDrawMaskVisible() const                                        { return false; }
    virtual bool                ShouldDrawCachedPrimArea() const                                     { return false; }
    virtual void                BeginScene()                                                         { }
protected:
    unsigned FillFlags; // Current fillflags for the HAL.
};

#if defined(SF_RENDERER_PROFILE)
class ProfileModifierOverdraw : public ProfileModifier
{
public:
    ProfileModifierOverdraw();

    virtual Color               GetClearColor(DrawModeType, Color c) const;
    virtual BlendMode           GetBlendMode(DrawModeType, BlendMode mode) const;
    virtual PrimitiveFillType   GetFillType(DrawModeType, PrimitiveFillType fill) const;
    virtual Color               GetColor(DrawModeType, Color color) const;
    virtual bool                IsCxformChanged(DrawModeType) const;
    virtual Cxform              GetCxform(DrawModeType, const Cxform& cx) const;
    virtual bool                ShouldDrawMaskVisible() const     { return true; }
    virtual bool                ShouldDrawCachedPrimArea() const  { return true; }

private:
    Cxform                      DrawModeCx[DrawMode_Count];     // Color transforms applied, based on the DrawMode.
};

class ProfileModifierBatch : public ProfileModifier
{
public:
    ProfileModifierBatch();

    virtual Color               GetClearColor(DrawModeType, Color c) const;
    virtual PrimitiveFillType   GetFillType(DrawModeType, PrimitiveFillType fill) const;
    virtual Color               GetColor(DrawModeType, Color color) const;
    virtual bool                IsCxformChanged(DrawModeType) const;
    virtual Cxform              GetCxform(DrawModeType, const Cxform& cx) const;
    virtual void                SetBatch(DrawModeType, HAL* phal, Primitive* prim, unsigned index);
    virtual void                SetBatch(DrawModeType, HAL* phal, ComplexMesh* prim, unsigned index);
    virtual void                SetHighlightedBatch(int highlight) { HighlightBatch = highlight; }
    virtual bool                ShouldDrawMaskVisible() const;
    virtual bool                ShouldDrawCachedPrimArea() const  { return true; }
    virtual void                BeginScene();
protected:
    void                        setBatchHelper( DrawModeType type, void* prim, unsigned index );

    Color                       NextBatchColor;
    int                         SceneBatchCount;
    bool                        MaskStencilRender;
    int                         HighlightBatch;
};

class ProfileModifierBlending : public ProfileModifier
{
public:
    virtual Color               GetClearColor(DrawModeType, Color c) const;
    virtual BlendMode           GetBlendMode(DrawModeType, BlendMode mode) const;
    virtual PrimitiveFillType   GetFillType(DrawModeType, PrimitiveFillType fill) const;
    virtual Color               GetColor(DrawModeType, Color color) const;
    virtual bool                IsCxformChanged(DrawModeType) const;
    virtual Cxform              GetCxform(DrawModeType, const Cxform& cx) const;
    virtual bool                ShouldDrawCachedPrimArea() const  { return true; }
};

class ProfileModifierTDensity : public ProfileModifier
{
public:
    virtual Color               GetClearColor(DrawModeType, Color c) const;
    virtual BlendMode           GetBlendMode(DrawModeType, BlendMode mode) const;
    virtual Color               GetColor(DrawModeType, Color color) const;
    virtual bool                IsCxformChanged(DrawModeType) const;
    virtual Cxform              GetCxform(DrawModeType, const Cxform& cx) const;
    virtual void                SetBatch(DrawModeType, HAL* phal, Primitive* prim, unsigned index);
    virtual void                SetBatch(DrawModeType, HAL* phal, ComplexMesh* prim, unsigned index);
    virtual bool                ShouldDrawCachedPrimArea() const  { return true; }
protected:
    void                        setUniforms(HAL *phal, PrimitiveFill* pfill);
};
#endif // SF_RENDERER_PROFILE

class ProfileViews : public NewOverrideBase<StatRender_Mem>
{
public:
    enum ProfileFlags
    {
        Channel_Red = 0,
        Channel_Green = 16,
        Channel_Blue = 32,
        Channel_Flags = 48,
        Profile_Fill = 0x100,
        Profile_Mask = 0x200,
        Profile_Filter = 0x400,
        Profile_Batch = 0x800,
        Profile_Blend = 0x1000,

        ProfileFlag_NoFilterCaching = 0x00000001,   // Flag that disables caching of filters (sent in Channel_Flags).
    };

    virtual ~ProfileViews()
    {
    }

#if defined(SF_RENDERER_PROFILE)
    ProfileViews() :
        ProfileMode(Profile_None),
        NextProfileMode(Profile_None),
        ProfileFlags(0),
        NextProfileFlags(0),
        HighlightBatch(-1),
        CurrentDrawMode(DrawMode_Count),
        AvailableModes(0),
        AvailableFlags(0)
    {
    }

    // New
    static Color         GetColorForBatch(UPInt base, unsigned index);

    // Determine whether a mode or flag is available (it can be achieved correctly by the current renderer).
    // Before InitHAL is called, the Profiler (from Render::HAL::GetProfiler) reports that no modes or flags are available.
    bool                 IsModeAvailable(ProfilerModes mode) const     { return (AvailableModes & (1<<mode)) != 0; }
    bool                 IsFlagAvailable(unsigned flag) const          { return (AvailableFlags & flag) != 0; }

    // Sets the bit-mask available of the ProfilerModes and ProfilerFlags. These are intended for internal use only within InitHAL.
    void                 SetModeAvailability(unsigned modes)           { AvailableModes = modes; }
    void                 SetFlagAvailability(unsigned flags)           { AvailableFlags = flags; }

    void                 SetProfileMode(ProfilerModes mode)            { NextProfileMode = mode; }
    ProfilerModes        GetProfileMode() const                        { return ProfileMode; }
    void                 SetProfileFlags(unsigned flags)               { NextProfileFlags = flags; }
    void                 SetProfileFlag(unsigned flag, bool state);
    unsigned             GetProfileFlag(unsigned flag) const           { return NextProfileFlags & flag;}
    void                 SetHighlightedBatch(unsigned batch)           { HighlightBatch = batch; }
    virtual DrawModeType GetDrawMode() const                           { return CurrentDrawMode; }
    virtual void         SetDrawMode(DrawModeType mode)                { CurrentDrawMode = mode; }
    virtual bool         IsFilterCachingEnabled() const                { return (ProfileFlags & ProfileFlag_NoFilterCaching) == 0; }
    virtual bool         IsBlendCachingEnabled() const                 { return (ProfileFlags & ProfileFlag_NoBlendCaching) == 0; }

    // Interface to the current ProfileModifier
    virtual const ProfileModifier&  GetCurrentProfileModifier() const;
    virtual ProfileModifier&        GetCurrentProfileModifier();
    virtual Color                   GetClearColor(Color c) const                            { return GetCurrentProfileModifier().GetClearColor(CurrentDrawMode, c); }
    virtual BlendMode               GetBlendMode(BlendMode mode) const                      { return GetCurrentProfileModifier().GetBlendMode(CurrentDrawMode, mode); }
    virtual PrimitiveFillType       GetFillType(PrimitiveFillType fill) const               { return GetCurrentProfileModifier().GetFillType(CurrentDrawMode, fill); }
    virtual Color                   GetColor(Color color) const                             { return GetCurrentProfileModifier().GetColor(CurrentDrawMode, color); }
    virtual bool                    IsCxformChanged() const                                 { return GetCurrentProfileModifier().IsCxformChanged(CurrentDrawMode); }
    virtual Cxform                  GetCxform(const Cxform& cx) const                       { return GetCurrentProfileModifier().GetCxform(CurrentDrawMode, cx); }
    virtual void                    SetFillFlags(unsigned flags)                            { return GetCurrentProfileModifier().SetFillFlags(CurrentDrawMode, flags); }
    virtual void                    SetBatch(HAL* phal, Primitive* prim, unsigned index)    { return GetCurrentProfileModifier().SetBatch(CurrentDrawMode, phal, prim, index); }
    virtual void                    SetBatch(HAL* phal, ComplexMesh* prim, unsigned index)  { return GetCurrentProfileModifier().SetBatch(CurrentDrawMode, phal, prim, index); }
    virtual bool                    ShouldDrawMaskVisible() const                           { return GetCurrentProfileModifier().ShouldDrawMaskVisible(); }
    virtual bool                    ShouldDrawCachedPrimArea() const                        { return GetCurrentProfileModifier().ShouldDrawCachedPrimArea(); }

    // Sets the next cached profile mode/flags. Modes and flags cannot be change inside a scene, this should only be called outside
    // of a HAL::BeginScene/EndScene bracket. 
    void                            BeginScene(HAL* phal);

#else   // SF_RENDERER_PROFILE

    // If SF_RENDERER_PROFILE is not defined, then remove all functionality from this class. The compiler should optimize the calls out.

    bool                 IsModeAvailable(ProfilerModes mode) const     { SF_UNUSED(mode); return false; }
    bool                 IsFlagAvailable(unsigned flag) const          { SF_UNUSED(flag); return false; }
    void                 SetModeAvailability(unsigned modes)           { SF_UNUSED(modes); }
    void                 SetFlagAvailability(unsigned flags)           { SF_UNUSED(flags); }

    void                 SetProfileMode(ProfilerModes mode)            { SF_UNUSED(mode); }
    ProfilerModes        GetProfileMode() const                        { return Profile_None; }
    void                 SetProfileFlags(unsigned flags)               { SF_UNUSED(flags); }
    void                 SetProfileFlag(unsigned flag, bool state)     { SF_UNUSED2(flag, state); }
    unsigned             GetProfileFlag(unsigned flag) const           { SF_UNUSED(flag); return 0; }
    void                 SetHighlightedBatch(unsigned batch)           { SF_UNUSED(batch); }
    virtual DrawModeType GetDrawMode() const                           { return DrawMode_Count; }
    virtual void         SetDrawMode(DrawModeType mode)                { SF_UNUSED(mode); }
    virtual bool         IsFilterCachingEnabled() const                { return true; }
    virtual bool         IsBlendCachingEnabled() const                 { return true; }

    virtual const ProfileModifier&  GetCurrentProfileModifier() const                       { return PMDefault; }
    virtual ProfileModifier&        GetCurrentProfileModifier()                             { return PMDefault; }
    virtual Color                   GetClearColor(Color c) const                            { return c; }
    virtual BlendMode               GetBlendMode(BlendMode mode) const                      { return mode; }
    virtual PrimitiveFillType       GetFillType(PrimitiveFillType fill) const               { return fill; }
    virtual Color                   GetColor(Color color) const                             { return color; }
    virtual bool                    IsCxformChanged() const                                 { return false; }
    virtual Cxform                  GetCxform(const Cxform& cx) const                       { return cx; }
    virtual void                    SetFillFlags(unsigned flags)                            { SF_UNUSED(flags); }
    virtual void                    SetBatch(HAL* phal, Primitive* prim, unsigned index)    { SF_UNUSED3(phal, prim, index); }
    virtual void                    SetBatch(HAL* phal, ComplexMesh* prim, unsigned index)  { SF_UNUSED3(phal, prim, index); }
    virtual bool                    ShouldDrawMaskVisible() const                           { return false; }
    virtual bool                    ShouldDrawCachedPrimArea() const                        { return false; }

    void                            BeginScene(HAL* phal)                                   { SF_UNUSED(phal); }

#endif // SF_RENDERER_PROFILE

protected:

#if defined(SF_RENDERER_PROFILE)
    ProfilerModes           ProfileMode;           // The current profiling mode.
    ProfilerModes           NextProfileMode;       // Shadow state, the mode cannot be updated in the middle of a scene.
    unsigned                ProfileFlags;          // The current profiling flags.
    unsigned                NextProfileFlags;      // Shadow state, the flags cannot be updated in the middle of a scene.
    int                     HighlightBatch;        // Index of the batch that should be highlighted (-1 for no highlighting).
    DrawModeType            CurrentDrawMode;       // Indicates the current content being rendered.

    ProfileModifierOverdraw PMOverdraw;            // The overdraw profile modifier
    ProfileModifierBatch    PMBatch;               // The batch profile modifier
    ProfileModifierBlending PMBlending;            // The blending profile modifier
    ProfileModifierTDensity PMTDensity;            // The texture density profile modifier

    unsigned                AvailableModes;        // The mask of modes that are possible (see ProfilerModes). May be enabled/disabled by SetAvailableModes
    unsigned                AvailableFlags;        // The set of flags that are possible (see ProfilerFlags). May be enabled/disabled by SetAvailableFlags

#endif // SF_RENDERER_PROFILE

    ProfileModifier         PMDefault;             // The default profile modifier, which does normal rendering.
};

// Intended to modify the behavior of the HAL, during runtime, to rendering shapes in different ways.
// These different methods of rendering allow the user to obtain information visually about their scene,
// such as the amount of overdraw, or the amount of batching happening. It also contains certain flags,
// which may be useful in debugging certain situations, such as the ability to disable the caching of filters.
// Each HAL must override this class to perform their specific functions related to the different modes.
template<class HALBase>
class ProfilerHAL : public HALBase
{
public:

    // Constructor/Destructor
    ProfilerHAL(ThreadCommandQueue* commandQueue) :
        HALBase(commandQueue)
    {
    }

    ~ProfilerHAL()
    {
    }

#if defined(SF_RENDERER_PROFILE)
    // Render::HAL overrides
    virtual bool BeginScene();
    virtual void beginDisplay(BeginDisplayData* data);
    virtual void PushMask_BeginSubmit(MaskPrimitive* prim);
    virtual void EndMaskSubmit();
    virtual void DrawProcessedPrimitive(Primitive* pprimitive, PrimitiveBatch* pstart, PrimitiveBatch *pend);
    virtual void DrawProcessedComplexMeshes(ComplexMesh* complexMesh, const StrideArray<HMatrix>& matrices);
    virtual void PushFilters(FilterPrimitive* prim);
    virtual void PopFilters();
    virtual void PushBlendMode(BlendPrimitive* prim);
    virtual void PopBlendMode();
    virtual void PrepareCacheable(CacheablePrimitive*, bool unprepare);
    virtual void clearSolidRectangle(const Rect<int>& r, Color color, bool blend);

protected:
    virtual bool shouldRenderFilters(const FilterPrimitive*) const;
    virtual bool shouldRenderTargetBlend(const BlendPrimitive*) const;
#endif
};

#if defined(SF_RENDERER_PROFILE)
template<class HALBase>
inline bool ProfilerHAL<HALBase>::BeginScene()
{
    // Cannot change profile modes/flags in the middle of a scene. Buffer them until BeginScene is called.
    HALBase::Profiler.BeginScene(this);
    return HALBase::BeginScene();
}

template<class HALBase>
inline void ProfilerHAL<HALBase>::beginDisplay(BeginDisplayData* data)
{
    HALBase::Profiler.SetDrawMode(DrawMode_Fill);
    HALBase::beginDisplay(data);
}

template<class HALBase>
inline void ProfilerHAL<HALBase>::PushMask_BeginSubmit(MaskPrimitive* prim)
{
    HALBase::Profiler.SetDrawMode(DrawMode_Mask);
    HALBase::PushMask_BeginSubmit(prim);
}

template<class HALBase>
inline void ProfilerHAL<HALBase>::EndMaskSubmit()
{
    HALBase::EndMaskSubmit();
    HALBase::Profiler.SetDrawMode(DrawMode_Fill);
}

template<class HALBase>
inline void ProfilerHAL<HALBase>::DrawProcessedPrimitive(Primitive* pprimitive, PrimitiveBatch* pstart, PrimitiveBatch *pend)
{
    // If we are drawing masks as visible, do that.
    if (HALBase::Profiler.ShouldDrawMaskVisible() && (HALBase::HALState & HALBase::HS_DrawingMask))
    {
        HALBase::applyDepthStencilMode(HALBase::DepthStencil_Disabled, HALBase::MaskStackTop);
        HALBase::DrawProcessedPrimitive(pprimitive, pstart, pend);
        HALBase::applyDepthStencilMode(HALBase::DepthStencil_StencilIncrementEqual, HALBase::MaskStackTop);
    }

    // Now draw the real primitive.
    HALBase::DrawProcessedPrimitive(pprimitive, pstart, pend);
}

template<class HALBase>
inline void ProfilerHAL<HALBase>::DrawProcessedComplexMeshes(ComplexMesh* complexMesh, const StrideArray<HMatrix>& matrices)
{
    // If we are drawing masks as visible, do that.
    if (HALBase::Profiler.ShouldDrawMaskVisible() && (HALBase::HALState & HALBase::HS_DrawingMask))
    {
        HALBase::applyDepthStencilMode(HALBase::DepthStencil_Disabled, HALBase::MaskStackTop);
        HALBase::DrawProcessedComplexMeshes(complexMesh, matrices);
        HALBase::applyDepthStencilMode(HALBase::DepthStencil_StencilIncrementEqual, HALBase::MaskStackTop);
    }

    // Now draw the real primitive.
    HALBase::DrawProcessedComplexMeshes(complexMesh, matrices);
}

template<class HALBase>
inline void ProfilerHAL<HALBase>::PushFilters(FilterPrimitive* prim)
{
    if (HALBase::Profiler.ShouldDrawCachedPrimArea())
    {
        HALBase::profilerDrawCacheablePrimArea(prim);
        typename HALBase::FilterStackEntry e = {prim, 0};
        ProfilerHAL<HALBase>::FilterStack.PushBack(e);
    }
    else
    {
        HALBase::PushFilters(prim);
    }
}

template<class HALBase>
inline void ProfilerHAL<HALBase>::PopFilters()
{
    if ( HALBase::Profiler.ShouldDrawCachedPrimArea() )
    {
        ProfilerHAL<HALBase>::FilterStack.PopBack();
        if ( ProfilerHAL<HALBase>::FilterStack.GetSize() == 0 )
            HALBase::Profiler.SetDrawMode(DrawMode_Fill);
    }
    else
    {
        HALBase::PopFilters();
    }
}

template<class HALBase>
inline void ProfilerHAL<HALBase>::PushBlendMode(BlendPrimitive* prim)
{
    if (HALBase::Profiler.ShouldDrawCachedPrimArea() &&
        prim && (BlendState::IsTargetAllocationNeededForBlendMode(prim->GetBlendMode()) ||
        prim->GetBlendMode() == Blend_Alpha || prim->GetBlendMode() == Blend_Erase))
    {
        typename HALBase::BlendStackEntry e = {prim, 0};
        ProfilerHAL<HALBase>::BlendModeStack.PushBack(e);
        HALBase::profilerDrawCacheablePrimArea(prim);
    }
    else
    {
        HALBase::PushBlendMode(prim);
    }
}

template<class HALBase>
inline void ProfilerHAL<HALBase>::PopBlendMode()
{
    BlendPrimitive* prim = ProfilerHAL<HALBase>::BlendModeStack.Back().pPrimitive;
    if ( HALBase::Profiler.ShouldDrawCachedPrimArea() &&
        (BlendState::IsTargetAllocationNeededForBlendMode(prim->GetBlendMode()) ||
        prim->GetBlendMode() == Blend_Alpha || prim->GetBlendMode() == Blend_Erase))
    {
        ProfilerHAL<HALBase>::BlendModeStack.PopBack();
        if ( ProfilerHAL<HALBase>::BlendModeStack.GetSize() == 0 )
            HALBase::Profiler.SetDrawMode(DrawMode_Fill);
    }
    else
    {
        HALBase::PopBlendMode();
    }
}

template<class HALBase>
inline void ProfilerHAL<HALBase>::PrepareCacheable(CacheablePrimitive* prim, bool unprepare)
{
    // If we are not rendering the cacheable as a simple area, don't prepare it.
    if (!HALBase::Profiler.ShouldDrawCachedPrimArea())
        HALBase::PrepareCacheable(prim, unprepare);
}

template<class HALBase>
inline void ProfilerHAL<HALBase>::clearSolidRectangle(const Rect<int>& r, Color color, bool blend)
{
    DrawModeType previousDrawMode = HALBase::Profiler.GetDrawMode();
    HALBase::Profiler.SetDrawMode(DrawMode_Clear);
    color = HALBase::Profiler.GetClearColor(color);
    HALBase::clearSolidRectangle(r, color, blend);
    HALBase::Profiler.SetDrawMode(previousDrawMode);
}

template<class HALBase>
inline bool ProfilerHAL<HALBase>::shouldRenderFilters(const FilterPrimitive* prim) const
{
    // If we're rendering a profile mode, do not filter content. If we do this,
    // and filter caching is enabled (which it is by default), the profile-mode version
    // of the filter will be cached, and show up incorrectly once the profile-mode is turned off.
    if (HALBase::Profiler.GetProfileMode() != Profile_None)
        return false;

    return HALBase::shouldRenderFilters(prim);
}

template<class HALBase>
inline bool ProfilerHAL<HALBase>::shouldRenderTargetBlend(const BlendPrimitive* prim) const
{
    // If we're rendering a profile mode, do not render cached blends. If we do this,
    // and blend primitive caching is enabled (which it is by default), the profile-mode version
    // of the blend will be cached, and show up incorrectly once the profile-mode is turned off.
    if (HALBase::Profiler.GetProfileMode() != Profile_None)
        return false;

    return HALBase::shouldRenderTargetBlend(prim);
}

#endif // SF_RENDERER_PROFILE

}}; // Scaleform

#endif // INC_SF_Render_Profiler_H
