/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_ResourceHandle.h
Content     :   Resource handle and resource binding support for GFx
Created     :   February 8, 2007
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_RESOURCEHANDLE_H
#define INC_SF_GFX_RESOURCEHANDLE_H

#include "GFx/GFx_Resource.h"

namespace Scaleform { namespace GFx {

// ***** Classes Declared

class ResourceHandle;

struct ResourceBindData;
class ResourceBinding;
class ResourceData;
class MovieDefImpl;

// ****** Resource Handle

/*
    ResourceHandle represents a handle to a Resource that can be
    represented either as a pointer or an index in a ResourceBinding
    table. An index is commonly used when resources which are referenced
    by a GFxMovieDataDef are bound to potentially different files in
    different MovieDefImpl instances. Keeping an index allows the
    common loaded data to be shared among differently-bound implementations.

    In order to reference ResourceHandle's Resource we need to
    resolve it by using GetResource or GetResourceAndBinding methods,
    which rely on a binding table. The binding table itself is stored
    as a part of MovieDefImpl.
*/

class ResourceHandle
{
public:
    enum HandleType
    {
        RH_Pointer,
        RH_Index
    };

protected:    
    HandleType  HType;
    union
    {
        unsigned    BindIndex;
        Resource*   pResource;
    };

public:

    ResourceHandle()
    {
        HType       = RH_Pointer;
        pResource   = 0;
    }    
    
    explicit ResourceHandle(Resource* presource)
    {
        HType       = RH_Pointer;
        pResource   = presource;
        if (presource)
            presource->AddRef();
    }

    ResourceHandle(HandleType typeIndex, unsigned index)
    {
        SF_UNUSED(typeIndex);
        SF_ASSERT(typeIndex == RH_Index);
        HType =     RH_Index;
        BindIndex = index;
    }
    
    ResourceHandle(const ResourceHandle& src)
    {
        HType       = src.HType;
        pResource   = src.pResource;
        if ((HType == RH_Pointer) && pResource)
            pResource->AddRef();
    }

    ~ResourceHandle()
    {
        if ((HType == RH_Pointer) && pResource)
            pResource->Release();
    }

    ResourceHandle& operator = (const ResourceHandle& src)
    {
        if ((src.HType == RH_Pointer) && src.pResource)
            src.pResource->AddRef();
        if ((HType == RH_Pointer) && pResource)
            pResource->Release();
        HType       = src.HType;
        pResource   = src.pResource;
        return *this;
    }

    // Determines similarity of handles. In general, this is exceprt as operator ==,
    // however, due to indexing pointed-to results might be different if they are
    // in a logically different scope. For this reason we made a custom function.
    inline bool         Equals(const ResourceHandle &other) const
    {
        return (HType == other.HType) && (pResource == other.pResource);
    }

    // A handle/resource pointer is considered null iff (1) it has a pointer
    // type and (2) its pointer value is null.
    inline bool         IsNull() const          { return (HType == RH_Pointer) && (pResource == 0); }

    inline bool         IsIndex() const         { return (HType == RH_Index); }
    inline unsigned     GetBindIndex() const    { SF_ASSERT(IsIndex()); return BindIndex; }
    // Obtains resource pointer from handle, but only if types is pointer.
    inline Resource*    GetResourcePtr() const { return (HType == RH_Pointer) ? pResource : 0; };
    // Obtains a pointer to the resource; can return a null pointer if binding failed or handle is null.
    inline Resource*    GetResource(const ResourceBinding* pbindings) const;
    // Obtains a resource and its relevant binding, which may be different from a passed one.
    inline Resource*    GetResourceAndBinding(ResourceBinding* pbindings, ResourceBinding** presBinding) const;

};

template<class C>
class ResourcePtr : public ResourceHandle
{    
public:

    ResourcePtr() : ResourceHandle() { }
    ResourcePtr(C* pres) : ResourceHandle(pres) { }
    ResourcePtr(const ResourcePtr& src) : ResourceHandle(src) { }
    
    ResourcePtr<C>& operator = (const ResourcePtr<C>& other)
    {
        *((ResourceHandle*)this) = (const ResourceHandle&)other;
        return *this;
    }

    ResourcePtr<C>& operator = (C* pres)
    {
        if (pres)
            pres->AddRef();
        if ((HType == RH_Pointer) && pResource)
            pResource->Release();
        HType       = RH_Pointer;
        pResource   = pres;
        return *this;
    }

