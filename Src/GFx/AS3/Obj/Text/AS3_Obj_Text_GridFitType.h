//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Text_GridFitType.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Text_GridFitType_H
#define INC_AS3_Obj_Text_GridFitType_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_text
{
    extern const TypeInfo GridFitTypeTI;
    extern const ClassInfo GridFitTypeCI;
} // namespace fl_text

namespace ClassTraits { namespace fl_text
{
    class GridFitType;
}}

namespace InstanceTraits { namespace fl_text
{
    class GridFitType;
}}

namespace Classes { namespace fl_text
{
    class GridFitType;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_text
{
    class GridFitType : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::GridFitType"; }
#endif
    public:
        typedef Classes::fl_text::GridFitType ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        GridFitType(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 3 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_text
{
    class GridFitType : public Class
    {
        friend class ClassTraits::fl_text::GridFitType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_text::GridFitTypeTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::GridFitType"; }
#endif
    public:
        typedef GridFitType SelfType;
        typedef GridFitType ClassType;
        
    private:
        GridFitType(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* NONE;
        const char* PIXEL;
        const char* SUBPIXEL;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

