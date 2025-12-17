/**************************************************************************

PublicHeader:   Render
Filename    :   Renderer2D.h
Content     :   2D renderer interface that renderers TreeRoot
Created     :   September 23, 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_Renderer2D_H
#define INC_SF_Render_Renderer2D_H

#include "Render_TreeNode.h"
#include "Render_MeshCacheConfig.h"
#include "Render_GlyphCacheConfig.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {

class Renderer2DImpl;
class GlyphCache;
struct ToleranceParams;

// Parameter to Display, specifying which pass to render.
enum DisplayPass
{
    Display_Prepass =   0x0001, // Prepass rendering renders all render targets in the scene. This is required on some platforms (X360/Wii).
    Display_Final   =   0x0002, // Final pass rendering, using cached results of the prepass (if required).
    Display_All     =   0x0003  // Render both passes concurrently.
};


// ***** Renderer2D

// Renderer2D implements rendering of a tree staring with TreeRoot through
// its Display operation.
//
// Internally, the renderer caches the rendering tree structure and its resulting
// batches so that it can minimize the number of rendering calls, and support
// concurrent tree update through the RenderContext. 

class Renderer2D : public RefCountBase<Renderer2D, StatRender_Mem>
{
    Renderer2DImpl* pImpl;
    void* getThis() { return this; }

public:
    Renderer2D(HAL* phal);
    ~Renderer2D();

    Renderer2DImpl*         GetImpl() const { return pImpl; }
    Context::RenderNotify*  GetContextNotify() const;

    HAL*         GetHAL() const;
    // Query MeshCache configuration interface.
    MeshCacheConfig*   GetMeshCacheConfig() const;
    // Query GlyphCache configuration interface.   
    GlyphCacheConfig*  GetGlyphCacheConfig() const;

    const ToleranceParams&  GetToleranceParams() const;
    void                    SetToleranceParams(const ToleranceParams& params);
        
    // Delegated interface.
    bool    BeginFrame();
    void    EndFrame();
    void    FinishFrame();
    bool    BeginScene();
    void    EndScene();
    void    BeginDisplay(Color backgroundColor, const Viewport& viewport);
    void    EndDisplay();
    
    // Renders a display tree starting with TreeRoot. TreeRoot must've
    // been allocated from the specified context.
    void    Display(TreeRoot *pnode);

    void    Display(const DisplayHandle<TreeRoot>& hroot)
    { Display(hroot.GetRenderEntry()); }
};

}} // Scaleform::Render

#endif
