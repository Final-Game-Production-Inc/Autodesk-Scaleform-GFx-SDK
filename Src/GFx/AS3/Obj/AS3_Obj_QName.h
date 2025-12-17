//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_QName.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_QName_H
#define INC_AS3_Obj_QName_H

#include "AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl
{
    extern const TypeInfo QNameTI;
    extern const ClassInfo QNameCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl
{
    class QName;
}}

namespace InstanceTraits { namespace fl
{
    class QName;
}}

namespace Classes { namespace fl
{
    class QName;
}}

//##protect##"forward_declaration"
#define SF_URI_AS_NAMESPACE
//##protect##"forward_declaration"

namespace Instances { namespace fl
{
    class QName : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl::QName;
        
    public:
        typedef QName SelfType;
        typedef Classes::fl::QName ClassType;
        typedef InstanceTraits::fl::QName TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl::QNameTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl::QName"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        QName(InstanceTraits::Traits& t);

//##protect##"instance$methods"
#ifdef SF_URI_AS_NAMESPACE
        // ns == NULL means "any namespace".
        QName(InstanceTraits::Traits& t, const ASString& n, Namespace* ns);
#else
        QName(InstanceTraits::Traits& t, const ASString& n, const ASString& u);
        ~QName();
#endif

    public:
#ifdef SF_URI_AS_NAMESPACE
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;
#endif

        virtual void AS3Constructor(unsigned argc, const Value* argv);

#ifdef SF_URI_AS_NAMESPACE
        Namespace* GetNamespace() const { return Ns; }

        bool IsAnyNamespace() const
        {
            return Ns.GetPtr() == NULL;
        }
#else
        ASStringNode* GetUri() const
        {
            return Uri;
        }
        bool IsAnyNamespace() const
        {
            return Uri == NULL;
        }
#endif
        const ASString& GetLocalName() const
        {
            return LocalName;
        }

    public:
        virtual void GetNextPropertyName(Value& name, GlobalSlotIndex ind) const;
        virtual GlobalSlotIndex GetNextDynPropIndex(GlobalSlotIndex ind) const;
        virtual void GetNextPropertyValue(Value& value, GlobalSlotIndex ind);
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_localNameGet, 
            mid_uriGet, 
            mid_AS3valueOf, 
            mid_AS3toString, 
            mid_toStringProto, 
        };
        void localNameGet(ASString& result);
        void uriGet(Value& result);
        void AS3valueOf(SPtr<Instances::fl::QName>& result);
        void AS3toString(ASString& result);

        // C++ friendly wrappers for AS3 methods.
        ASString localNameGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            localNameGet(result);
            return result;
        }
        Value uriGet()
        {
            Value result;
            uriGet(result);
            return result;
        }
        SPtr<Instances::fl::QName> AS3valueOf()
        {
            SPtr<Instances::fl::QName> result;
            AS3valueOf(result);
            return result;
        }
        ASString AS3toString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            AS3toString(result);
            return result;
        }
        static void toStringProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);

//##protect##"instance$data"
        ASString        LocalName;
#ifdef SF_URI_AS_NAMESPACE
        SPtr<Namespace> Ns;
#else
        ASStringNode*   Uri;
#endif
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl
{
    class QName : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::QName"; }
#endif
    public:
        typedef Instances::fl::QName InstanceType;

    public:
        QName(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl::QName> MakeInstance(QName& t)
        {
            return Pickable<Instances::fl::QName>(new(t.Alloc()) Instances::fl::QName(t));
        }
        SPtr<Instances::fl::QName> MakeInstanceS(QName& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 4 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[4];
//##protect##"instance_traits$methods"
#ifdef SF_URI_AS_NAMESPACE
        Pickable<Instances::fl::QName> MakeInstance(Traits& t, const ASString& n, Instances::fl::Namespace* ns)
        {
            return Pickable<Instances::fl::QName>(new(t.Alloc()) Instances::fl::QName(t, n, ns));
        }
#else
        Pickable<Instances::fl::QName> MakeInstance(Traits& t, const ASString& n, const ASString& u)
        {
            return Pickable<Instances::fl::QName>(new(t.Alloc()) Instances::fl::QName(t, n, u));
        }
#endif
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl
{
    class QName : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::QName"; }
#endif
    public:
        typedef Classes::fl::QName ClassType;
        typedef InstanceTraits::fl::QName InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        QName(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl
{
    class QName : public Class
    {
        friend class ClassTraits::fl::QName;
        static const TypeInfo& GetTypeInfo() { return AS3::fl::QNameTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::QName"; }
#endif
    public:
        typedef QName SelfType;
        typedef QName ClassType;
        
    private:
        QName(ClassTraits::Traits& t);
    public:
        virtual void InitPrototype(AS3::Object& obj) const;
        enum { ThunkInfoNum = 1 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[1];
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
        virtual void Construct(Value& _this, unsigned argc, const Value* argv, bool extCall = false);
        virtual void Call(const Value& _this, Value& result, unsigned argc, const Value* const argv);
//##protect##"class_$methods"

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

