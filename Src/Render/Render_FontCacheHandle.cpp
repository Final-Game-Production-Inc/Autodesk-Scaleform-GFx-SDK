/**************************************************************************

Filename    :   Render_FontCacheHandle.cpp
Content     :   Abstract base class for system and built-in fonts.
Created     :   6/18/2007
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_FontCacheHandle.h"
#include "Render_GlyphCache.h"

#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {


void FontCacheHandleRef::releaseFont()
{
    FontCacheHandleManager* manager = pManager.Exchange_NoSync(0);
    if (manager)
    {
        manager->fontLost(this);
        manager->Release();
    }
}

void FontCacheHandleRef::releaseFont_NoNotify()
{
    FontCacheHandleManager* manager = pManager.Exchange_NoSync(0);
    if (manager)
        manager->Release();
}




void FontCacheHandleManager::ProcessKillList()
{
    Lock::Locker scopeLock(getLock());
    destroyFontList_NTS(Font_KillList);
}



void FontCacheHandleManager::DestroyAllFonts()
{
    Lock::Locker scopeLock(getLock());
    destroyFontList_NTS(Font_KillList);
    destroyFontList_NTS(Font_LiveList);
}



void FontCacheHandleManager::fontLost(FontCacheHandleRef* href)
{
    Lock::Locker scopeLock(getLock());    

    // Since handle.pKeySet is always modified in lock, we can
    // check it directly to see if MeshKeySet was destroyed on us.
    // This is possible if RenderThread swapped out all related meshes
    // or called DestroyAllKeys().
    // Key set was already destroyed, nothing to do.
    FontCacheHandle* handle = href->pFontHandle;
    if (!handle)
        return;

    // Move to kill/evict list. 
    handle->RemoveNode();
    Fonts[Font_KillList].PushBack(handle);
    handle->pFont = 0;
}


void FontCacheHandleManager::destroyFontList_NTS(FontListType type)
{
    List<FontCacheHandle>& list = Fonts[type];
    if (list.IsEmpty())
        return;

    FontCacheHandle* font = list.GetFirst();

    bool merge = false;
    do {
        FontCacheHandle* temp = font->pNext;

        if (type != Font_KillList)
        {   // Update mesh providers if destroying main lost.
            SF_ASSERT(font->pFont->hRef.pFontHandle == font);
            font->pFont->hRef.pFontHandle = 0;
            font->pFont->hRef.releaseFont_NoNotify();
            // releaseFont_NoNotify will clear hRef.pManager.
            font->pFont = 0;
        }
        else
        {   // For kill list, the Font object should be clear.
            SF_ASSERT(font->pFont == 0);
            if (pCache)
            {
                pCache->CleanUpFont(font);
                merge = true;
            }
        }
        delete font;
        font = temp;
    } while(!list.IsNull(font));
    list.Clear();
    if (merge)
        pCache->MergeCacheSlots();
}



FontCacheHandle* FontCacheHandleManager::RegisterFont(Font* font)
{
    FontCacheHandleRef& handle = font->hRef;
    SF_ASSERT(!handle.pManager || (handle.pManager == this));

    // When we are called here, we are assumed to own MeshKeySetHandle container
    // reference, such that it can't possibly be destroyed in the other thread,
    // This means that our access is safe.
    if (!handle.pManager)
    {
        AddRef();
        handle.pManager = this;
    }
    if (!handle.pFontHandle)
    {
        Lock::Locker scope(getLock());

        handle.pFontHandle = SF_HEAP_NEW(pRenderHeap) FontCacheHandle(this, font);
        if (!handle.pFontHandle)
            return 0;
        Fonts[Font_LiveList].PushBack(handle.pFontHandle);
    }
    return handle.pFontHandle;
}








}} // Scaleform::Render

