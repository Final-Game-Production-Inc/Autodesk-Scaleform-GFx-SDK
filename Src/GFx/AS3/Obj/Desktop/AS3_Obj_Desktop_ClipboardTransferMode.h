//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Desktop_ClipboardTransferMode.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Desktop_ClipboardTransferMode_H
#define INC_AS3_Obj_Desktop_ClipboardTransferMode_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_desktop
{
    extern const TypeInfo ClipboardTransferModeTI;
    extern const ClassInfo ClipboardTransferModeCI;
} // namespace fl_desktop

namespace ClassTraits { namespace fl_desktop
{
    class ClipboardTransferMode;
}}

namespace InstanceTraits { namespace fl_desktop
{
    class ClipboardTransferMode;
}}

namespace Classes { namespace fl_desktop
{
    class ClipboardTransferMode;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_desktop
{
    class ClipboardTransferMode : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::ClipboardTransferMode"; }
#endif
    public:
        typedef Classes::fl_desktop::ClipboardTransferMode ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        ClipboardTransferMode(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 4 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_desktop
{
    class ClipboardTransferMode : public Class
    {
        friend class ClassTraits::fl_desktop::ClipboardTransferMode;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_desktop::ClipboardTransferModeTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::ClipboardTransferMode"; }
#endif
    public:
        typedef ClipboardTransferMode SelfType;
        typedef ClipboardTransferMode ClassType;
        
    private:
        ClipboardTransferMode(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* CLONE_ONLY;
        const char* CLONE_PREFERRED;
        const char* ORIGINAL_ONLY;
        const char* ORIGINAL_PREFERRED;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

