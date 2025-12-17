/**************************************************************************

Filename    :   GFx_Sprite.cpp
Content     :   Implementation of MovieClip character
Created     :   
Authors     :   Michael Antonov, Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_Alg.h"
#include "Kernel/SF_ListAlloc.h"
#include "Kernel/SF_List.h"
#include "GFx/GFx_Sprite.h"
#include "Render/Render_TreeLog.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_FontManager.h"
#include "GFx/GFx_StaticText.h" // for textsnapshot
#include "GFx/GFx_DrawingContext.h" // for textsnapshot
#include "GFx/IME/GFx_IMEManager.h"

#include <string.h> // for memset
#include <float.h>
#include <stdlib.h>
#ifdef SF_MATH_H
#include SF_MATH_H
#else
#include <math.h>
#endif

namespace Scaleform { namespace GFx {

SF_INLINE MemoryHeap* Sprite::GetMovieHeap() const
{
    return pASRoot->GetMovieImpl()->GetMovieHeap();
}

//@TODO: Must be removed!
// Create A (mutable) instance of our definition.  The instance is created to
// live (temporarily) on some level on the parent InteractiveObject's display list.
// DisplayObjectBase*   SpriteDef::CreateCharacterInstance(InteractiveObject* parent, ResourceId id,
//                                                       MovieDefImpl *pbindingImpl)
// {
//     // Most of the time pbindingImpl will be ResourceMovieDef come from parents scope.
//     // In some cases we call CreateCharacterInstance with a different binding context
//     // then the parent sprite; this can happen for import-bound characters, for example.
//     MovieImpl* proot = parent->GetMovieImpl();    
//     Sprite*    psi = 
//         SF_HEAP_NEW(proot->GetMovieHeap()) Sprite(this, pbindingImpl, proot, parent, id);
//     return psi;
// }

//
// ***** Sprite Implementation
//


Sprite::Sprite(TimelineDef* pdef, MovieDefImpl* pdefImpl,
                     ASMovieRootBase* pr, InteractiveObject* pparent,
                     ResourceId id, bool loadedSeparately)
                     :
DisplayObjContainer(pdefImpl, pr, pparent, id),
pDef(pdef),    
PlayStatePriv(State_Playing),
CurrentFrame(0),    
#ifdef GFX_ENABLE_SOUND
pActiveSounds(NULL),
#endif
pHitAreaHandle(0),
pHitAreaHolder(0),
//pUserData(0),
Flags(0),
MouseStatePriv(UP)
{
    SF_ASSERT(pDef && pDefImpl);
    pASRoot = pr;

#ifdef GFX_ENABLE_SOUND
    pActiveSounds = NULL;
#endif
    if (pdef->GetResourceType() == Resource::RT_SpriteDef)
    {
        SpriteDef* sp = static_cast<SpriteDef*>(pdef);
        SetScale9Grid(sp->GetScale9Grid());

        Flags |= Flags_SpriteDef;
    }

    SetSpriteFlag();
    SetUpdateFrame(true);
    SetHasLoopedPriv(false);

    // By default LockRoot is false.
    SetLockRoot(false);
    SetLoadedSeparatelyFlag(loadedSeparately);

    // Since loadedSeparately flag does not get set for imports (because it only
    // enables _lockroot behavior not related to imports), we need to check for
    // pMovieDefImpl specifically. Imports receive root node so that they can
    // have their own font manager.
    bool importFlag = pparent && !loadedSeparately && 
        (pparent->GetResourceMovieDef() != pdefImpl);

    //!AB: we can't check for "!parent" here since sprite may be 
    // created via ActionScript with null parent and then attached to 
    // the stage.
    if (loadedSeparately || /*!pparent ||*/ importFlag)
    {
        AssignRootNode(importFlag);
    }
}


Sprite::~Sprite()
{
#ifdef GFX_ENABLE_SOUND
    if (pActiveSounds)
        delete pActiveSounds;
#endif
    //Root->DropRef();

    ClearDisplayList(); // need to clean display list here to make sure the correct GetRenderContainer is used.

//     if (pUserData)
//         delete pUserData;
}

void Sprite::SetLoadedSeparately(bool v)
{
    SetLoadedSeparatelyFlag(v);
    if (v)
        AssignRootNode();
}

void Sprite::SetDirtyFlag() 
{ 
    //GetMovieImpl()->SetDirtyFlag(); 
}

void Sprite::SetRootNodeLoadingStat(unsigned bytesLoaded, unsigned loadingFrame)
{
    if (pRootNode)
    {
        pRootNode->BytesLoaded = bytesLoaded;
        pRootNode->LoadingFrame = pRootNode->ImportFlag ? 0 : loadingFrame;
    }
}

#ifndef SF_NO_IME_SUPPORT

void Sprite::SetIMECandidateListFont(FontHandle* pfontHandle)
{
	if (pfontHandle)
		pRootNode->pFontManager->SetIMECandidateFont(pfontHandle);
}

void Sprite::SetIMECandidateListFont(FontResource* pfont)
{
	if(!GetAvmSprite()->CheckCandidateListLevel(GFX_CANDIDATELIST_LEVEL))
        return;

    SF_ASSERT(pRootNode);
    if (!pRootNode)
        return;
    if (!pRootNode->pFontManager)
        return;
    ResourceBinding* pbinding = pfont->GetBinding();
    Ptr<FontHandle> pfontHandle;
    if (pfont->GetFont()->IsResolved())
    {
        MovieDefImpl* pdefImpl = pbinding ? pbinding->GetOwnerDefImpl() : NULL;
        pfontHandle = *SF_HEAP_NEW(GetMovieHeap()) FontHandle(NULL, pfont->GetFont(), GFX_CANDIDATELIST_FONTNAME, 0, pdefImpl);
    }
    else
    {
        Ptr<FontHandle> ptmp = *pRootNode->pFontManager->CreateFontHandle(pfont->GetName(), pfont->GetFont()->GetFontFlags(), false);
        if (ptmp)
            pfontHandle = *SF_HEAP_NEW(GetMovieHeap()) FontHandle(NULL, ptmp->GetFont(), GFX_CANDIDATELIST_FONTNAME, 0, ptmp->pSourceMovieDef);
    }
    if (pfontHandle)
        pRootNode->pFontManager->SetIMECandidateFont(pfontHandle);
}
#endif //#ifdef SF_NO_IME_SUPPORT

void Sprite::ForceShutdown ()
{
    DisplayObjContainer::ForceShutdown();

    if (HasAvmObject())
        GetAvmSprite()->ForceShutdown();

    RemoveFromPlayList();
}

InteractiveObject* Sprite::GetTopParent(bool ignoreLockRoot) const
{
    // Return _root, considering _lockroot. 
    if (!GetParent() ||
        (!ignoreLockRoot && IsLoadedSeparately() && IsLockRoot()) )
    {
        // If no parent, we ARE the root.
        return const_cast<Sprite*>(this);
    }

    //// HACK!
    if (IsUnloaded())
    {
        //!AB: This code seems still useful, since Invoke might use this method
        // and to call it on unloaded characters. This is happening in w****d1.swf
        // when char dies, for example. TODO: investigate.
        //SF_DEBUG_WARNING(1, "GetASRootMovie called on unloaded sprite");
        return GetMovieImpl()->pMainMovie;
    }

    return GetParent()->GetTopParent(ignoreLockRoot);
}

UInt32  Sprite::GetBytesLoaded() const
{  
    MovieDefRootNode* root = FindRootNode();
    if (root)
        return root->BytesLoaded;
    // We should never end up here.
    return 0;
}

