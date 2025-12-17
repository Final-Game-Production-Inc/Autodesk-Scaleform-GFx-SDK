/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_Resource.h
Content     :   Resource and Resource Library definition for GFx
Created     :   January 11, 2007
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_RESOURCE_H
#define INC_SF_GFX_RESOURCE_H

#include "Kernel/SF_Types.h"
#include "Kernel/SF_Stats.h"
#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_Hash.h"

#include "Kernel/SF_Threads.h"

#include "GFx/GFx_String.h"

namespace Scaleform { namespace GFx {

// ***** Classes Declared

class ResourceId;
class ResourceKey;
class Resource;
class ResourceReport;

class ResourceLib;
class ResourceWeakLib;



/* *** ResourceId

ResourceId describes an id used to identify a resource in an SWF/GFX file.
ResourceId is used for indexing the resource in the resource table loaded
into MovieDataDef.

Most of the resource ids are assigned by the Flash Studio; however, extra
ids can also be generated in GFx either during loading (IdType_Static) or
when exporting dynamic resources with GFxExport (IdType_Export). Such ids
have their corresponding bits set, differentiating them from the original
SWF file resource ids.

In GFxExport, ids are used primarily to generate unique file names for
exported resources. 
*/


// Character Id wrapper class.
class ResourceId
{
    // Make sure the member variables stay with no padding
    // Padding breaks the hash function that uses this struct as a key
    UInt32    Id;

public:
    enum IdTypeConstants
    {
        IdType_Bit_IndexMask    = 0x0000FFFFu,
        IdType_Bit_TypeMask     = 0x0FFF0000u,

        // 2 bits - Describe Id generation source.
        IdType_Bit_SWF          = 0x00000,   // Id comes from SWF File.
        IdType_Bit_Static       = 0x10000,   // Id assigned uniquely during loading.
        IdType_Bit_Export       = 0x20000,   // Id assigned uniquely during export.
        IdType_Bit_GenMask      = 0x30000,

        // The amount to shift type code so that id does not
        // conflict with index and generation source.
        IdType_Bit_TypeShift    = 18,

        // Indicates an explicitly assigned invalid id.
        InvalidId               = (1 << IdType_Bit_TypeShift) | IdType_Bit_SWF
    };

    //  NOTE: Only the lower bits of resourceId (0xFFFF mask) can be used to identify
    //        characters instantiated on stage, due to the way depth is combined
    //        with id in display list searches. If high bits need to be used that
    //        logic (mostly in PlayerImpl and Sprite) would need to be changed.

    // IdType can be one of the following values describing what the id represents.
    enum IdType
    {
        IdType_None             = 0,
        IdType_InternalConstant = (0 << IdType_Bit_TypeShift) | IdType_Bit_Static,
        IdType_GradientImage    = (1 << IdType_Bit_TypeShift) | IdType_Bit_Static,
        IdType_DynFontImage     = (2 << IdType_Bit_TypeShift) | IdType_Bit_Static,
        IdType_FontImage        = (1 << IdType_Bit_TypeShift) | IdType_Bit_Export
    };

    inline explicit ResourceId()                              { Id = InvalidId; }
    inline explicit ResourceId(UInt32 resId)                  { Id = resId; }
    inline ResourceId(const ResourceId &src)               { Id = src.Id; }
    inline ResourceId operator = (const ResourceId &src)   { Id = src.Id; return *this; }

    bool operator == (const ResourceId &other) const          { return Id == other.Id; }
    bool operator != (const ResourceId &other) const          { return !operator == (other); }
    // Numeric compare, so that we can compare with InvalidId, etc.
    bool operator == (UInt32 idVal) const                        { return Id == idVal; }
    bool operator != (UInt32 idVal) const                        { return !operator == (idVal); }


    // Returns a Id integer code that uniquely identifies the character.
    inline  UInt32          GetIdValue() const  { return Id; }

    // Returns index and 
    inline  unsigned        GetIdIndex() const  { return Id & IdType_Bit_IndexMask;  }
    inline  IdType          GetIdType() const   { return (IdType) (Id & IdType_Bit_TypeMask); }


