//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Utils_Dictionary.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Utils_Dictionary_H
#define INC_AS3_Obj_Utils_Dictionary_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_utils
{
    extern const TypeInfo DictionaryTI;
    extern const ClassInfo DictionaryCI;
} // namespace fl_utils

namespace ClassTraits { namespace fl_utils
{
    class Dictionary;
}}

namespace InstanceTraits { namespace fl_utils
{
    class Dictionary;
}}

namespace Classes { namespace fl_utils
{
    class Dictionary;
}}

//##protect##"forward_declaration"

///////////////////////////////////////////////////////////////////////////
typedef HashLH<Value, Value, Value::HashFunctor> ValueContainerType;
void ForEachChild_GC(RefCountCollector<Mem_Stat>* prcc, bool WeakKeys, ValueContainerType& v, RefCountBaseGC<Mem_Stat>::GcOp op
                     SF_DEBUG_ARG(const RefCountBaseGC<Mem_Stat>& owner));

//##protect##"forward_declaration"

namespace Instances { namespace fl_utils
{
    class Dictionary : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_utils::Dictionary;
        
    public:
        typedef Dictionary SelfType;
        typedef Classes::fl_utils::Dictionary ClassType;
        typedef InstanceTraits::fl_utils::Dictionary TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_utils::DictionaryTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_utils::Dictionary"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Dictionary(InstanceTraits::Traits& t);

//##protect##"instance$methods"
    public:
        Dictionary(InstanceTraits::Traits& t, bool weakKeys);
        virtual ~Dictionary();

        virtual CheckResult SetProperty(const Multiname& prop_name, const Value& value);
        virtual CheckResult GetProperty(const Multiname& prop_name, Value& value);
        virtual void GetDynamicProperty(AbsoluteIndex ind, Value& value);
        virtual CheckResult DeleteProperty(const Multiname& prop_name);
        virtual void AddDynamicSlotValuePair(const Value& prop_name, const Value& value,
            SlotInfo::Attribute a = SlotInfo::aNone);
        virtual PropRef FindDynamicSlot(const Multiname& prop_name);

        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;

    public:
        virtual void GetNextPropertyName(Value& name, GlobalSlotIndex ind) const;
        // This method will enumerate only dynamic properties.
        // Initial index should be invalid (for example -1)
        virtual GlobalSlotIndex GetNextDynPropIndex(GlobalSlotIndex ind) const;

    public:
        UPInt GetSize() const { return ValueH.GetSize(); }
        bool IsWeakKeys() const { return WeakKeys; }

        const ValueContainerType& GetContainer() const { return ValueH; }

    protected:
        virtual void AS3Constructor(unsigned argc, const Value* argv);

        ValueContainerType::Iterator FindKey(const Multiname& prop_name) const;

//##protect##"instance$methods"

//##protect##"instance$data"
    private:
        bool WeakKeys;
        mutable ValueContainerType ValueH;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_utils
{
    class Dictionary : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Dictionary"; }
#endif
    public:
        typedef Instances::fl_utils::Dictionary InstanceType;

    public:
        Dictionary(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_utils::Dictionary> MakeInstance(Dictionary& t)
        {
            return Pickable<Instances::fl_utils::Dictionary>(new(t.Alloc()) Instances::fl_utils::Dictionary(t));
        }
        SPtr<Instances::fl_utils::Dictionary> MakeInstanceS(Dictionary& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

//##protect##"instance_traits$methods"
        static Pickable<Instances::fl_utils::Dictionary> MakeInstance(Dictionary& t, bool weakKeys)
        {
            return Pickable<Instances::fl_utils::Dictionary>(new(t.Alloc()) Instances::fl_utils::Dictionary(t, weakKeys));
        }
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_utils
{
    class Dictionary : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Dictionary"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_utils::Dictionary InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Dictionary(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

