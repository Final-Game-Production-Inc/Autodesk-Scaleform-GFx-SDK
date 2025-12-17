/**************************************************************************

PublicHeader:   GFx
Filename    :   AS2_FunctionRefImpl.h
Content     :   ActionScript implementation classes
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#ifndef INC_SF_GFX_FUNCTIONREFIMPL_H
#define INC_SF_GFX_FUNCTIONREFIMPL_H

#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_Object.h"

namespace Scaleform { namespace GFx { namespace AS2 {

//
//////////// FunctionRefBase //////////////////
//
SF_INLINE void FunctionRefBase::Init(FunctionObject* funcObj, UByte flags)
{
    Flags = flags;
    Function = funcObj; 
    if (!(Flags & FuncRef_Weak) && Function)
        Function->AddRef();
    pLocalFrame = 0; 
}

SF_INLINE void FunctionRefBase::Init(FunctionObject& funcObj, UByte flags)
{
    Flags = flags;
    Function = &funcObj; 
    pLocalFrame = 0; 
}

SF_INLINE void FunctionRefBase::Init(const FunctionRefBase& orig, UByte flags)
{
    Flags = flags;
    Function = orig.Function; 
    if (!(Flags & FuncRef_Weak) && Function)
        Function->AddRef();
    pLocalFrame = 0; 
    if (orig.pLocalFrame != 0) 
        SetLocalFrame (orig.pLocalFrame, orig.Flags & FuncRef_Internal);
}

SF_INLINE void FunctionRefBase::DropRefs()
{
    if (!(Flags & FuncRef_Weak) && Function)
        Function->Release ();
    Function = 0;
    if (!(Flags & FuncRef_Internal) && pLocalFrame != 0)
        pLocalFrame->Release ();
    pLocalFrame = 0;

}

SF_INLINE void FunctionRefBase::Invoke(const FnCall& fn, const char* pmethodName) const
{
    SF_ASSERT (Function);
    Function->Invoke(fn, pLocalFrame, pmethodName);
}

SF_INLINE bool FunctionRefBase::operator== (const FunctionRefBase& f) const
{
    return Function == f.Function;
}

SF_INLINE bool FunctionRefBase::operator!= (const FunctionRefBase& f) const
{
    return Function != f.Function;
}

SF_INLINE bool FunctionRefBase::operator== (const CFunctionPtr f) const
{
    if (f == NULL)
        return Function == NULL;
    return IsCFunction() && static_cast<CFunctionObject*>(Function)->pFunction == f;
}

SF_INLINE bool FunctionRefBase::operator!= (const CFunctionPtr f) const
{
    if (f == NULL)
        return Function != NULL;
    return IsCFunction() && static_cast<CFunctionObject*>(Function)->pFunction == f;
}

SF_INLINE bool FunctionRefBase::IsCFunction () const
{
    SF_ASSERT (Function);
    return Function->IsCFunction ();
}

SF_INLINE bool FunctionRefBase::IsAsFunction () const
{
    SF_ASSERT (Function);
    return Function->IsAsFunction ();
}

}}} // SF::GFx::AS2

#endif // INC_SF_GFX_FUNCTIONREFIMPL_H

