/**************************************************************************

PublicHeader:   GFx
Filename    :   AS3_Abc_Type.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_AS3_Abc_Type_H
#define INC_AS3_Abc_Type_H

#include "Kernel/SF_Array.h"
#include "Kernel/SF_ArrayPaged.h"
#include "Kernel/SF_StringHash.h"
#include "Kernel/SF_Alg.h"
#include "../AS3_StringManager.h"
#include "GFx/GFx_PlayerStats.h"

// #define VERSION_AIR
// #define SF_AS3_16BIT_ABC_INDEX

#ifdef SF_BUILD_DEBUG
#define GFX_AS3_TRACE
#endif

///////////////////////////////////////////////////////////////////////////////
#if 1
    #define REPORT_NOT_IMPLEMENTED
#else
/*
    // Minimalistic version of VM.
    #define AS3_NO_FLASH
    #ifdef GFX_AS3_VERBOSE
        #undef GFX_AS3_VERBOSE
    #endif
*/
#endif

///////////////////////////////////////////////////////////////////////////////
#if defined(GFX_AS3_VERBOSE)

#define SF_DEBUG_ARG(x) , x
#define SF_DEBUG_CODE(x) x

#else

#define SF_DEBUG_ARG(x)
#define SF_DEBUG_CODE(x)

#endif

#define WARN_NOT_IMPLEMENTED(x) GetVM().GetUI().Output(FlashUI::Output_Warning, "The method " x " is not implemented\n");
#define WARN_NOT_IMPLEMENTED_STATIC(x) vm.GetUI().Output(FlashUI::Output_Warning, "The method " x " is not implemented\n");

#define WARN_NOT_SUPPORTED_MISSING_DEPENDENCY(x) GetVM().GetUI().Output(FlashUI::Output_Warning, "The method " x " is not supported because core dependencies have been removed\n");

#ifdef REPORT_NOT_IMPLEMENTED
#define NOT_IMPLEMENTED(x) GetVM().ThrowError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG(x)));
    #define NOT_IMPLEMENTED_STATIC(x) vm.ThrowError(VM::Error(VM::eNotImplementedError, vm SF_DEBUG_ARG(x)));
#else
    #define NOT_IMPLEMENTED(x) WARN_NOT_IMPLEMENTED(x)
    #define NOT_IMPLEMENTED_STATIC(x) WARN_NOT_IMPLEMENTED_STATIC(x)
#endif


namespace Scaleform
{

///////////////////////////////////////////////////////////////////////////
enum { Mem_Stat = GFx::StatMV_VM };


///////////////////////////////////////////////////////////////////////////
// String utilities.

inline
String operator +(const char* l, const String& r)
{
    return String(l, r);
}

///////////////////////////////////////////////////////////////////////////
inline
String operator +(const char* l, const GFx::ASString& r)
{
    return String(l) + String(r.ToCStr(), r.GetSize());
}

inline
String operator +(const String& l, const GFx::ASString& r)
{
    return l + String(r.ToCStr(), r.GetSize());
}

} // namespace Scaleform


///////////////////////////////////////////////////////////////////////////
namespace Scaleform { namespace GFx { namespace AS3 
{

///////////////////////////////////////////////////////////////////////////
// Forward declaration.
class Object;
class Class;
class Value;
class VM;
class VMAppDomain;

/////////////////////////////////////////////////////////////////////////// 

// ***** SPtr<>

// SPtr is a smart pointer used for garbage-collectible classes within the VM.
// A special aspect of this class is that it uses lower bit of the pointer
// value as a marker to disable the next Release() call as needed for
// garbage collection of cycles. This bit is only applied by the garbage
// collector, allowing object destructor to be used instead of having an
// explicit finalize method, which would be error-prone.
//
// All member SPtr<> references to child objects must be referenced from
// the ForEachChild_GC() virtual function of that class for collector to
// see it properly.

template <typename T>
class SPtr
{
public:
    friend class Value;
    template <typename AT> friend class APtr;

    typedef T       ObjType;
    typedef SPtr<T> SelfType;

public:
    // Constructors.

    SPtr(ObjType* v = NULL)
        : pObject(v)
    {
        SF_ASSERT(v == NULL || (Scaleform::UPInt)v > 4);
        AddRef();
    }
    // Copy constructor.
    SPtr(const SPtr<ObjType>& other)
        : pObject(other.pObject)
    {
        AddRef();
    }
    ~SPtr()
    {
        Release();
    }

public:
    // Pick constructors.
    // We save on AddRef().