// Returns frames loaded, sensitive to background loading.
unsigned    Sprite::GetLoadingFrame() const
{
    return (pRootNode && !pRootNode->ImportFlag) ?
        pRootNode->LoadingFrame : GetFrameCount();
}

void    Sprite::OnIntervalTimer(void *timer)
{
    SF_UNUSED(timer);
}   


// "transform" matrix describes the transform applied to parent and us,
// including the object's matrix itself. This means that if transform is
// identity, GetBoundsTransformed will return local bounds and NOT parent bounds.
RectF  Sprite::GetBounds(const Matrix &transform) const
{
    RectF r = DisplayObjContainer::GetBounds(transform);

    if (pDrawingAPI)
    {
        RectF tempRect;
        pDrawingAPI->ComputeBound(&tempRect);
        if (!tempRect.IsEmpty())
        {
            tempRect = transform.EncloseTransform(tempRect);
            if (!r.IsEmpty())
                r.Union(tempRect);
            else
                r = tempRect;
        }
    }

    if (HasScrollRect())
    {
        const RectD& scrRectD = *GetScrollRect();
        RectF scrRect(float(scrRectD.x1), float(scrRectD.y1), float(scrRectD.x2), float(scrRectD.y2));
        scrRect = transform.EncloseTransform(scrRect);
        r.Intersect(scrRect);
    }
    return r;
}


// "transform" matrix describes the transform applied to parent and us,
// including the object's matrix itself. This means that if transform is
// identity, GetBoundsTransformed will return local bounds and NOT parent bounds.
RectF  Sprite::GetRectBounds(const Matrix &transform) const
{
    return DisplayObjContainer::GetRectBounds(transform);
}

RectF Sprite::GetFocusRect() const
{
    // Here is the difference from Flash: GFx will use 'hitArea's rectangle
    // as a focusRect, if such hitArea is installed for the sprite.
    // The hitArea's rectangle will be used ONLY if the hitArea is installed
    // to 'this' sprite; in the case when hitArea is installed for one of its
    // child it will not be used. It is possible to find such hitAreas, however,
    // it is unclear what to do if more than one child has a hitArea. That is why,
    // at the moment only immediate hitArea is used.
    // The standard Flash Player doesn't use hitArea for focusRect at all.
    Sprite* phitArea = GetHitArea();
    Matrix     thisToHitAreaTransform;

    if (phitArea)
    {
        thisToHitAreaTransform = phitArea->GetMatrix();

        // now we need to translate hitArea's focus rect to 'this' sprite's
        // coordinate system.
        RectF haRect = phitArea->GetFocusRect();
        return thisToHitAreaTransform.EncloseTransform(haRect);
    }
    return DisplayObjContainer::GetFocusRect();
}

#ifdef GFX_AS_ENABLE_TEXTSNAPSHOT
void    Sprite::GetTextSnapshot(StaticTextSnapshotData* pdata) const
{
    DisplayObjectBase*   ch;
    UPInt   i, n = mDisplayList.GetCount();

    for (i = 0; i < n; i++)
    {
        ch = mDisplayList.GetDisplayObject(i);
        if (ch != NULL)
        {
            // Check if character is a static textfield
            CharacterDef* chDef = ch->GetCharacterDef();
            if (chDef && chDef->GetResourceType() == Resource::RT_TextDef)
            {
                StaticTextCharacter* pstextChar = static_cast<StaticTextCharacter*>(ch);
                pdata->Add(pstextChar);
            }
        }
    }
}
#endif  // SF_NO_FXPLAYER_AS_TEXTSNAPSHOT


void    Sprite::Restart()
{
    mDisplayList.MarkAllEntriesForRemoval(this);
    CurrentFrame = 0;   
    RollOverCnt = 0;
    SetUpdateFrame();
    SetHasLoopedPriv(false);
    PlayStatePriv = State_Playing;

    // Execute this frame's init actions, if necessary.
    if (HasAvmObject())
        GetAvmSprite()->ExecuteInitActionFrameTags(CurrentFrame);

    ExecuteFrameTags(CurrentFrame);
    mDisplayList.UnloadMarkedObjects(this);
    SetDirtyFlag();
}

// Return the topmost entity that the given point covers.  NULL if none.
// Coords are in parent's frame.
DisplayObject::TopMostResult Sprite::GetTopMostMouseEntity(
    const Render::PointF &pt, TopMostDescr* pdescr) 
{
    TopMostResult tr = DisplayObjContainer::GetTopMostMouseEntity(pt, pdescr);
    if (tr == TopMost_Continue)
    {
        if (pDrawingAPI)
        {
            Sprite* phitAreaHolder = GetHitAreaHolder();
            // a hack for AS3, allow testing against drawn shapes regardless to ActsAsButton.
            // see test_topmostentity_w_drawingapi.swf test.
            if ((GetASMovieRoot()->GetAVMVersion() == 2 && pdescr->TestAll) || 
                ActsAsButton() || (phitAreaHolder && (pdescr->TestAll || phitAreaHolder->ActsAsButton())))
            {
                if (pDrawingAPI->DefPointTestLocal(pdescr->LocalPt, true, this))
                {
                    if (phitAreaHolder && (pdescr->TestAll || phitAreaHolder->ActsAsButton())) 
                    {
                        pdescr->pResult = phitAreaHolder;
                        pdescr->pHitArea = this;
                        return TopMost_Found;
                    }
                    else
                    {
                        // Sprites with hit area also shouldn't receive mouse events if this hit area is not the sprite's child
                        // We need to check here if a hit area is our child  
                        Sprite* phitArea = GetHitArea();
                        if (GetASMovieRoot()->GetAVMVersion() == 1)
                        {
                            // AS1.0 & 2.0 only.
                            if (phitArea)
                            {
                                InteractiveObject* parent = phitArea;
                                do {
                                    parent = parent->GetParent();
                                } while (parent && parent != this);
                                if (!parent)
                                {
                                    // hit area is not our child so we should not receive mouse events
                                    pdescr->pResult = NULL;
                                    return TopMost_FoundNothing;
                                }
                                if (phitArea == pdescr->pHitArea)
                                {
                                    pdescr->pResult = this;
                                    return TopMost_Found;
                                }
                                else
                                {
                                    pdescr->pResult = NULL;
                                    return TopMost_FoundNothing;
                                }
                            }
                            else
                            {
                                pdescr->pResult = this;
                                return TopMost_Found;
                            }
                        }
                        else // AS3
                        {
                            // according to AS3 tests 'test_nested_rollover_events' & 'hitAreaTest.swf'
                            // this code is not necessary should not be executed for AS3;
                            // however, AS2 still needs it ('test_hitarea_event_handlers.swf').
                            if (phitArea)
                            {
                                // hitArea must be ignored if hit occurred on a child InteractiveObject.
                                if (!pdescr->pResult || pdescr->pResult == this || !pdescr->pResult->IsInteractiveObject())
                                {
                                    if (phitArea == pdescr->pHitArea)
                                    {
                                        pdescr->pResult = this;
                                        return TopMost_Found;
                                    }
                                    else
                                    {
                                        pdescr->pResult = NULL;
                                        return TopMost_FoundNothing;
                                    }
                                }
                            }
                            else
                            {
                                pdescr->pResult = this;
                                return TopMost_Found;
                            }
                        }
                    }
                }
            }
        }
    }
    return tr;
}

