//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Desktop_NotificationType.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Desktop_NotificationType_H
#define INC_AS3_Obj_Desktop_NotificationType_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_desktop
{
    extern const TypeInfo NotificationTypeTI;
    extern const ClassInfo NotificationTypeCI;
} // namespace fl_desktop

namespace ClassTraits { namespace fl_desktop
{
    class NotificationType;
}}

namespace InstanceTraits { namespace fl_desktop
{
    class NotificationType;
}}

namespace Classes { namespace fl_desktop
{
    class NotificationType;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_desktop
{
    class NotificationType : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::NotificationType"; }
#endif
    public:
        typedef Classes::fl_desktop::NotificationType ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        NotificationType(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_desktop
{
    class NotificationType : public Class
    {
        friend class ClassTraits::fl_desktop::NotificationType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_desktop::NotificationTypeTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::NotificationType"; }
#endif
    public:
        typedef NotificationType SelfType;
        typedef NotificationType ClassType;
        
    private:
        NotificationType(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* CRITICAL;
        const char* INFORMATIONAL;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

