//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Security_ReferencesValidationSetting.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Security_ReferencesValidationSetting_H
#define INC_AS3_Obj_Security_ReferencesValidationSetting_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_security
{
    extern const TypeInfo ReferencesValidationSettingTI;
    extern const ClassInfo ReferencesValidationSettingCI;
} // namespace fl_security

namespace ClassTraits { namespace fl_security
{
    class ReferencesValidationSetting;
}}

namespace InstanceTraits { namespace fl_security
{
    class ReferencesValidationSetting;
}}

namespace Classes { namespace fl_security
{
    class ReferencesValidationSetting;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_security
{
    class ReferencesValidationSetting : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::ReferencesValidationSetting"; }
#endif
    public:
        typedef Classes::fl_security::ReferencesValidationSetting ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        ReferencesValidationSetting(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 3 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_security
{
    class ReferencesValidationSetting : public Class
    {
        friend class ClassTraits::fl_security::ReferencesValidationSetting;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_security::ReferencesValidationSettingTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::ReferencesValidationSetting"; }
#endif
    public:
        typedef ReferencesValidationSetting SelfType;
        typedef ReferencesValidationSetting ClassType;
        
    private:
        ReferencesValidationSetting(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* NEVER;
        const char* VALID_IDENTITY;
        const char* VALID_OR_UNKNOWN_IDENTITY;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