    // Generate Resource string, up to 8 characters - must pass a buffer of 9.
    unsigned                GenerateIdString(char* pbuffer, UPInt bufferSize =9, char suffixLetter = 0) const;

    // Generate nest Id for the IdType_Bit_Static and IdType_Bit_Export types.
    // The Id to use is returned,
    inline  ResourceId   GenerateNextId()
    {        
        SF_ASSERT(Id & IdType_Bit_GenMask);        
        SF_ASSERT((Id & IdType_Bit_IndexMask) != IdType_Bit_IndexMask); // Detect overflows.
        ResourceId result = *this;
        Id++;
        return result;
    }


    // Hash Op for when ResourceIds are used in Hash table
    class HashOp
    {
    public:
        UPInt operator () (const ResourceId& idref)
        {
            return (idref.Id ^ (idref.Id >> (16 - 8)));
        }
    };
};




// ***** File information for file formats from which images can come

class FileTypeConstants
{
public:

    // All File formats supported and/or used by GFx.
    enum FileFormatType
    {        
        File_Unopened,
        File_Unknown,

        // SWF file formats.
        File_SWF,
        File_GFX,

        // Image formats supported by Flash.        
        File_JPEG   = 10,  // Format codes are serialized by 'gfxexport'. They should not change.
        File_PNG    = 11,
        File_GIF    = 12,
        // Other image formats.
        File_TGA    = 13,
        File_DDS    = 14,        
        File_HDR    = 15,
        File_BMP    = 16,
        File_DIB    = 17,
        File_PFM    = 18,
        File_TIFF   = 19,

        // Sound formats
        File_WAVE   = 20,

        //Additional Image Format
        File_PVR    = 21,
        File_ETC    = 22,
        File_SIF    = 23,
        File_GXT    = 24,
        File_GTX    = 25,
        File_GNF    = 26,
        File_KTX    = 27,

        File_NextAvail,

        //not really file formats, used as formatID in gfxexport
        File_DDSBC       = 65533,
        File_MultiFormat = 65534,
        File_Original  = 65535
    };
};


// ***** ResourceKey - serves as a general key used in resource matching.


struct ResourceFileInfo : public RefCountBaseNTS<ResourceFileInfo, Stat_Default_Mem>
{
    FileTypeConstants::FileFormatType   Format;
    const struct ExporterInfo*          pExporterInfo;    
    String                              FileName;

    ResourceFileInfo();
    ResourceFileInfo(const ResourceFileInfo &src);

    UPInt  GetHashCode() const;
};


// General resource keys. Highlights:
//  - Wraps and holds key data, utilizing AddRef semantics.
//  - Can be copied by value.
class ResourceKey
{
public:

    enum KeyType
    {
        Key_None,
        Key_Unique,
        Key_File,       // hdata = ResourceFileInfo*
        Key_Gradient,
        Key_SubImage
    };

    typedef void*   KeyHandle;

    // KeyInterface provides functionality for key value / hash matching and
    // for data lifetime management. Separating interface and its data allows
    // for a more flexible storage of the key, potentially as a part of a resource.
    class KeyInterface
    {
    public:
        virtual ~KeyInterface() { }

        // Reference counting on the data object, if necessary.
        virtual void    AddRef(KeyHandle hdata)     = 0;
        virtual void    Release(KeyHandle hdata)    = 0;

        // Key/Hash code implementation.
        virtual KeyType GetKeyType(KeyHandle hdata) const                           = 0;
        virtual UPInt   GetHashCode(KeyHandle hdata) const                          = 0;
        virtual bool    KeyEquals(KeyHandle hdata, const ResourceKey& other)     = 0;

        // Get file info about the resource, potentially available with Key_File.
        
        // MA: FileInfo can not be available here, since FileFormat, etc may
        // not yet be available when we create a key. In other words, creating a key
        // should not require information that can only be obatined by opening a file.
        //virtual const ResourceFileInfo* GetFileInfo(KeyHandle) const    { return 0; }

        virtual const char* GetFileURL(KeyHandle) const    { return 0; }
    };

protected:
    // Key data.
    KeyInterface* pKeyInterface;
    KeyHandle     hKeyData;

public:
    ResourceKey();
    ResourceKey(KeyInterface* pki, KeyHandle hk);
    ResourceKey(const ResourceKey &src);