    bool operator == (const ResourcePtr<C>& other) const
    {
        return ((HType == other.HType) && (pResource == other.pResource));
    }
    bool operator != (const ResourcePtr<C>& other) const
    {
        return !operator == (other);
    }
    
    // Sets a value from handle WITHOUT doing type check.
    void SetFromHandle(const ResourceHandle& src)
    {
        *((ResourceHandle*)this) = src;
    }
};



// For each resource, we store the Bindings in the context of which
// it is expected to work. In most cases, pBindings will be the same
// as 'this' class, however, it can be different for resources coming
// from imports.
struct ResourceBindData
{
    // We need to AddRef to resource because this may be the only place
    // where smart pointers to bound images and fonts are stored.
    // (Note that for imports AddRef is redundant but we do it anyway).
    Ptr<Resource>       pResource;
    ResourceBinding*    pBinding;

    ResourceBindData()
    {
        pBinding = 0;
    }
    ResourceBindData(const ResourceBindData& src)
    {
        pResource = src.pResource;
        pBinding  = src.pBinding;
    }

    ResourceBindData& operator = (const ResourceBindData& src)
    {
        pResource = src.pResource;
        pBinding  = src.pBinding;
        return *this;
    }
};



class ResourceBinding
{
    // Forward declaration for our owner MovieDefImpl.
    friend class MovieDefImpl;
private:

    MemoryHeap*                     pHeap;
    
    volatile unsigned               ResourceCount;
    ResourceBindData* volatile      pResources;

    // We use lock for now - it can be optimized with recount-based
    // lock free algorithm later.
    mutable Lock                    ResourceLock;

    // If this flag is set, locking is not necessary on read since
    // resource is now real-only. This allows for more efficient
    // resource look up after loading has finished.
    volatile bool                   Frozen;

    // TBD: Another option: Lock and copy list in the beginning of Advance?

    // MovieDefImpl that owns us. Since bindings are created only
    // in GFxMovieDefImpls, we store a pointer to it here. Use 'union'
    // so that we can AddRef to the resource without MovieDefImpl decl.
    union {
        Resource*                   pOwnerDefRes;
        MovieDefImpl*               pOwnerDefImpl;        
    };    

public:

    ResourceBinding(MemoryHeap *pheap);
    ~ResourceBinding();

    // Called to destroy ResourceBinding table contents, releasing
    // its references, before destructor.
    void            Destroy();

    
    // *** Owner MovieDefImpl management

    // It would be better to pass this in constructor, but doing so
    // generates a warning in MovieDefImpl initializer list.
    void SetOwnerDefImpl(class MovieDefImpl* powner)
    { Lock::Locker lock(&ResourceLock); pOwnerDefImpl = powner; }

    class MovieDefImpl* GetOwnerDefImpl() const { return pOwnerDefImpl; }
    // This version must be used if accessing DefImpl during loading; performs a lock.
    class MovieDefImpl* GetOwnerDefImplAddRef() const
    {
        Lock::Locker lock(&ResourceLock);
        if (pOwnerDefRes) pOwnerDefRes->AddRef();
        return pOwnerDefImpl;
    }

    
    // TBD: Technically binding array can grow as Bind catches up
    // with Read. Therefore, we might need to deal with some threading
    // considerations here.
    void            SetBindData(unsigned index, const ResourceBindData &bd);

    // Locked version GetResourceData. 
    void            GetResourceData_Locked(ResourceBindData *pdata, unsigned index) const;
    

    // Inline version of GetResourceData for fast access to frozen resources.
    // Non-frozen binding tables are more expensive to access since they require a lock.
    // NOTE that returned ResourceBindData CAN be null; this can happen if an import
    // failed to load and this left an initialized binding slot.
    inline void     GetResourceData(ResourceBindData *pdata, unsigned index) const
    {
        // For inline compactness, we only handle success of bound check here;
        // failure will be assigned correctly in GetResourceData_Locked.
        if (Frozen && (index < ResourceCount))        
            *pdata = pResources[index];
        else        
            GetResourceData_Locked(pdata, index);
    }


    // Get resource data by value.
    ResourceBindData    GetResourceData(const ResourceHandle &h)
    {
        ResourceBindData bd;

        if (h.IsIndex())        
            GetResourceData(&bd, h.GetBindIndex());
        else
        {
            bd.pBinding = this;
            bd.pResource= h.GetResourcePtr();
        }               
        return bd;
    }

 
    unsigned GetResourceCount() const { return ResourceCount;  }