#ifdef GFX_ENABLE_SOUND
Sprite::ActiveSounds::ActiveSounds()
{
    Volume = 100;
    SubVolume = 0;
    Pan = 0;
}
Sprite::ActiveSounds::~ActiveSounds()
{
    size_t i;
    if (pStreamSound)
    {
        pStreamSound->Stop();
        pStreamSound = NULL;
    }
    for(i = 0; i < ASSounds.GetSize(); ++i)
    {
        ASSounds[i]->ReleaseTarget();
    }
}
Sprite::ActiveSoundItem::ActiveSoundItem()
{
    pSoundObject = NULL;
    pResource = NULL;
}

Sprite::ActiveSoundItem::~ActiveSoundItem()
{
    if (pChannel)
        pChannel->Stop();
    if (pResource)
    {
        pResource->DecPlayingCount();
        if (!pResource->IsPlaying())
        {
            pResource->GetSoundInfo()->ReleaseResource();
        }
        pResource->Release();
    }
}

bool    Sprite::IsSoundPlaying(ASSoundIntf* psobj)
{
    if (pActiveSounds)
    {
        for(size_t i = 0; i < pActiveSounds->Sounds.GetSize(); ++i)
        {
            if (pActiveSounds->Sounds[i]->pSoundObject == psobj)
                return pActiveSounds->Sounds[i]->pChannel && pActiveSounds->Sounds[i]->pChannel->IsPlaying();
        }
    }
    return false;
}
// Add an active sound
void    Sprite::AddActiveSound(Sound::SoundChannel* pchan, ASSoundIntf* psobj, SoundResource* pres)
{
    SF_ASSERT(pchan);
    if (!pActiveSounds)
        pActiveSounds = SF_NEW ActiveSounds;
    Ptr<ActiveSoundItem> psi;
    for(size_t i = 0; i < pActiveSounds->Sounds.GetSize(); ++i)
    {
        if (pActiveSounds->Sounds[i]->pChannel.GetPtr() == pchan)
        {
            psi = pActiveSounds->Sounds[i];
            break;
        }
    }
    if (!psi)
    {
        psi = *SF_NEW ActiveSoundItem;
        psi->pChannel = pchan;
        pActiveSounds->Sounds.PushBack(psi);
        ModifyOptimizedPlayListLocal<Sprite>();
    }
    psi->pSoundObject = psobj;
    psi->pResource = pres;
    if (psi->pResource)
    {
        psi->pResource->IncPlayingCount();
        psi->pResource->AddRef();
    }
}

void    Sprite::SetStreamingSound(Sound::SoundChannel* pchan)
{
    if (!pchan && !pActiveSounds)
        return;
    if (!pActiveSounds)
        pActiveSounds = SF_NEW ActiveSounds;
    if (pActiveSounds->pStreamSound)
        pActiveSounds->pStreamSound->Stop();
    pActiveSounds->pStreamSound = pchan;
    if (pActiveSounds->pStreamSound)
    {
        pActiveSounds->pStreamSound->SetVolume(GetRealSoundVolume());
        AddActiveSound(pchan, NULL, NULL);
    }
}

Sound::SoundChannel* Sprite::GetStreamingSound() const
{
    if (!pActiveSounds)
        return NULL;
    return pActiveSounds->pStreamSound;
}

// Detach an AS sound object from this sprite
void    Sprite::DetachSoundObject(ASSoundIntf* psobj)
{
    if (pActiveSounds && psobj)
    {
        size_t i;
        for(i = 0; i < pActiveSounds->Sounds.GetSize(); ++i)
        {
            Ptr<ActiveSoundItem> psi = pActiveSounds->Sounds[i];
            if (psi->pSoundObject == psobj)
                psi->pSoundObject = NULL;
        }
        for(i = 0; i < pActiveSounds->ASSounds.GetSize(); ++i)
        {
            if (pActiveSounds->ASSounds[i] == psobj)
            {
                pActiveSounds->ASSounds.RemoveAt(i);
                break;
            }
        }
    }
}

float   Sprite::GetActiveSoundPosition(ASSoundIntf* psobj)
{
    if (pActiveSounds && psobj)
    {
        size_t i;
        for(i = 0; i < pActiveSounds->Sounds.GetSize(); ++i)
        {
            Ptr<ActiveSoundItem> psi = pActiveSounds->Sounds[i];
            if (psi->pSoundObject == psobj && psi->pChannel)
                return psi->pChannel->GetPosition();
        }
    }
    return 0.0f;
}
// Register an AS sound object which is attached to this sprite
void    Sprite::AttachSoundObject(ASSoundIntf* psobj)
{
    SF_ASSERT(psobj);
    if (!pActiveSounds)
        pActiveSounds = SF_NEW ActiveSounds;
    pActiveSounds->ASSounds.PushBack(psobj);
}

// Return a sound volume which is saved inside this object.
// It is used just for displaying this value
int     Sprite::GetSoundVolume()
{
    if (pActiveSounds)
        return pActiveSounds->Volume;
    return 100;
}
// Return a sound volume for SubAudio track which is saved inside this object.
int     Sprite::GetSubSoundVolume()
{
    if (pActiveSounds)
        return pActiveSounds->SubVolume;
    return 100;
}

// Return a calculated sound volume.
float    Sprite::GetRealSoundVolume()
{
    float v = GetSoundVolume()/ 100.0f;
    InteractiveObject* parent = GetParent();
    while (parent)
    {
        if (parent->IsSprite())
            v *= parent->CharToSprite()->GetSoundVolume()/100.0f;
        parent = parent->GetParent();
    }
    return v;
}

// Return a calculated sound volume for SubAudio track.
float    Sprite::GetRealSubSoundVolume()
{
    float v = GetSubSoundVolume()/ 100.0f;
    InteractiveObject* parent = GetParent();
    while (parent)
    {
        if (parent->IsSprite())
            v *= parent->CharToSprite()->GetSubSoundVolume()/100.0f;
        parent = parent->GetParent();
    }
    return v;
}

// Save the new sound volume and propagate it to the child MovieClip
void    Sprite::SetSoundVolume(int volume, int subvol)
{
    if (!pActiveSounds)
        pActiveSounds = SF_NEW ActiveSounds;
    pActiveSounds->Volume    = volume;
    pActiveSounds->SubVolume = subvol;
    UpdateActiveSoundVolume();
}

// Update the volume of all sound started by this sprite
void    Sprite::UpdateActiveSoundVolume()
{
    if (!pActiveSounds) return;
    float v = GetRealSoundVolume();
    for (size_t i = 0; i < pActiveSounds->Sounds.GetSize(); ++i)
    {
        Ptr<ActiveSoundItem> psi = pActiveSounds->Sounds[i];
        if (psi->pSoundObject && psi->pSoundObject->HasPrivateOverride())
            v = psi->pSoundObject->PrivateVolume()/100.0f;
        psi->pChannel->SetVolume(v);
    }
    for (unsigned i = 0; i < mDisplayList.GetCount(); i++)
    {
        const DisplayList::DisplayEntry& dobj = mDisplayList.GetDisplayEntry(i);
        DisplayObjectBase* ch = dobj.GetDisplayObject();
        if (ch->IsInteractiveObject())
        {
            InteractiveObject* pasc = ch->CharToInteractiveObject_Unsafe();
            if (pasc->IsSprite())
                (pasc->CharToSprite())->UpdateActiveSoundVolume();
        }
    }
}

// Return a sound pan which is saved inside this object.
// It is used just for displaying this value
int     Sprite::GetSoundPan()
{
    if (pActiveSounds)
        return pActiveSounds->Pan;
    return 0;
}