    ResourceKey& operator = (const ResourceKey &src);

    ~ResourceKey()
    {
        if (pKeyInterface)
            pKeyInterface->Release(hKeyData);
    }

    KeyType GetKeyType() const
    {
        return pKeyInterface ? pKeyInterface->GetKeyType(hKeyData) : Key_None;
    }

    /*
    // TBD: Get file for Key_File only.
    const ResourceFileInfo* GetFileInfo() const
    {
        return pKeyInterface ? pKeyInterface->GetFileInfo(hKeyData) : 0;
    }
    */

    const char* GetFileURL() const
    {
        return pKeyInterface ? pKeyInterface->GetFileURL(hKeyData) : 0;
    }


    // Equality Comparison: compares keys using KeyEquals.
    // Since both keys can be implemented bu different interfaces,
    // implementation is responsible for performing a check. It 
    // can do so based on two criteria:
    //    1) Interface pointer comparison (requires a singleton interface object).
    //    2) Doing KeyType comparison to a known type through pother->GetKeyType().
    bool operator == (const ResourceKey& other) const
    {
        if (pKeyInterface && other.pKeyInterface)
            // A check such as below above is the responsibility
            // of the KeyInterface implementor.
            // && pKeyInterface->GetKeyType() == other.pKeyInterface->GetKeyType()
            return pKeyInterface->KeyEquals(hKeyData, other);

        return 0;
    }


    // Accessors, useful for KeyInterface implementation.
    KeyInterface* GetKeyInterface() const   { return pKeyInterface; }
    KeyHandle     GetKeyData() const        { return hKeyData; }

    // Hash function used when matching keys.
    class HashOp
    {
    public:
        UPInt operator () (const ResourceKey& key)
        {
            return key.pKeyInterface ? key.pKeyInterface->GetHashCode(key.hKeyData) : 0;
        }
    };
};



// ***** Resource

// Resource library managing a certain type of resource
class ResourceLibBase : public RefCountBase<ResourceLibBase, Stat_Default_Mem>
{
public:
    virtual void RemoveResourceOnRelease(Resource *pres) = 0;

    // Pin/Unpin.
    virtual void PinResource(Resource* pres)     = 0;
    virtual void UnpinResource(Resource* pres)   = 0;
};


// Resources can live in the resource library and have following characteristics:
//  - Thread-Safe RefCounting
//  - Back-link to update library on death, i.e. when refCount == 0
//  - Locking on library update
//  - Pin/Unpin methods that allow users to pin them with a ref in strong library.
//  - A unique key that can be used to locate/match them.

class Resource : public NewOverrideBase<Stat_Default_Mem>
{
    friend class ResourceWeakLib;    

private:
    AtomicInt<SInt32>  RefCount;
    // Library to which we belong, if any.
    ResourceLibBase* pLib;

public:

    // Thread-Safe ref-count implementation.
    void        AddRef();  
    void        Release();
    bool        AddRef_NotZero();
    SInt32      GetRefCount() const;
    

    Resource()
    {
        RefCount.Value = 1; // Non-Atomic write ok on construction. 
        pLib           = 0;
    }
    virtual ~Resource()
    {
    }

    // Pin/Unpin delegate to the owner library, if any.
    void    PinResource()       { if (pLib) pLib->PinResource(this); }
    void    UnpinResource()     { if (pLib) pLib->UnpinResource(this);  }



    // Assigns owner library for resource.
    void        SetOwnerResourceLib(ResourceLibBase *plib)
    {   // Can not reassign the library of the resource.
        SF_ASSERT((pLib == 0) || (plib == 0));
        pLib = plib;
    }


    // *** Resource Type and Use

    enum ResourceType
    {
        // Set this bit if CharacterDef
        RT_CharacterDef_Bit = 0x0080,

        // Non-character types.
        RT_None             = 0,
        RT_Image,
        RT_Font,
        // Internal.
        RT_MovieDef,
        RT_SoundSample,
        