    SPtr(SelfType& other, PickType)
        : pObject(other.pObject)
    {
        other.pObject = NULL;
        // No AddRef() on purpose.
    }
    SPtr(Pickable<ObjType> v)
        : pObject(v.GetPtr())
    {
        SF_ASSERT(v.GetPtr() == NULL || (Scaleform::UPInt)v.GetPtr() > 4);
        // No AddRef() on purpose.
    }

public:
    // Assignment operator.
    SelfType& operator =(const SelfType& other)
    {
        return Set(other);
    }
    SelfType& operator =(ObjType* p)
    {
        return SetPtr(p);
    }
    SelfType& operator =(Pickable<ObjType> v)
    {
        return Pick(v);
    }

public:
    // Pickers.

    // Pick a value.
    SelfType& Pick(SelfType& other)
    {
        if (&other != this)
        {
            Release();

            pObject = other.pObject;
            other.pObject = NULL;
        }

        return *this;
    }

    SelfType& Pick(Pickable<ObjType> v)
    {
        if (v.GetPtr() != pObject)
        {
            Release();

            pObject = v.GetPtr();
        }

        return *this;
    }

    SelfType& Pick(ObjType* p)
    {
        if (p != pObject)
        {
            Release();

            pObject = p;
        }

        return *this;
    }

public:
    // Setters.

    SelfType& SetPtr(ObjType* p)
    {
        if (p != pObject)
        {
            SF_ASSERT(p == NULL || (Scaleform::UPInt)p > 4);

            if (p)
                p->AddRef();
            Release();

            pObject = p;
        }

        return *this;
    }
    SelfType& Set(const SelfType& other)
    {
        if (&other != this)
        {
            other.AddRef();
            Release();

            pObject = other.pObject;
        }

        return *this;
    }

public:
    // Getters.

    ObjType* operator->() const
    {
        return pObject;
    }
    ObjType& operator*() const
    {
        SF_ASSERT(pObject);
        return *pObject;
    }

    // Obtain pointer reference directly, for D3D interfaces
    ObjType*& GetRawRef() { return pObject; }
    ObjType** GetRawPtr() const { return &pObject; }
    ObjType*  GetPtr() const
    {
        return pObject;
    }
    // Conversion
    operator ObjType*() const
    {
        return pObject;
    }

private:
    void AddRef() const
    {
        if (pObject)
            pObject->AddRef();
    }
    void Release()
    {
        if (pObject)
        {
            if ((Scaleform::UPInt)pObject & 1)
                pObject = (ObjType*)((Scaleform::UPInt)pObject - 1);
            else
            {
                pObject->Release();
                // SF_DEBUG_CODE(pObject = NULL);
            }
        }
    }

private:
    mutable ObjType* pObject;
}; // SPtr

///////////////////////////////////////////////////////////////////////////
namespace ClassTraits
{
    class Traits;
}

typedef Pickable<ClassTraits::Traits> (*TTraitsFactory)(VM& vm);

struct TypeInfo
{
    enum {
        TimeTypeOffset = 0,
        ObjectTypeOffset = 1,
        ImplTypeOffset = 2,
        FinalTypeOffset = 3,
        ClassTypeOffset = 4,
        // CW compatibility flags.
        NotImplTypeOffset = 5,
    };
    enum {
        TimeTypeMask = 1 << TimeTypeOffset,
        ObjectTypeMask = 1 << ObjectTypeOffset,
        ImplTypeMask = 1 << ImplTypeOffset,
        FinalTypeMask = 1 << FinalTypeOffset,
        ClassTypeMask = 1 << ClassTypeOffset,
        NotImplTypeMask = 1 << NotImplTypeOffset,
    };

    enum TimeType {RunTime = 0, CompileTime = TimeTypeMask};
    enum ObjectType {SealedObject = 0, DynamicObject = ObjectTypeMask};
    enum ImplType {Cpp = 0, Abc = ImplTypeMask};
    enum FinalType {Alterable = 0, Final = FinalTypeMask};
    enum ClassType {TypeClass = 0, TypeInterface = ClassTypeMask};
    enum NotImplType {Implemented = 0, NotImplemented = NotImplTypeMask};

    static const TypeInfo* None[];

    bool IsCompileTime() const { return (Flags & TimeTypeMask) != 0; }
    bool IsDynamicObject() const { return (Flags & ObjectTypeMask) != 0; }
    bool IsAbcObject() const { return (Flags & ImplTypeMask) != 0; }
    bool IsFinalType() const { return (Flags & FinalTypeMask) != 0; }
    bool IsInterface() const { return (Flags & ClassTypeMask) != 0; }
    bool IsClass() const { return (Flags & ClassTypeMask) == 0; }
    // CW compatibility flags.
    bool IsNotImplemented() const { return (Flags & NotImplTypeMask) != 0; }
    bool IsImplemented() const { return (Flags & NotImplTypeMask) == 0; }

    const char* GetName() const { return Name; }
    const char* GetPkgName() const { return PkgName; }
    const TypeInfo* GetParent() const { return Parent; }