// Return a calculated sound pan.
float    Sprite::GetRealSoundPan()
{
    float v = GetSoundPan()/ 100.0f;
    InteractiveObject* parent = GetParent();
    while (parent)
    {
        if (parent->IsSprite())
        {
            v *= parent->CharToSprite()->GetSoundPan()/100.0f;
        }
        parent = parent->GetParent();
    }
    return v;
}

// Save the new sound volume and propagate it to the child MovieClip
void    Sprite::SetSoundPan(int pan)
{
    if (!pActiveSounds)
        pActiveSounds = SF_NEW ActiveSounds;
    pActiveSounds->Pan = pan;
    UpdateActiveSoundPan();
}

// Update the volume of all sound started by this sprite
void    Sprite::UpdateActiveSoundPan()
{
    if (!pActiveSounds) return;
    float v = GetRealSoundPan();
    for (size_t i = 0; i < pActiveSounds->Sounds.GetSize(); ++i)
    {
        Ptr<ActiveSoundItem> psi = pActiveSounds->Sounds[i];
        if (psi->pSoundObject && psi->pSoundObject->HasPrivateOverride())
            v = psi->pSoundObject->PrivatePan()/100.0f;
        psi->pChannel->SetPan(v);
    }
    for (unsigned i = 0; i < mDisplayList.GetCount(); i++)
    {
        const DisplayList::DisplayEntry& dobj = mDisplayList.GetDisplayEntry(i);
        DisplayObjectBase* ch = dobj.GetDisplayObject();
        if (ch->IsInteractiveObject())
        {
            InteractiveObject* pasc = ch->CharToInteractiveObject_Unsafe();
            if (pasc->IsSprite())
                pasc->CharToSprite()->UpdateActiveSoundPan();
        }
    }
}

// Stop all active sound in this MovieClip and all child clips
void    Sprite::StopActiveSounds()
{
    if (pActiveSounds)
    {
        for (size_t i = 0; i < pActiveSounds->Sounds.GetSize(); ++i)
        {
            Ptr<ActiveSoundItem> psi = pActiveSounds->Sounds[i];
            psi->pChannel->Stop();
        }
        pActiveSounds->Sounds.Clear();
    }
    for (unsigned i = 0; i < mDisplayList.GetCount(); i++)
    {
        const DisplayList::DisplayEntry& dobj = mDisplayList.GetDisplayEntry(i);
        DisplayObjectBase* ch = dobj.GetDisplayObject();
        if (ch->IsInteractiveObject())
        {
            InteractiveObject* pasc = ch->CharToInteractiveObject_Unsafe();
            if (pasc->IsSprite())
                (pasc->CharToSprite())->StopActiveSounds();
        }
    }
}

// Stop active sound with given resource in this sprite
void    Sprite::StopActiveSounds(SoundResource* pres)
{
    if (pActiveSounds)
    {
        for (size_t i = 0; i < pActiveSounds->Sounds.GetSize(); )
        {
            Ptr<ActiveSoundItem> psi = pActiveSounds->Sounds[i];
            if (psi->pResource == pres)
            {
                psi->pChannel->Stop();
                pActiveSounds->Sounds.RemoveAt(i);
            }
            else
            {
                ++i;
            }
        }
    }
    for (unsigned i = 0; i < mDisplayList.GetCount(); i++)
    {
        const DisplayList::DisplayEntry& dobj = mDisplayList.GetDisplayEntry(i);
        DisplayObjectBase* ch = dobj.GetDisplayObject();
        if (ch->IsInteractiveObject())
        {
            InteractiveObject* pasc = ch->CharToInteractiveObject_Unsafe();
            if (pasc->IsSprite())
                (pasc->CharToSprite())->StopActiveSounds(pres);
        }
    }
}

// Stop active sound with given sound object in this sprite
void   Sprite::StopActiveSounds(ASSoundIntf* psndobj)
{
    if (pActiveSounds)
    {
        for (size_t i = 0; i < pActiveSounds->Sounds.GetSize(); )
        {
            Ptr<ActiveSoundItem> psi = pActiveSounds->Sounds[i];
            if (psi->pSoundObject == psndobj)
            {
                psi->pChannel->Stop();
                pActiveSounds->Sounds.RemoveAt(i);
            }
            else
            {
                ++i;
            }
        }
    }
    for (unsigned i = 0; i < mDisplayList.GetCount(); i++)
    {
        const DisplayList::DisplayEntry& dobj = mDisplayList.GetDisplayEntry(i);
        DisplayObjectBase* ch = dobj.GetDisplayObject();
        if (ch->IsInteractiveObject())
        {
            InteractiveObject* pasc = ch->CharToInteractiveObject_Unsafe();
            if (pasc->IsSprite())
                (pasc->CharToSprite())->StopActiveSounds(psndobj);
        }
    }
}

void   Sprite::CheckActiveSounds()
{
    if (pActiveSounds)
    {
        // we need to make a copy of active sounds array here because
        // during onSoundComplete call this movie clip can be removed
        Array<Ptr<ActiveSoundItem> > sounds;
        UPInt i;
        for(i = 0; i < pActiveSounds->Sounds.GetSize(); ++i)
            sounds.PushBack(pActiveSounds->Sounds[i]);

        Array<Ptr<ActiveSoundItem> > completeSounds;

        for(i = 0; i < sounds.GetSize();)
        {
            Ptr<ActiveSoundItem> psi = sounds[i];
            if (!psi->pChannel->IsPlaying())
            {
                if (psi->pSoundObject)
                    psi->pSoundObject->ExecuteOnSoundComplete();
                completeSounds.PushBack(sounds[i]);
                sounds.RemoveAt(i);
                ModifyOptimizedPlayListLocal<Sprite>();
            }
            else
                ++i;
        }

        // remove all completed sounds from active sounds array
        for (i = 0; i < completeSounds.GetSize(); ++i)
        {
            int idx = FindActiveSound(completeSounds[i]);
            if (idx != -1)
                pActiveSounds->Sounds.RemoveAt(idx);
        }
    }
}

// Release an active sound which is attached to this sprite. It is used to pass
// active sounds between MovieClips with MovieClip.attachAudio method
Sprite::ActiveSoundItem*  Sprite::ReleaseActiveSound(Sound::SoundChannel* pchan)
{
    SF_ASSERT(pchan);
    if (!pActiveSounds)
        return NULL;
    for(size_t i = 0; i < pActiveSounds->Sounds.GetSize(); ++i)
    {
        Ptr<ActiveSoundItem> pas = pActiveSounds->Sounds[i];
        if (pas->pChannel.GetPtr() == pchan)
        {
            pActiveSounds->Sounds.RemoveAt(i);
            ModifyOptimizedPlayListLocal<Sprite>();
            pas->AddRef();
            return pas.GetPtr();
        }
    }
    return NULL;
}
// Attach an active sound released by ReleaseActiveSound method.
void    Sprite::AttachActiveSound(Sprite::ActiveSoundItem* psi)
{
    SF_ASSERT(psi);
    if (!pActiveSounds)
        pActiveSounds = SF_NEW ActiveSounds;
    pActiveSounds->Sounds.PushBack(Ptr<ActiveSoundItem>(psi));
    ModifyOptimizedPlayListLocal<Sprite>();
}