        // CharacterDef derived types.
        RT_MovieDataDef     = RT_CharacterDef_Bit | 0,
        RT_ButtonDef,
        RT_TextDef,
        RT_EditTextDef,
        RT_SpriteDef,
        RT_ShapeDef,
        RT_VideoDef,

        // Mask and shift applied to store ResourceType in type code.
        RT_TypeCode_Mask    = 0xFF00,
        RT_TypeCode_Shift   = 8
    };

    // Note: Uses correspond to a type,
    // but they don't have to be treated differently.
    enum ResourceUse
    {
        Use_None        = 0,
        Use_Bitmap      = 1,
        Use_Gradient    = 2,
        Use_FontTexture = 3,
        Use_SoundSample = 4,
        // Use mask within TypeCode.
        Use_TypeCode_Mask = 0xFF
    };

    
    // Get the resource key that can be used for library matching.
    virtual ResourceKey  GetKey()                        { SF_ASSERT(0); return ResourceKey(); }
    // Query Resource type code, which is a combination of ResourceType and ResourceUse.
    virtual unsigned        GetResourceTypeCode() const  { return MakeTypeCode(RT_None); }

    virtual ResourceReport* GetResourceReport()          { return 0; }
    

    // Simple Resource Type and Use access.
    inline ResourceType    GetResourceType() const
    {
        return (ResourceType) 
            ((GetResourceTypeCode() & RT_TypeCode_Mask) >> RT_TypeCode_Shift);
    }
    inline ResourceUse    GetResourceUse() const
    {
        return (ResourceUse) (GetResourceTypeCode() & Use_TypeCode_Mask);
    }

    static inline unsigned SF_STDCALL MakeTypeCode(ResourceType rt, ResourceUse ru = Use_None)
    {
        return (rt << RT_TypeCode_Shift) | ru;
    }
};


// Reports Heap/Memory data about a resource, useful if resource has
// a separate heap, name and stats.
// This class is primarily used to report GFxMovieDataDef.
class ResourceReport
{
public:
    virtual ~ResourceReport() { }

    virtual String      GetResourceName() const { return String(); }
    virtual MemoryHeap* GetResourceHeap() const { return 0; }
    virtual void        GetStats(StatBag* pbag, bool reset = true) { SF_UNUSED2(pbag, reset); }
};


// This is defined to enable use of dedicated image heap for shared images in ResourceLib,
// as well as nested image heap in movie data. This is done to primarily reduce global
// heap fragmentation and to reduce bookkeeping overhead for images. Pros/Cons need
// more investigation.
#define GFX_USE_IMAGE_HEAPS


// ***** ResourceLib implementation.

/*  ResourceLib Architecture

    ResourceLib is a thread-safe library used for sharing resources based on
    their keys. Different types of resources can exist in the library simultaneously,
    including both user-created external resources as well as resources created
    internally by the flash file. If two Loader objects have the same library,
    they will be able to share resources.

    
    Strong and Weak Libraries    

    To facilitate automated sharing of resources without reference counting problems 
    the library is separated into a strong and weak library classes, as follows:
        - ResourceLib is a strong library, owned by loader
        - ResourceWeakLib is a weak library, owned by ResourceLib and also
          owned/shared by the movies. ResourceWeakLib can exist without the strong
          lib, but not the other way around.

    Any sharable resource loaded into memory always exists in a weak library; however,
    the weak library does not AddRef to the resource. If the resource is released because
    it reference count is decremented, it will also update ResourceWeakLib in a thread
    safe manner. Movie resources will get released if the user releases all of their
    smart pointer references to the movie instances and definitions. No extra work needs
    to be done if the developer is content with automated sharing.
    
    If the developer would like the resource to remain alive, they can do so by either
    keeping a smart pointer to it or 'pinning' it in memory. Pinning a resource creates
    a smart pointer to it in the strong library; it is achieved by calling PinResource().
    Once a resource is pinned it will remain alive and sharable until a strong library
    itself is released (this normally happens when Loader is destroyed).


    Adding Resources to Library

    Since ResourceLib is meant to be thread safe, it synchronizes access to its
    data structures. However, simple data locking is not enough to ensure that
    loading threads cooperate. Consider a traditional approach for creation and 
    library sharing of resources, which may include the following algorithm:

        presource = Library.Get(resourceKey);
        if (presource == NULL)
        {
            presource = LoadResource(resourceKey);
            Library.Add(resourceKey, presource);
        }

    This logic can malfunction if several threads try to load the resource at the
    same time. Specifically, if thread B attempts to load a resource while thread A is
    already in the process of doing so, the logic above will end up loading it twice,
    wasting both memory and processing resources.

    To avoid this problem ResourceLib the makes use of bind handles. The actual
    insertion of a resource therefore relies on the following logic:

        ResourceLib::BindHandle bh;        

        if (Library.BindResourceKey(&bh, resourceKey) == ResourceLib::RS_NeedsResolve)
        {
            presource = *LoadResource(resourceKey);
            if (presource)
                bh.ResolveResource(presource);
            else
            {
                bh.CancelResolve("error message);
                HandleError();
            }
        }
        else
        {
            presource = *bh.WaitForResolve();
            if (!presource)            
                HandleError();
        }
    
    Here, the initial thread requesting the resource creates a bind handle and
    proceeds to resolve it if the bind status is RS_NeedsResolve. Once finished
    with loading, it resolves it with ResolveResource (or cancels loading with
    CancelResolve). Overall, BindResourceKey and ResolveResource calls are similar
    to the Get and Add calls in the earlier example. There major difference,
    however, occurs if a second thread comes in to load the resource while the
    first one is in the process of doing so. In this case the bind status returned
    by the BindResourceKey function will be RS_WaitingResolve, allowing the second
    thread to wait for resource loading completion. Once the first thread is done
    loading and resolves the resource, WaitForResolve will return the resource
    with its content appropriately shared.

*/


// Strong resource library
//  - Contains an array of strong pointers.
//  - Delegates to ResourceWeakLib for all lookup purposes.
//  - ResourceWeakLib always 'lives longer'.
//  - Maintains a dedicated heap for shared images (if GFX_USE_IMAGE_HEAPS is defined).

class ResourceLib : public RefCountBase<ResourceLib, Stat_Default_Mem>
{
    friend class ResourceWeakLib;    
    