    // Freeze the Resource binding once it is no longer being modified.
    void    Freeze() { Frozen = 1;  }    
    

    template<class C>
    C* operator [] (const ResourcePtr<C> &ref) const
    {
        return static_cast<C*>(ref.GetResource(this));
    }
};

inline Resource* ResourceHandle::GetResource(const ResourceBinding* pbinding) const
{
    if (HType == RH_Pointer)
        return pResource;

    ResourceBindData rbd;
    pbinding->GetResourceData(&rbd, BindIndex);
    return  rbd.pResource;
}

// Obtains a resource and its relevant binding, which may be different from a passed one.
inline Resource* ResourceHandle::GetResourceAndBinding(ResourceBinding* pbinding,
                                                       ResourceBinding** presBinding) const
{
    if (HType == RH_Pointer)
    {
        *presBinding = pbinding;
        return pResource;
    }

    ResourceBindData rbd;
    pbinding->GetResourceData(&rbd, BindIndex);
    *presBinding = rbd.pBinding;
    return  rbd.pResource;

}


// ***** ResourceData - TBD: Move to GFxMovieDef?

class LoadStates;

// A data object that can be used to create a resource. Data objects are created
// at load time and stored in GFxMovieDataDef; they allow resource binding index
// slots to be resolved when MovieDefImpl is created. As an example, resource
// data for a gradient may provide enough information to create a Resource image
// that is used for a gradient in MovieDefImpl. Every MovieDefImpl may have
// a different gradient image based on their different gradient parameters.
class ResourceData
{
public:

    typedef void*   DataHandle;

    // DataInterface     
    class DataInterface
    {
    public:
        virtual ~DataInterface() { }

        // Reference counting on the data object, if necessary.
        // Default implementation assumes that DataHandle object is derived from
        // RefCountBase<C>. If that is not the case, it needs to be overriden.
        virtual void    AddRef(DataHandle hdata)
        { SF_ASSERT(hdata); ((RefCountImpl*)hdata)->AddRef(); }
        virtual void    Release(DataHandle hdata)
        { SF_ASSERT(hdata); ((RefCountImpl*)hdata)->Release(); }

        // Creates/Loads resource based on data and loading process.
        // Fills in resource information into pbindData as follows:
        //  - pResource: set if CreateResource succeeds, 0 otherwise.
        //  - pResourceBinding : set only if resource requires custom binding;
        //                       otherwise retains its original value.
        virtual bool    CreateResource(DataHandle hdata, ResourceBindData *pbindData,
                                       LoadStates *plp, MemoryHeap *pheap) const = 0;

        // Also need to create key ??
    };

protected:
    // Key data.
    DataInterface* pInterface;
    DataHandle     hData;

public:

    ResourceData()
    {
        pInterface = 0;
        hData      = 0;
    }

    ResourceData(DataInterface* pki, DataHandle hk)
    {
        if (pki)
            pki->AddRef(hk);
        pInterface = pki;
        hData      = hk;
    }

    ResourceData(const ResourceData &src)
    {
        if (src.pInterface)
            src.pInterface->AddRef(src.hData);
        pInterface = src.pInterface;
        hData      = src.hData;
    }

    ~ResourceData()
    {
        if (pInterface)
            pInterface->Release(hData);
    }

    ResourceData& operator = (const ResourceData& src)
    {
        if (src.pInterface)
            src.pInterface->AddRef(src.hData);
        if (pInterface)
            pInterface->Release(hData);
        pInterface = src.pInterface;
        hData      = src.hData;
        return *this;
    }


    // *** Interface implementation

    bool            IsValid() const           { return pInterface != 0; }

    // Accessors, useful for DataInterface implementation.
    DataInterface*  GetDataInterface() const  {  return pInterface; }
    DataHandle      GetData() const           { return hData; }

    bool            CreateResource(ResourceBindData *pbindData, LoadStates *plp,
                                   MemoryHeap *pheap) const
    {        
        return pInterface ? pInterface->CreateResource(hData, pbindData, plp, pheap) : 0;
    }
};

}} // Scaleform::GFx

#endif // INC_SF_GFX_RESOURCEHANDLE_H

