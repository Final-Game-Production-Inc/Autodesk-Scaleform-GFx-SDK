/**************************************************************************

Filename    :   AS3_Obj_ClassClass.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_AS3_Obj_ClassClass_H
#define INC_AS3_Obj_ClassClass_H

#include "../AS3_Instance.h"

namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl
{
    extern const TypeInfo ClassTI;
    extern const ClassInfo ClassCI;
} // namespace fl
    
namespace InstanceTraits
{
    // Dummy Traits for the ClassClass needed to create new objects of ClassClass.
    // ClassClass MAY NOT create objects ...
    class ClassClass : public CTraits
    {
    public:
        ClassClass(VM& vm, const ClassInfo& ci) : CTraits(vm, ci) {}

    public:
        // Class Class MAY NOT create objects ...
        virtual void MakeObject(Value& result, Traits& t);
    };
    
}

namespace Classes
{
    // ClassClass ...
    // The ClassClass itself is created using the ClassClass Traits ...
    class ClassClass : public Class
    {
        friend class AS3::VM; // Because of SetupPrototype();
        
    public:
        typedef ClassClass SelfType;
        
    public:
        ClassClass(ClassTraits::Traits& t);
        ~ClassClass();
        
    private:
        SelfType& GetSelf()
        {
            return *this;
        }
        
        void SetupPrototype();
    };
    
}

namespace ClassTraits
{
    // This is the Traits to create ClassClass ...
    // Special case ...
    class ClassClass : public Traits
    {
        typedef ClassClass Self;

    public:
        ClassClass(VM& vm);

    public:
        Self& GetSelf()
        {
            return *this;
        }

        virtual bool Coerce(const Value& value, Value& result) const;

        // We are not going to use this one ...
        virtual void MakeObject(Value& result, AS3::Traits* t = NULL);
        void SetParentTraits(const Traits* pt)
        {
            Traits::SetParentTraits(pt);
        }

        static ClassInfo Info;
    };
}

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif // INC_AS3_Obj_ClassClass_H