    struct ResourcePtrHashFunc
    {    
        typedef Resource* C;
        // Hash code is computed based on a pointer.
        UPInt  operator() (const C& data) const                     { return (((UPInt)data) >> 6) ^ ((UPInt)data); }
        // Hash update and value access.
        static UPInt    SF_STDCALL GetCachedHash(const C& data)                { return (((UPInt)data) >> 6) ^ ((UPInt)data); }
        static void     SF_STDCALL SetCachedHash(C& data, UPInt hashValue)     { SF_UNUSED2(data, hashValue); }
        static C&       SF_STDCALL GetValue(C& data)                           { return data; }
        static const C& SF_STDCALL GetValue(const C& data)                     { return data; }
    };

    typedef HashSetUncached<Resource*, ResourcePtrHashFunc> PinHashSet;

    // Strong pointer to the WeakLib.
    ResourceWeakLib*      pWeakLib;
    // 'Pin' hash. Every object is only pinned once. AddRef'ed manually.
    // Updates performed only during weak library lock.
    PinHashSet               PinSet;
    // Set if this resource is used by debug loader.
    bool                     DebugFlag;

public:

    ResourceLib(bool debug = false);
    ~ResourceLib();

    enum ResolveState
    {
        RS_Unbound,
        RS_Available,
        RS_WaitingResolve,
        RS_NeedsResolve,
        RS_Error
    };

    
   // Resource Slots: Allocated in a separate list during a lock
    // 

    class ResourceSlot : public RefCountBase<ResourceSlot, Stat_Default_Mem>
    {
    public:
        enum ResolveState
        {
            Resolve_InProgress,
            Resolve_Success,
            Resolve_Fail
        };

    private:
        Ptr<ResourceWeakLib> pLib;
        volatile ResolveState State;
        // AddRefed to ensure that resource doesn't get released
        // while there are outstanding BindHandles waiting on it.
        Resource* volatile  pResource;
        ResourceKey         Key;
        String              ErrorMessage;

#ifdef SF_ENABLE_THREADS
        Scaleform::Event    ResolveComplete;
#endif        