int     Sprite::FindActiveSound(ActiveSoundItem* item)
{
    if (pActiveSounds)
    {
        for (UPInt i = 0; i < pActiveSounds->Sounds.GetSize(); ++i)
        {
            if (pActiveSounds->Sounds[i] == item)
                return (int)i;
        }
    }
    return -1;
}

// Release all AS sound objects with given MovieDefImpl from this sprite
void    Sprite::ReleaseAllSounds(MovieDefImpl* powner)
{
    if (pActiveSounds)
    {
        for(size_t i = 0; i < pActiveSounds->ASSounds.GetSize(); )
        {
            ASSoundIntf* psnd = pActiveSounds->ASSounds[i];
            if (psnd->GetOwner() == powner)
            {
                psnd->ReleaseTarget();
                pActiveSounds->ASSounds.RemoveAt(i);
            }
            else {
                ++i;
            }
        }
    }
}

#endif // GFX_ENABLE_SOUND

void    Sprite::SetPause(bool pause)
{
#ifdef GFX_ENABLE_SOUND
    if (pActiveSounds)
    {
        for (size_t i = 0; i < pActiveSounds->Sounds.GetSize(); ++i)
        {
            ActiveSoundItem* psi = pActiveSounds->Sounds[i];
            if (psi->pChannel)
            {
                psi->pChannel->Pause(pause);
            }
        }
    }
    for (unsigned i = 0; i < mDisplayList.GetCount(); i++)
    {
        const DisplayList::DisplayEntry& dobj = mDisplayList.GetDisplayEntry(i);
        DisplayObjectBase*       ch   = dobj.GetDisplayObject();
        if (ch->IsInteractiveObject())
        {
            InteractiveObject* pasc = ch->CharToInteractiveObject_Unsafe();
            pasc->SetPause(pause);
        }
    }
#else
    SF_UNUSED(pause);
#endif
}


// Increment CurrentFrame, and take care of looping.
void    Sprite::IncrementFrameAndCheckForLoop()
{
    CurrentFrame++;

    unsigned loadingFrame = GetLoadingFrame();
    unsigned frameCount   = pDef->GetFrameCount();

    if ((loadingFrame < frameCount) && (CurrentFrame >= loadingFrame))
    {
        // We can not advance past the loading frame, since those tags are not yet here.
        CurrentFrame = (loadingFrame > 0) ? loadingFrame - 1 : 0;
    }
    else if (CurrentFrame >= frameCount)
    {
        // Loop.
        CurrentFrame = 0;
        SetHasLoopedPriv(true);
        if (frameCount > 1)
        {
            mDisplayList.MarkAllEntriesForRemoval(this);
            SetDirtyFlag();
        }
        else
        {
            // seems like we can stop advancing 1 frame sprite here
            SetPlayState(State_Stopped);
        }
    }
}

void    Sprite::AdvanceFrame(bool nextFrame, float framePos)
{
    SF_UNUSED(framePos);
    if (IsAdvanceDisabled() || IsUnloading())
        return;

    // Keep this (particularly GASEnvironment) alive during execution!
    Ptr<Sprite> thisPtr(this);

    SF_ASSERT(pDef && GetMovieImpl() != NULL);

#ifdef GFX_ENABLE_SOUND
    if (nextFrame)
    {
        CheckActiveSounds();
        if (PlayStatePriv != State_Stopped)
        {
            SoundStreamDef* psoundDef = pDef->GetSoundStream();
            if (psoundDef)
            {
                if (!psoundDef->ProcessSwfFrame(GetMovieImpl(), CurrentFrame, this))
                    pDef->SetSoundStream(NULL);
            }
        }
    }
#endif // GFX_ENABLE_SOUND

    if (HasAvmObject())
        GetAvmSprite()->AdvanceFrame(nextFrame, framePos);
}

void    Sprite::ExecuteFrame0Events()
{
    if (!IsOnEventLoadCalled())
    {       
        // Must do loading events for root level sprites.
        // For child sprites this is done by the dlist,
        // but root movies don't get added to a dlist, so we do it here.
        // Also, this queues up tags and actions for frame 0.
        SetOnEventLoadCalled();

        // Execute this frame's init actions, if necessary.
        if (HasAvmObject())
            GetAvmSprite()->ExecuteInitActionFrameTags(0);
        // Can't call OnLoadEvent here because implementation of onLoad handler is different
        // for the root: it seems to be called after the first frame as a special case, presumably
        // to allow for load actions to exits. That, however, is NOT true for nested movie clips.
        ExecuteFrameTags(0);
        if (HasAvmObject())
            GetAvmSprite()->ExecuteFrame0Events();

        // Required to propogate view/proj matrices for movies loaded from cmd line.
//        UpdateViewAndPerspective();
    }
}

// Execute the tags associated with the specified frame.
// frame is 0-based
void    Sprite::ExecuteFrameTags(unsigned frame)
{
    if (!IsMovieClip() && frame != 0)
        return; 

    // Keep this (particularly GASEnvironment) alive during execution!
    Ptr<Sprite> thisPtr(this);

    SF_ASSERT(frame != unsigned(~0));
    //SF_ASSERT(frame < GetLoadingFrame());
    if (frame >= GetLoadingFrame())
        return;

    // Execute this frame's init actions, if necessary.
    //!AB: replaced by explicit call before every ExecuteFrameTag
    //if (HasAvmObject())
    //    GetAvmSprite()->ExecuteInitActionFrameTags(frame);

    const Frame playlist = pDef->GetPlaylist(frame);
    for (unsigned i = 0; i < playlist.GetTagCount(); i++)
    {
        ExecuteTag*  e = playlist.GetTag(i);

#ifdef GFX_TRACE_TIMELINE_EXECUTION
        printf("\nexecution, fr = %d, %s\n", frame, GetCharacterHandle()->GetNamePath().ToCStr());
        e->Trace("");
#endif
        e->ExecuteWithPriority(this, ActionPriority::AP_Frame);
    }
    // Pass to AVM object
    if (HasAvmObject())
        GetAvmSprite()->ExecuteFrameTags(frame);
}

void Sprite::ExecuteImportedInitActions(MovieDef* psourceMovie)
{
    SF_ASSERT(psourceMovie);

    MovieDefImpl* pdefImpl = static_cast<MovieDefImpl*>(psourceMovie);
    MovieDataDef* pdataDef = pdefImpl->GetDataDef();

    for(UPInt f = 0, fc = pdataDef->GetInitActionListSize(); f < fc; ++f)
    {
        Frame actionsList;
        if (pdataDef->GetInitActions(&actionsList, (int)f))
        {
            for(unsigned i = 0; i < actionsList.GetTagCount(); ++i)
            {
                ExecuteTag*  e = actionsList.GetTag(i);
                // InitImportActions must get the right binding context since
                // their binding scope does not match that of sprite.
                if (e->IsInitImportActionsTag())
                    ((GFxInitImportActions*)e)->ExecuteInContext(this, pdefImpl);
                else
                    e->ExecuteWithPriority(this, ActionPriority::AP_Highest);
            }
        }        
    }
    SetDirtyFlag();
}



// Add the given action buffer to the list of action
// buffers to be processed at the end of the next frame advance in root.    
// void    Sprite::AddActionBuffer(GASActionBuffer* a, ActionPriority::Priority prio) 
// {
//     MovieImpl::ActionEntry* pe = GetMovieImpl()->InsertEmptyAction(prio);
//     if (pe) pe->SetAction(this, a);
// }

