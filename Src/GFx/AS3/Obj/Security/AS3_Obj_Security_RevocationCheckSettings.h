//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Security_RevocationCheckSettings.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Security_RevocationCheckSettings_H
#define INC_AS3_Obj_Security_RevocationCheckSettings_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_security
{
    extern const TypeInfo RevocationCheckSettingsTI;
    extern const ClassInfo RevocationCheckSettingsCI;
} // namespace fl_security

namespace ClassTraits { namespace fl_security
{
    class RevocationCheckSettings;
}}

namespace InstanceTraits { namespace fl_security
{
    class RevocationCheckSettings;
}}

namespace Classes { namespace fl_security
{
    class RevocationCheckSettings;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_security
{
    class RevocationCheckSettings : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::RevocationCheckSettings"; }
#endif
    public:
        typedef Classes::fl_security::RevocationCheckSettings ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        RevocationCheckSettings(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 4 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_security
{
    class RevocationCheckSettings : public Class
    {
        friend class ClassTraits::fl_security::RevocationCheckSettings;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_security::RevocationCheckSettingsTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::RevocationCheckSettings"; }
#endif
    public:
        typedef RevocationCheckSettings SelfType;
        typedef RevocationCheckSettings ClassType;
        
    private:
        RevocationCheckSettings(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        Value ALWAYS_REQUIRED;
        Value BEST_EFFORT;
        Value NEVER;
        Value REQUIRED_IF_AVAILABLE;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

