//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_URLRequestMethod.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Net_URLRequestMethod_H
#define INC_AS3_Obj_Net_URLRequestMethod_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_net
{
    extern const TypeInfo URLRequestMethodTI;
    extern const ClassInfo URLRequestMethodCI;
} // namespace fl_net

namespace ClassTraits { namespace fl_net
{
    class URLRequestMethod;
}}

namespace InstanceTraits { namespace fl_net
{
    class URLRequestMethod;
}}

namespace Classes { namespace fl_net
{
    class URLRequestMethod;
}}

//##protect##"forward_declaration"
#ifdef DELETE
#undef DELETE
#endif
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_net
{
    class URLRequestMethod : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::URLRequestMethod"; }
#endif
    public:
        typedef Classes::fl_net::URLRequestMethod ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        URLRequestMethod(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 6 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_net
{
    class URLRequestMethod : public Class
    {
        friend class ClassTraits::fl_net::URLRequestMethod;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_net::URLRequestMethodTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::URLRequestMethod"; }
#endif
    public:
        typedef URLRequestMethod SelfType;
        typedef URLRequestMethod ClassType;
        
    private:
        URLRequestMethod(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* DELETE;
        const char* GET;
        const char* HEAD;
        const char* OPTIONS;
        const char* POST;
        const char* PUT;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

