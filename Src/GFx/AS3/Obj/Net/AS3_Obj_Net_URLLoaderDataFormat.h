//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_URLLoaderDataFormat.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Net_URLLoaderDataFormat_H
#define INC_AS3_Obj_Net_URLLoaderDataFormat_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_net
{
    extern const TypeInfo URLLoaderDataFormatTI;
    extern const ClassInfo URLLoaderDataFormatCI;
} // namespace fl_net

namespace ClassTraits { namespace fl_net
{
    class URLLoaderDataFormat;
}}

namespace InstanceTraits { namespace fl_net
{
    class URLLoaderDataFormat;
}}

namespace Classes { namespace fl_net
{
    class URLLoaderDataFormat;
}}

//##protect##"forward_declaration"
#ifdef BINARY
#undef BINARY
#endif

#ifdef TEXT
#undef TEXT
#endif
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_net
{
    class URLLoaderDataFormat : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::URLLoaderDataFormat"; }
#endif
    public:
        typedef Classes::fl_net::URLLoaderDataFormat ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        URLLoaderDataFormat(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 3 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_net
{
    class URLLoaderDataFormat : public Class
    {
        friend class ClassTraits::fl_net::URLLoaderDataFormat;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_net::URLLoaderDataFormatTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::URLLoaderDataFormat"; }
#endif
    public:
        typedef URLLoaderDataFormat SelfType;
        typedef URLLoaderDataFormat ClassType;
        
    private:
        URLLoaderDataFormat(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* BINARY;
        const char* TEXT;
        const char* VARIABLES;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