    public:

        ResourceSlot(ResourceWeakLib* plib, const ResourceKey& key);
        ~ResourceSlot();
        
        const ResourceKey& GetKey() const { return Key; }

        // *** Interface for Waiter
        // If we are not responsible, wait to receive resource.
        // The returned resource will be AddRefed.
        Resource*    WaitForResolve();
        // Check availability.
        bool            IsResolved() const;
        const char*     GetError() const;
                
        // *** Interface for resolver
        // If we are responsible call one of these two:
        void            Resolve(Resource* pres);       
        void            CancelResolve(const char* perrorMessage);
    };


    class BindHandle
    {
    public:

        ResolveState    State;
        union
        {
            Resource*       pResource;  // Valid only if State == RS_Available
            ResourceSlot*   pSlot;      // Valid only if State >= RS_WaitingResolve
        };

        BindHandle()
        {
            State       = RS_Unbound;
            pResource   = 0;
        }

        ~BindHandle()
        {
            if (State == RS_Available)
                pResource->Release();
            else if (State >= RS_WaitingResolve)
                pSlot->Release();
        }

        // Clears the bind handle so that it can be reused.
        void                    Clear()
        {
            if (State == RS_Available)
                pResource->Release();
            else if (State >= RS_WaitingResolve)
            {
                /*
                // If this hits, that means we didn't close the handle properly by calling 
                // ResolveResource or CancelResolve. Doing so is an error, since it could
                // leave other threads waiting indefinitely.
                SF_ASSERT(State != RS_NeedsResolve);
                */

                pSlot->Release();
            }
            // Reset values.
            State       = RS_Unbound;
            pResource   = 0;
        }


        // Obtain state; based on it you can call different functions.
        inline  ResolveState    GetState() const        { return State; }
        inline  bool            IsAvailable() const     { return State == RS_Available; }

        // Determine if we need resolve. If so,
        // we can all either Resolve() or CancelResolve().        
        // Note that release of all bind handles is treated as cancel.
        // - problem: what happens to other bind handles? They report error. 
        inline  bool            NeedsResolve() const    { return State == RS_NeedsResolve; }

        // If we are not responsible for resolving, wait to receive resource.
        // The result returned from WaitToResolve will be AddRef'ed, so the
        // user is responsible for releasing it.
        Resource*            WaitForResolve();

        // Get resource pointer; only if GetState() == RS_Available.
        inline  Resource*    GetResource() const
        {
            SF_ASSERT(State == RS_Available);
            return pResource;
        }       

        // Get error in case WaitForResolve returned 0.
        inline const char*     GetResolveError() const
        {
            return (State >= RS_WaitingResolve) ? pSlot->GetError() : "";
        }

        // *** Interface for resolver

        // If we are responsible call one of these two:
        inline void    ResolveResource(Resource* pres)
        {
            SF_ASSERT(State == RS_NeedsResolve);
            pSlot->Resolve(pres);
        }

        inline void    CancelResolve(const char* perrorMessage)
        {
            SF_ASSERT(State == RS_NeedsResolve);
            pSlot->CancelResolve(perrorMessage);
        }
    };


    // Returns AddRefed ResourceWeakLib ptr.
    inline ResourceWeakLib* GetWeakLib() const  { return pWeakLib; }

    // Resource access - Delegates to WeakLib.
    inline Resource*        GetResource(const ResourceKey &k);    
    inline ResolveState     BindResourceKey(BindHandle *phandle, const ResourceKey &k);
    // Debug resource list access.
    inline void             GetResourceArray(Array<Ptr<Resource> > *presources);

    inline void             UnpinAll();

    // Obtain heap that should be used for image allocations with this resource library.
    inline MemoryHeap*      GetImageHeap() const;

};


class ResourceWeakLib : public ResourceLibBase
{
    friend class ResourceLib;
    friend class ResourceLib::ResourceSlot;

    typedef ResourceLib::ResourceSlot    ResourceSlot;    

    // Node that needs to be stored in case resource has not been resolved yet.
    struct ResourceNode
    {
        enum NodeType
        {
            Node_Resource,
            Node_Resolver
        };

