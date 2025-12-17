/**************************************************************************

Filename    :   AS3_Obj_Activation.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_AS3_Obj_Activation_H
#define INC_AS3_Obj_Activation_H

#include "AS3_Obj_Object.h"

namespace Scaleform { namespace GFx { namespace AS3 
{
    
namespace InstanceTraits
{
    // It looks like this class is similar to InstanceTraits::Function     
    class Activation : public RTraits
    {
        friend class AS3::VM; // Because of Activation(VM& vm, Class& c)
        
    public:
        Activation(VMAbcFile& file, VM& vm, const Abc::MethodBodyInfo& mbi SF_DEBUG_ARG(const ASString& name));

    public:
        virtual void MakeObject(Value& result, Traits& t);
        virtual unsigned GetFixedMemSize() const;
        
    private:
        // Dummy constructor, should never be called.
        Activation(VM& vm);
    };
}

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif // INC_AS3_Obj_Activation_H
