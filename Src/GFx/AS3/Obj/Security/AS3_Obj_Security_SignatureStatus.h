//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Security_SignatureStatus.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Security_SignatureStatus_H
#define INC_AS3_Obj_Security_SignatureStatus_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_security
{
    extern const TypeInfo SignatureStatusTI;
    extern const ClassInfo SignatureStatusCI;
} // namespace fl_security

namespace ClassTraits { namespace fl_security
{
    class SignatureStatus;
}}

namespace InstanceTraits { namespace fl_security
{
    class SignatureStatus;
}}

namespace Classes { namespace fl_security
{
    class SignatureStatus;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_security
{
    class SignatureStatus : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::SignatureStatus"; }
#endif
    public:
        typedef Classes::fl_security::SignatureStatus ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        SignatureStatus(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 3 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_security
{
    class SignatureStatus : public Class
    {
        friend class ClassTraits::fl_security::SignatureStatus;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_security::SignatureStatusTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::SignatureStatus"; }
#endif
    public:
        typedef SignatureStatus SelfType;
        typedef SignatureStatus ClassType;
        
    private:
        SignatureStatus(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* INVALID;
        const char* UNKNOWN;
        const char* VALID;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

