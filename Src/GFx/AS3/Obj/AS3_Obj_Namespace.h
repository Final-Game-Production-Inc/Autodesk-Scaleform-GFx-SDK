//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Namespace.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Namespace_H
#define INC_AS3_Obj_Namespace_H

#include "AS3_Obj_Object.h"
//##protect##"includes"
#include "Kernel/SF_Hash.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl
{
    extern const TypeInfo NamespaceTI;
    extern const ClassInfo NamespaceCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl
{
    class Namespace;
}}

namespace InstanceTraits { namespace fl
{
    class Namespace;
}}

namespace Classes { namespace fl
{
    class Namespace;
}}

//##protect##"forward_declaration"
class VM;

// NamespaceInstanceFactory, used for namespace interning, is implemented in source file.
class NamespaceInstanceFactory;

namespace InstanceTraits { namespace fl
{
    class Namespace;
}}

namespace Instances { namespace fl
{
    class Namespace : public GASRefCountBase
    {
        friend class InstanceTraits::fl::Namespace;

#ifdef GFX_AS3_VERBOSE
    public:
        virtual const char* GetAS3ObjectType() const { return "Namespace"; }
        virtual Scaleform::String GetAS3ObjectName() const { return Scaleform::String(Uri.ToCStr()); }
#endif
    private:
        friend class AS3::VM;

        // ATTENSION.
        // Object created with this constructor is not completely initialized.
        // It doesn't have Traits set up.
        Namespace(VM& vm, Abc::NamespaceKind kind, const char* uri);

    protected:
        // Creates a namespace. 'factory' argument can be null if namespace is not interned,
        // and thus wasn't added to factory hash-table.
        Namespace(
            VM& vm,
            NamespaceInstanceFactory* factory, 
            Abc::NamespaceKind kind, 
            const ASString& uri, 
            const Value& prefix
            );

        ~Namespace();

    public:
        virtual void AS3Constructor(unsigned argc, const Value* argv);
        Namespace& operator =(const Namespace& other);

        //
        bool operator == (const Namespace& other) const
        {
            return (Uri == other.Uri) && (Kind == other.Kind);
        }
        bool operator != (const Namespace& other) const
        {
            return !operator ==(other);
        }

        //
        const Value& GetPrefix() const      { return Prefix; }
        void SetPrefix(const Value& value)  { Prefix = value; }        
        //
        const ASString& GetUri() const      { return Uri; }
        void SetUri(const ASString& value)  { Uri = value; }       
        void SetUri(const Value& value);
        //
        Abc::NamespaceKind GetKind() const      { return static_cast<Abc::NamespaceKind>(Kind); }
        void SetKind(Abc::NamespaceKind value)  { Kind = value; }

#ifdef SF_AS3_ENABLE_INSTANCE_NAMESPACES
        //
        AS3::Traits* GetOwner() const
        {
            return pOwner;
        }
        void SetOwner(AS3::Traits& tr)
        {
            pOwner = &tr;
        }

        // This method doesn't seem to be used any more.
        bool IsParentOf(const Namespace& other) const
        {
            if (GetOwner() && other.GetOwner())
                return GetOwner()->IsParentOf(*other.GetOwner());

            return false;
        }
#endif

        // GC
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;

        static unsigned CalcHash(Abc::NamespaceKind kind, const ASString& uri)
        {
            return (uri.GetHash() << 2) ^ kind;
        }    
        unsigned  GetHash() const
        {
            return CalcHash(GetKind(), Uri); 
        }
        VM& GetVM() const
        {
            return VMRef;
        }
        //
        bool GetIsInterface() const
        {
            return IsInterface;
        }
        void SetIsInterface(bool flag = true)
        {
            IsInterface = flag;
        }

        void GetNextPropertyName(Value& name, GlobalSlotIndex ind) const;
        GlobalSlotIndex GetNextPropIndex(GlobalSlotIndex ind) const;
        void GetNextPropertyValue(Value& value, GlobalSlotIndex ind);

    public:
        bool IsAS3() const
        {
            return GetKind() == Abc::NS_Public && Uri == NS_AS3;
        }
        bool IsVMPublic() const;

    private:
        SPInt                           Kind:4; // One bit for sign.
        UPInt                           IsInterface:1;

        VM&                             VMRef;
        ASString                        Uri;
        Ptr<NamespaceInstanceFactory>   pFactory;
#ifdef SF_AS3_ENABLE_INSTANCE_NAMESPACES
        SPtr<AS3::Traits>               pOwner;
#endif
        Value                           Prefix;
    };
}} // namespace Instances { namespace fl

//##protect##"forward_declaration"

namespace InstanceTraits { namespace fl
{
    class Namespace : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Namespace"; }
#endif
    public:
        typedef Instances::fl::Namespace InstanceType;

    public:
        Namespace(VM& vm, const ClassInfo& ci);

    public: 
        virtual void MakeObject(Value& result, Traits& t);

        static void prefixGet(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void uriGet(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void AS3valueOf(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void AS3toString(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void valueOfProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void toStringProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        enum { ThunkInfoNum = 4 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[4];
//##protect##"instance_traits$methods"
        ~Namespace();

        // Creates a new namespace without interning it. May be useful for private namespaces.
        Pickable<Instances::fl::Namespace> MakeInstance(Abc::NamespaceKind kind, const ASString& uri, const Value& prefix)
        {
            return Pickable<Instances::fl::Namespace>(SF_HEAP_AUTO_NEW(this) Instances::fl::Namespace(GetVM(), pNamespaceFactory, kind, uri, prefix));
        }
        // Creates a new namespace and/or interns it for sharing.
        Pickable<Instances::fl::Namespace> MakeInternedInstance(Abc::NamespaceKind kind, const ASString& uri,
            const Value& prefix = Value::GetUndefined());
        // Get an interned namespace.
        Instances::fl::Namespace& GetInternedInstance(Abc::NamespaceKind kind, const ASString& uri,
            const Value& prefix = Value::GetUndefined());

//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
        Ptr<NamespaceInstanceFactory> pNamespaceFactory;
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl
{
    class Namespace : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Namespace"; }
#endif
    public:
        typedef Classes::fl::Namespace ClassType;
        typedef InstanceTraits::fl::Namespace InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Namespace(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
        virtual bool Coerce(const Value& value, Value& result) const;
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl
{
    class Namespace : public Class
    {
        friend class ClassTraits::fl::Namespace;
        static const TypeInfo& GetTypeInfo() { return AS3::fl::NamespaceTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Namespace"; }
#endif
    public:
        typedef Namespace SelfType;
        typedef Namespace ClassType;
        
    private:
        Namespace(ClassTraits::Traits& t);
    public:
        virtual void InitPrototype(AS3::Object& obj) const;
        enum { ThunkInfoNum = 2 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[2];
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
    public:
        virtual void Call(const Value& _this, Value& result, unsigned argc, const Value* const argv);
        virtual void Construct(Value& result, unsigned argc, const Value* argv, bool extCall = false);
//##protect##"class_$methods"

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

