/**************************************************************************

Filename    :   AS3_VM.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_AS3_VM_H
#define INC_AS3_VM_H


#include "Obj/AS3_Obj_Function.h"
#include "Obj/AS3_Obj_Activation.h"
#include "Obj/AS3_Obj_Namespace.h" // Because of MakePublickNamespace().
#include "Obj/AS3_Obj_ClassClass.h"

#include "AS3_FlashUI.h" // for UI.

#include "AS3_StringManager.h"
#include "GFx/AMP/Amp_ViewStats.h"
#include "GFx/AMP/Amp_Server.h"

#include "AS3_ValueStack.h"
#include "AS3_ObjCollector.h"
#include <math.h>

// Enabling of CallFrameCache is supposed to speed up VM. In reality it slows
// it down by about 10%, though it reduces size of executable by about 10k.
// (Measured on Windows)
// #define SF_AS3_ENABLE_CALLFRAME_CACHE

// Define to enable getabsobject opcode.
// This opcode should be disabled only for debugging.
#define SF_AS3_ENABLE_GETABSOBJECT
#define SF_AS3_VARARGNUM 4095
#define SF_AS3_SETUP_INSTANCE_CONSTRUCTOR_IN_CLASS
#define SF_AS3_STORE_PRIMITIVE_VALUES_IN_BYTECODE
// #define SF_AS3_EMIT_DEF_ARGS

// #define SF_AS3_ENABLE_EXPLICIT_GO

#if !defined(SF_AS3_ENABLE_EXPLICIT_GO) && (defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2))
    #define SF_AS3_ENABLE_EXPLICIT_GO
#endif

namespace Scaleform { class ConsoleUI; }
namespace Scaleform { namespace GFx { namespace AMP { class ViewStats; } } }
namespace Scaleform { namespace GFx { class AS3ValueObjectInterface; } }

///////////////////////////////////////////////////////////////////////////////
namespace Scaleform { namespace GFx { namespace AS3
{

///////////////////////////////////////////////////////////////////////////
// Forward declaration.
namespace Classes { 
    namespace fl
    {
        class Boolean;
        class Number;
        class int_;
        class uint;
        class String;
        class Array;
        class Catch;
        class Error;
        class Date;
        class Math;
        class RegExp;
        class QName;
        class XML;
        class XMLList;
    }

    namespace fl_vec
    {
        class Vector;
        class Vector_int;
        class Vector_uint;
        class Vector_double;
        class Vector_String;
        class Vector_object;
    }

    namespace fl_utils
    {
        class ByteArray;
    }
}

namespace ClassTraits { 
    namespace fl
    {
        class UserDefined;
        class Boolean;
        class Number;
        class int_;
        class uint;
        class String;
        class Array;
        class Catch;

        class QName;
        class XML;
        class XMLList;

        class Domain;
    }

    namespace fl_vec
    {
        class Vector;
        class Vector_int;
        class Vector_uint;
        class Vector_double;
        class Vector_String;
        class Vector_object;
    }

    namespace fl_system
    {
        class ApplicationDomain;
    }

    namespace fl_utils
    {
        class ByteArray;
    }
}

namespace Instances
{
    class Function;
}

namespace Instances {
    namespace fl
    {
        class Function;
        class XML;
        class XMLList;
        class QName;
        class GlobalObject;
    }

    namespace fl_utils
    {
        class ByteArray;
    }
}

namespace InstanceTraits { namespace fl
{
    class QName;
}}

#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
namespace AOT
{
    struct InfoCollector;
}
#endif


///////////////////////////////////////////////////////////////////////////

// MultinameHash is used to map {Name, Namespace} -> Traits/Class
// VM should maintain a list of all registered ClassTraits so that it
// can look them up based on multiname for coerce/astype operations.
//
// Currently used by VM::ClassSet, but that should be replaced with traits.

template<class C, int SID = Stat_Default_Mem>
class MultinameHash
{
    friend class VM;

    struct Key
    {
        ASString                         Name;
        SPtr<const Instances::fl::Namespace> pNs;

        Key(const ASString& name, const Instances::fl::Namespace* ns) : Name(name), pNs(ns) { }
        Key(const Key& other) : Name(other.Name), pNs(other.pNs) { }

        bool operator == (const Key& other) const
        { return (Name == other.Name) && (*pNs == *other.pNs); }
        
        struct HashFunctor
        {
            UPInt operator()(const Key& key) const
            { return key.Name.GetHash() ^ key.pNs->GetHash(); }
        };
    };

    // TODO: check performance with HashUncachedLH instead of HashLH.
    typedef HashDH<Key, C, typename Key::HashFunctor, SID> TContainer;
    TContainer Entries;

public:
    typedef C ValueType;
    typedef MultinameHash<C, SID> SelfType;

public:
    MultinameHash(void* pheap) : Entries(pheap) {}
    ~MultinameHash()
    {
        Clear();
    }

public:
    C*  Get(const ASString& name, const Instances::fl::Namespace& ns)
    {
        return Entries.Get(Key(name, &ns));
    }

    C*  Get(const Multiname& mn)
    {
        if (!mn.IsMultiname())
            return Get(mn.GetName(), mn.GetNamespace());
        
        const NamespaceSet::TContainer& nsArray = mn.GetNamespaceSet().GetNamespaces();

        C* p = 0;
        const UPInt size = nsArray.GetSize();
        for (UPInt i =0; !p && i < size; ++i)
            p = Get(mn.GetName(), *nsArray[i]);
        return p;        
    }

    void Add(const ASString& name, const Instances::fl::Namespace& ns, const C& val)
    {       
        Entries.Add(Key(name, &ns), val);
    }
    void Set(const ASString& name, const Instances::fl::Namespace& ns, const C& val)
    {       
        Entries.Set(Key(name, &ns), val);
    }
    void Remove(const ASString& name, const Instances::fl::Namespace& ns)
    {
        Entries.Remove(Key(name, &ns));
    }
    void RemoveValue(const ValueType& v)
    {
        typename TContainer::Iterator it = Entries.Begin();

        for (; !it.IsEnd(); ++it)
        {
            if (it->Second == v)
            {
                //it.Remove(); // Won't compile.
                Entries.Remove(it->First);
                break;
            }
        }
    }

    void Clear()
    {
       Entries.Clear();
    }
    void GetMultinames(Array<SPtr<const Instances::fl::Namespace> >& multinames) const
    {
        for (MultinameHash<ClassTraits::Traits*, StatMV_VM_VM_Mem>::TContainer::ConstIterator it = Entries.Begin(); it != Entries.End(); ++it)
        {
            multinames.PushBack(it->First.pNs);
        }
    }
    bool IsEmpty() const
    {
        return Entries.IsEmpty();
    }
}; // class MultinameHash


template<class C, int SID = Stat_Default_Mem>
class AbcMultinameHash
{
    struct Key
    {
        UPInt NsInd;
        SPInt NextMnInd;

        Key(UPInt ind, SInt32 next_ind = -1) : NsInd(ind), NextMnInd(next_ind) { }
        Key(const Key& other) : NsInd(other.NsInd), NextMnInd(other.NextMnInd) { }

        bool operator == (const Key& other) const
        { return (NsInd == other.NsInd) && (NextMnInd == other.NextMnInd); }
    };

public:
    typedef C ValueType;
    typedef AbcMultinameHash<C> SelfType;

public:
    ~AbcMultinameHash()
    {
        Clear();
    }

public:
    ///
    C*  Get(UPInt ind, SInt32 next_ind = -1)
    {
        return Entries.Get(Key(ind, next_ind));
    }
    C*  Get(const Abc::Multiname& mn)
    {
        if (mn.IsQName())
            return Get(mn.GetNamespaceInd(), mn.GetNextMultinameIndex());

        return NULL;
    }

    ///
    void Add(UPInt nsInd, const C& val)
    {       
        Entries.Add(Key(nsInd), val);
    }
    void Add(const Abc::Multiname& mn, const C& val)
    {       
        Entries.Add(Key(mn.GetNamespaceInd(), mn.GetNextMultinameIndex()), val);
    }

    ///
    void Clear()
    {
        Entries.Clear();
    }

    ///
    void ForEachChild_GC(RefCountCollector<Mem_Stat>* prcc, RefCountBaseGC<Mem_Stat>::GcOp op
                         SF_DEBUG_ARG(const RefCountBaseGC<Mem_Stat>& owner)) const
    {
        typename ContainerType::ConstIterator it = Entries.Begin();
        while (!it.IsEnd())
        {
            const C& v = it->Second;
            // Fr some reason AS3::ForEachChild_GC<C, Mem_Stat>(it->Second, op); doesn't want to compile.
            if (v)
                (*op)(prcc, const_cast<const AS3::RefCountBaseGC<Mem_Stat>**>(reinterpret_cast<AS3::RefCountBaseGC<Mem_Stat>**>(v.GetRawPtr())) 
                SF_DEBUG_ARG(owner));
            ++it;
        }
    }

private:
    typedef HashLH<Key, C, FixedSizeHash<Key>, SID> ContainerType;
    ContainerType Entries;
}; // class AbcMultinameHash

class VMAppDomain;

namespace TR
{
    class State;
}

class VMFile : public GASRefCountBase
{
    friend class TR::State; // Because of ASStringNodeSet.
    friend class Tracer; // Because of ASStringNodeSet.

public:
    VMFile(VM& vm, VMAppDomain& appDomain);
    virtual ~VMFile();

public:
    virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;

public:
    // Obtains interned Namespace/NamespaceSets for the file.
    // nsIndex is zero-based.
    Instances::fl::Namespace&   GetInternedNamespace(const Abc::Multiname& mn);
    Instances::fl::Namespace&   GetInternedNamespace(UPInt nsIndex);
    NamespaceSet&           GetInternedNamespaceSet(UPInt nsSetIndex);
    // strIndex is zero based.
    ASString                GetInternedString(UPInt strIndex);
    InstanceTraits::Traits& GetActivationInstanceTraits(const Abc::MbiInd mbi_ind SF_DEBUG_ARG(const ASString& name));

    virtual const Abc::Multiname& GetMultiname(UPInt ind) const = 0;

    void RegisterLoadedClass(const SPtr<ClassTraits::Traits>& tr)
    {
        LoadedClasses.PushBack(tr);
    }
    UPInt GetNumOfLoadedClasses() const
    {
        return LoadedClasses.GetSize();
    }

public:
    VMAppDomain& GetAppDomain() const
    {
        return *AppDomain;
    }

    VM& GetVM() const
    {
        return VMRef;
    }

protected:
    virtual Pickable<Instances::fl::Namespace> MakeInternedNamespace(const Abc::Multiname& mn) const = 0;
    virtual Pickable<Instances::fl::Namespace> MakeInternedNamespace(UPInt nsIndex) const = 0;
    virtual Pickable<NamespaceSet> MakeInternedNamespaceSet(UPInt nsSetIndex) = 0;
    virtual Pickable<InstanceTraits::Traits> MakeActivationInstanceTraits(const Abc::MbiInd mbi_ind SF_DEBUG_ARG(const ASString& name)) = 0;
    virtual ASString MakeInternedString(UPInt strIndex) const = 0;

    void Clear();

    void AddClassTrait(const ASString& name, const Instances::fl::Namespace& ns, ClassTraits::Traits* val);
    void RemoveClassTrait(const ASString& name, const Instances::fl::Namespace& ns);
    void RemoveClassTraitValue(ClassTraits::Traits* val);

    ClassTraits::Traits* GetLoadedClass(UPInt i)
    {
        return LoadedClasses[i];
    }
    void ClearLoadedClasses()
    {
        LoadedClasses.Clear();
    }

private:
    // It should store Activation Traits and not just Traits.
    typedef HashLH<Abc::MbiInd, SPtr<InstanceTraits::Traits>, FixedSizeHash<Abc::MbiInd>, StatMV_VM_VMAbcFile_Mem> TActivationTraitsCache;
    typedef HashSetUncachedLH<Ptr<ASStringNode>, ASStringNodePtrHashFunc, ASStringNodePtrHashFunc, StatMV_VM_VMAbcFile_Mem> TASStringNodeSet;

    VM& VMRef;
    SPtr<VMAppDomain> AppDomain;

    // Interned namespaces and namespace-sets arrays are file-specific.    
    //ArrayLH<SPtr<Instances::fl::Namespace> > IntNamespaces;
    AbcMultinameHash<SPtr<Instances::fl::Namespace>, StatMV_VM_VMAbcFile_Mem> IntNamespaces;
    ArrayLH<SPtr<NamespaceSet>, StatMV_VM_VMAbcFile_Mem> IntNamespaceSets;
    ArrayLH<SPtr<ASStringNode>, StatMV_VM_VMAbcFile_Mem> IntStrings;
    TActivationTraitsCache ActivationTraitsCache;
    // LoadedClasses keeps a record of classes directly loaded from a file
    // and classes created indirectly from directly loaded classes.
    // At this time indirectly created classes are Vector-based classes.
    ArrayLH<SPtr<ClassTraits::Traits>, StatMV_VM_VMAbcFile_Mem> LoadedClasses;
    TASStringNodeSet ASStringNodeSet;
}; // class VMFile

struct MethodInfo
{
    MethodInfo() : MaxOpStackSize(0) {}

    UPInt MaxOpStackSize;
    Abc::TOpCode OpCode;
    Abc::MethodBodyInfo::Exception Exception;
};

// VMAbcFile wraps the use of Abc::File in the VM and stores extra VM-specific
// data associated with it, such as string pool and {Multiname -> Class} mapping tables.

class VMAbcFile : public VMFile
{
    friend class VM; // Because of Load().
    friend class Tracer; // Because of AbsObjects.
    friend class Classes::fl_vec::Vector; // Because of LoadedClasses.

#ifdef GFX_AS3_VERBOSE
public:
    virtual const char* GetAS3ObjectType() const { return "VMAbcFile"; }
    virtual Scaleform::String GetAS3ObjectName() const { return Scaleform::String(File->GetName()); }
#endif

public:
    typedef HashSetLH<SPtr<Instances::fl::GlobalObjectScript> > TGlobalObjectScriptSet;

    VMAbcFile(
        VM& vm, 
        const Ptr<Abc::File>& file, 
        VMAppDomain& appDomain 
        SF_AOTC_ARG(AOT::InfoCollector* ic)
        SF_AOTC2_ARG(AOT::InfoCollector* ic)
        );
    virtual ~VMAbcFile();

public:
    Value                       GetDetailValue(const Abc::ValueDetail& d);
    InstanceTraits::Function&   GetFunctionInstanceTraits(Instances::fl::GlobalObjectScript& gos, UInt32 method_ind);

    virtual const Abc::Multiname& GetMultiname(UPInt ind) const;

    // TBD: Move String pool here, so that it is VM-specific (and
    // so that Abc file can be shared between VMs safely).
    // 

    // TBD: We should store instanceTraits/classes for private typenames
    // of the file here, since they shouldn't be added to the vm-wide
    // multiname type table.

    // Research: MN index -> Class table here? Could allow for
    // faster coerce implementation... but could there be potential lookup conflicts?
    // This may not be worth it, since we'd need a hash-table (there could be
    // way too many multinames in file).


    // Delegates to original file
    const Abc::File&            GetAbcFile() const    { return *File; }
    const Abc::ConstPool&       GetConstPool() const  { return File->GetConstPool(); }
    const Abc::MethodTable&     GetMethods() const    { return File->GetMethods(); }
    const Abc::ClassTable&      GetClasses() const    { return File->GetClasses(); }
    const Abc::MetadataTable&   GetMetadata() const   { return File->GetMetadata(); }
    const Abc::TraitTable&      GetTraits() const     { return File->GetTraits(); }
    const Abc::MethodBodyTable& GetMethodBody() const { return File->GetMethodBodyTable(); }
	const Abc::MethodInfo&		GetMethodInfo(Abc::MiInd ind) const { return File->GetMethods().Get(ind); }
    Abc::MbiInd GetMethodBodyInfoInd(Abc::MiInd ind) const { return GetMethodInfo(ind).GetMethodBodyInfoInd(); }
    const Abc::MethodBodyInfo&  GetMethodBodyInfo(Abc::MiInd ind) const
    {
        const Abc::MethodInfo& mi = GetMethodInfo(ind);
        SF_ASSERT(mi.GetMethodBodyInfoInd().IsValid());

        return mi.GetMethodBodyInfo(GetMethodBody());
    }

    const Abc::TOpCode& GetOpCode(const CallFrame& cf);

    const Abc::MethodBodyInfo::Exception& GetException(Abc::MbiInd ind) const
    {
#ifdef SF_AS3_USE_WORDCODE
        return MethodInfoArray[ind.Get()].Exception;
#else
        return GetMethodBody().Get(ind).GetException();
#endif
    }

    const TGlobalObjectScriptSet& GetGOScripts() const
    {
        return GlobalObjects;
    }

private:
    virtual Pickable<Instances::fl::Namespace> MakeInternedNamespace(const Abc::Multiname& mn) const;
    virtual Pickable<Instances::fl::Namespace> MakeInternedNamespace(UPInt nsIndex) const;
    virtual Pickable<NamespaceSet> MakeInternedNamespaceSet(UPInt nsSetIndex);
    virtual Pickable<InstanceTraits::Traits> MakeActivationInstanceTraits(const Abc::MbiInd mbi_ind SF_DEBUG_ARG(const ASString& name));
    virtual ASString MakeInternedString(UPInt strIndex) const;

    virtual void    ForEachChild_GC(Collector* prcc, GcOp op) const;
    virtual void    Finalize_GC();
    virtual bool    HasFinalize_GC() const { return true; }
    // Register all ClassTraits from a file.
    // Return true if a file should be loaded.
    bool            RegisterUserDefinedClassTraits();
    void            UnregisterUserDefinedClassTraits();

    bool            RegisterScrips(bool to_execute);

    CheckResult     Load(bool to_execute);
public: //?
    // UnRegister() will unregister this file with VM. It won't automatically
    // unload file. Unloading happens after releasing of all references to this file
    // and calling garbage collector.
    void            UnRegister();

private:
    // Avoid warning; no assignment operator.
    VMAbcFile& operator =(const VMAbcFile&);
    
private:
    typedef HashLH<UInt32, SPtr<InstanceTraits::Function>, FixedSizeHash<UInt32>, StatMV_VM_VMAbcFile_Mem> TFunctionTraitsCache;
    typedef HashSetUncachedLH<ASString, ASStringNodeHashFunc, ASStringNodeHashFunc, StatMV_VM_VMAbcFile_Mem> TAliasClass;

    Ptr<Abc::File>  File;

#ifdef SF_AS3_ENABLE_GETABSOBJECT
    HashSetLH<SPtr<GASRefCountBase> >               AbsObjects;
#endif

    // GlobalObjects registers associated global objects.
    TGlobalObjectScriptSet                          GlobalObjects;
    ArrayLH<SPtr<VMAbcFile>, StatMV_VM_VMAbcFile_Mem> Children;
    TFunctionTraitsCache                            FunctionTraitsCache;

    // Exceptions maps *method body index* (from Abc file) into MethodInfo.
    ArrayLH<MethodInfo, StatMV_VM_VMAbcFile_Mem>    MethodInfoArray;

    TAliasClass                                     AliasClass;

#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
    AOT::InfoCollector* pIC;
#endif
}; // VMAbcFile


///////////////////////////////////////////////////////////////////////////
typedef ValueRegisterFile RegistersType;

///////////////////////////////////////////////////////////////////////////
class CallFrame : public NewOverrideBase<StatMV_VM_CallFrame_Mem>
{
    friend class VM;
    friend class CallFrameCache;
    friend class VMAbcFile;
    
public:
#ifndef SF_AS3_ENABLE_CALLFRAME_CACHE
    CallFrame(
        const Value& invoker, // invoker can be NULL if there is nothing to track.
        VMAbcFile& file,
        Abc::MbiInd mbi_ind, // Method body info index.
        bool discard_result,
#ifdef SF_AS3_ENABLE_EXPLICIT_GO
        Instances::fl::GlobalObjectScript& gos,
#endif
        const ScopeStackType& _saved_scope,
        const Traits& ot // Origination Traits
        SF_DEBUG_ARG(const ASString& name)
        SF_DEBUG_ARG(UInt64 startTicks)
        );
    CallFrame(const CallFrame& other);

    CallFrame& operator =(const CallFrame& other);
#endif

    CallFrame(MemoryHeap* heap);
    ~CallFrame();

public:
#ifdef SF_AS3_ENABLE_CALLFRAME_CACHE
    void Setup(
        const Value& invoker, // invoker can be NULL if there is nothing to track.
        VMAbcFile& file,
        SInt32 mbi_ind, // Method body info index.
        const ScopeStackType* _saved_scope,
        bool discard_result,
        const Traits* ot // Origination Traits
        SF_DEBUG_ARG(const ASString& name)
        SF_DEBUG_ARG(UInt64 startTicks)
        );
#endif

#if 0
    const Abc::MethodBodyInfo& GetMethodBodyInfo() const
    {
        SF_ASSERT(pMBI);
        return *pMBI;
    }
#else
    Abc::MbiInd GetMethodBodyInd() const
    {
        return MBIIndex;
    }
    const Abc::MethodBodyInfo& GetMethodBodyInfo() const
    {
        return GetFile().GetMethodBody().Get(GetMethodBodyInd());
    }
    const Abc::MethodBodyInfo::Exception& GetException() const
    {
        return GetFile().GetException(GetMethodBodyInd());
    }
#endif
    const Abc::Multiname& GetReturnType() const
    {
        return GetMethodBodyInfo().GetMethodInfo(GetFile().GetMethods()).
               GetReturnType(GetFile().GetConstPool());
    }
    // Can throw exceptions.
#ifdef GFX_AS3_TRACE
    const Abc::TOpCode& GetCode() const
    {
        if (OCPtr == NULL)
            OCPtr = &GetFile().GetOpCode(*this);

        return *OCPtr;
    }
#else
    const Abc::TOpCode& GetCode() const
    {
        return GetFile().GetOpCode(*this);
    }
#endif
    Abc::TOpCodeVTP GetCP() const;
    const ScopeStackType& GetSavedScope() const
    {
        SF_ASSERT(pSavedScope);
        return *pSavedScope;
    }
    // Register's index starts with ZERO ...
    Value& GetRegister(UPInt ind) const
    {
        return pRegisterFile->operator[](ind);
    }
	const Value& GetThis() const
	{
		return GetRegister(0);
	}
    UPInt GetScopeStackBaseInd() const
    {
        return ScopeStackBaseInd;
    }
    bool NeedToDiscardResult() const
    {
        return DiscardResult;
    }
    const Traits& GetOriginationTraits() const
    {
        SF_ASSERT(OriginationTraits);
        return *OriginationTraits;
    }
    VM& GetVM() const
    {
        return GetFile().GetVM();
    }
    VMAbcFile& GetFile() const
    {
        SF_ASSERT(pFile);
        return *pFile;
    }
    const Value& GetInvoker() const
    {
        return Invoker;
    }
#ifdef SF_AS3_ENABLE_EXPLICIT_GO
    Instances::fl::GlobalObjectScript& GetGlobalObjectScript() const
    {
        SF_ASSERT(GOS);

//         if (!GOS->IsInitialized())
//             GOS->InitOnDemand();

        return *GOS;
    }
#endif

#ifdef GFX_ENABLE_XML
    // If Default Namespace is NULL,then it is a public namespace.
    Instances::fl::Namespace* GetDefXMLNamespace() const
    {
        return DefXMLNamespace;
    }
#endif

#ifdef SF_AS3_ENABLE_CALLFRAME_CACHE
    void Clear();
#endif

    Instances::fl::GlobalObjectScript* GetGlobalObject() const;

    SF_DEBUG_CODE(UInt32 GetCurrFileInd() const { return CurrFileInd; } )
    SF_DEBUG_CODE(ASString GetCurrFileName() const { return GetFile().GetInternedString(CurrFileInd); } )
    SF_DEBUG_CODE(UInt32 GetCurrLineNumber() const { return CurrLineNumber; } )

    SF_DEBUG_CODE(ASStringNode* GetName() const { return Name; } )
    SF_DEBUG_CODE(UInt64 GetStartTicks() const { return StartTicks; } )
    SF_DEBUG_CODE(bool IsRegisteredFunction() const { return RegisteredFunction; } )
    
public:
    void ClearOpStack();
    void ClearScopeStack()
    {
        pScopeStack->Resize(ScopeStackBaseInd);
    }

#ifndef SF_AS3_ENABLE_CALLFRAME_CACHE
    void SetACopy()
    {
        ACopy = true;
    }
#endif
    
private:
    void SetCP(Abc::TOpCodeVTP cp)
    {
        CP = cp;
    }
    SF_INLINE
    void SetRegister(UPInt ind, const Value& value)
    {
        pRegisterFile->operator[](ind) = value;
    }
    SF_INLINE
    void SetRegisterPickUnsafe(UPInt ind, Value& value)
    {
        pRegisterFile->operator[](ind).PickUnsafe(value);
    }
    SF_INLINE
    void SwapRegister(UPInt ind, Value& value)
    {
        pRegisterFile->operator[](ind).Swap(value);
    }
    void SetupRegisters(const Abc::MethodInfo& mi, const Value& _this, unsigned argc, const Value* argv);
    SF_DEBUG_CODE(void SetRegisteredFunction(bool registered) { RegisteredFunction = registered; } )
    
private:
    bool                        DiscardResult;
#ifdef SF_AS3_EMIT_DEF_ARGS
    mutable bool                OpStackIsAllocated;
#endif
#ifndef SF_AS3_ENABLE_CALLFRAME_CACHE
    bool                        ACopy;
#endif
    SF_DEBUG_CODE(bool          RegisteredFunction;)
    UPInt                       ScopeStackBaseInd; // Hot data.
    mutable Abc::TOpCodeVTP     CP;
#ifdef GFX_AS3_TRACE
    mutable const Abc::TOpCode* OCPtr;
#endif
    RegistersType*              pRegisterFile;
    MemoryHeap*                 pHeap;
    VMAbcFile*                  pFile;
    Abc::MbiInd                 MBIIndex;
    // This member shouldn't be const because objects in saved scope may be modified later ...
    const ScopeStackType*       pSavedScope;
    const Traits*               OriginationTraits;
    ScopeStackType*             pScopeStack;
    Value*                      PrevInitialStackPos;

#ifdef SF_AS3_ENABLE_EXPLICIT_GO
    Instances::fl::GlobalObjectScript* GOS;
#endif

    // Default XML Namespace.
    SPtr<Instances::fl::Namespace>  DefXMLNamespace;

    Value*                          PrevFirstStackPos;
    SF_DEBUG_CODE(UInt16            PrevReservedNum;)
    SF_DEBUG_CODE(SPtr<ASStringNode> Name;)
    SF_DEBUG_CODE(UInt32            CurrFileInd;)
    SF_DEBUG_CODE(UInt32            CurrLineNumber;)
    SF_DEBUG_CODE(UInt64            StartTicks;)
    // We need this member to prevent destruction of invoker, what may happen if, for example, function
    // was created dynamically on stack by the "newfunction" opcode.
    Value                           Invoker;
}; // class CallFrame

///////////////////////////////////////////////////////////////////////////
#ifdef SF_AS3_ENABLE_CALLFRAME_CACHE
class CallFrameCache
{
public:
    CallFrameCache(MemoryHeap* heap);
    ~CallFrameCache();

public:
    CallFrame* MakeCallFrame();
    void DestroyCallFrame(CallFrame* cf);

private:
    typedef ArrayConstPolicy<16, 16, true> SizePolicy;
    typedef ArrayDH_POD<CallFrame*, StatMV_VM_VM_Mem, SizePolicy> CacheType;

    MemoryHeap* Heap;
    CacheType CallFrames;
};
#endif

///////////////////////////////////////////////////////////////////////////
template <typename T>
struct Efficient
{
    typedef T type;
};
template <>
struct Efficient<Value>
{
    typedef Value& type;
};

///////////////////////////////////////////////////////////////////////////
class FileLoader
{
public:
    virtual ~FileLoader() {}

public:
    virtual UPInt GetSize() const = 0;
    // Last file in the list is the main file to run.
    virtual const Ptr<Abc::File>& GetFile(UPInt n) = 0;
    virtual const Abc::File* GetFile(const ASString& name) = 0;
    virtual void AddFile(Ptr<AS3::Abc::File>& file) = 0;
};

///////////////////////////////////////////////////////////////////////////
class XMLSupport : public GASRefCountBase
{
    friend class VM;

protected:
    XMLSupport(VM& vm, bool enabled = false);

public:
    virtual ~XMLSupport();

public:
    virtual void ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op) const;

public:
    bool IsEnabled() const { return Enabled; }

    virtual CheckResult EqualsXML(bool& result, Instances::fl::XML& l, Instances::fl::XML& r) const; 
    virtual CheckResult EqualsXML(bool& stop, bool& result, Instances::fl::XML& l, const Value& r) const; 
    virtual CheckResult EqualsXMLList(bool& result, Instances::fl::XMLList& l, const Value& v) const; 
    virtual CheckResult EqualsQName(bool& result, Instances::fl::QName& l, Instances::fl::QName& r) const; 

    virtual bool Add(Value& result, Object& l, Object& r) const; 
    virtual CheckResult ToXMLString(VM& vm, Value& v) const;
    virtual void DescribeType(VM& vm, SPtr<Instances::fl::XML>& result, const Value& value) const;

    // XML
    virtual const ClassTraits::Traits* GetClassTraitsXML() const;
    virtual InstanceTraits::Traits* GetITraitsXML() const;
    // XMLList
    virtual const ClassTraits::Traits* GetClassTraitsXMLList() const;
    virtual InstanceTraits::Traits* GetITraitsXMLList() const;

private:
    bool Enabled;
};

///////////////////////////////////////////////////////////////////////////
CheckResult GetArrayInd(ASStringNode* sn, UInt32& ind);
CheckResult GetArrayInd(const Multiname& prop_name, UInt32& ind);

CheckResult GetVectorInd(ASStringNode* sn, UInt32& ind);
CheckResult GetVectorInd(const Multiname& prop_name, UInt32& ind);

CheckResult GetStrNumber(ASStringNode* sn, Value::Number& result);

///////////////////////////////////////////////////////////////////////////
bool IsScaleformGFx(const Instances::fl::Namespace& ns);

///////////////////////////////////////////////////////////////////////////
namespace Classes
{
    class UserDefined;
}

namespace Classes {
    namespace fl
    {
        class System;
    }
}

namespace InstanceTraits
{
    namespace fl
    {
        class GlobalObject;
        class GlobalObjectScript;
    }
}

namespace InstanceTraits
{
    class UserDefined;
}

namespace TR
{
    class State;
}

namespace AOT
{
    struct NamespaceInfo;
}

class AvmDisplayObj;
class Stage;
class MovieRoot;
class Scripts;

typedef MultinameHash<ClassTraits::Traits*, StatMV_VM_VM_Mem>   TClassTraitsCache;

class VMAppDomain : public GASRefCountBase
{
public:
    VMAppDomain(VM& vm, VMAppDomain* parentDomain);
    virtual ~VMAppDomain();

    void AddChild(VMAppDomain& childDomain);
    SPtr<VMAppDomain> AddNewChild(VM& vm);
    VMAppDomain* GetParent() const;
    bool RemoveChild(VMAppDomain& childDomain);
    ClassTraits::Traits** GetClassTrait(const ASString& name, const Instances::fl::Namespace& ns);
    ClassTraits::Traits** GetClassTrait(const Multiname& mn);
    ClassTraits::Traits** GetClassTraitNoParent(const ASString& name, const Instances::fl::Namespace& ns);
    void AddClassTrait(const ASString& name, const Instances::fl::Namespace& ns, ClassTraits::Traits* val);
    void SetClassTrait(const ASString& name, const Instances::fl::Namespace& ns, ClassTraits::Traits* val);
    void RemoveClassTrait(const ASString& name, const Instances::fl::Namespace& ns);
    void RemoveClassTraitValue(ClassTraits::Traits* val);
    void GetMultinames(Array<SPtr<const Instances::fl::Namespace> >& multinames) const;
    bool IsEmpty() const;

    virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;

    static void SetEnabled(bool enabled) { Enabled = enabled; }
    static bool IsEnabled() { return Enabled; }

private:
    typedef ArrayLH<SPtr<VMAppDomain>, StatMV_VM_VM_Mem> TChildDomains;

    VMAppDomain*        ParentDomain;
    TClassTraitsCache   ClassTraitsSet;
    TChildDomains       ChildDomains;

    static bool         Enabled;
};

inline
void VMFile::AddClassTrait(const ASString& name, const Instances::fl::Namespace& ns, ClassTraits::Traits* val)
{
    GetAppDomain().AddClassTrait(name, ns, val);
}

inline
void VMFile::RemoveClassTrait(const ASString& name, const Instances::fl::Namespace& ns)
{
    GetAppDomain().RemoveClassTrait(name, ns);
}

inline
void VMFile::RemoveClassTraitValue(ClassTraits::Traits* val)
{
    GetAppDomain().RemoveClassTraitValue(val);
}

///////////////////////////////////////////////////////////////////////////
class VM : public NewOverrideBase<StatMV_VM_VM_Mem>
{
    friend class XMLSupportImpl; // Because of PrepareClassTraits().
    friend class CallFrame; // Because of Strings& GetStrings().
    friend class Object; // Because of Strings& GetStrings().
    friend class Traits; // Because of different class traits.
    friend class StackReader; // Because of op_stack, e.t.c.
    friend class SlotInfo; // Because of Execute().
    friend class InitializerGuard; // Because of EnterInitializer()/LeaveInitializer().
    friend class VMAbcFile; // Because of GlobalObjects.
    friend class Class; // Because of OpStack;
    friend class Scripts; // Because of OpStack;

    friend class Instances::ThunkFunction; // Because of Execute().
    friend class Instances::fl::GlobalObjectCPP; // Because it is a part of VM.
    friend class Instances::fl::GlobalObjectScript; // Because of ExecuteCode().
    friend class Instances::FunctionBase; // Because of PushResult()/RetrieveResult()
    friend class Instances::Function; // Because of ExecuteCode().
    friend class Instances::fl_utils::ByteArray; // Because of deserialization.

    friend class Classes::fl::Object; // Because of different data type classes.
    friend class Classes::fl::System; // Because of the Garbage Collector.
    friend class Classes::fl_vec::Vector; // Because of GetGlobalObjectCPP().
    friend class Classes::fl::XML; // Because of OpStack.
    friend class Classes::UserDefined; // Because of AddFrame().

    friend class InstanceTraits::fl::GlobalObjectScript; // Because of RegisterClassTraits().
    friend class InstanceTraits::UserDefined; // Because of AddFrame().

    friend class Tracer; // It needs to access internal data because it generates new code.
    friend class TR::State; // Because of GetUserDefinedTraits().

    friend class AvmDisplayObj; // Because of ExecuteCode().
    friend class AvmBitmap; // Because of ExecuteCode().
    friend class Stage; // Because of ExecuteCode().
    friend class MovieRoot; // Because of ExecuteCode().
    friend class Scaleform::ConsoleUI; // Because of ExecuteCode().

    friend int Read(VM& vm, Multiname& obj);    
    
public:
    VM(FlashUI& _ui, AS3::StringManager& sm, ASRefCountCollector& gc SF_AOTC_ARG(AOT::InfoCollector* ic) SF_AOTC2_ARG(AOT::InfoCollector* ic));
    virtual ~VM();

    SF_AMP_CODE(virtual AMP::ViewStats* GetAdvanceStats() const { return NULL; } )
    
public:
    enum ErrorID
    {
        eOutOfMemoryError                               = 1000,
        eNotImplementedError                            = 1001,
        eInvalidPrecisionError                          = 1002,
        eInvalidRadixError                              = 1003,
        eInvokeOnIncompatibleObjectError                = 1004,
        eArrayIndexNotIntegerError                      = 1005,
        eCallOfNonFunctionError                         = 1006,
        eConstructOfNonFunctionError                    = 1007,
        eAmbiguousBindingError                          = 1008,
        eConvertNullToObjectError                       = 1009,
        eConvertUndefinedToObjectError                  = 1010,
        eIllegalOpcodeError                             = 1011,
        eLastInstExceedsCodeSizeError                   = 1012,
        eFindVarWithNoScopeError                        = 1013,
        eClassNotFoundError                             = 1014,
        eIllegalSetDxns                                 = 1015,
        eDescendentsError                               = 1016,
        eScopeStackOverflowError                        = 1017,
        eScopeStackUnderflowError                       = 1018,
        eGetScopeObjectBoundsError                      = 1019,
        eCannotFallOffMethodError                       = 1020,
        eInvalidBranchTargetError                       = 1021,
        eIllegalVoidError                               = 1022,
        eStackOverflowError                             = 1023,
        eStackUnderflowError                            = 1024,
        eInvalidRegisterError                           = 1025,
        eSlotExceedsCountError                          = 1026,
        eMethodInfoExceedsCountError                    = 1027,
        eDispIdExceedsCountError                        = 1028,
        eDispIdUndefinedError                           = 1029,
        eStackDepthUnbalancedError                      = 1030,
        eScopeDepthUnbalancedError                      = 1031,
        eCpoolIndexRangeError                           = 1032,
        eCpoolEntryWrongTypeError                       = 1033,
        eCheckTypeFailedError                           = 1034,
        eIllegalSuperCallError                          = 1035,
        eCannotAssignToMethodError                      = 1037,
        eRedefinedError                                 = 1038,
        eCannotVerifyUntilReferencedError               = 1039,
        eCantUseInstanceofOnNonObjectError              = 1040,
        eIsTypeMustBeClassError                         = 1041,
        eInvalidMagicError                              = 1042,
        eInvalidCodeLengthError                         = 1043,
        eInvalidMethodInfoFlagsError                    = 1044,
        eUnsupportedTraitsKindError                     = 1045,
        eMethodInfoOrderError                           = 1046,
        eMissingEntryPointError                         = 1047,
        ePrototypeTypeError                             = 1049,
        eConvertToPrimitiveError                        = 1050,
        eIllegalEarlyBindingError                       = 1051,
        eInvalidURIError                                = 1052,
        eIllegalOverrideError                           = 1053,
        eIllegalExceptionHandlerError                   = 1054,
        eWriteSealedError                               = 1056,
        eIllegalSlotError                               = 1057,
        eIllegalOperandTypeError                        = 1058,
        eClassInfoOrderError                            = 1059,
        eClassInfoExceedsCountError                     = 1060,
        eNumberOutOfRangeError                          = 1061,
        eWrongArgumentCountError                        = 1063,
        eCannotCallMethodAsConstructor                  = 1064,
        eUndefinedVarError                              = 1065,
        eFunctionConstructorError                       = 1066,
        eIllegalNativeMethodBodyError                   = 1067,
        eCannotMergeTypesError                          = 1068,
        eReadSealedError                                = 1069,
        ePropertyNotFoundError                          = 1069, //manually added duplicated number to match actual error
        eCallNotFoundError                              = 1070,
        eAlreadyBoundError                              = 1071,
        eZeroDispIdError                                = 1072,
        eDuplicateDispIdError                           = 1073,
        eConstWriteError                                = 1074,
        eMathNotFunctionError                           = 1075,
        eMathNotConstructorError                        = 1076,
        eWriteOnlyError                                 = 1077,
        eIllegalOpMultinameError                        = 1078,
        eIllegalNativeMethodError                       = 1079,
        eIllegalNamespaceError                          = 1080,
        eReadSealedErrorNs                              = 1081,
        eNoDefaultNamespaceError                        = 1082,
        eXMLPrefixNotBound                              = 1083,
        eXMLBadQName                                    = 1084,
        eXMLUnterminatedElementTag                      = 1085,
        eXMLOnlyWorksWithOneItemLists                   = 1086,
        eXMLAssignmentToIndexedXMLNotAllowed            = 1087,
        eXMLMarkupMustBeWellFormed                      = 1088,
        eXMLAssigmentOneItemLists                       = 1089,
        eXMLMalformedElement                            = 1090,
        eXMLUnterminatedCData                           = 1091,
        eXMLUnterminatedXMLDecl                         = 1092,
        eXMLUnterminatedDocTypeDecl                     = 1093,
        eXMLUnterminatedComment                         = 1094,
        eXMLUnterminatedAttribute                       = 1095,
        eXMLUnterminatedElement                         = 1096,
        eXMLUnterminatedProcessingInstruction           = 1097,
        eXMLNamespaceWithPrefixAndNoURI                 = 1098,

        eRegExpFlagsArgumentError                       = 1100,
        eNoScopeError                                   = 1101,
        eIllegalDefaultValue                            = 1102,
        eCannotExtendFinalClass                         = 1103,
        eXMLDuplicateAttribute                          = 1104,
        eCorruptABCError                                = 1107,
        eInvalidBaseClassError                          = 1108,
        eDanglingFunctionError                          = 1109,
        eCannotExtendError                              = 1110,
        eCannotImplementError                           = 1111,
        eCoerceArgumentCountError                       = 1112,
        eInvalidNewActivationError                      = 1113,
        eNoGlobalScopeError                             = 1114,
        eNotConstructorError                            = 1115,
        eApplyError                                     = 1116,
        eXMLInvalidName                                 = 1117,
        eXMLIllegalCyclicalLoop                         = 1118,
        eDeleteTypeError                                = 1119,
        eDeleteSealedError                              = 1120,
        eDuplicateMethodBodyError                       = 1121,
        eIllegalInterfaceMethodBodyError                = 1122,
        eFilterError                                    = 1123,
        eInvalidHasNextError                            = 1124,
        eOutOfRangeError                                = 1125,
        eVectorFixedError                               = 1126,
        eTypeAppOfNonParamType                          = 1127,
        eWrongTypeArgCountError                         = 1128,

        eFileOpenError                                  = 1500,
        eFileWriteError                                 = 1501,
        eScriptTimeoutError                             = 1502,
        eScriptTerminatedError                          = 1503,
        eEndOfFileError                                 = 1504,
        eStringIndexOutOfBoundsError                    = 1505,
        eInvalidRangeError                              = 1506,
        eNullArgumentError                              = 1507,
        eInvalidArgumentError                           = 1508,
        eShellCompressedDataError                       = 1509,
        eArrayFilterNonNullObjectError                  = 1510,

        eParamRangeError                                = 2006,
        eNullPointerError                               = 2007,
        eInvalidEnumError                               = 2008,
        eCannotInstantiateError                         = 2012,
        eArgumentError                                  = 2015,
        eAddObjectItselfError                           = 2024,
        eMustBeChildError                               = 2025,
        eMustBeNonNegative                              = 2027,
        eEOFError                                       = 2030,
        eCompressedDataError                            = 2058,

        eIllegalOperationError                          = 2077,

        // flash.utils.Proxy - related error codes.
        eProxyGetPropertyError                          = 2088,
        eProxySetPropertyError                          = 2089,
        eProxyCallPropertyError                         = 2090,
        eProxyHasPropertyError                          = 2091,
        eProxyDeletePropertyError                       = 2092,
        eProxyGetDescendantsError                       = 2093,
        eProxyNextNameIndexError                        = 2105,
        eProxyNextNameError                             = 2106,
        eProxyNextValueError                            = 2107,

        eSceneNotFound                                  = 2108,
        eFrameLabelNotFoundInScene                      = 2109,

        eSWFHasInvalidData                              = 2136,
    };

public:
    bool CheckObject(const Value& v);

    static void exec_nop();
    int exec_throw(Abc::TOpCodeVTP cp, CallFrame& cf);
    void exec_getsuper(VMFile& file, const Traits& ot, const Abc::Multiname& mn);
    void exec_setsuper(VMFile& file, const Traits& ot, const Abc::Multiname& mn);
    void exec_dxns(CallFrame& cf, UInt32 index);
    void exec_dxnslate();
    void exec_kill(UInt32 v);

    int exec_ifnlt(SInt32 v);
    int exec_ifnlt_ti(SInt32 v);
    int exec_ifnlt_td(SInt32 v);

    int exec_ifnle(SInt32 v);
    int exec_ifnle_ti(SInt32 v);
    int exec_ifnle_td(SInt32 v);

    int exec_ifngt(SInt32 v);
    int exec_ifngt_ti(SInt32 v);
    int exec_ifngt_td(SInt32 v);

    int exec_ifnge(SInt32 v);
    int exec_ifnge_ti(SInt32 v);
    int exec_ifnge_td(SInt32 v);

    static int exec_jump(SInt32 v);

    int exec_iftrue(SInt32 v);
    int exec_iftrue_tb(SInt32 v);

    int exec_iffalse(SInt32 v);
    int exec_iffalse_tb(SInt32 v);

    int exec_ifeq(SInt32 v);
    int exec_ifeq_ti(SInt32 v);
    int exec_ifeq_td(SInt32 v);

    int exec_ifne(SInt32 v);
    int exec_ifne_ti(SInt32 v);
    int exec_ifne_td(SInt32 v);

    int exec_iflt(SInt32 v);
    int exec_iflt_ti(SInt32 v);
    int exec_iflt_td(SInt32 v);

    int exec_ifle(SInt32 v);
    int exec_ifle_ti(SInt32 v);
    int exec_ifle_td(SInt32 v);

    int exec_ifgt(SInt32 v);
    int exec_ifgt_ti(SInt32 v);
    int exec_ifgt_td(SInt32 v);

    int exec_ifge(SInt32 v);
    int exec_ifge_ti(SInt32 v);
    int exec_ifge_td(SInt32 v);

    int exec_ifstricteq(SInt32 v);
    int exec_ifstrictne(SInt32 v);
    int exec_lookupswitch();
    void exec_pushwith();
    void exec_popscope();
    void exec_nextname();
    void exec_hasnext();
    void exec_pushnull();
    void exec_pushundefined();
    void exec_nextvalue();
    void exec_pushbyte(UInt8 v);
    void exec_pushshort(SInt32 v);
    void exec_pushtrue();
    void exec_pushfalse();
    void exec_pushnan();
    void exec_pop();
    void exec_dup();
    void exec_pop_nrc();
    void exec_dup_nrc();
    void exec_swap();
    void exec_pushstring(VMFile& file, const UPInt ind);
    void exec_pushint(SInt32 v);
    void exec_pushuint(UInt32 v);
    void exec_pushdouble(double v);
    void exec_pushscope();
    void exec_pushnamespace(Instances::fl::Namespace& v);
    void exec_hasnext2(UInt32 object_reg, UInt32 index_reg);
#if 0
    void exec_li8();
    void exec_li16();
    void exec_li32();
    void exec_lf32();
#endif
    static void exec_lf64();
    static void exec_si8();
    static void exec_si16();
    static void exec_si32();
    static void exec_sf32();
    static void exec_sf64();
    void exec_newfunction(const CallFrame& cf, UInt32 method_ind);
    void exec_call(UInt32 arg_count);
    void exec_callobject(UInt32 arg_count);
    void exec_construct(UInt32 arg_count);
    void exec_callmethod(UInt32 method_index, UInt32 arg_count);
    void exec_callsupermethod(const Traits& ot, UInt32 method_index, UInt32 arg_count);
    void exec_callgetter(UInt32 method_index, UInt32 arg_count);
    void exec_callsupergetter(const Traits& ot, UInt32 method_index, UInt32 arg_count);
    void exec_callstatic(VMAbcFile& file, Abc::MiInd ind, UInt32 arg_count);
    void exec_callsuper(VMFile& file, const Traits& ot, const Abc::Multiname& mn, UInt32 arg_count);
    void exec_callproperty(VMFile& file, const Abc::Multiname& mn, UInt32 arg_count);
    void exec_returnvoid();
    void exec_returnvalue();
    void exec_constructsuper(const Traits& ot, UInt32 arg_count);
    void exec_constructprop(VMFile& file, const Abc::Multiname& mn, UInt32 arg_count);
    void exec_callproplex(VMFile& file, const Abc::Multiname& mn, UInt32 arg_count);
    void exec_callsupervoid(VMFile& file, const Traits& ot, const Abc::Multiname& mn, UInt32 arg_count);
    void exec_callpropvoid(VMFile& file, const Abc::Multiname& mn, UInt32 arg_count);
    static void exec_sxi1();
    static void exec_sxi8();
    static void exec_sxi16();
    void exec_applytype(UInt32 arg_count);
    void exec_newobject(UInt32 arg_count);
    void exec_newarray(UInt32 arr_size);
    void exec_newactivation(const CallFrame& cf);
    void exec_newclass(VMAbcFile& file, UInt32 v);
    void exec_getdescendants(VMFile& file, const Abc::Multiname& mn);
    void exec_newcatch(VMAbcFile& file, const Abc::MethodBodyInfo::ExceptionInfo& e);
    void exec_findpropstrict(VMFile& file, const Abc::Multiname& mn, const ScopeStackType& ss);
    void exec_findproperty(VMFile& file, const Abc::Multiname& mn, const ScopeStackType& ss, Instances::fl::GlobalObject& go);
    void exec_getlex(VMFile& file, const Abc::Multiname& mn, const ScopeStackType& ss);
    void exec_setproperty(VMFile& file, const Abc::Multiname& mn);
    void exec_getlocal(UInt32 v);
    void exec_setlocal(UInt32 v);
    void exec_getglobalscope();
    void exec_getscopeobject(UInt32 index);
    void exec_getproperty(VMFile& file, const Abc::Multiname& mn);
    void exec_getouterscope(CallFrame& cf, UInt32 scope_index);
    void exec_initproperty(VMFile& file, const Abc::Multiname& mn);
    void exec_deleteproperty(VMFile& file, const Abc::Multiname& mn);
    void exec_getslot(UInt32 slot_index);
    void exec_setslot(UInt32 slot_index);
    void exec_getglobalslot(UInt32 slot_index);
    void exec_setglobalslot(UInt32 slot_index);
    void exec_convert_s();
    void exec_esc_xelem();
    void exec_esc_xattr();
    void exec_convert_i();
    void exec_convert_u();
    void exec_convert_d();
    void exec_convert_b();
    void exec_convert_o();
    void exec_checkfilter();
    void exec_coerce(VMFile& file, const Abc::Multiname& mn);
    void exec_coerce_a();
    void exec_coerce_s();
    void exec_astype(VMFile& file, const Abc::Multiname& mn);
    void exec_astypelate();
    void exec_typeof();

    void exec_not();
    void exec_not_tb();

    void exec_bitnot();
    void exec_modulo();
    void exec_lshift();
    void exec_rshift();
    void exec_urshift();
    void exec_bitand();
    void exec_bitor();
    void exec_bitxor();

    void exec_equals();
    void exec_strictequals();
    void exec_lessthan();
    void exec_lessequals();
    void exec_greaterthan();
    void exec_greaterequals();

    void exec_instanceof();
    void exec_istype(VMFile& file, const Abc::Multiname& mn);
    void exec_istypelate();
    void exec_in();
    void exec_getabsobject(UPInt addr);
    // slot_ind is ONE based.
    // slot_ind - 1 is an index in a slot table.
    void exec_getabsslot(UPInt slot_ind);
    void exec_setabsslot(UPInt slot_ind);
    void exec_initabsslot(UPInt slot_ind);

    void exec_increment();
    void exec_increment_i();
    void exec_increment_ti();
    void exec_increment_tu();

    void exec_decrement();
    void exec_decrement_i();
    void exec_decrement_ti();
    void exec_decrement_tu();

    void exec_inclocal(UInt32 v);
    void exec_inclocal_i(UInt32 v);
    void exec_inclocal_ti(UInt32 v);
    void exec_inclocal_tu(UInt32 v);

    void exec_declocal(UInt32 v);
    void exec_declocal_i(UInt32 v);
    void exec_declocal_ti(UInt32 v);
    void exec_declocal_tu(UInt32 v);

    void exec_negate();
    void exec_negate_i();
    void exec_negate_ti();
    void exec_negate_td();

    void exec_add();
    void exec_add_d();
    void exec_add_i();
    void exec_add_ti();
    void exec_add_td();

    void exec_subtract();
    void exec_subtract_i();
    void exec_subtract_ti();
    void exec_subtract_td();

    void exec_multiply();
    void exec_multiply_i();
    void exec_multiply_ti();
    void exec_multiply_td();

    void exec_divide();
    void exec_divide_td();

    void exec_getlocal0();
    void exec_getlocal1();
    void exec_getlocal2();
    void exec_getlocal3();
    void exec_setlocal0();
    void exec_setlocal1();
    void exec_setlocal2();
    void exec_setlocal3();

#ifdef ENABLE_STRICT_SETSLOT
    void exec_setslot_str(UInt32 offset);
    void exec_setslot_num(UInt32 offset);
    void exec_setslot_uint(UInt32 offset);
    void exec_setslot_sint(UInt32 offset);
    void exec_setslot_bool(UInt32 offset);
    void exec_setslot_value(UInt32 offset);
    void exec_setslot_obj_as(UInt32 offset);
    void exec_setslot_obj_cpp(UInt32 offset);
#endif

    void exec_debug();
    void exec_debugline(CallFrame& cf, UInt32 v);
    void exec_debugfile(CallFrame& cf, UInt32 v);

public:
    // AOT-specific data types.
    class ResourceGuard
    {
    public:
        ResourceGuard(VM& vm, UInt16 osn, UInt16 rn);
        ~ResourceGuard();

    public:
        UPInt GetScopeStackBaseInd() const
        {
            return ScopeStackBaseInd;
        }

    private:
        ResourceGuard& operator=(const ResourceGuard&);

    private:
        const UInt16                RegNum;
        SF_DEBUG_CODE(UInt16        PrevReservedNum;)
        UPInt                       ScopeStackBaseInd;
        VM&                         VMRef;
        Value*                      PrevFirstStackPos;
        // Default XML Namespace.
        SPtr<Instances::fl::Namespace>  DefXMLNamespace;
    };

    friend class ResourceGuard; 

public:
    // AOT-specific methods.

    void aot_throw();
    void aot_dxns(ResourceGuard& cf, Instances::fl::Namespace& ns);
    void aot_newclass(Class& cl);

    //
    Value aot_pop()
    {
        return Value(OpStack);
    }

    //
    SInt32 aot_popint()
    {
        return OpStack.PopPrimitive<SInt32>();
    }

    //
    template <typename T>
    void aot_pushop(Pickable<T> p)
    {
        OpStack.PushBack(p);
    }
    void aot_pushop(Object& v)
    {
        OpStack.PushBack(Value(&v));
    }
    void aot_pushop(Class& v)
    {
        OpStack.PushBack(Value(&v));
    }
    void aot_pushop(const Value& v)
    {
        OpStack.PushBack(v);
    }

    //
    template <typename T>
    void aot_setop(Pickable<T> p)
    {
        OpStack.Top0().Assign(p);
    }
    void aot_setop(Object& v)
    {
        OpStack.Top0().Assign(Value(&v));
    }
    void aot_setop(Class& v)
    {
        OpStack.Top0().Assign(Value(&v));
    }
    void aot_setop(const Value& v)
    {
        OpStack.Top0().Assign(v);
    }

    template <typename T>
    void aot_setlocal(UInt32 i, const T& v)
    {
        SetRegister(AbsoluteIndex(i), Value(v));
    }
    void aot_setlocal(UInt32 i, const Value& v)
    {
        SetRegister(AbsoluteIndex(i), v);
    }

    //
    void aot_getarg(const Value& a)
    {
        OpStack.PushBack(a);
    }

    //
    bool aot_nlt();
    bool aot_nlt_ti();
    bool aot_nlt_td();

    bool aot_nle();
    bool aot_nle_ti();
    bool aot_nle_td();

    bool aot_ngt();
    bool aot_ngt_ti();
    bool aot_ngt_td();

    bool aot_nge();
    bool aot_nge_ti();
    bool aot_nge_td();

    bool aot_true();
    bool aot_true_tb();

    bool aot_false();
    bool aot_false_tb();

    bool aot_eq();
    bool aot_eq_ti();
    bool aot_eq_td();

    bool aot_ne();
    bool aot_ne_ti();
    bool aot_ne_td();

    bool aot_lt();
    bool aot_lt_ti();
    bool aot_lt_td();

    bool aot_le();
    bool aot_le_ti();
    bool aot_le_td();

    bool aot_gt();
    bool aot_gt_ti();
    bool aot_gt_td();

    bool aot_ge();
    bool aot_ge_ti();
    bool aot_ge_td();

    bool aot_stricteq();
    bool aot_strictne();

    void aot_getglobalslot(Instances::fl::GlobalObject& go, UInt32 slot_index);
    void aot_setglobalslot(Instances::fl::GlobalObject& go, UInt32 slot_index);

public:
    SPtr<VMAbcFile> LoadFile(const Ptr<Abc::File>& file, VMAppDomain& appDomain, bool to_execute = false);

public:
    // GetRegisteredClassTraits() returns a hashed value if any.
    const ClassTraits::Traits* GetRegisteredClassTraits(const Multiname& mn, VMAppDomain& appDomain);
    const ClassTraits::Traits* GetRegisteredClassTraits(const ASString& name, const Instances::fl::Namespace& ns, VMAppDomain& appDomain);

    // Classes having TypeInfo always belong to the System Application Domain.
    const ClassTraits::Traits* Resolve2ClassTraits(const TypeInfo& ti);
    const ClassTraits::Traits* Resolve2ClassTraits(const Multiname& mn, VMAppDomain& appDomain);
    const ClassTraits::Traits* Resolve2ClassTraits(const ASString& name, const Instances::fl::Namespace& ns, VMAppDomain& appDomain);
    const ClassTraits::Traits* Resolve2ClassTraits(VMFile& file, const Abc::Multiname& mn);

    void RegisterClassTraits(const ASString& name, const Instances::fl::Namespace& ns, ClassTraits::Traits& ctr, VMAppDomain& appDomain)
    {
        appDomain.AddClassTrait(name, ns, &ctr);
    }

    void RegisterGlobalObject(Instances::fl::GlobalObject& go)
    {
        GlobalObjects.PushBack(&go);
    }
    void UnregisterGlobalObject(Instances::fl::GlobalObject& go);

    ASString GetAliasByClass(Class& cl) const;
    Class* GetClassByAlias(const ASString& alias) const;
    void SetClassAlias(const ASString& alias, Class& cl);

public:
    // Utility method.
    // Not const because of GetStringManager().
    // Non-const because of Resolve2Class().
    bool IsOfType(const Value& v, const char* type_name, VMAppDomain& appDomain);
    // Handles primitive values correctly.
    bool IsOfType(const Value& v, const ClassTraits::Traits& ctr) const;

public:
    ///////////////////////////////////////////////////////////////////////////
    class Error
    {
    public:
        Error(VM::ErrorID id, VM& vm);

#ifdef GFX_AS3_VERBOSE
        // 1 arg.
        Error(VM::ErrorID id, VM& vm, int arg1);
        Error(VM::ErrorID id, VM& vm, const StringDataPtr arg1);
        Error(VM::ErrorID id, VM& vm, const Value& arg1);

        // 2 args.
        Error(VM::ErrorID id, VM& vm, int arg1, int arg2);
        Error(VM::ErrorID id, VM& vm, const StringDataPtr arg1, const StringDataPtr arg2);
        Error(VM::ErrorID id, VM& vm, const Value& arg1, const Value& arg2);
        Error(VM::ErrorID id, VM& vm, const Value& arg1, const StringDataPtr arg2);
        Error(VM::ErrorID id, VM& vm, const StringDataPtr arg1, const Value& arg2);

        // 4 args.
        Error(VM::ErrorID id, VM& vm, const StringDataPtr arg1, int arg2, int arg3, int arg4);
#endif

    public:
        const ASString& GetMessage() const { return Message; }
        VM::ErrorID GetErrorID() const { return ID; }

    private:
        VM::ErrorID ID;
        ASString Message;
    };

private:
    // By default execute only one top stack frame (including all function calls).
    // Return number of stack frames left to execute. 
    // Return ZERO if execution is finished completely.
    // !!! It is dangerous to expose this method to users.
    int ExecuteCode(unsigned max_stack_depth = 1);

    bool HasFinished() const
    {
        return CallStack.GetSize() == 0;
    }

    // saved_scope shouldn't be *const*.
    SF_INLINE
    void AddFrame(
        const Value& invoker, // invoker can be NULL if there is nothing to track.
        VMAbcFile& file,
        Abc::MbiInd mbi_ind, // Method body info index.
        const Value& _this,
        int arg_count, 
        const Value* args,
        bool discard_result,
#ifdef SF_AS3_ENABLE_EXPLICIT_GO
        Instances::fl::GlobalObjectScript& gos,
#endif
        const ScopeStackType& saved_scope,
        const Traits& ot // Origination Traits
        SF_DEBUG_ARG(const ASString& name)
        )
    {
        if (CallStack.GetSize() == MaxCallStackSize)
            return ThrowError(Error(eStackOverflowError, *this));
        
#ifdef SF_AS3_ENABLE_CALLFRAME_CACHE
        CallFrame* cf = CFCache.MakeCallFrame();
        cf->Setup(
            invoker,
            file,
            mbi_ind,
            saved_scope, 
            discard_result,
            ot
            SF_DEBUG_ARG(name)
            SF_DEBUG_ARG(Timer::GetProfileTicks())
            );

        cf->SetupRegisters(file.GetMethodBody().Get(mbi_ind).GetMethodInfo(file.GetMethods()), _this, arg_count, args);

        if (IsException())
            return;

#ifdef SF_AMP_SERVER
        if (GetAdvanceStats() != NULL && AmpServer::GetInstance().GetProfileLevel() >= Amp_Profile_Level_ActionScript)
        {
            if (AmpServer::GetInstance().IsProfiling())
            {                
                UInt32 fileId = cf->GetFile().GetAbcFile().GetFileHandle() + (cf->GetMethodBodyInfo().GetMethodInfoInd() << 16);
                if (!cf->IsRegisteredFunction())
                {
                    cf->SetRegisteredFunction(true);
                    GetAdvanceStats()->RegisterScriptFunction(fileId, cf->GetFile().GetAbcFile().GetSwfFileOffset(), cf->Name->pData, 0, 3, false);
                }
                GetAdvanceStats()->PushCallstack(fileId, cf->GetFile().GetAbcFile().GetSwfFileOffset(), cf->GetStartTicks());
            }
        }
#endif
        CallStack.PushBack(cf);

#else

        CallFrame cf(
            invoker,
            file,
            mbi_ind,
            discard_result,
#ifdef SF_AS3_ENABLE_EXPLICIT_GO
            gos,
#endif
            saved_scope, 
            ot
            SF_DEBUG_ARG(name)
            SF_DEBUG_ARG(Timer::GetProfileTicks())
            );

        const Abc::MethodInfo& mi = file.GetMethodBody().Get(mbi_ind).GetMethodInfo(file.GetMethods());
        cf.SetupRegisters(mi, _this, arg_count, args);

        if (IsException())
        {
            // Magic logic.
            // It is necessary to set the ACopy flag to true to force destructor of CallFrame to release
            // reserved resources.
            cf.ACopy = true;
            return;
        }

#ifdef SF_AMP_SERVER
        if (GetAdvanceStats() != NULL && AmpServer::GetInstance().GetProfileLevel() >= Amp_Profile_Level_ActionScript)
        {
            if (AmpServer::GetInstance().IsProfiling())
            {                
                UInt32 fileId = cf.GetFile().GetAbcFile().GetFileHandle() + (cf.GetMethodBodyInfo().GetMethodInfoInd().Get() << 16);
                if (!cf.IsRegisteredFunction())
                {
                    cf.SetRegisteredFunction(true);
                    GetAdvanceStats()->RegisterScriptFunction(fileId, cf.GetFile().GetAbcFile().GetSwfFileOffset(), cf.Name->pData, 0, 3, false);
                }
                GetAdvanceStats()->PushCallstack(fileId, cf.GetFile().GetAbcFile().GetSwfFileOffset(), cf.GetStartTicks());
            }
        }
#endif
        CallStack.PushBack(cf);
#endif
    }
    
public:
    Pickable<Instances::fl::Object>             MakeObject() const;
    Pickable<Instances::fl::Array>              MakeArray() const;
    Pickable<Instances::fl_utils::ByteArray>    MakeByteArray() const;

public:
    // typedef ValueArray OpStackType;
    typedef ValueStack OpStackType;

#ifdef SF_AS3_ENABLE_CALLFRAME_CACHE
    typedef ArrayDH_POD<CallFrame*, StatMV_VM_VM_Mem> CallStackType;
#else
    typedef ArrayPagedCC<CallFrame, 6, 64, StatMV_VM_VM_Mem> CallStackType;
#endif

public:
    // Return Traits for a given value.

    const ClassTraits::Traits& GetClassTraits(const Value& v) const;
    InstanceTraits::Traits& GetInstanceTraits(const Value& v) const;
    Traits& GetValueTraits(const Value& v) const;

public:
    StringManager& GetStringManager() const
    {
        return StringManagerRef;
    }

    // This is a wrapper, which will/should call "toString"
    ASString AsString(const Value& value);
    
    // Needed only to display OpStack ...
    const OpStackType& GetOpStack() const
    {
        return OpStack;
    }
    UPInt GetScopeStackBaseInd() const
    {
        if (CallStack.GetSize() == 0)
            return 0;
        
        return GetCurrCallFrame().GetScopeStackBaseInd();
    }
    UPInt GetOpStackBaseInd() const
    {
        if (CallStack.GetSize() == 0)
            return 0;

        return GetCurrCallFrame().GetScopeStackBaseInd();
    }
    const CallFrame& GetCurrCallFrame() const
    {
        SF_ASSERT(CallStack.GetSize() > 0);
#ifdef SF_AS3_ENABLE_CALLFRAME_CACHE
        return *CallStack.Back();
#else
        return CallStack.Back();
#endif
    }
    const Value& GetThis() const
    {
        return GetCurrCallFrame().GetThis();
    }
    const CallStackType& GetCallStack() const
    {
        return CallStack;
    }
    bool IsInitialized() const
    {
        return Initialized;
    }

    void GetStackTraceASString(ASString& result, const char* line_pref = "\t") const;
    // Retrieve name of a file, which contains currently called AS3 function.
    void GetCodeFileName(ASString& result) const;

    ASString describeTypeEx(const Value& value) const;

public:
    //////
    ASString GetASString(const char *pstr) const
    {
        return GetStringManager().CreateString(pstr);
    }
    ASString GetASString(const char *pstr, UPInt length) const
    {
        return GetStringManager().CreateString(pstr, length);
    }
    ASString GetASString(const String& str) const
    {
        return GetStringManager().CreateString(str);
    }

public:
    const RegistersType& GetRegisters() const
    {
        return RegisterFile;
    }

    const ScopeStackType& GetScopeStack() const
    {
        return ScopeStack;
    }

    // Register's index starts with ZERO ...
    const Value& GetRegister(AbsoluteIndex ind) const
    {
        return RegisterFile[ind.Get()];
    }
    
private:
    RegistersType& GetRegisters()
    {
        return RegisterFile;
    }

    ScopeStackType& GetScopeStack()
    {
        return ScopeStack;
    }

    SF_INLINE
    Value& GetRegister(AbsoluteIndex ind)
    {
        return RegisterFile[ind.Get()];
    }

    SF_INLINE
    void SetRegister(AbsoluteIndex ind, const Value& value)
    {
        RegisterFile[ind.Get()] = value;
    }
    SF_INLINE
    void SetRegisterPick(AbsoluteIndex ind, Value& value)
    {
        RegisterFile[ind.Get()].Pick(value);
    }
    SF_INLINE
    void SetRegisterPickUnsafe(AbsoluteIndex ind, Value& value)
    {
        RegisterFile[ind.Get()].PickUnsafe(value);
    }
    SF_INLINE
    void SwapRegister(AbsoluteIndex ind, Value& value)
    {
        RegisterFile[ind.Get()].Swap(value);
    }
    
public:
    // this method is not private to let raw function trace their results.
    FlashUI& GetUI() const
    {
        return UI;
    }
    
    //
    ASRefCountCollector& GetGC() const
    {
        return GC.GetGC();
    }

    //
    const TGlobalObjects& GetGlobalObjects() const
    {
        return GlobalObjects;
    }

    MemoryHeap* GetMemoryHeap() const
    {
        return MHeap;
    }

    bool IsInAS3VMDestructor() const
    {
        return InDestructor;
    }

public:
    //
    bool IsClassClass(const ClassTraits::Traits& c) const
    {
        return &c == &GetClassTraitsClassClass();
    }

    //
    bool IsObject(const ClassTraits::Traits& c) const
    {
        return &c == &GetClassTraitsObject();
    }

public:
	// Raw access to classes for performance reason.

	// Class.
	Classes::ClassClass& GetClassClass() const
	{
		SF_ASSERT(TraitsClassClass.GetPtr());
        return (Classes::ClassClass&)TraitsClassClass->GetInstanceTraits().GetClass();
	}
    const ClassTraits::ClassClass& GetClassTraitsClassClass() const
    {
        SF_ASSERT(TraitsClassClass.GetPtr());
        return *TraitsClassClass;
    }
    InstanceTraits::Traits& GetITraitsClass() const
    {
        return GetClassTraitsClassClass().GetInstanceTraits();
    }

	// Object.
	Classes::fl::Object& GetClassObject() const
	{
        SF_ASSERT(TraitsObject.GetPtr());
        return (Classes::fl::Object&)TraitsObject->GetInstanceTraits().GetClass();
	}
    ClassTraits::fl::Object& GetClassTraitsObject() const
    {
        SF_ASSERT(TraitsObject.GetPtr());
        return *TraitsObject;
    }
    InstanceTraits::Traits& GetITraitsObject() const
    {
        return GetClassTraitsObject().GetInstanceTraits();
    }

	// Namespace.
	Classes::fl::Namespace& GetClassNamespace() const
	{
        SF_ASSERT(TraitsNamespace.GetPtr());
        return (Classes::fl::Namespace&)TraitsNamespace->GetInstanceTraits().GetClass();
	}
    const ClassTraits::fl::Namespace& GetClassTraitsNamespace() const
    {
        SF_ASSERT(TraitsNamespace.GetPtr());
        return *TraitsNamespace;
    }
    InstanceTraits::Traits& GetITraitsNamespace() const
    {
        return GetClassTraitsNamespace().GetInstanceTraits();
    }

	// Function.
	Classes::Function& GetClassFunction() const
	{
        SF_ASSERT(TraitsFunction.GetPtr());
        return (Classes::Function&)TraitsFunction->GetInstanceTraits().GetClass();
	}
    const ClassTraits::Function& GetClassTraitsFunction() const
    {
        SF_ASSERT(TraitsFunction.GetPtr());
        return *TraitsFunction;
    }
    InstanceTraits::Traits& GetITraitsFunction() const
    {
        return GetClassTraitsFunction().GetInstanceTraits();
    }

	// Boolean.
    Classes::fl::Boolean& GetClassBoolean() const;
    const ClassTraits::fl::Boolean& GetClassTraitsBoolean() const
    {
        SF_ASSERT(TraitsBoolean.GetPtr());
        return *TraitsBoolean;
    }
    InstanceTraits::Traits& GetITraitsBoolean() const;

	// Number.
	Classes::fl::Number& GetClassNumber() const;
    const ClassTraits::fl::Number& GetClassTraitsNumber() const
    {
        SF_ASSERT(TraitsNumber.GetPtr());
        return *TraitsNumber;
    }
    InstanceTraits::Traits& GetITraitsNumber() const;

	// int.
	Classes::fl::int_& GetClassSInt() const;
    const ClassTraits::fl::int_& GetClassTraitsSInt() const
    {
        SF_ASSERT(TraitsInt.GetPtr());
        return *TraitsInt;
    }
    InstanceTraits::Traits& GetITraitsSInt() const;

	// uint.
	Classes::fl::uint& GetClassUInt() const;
    const ClassTraits::fl::uint& GetClassTraitsUInt() const
    {
        SF_ASSERT(TraitsUint.GetPtr());
        return *TraitsUint;
    }
    InstanceTraits::Traits& GetITraitsUInt() const;

	// String.
	Classes::fl::String& GetClassString() const;
    const ClassTraits::fl::String& GetClassTraitsString() const
    {
        SF_ASSERT(TraitsString.GetPtr());
        return *TraitsString;
    }
    InstanceTraits::Traits& GetITraitsString() const;

	// Array.
	Classes::fl::Array& GetClassArray() const;
    const ClassTraits::fl::Array& GetClassTraitsArray() const
    {
        SF_ASSERT(TraitsArray.GetPtr());
        return *TraitsArray;
    }
    InstanceTraits::Traits& GetITraitsArray() const;

    // ByteArray.
    Classes::fl_utils::ByteArray& GetClassByteArray() const;
    const ClassTraits::fl_utils::ByteArray& GetClassTraitsByteArray() const
    {
        SF_ASSERT(TraitsByteArray.GetPtr());
        return *TraitsByteArray;
    }
    InstanceTraits::Traits& GetITraitsByteArray() const;

    // QName.
    Classes::fl::QName& GetClassQName() const;
    const ClassTraits::fl::QName& GetClassTraitsQName() const
    {
        SF_ASSERT(TraitsQName.GetPtr());
        return *TraitsQName;
    }

    // Catch.
    const ClassTraits::fl::Catch& GetClassTraitsCatch() const
    {
        SF_ASSERT(TraitsCatch.GetPtr());
        return *TraitsCatch;
    }

    // Vector.
    Classes::fl_vec::Vector& GetClassVector() const;
    const ClassTraits::fl_vec::Vector& GetClassTraitsVector() const
    {
        SF_ASSERT(TraitsVector.GetPtr());
        return *TraitsVector;
    }

    // Vector$int.
    Classes::fl_vec::Vector_int& GetClassVectorSInt() const;
    const ClassTraits::fl_vec::Vector_int& GetClassTraitsVectorSInt() const
    {
        SF_ASSERT(TraitsVector_int.GetPtr());
        return *TraitsVector_int;
    }
    InstanceTraits::Traits& GetITraitsVectorSInt() const;

    // Vector$uint.
    Classes::fl_vec::Vector_uint& GetClassVectorUInt() const;
    const ClassTraits::fl_vec::Vector_uint& GetClassTraitsVectorUInt() const
    {
        SF_ASSERT(TraitsVector_uint.GetPtr());
        return *TraitsVector_uint;
    }
    InstanceTraits::Traits& GetITraitsVectorUInt() const;

    // Vector$double.
    Classes::fl_vec::Vector_double& GetClassVectorNumber() const;
    const ClassTraits::fl_vec::Vector_double& GetClassTraitsVectorNumber() const
    {
        SF_ASSERT(TraitsVector_Number.GetPtr());
        return *TraitsVector_Number;
    }
    InstanceTraits::Traits& GetITraitsVectorNumber() const;

    // Vector$String.
    Classes::fl_vec::Vector_String& GetClassVectorString() const;
    const ClassTraits::fl_vec::Vector_String& GetClassTraitsVectorString() const
    {
        SF_ASSERT(TraitsVector_String.GetPtr());
        return *TraitsVector_String;
    }
    InstanceTraits::Traits& GetITraitsVectorString() const;

    // Vector$Object.
//     Classes::Vector_object& GetClassVectorObject() const
//     {
//         SF_ASSERT(TraitsVector_Object.GetPtr());
//         return *TraitsVector_Object;
//     }

    const ClassTraits::fl_system::ApplicationDomain& GetClassApplicationDomain() const
    {
        return *TraitsApplicationDomain;
    }
    InstanceTraits::Traits& GetITraitsApplicationDomain() const;

    const ClassTraits::fl::Domain& GetClassDomain() const
    {
        return *TraitsDomain;
    }
    InstanceTraits::Traits& GetITraitsDomain() const;

    InstanceTraits::Traits& GetITraitsNull() const
    {
        SF_ASSERT(TraitsNull.GetPtr());
        return *TraitsNull;
    }

    InstanceTraits::Traits& GetITraitsVoid() const
    {
        SF_ASSERT(TraitsVoid.GetPtr());
        return *TraitsVoid;
    }

public:
    static bool IsFixedNumType(const Traits& tr)
    {
        const BuiltinTraitsType tt = tr.GetTraitsType();
        return tt == Traits_SInt || tt == Traits_UInt;
    }
    static bool IsNumericType(const Traits& tr)
    {
        const BuiltinTraitsType tt = tr.GetTraitsType();
        return tt >= Traits_SInt && tt <= Traits_Number;
    }
    static bool IsPrimitiveType(const Traits& tr)
    {
        const BuiltinTraitsType tt = tr.GetTraitsType();
        return tt >= Traits_Boolean && tt <= Traits_String;
    }

    bool IsVMCachedType(const InstanceTraits::Traits& tr) const;
    bool IsVMCachedType(const ClassTraits::Traits& tr) const;
    bool IsVMCachedType(const Traits& tr) const;

private:
    // Return default value for a given class.
    Value GetDefaultValue(const ClassTraits::Traits& ctr) const;
    // Return default value for a given multiname.
    Value GetDefaultValue(VMAbcFile& file, const Abc::Multiname& mn);

    InstanceTraits::Traits& GetFunctReturnType(const ThunkInfo& thunk, VMAppDomain& appDomain);
    InstanceTraits::Traits& GetFunctReturnType(const Value& value, VMAppDomain& appDomain);

public:
    // Exceptions ...
    void Throw(const Value& e)
    {
        HandleException = true;
        ExceptionObj = e;
    }

    // Outputs the error to log.
    void OutputError(const Value& e);

    void ThrowError(const Error& e);
    void ThrowEvalError(const Error& e);
    void ThrowRangeError(const Error& e);
    void ThrowReferenceError(const Error& e);
    void ThrowSecurityError(const Error& e);
    void ThrowSyntaxError(const Error& e);
    void ThrowTypeError(const Error& e);
    void ThrowURIError(const Error& e);
    void ThrowVerifyError(const Error& e);
    void ThrowUninitializedError(const Error& e);
    void ThrowArgumentError(const Error& e);
    void ThrowMemoryError(const Error& e);

    bool IsException() const
    {
        return HandleException;
    }
    const Value& GetExceptionValue() const
    {
        return ExceptionObj;
    }
    void IgnoreException()
    {
        SF_ASSERT(IsException());
        HandleException = false;
    }

    void OutputAndIgnoreException()
    {   // Ignore before Output so that toString() succeeds.
        IgnoreException();
        OutputError(ExceptionObj);
        ExceptionObj.SetUndefined();
    }

    
public:
    // Namespace related methods.
    
    Pickable<Instances::fl::Namespace> MakeNamespace(Abc::NamespaceKind kind)
    {
        return MakeNamespace(kind, GetStringManager().CreateEmptyString());
    }
    Pickable<Instances::fl::Namespace> MakeNamespace(Abc::NamespaceKind kind, const ASString& uri,
                                        const Value& prefix = Value::GetUndefined()) const;
    
    Pickable<Instances::fl::Namespace> MakeInternedNamespace(Abc::NamespaceKind kind, const char* name) const;
    Pickable<Instances::fl::Namespace> MakeInternedNamespace(Abc::NamespaceKind kind, const StringDataPtr& uri) const;
    Pickable<Instances::fl::Namespace> MakeInternedNamespace(Abc::NamespaceKind kind, const ASString& uri) const;               
    Pickable<Instances::fl::Namespace> MakeInternedNamespace(const Abc::NamespaceInfo& ni) const
    {
        return MakeInternedNamespace(ni.GetKind(), ni.GetNameURI());
    }
    Pickable<Instances::fl::Namespace> MakeInternedNamespace(const AOT::NamespaceInfo& ni) const;

    Instances::fl::Namespace& GetInternedNamespace(Abc::NamespaceKind kind, const char* name) const;
    Instances::fl::Namespace& GetInternedNamespace(Abc::NamespaceKind kind, const ASString& uri) const;               
    Instances::fl::Namespace& GetInternedNamespace(Abc::NamespaceKind kind, const StringDataPtr& uri) const;
    Instances::fl::Namespace& GetInternedNamespace(const Abc::NamespaceInfo& ni) const
    {
        return GetInternedNamespace(ni.GetKind(), ni.GetNameURI());
    }

    Instances::fl::Namespace& GetPublicNamespace() const
    {
        SF_ASSERT(PublicNamespace.GetPtr());
        return *PublicNamespace;
    }
    Pickable<Instances::fl::Namespace> MakePublicNamespace() const
    {
        SF_ASSERT(PublicNamespace.GetPtr());
        return Pickable<Instances::fl::Namespace>(PublicNamespace.GetPtr(), PickValue);
    }
    Instances::fl::Namespace& GetAS3Namespace() const
    {
        SF_ASSERT(AS3Namespace.GetPtr());
        return *AS3Namespace;
    }
    Instances::fl::Namespace& GetVectorNamespace() const
    {
        SF_ASSERT(VectorNamespace.GetPtr());
        return *VectorNamespace;
    }
    Instances::fl::Namespace& GetProxyNamespace() const
    {
        SF_ASSERT(ProxyNamespace.GetPtr());
        return *ProxyNamespace;
    }
    Instances::fl::Namespace& GetXMLNamespace() const
    {
        SF_ASSERT(XMLNamespace.GetPtr());
        return *XMLNamespace;
    }

    // If Default Namespace is NULL,then it is a public namespace.
    Instances::fl::Namespace* GetDefXMLNamespace() const
    {
#if 0

#ifdef GFX_ENABLE_XML
        return GetCurrCallFrame().GetDefXMLNamespace();
#else
        return NULL;
#endif

#else
        return DefXMLNamespace;
#endif
    }

    void SetDefXMLNamespace(const SPtr<Instances::fl::Namespace>& ns)
    {
        DefXMLNamespace = ns;
    }
    
public:
    // Get current ActionScript Global Object.
    Instances::fl::GlobalObjectScript* GetGlobalObject() const;
    const Instances::fl::GlobalObjectCPP& GetGlobalObjectCPP() const
    {
        return *GlobalObject;
    }

    // Get Class using qualified name gname.
    // Classes will be initialized on demand.
    // Return NULL in case of exception or if a class cannot be found.
    Class* GetClass(const StringDataPtr& gname, VMAppDomain& appDomain);
    // Return false in case of exception or if a class cannot be found.
    bool GetClassUnsafe(const StringDataPtr& gname, VMAppDomain& appDomain, Value& result);

    // Call Construct() and execute code if necessary.
    CheckResult CreateObject(const char* gname, VMAppDomain& appDomain, Value& result, unsigned argc = 0, const Value* argv = NULL)
    {
        const bool need2execute = Construct(gname, appDomain, result, argc, argv, false); 

        if (IsException()) 
            return false;

        if (need2execute)
            ExecuteCode();

        return !IsException();
    }

    // Create an instance of a class with the qualified name gname.
    // Constructs a builtin class such as "flash.display.Graphics" and fills
    // in a SPtr to it. Returns false of constructor failed, which is different from
    // regular Construct since ExecuteCode is not needed for builtins.
    template<class C>
    CheckResult ConstructBuiltinObject(SPtr<C> &pobj, const char* gname,
                                       unsigned argc = 0, const Value* argv = NULL)
    {
        SF_ASSERT(((UPInt)static_cast<Object*>((C*)0)) == 0);
        return constructBuiltinObject(
            reinterpret_cast<SPtr<Object>&>(pobj), gname, argc, argv);
    }

    // Constructs a builtin class such as "flash.display.Graphics" and fills
    // in a Value to it. Returns false of constructor failed, which is different from
    // regular Construct since ExecuteCode is not needed for builtins.
    CheckResult ConstructBuiltinValue(Value& v, const char* gname,
                                      unsigned argc = 0, const Value* argv = NULL)
    {
        Construct(gname, GetCurrentAppDomain(), v, argc, argv);

        return (!IsException() && !v.IsNullOrUndefined());
    }

    CheckResult ConstructBuiltinValue(Value& v, const TypeInfo& ti,
        unsigned argc = 0, const Value* argv = NULL);

    // Register user-defined ClassInfo structures.
    void RegisterClassInfoTable(const ClassInfo* table[]);

private:
    // Returns *true* if ExecuteCode() should be called after this method.
    // Basically, it returns *false* if a constructed object is a pure C++ object.
    // Can throw exceptions.
    // !!! This method shouldn't be exposed to users because it may require ExecuteCode() call.
    bool Construct(const char* gname, VMAppDomain& appDomain, Value& result, unsigned argc = 0, const Value* argv = NULL, 
        bool extCall = true);
    bool Construct(Class& cl, Value& result, unsigned argc = 0, const Value* argv = NULL, bool extCall = true);

    // Private helper to ConstructBuiltinObject
    CheckResult constructBuiltinObject(SPtr<Object> &pobj, const char* gname,
                                       unsigned argc, const Value* argv);    
    void ThrowErrorInternal(const Error& e, const TypeInfo& ti);

public:
    // public for AOTC.
    // Get C++ Global Object, which holds all builtin classes.
    Instances::fl::GlobalObjectCPP& GetGlobalObjectCPP()
    {
        return *GlobalObject;
    }

private:
    const Value& GetGlobalObjectCPPValue() const
    {
        return GlobalObjectValue;
    }
    ASStringNode* CreateConstStringNode(const char* msg);

private:
    bool IsInInitializer() const
    {
        return InInitializer > 0;
    }
    void EnterInitializer()
    {
        ++InInitializer;
    }
    void LeaveInitializer()
    {
        --InInitializer;
    }

private:
    enum State {sExecute, sStepInto, sReturn};

    // Return true if exception handling code is not in this frame.
    bool ProcessException(Abc::TOpCodeVTP& cp, CallFrame& cf, State& state);
    bool NeedToStepInto(UPInt call_stack_size, State& state);
    
private:
    int OnException(Abc::TCodeOffset offset, CallFrame& cf);

	void DropCallFrame();

    SF_INLINE
    void PushResult(Value& value)
    {
        OpStack.PushBack(value);
    }
    SF_INLINE
    void PickPushResult(Value& value)
    {
        OpStack.PickPushBack(value);
    }

    SF_INLINE
    void PickResult(Value& result)
    {
        result.Pick(OpStack);
    }
    SF_INLINE
    void PickResultUnsafe(Value& result)
    {
        result.PickUnsafe(OpStack);
    }

    void ExecuteAndRetrieveResult(Value& result)
    {
        if (IsException())
            return;

        const int stack_depth = ExecuteCode();
        SF_UNUSED(stack_depth);
        SF_ASSERT(stack_depth == 0);

        if (IsException())
            return;

        PickResult(result);
    }

    // Result is on stack.
    SF_INLINE
    void Execute(const Value& func, const Value& _this, unsigned argc, const Value* argv, bool discard_result = false)
    {
        Value result = Value::GetUndefined();

        ExecuteInternalUnsafe(func, _this, result, argc, argv, !discard_result);
    }
    void ExecuteVTableIndUnsafe(UInt32 ind, const Traits& tr, Value& _this, unsigned argc, const Value* argv);
    void ExecuteThunkUnsafe(const ThunkInfo& thunk, const Value& _this, Value& result, unsigned argc, const Value* argv, bool result_on_stack);
    void ExecuteMethodIndUnsafe(const Value& func, const Value& real_func, const Value& _this, Value& result, unsigned argc, const Value* argv, bool result_on_stack SF_DEBUG_ARG(const SInt32 ind));
	void ExecuteInternalUnsafe(const Value& func, const Value& _this, Value& result, unsigned argc, const Value* argv, bool result_on_stack = false);
    // Can throw exceptions.
    // Non-const because of Resolve2Class.
    void Coerce2ReturnType(const Value& value, Value& result);

    void FindProperty(PropRef& result, const Multiname& mn, const ScopeStackType& ss, VMAppDomain& appDomain);

public:
    // Result is returned as an argument.
    SF_INLINE
    void ExecuteUnsafe(const Value& func, const Value& _this,
                       Value& result, unsigned argc, const Value* argv)
    {
        // Retrieve result.
        SF_ASSERT(!result.IsRefCounted());
        ExecuteInternalUnsafe(func, _this, result, argc, argv);
    }
    SF_INLINE
    void Execute(const Value& func, const Value& _this,
                 Value& result, unsigned argc, const Value* argv)
    {
        // Retrieve result.
        Value _result;
        ExecuteInternalUnsafe(func, _this, _result, argc, argv);
        Alg::Swap(_result, result);
    }

    // Execute function.
    SF_INLINE
    void ExecuteFunction(const Value& func, Value& result, unsigned argc, const Value* argv)
    {
        // Retrieve result.
        Value _result;
        ExecuteInternalUnsafe(func, Value::GetUndefined(), _result, argc, argv);
        Alg::Swap(_result, result);
    }
    // Execute function and ignore result.
    SF_INLINE
    void ExecuteFunction(const Value& func, unsigned argc, const Value* argv)
    {
        Value _result;
        ExecuteInternalUnsafe(func, Value::GetUndefined(), _result, argc, argv);
    }

    // Execute call and ignore exception, if any, after tracing it to output.
    // This is a helper method for external use.
    void Execute_IgnoreExceptionUnsafe(const Value& func, const Value& _this,
                                       Value& result, unsigned argc, const Value* argv);

private:
    VM& GetSelf()
    {
        return *this;
    }

private:
    class GCGuard
    {
    public:
        GCGuard(ASRefCountCollector& gc)
            : GC(gc)
        {

        }
        ~GCGuard()
        {
            ForceCollect();
        }

    public:
        ASRefCountCollector& GetGC() const { return GC; }
        void ForceCollect() { GC.ForceCollect(NULL, ASRefCountCollector::Collect_Shutdown); }

    private:
        GCGuard& operator =(const GCGuard&);

    private:
        ASRefCountCollector& GC;
    };

private:
    ClassTraits::Traits* GetClassTraits(const ASString& name, const Instances::fl::Namespace& ns, VMAppDomain& appDomain);
    ClassTraits::UserDefined& GetUserDefinedTraits(VMFile& file, const Abc::ClassInfo& ci);

    // Utility methods.

    void RegisterClassTraits(ClassTraits::Traits& tr, VMAppDomain& appDomain);
    void RegisterClassTraitsVector(ClassTraits::Traits& tr, VMAppDomain& appDomain);

    template <typename T>
    SF_INLINE Pickable<T> PrepareClassTraits(T* p)
    {
        RegisterClassTraits(*p, GetSystemAppDomain());
        return Pickable<T>(p);
    }

public:
    void AddVMAbcFileWeak(VMAbcFile* f);
    bool RemoveVMAbcFileWeak(VMAbcFile* f);
    SPtr<VMAbcFile> FindVMAbcFileWeak(const char* name, VMAppDomain& appDomain) const;
    UPInt GetAllLoadedAbcFiles(Scaleform::Array<SPtr<VMAbcFile> >* destArray) const;

#if defined(SF_OBJ_COLLECTOR_ENABLED)
    virtual void Collect(class ObjectCollector&);
#endif
//protected:
    void UnregisterAllAbcFiles();

public:
     // XML-related.

#ifdef GFX_ENABLE_XML
    void EnableXMLSupport();
#endif

    const XMLSupport& GetXMLSupport() const { return *XMLSupport_; }

public:
    // Application Domain - related.
    VMAppDomain& GetCurrentAppDomain() const
    {
        SF_ASSERT(CurrentDomain);
        return *CurrentDomain;
    }
    VMAppDomain& GetSystemAppDomain() const
    {
        SF_ASSERT(SystemDomain);
        return *SystemDomain;
    }
    VMAppDomain& GetFrameAppDomain() const;
    bool RemoveAppDomain(VMAppDomain& appDomain);

private:
    typedef HashUncachedLH<ASString, Class*, ASStringNodeHashFunc, StatMV_VM_VM_Mem> AliasClassType;
    typedef HashLH<Class*, ASString, FixedSizeHash<Class*>, StatMV_VM_VM_Mem> ClassAliasType;

    //enum {MaxCallStackSize = 1024};
    enum {MaxCallStackSize = 128};

    bool                Initialized;
    bool                InDestructor;
    bool                LoadingAbcFile;
    AS3::StringManager& StringManagerRef;
    // Garbage Collector;
    GCGuard             GC;

    FlashUI&            UI;
    int                 InInitializer;
    MemoryHeap* const   MHeap;
    SPtr<XMLSupport>    XMLSupport_;

    OpStackType         OpStack;
    RegistersType       RegisterFile;
    ScopeStackType      ScopeStack;
    
    // Exception-handling related ...
    bool                HandleException;
    Value               ExceptionObj;

    TGlobalObjects      GlobalObjects;

#ifdef SF_AS3_ENABLE_CALLFRAME_CACHE
    CallFrameCache      CFCache;
#endif

    // CallStack has to be declared after all other stacks because
    // CallFrame depends on them.
    CallStackType       CallStack;

    SF_AMP_CODE(UInt64 ActiveLineTimestamp;)
    SF_AMP_CODE(void SetActiveLine(UInt32 lineNumber);)
    SF_AMP_CODE(void SetActiveFile(UInt64 fileId);)

    ///////////////////////////////////////////////////////////////////////
    SPtr<VMAppDomain> SystemDomain;
    SPtr<VMAppDomain> CurrentDomain;

#ifndef SF_AS3_CLASS_AS_SLOT
    // GenericClasses holds strong references to "Generic Classes".
    // At this time we have only Vector.<> as a "Generic Class".
    ArrayLH<SPtr<ClassTraits::Traits>, StatMV_VM_VM_Mem>    GenericClasses;
#endif

    // Public namespace.
    SPtr<Instances::fl::Namespace>          PublicNamespace;
    SPtr<Instances::fl::Namespace>          AS3Namespace;
    SPtr<Instances::fl::Namespace>          VectorNamespace;
    SPtr<Instances::fl::Namespace>          ProxyNamespace;
    // Namespace with URI "http://www.w3.org/XML/1998/namespace".
    SPtr<Instances::fl::Namespace>          XMLNamespace;
    
    SPtr<ClassTraits::ClassClass>               TraitsClassClass;
    SPtr<ClassTraits::fl::Object>               TraitsObject;
    SPtr<ClassTraits::fl::Namespace>            TraitsNamespace;
    SPtr<ClassTraits::Function>                 TraitsFunction;
    SPtr<ClassTraits::fl::Boolean>              TraitsBoolean;
    SPtr<ClassTraits::fl::Number>               TraitsNumber;
    SPtr<ClassTraits::fl::int_>                 TraitsInt;
    SPtr<ClassTraits::fl::uint>                 TraitsUint;
    SPtr<ClassTraits::fl::String>               TraitsString;
    SPtr<ClassTraits::fl::Array>                TraitsArray;
    SPtr<ClassTraits::fl::QName>                TraitsQName;
    SPtr<ClassTraits::fl::Catch>                TraitsCatch;
    SPtr<ClassTraits::fl_vec::Vector>           TraitsVector;
    SPtr<ClassTraits::fl_vec::Vector_int>       TraitsVector_int;
    SPtr<ClassTraits::fl_vec::Vector_uint>      TraitsVector_uint;
    SPtr<ClassTraits::fl_vec::Vector_double>    TraitsVector_Number;
    SPtr<ClassTraits::fl_vec::Vector_String>    TraitsVector_String;
    SPtr<ClassTraits::fl_system::ApplicationDomain>    TraitsApplicationDomain;
    SPtr<ClassTraits::fl::Domain>               TraitsDomain;

    SPtr<InstanceTraits::Traits>        TraitsNull;
    SPtr<InstanceTraits::Traits>        TraitsVoid;

    SPtr<InstanceTraits::Function>      NoFunctionTraits;

    // Default XML Namespace.
    SPtr<Instances::fl::Namespace>      DefXMLNamespace;
    
    // CPP GlobalObject ...
    SPtr<InstanceTraits::fl::GlobalObject>  TraitaGlobalObject;
    SPtr<Instances::fl::GlobalObjectCPP>    GlobalObject;
    // This is the same GlobalObject. We just need it for convenience to return
    // in GetGlobalObject().
    Value                                   GlobalObjectValue;

    // TraitsByteArray HAS TO follow GlobalObject because it implements interfaces.
    SPtr<ClassTraits::fl_utils::ByteArray>  TraitsByteArray;

    AliasClassType                              AliasClass;
    ClassAliasType                              ClassAlias;
    ArrayLH_POD<VMAbcFile*, StatMV_VM_VM_Mem>   VMAbcFilesWeak;

#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
    AOT::InfoCollector* pIC;
#endif
}; // class VM

///////////////////////////////////////////////////////////////////////////
class InitializerGuard
{
public:
    InitializerGuard(VM& vm)
        : Resource(vm)
    {
        Resource.EnterInitializer();
    }
    ~InitializerGuard()
    {
        Resource.LeaveInitializer();
    }

private:
    InitializerGuard& operator =(const InitializerGuard&);

private:
    VM& Resource;
};

///////////////////////////////////////////////////////////////////////////
#if defined(SF_FPE_ENABLE)
class FPEGuard
{
public:
    FPEGuard();
    ~FPEGuard();

private:
#if defined(SF_OS_WIN32) || defined(SF_OS_XBOX360)
    unsigned State;
#endif
};
#endif


///////////////////////////////////////////////////////////////////////////
inline
XMLSupport::XMLSupport(VM& vm, bool enabled) : GASRefCountBase(&vm.GetGC()), Enabled(enabled)
{
}

///////////////////////////////////////////////////////////////////////////
inline
ASString Object::GetASString(const char *pstr) const
{
    return GetVM().GetStringManager().CreateString(pstr);
}

inline
ASString Object::GetASString(const char *pstr, UPInt length) const
{
    return GetVM().GetStringManager().CreateString(pstr, length);
}

inline
ASString Object::GetASString(const String& str) const
{
    return GetVM().GetStringManager().CreateString(str);
}

SF_INLINE
bool Object::IsException() const
{
    return GetVM().IsException();
}

SF_INLINE
StringManager& Object::GetStringManager() const
{
    return GetVM().GetStringManager();
}

SF_INLINE
Instances::fl::GlobalObjectCPP& Object::GetGlobalObjectCPP() const
{
    return GetVM().GetGlobalObjectCPP();
}

inline
CheckResult Object::GetSlotValueUnsafe(const ASString& prop_name, Instances::fl::Namespace& ns, Value& value)
{
    PropRef prop;

    AS3::FindObjProperty(prop, GetVM(), Value(this), Multiname(ns, Value(prop_name)));

    return prop && prop.GetSlotValueUnsafe(GetVM(), value);
}

inline
void Object::AddDynamicFunc(const ThunkInfo& func)
{
    // We do not need namespace here. 
    // It is always public for dynamic properties.

    // Can we use Getters/Setters as dynamic properties?
    SF_ASSERT(func.GetCodeType() == CT_Method);
    SF_ASSERT(func.NamespaceName == NULL || *func.NamespaceName == 0);

    AddDynamicSlotValuePair(
        GetStringManager().CreateConstString(func.Name), 
        func, 
        SlotInfo::aDontEnum
        );
}

inline
Value& Object::GetRegister(AbsoluteIndex ind)
{
    return GetVM().GetRegister(ind);
}

///////////////////////////////////////////////////////////////////////////
SF_INLINE
StringManager& Traits::GetStringManager() const
{
    return GetVM().GetStringManager();
}

inline
ASString Traits::GetASString(const char *pstr) const
{
    return GetVM().GetStringManager().CreateString(pstr);
}

inline
ASString Traits::GetASString(const char *pstr, UPInt length) const
{
    return GetVM().GetStringManager().CreateString(pstr, length);
}

inline
ASString Traits::GetASString(const String& str) const
{
    return GetVM().GetStringManager().CreateString(str);
}

inline
const SlotInfo* Traits::FindSlotInfo(VMAbcFile& file, const Abc::Multiname& mn) const 
{
    const Instances::fl::Namespace& ns = GetVM().GetInternedNamespace(mn.GetNamespace(file.GetConstPool()));
    return Slots::FindSlotInfo(file.GetInternedString(mn.GetNameInd()), ns);
}

///////////////////////////////////////////////////////////////////////////
inline
SPtr<Class> Value::constructor(VM& vm) const
{
    return &vm.GetValueTraits(*this).GetConstructor();
}

///////////////////////////////////////////////////////////////////////////
#ifdef SF_AS3_ENABLE_CALLFRAME_CACHE
SF_INLINE
void CallFrame::Setup(
    const Value& invoker,
    VMAbcFile& file,
    SInt32 mbi_ind, // Method body info index.
    const ScopeStackType* _saved_scope,
    bool discard_result,
    const Traits* ot
    SF_DEBUG_ARG(const ASString& name)
    SF_DEBUG_ARG(UInt64 startTicks)
    )
{
    DiscardResult = discard_result;
    SF_DEBUG_CODE(RegisteredFunction = false;)
    ScopeStackBaseInd = file.GetVM().GetScopeStack().GetSize();
    CP = NULL;
    SF_DEBUG_CODE( CurrFileInd = 0; )
    SF_DEBUG_CODE( CurrLineNumber = 0; )
    SF_DEBUG_CODE( StartTicks = startTicks; )
    pHeap = file.GetVM().GetMemoryHeap();
    pFile = &file;
    MBIIndex = mbi_ind;
    pSavedScope = _saved_scope;
    SF_DEBUG_CODE( Name = name.GetNode(); )
    pRegisterFile = &file.GetVM().GetRegisters();
    OriginationTraits = ot;
    pScopeStack = &file.GetVM().GetScopeStack();
    Invoker = invoker;

    // Store previous stack position.
    VM::OpStackType& stack = file.GetVM().OpStack;
    PrevInitialStackPos = stack.GetCurrent();
    SF_DEBUG_CODE(PrevReservedNum = stack.GetNumOfReservedElem();)
    SF_DEBUG_CODE(PrevFirstStackPos = stack.GetFirst());

    const Abc::MethodBodyInfo& mbi = GetMethodBodyInfo();

    // !!! We need this PLUS ONE at this time because return and returnvoid
    // put result in method's stack.
    stack.Reserve(static_cast<UInt16>(mbi.GetMaxStackSize() + 1));

    pRegisterFile->Reserve(static_cast<UInt16>(mbi.GetMaxLocalRegisterIndex()));
}

#else

SF_INLINE
CallFrame::CallFrame(
    const Value& invoker,
    VMAbcFile& file,
    Abc::MbiInd mbi_ind,
    bool discard_result,
#ifdef SF_AS3_ENABLE_EXPLICIT_GO
    Instances::fl::GlobalObjectScript& gos,
#endif
    const ScopeStackType& _saved_scope,
    const Traits& ot
    SF_DEBUG_ARG(const ASString& name)
    SF_DEBUG_ARG(UInt64 startTicks)
    )
    : DiscardResult(discard_result)
#ifdef SF_AS3_EMIT_DEF_ARGS
    , OpStackIsAllocated(false)
#endif
    , ACopy(false)
    SF_DEBUG_ARG(RegisteredFunction(false))
    , ScopeStackBaseInd(file.GetVM().GetScopeStack().GetSize())
    , CP(NULL)
#ifdef GFX_AS3_TRACE
    , OCPtr(NULL)
#endif
    , pRegisterFile(&file.GetVM().GetRegisters())
    , pHeap(file.GetVM().GetMemoryHeap())
    , pFile(&file)
    , MBIIndex(mbi_ind)
    , pSavedScope(&_saved_scope)
    , OriginationTraits(&ot)
    , pScopeStack(&file.GetVM().GetScopeStack())
#ifdef SF_AS3_ENABLE_EXPLICIT_GO
    , GOS(&gos)
#endif
    SF_DEBUG_ARG(Name(name.GetNode()))
    SF_DEBUG_ARG(CurrFileInd(0))
    SF_DEBUG_ARG(CurrLineNumber(0))
    SF_DEBUG_ARG(StartTicks(startTicks))
    , Invoker(invoker)
{
    // Store previous stack position.
    VM::OpStackType& stack = file.GetVM().OpStack;
#ifdef SF_BUILD_DEBUG
    SF_ASSERT(stack.IsValid());
#endif
    PrevInitialStackPos = stack.GetCurrent();
    SF_DEBUG_CODE(PrevReservedNum = stack.GetNumOfReservedElem();)
    PrevFirstStackPos = stack.GetFirst();

    const Abc::MethodBodyInfo& mbi = GetMethodBodyInfo();

#ifndef SF_AS3_EMIT_DEF_ARGS
    // !!! We need this PLUS ONE at this time because return and returnvoid
    // put result on method's stack.
    stack.Reserve(static_cast<UInt16>(mbi.GetMaxStackSize() + 1));
#endif

    pRegisterFile->Reserve(static_cast<UInt16>(mbi.GetMaxLocalRegisterIndex()));

#ifdef GFX_ENABLE_XML
    VM& vm = GetVM();
    if (vm.GetDefXMLNamespace())
    {
        DefXMLNamespace = vm.GetDefXMLNamespace();
        vm.SetDefXMLNamespace(NULL);
    }
#endif
}
#endif


}}} // namespace Scaleform { namespace GFx { namespace AS3 {

#endif // INC_AS3_VM_H