    bool IsBoolean() const { return (PkgName == NULL || PkgName[0] == 0) && strcmp(Name, "Boolean") == 0; }
    bool IsInt() const { return (PkgName == NULL || PkgName[0] == 0) && strcmp(Name, "int") == 0; }
    bool IsUInt() const { return (PkgName == NULL || PkgName[0] == 0) && strcmp(Name, "uint") == 0; }
    bool IsNumber() const { return (PkgName == NULL || PkgName[0] == 0) && strcmp(Name, "Number") == 0; }
    bool IsString() const { return (PkgName == NULL || PkgName[0] == 0) && strcmp(Name, "String") == 0; }

    UInt32              Flags;
    // Future development.
    UInt16              InstanceSize;
    UInt16              ClassMethodNum;
    UInt16              ClassMemberNum;
    UInt16              InstanceMethodNum;
    UInt16              InstanceMemberNum;
    const char*         Name;
    const char*         PkgName;
    const TypeInfo*     Parent;
    const TypeInfo**    Implements; // An array of pointers. Last record should be NULL.
};

struct ThunkInfo;
struct MemberInfo;

struct ClassInfo
{
    const TypeInfo*     Type;
    TTraitsFactory      Factory;
    const ThunkInfo*    ClassMethod;
    const MemberInfo*   ClassMember;
    const ThunkInfo*    InstanceMethod;
    const MemberInfo*   InstanceMember;

    bool IsCompileTime() const { return Type->IsCompileTime(); }
    bool IsDynamicObject() const { return Type->IsDynamicObject(); }
    bool IsAbcObject() const { return Type->IsAbcObject(); }
    bool IsFinalType() const { return Type->IsFinalType(); }
    // CW compatibility flags.
    bool IsNotImplemented() const { return Type->IsNotImplemented(); }
    bool IsImplemented() const { return Type->IsImplemented(); }

    const char* GetName() const { return Type->GetName(); }
    const char* GetPkgName() const { return Type->GetPkgName(); }
    const TypeInfo* GetParent() const { return Type->GetParent(); }

    UInt16 GetInstanceSize() const { return Type->InstanceSize; }
    UInt16 GetClassMethodNum() const { return Type->ClassMethodNum; }
    UInt16 GetClassMemberNum() const { return Type->ClassMemberNum; }
    UInt16 GetInstanceMethodNum() const { return Type->InstanceMethodNum; }
    UInt16 GetInstanceMemberNum() const { return Type->InstanceMemberNum; }
};

namespace Classes
{
#ifdef GFX_AS3_SUPPORT
    extern const ClassInfo* ClassRegistrationTable[];
#endif
}

///////////////////////////////////////////////////////////////////////////
namespace fl {
    extern const ClassInfo ClassCI;
}

///////////////////////////////////////////////////////////////////////////
namespace Abc
{
    ///////////////////////////////////////////////////////////////////////////
    // Instruction offset within TOpCode (which is an OpCodeBuffer)
    typedef UPInt TCodeOffset;

    ///////////////////////////////////////////////////////////////////////////
    // NamespaceKind describes the type of namespace NamespaceInfo represents.
    enum NamespaceKind
    {
        NS_Undefined = -1,
        NS_Public,
        NS_Protected,
        NS_StaticProtected,
        NS_Private,  // Private NS shouldn't be interned (private namespaces don't match).
        NS_Explicit, // What is this for? Perhaps it identifies NSs with explicit URIs?
        NS_PackageInternal
    };

    ///////////////////////////////////////////////////////////////////////////
#ifdef SF_AS3_16BIT_ABC_INDEX
    typedef SInt16 SInd;
    typedef UInt16 UInd;
#else
    typedef SInt32 SInd;
    typedef UInt32 UInd;
#endif

    ///////////////////////////////////////////////////////////////////////////
    // MethodInfo index
    class MiInd
    {
    public:
        explicit MiInd(SInd ind = -1) : Ind(ind) {}
        bool operator ==(const MiInd& other) const { return Ind == other.Ind; }

    public:
        SInd Get() const
        {
            return Ind;
        }

        bool IsValid() const
        {
            return Ind >= 0;
        }

    private:
        SInd    Ind;
    };

    // MethodBodyInfo index
    class MbiInd
    {
    public:
        explicit MbiInd(SInd ind = -1) : Ind(ind) {}
        bool operator ==(const MbiInd& other) const { return Ind == other.Ind; }

    public:
        SInd Get() const
        {
            return Ind;
        }

        bool IsValid() const
        {
            return Ind >= 0;
        }

    private:
        // Make sure the member variables stay with no padding
        // Padding breaks the hash function that uses this struct as a key
        SInd    Ind;
    };

}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {


#endif // INC_AS3_Abc_Type_H