// Set the sprite state at the specified frame number.
// 0-based frame numbers!!  (in contrast to ActionScript and Flash MX)
void    Sprite::GotoFrame(unsigned targetFrameNumber)
{
    if (!IsMovieClip() || IsUnloading())
        return;
    //  IF_VERBOSE_DEBUG(LogMsg("sprite::GotoFrame(%d)\n", targetFrameNumber));//xxxxx

    targetFrameNumber = (unsigned)Alg::Clamp<int>(targetFrameNumber, 0, (int)GetLoadingFrame() - 1);

#ifdef GFX_ENABLE_SOUND
    // stop streaming sound
    SetStreamingSound(NULL);
#endif

    if (targetFrameNumber < CurrentFrame)
    {
        //SF_ASSERT(!(GetName() == "ss7" && targetFrameNumber == 6 && CurrentFrame == 7));
#ifdef GFX_TRACE_TIMELINE_EXECUTION
        printf("\n #### gotoFrame backward, from %d to %d, this = %s\n", 
            CurrentFrame, targetFrameNumber, GetCharacterHandle()->GetNamePath().ToCStr());
        DumpDisplayList(0, "Before");
#endif
        // Mark all "deletable" characters for removal. The "deletable" characters include 
        // the following characters:
        //    the ones, created by the timeline, even if they were touched by ActionScript 
        //    (AcceptAnimMoves = false), but only if the "CreateFrame" property of these 
        //    characters is greater or equal than TargetFrame (so, they were created ON 
        //    or AFTER the TargetFrame). 
        mDisplayList.MarkAllEntriesForRemoval(this, (targetFrameNumber) ? targetFrameNumber-1 : 0);
        //DumpDisplayList(0, "Interm1");

        if (targetFrameNumber >= 1)
        {
            // We need to make a snapshot from Frame 0 to targetFrame - 1. 
            // The TargetFrame is excluded because we would execute all tags for 
            // the target frame by the regular way - calling ExecuteFrameTags. 
            TimelineSnapshot snapshot(TimelineSnapshot::Direction_Backward, GetMovieImpl()->GetHeap(), this);
#ifdef GFX_TRACE_TIMELINE_EXECUTION
            printf("+++ id = %d, %s, fr %d to %d, currentframe %d ++++++++\n", 
                GetId(), (GetName().ToCStr() ? GetName().ToCStr() : ""),
                0, targetFrameNumber, CurrentFrame);
#endif
            snapshot.MakeSnapshot(pDef, 0, targetFrameNumber-1);

            // Set the current frame to target one and execute the snapshot
            CurrentFrame = targetFrameNumber;
            snapshot.ExecuteSnapshot(this);
        }
        else
        {
            // if targetFrame is 0, no snapshot is necessary
            CurrentFrame = targetFrameNumber;
        }
        //DumpDisplayList(0, "Interm2");

        // Execute this frame's init actions, if necessary.
        if (HasAvmObject())
            GetAvmSprite()->ExecuteInitActionFrameTags(targetFrameNumber);
        // Execute target frame's tags
        ExecuteFrameTags(targetFrameNumber);
        //DumpDisplayList(0, "Interm3");

        // Unload all remaining mark-for-remove characters. Note, this is not just removing 
        // such characters from the display list. "Unload" means: if the character or one of 
        // its children has onUnload handler, then this character is not removed from the 
        // display list - it is being moved to another depth and the onUnload event is 
        // scheduled. If no onUnload event handler is defined (neither the character nor 
        // its children have it) then this character is unloaded instantly and removed 
        // from the display list. 
        mDisplayList.UnloadMarkedObjects(this);

#ifdef GFX_TRACE_TIMELINE_EXECUTION
        DumpDisplayList(0, "After");
        printf(" ^^^^ end of gotoFrame backward, this = %s\n\n", 
            GetCharacterHandle()->GetNamePath().ToCStr());
#endif
        mDisplayList.CheckConsistency(this);
    }
    else if (targetFrameNumber > CurrentFrame)
    {
#ifdef GFX_TRACE_TIMELINE_EXECUTION
        printf("\n #### gotoFrame forward, from %d to %d, this = %s\n", 
            CurrentFrame, targetFrameNumber, GetCharacterHandle()->GetNamePath().ToCStr());
        DumpDisplayList(0, "Before");
#endif
        // Need to gather a snapshot only if targetFrame is greater than 1
        // and if it is greater than next frame (CurrentFrame + 1). For the next 
        // frame it is not enough to just execute frame tags by the regular way - 
        // calling to ExecuteFrameTags. 
        if (targetFrameNumber > 1 && targetFrameNumber > CurrentFrame + 1)
        {
            TimelineSnapshot snapshot(TimelineSnapshot::Direction_Forward, GetMovieHeap(), this);
            // Making the snapshot starting from the CurrentFrame + 1 and finishing
            // at targetFrame - 1. We will execute tags for the target frame
            // on regular basis by calling to ExecuteFrameTags.
#ifdef GFX_TRACE_TIMELINE_EXECUTION
            printf("+++ id = %d, %s, fr %d to %d, currentframe %d ++++++++\n", 
                GetId(), (GetName().ToCStr() ? GetName().ToCStr() : ""),
                CurrentFrame, targetFrameNumber, CurrentFrame);
#endif
            snapshot.MakeSnapshot(pDef, CurrentFrame + 1, targetFrameNumber-1);

            if (HasAvmObject())
            {
                // need to run init actions for every frame being skipped, as well as
                // for target one. But for the target frame InitAction tags will be
                // executed from inside the ExecuteFrameTags.
                for (unsigned f = CurrentFrame + 1; f < targetFrameNumber; ++f)
                {
                    GetAvmSprite()->ExecuteInitActionFrameTags(f);
                }
            }

            CurrentFrame = targetFrameNumber;
            snapshot.ExecuteSnapshot(this);
        }
        else
            CurrentFrame = targetFrameNumber;

        // Execute this frame's init actions, if necessary.
        if (HasAvmObject())
            GetAvmSprite()->ExecuteInitActionFrameTags(targetFrameNumber);
        // Execute target frame's tags
        ExecuteFrameTags(targetFrameNumber);

#ifdef GFX_TRACE_TIMELINE_EXECUTION
        DumpDisplayList(0, "After");
        printf(" ^^^^ end of gotoFrame forward, this = %s\n\n", 
            GetCharacterHandle()->GetNamePath().ToCStr());
#endif

        mDisplayList.CheckConsistency(this);
    }

    // GotoFrame stops by default.
    PlayStatePriv = State_Stopped;
}

bool    Sprite::GotoLabeledFrame(const char* label, int offset)
{
    unsigned    targetFrame = SF_MAX_UINT;
    if (pDef->GetLabeledFrame(label, &targetFrame, 0))
    {
        GotoFrame((unsigned)((int)targetFrame + offset));
        return true;
    }
    else
    {       
        LogWarning("MovieImpl::GotoLabeledFrame('%s') unknown label", label);
        return false;
    }
}

void    Sprite::SetPlayState(PlayState s)
{ 
    PlayStatePriv = s;
    if (!IsUnloading() && !IsUnloaded())
        ModifyOptimizedPlayListLocal<Sprite>();
#ifdef GFX_ENABLE_SOUND
    if (PlayStatePriv == State_Stopped)
    {
        SetStreamingSound(NULL);
    }
#endif
}

void    Sprite::CloneInternalData(const InteractiveObject* src)
{
    SF_ASSERT(src);
    DisplayObjContainer::CloneInternalData(src);

    if (HasAvmObject())
        GetAvmSprite()->CloneInternalData(src);
//     if (src->IsSprite())
//     {
//         const Sprite* pSrcSprite = src->CharToSprite();
//         if (pSrcSprite->ActsAsButton())
//             SetHasButtonHandlers (true);
//     }
    SetDirtyFlag();
}

