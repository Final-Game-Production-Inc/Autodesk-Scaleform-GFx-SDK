//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Utils_Proxy.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Utils_Proxy_H
#define INC_AS3_Obj_Utils_Proxy_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_utils
{
    extern const TypeInfo ProxyTI;
    extern const ClassInfo ProxyCI;
} // namespace fl_utils
namespace fl
{
    extern const TypeInfo anyTI;
    extern const ClassInfo anyCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
} // namespace fl

namespace ClassTraits { namespace fl_utils
{
    class Proxy;
}}

namespace InstanceTraits { namespace fl_utils
{
    class Proxy;
}}

namespace Classes { namespace fl_utils
{
    class Proxy;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_utils
{
    class Proxy : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_utils::Proxy;
        
    public:
        typedef Proxy SelfType;
        typedef Classes::fl_utils::Proxy ClassType;
        typedef InstanceTraits::fl_utils::Proxy TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_utils::ProxyTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_utils::Proxy"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Proxy(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        CheckResult MakeName(const Multiname& prop_name, Value& name) const;
        SInt32 GetMethodInd(const char* name) const;

        virtual CheckResult SetProperty(const Multiname& prop_name, const Value& value);
        virtual CheckResult GetProperty(const Multiname& prop_name, Value& value);
        virtual CheckResult DeleteProperty(const Multiname& prop_name);
        virtual CheckResult ExecutePropertyUnsafe(const Multiname& prop_name, Value& result, unsigned argc, const Value* argv);
        virtual bool HasProperty(const Multiname& prop_name, bool check_prototype);

        virtual void GetNextPropertyName(Value& name, GlobalSlotIndex ind) const;
        virtual GlobalSlotIndex GetNextDynPropIndex(GlobalSlotIndex ind) const;
        virtual void GetNextPropertyValue(Value& value, GlobalSlotIndex ind);

        virtual void GetDescendants(fl::XMLList& list, const Multiname& prop_name);
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_flash_proxycallProperty, 
            mid_flash_proxydeleteProperty, 
            mid_flash_proxygetDescendants, 
            mid_flash_proxygetProperty, 
            mid_flash_proxyhasProperty, 
            mid_flash_proxyisAttribute, 
            mid_flash_proxynextName, 
            mid_flash_proxynextNameIndex, 
            mid_flash_proxynextValue, 
            mid_flash_proxysetProperty, 
        };
        void flash_proxycallProperty(Value& result, unsigned argc, const Value* const argv);
        void flash_proxydeleteProperty(bool& result, const Value& name);
        void flash_proxygetDescendants(Value& result, const Value& name);
        void flash_proxygetProperty(Value& result, const Value& name);
        void flash_proxyhasProperty(bool& result, const Value& name);
        void flash_proxyisAttribute(bool& result, const Value& name);
        void flash_proxynextName(ASString& result, SInt32 index);
        void flash_proxynextNameIndex(SInt32& result, SInt32 index);
        void flash_proxynextValue(Value& result, SInt32 index);
        void flash_proxysetProperty(const Value& result, const Value& name, const Value& value);

        // C++ friendly wrappers for AS3 methods.
        bool flash_proxydeleteProperty(const Value& name)
        {
            bool result;
            flash_proxydeleteProperty(result, name);
            return result;
        }
        Value flash_proxygetDescendants(const Value& name)
        {
            Value result;
            flash_proxygetDescendants(result, name);
            return result;
        }
        Value flash_proxygetProperty(const Value& name)
        {
            Value result;
            flash_proxygetProperty(result, name);
            return result;
        }
        bool flash_proxyhasProperty(const Value& name)
        {
            bool result;
            flash_proxyhasProperty(result, name);
            return result;
        }
        bool flash_proxyisAttribute(const Value& name)
        {
            bool result;
            flash_proxyisAttribute(result, name);
            return result;
        }
        ASString flash_proxynextName(SInt32 index)
        {
            ASString result(GetStringManager().CreateEmptyString());
            flash_proxynextName(result, index);
            return result;
        }
        SInt32 flash_proxynextNameIndex(SInt32 index)
        {
            SInt32 result;
            flash_proxynextNameIndex(result, index);
            return result;
        }
        Value flash_proxynextValue(SInt32 index)
        {
            Value result;
            flash_proxynextValue(result, index);
            return result;
        }
        void flash_proxysetProperty(const Value& name, const Value& value)
        {
            flash_proxysetProperty(Value::GetUndefined(), name, value);
        }

//##protect##"instance$data"
        mutable SInt32  callPropertyInd;
        mutable SInt32  deletePropertyInd;
        mutable SInt32  getDescendantsInd;
        mutable SInt32  getPropertyInd;
        mutable SInt32  hasPropertyInd;
        mutable SInt32  isAttributeInd;
        mutable SInt32  nextNameInd;
        mutable SInt32  nextNameIndexInd;
        mutable SInt32  nextValueInd;
        mutable SInt32  setPropertyInd;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_utils
{
    class Proxy : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Proxy"; }
#endif
    public:
        typedef Instances::fl_utils::Proxy InstanceType;

    public:
        Proxy(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_utils::Proxy> MakeInstance(Proxy& t)
        {
            return Pickable<Instances::fl_utils::Proxy>(new(t.Alloc()) Instances::fl_utils::Proxy(t));
        }
        SPtr<Instances::fl_utils::Proxy> MakeInstanceS(Proxy& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 10 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[21];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_utils
{
    class Proxy : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Proxy"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_utils::Proxy InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Proxy(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

