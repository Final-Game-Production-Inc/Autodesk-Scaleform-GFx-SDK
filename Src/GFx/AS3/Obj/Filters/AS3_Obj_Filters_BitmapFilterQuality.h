//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Filters_BitmapFilterQuality.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Filters_BitmapFilterQuality_H
#define INC_AS3_Obj_Filters_BitmapFilterQuality_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_filters
{
    extern const TypeInfo BitmapFilterQualityTI;
    extern const ClassInfo BitmapFilterQualityCI;
} // namespace fl_filters

namespace ClassTraits { namespace fl_filters
{
    class BitmapFilterQuality;
}}

namespace InstanceTraits { namespace fl_filters
{
    class BitmapFilterQuality;
}}

namespace Classes { namespace fl_filters
{
    class BitmapFilterQuality;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_filters
{
    class BitmapFilterQuality : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::BitmapFilterQuality"; }
#endif
    public:
        typedef Classes::fl_filters::BitmapFilterQuality ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        BitmapFilterQuality(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 3 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_filters
{
    class BitmapFilterQuality : public Class
    {
        friend class ClassTraits::fl_filters::BitmapFilterQuality;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_filters::BitmapFilterQualityTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::BitmapFilterQuality"; }
#endif
    public:
        typedef BitmapFilterQuality SelfType;
        typedef BitmapFilterQuality ClassType;
        
    private:
        BitmapFilterQuality(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const SInt32 HIGH;
        const SInt32 LOW;
        const SInt32 MEDIUM;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

