//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Filters_DisplacementMapFilterMode.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Filters_DisplacementMapFilterMode_H
#define INC_AS3_Obj_Filters_DisplacementMapFilterMode_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_filters
{
    extern const TypeInfo DisplacementMapFilterModeTI;
    extern const ClassInfo DisplacementMapFilterModeCI;
} // namespace fl_filters

namespace ClassTraits { namespace fl_filters
{
    class DisplacementMapFilterMode;
}}

namespace InstanceTraits { namespace fl_filters
{
    class DisplacementMapFilterMode;
}}

namespace Classes { namespace fl_filters
{
    class DisplacementMapFilterMode;
}}

//##protect##"forward_declaration"
#ifdef IGNORE
#undef IGNORE
#endif
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_filters
{
    class DisplacementMapFilterMode : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::DisplacementMapFilterMode"; }
#endif
    public:
        typedef Classes::fl_filters::DisplacementMapFilterMode ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        DisplacementMapFilterMode(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 4 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_filters
{
    class DisplacementMapFilterMode : public Class
    {
        friend class ClassTraits::fl_filters::DisplacementMapFilterMode;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_filters::DisplacementMapFilterModeTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::DisplacementMapFilterMode"; }
#endif
    public:
        typedef DisplacementMapFilterMode SelfType;
        typedef DisplacementMapFilterMode ClassType;
        
    private:
        DisplacementMapFilterMode(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* CLAMP;
        const char* COLOR;
        const char* IGNORE;
        const char* WRAP;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

