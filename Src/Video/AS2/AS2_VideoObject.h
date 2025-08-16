/**************************************************************************

Filename    :   AS2_VideoObject.h
Content     :   GFx video: AS2 VideoObject and AvmVideoCharacter classes
Created     :   May, 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_VIDEO_AS2VIDEOOBJECT_H
#define INC_GFX_VIDEO_AS2VIDEOOBJECT_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_ObjectProto.h"
#include "GFx/AS2/AS2_AvmCharacter.h"

#include "Video/Video_VideoCharacter.h"

namespace Scaleform { namespace GFx { namespace AS2 {

class VideoObject;
class VideoProto;
class VideoCtorFunction;

class AvmVideoCharacter;

//////////////////////////////////////////////////////////////////////////
//

class VideoObject : public Object
{
    friend class VideoProto;
    friend class VideoCtorFunction;

    WeakPtr<InteractiveObject> pVideo;

    void commonInit() {}

#ifdef GFX_AS_ENABLE_GC
protected:
    virtual void Finalize_GC();
#endif // GFX_AS_ENABLE_GC

protected:    
    VideoObject(ASStringContext *psc, Object* pprototype) : Object(psc)
    { 
        Set__proto__(psc, pprototype);
        commonInit();
    }

public:
    VideoObject(GlobalContext* gCtxt, InteractiveObject* pvideo) : Object(gCtxt->GetGC()), pVideo(pvideo)
    {
        commonInit();
        ASStringContext* psc = AS2::ToAvmCharacter(pvideo)->GetASEnvironment()->GetSC();
        Set__proto__(psc, gCtxt->GetPrototype(ASBuiltin_Video));
    }
    VideoObject(Environment *penv) : Object(penv)
    { 
        commonInit();
        Set__proto__(penv->GetSC(), penv->GetPrototype(ASBuiltin_Video));
    }

    ObjectType GetObjectType() const    { return Object_VideoASObject; }
    virtual InteractiveObject* GetASCharacter() { return Ptr<InteractiveObject>(pVideo).GetPtr(); }
    virtual const InteractiveObject* GetASCharacter() const { return Ptr<InteractiveObject>(pVideo).GetPtr(); }
};

//////////////////////////////////////////////////////////////////////////
//

class VideoProto : public Prototype<VideoObject>
{
public:
    VideoProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor);

    static void AttachVideo(const FnCall& fn);
    static void Clear(const FnCall& fn);
};

//////////////////////////////////////////////////////////////////////////
//

class VideoCtorFunction : public CFunctionObject
{
public:
    VideoCtorFunction(ASStringContext *psc);

    virtual Object* CreateNewObject(Environment *penv) const;

    static void GlobalCtor(const FnCall& fn);
    static FunctionRef Register(GlobalContext* pgc);
};

//////////////////////////////////////////////////////////////////////////
//

class AvmVideoCharacter : public AvmCharacter, public AvmVideoCharacterBase
{
public:
    AvmVideoCharacter(Video::VideoCharacter* pchar);

    bool   SetMember(Environment* penv, const ASString& name, const Value& val, const PropFlags& flags);
    bool   GetMember(Environment* penv, const ASString& name, Value* val);
    UInt32 GetStandardMemberBitMask() const;

    virtual Object*    GetASObject()         { return GetVideoASObject(); }
    virtual Object*    GetASObject() const   { return const_cast<AvmVideoCharacter*>(this)->GetVideoASObject(); }
    VideoObject*       GetVideoASObject();
    virtual ObjectType GetObjectType() const { return Object_Video; }

    virtual AvmVideoCharacterBase* ToAvmVideoCharacterBase() { return this; }
    virtual AvmInteractiveObjBase* ToAvmInteractiveObjBase()
                                             { return static_cast<AvmCharacter*>(this); }
    // GFx::AvmDisplayObjBase interface
    virtual AvmSpriteBase*    ToAvmSpriteBase()    { return NULL; }
    virtual AvmButtonBase*    ToAvmButttonBase()   { return NULL; }
    virtual AvmTextFieldBase* ToAvmTextFieldBase() { return NULL; }
    virtual AvmDisplayObjContainerBase* ToAvmDispContainerBase() { return NULL; }

    virtual const char* GetAbsolutePath(String *ppath) const
                                             { return AvmCharacter::GetAbsolutePath(ppath); }

    virtual bool OnEvent(const EventId& id)  { return AvmCharacter::OnEvent(id); }
    virtual void OnEventLoad()               { AvmCharacter::OnEventLoad(); }
    virtual void OnEventUnload()             { AvmCharacter::OnEventUnload(); }
    virtual bool OnUnloading(bool)           { pDispObj->RemoveFromPlayList(); return true; }

    virtual bool HasEventHandler(const EventId&) const { return false; }

    virtual unsigned GetCursorType() const   { return AvmCharacter::GetCursorType(); }
    virtual bool ActsAsButton() const        { return false; }

    virtual void CloneInternalData(const InteractiveObject* src)           { AvmCharacter::CloneInternalData(src); }
    virtual void CopyPhysicalProperties(const InteractiveObject *poldChar) { AvmCharacter::CopyPhysicalProperties(poldChar); }
    virtual void OnFocus(InteractiveObject::FocusEventType event, InteractiveObject* oldOrNewFocusCh, 
                         unsigned controllerIdx, FocusMovedType fmt)
                                             { AvmCharacter::OnFocus(event, oldOrNewFocusCh, controllerIdx, fmt); }

    virtual void OnGettingKeyboardFocus(unsigned, FocusMovedType) {}
    virtual bool OnKeyEvent(const EventId&, int*)                 { return false; }
    virtual bool OnLosingKeyboardFocus(InteractiveObject*, unsigned , FocusMovedType = GFx_FocusMovedByKeyboard)
                                                                  { return true; }
    virtual bool IsFocusEnabled(FocusMovedType = GFx_FocusMovedByKeyboard) const      { return true; }
    virtual bool IsTabable() const           { return AvmCharacter::IsTabable(); }

protected:
    Ptr<VideoObject> ASVideoObj;
};

}}} // Scaleform::GFx::AS2

#endif // GFX_ENABLE_VIDEO

#endif // INC_GFX_VIDEO_AS2VIDEOOBJECT_H