bool  Sprite::MustBeInPlaylist() const
{
    if (HasAvmObject())
        return GetAvmSprite()->MustBeInPlaylist();
    return false;
}


// Returns 0 if nothing to do
// 1 - if need to add to optimized play list
// -1 - if need to remove from optimized play list
int Sprite::CheckAdvanceStatus(bool playingNow)
{
    int rv = 0;
    // First of all, check if advance is disabled at all. Advance is disabled
    // * if noAdvance extension property is set;
    // * if sprite is invisible and _global.noInvisibleAdvance extension property is set;
    // * if sprite is unloading or unloaded from timeline.
    bool advanceDisabled = (IsAdvanceDisabled() || !CanAdvanceChar());

    // Check if movie is playable.
    //bool advancable = (!advanceDisabled && GetPlayState() == State_Playing);
    bool advancable = (!advanceDisabled && (GetPlayState() == State_Playing
#ifdef GFX_ENABLE_SOUND
        || (pActiveSounds && pActiveSounds->Sounds.GetSize() != 0)
#endif
        ));

    if (advancable) 
    {
        // if it is already playing and advancable - do nothing (return 0).
        // otherwise, return 1 (play).
        rv = !playingNow; //(playingNow) ? 0 : 1
#ifdef SF_TRACE_ADVANCE
        if (rv)
        {
            printf("+++ Advance: Starting to play: %s, state %s\n", GetCharacterHandle()->GetNamePath().ToCStr(),
                (GetPlayState() == State_Playing) ? "playing" : "stopped");
        }
#endif
    }
    else
    {
        if (playingNow)
        {
            // check, if the onEnterFrame exists
            if (!advanceDisabled)
            {
                if (!MustBeInPlaylist())
                {
                    rv = -1;
#ifdef SF_TRACE_ADVANCE
                    printf("+++ Advance: Stopping play (no onEnterFrame): %s, state %s\n", 
                        GetCharacterHandle()->GetNamePath().ToCStr(),
                        (GetPlayState() == State_Playing) ? "playing" : "stopped");
#endif
                }
            }
            else
            {
                rv = -1;
#ifdef SF_TRACE_ADVANCE
                printf("+++ Advance: Stopping play (adv disabled): %s, state %s\n", 
                    GetCharacterHandle()->GetNamePath().ToCStr(),
                    (GetPlayState() == State_Playing) ? "playing" : "stopped");
#endif
            }
        }
        else
        {
            if (!advanceDisabled)
            {
                if (MustBeInPlaylist())
                {
                    rv = 1;
#ifdef SF_TRACE_ADVANCE
                    printf("+++ Advance: Starting to play (onEnterFrame): %s, state %s\n", 
                        GetCharacterHandle()->GetNamePath().ToCStr(),
                        (GetPlayState() == State_Playing) ? "playing" : "stopped");
#endif
                }
            }
        }
    }
    return rv;
}

// Add an object to the display list.
DisplayObjectBase*   Sprite::AddDisplayObject(
    const CharPosInfo &pos, 
    const ASString& name,
    const ArrayLH<SwfEvent*, StatMD_Tags_Mem>* peventHandlers,
    const void *pinitSource,
    unsigned createFrame,
    UInt32 addFlags,
    CharacterCreateInfo* pcharCreateOverride,
    InteractiveObject* origChar)
{
    if (HasAvmObject())
        return GetAvmSprite()->AddDisplayObject(pos, name, peventHandlers, pinitSource, createFrame, addFlags, pcharCreateOverride, origChar);
    return NULL;
}

void Sprite::CreateAndReplaceDisplayObject(const CharPosInfo &pos, const ASString& name, DisplayObjectBase** newChar)
{
    if (newChar)
        *newChar = NULL;
    DisplayObjContainer::CreateAndReplaceDisplayObject(pos, name, newChar);

    if (newChar && *newChar)
    {
        // we need to set correct create frame to the character
        (*newChar)->SetCreateFrame(CurrentFrame);
    }
}

bool Sprite::IsLevelMovie() const
{
    if (HasAvmObject())
        return GetAvmSprite()->IsLevelMovie();
    return false;
}

int Sprite::GetLevel() const
{
    return GetAvmSprite()->GetLevel();
}

bool Sprite::OnEvent(const EventId& id)
{
    if (HasAvmObject())
        return GetAvmSprite()->OnEvent(id);
    return false;
}

// Handle a button event.
bool    Sprite::OnMouseEvent(const EventId& id)
{
    if (IsLevelMovie()) // level movies should not handle button events
        return false;

    // Handle it ourself?
    bool            handled = false;
    if (HasAvmObject())
        handled = GetAvmSprite()->OnEvent(id);
    InteractiveObject* pparent;

    if (!handled && ((pparent=GetParent()) != 0))
    {
        // We couldn't handle it ourself, so
        // pass it up to our parent.
        return pparent->OnMouseEvent(id);
    }
    return false;
}

// Special event handler; ensures that unload is called on child items in timely manner.
void    Sprite::OnEventUnload()
{
    SetUnloading();
#ifdef GFX_ENABLE_SOUND
    if (pActiveSounds)
        delete pActiveSounds;
    pActiveSounds = NULL;
#endif

    if (pHitAreaHandle)
        SetHitArea(0);

    SetDirtyFlag();

    // Cause all children to be unloaded.
    mDisplayList.Clear(this);
    DisplayObjContainer::OnEventUnload();
}

bool    Sprite::OnUnloading()
{
    SetDirtyFlag();

    // Cause all children to be unloaded.
    bool mayRemove = mDisplayList.UnloadAll(this);
    if (HasAvmObject())
        mayRemove = GetAvmObjImpl()->OnUnloading(mayRemove);
    return mayRemove;
}

bool    Sprite::OnKeyEvent(const EventId& id, int* pkeyMask)
{ 
    if (!HasAvmObject())
        return false;
    bool rv = false;
    if (id.Id == EventId::Event_KeyDown)
    {
        // run onKeyDown first.
        rv = GetAvmSprite()->OnEvent(id);

        // also simmulate on(keyPress)
        // covert Event_KeyDown to Event_KeyPress
        SF_ASSERT(pkeyMask != 0);

        // check if keyPress already was handled then do not handle it again
        if (!((*pkeyMask) & InteractiveObject::KeyMask_KeyPress))
        {
            int kc = id.ConvertToButtonKeyCode();
            if (kc && (rv = GetAvmSprite()->OnEvent(EventId(EventId::Event_KeyPress, short(kc), 0)))!=0)
            {
                *pkeyMask |= InteractiveObject::KeyMask_KeyPress;
            }
        }

        MovieImpl* proot = GetMovieImpl();
        if (proot->IsKeyboardFocused(this, id.KeyboardIndex) && (id.KeyCode == Key::Return || id.KeyCode == Key::Space))
        {
            if (IsFocusRectEnabled() || proot->IsAlwaysEnableKeyboardPress())
            {
                // if focused and enter - simulate on(press)/onPress and on(release)/onRelease
                EventId e(id);
                e.Id = EventId::Event_Press;
                GetAvmSprite()->OnEvent (e);
                e.Id = EventId::Event_Release;
                GetAvmSprite()->OnEvent (e);
            }
        }
        return rv;
    }
    else
        return GetAvmSprite()->OnEvent(id); 
}

