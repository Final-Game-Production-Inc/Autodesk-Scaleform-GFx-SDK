/**************************************************************************

Filename    :   Video_VideoAS3.cpp
Content     :   AS3-Specific part of video linking, located in separate
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

#ifdef GFX_AS3_SUPPORT
#include "Video/AS3/AS3_VideoCharacter.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#endif

namespace Scaleform { namespace GFx {

//////////////////////////////////////////////////////////////////////////
//

#ifdef GFX_AS3_SUPPORT
// AS3 VM classes table
namespace AS3 { 

namespace fl_media
{
    extern const ClassInfo VideoCI;
}

namespace fl_net
{
    extern const ClassInfo NetStreamCI;
}

namespace Classes
{
    class Video;
    class NetStream;

    const ClassInfo* VideoInheritanceTable[] =
    {
        &fl_media::VideoCI,
        &fl_net::NetStreamCI,
        NULL
    };
}} // AS3::Classes
#endif

namespace Video {

//////////////////////////////////////////////////////////////////////////
//

class AS3VideoSupportImpl : public AS3VideoSupport
{
public:
    AS3VideoSupportImpl() {}
    virtual DisplayObjectBase* CreateASCharacter(MovieImpl* proot, CharacterDef* pcharDef,
                                                 InteractiveObject* pparent,
                                                 ResourceId rid, MovieDefImpl* pbindingImpl);
    virtual void RegisterASClasses(AS3::VM* pvm);
};

AS3VideoSupport* AS3VideoSupport::CreateInstance()
{
#ifdef GFX_AS3_SUPPORT
    return SF_NEW AS3VideoSupportImpl;
#else
    return NULL;
#endif
}

//////////////////////////////////////////////////////////////////////////
//

// AS3 video character instance
DisplayObjectBase* AS3VideoSupportImpl::CreateASCharacter(MovieImpl* proot, CharacterDef* pcharDef,
                                                          InteractiveObject* pparent,
                                                          ResourceId rid, MovieDefImpl* pbindingImpl)
{
#ifdef GFX_AS3_SUPPORT
    UByte* pm = (UByte*)SF_HEAP_ALLOC(proot->GetMovieHeap(),
        ((sizeof(VideoCharacter) + 3) & ~3) + sizeof(AS3::AvmVideoCharacter), StatMV_ActionScript_Mem);

    VideoCharacter* pvideoChar = new (pm) VideoCharacter(static_cast<VideoCharacterDef*>(pcharDef),
        pbindingImpl, proot->pASMovieRoot, pparent, rid);

    AS3::AvmVideoCharacter* pavmChar = new (pm + ((sizeof(VideoCharacter) + 3) & ~3)) AS3::AvmVideoCharacter(pvideoChar);
    SF_UNUSED(pavmChar);

    return pvideoChar;
#else
    SF_UNUSED5(proot, pcharDef, pparent, rid, pbindingImpl);
    return NULL;
#endif
}

// AS3 VM classes registration
void AS3VideoSupportImpl::RegisterASClasses(AS3::VM* pvm)
{
#ifdef GFX_AS3_SUPPORT
    SF_ASSERT(pvm);
    pvm->RegisterClassInfoTable(AS3::Classes::VideoInheritanceTable);
#else
    SF_UNUSED(pvm);
#endif
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO
