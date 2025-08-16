/**************************************************************************

Filename    :   AS2_VideoObject.cpp
Content     :   GFx video: AS2 VideoObject and AvmVideoCharacter classes
Created     :   May, 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Video/AS2/AS2_VideoObject.h"

#ifdef GFX_ENABLE_VIDEO

#include "Video/AS2/AS2_NetStream.h"

namespace Scaleform { namespace GFx {

using namespace Video;

namespace AS2 {

//////////////////////////////////////////////////////////////////////////
//

AvmVideoCharacter::AvmVideoCharacter(VideoCharacter* pchar) : AvmCharacter(pchar), AvmVideoCharacterBase()
{
    pProto = GetVideoASObject()->Get__proto__();
}

bool AvmVideoCharacter::SetMember(Environment* penv, const ASString& name, const Value& val,
                                  const PropFlags& flags)
{
    if (name.IsStandardMember())                    
        if (SetStandardMember(GetStandardMemberConstant(name), val, 0))
            return true;

    if (name == "deblocking")
    {
        Deblocking = (int)val.ToNumber(penv);
        return true;
    }
    else if (name == "smoothing")
    {
        Smoothing = val.ToBool(penv);
        return true;
    }

    if (GetVideoASObject())
    {
        return GetVideoASObject()->SetMember(penv, name, val, flags);
    }
    return false;
}

bool AvmVideoCharacter::GetMember(Environment* penv, const ASString& name, Value* val)
{
    if (name.IsStandardMember())                    
        if (GetStandardMember(GetStandardMemberConstant(name), val, 0))
            return true;        

    if (name == "width")
    {
        val->SetNumber(PictureWidth);
        return true;
    }
    else if (name == "height")
    {
        val->SetNumber(PictureHeight);
        return true;
    }
    else if (name == "deblocking")
    {
        val->SetNumber(Deblocking);
        return true;
    }
    else if (name == "smoothing")
    {
        val->SetBool(Smoothing);
        return true;
    }

    if (GetVideoASObject())
    {
        return GetVideoASObject()->GetMember(penv, name, val);
    }
    return false;
}

UInt32 AvmVideoCharacter::GetStandardMemberBitMask() const
{
    return UInt32(AvmCharacter::M_BitMask_PhysicalMembers |
                  AvmCharacter::M_BitMask_CommonMembers | 
                  (1 << AvmCharacter::M_xmouse) |
                  (1 << AvmCharacter::M_ymouse));
}

VideoObject* AvmVideoCharacter::GetVideoASObject()
{ 
    if (!ASVideoObj)
        ASVideoObj = *SF_HEAP_AUTO_NEW(this) VideoObject(GetGC(), pDispObj);
    return ASVideoObj;
}

//////////////////////////////////////////////////////////////////////////
//

void VideoProto::AttachVideo(const FnCall& fn)
{
    if (fn.NArgs < 1)
    {
        fn.Env->LogScriptError("Error: Video.attachVideo requires one argument (NetStream object)\n");
        return;
    }
    if (!fn.Arg(0).IsObject())
        return;
    if (!fn.ThisPtr || fn.ThisPtr->GetObjectType() != Object_Video)
        return;    
    AvmCharacter* pchar  = static_cast<AvmCharacter*>(fn.ThisPtr);
    VideoCharacter* pvideo = static_cast<VideoCharacter*>(pchar->GetDispObj());

    Object* obj = fn.Arg(0).ToObject(fn.Env);
    VideoProvider* pprovider = NULL;
    if (obj->GetObjectType() == Object_NetStream)
    {
        NetStream* pnetstream = static_cast<NetStream*>(fn.Arg(0).ToObject(fn.Env));
        if (pnetstream)
            pprovider = pnetstream->GetVideoProvider();
    }
    if (pvideo)
    {
        pvideo->AttachVideoProvider(pprovider);
        if (pprovider)
        {
            if (!fn.Env->CheckExtensions())
            {
                VideoCharacter* currentVideo = pprovider->RemoveFirstVideoCharacter();
                if (currentVideo)
                    currentVideo->AttachVideoProvider(NULL);
            }
            pprovider->RegisterVideoCharacter(pvideo);
        }
    }
}

void VideoProto::Clear(const FnCall& fn)
{
    AvmCharacter* pchar = static_cast<AvmCharacter*>(fn.ThisPtr);
    VideoCharacter* pvideo = static_cast<VideoCharacter*>(pchar->GetDispObj());
    if (pvideo)
        pvideo->ReleaseTexture();
}

//////////////////////////////////////////////////////////////////////////
//

static const NameFunction AS2_VideoFunctionTable[] = 
{
    { "attachVideo", &VideoProto::AttachVideo },
    { "clear",       &VideoProto::Clear },
    { 0, 0 }
};

VideoProto::VideoProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor) : 
    Prototype<VideoObject>(psc, pprototype, constructor)
{
    InitFunctionMembers(psc, AS2_VideoFunctionTable);
}

//////////////////////////////////////////////////////////////////////////
//

VideoCtorFunction::VideoCtorFunction(ASStringContext *psc) : CFunctionObject(psc, GlobalCtor)
{
    SF_UNUSED(psc);
}

Object* VideoCtorFunction::CreateNewObject(Environment *penv) const
{ 
    Object* obj = SF_HEAP_NEW(penv->GetHeap()) VideoObject(penv);
    return obj;
}

void VideoCtorFunction::GlobalCtor(const FnCall& fn)
{
    VideoObject* nobj = 0;
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object::Object_Video
                   && !fn.ThisPtr->IsBuiltinPrototype())
        nobj = static_cast<VideoObject*>(fn.ThisPtr);
    else
        nobj = SF_HEAP_NEW(fn.Env->GetHeap()) VideoObject(fn.Env);

   fn.Result->SetAsObject(nobj);
}

FunctionRef VideoCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef ctor(*SF_HEAP_NEW(pgc->GetHeap()) VideoCtorFunction(&sc));
    Ptr<Object> pproto = *SF_HEAP_NEW(pgc->GetHeap()) VideoProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_Video, pproto);
    pgc->pGlobal->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_Video), Value(ctor));
    return ctor;
}

#ifdef GFX_AS_ENABLE_GC
void VideoObject::Finalize_GC()
{
    pVideo.~WeakPtr<InteractiveObject>();
    Object::Finalize_GC();
}
#endif // GFX_AS_ENABLE_GC

}}} // Scaleform::GFx::AS2

#endif // GFX_ENABLE_VIDEO
