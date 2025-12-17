//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_SharedObjectFlushStatus.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Net_SharedObjectFlushStatus_H
#define INC_AS3_Obj_Net_SharedObjectFlushStatus_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_net
{
    extern const TypeInfo SharedObjectFlushStatusTI;
    extern const ClassInfo SharedObjectFlushStatusCI;
} // namespace fl_net

namespace ClassTraits { namespace fl_net
{
    class SharedObjectFlushStatus;
}}

namespace InstanceTraits { namespace fl_net
{
    class SharedObjectFlushStatus;
}}

namespace Classes { namespace fl_net
{
    class SharedObjectFlushStatus;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_net
{
    class SharedObjectFlushStatus : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::SharedObjectFlushStatus"; }
#endif
    public:
        typedef Classes::fl_net::SharedObjectFlushStatus ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        SharedObjectFlushStatus(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_net
{
    class SharedObjectFlushStatus : public Class
    {
        friend class ClassTraits::fl_net::SharedObjectFlushStatus;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_net::SharedObjectFlushStatusTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::SharedObjectFlushStatus"; }
#endif
    public:
        typedef SharedObjectFlushStatus SelfType;
        typedef SharedObjectFlushStatus ClassType;
        
    private:
        SharedObjectFlushStatus(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* FLUSHED;
        const char* PENDING;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

