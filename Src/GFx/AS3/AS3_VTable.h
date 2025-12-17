/**************************************************************************

Filename    :   AS3_VTable.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_AS3_VTable_H
#define INC_AS3_VTable_H

#include "AS3_Value.h"
#include "AS3_Slot.h"
#include "AS3_Traits.h"


namespace Scaleform { namespace GFx { namespace AS3
{

///////////////////////////////////////////////////////////////////////////
class VTable : public NewOverrideBase<StatMV_VM_VTable_Mem>
{
    friend class Traits;
    friend class InstanceTraits::Traits;

    typedef VTable SelfType;
    
public:
    VTable(Traits& tr);
    VTable(Traits& tr, const VTable& parent);
    
public:
    void GetMethod(Value& value, AbsoluteIndex ind, Object& _this, bool super) const;
    // Return a raw value. It is not correct to use this method to create 
    // callable objects. This method should be used only for displaying of internal information.
    const Value& GetRaw(AbsoluteIndex ind) const
    {
        return VTMethods[ind.Get()];
    }
    // Similar to GetRaw(), but it will convert MethodInd to VTableInd.
    Value GetValue(AbsoluteIndex ind) const;
	SF_DEBUG_CODE(const ASString& GetName(AbsoluteIndex ind) const { return Names[ind.Get()]; } )
    Traits& GetTraits() const
    {
        SF_ASSERT(pTraits);
        return *pTraits;
    }
    VMAbcFile* GetFilePtr() const
    {
        return GetTraits().GetFilePtr();
    }

public:
    typedef RefCountCollector<Mem_Stat> Collector;
    void ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op) const;

private:
    void SetMethod(
        AbsoluteIndex ind, 
        const Value& m,
        SlotInfo::BindingType dt
        SF_DEBUG_ARG(const ASString& name)
        );
    AbsoluteIndex AddMethod(
        const Value& m, 
        SlotInfo::BindingType dt
        SF_DEBUG_ARG(const ASString& name)
        );
	SF_DEBUG_CODE(void SetMethodName(AbsoluteIndex ind, SlotInfo::BindingType dt, const ASString& name);)

//     void MakeScopeStackCPP(const Traits& tr);

    SelfType& GetSelf() { return *this; }

private:
    typedef ArrayCC<ASString, StatMV_VM_VTable_Mem> NamesT;
    typedef ArrayLH<Value, StatMV_VM_VTable_Mem>    MethodsT;

    Traits*         pTraits;
    MethodsT        VTMethods;
	SF_DEBUG_CODE(NamesT Names;)
};

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

#endif // INC_AS3_VTable_H

