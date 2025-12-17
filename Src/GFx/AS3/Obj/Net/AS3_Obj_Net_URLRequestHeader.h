//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_URLRequestHeader.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Net_URLRequestHeader_H
#define INC_AS3_Obj_Net_URLRequestHeader_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_net
{
    extern const TypeInfo URLRequestHeaderTI;
    extern const ClassInfo URLRequestHeaderCI;
} // namespace fl_net

namespace ClassTraits { namespace fl_net
{
    class URLRequestHeader;
}}

namespace InstanceTraits { namespace fl_net
{
    class URLRequestHeader;
}}

namespace Classes { namespace fl_net
{
    class URLRequestHeader;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_net
{
    class URLRequestHeader : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_net::URLRequestHeader;
        
    public:
        typedef URLRequestHeader SelfType;
        typedef Classes::fl_net::URLRequestHeader ClassType;
        typedef InstanceTraits::fl_net::URLRequestHeader TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_net::URLRequestHeaderTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_net::URLRequestHeader"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        URLRequestHeader(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void    AS3Constructor(unsigned argc, const Value* argv);
//##protect##"instance$methods"

    public:
        // AS3 API members.
        ASString name;
        ASString value;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_net
{
    class URLRequestHeader : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::URLRequestHeader"; }
#endif
    public:
        typedef Instances::fl_net::URLRequestHeader InstanceType;

    public:
        URLRequestHeader(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_net::URLRequestHeader> MakeInstance(URLRequestHeader& t)
        {
            return Pickable<Instances::fl_net::URLRequestHeader>(new(t.Alloc()) Instances::fl_net::URLRequestHeader(t));
        }
        SPtr<Instances::fl_net::URLRequestHeader> MakeInstanceS(URLRequestHeader& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_net
{
    class URLRequestHeader : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::URLRequestHeader"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_net::URLRequestHeader InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        URLRequestHeader(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

