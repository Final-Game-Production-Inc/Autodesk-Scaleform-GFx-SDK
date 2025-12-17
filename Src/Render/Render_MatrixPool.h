/**************************************************************************

PublicHeader:   Render
Filename    :   Render_MatrixPool.h
Content     :   Handle-indexed pool for different-sized matrices
                and color transforms.
Created     :   June 7, 2010
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_MatrixPool_H
#define INC_SF_Render_MatrixPool_H

#include "Kernel/SF_MemoryHeap.h"
#include "Kernel/SF_List.h"
#include "Kernel/SF_RefCount.h"
#include "Render_Types2D.h"
#include "Render_Matrix2x4.h"
#include "Render_Matrix3x4.h"
#include "Render_CxForm.h"
#include "Render_Stats.h"

//--------------------------------------------------------------------
// MatrixPool is a memory pool used for storing matrices and Cxforms
// by the renderer back end. Matrix data managed by the pool is accessed
// through HMatrix handle objects, which have accessors such as Get/SetMatrix2D,
// Get/SetCxform and Get/SetTextureMatrix. A single matrix handle is typically
// associated with once renderer ShapeCacheNode and its unused internal
// components are not allocated unless initialized.
//
// Public interface includes the following:
//   - MatrixPool class, created by renderer.
//   - HMatrix handles, created by the MatrixPool.

// Public class mappings from MatrixPoolImpl namespace:
namespace Scaleform { namespace Render {
namespace MatrixPoolImpl {
    class HMatrix;
    class MatrixPool;
};
    using MatrixPoolImpl::HMatrix;
    using MatrixPoolImpl::MatrixPool;
}} //  Scaleform::Render


//--------------------------------------------------------------------
// Implementation namespace
namespace Scaleform { namespace Render { namespace MatrixPoolImpl {

// Implementation helpers.
struct EntryHandle;
class  DataHeader;
struct DataPage;
struct HandlePage;
struct HandlePageBase;
class  EntryHandleTable;


// ***** MatrixPool Control Constants

enum {
    // Allocation unit (16 bytes, good enough for 4 floats).
    Pool_DataUnitSize   = 16,
    Pool_DataUnitAlign  = 16,
    Pool_DataUnitMask   = Pool_DataUnitAlign - 1,
    
    // Size of Element_UserData
    Pool_UserDataUnits  = 4,
    Pool_UserDataSize   = Pool_DataUnitSize * Pool_UserDataUnits
};

// HMatrixConstants constants are a part of a public interface 
// as they are accessed through HMatrix, as HMatrix::Has_Cxfrom.
// These are separated into a separate class so that they can be
// used in several classed conveniently.

class HMatrixConstants
{
public:
    // Matrix Element bits describe presence and location of matrix elements.
    enum ElementIndex
    {
        Element_Cxform      = 0,
        Element_T0          = 1,
        Element_T1          = 2,
        Element_UserData    = 3,
        Element_Matrix      = 4,
        Element_Last_Index  = Element_Matrix
    };

    // Bits encoded into size & offsets
    enum FormatBits
    {
        // Bit mapping for matrix components that a matrix has.
        Has_Cxform          = 1 << Element_Cxform,
        Has_T0              = 1 << Element_T0,
        Has_T1              = 1 << Element_T1,
        Has_UserData        = 1 << Element_UserData,
        // Store Matrix 2D/3D as last element since it has variable size.
        Has_3D              = 1 << Element_Matrix,

        // Additional "combination" flags.
        Has_2D              = 0,
        Has_T0_T1           = Has_T0 | Has_T1,
        Has_Cxform_T0       = Has_Cxform | Has_T0,
        Has_Cxform_T0_T1    = Has_Cxform | Has_T0 | Has_T1,
        Has_Cxform_3D       = Has_Cxform | Has_3D,
        Has_Cxform_T0_3D    = Has_Cxform | Has_T0 | Has_3D,
        Has_Cxform_T0_T1_3D = Has_Cxform | Has_T0 | Has_T1 | Has_3D,
    };

    enum {
        // Low three bits index table
        Has_TableSize = 1 << Element_Last_Index, // 16
        Has_TableMask = Has_TableSize - 1
    };

    // ElementsInfo describes location of matrix set elements within
    // a data block. Size can be obtained from last element.
    struct ElementsInfo 
    {
        UByte Offsets[Element_Last_Index+1]; // Cx, T0, T1, UserData, M
    };

    static ElementsInfo MatrixElementSizeTable[Has_TableSize];

    static unsigned GetFormatDataSize(unsigned formatBits)
    {
        unsigned frontSize = 
            MatrixElementSizeTable[formatBits&Has_TableMask].Offsets[Element_Last_Index];
        return (frontSize + ((formatBits & Has_3D) ? 3 : 2)) * Pool_DataUnitSize;
    }
};


// EntryHandle is pointer to matrix data stored by HMatrix,
// it allows matrix data to be movable.
struct EntryHandle
{
    typedef HMatrixConstants::ElementIndex ElementIndex;

    // Handle either points to data or next free handle item.
    union {
        DataHeader*  pHeader;
        EntryHandle* pNext;
    };
   
    void  ReleaseHandle();
    inline HandlePage* GetHandlePage();

    // Realloc
    void ReallocToFormat(unsigned formatBits);
    void ReallocAddElement(ElementIndex index);
    
    // *** Inlines to DataHeader delegated for convenience.
    inline void      AddRef();
    inline void      Release();
    //    
    inline void*     GetData(ElementIndex index);
    inline void*     GetUserDataPtr();
    inline Matrix2F* GetMatrixPtr();
    inline Matrix3F* GetMatrix3DPtr();
    inline Cxform*   GetCxformPtr();
    inline Matrix2F* GetTextureMatrixPtr(ElementIndex e);
    //
    inline bool      FormatHasElement(ElementIndex index) const;
    inline unsigned  GetFormatBits() const;

};

// EntryHandleTable allocates and manages lifetime of EntryHandle tables.
class EntryHandleTable
{    
    friend struct EntryHandle;

    MemoryHeap*         pHeap;
    MatrixPool*         pPool;
    // Full set of allocated context entries.
    List<HandlePage, HandlePageBase> FullPages;
    List<HandlePage, HandlePageBase> PartiallyFreePages;

    // Allocates a page and adds its items to free list.
    bool    allocEntryPage();   
    void    freeEntryPage(HandlePage* ppage);

public:
    EntryHandleTable(MemoryHeap* pheap, MatrixPool* pool);
    ~EntryHandleTable();

    // Allocate/Free of ContextEntrys.
    EntryHandle*  AllocEntry(DataHeader* header = 0);
    void          FreeEntry(EntryHandle* p); 
};


// DataHeader is placed in memory before every matrix data set.
// Must be max/aligned to 16 bytes.
class DataHeader : public HMatrixConstants
{    
public:
    EntryHandle* pHandle;
    // Remainder must be 8 bytes max, so as to fit into 16 bytes on 64-bit systems.
    UInt32       RefCount;  // UInt32 to makes ref-count big enough for NullHandle.
    SInt16       DataPageOffset;
    UByte        UnitSize;   // Size of this block in units, includes self.
    UByte        Format;

    void   AddRef() { RefCount++; }
    void   Release();
    
    UByte*       GetDataStart()
    { return ((UByte*)this + Pool_DataUnitSize); }
    void*       GetData(ElementIndex index)
    { return (GetDataStart() + GetElementInfo().Offsets[index] * Pool_DataUnitSize); }

    void*       GetUserDataPtr()    { return GetData(Element_UserData); }
    Matrix2F*   GetMatrixPtr()      { return (Matrix2F*)GetData(Element_Matrix); }
    Matrix3F*   GetMatrix3DPtr()    { return (Matrix3F*)GetData(Element_Matrix); }
    Cxform*     GetCxformPtr()      { return (Cxform*)GetData(Element_Cxform); }
    Matrix2F*   GetTextureMatrixPtr(ElementIndex e) { return (Matrix2F*)GetData(e); }


    const ElementsInfo& GetElementInfo() const
    { return MatrixElementSizeTable[Format & Has_TableMask]; }

    bool FormatHasElement(ElementIndex index) 
    {
        SF_ASSERT(index < Element_Matrix); // Matrix is not optional
        return (Format & (1<<index)) != 0;
    }

    unsigned GetFormatBits() const { return (unsigned)Format; }

    // Internal helpers
    inline DataPage*    GetDataPage();
    inline DataHeader*  GetNextHeader(unsigned unitSize);
};


// *** EntryHandle inlines

inline void      EntryHandle::AddRef()              { pHeader->AddRef(); }
inline void      EntryHandle::Release()             { pHeader->Release(); }
inline void*     EntryHandle::GetData(ElementIndex index) { return pHeader->GetData(index); }
inline void*     EntryHandle::GetUserDataPtr()      { return pHeader->GetUserDataPtr(); }
inline Matrix2F* EntryHandle::GetMatrixPtr()        { return pHeader->GetMatrixPtr(); }
inline Matrix3F* EntryHandle::GetMatrix3DPtr()      { return pHeader->GetMatrix3DPtr(); }
inline Cxform*   EntryHandle::GetCxformPtr()        { return pHeader->GetCxformPtr(); }
inline Matrix2F* EntryHandle::GetTextureMatrixPtr(ElementIndex e)
{ return pHeader->GetTextureMatrixPtr(e); }
//
inline bool      EntryHandle::FormatHasElement(ElementIndex index) const
{ return pHeader->FormatHasElement(index); }
inline unsigned  EntryHandle::GetFormatBits() const
{ return pHeader->GetFormatBits(); }



//--------------------------------------------------------------------
// ***** HMatrix 
//--------------------------------------------------------------------

// HMatrix is a handle to multi-item matrix set reference allocated from MatrixPool
// and used by renderer back-end. Using handles allows matrix data set
// to have optional components (Cxforms and T0/T1 texture matrices are not
// allocated unless used) and allows matrix memory to be defragmented
// through a memory squeeze.

class HMatrix : public HMatrixConstants
{
    friend class  MatrixPool;
    friend struct DefaultMatrixData;
    EntryHandle *pHandle;

    // Null handle points to identity matrix.
    static EntryHandle NullHandle;

    static ElementIndex toTextureElementIndex(unsigned index)
    {
        SF_ASSERT(index < 2);
        return (ElementIndex)(Element_T0 + index);
    }

    HMatrix(EntryHandle* handle) : pHandle(handle) { }

public:    
    HMatrix() : pHandle(&NullHandle) { }
    HMatrix(const HMatrix& other) : pHandle(other.pHandle)
    {          
        if (pHandle != &NullHandle)
            pHandle->AddRef();
    }
    ~HMatrix()
    {
        if (pHandle != &NullHandle)
            pHandle->Release();
    }

    void Clear()
    {
        if (pHandle != &NullHandle)
        {
            pHandle->Release();
            pHandle = &NullHandle;
        }        
    }

    bool IsNull() const { return pHandle == &NullHandle; }

    void operator = (const HMatrix& other);
    // Pointer value compares
    bool operator == (const HMatrix& other) const { return pHandle == other.pHandle; }
    bool operator != (const HMatrix& other) const { return pHandle != other.pHandle; }

    // Setters are in source file.
    void            SetMatrix2D(const Matrix2F& m);
    void            SetCxform(const Cxform& m);
    void            SetTextureMatrix(const Matrix2F& m, unsigned index);
    void            SetMatrix3D(const Matrix3F& m);
    // Writes user data into HMatrix. Size limited to Pool_UserDataSize (16*3 bytes)
    // Initially added to support distance field font shader arguments.
    void            SetUserData(const void* data, UPInt size);
    void            SetUserDataFloat(const float *data, UPInt count)
    { SetUserData(data, count * sizeof(float)); }
    
    const Matrix3F& GetMatrix3D() const 
    {
        if (!Has3D())
            return Matrix3F::Identity;
        return *pHandle->GetMatrix3DPtr();    
    }
    
    const Matrix2F& GetMatrix2D() const
    {   // No extra check needed since NullHandle contains identity matrix data.
        return *pHandle->GetMatrixPtr();
    }
    const Cxform&   GetCxform() const
    {
        if (!pHandle->FormatHasElement(Element_Cxform))
            return Cxform::Identity;
        return *pHandle->GetCxformPtr();
    }
    const Matrix2F& GetTextureMatrix(unsigned index) const
    {
        ElementIndex e = toTextureElementIndex(index);
        if (!pHandle->FormatHasElement(e))
            return Matrix2F::Identity;
        return *pHandle->GetTextureMatrixPtr(e);
    }

    // Returns user data pointer or Null if not available.
    const void*     GetUserData() const
    {
        if (!pHandle->FormatHasElement(Element_UserData))
            return 0;
        return pHandle->GetUserDataPtr();
    }
    const float*    GetUserDataFloat() const
    {
        return (const float*)GetUserData();
    }

    unsigned  GetFormatBits() const
    {
        return pHandle->GetFormatBits();
    }
    bool HasCxform() const
    {
        return pHandle->FormatHasElement(Element_Cxform);
    }
    bool HasTextureMatrix(unsigned index) const
    {
        return pHandle->FormatHasElement(toTextureElementIndex(index));
    }
    bool HasUserData() const
    {
        return pHandle->FormatHasElement(Element_UserData);
    }

    bool Has3D() const { return (GetFormatBits() & Has_3D) != 0; }
};


//--------------------------------------------------------------------
// ***** MatrixPool
//--------------------------------------------------------------------

// MatrixPool is a squeezable matrix pool maintained by renderer; typically
// there is only a single shared instance of this class. Matrix data is
// allocated from this class by calling CreateMatrix(); this data is later
// referenced and modified through handles.

class MatrixPool : public RefCountBase<MatrixPool, StatRender_MatrixPool_Mem>,
                   public HMatrixConstants
{
    friend struct EntryHandle;
    friend class  EntryHandleTable;
    friend class  DataHeader;
public:

    MatrixPool(MemoryHeap* pheap);
    ~MatrixPool();

    HMatrix  CreateMatrix(unsigned formatBits = Has_2D);
    HMatrix  CreateMatrix(const Matrix2F& m, unsigned formatBits = Has_2D);
    HMatrix  CreateMatrix(const Matrix3F& m, unsigned formatBits = Has_3D);
    HMatrix  CreateMatrix(const Matrix2F& m, const Cxform& cx,
                          unsigned formatBits = Has_2D|Has_Cxform);
    HMatrix  CreateMatrix(const Matrix3F& m, const Cxform& cx,
                          unsigned formatBits = Has_3D|Has_Cxform);

    enum SqueezeType
    {
        Squeeze_Incremental, 
        Squeeze_Full,
        Squeeze_Force
    };

    bool    MemorySqueeze(SqueezeType type);
    
    UPInt   GetAllocatedSpace() const { return AllocatedSpace; }

private:

    MemoryHeap* pHeap;
    // Total user space in all allocated data pages,
    // (not counting page headers).
    UPInt       AllocatedSpace;
    UPInt       DataPageCount; // Informational tracking.
    // Total freed space in all allocated data pages.
    UPInt       FreedSpace;    

    EntryHandleTable HandleTable;    

    // Chain of allocated data pages.
    List<DataPage> DataPages;

    // Page from which we allocate, can be one of the following:
    //  - This can be null, if not initialized yet. 
    //  - Point to Last page in DataPages.
    //  - Point to the last squeeze location.
    DataPage* pAllocPage;
    // The first page from which squeeze is expected to resume, if not null.
    DataPage* pSqueezePage;
    // Single cached "free page", if not null.
    DataPage* pLastFreedPage;


    MatrixPool* getThis() { return this; }

    EntryHandle* createMatrixHelper(const Matrix2F& m, const Cxform& cx,
                                    unsigned formatBits);
    EntryHandle* createMatrixHelper(const Matrix3F& m, const Cxform& cx,
                                    unsigned formatBits);

    // Allocates data page adding it to the end of the list.
    bool        allocDataPage();
    void        freeDataPage(DataPage* page);

    bool        ensureAllocSpace(UPInt size);
    // Allocates data from last page
    DataHeader* allocData(UPInt size, EntryHandle* handle = 0);
    // Allocates a handle and data for new matrix
    EntryHandle* allocMatrixData(unsigned formatBits);

    bool        reallocMatrixData(EntryHandle* handle, unsigned formatBits);

    bool        squeezeMemoryRange(DataPage* squeezeStart, DataPage* squeezeEnd,
                                   SqueezeType type);
};

//#define SF_RENDER_MATRIXPOOL_UNITTEST
#ifdef  SF_RENDER_MATRIXPOOL_UNITTEST
void UnitTest();
#endif


}}} // Scaleform::Render::MatrixPoolImpl

#endif
