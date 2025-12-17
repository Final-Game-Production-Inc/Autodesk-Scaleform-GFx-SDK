//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_URLVariables.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Net_URLVariables_H
#define INC_AS3_Obj_Net_URLVariables_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_net
{
    extern const TypeInfo URLVariablesTI;
    extern const ClassInfo URLVariablesCI;
} // namespace fl_net
namespace fl
{
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_net
{
    class URLVariables;
}}

namespace InstanceTraits { namespace fl_net
{
    class URLVariables;
}}

namespace Classes { namespace fl_net
{
    class URLVariables;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_net
{
    class URLVariables : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_net::URLVariables;
        
    public:
        typedef URLVariables SelfType;
        typedef Classes::fl_net::URLVariables ClassType;
        typedef InstanceTraits::fl_net::URLVariables TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_net::URLVariablesTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_net::URLVariables"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        URLVariables(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void AS3Constructor(unsigned argc, const Value* argv);
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_decode, 
            mid_toString, 
        };
        void decode(const Value& result, const ASString& source);
        void toString(ASString& result);

        // C++ friendly wrappers for AS3 methods.
        void decode(const ASString& source)
        {
            decode(Value::GetUndefined(), source);
        }
        ASString toString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            toString(result);
            return result;
        }

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_net
{
    class URLVariables : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::URLVariables"; }
#endif
    public:
        typedef Instances::fl_net::URLVariables InstanceType;

    public:
        URLVariables(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_net::URLVariables> MakeInstance(URLVariables& t)
        {
            return Pickable<Instances::fl_net::URLVariables>(new(t.Alloc()) Instances::fl_net::URLVariables(t));
        }
        SPtr<Instances::fl_net::URLVariables> MakeInstanceS(URLVariables& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 2 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[3];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_net
{
    class URLVariables : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::URLVariables"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_net::URLVariables InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        URLVariables(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

