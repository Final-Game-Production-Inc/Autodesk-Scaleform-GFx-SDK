//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Printing_PrintJobOrientation.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Printing_PrintJobOrientation_H
#define INC_AS3_Obj_Printing_PrintJobOrientation_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_printing
{
    extern const TypeInfo PrintJobOrientationTI;
    extern const ClassInfo PrintJobOrientationCI;
} // namespace fl_printing

namespace ClassTraits { namespace fl_printing
{
    class PrintJobOrientation;
}}

namespace InstanceTraits { namespace fl_printing
{
    class PrintJobOrientation;
}}

namespace Classes { namespace fl_printing
{
    class PrintJobOrientation;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_printing
{
    class PrintJobOrientation : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::PrintJobOrientation"; }
#endif
    public:
        typedef Classes::fl_printing::PrintJobOrientation ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        PrintJobOrientation(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_printing
{
    class PrintJobOrientation : public Class
    {
        friend class ClassTraits::fl_printing::PrintJobOrientation;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_printing::PrintJobOrientationTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::PrintJobOrientation"; }
#endif
    public:
        typedef PrintJobOrientation SelfType;
        typedef PrintJobOrientation ClassType;
        
    private:
        PrintJobOrientation(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* LANDSCAPE;
        const char* PORTRAIT;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

