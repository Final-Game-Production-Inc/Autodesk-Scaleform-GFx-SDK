//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Filters_BitmapFilterType.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Filters_BitmapFilterType_H
#define INC_AS3_Obj_Filters_BitmapFilterType_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_filters
{
    extern const TypeInfo BitmapFilterTypeTI;
    extern const ClassInfo BitmapFilterTypeCI;
} // namespace fl_filters

namespace ClassTraits { namespace fl_filters
{
    class BitmapFilterType;
}}

namespace InstanceTraits { namespace fl_filters
{
    class BitmapFilterType;
}}

namespace Classes { namespace fl_filters
{
    class BitmapFilterType;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_filters
{
    class BitmapFilterType : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::BitmapFilterType"; }
#endif
    public:
        typedef Classes::fl_filters::BitmapFilterType ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        BitmapFilterType(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 3 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_filters
{
    class BitmapFilterType : public Class
    {
        friend class ClassTraits::fl_filters::BitmapFilterType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_filters::BitmapFilterTypeTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::BitmapFilterType"; }
#endif
    public:
        typedef BitmapFilterType SelfType;
        typedef BitmapFilterType ClassType;
        
    private:
        BitmapFilterType(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* FULL;
        const char* INNER;
        const char* OUTER;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