void    Sprite::DefaultOnEventLoad()
{   
    ExecuteFrameTags(0);
    SetJustLoaded(true);
}

// Do the events that happen when there is XML data waiting
// on the XML socket connection.
void    Sprite::OnEventXmlsocketOnxml()
{
    SF_DEBUG_WARNING(1, "OnEventXmlsocketOnxml: unimplemented");  
    if (HasAvmObject())
        GetAvmSprite()->OnEvent(EventId::Event_SockXML);
}

// Do the events That (appear to) happen on a specified interval.
void    Sprite::OnEventIntervalTimer()
{
    //SF_DEBUG_WARNING(1, "OnEventIntervalTimer: unimplemented");   
    //OnEvent(EventId::TIMER);
}

// Do the events that happen as a MovieClip (swf 7 only) loads.
void    Sprite::OnEventLoadProgress()
{   
    SF_DEBUG_WARNING(1, "OnEventLoadProgress: unimplemented");
    if (HasAvmObject())
        GetAvmSprite()->OnEvent(EventId::Event_LoadProgress);
}

bool Sprite::PointTestLocal(const Render::PointF &pt, UInt8 hitTestMask) const 
{ 
    if (IsHitTestDisableFlagSet())
        return false;

    if (!DoesScale9GridExist() && !GetBounds(Matrix2F()).Contains(pt))
        return false;

    if ((hitTestMask & HitTest_IgnoreInvisible) && !GetVisible())
        return false;

    if (DisplayObjContainer::PointTestLocal(pt, hitTestMask))
        return true;
    if (pDrawingAPI)
    {
        if (pDrawingAPI->DefPointTestLocal(pt, hitTestMask & HitTest_TestShape, this))
            return true;
    }

    return false;
}

void Sprite::OnGettingKeyboardFocus(unsigned controllerIdx, FocusMovedType fmt)
{
    if (fmt == GFx_FocusMovedByKeyboard &&
        ActsAsButton() && !GetMovieImpl()->IsDisableFocusRolloverEvent()) 
        OnMouseEvent(EventId(EventId::Event_RollOver, 0, 0, 0, UInt8(controllerIdx)));
}

// invoked when focused item is about to lose keyboard focus input (mouse moved, for example)
bool Sprite::OnLosingKeyboardFocus(InteractiveObject*, unsigned controllerIdx, FocusMovedType fmt) 
{
    if (fmt == GFx_FocusMovedByKeyboard && ActsAsButton() && GetMovieImpl()->IsFocusRectShown(controllerIdx) && 
        !GetMovieImpl()->IsDisableFocusRolloverEvent())
    {
        OnMouseEvent(EventId(EventId::Event_RollOut, 0, 0, 0, UInt8(controllerIdx)));
    }
    return true;
}

void Sprite::PropagateNoAdvanceGlobalFlag()
{
    bool actualValue = IsNoAdvanceGlobalFlagSet();
    for (UPInt i = 0, n = mDisplayList.GetCount(); i < n; ++i)
    {
        InteractiveObject* ch = mDisplayList.GetDisplayObject(i)->CharToInteractiveObject();
        if (ch)
        {
            ch->SetNoAdvanceGlobalFlag(IsNoAdvanceGlobalFlagSet() || actualValue);
            ch->PropagateNoAdvanceGlobalFlag();
            ch->ModifyOptimizedPlayList();
        }
    }   
}

void Sprite::PropagateNoAdvanceLocalFlag()
{
    bool actualValue = IsNoAdvanceLocalFlagSet();
    for (UPInt i = 0, n = mDisplayList.GetCount(); i < n; ++i)
    {
        InteractiveObject* ch = mDisplayList.GetDisplayObject(i)->CharToInteractiveObject();
        if (ch)
        {
            ch->SetNoAdvanceLocalFlag(IsNoAdvanceLocalFlagSet() || actualValue);
            ch->PropagateNoAdvanceLocalFlag();
            ch->ModifyOptimizedPlayList();
        }
    }   
}

void Sprite::SetVisible(bool visible)            
{ 
    SetVisibleFlag(visible); 
    bool noAdvGlob = !visible && GetMovieImpl()->IsNoInvisibleAdvanceFlagSet();
    if (noAdvGlob != IsNoAdvanceGlobalFlagSet())
    {
        SetNoAdvanceGlobalFlag(noAdvGlob);
        ModifyOptimizedPlayListLocal<Sprite>();
        PropagateNoAdvanceGlobalFlag();
    }
    SetDirtyFlag(); 
}

// Drawing API support
//=============================================================================

DrawingContext* Sprite::GetDrawingContext()
{
    if (!pDrawingAPI)
    {
        // special case to handle dynamic mask: need to remove
        // mask first, create new node, reinsert old node in the
        // new node and later set mask back (see bottom of the method).
        Ptr<DisplayObject> mask = GetMask();
        if (mask)
            SetMask(NULL);        

        pDrawingAPI = *GetMovieImpl()->CreateDrawingContext();
        Render::TreeContainer* c = ConvertToTreeContainer();
        // drawing API objects appear under the existing shapes.
        // See also Sprite::GetRenderContainer()
        c->Insert(0, pDrawingAPI->GetTreeNode());

        // Restoring dynamic mask (see above).
        if (mask)
            SetMask(mask);
    }
    return pDrawingAPI;
}

Render::TreeContainer* Sprite::GetRenderContainer()
{
    Render::TreeContainer* cont = DisplayObjContainer::GetRenderContainer();
    if (pDrawingAPI)
    {
        // return sprite's container, not the one that was created for drawing api by Sprite::GetDrawingContext()
        // it should contain at least two children: DrawingApi at 0 and Sprite's node at 1.
        SF_ASSERT(cont->GetSize() >= 2); 
        cont = static_cast<Render::TreeContainer*>(cont->GetAt(1));
    }
    return cont;
}

void Sprite::SetHitArea(Sprite* phitArea)
{
    Sprite* phaSprite = GetHitArea(); 
    if (phaSprite)
        phaSprite->SetHitAreaHolder(0);

    if (phitArea)
    {
        pHitAreaHandle = phitArea->GetCharacterHandle();
        phitArea->SetHitAreaHolder(this);
    }
    else
    {
        pHitAreaHandle = 0;
    }
    if (HasAvmObject())
        GetAvmSprite()->SetHitAreaNotify(phitArea);
}

Sprite* Sprite::GetHitArea() const
{
    if (pHitAreaHandle)
    {
        DisplayObject* pasCharacter = pHitAreaHandle->ResolveCharacter(GetMovieImpl());
        if (pasCharacter)
            return pasCharacter->CharToSprite();
    }
    return 0;
}

void Sprite::SetName(const ASString& name)
{
    DisplayObjContainer::SetName(name);
    // need to reset cached character if name is changed.
    mDisplayList.ResetCachedCharacter();
}

bool Sprite::IsFocusEnabled(FocusMovedType fmt) const
{
    if (fmt == GFx_FocusMovedByMouse)
        return InteractiveObject::IsFocusEnabled(fmt);

    // For AS3 FocusEnabled is always undefined. Shouldn't this logic be moved to
    // AS2_Avm... and AS3_Avm... files?
    if (!FocusEnabled.IsDefined() && HasAvmObject())
    {
        return GetAvmSprite()->IsFocusEnabled(fmt);
    }
    else if (FocusEnabled.IsFalse())
        return (ActsAsButton());
    
    return FocusEnabled.IsTrue();
}


}} // namespace Scaleform::GFx
