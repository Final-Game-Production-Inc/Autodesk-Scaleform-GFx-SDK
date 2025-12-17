//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Vec_Vector.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Vec_Vector_H
#define INC_AS3_Obj_Vec_Vector_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_vec
{
    extern const TypeInfo VectorTI;
    extern const ClassInfo VectorCI;
} // namespace fl_vec

namespace ClassTraits { namespace fl_vec
{
    class Vector;
}}

namespace InstanceTraits { namespace fl_vec
{
    class Vector;
}}

namespace Classes { namespace fl_vec
{
    class Vector;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_vec
{
    class Vector : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Vector"; }
#endif
    public:
        typedef Classes::fl_vec::Vector ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Vector(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_vec
{
    class Vector : public Class
    {
        friend class ClassTraits::fl_vec::Vector;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_vec::VectorTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Vector"; }
#endif
    public:
        typedef Vector SelfType;
        typedef Vector ClassType;
        
    private:
        Vector(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
    public:
        const ClassTraits::Traits& Resolve2Vector(const ClassTraits::Traits& elem) const;
        virtual const ClassTraits::Traits& ApplyTypeArgs(unsigned argc, const Value* argv);
//##protect##"class_$methods"

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