        NodeType        Type;

        union
        {
            ResourceSlot*   pResolver;
            Resource*       pResource;
        };

        bool    IsResource() const { return Type == Node_Resource; }

        // Comparison to key, required for hashtable.
        bool    operator == (const ResourceKey& k) const
        {
            SF_ASSERT(pResource != 0);
            if (Type == Node_Resource)
                return pResource->GetKey() == k;
            return pResolver->GetKey() == k;
        }
        bool    operator != (const ResourceKey& k) const
        {
            return operator != (k);
        }

        // Hash function used for nodes.
        struct HashOp
        {                
            // Hash code is computed based on a pointer.
            UPInt  operator() (const ResourceNode& node) const
            {
                SF_ASSERT(node.pResource != 0);
                ResourceKey k = (node.Type == Node_Resource) ?
                                    node.pResource->GetKey() : node.pResolver->GetKey();
                return ResourceKey::HashOp()(k);                
            }

            UPInt  operator() (const ResourceKey& k) const
            {
                return ResourceKey::HashOp()(k);
            }
        };
    };

    // Weak pointer to the strong library container.
    ResourceLib *                pStrongLib;
    Lock                         ResourceLock;
    // Collection of weak references to resources.
    HashSet<ResourceNode, ResourceNode::HashOp>    Resources;

#ifdef GFX_USE_IMAGE_HEAPS
    // Heap used by default for images stored in the resource library; images
    // allocated from this heap need to be released BEFORE the this library object.
    // Right now we do this for images only, however, it may make sense to do this
    // for sounds and potentially other data in the future.
    // To reduce likelihood of deallocating the library before the image accidentally,
    // its heap is used as a part of the image key.
    Ptr<MemoryHeap>               pImageHeap;
#endif

public:

    ResourceWeakLib(ResourceLib *pstrongLib);
    ~ResourceWeakLib();

    typedef ResourceLib::BindHandle      BindHandle;
    typedef ResourceLib::ResolveState    ResolveState;    
    
    // Lookup resource, but only if resolved.
    // All resources are considered AddRefed.
    Resource*    GetResource(const ResourceKey &k);    
    // Lookup resource and insert its slot. We get back a BindHandle.    
    ResolveState    BindResourceKey(BindHandle *phandle, const ResourceKey &k);

    // Pin-management: for strong links.
    // Returns 1 if resource is pinned in strong library.
    bool            IsPinned(Resource* pres);
    void            PinResource(Resource* pres);
    void            UnpinResource(Resource* pres);
    void            UnpinAll();    

    // Queries an AddRefed list of all resources, used for debug/stat display.
    // The list should not be kept permanently as it will prevent items in
    // the library from being properly released.
    void            GetResourceArray(Array<Ptr<Resource> > *presources);

    // Obtain heap that should be used for image allocations with this resource library.
#ifdef GFX_USE_IMAGE_HEAPS
    MemoryHeap*    GetImageHeap() const { return pImageHeap.GetPtr(); }    
#else
    MemoryHeap*    GetImageHeap() const { return Memory::GetGlobalHeap(); }
#endif

    // Virtual function called when resource RefCount has reached 0
    // and the resource is about to die.
    virtual void    RemoveResourceOnRelease(Resource *pres);    

    inline ResourceLib*  GetResourceLib() const { return pStrongLib; }
};

// ResourceLib inlines.
inline Resource* ResourceLib::GetResource(const ResourceKey &k)
{
    return pWeakLib->GetResource(k);
}
inline ResourceLib::ResolveState ResourceLib::BindResourceKey(
                ResourceLib::BindHandle *phandle, const ResourceKey &k)
{
    return pWeakLib->BindResourceKey(phandle, k);
}
inline void ResourceLib::GetResourceArray(Array<Ptr<Resource> > *presources)
{
    return pWeakLib->GetResourceArray(presources);
}

inline void ResourceLib::UnpinAll()
{
    return pWeakLib->UnpinAll();
}        

inline MemoryHeap* ResourceLib::GetImageHeap() const
{
    return pWeakLib->GetImageHeap();
}

}} // Scaleform::GFx

#endif // INC_SF_GFX_RESOURCE_H

