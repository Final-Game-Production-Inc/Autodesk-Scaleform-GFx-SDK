/**************************************************************************

Filename    :   Render_FontCacheHandle.h
Content     :   Abstract base class for system and built-in fonts.
Created     :   6/18/2007
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_FontCacheHandle_H
#define INC_SF_Render_FontCacheHandle_H

#include "Render_Font.h"

namespace Scaleform { namespace Render {

//------------------------------------------------------------------------
// ***** FontCacheHandleManager

// FontCacheHandleManager creates and manages lifetime of Font objects,
// a single object of this class is created by Renderer2D.
//
// FontCacheHandleManager object is always created on global heap, so as
// to allow its internal Lock to survive until it is released in
// associated GlyphCache (synchronization exchange protocol
// in FontHandle allows FontCacheHandleManager to be released on any thread).
class GlyphCache;
class FontCacheHandleManager : public RefCountBase<FontCacheHandleManager, StatRender_Font_Mem>
{
    friend class Font;
    friend class FontCacheHandle;
    friend class FontCacheHandleRef;
public:

    FontCacheHandleManager(MemoryHeap* renderHeap, GlyphCache* q=0) : pRenderHeap(renderHeap), pCache(q) { }
    ~FontCacheHandleManager()
    {
        DestroyAllFonts();
    }

    // Cleans up font cache handles for destroyed Font objects.
    void ProcessKillList();

    // Destroys all of the fonts; useful when shutting down renderer.
    // Should be called only after all references were released,
    // otherwise it will assert internally.    
    void DestroyAllFonts();

    FontCacheHandle* RegisterFont(Font* font);

private:
    enum FontListType
    {
        Font_LiveList,
        Font_KillList,
        Font_List_Count
    };

    Lock* getLock() { return &FontLock; }

    // Called from ~FontHandle to notify render thread
    // that Provider is being destroyed.
    void  fontLost(FontCacheHandleRef* href);
    void  destroyFontList_NTS(FontListType type);

private:
    Lock                    FontLock;
    List<FontCacheHandle>   Fonts[Font_List_Count];
    MemoryHeap*             pRenderHeap;
    GlyphCache*             pCache;
};



//------------------------------------------------------------------------
class FontCacheHandle : public ListNode<FontCacheHandle>, public NewOverrideBase<StatRender_Font_Mem>
{
    friend class Font;
public:
    FontCacheHandle(FontCacheHandleManager* man, Font* fnt) : pManager(man), pFont(fnt) {}

    FontCacheHandleManager*  pManager;
    Font*                    pFont;
};





}}

#endif

