/**************************************************************************

Filename    :   AS3_VideoCharacter.h
Content     :   GFx video: AS3 AvmVideoCharacter class
Created     :   March, 2011
Authors     :   Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_VIDEO_AS3VIDEOCHAR_H
#define INC_GFX_VIDEO_AS3VIDEOCHAR_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "GFx/AS3/AS3_AvmInteractiveObj.h"
#include "Video/Video_VideoCharacter.h"

namespace Scaleform { namespace GFx { namespace AS3 {

//////////////////////////////////////////////////////////////////////////
//

class AvmVideoCharacter : public AvmInteractiveObj, public AvmVideoCharacterBase
{
public:
    AvmVideoCharacter(Video::VideoCharacter* pchar);

    virtual AvmVideoCharacterBase* ToAvmVideoCharacterBase() { return this; }
    virtual AvmInteractiveObjBase* ToAvmInteractiveObjBase() 
                                                   { return static_cast<AvmInteractiveObj*>(this); }
    virtual const char* GetDefaultASClassName() const { return "flash.media.Video"; }

    // GFx::AvmDisplayObjBase interface
    virtual AvmSpriteBase*    ToAvmSpriteBase()    { return NULL; }
    virtual AvmButtonBase*    ToAvmButttonBase()   { return NULL; }
    virtual AvmTextFieldBase* ToAvmTextFieldBase() { return NULL; }
    virtual AvmDisplayObjContainerBase* ToAvmDispContainerBase() { return NULL; }

    virtual bool OnEvent(const EventId& id) { return AvmInteractiveObj::OnEvent(id); }
    virtual void OnEventLoad()              { AvmInteractiveObj::OnEventLoad(); }
    virtual void OnEventUnload()            { AvmInteractiveObj::OnEventUnload();}
    virtual bool OnUnloading(bool)          { return true; }
    virtual bool HasEventHandler(const EventId& id) const 
                                            { return AvmDisplayObj::HasEventHandler(id); }

    virtual bool ActsAsButton() const       { return true; }
    virtual const char* GetAbsolutePath(String *ppath) const
                                            { return AvmInteractiveObj::GetAbsolutePath(ppath); }
    virtual unsigned GetCursorType() const  { return AvmInteractiveObj::GetCursorType(); }
    virtual void CloneInternalData(const InteractiveObject* src)
                                            { AvmInteractiveObj::CloneInternalData(src); }
    virtual void CopyPhysicalProperties(const InteractiveObject *poldChar)
                                            { AvmInteractiveObj::CopyPhysicalProperties(poldChar); }
    virtual void OnFocus(InteractiveObject::FocusEventType event, InteractiveObject* oldOrNewFocusCh,
                         unsigned controllerIdx, FocusMovedType fmt)
                                            { AvmInteractiveObj::OnFocus(event, oldOrNewFocusCh, controllerIdx, fmt); }

    virtual void OnGettingKeyboardFocus(unsigned, FocusMovedType) {}
    virtual bool OnKeyEvent(const EventId&, int*) { return false; }
    virtual bool OnLosingKeyboardFocus(InteractiveObject*, unsigned , FocusMovedType = GFx_FocusMovedByKeyboard)
                                            { return true; }

    virtual bool IsFocusEnabled(FocusMovedType = GFx_FocusMovedByKeyboard) const     { return true; }
    virtual bool IsTabable() const          { return AvmInteractiveObj::IsTabable(); }
};

}}} // Scaleform::GFx:AS3

#endif // GFX_ENABLE_VIDEO

#endif // INC_GFX_VIDEO_AS3VIDEOCHAR_H
