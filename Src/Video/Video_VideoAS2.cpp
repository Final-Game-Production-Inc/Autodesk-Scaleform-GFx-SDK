/**************************************************************************

Filename    :   Video_VideoAS2.cpp
Content     :   AS2-Specific part of video linking, located in separate
                file to avoid video linking.
Created     :   April 20011
Authors     :   Vladislav Merker, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Video/Video_Video.h"

#ifdef GFX_AS2_SUPPORT
#include "Video/AS2/AS2_NetConnection.h"
#include "Video/AS2/AS2_NetStream.h"
#include "Video/AS2/AS2_VideoObject.h"
#include "GFx/AS2/Audio/AS2_SoundObject.h"
#endif

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

class AS2VideoSupportImpl : public AS2VideoSupport
{
public:
    AS2VideoSupportImpl() {}
    virtual DisplayObjectBase* CreateASCharacter(MovieImpl* proot, CharacterDef* pcharDef,
                                                 InteractiveObject* pparent,
                                                 ResourceId rid, MovieDefImpl* pbindingImpl);
    virtual void RegisterASClasses(AS2::GlobalContext& gc, AS2::ASStringContext& sc);
    virtual void AttachAudio(AS2::Object*, Sprite*);       
};

AS2VideoSupport* AS2VideoSupport::CreateInstance()
{
#ifdef GFX_AS2_SUPPORT
    return SF_NEW AS2VideoSupportImpl;
#else
    return NULL;
#endif
}


//////////////////////////////////////////////////////////////////////////
//

// AS2 video character instance
DisplayObjectBase* AS2VideoSupportImpl::CreateASCharacter(MovieImpl* proot, CharacterDef* pcharDef,
                                                          InteractiveObject* pparent,
                                                          ResourceId rid, MovieDefImpl* pbindingImpl)
{
#ifdef GFX_AS2_SUPPORT
    UByte* pm = (UByte*)SF_HEAP_ALLOC(proot->GetMovieHeap(),
        ((sizeof(VideoCharacter) + 3) & ~3) + sizeof(AS2::AvmVideoCharacter), StatMV_ActionScript_Mem);

    VideoCharacter* pvideoChar = new (pm) VideoCharacter(
        static_cast<VideoCharacterDef*>(pcharDef), pbindingImpl, proot->pASMovieRoot, pparent, rid);

    AS2::AvmVideoCharacter* pavmChar = new (pm + ((sizeof(VideoCharacter) + 3) & ~3)) AS2::AvmVideoCharacter(pvideoChar);
    SF_UNUSED(pavmChar);

    return pvideoChar;
#else
    SF_UNUSED5(proot, pcharDef, pparent, rid, pbindingImpl);
    return NULL;
#endif
}

// Register AS2 video and audio classes
#ifdef GFX_AS2_SUPPORT
void AS2VideoSupportImpl::RegisterASClasses(AS2::GlobalContext& gc, AS2::ASStringContext& sc)
{
#ifdef GFX_ENABLE_SOUND
    gc.AddBuiltinClassRegistry<AS2::ASBuiltin_Sound, AS2::SoundCtorFunction>(sc, gc.pGlobal);
#endif
    gc.AddBuiltinClassRegistry<AS2::ASBuiltin_NetConnection, AS2::NetConnectionCtorFunction>(sc, gc.pGlobal);
    gc.AddBuiltinClassRegistry<AS2::ASBuiltin_NetStream, AS2::NetStreamCtorFunction>(sc, gc.pGlobal);
    gc.AddBuiltinClassRegistry<AS2::ASBuiltin_Video, AS2::VideoCtorFunction>(sc, gc.pGlobal);
}
#else
void AS2VideoSupportImpl::RegisterASClasses(AS2::GlobalContext&, AS2::ASStringContext&) {}
#endif

// Attach AS2 audio target
void AS2VideoSupportImpl::AttachAudio(AS2::Object* psource, Sprite* ptarget)
{
#ifdef GFX_AS2_SUPPORT
    AS2::NetStream* pns = (AS2::NetStream*)psource;
    pns->SetAudioTarget(ptarget);
#else
    SF_UNUSED2(psource, ptarget);
#endif
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO
