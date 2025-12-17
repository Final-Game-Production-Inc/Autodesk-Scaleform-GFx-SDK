//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_ObjectEncoding.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Net_ObjectEncoding_H
#define INC_AS3_Obj_Net_ObjectEncoding_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_net
{
    extern const TypeInfo ObjectEncodingTI;
    extern const ClassInfo ObjectEncodingCI;
    extern const TypeInfo IDynamicPropertyWriterTI;
    extern const ClassInfo IDynamicPropertyWriterCI;
} // namespace fl_net

namespace ClassTraits { namespace fl_net
{
    class ObjectEncoding;
}}

namespace InstanceTraits { namespace fl_net
{
    class ObjectEncoding;
}}

namespace Classes { namespace fl_net
{
    class ObjectEncoding;
}}

//##protect##"forward_declaration"
    namespace Instances
    {
        class IDynamicPropertyWriter;
    }
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_net
{
    class ObjectEncoding : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::ObjectEncoding"; }
#endif
    public:
        typedef Classes::fl_net::ObjectEncoding ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        ObjectEncoding(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 3 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_net
{
    class ObjectEncoding : public Class
    {
        friend class ClassTraits::fl_net::ObjectEncoding;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_net::ObjectEncodingTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::ObjectEncoding"; }
#endif
    public:
        typedef ObjectEncoding SelfType;
        typedef ObjectEncoding ClassType;
        
    private:
        ObjectEncoding(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const UInt32 AMF0;
        const UInt32 AMF3;
        const UInt32 DEFAULT;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

