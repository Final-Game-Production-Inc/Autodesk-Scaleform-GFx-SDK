/**************************************************************************

Filename    :   AS3_ObjCollector.h
Content     :   A collector of objects to track object dependencies
Created     :   
Authors     :   Artem Bolgar

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFx_AS3_ObjCollector_H
#define INC_SF_GFx_AS3_ObjCollector_H

#include "GFx/GFx_Types.h"
#include "Kernel/SF_Hash.h"
#include "GFx/AS3/AS3_Object.h"

#if defined(SF_BUILD_DEBUG) || defined(SF_BUILD_DEBUGOPT)
#define SF_OBJ_COLLECTOR_ENABLED
#endif

namespace Scaleform {
namespace GFx {

class MovieDef;
class FontHandle;

namespace AS3 {

#ifdef SF_OBJ_COLLECTOR_ENABLED
class AvmDisplayObj;
class MovieRoot;

//#define SF_ENABLE_GRAPHVIZ

class ObjectCollector
{
    friend class ASRefCountCollector;

    struct ObjDesc
    {
        union
        {
            const RefCountBaseGC<Mem_Stat>* pAS3Obj;
            const AvmDisplayObj*            pDispObj;
            void*                           pObjVoid;
            const GFx::FontHandle*          pFontHandle;
            const GFx::MovieDef*            pMovieDef;
            UPInt                           RawValue;
        };
        union
        {
            const RefCountBaseGC<Mem_Stat>* pAS3ObjParent;
            const AvmDisplayObj*            pDispObjParent;
            const void*                     pParentVoid;
            const GFx::FontHandle*          pFontHandleParent;
            const GFx::MovieDef*            pMovieDefParent;
        };
        // a textual desc of the parent (source of the link).
        // Useful when parent is null, but we need to provide info where
        // the link came from (MovieRoot, VM, etc)
        String  ParentDesc;
        enum 
        {
            Obj_AS3Obj, 
            Obj_DispObj,
            Obj_FontHandle,
            Obj_MovieDef
        };
        UInt8 ObjType;     // see Obj_<> enum
        UInt8 ParentType;  // see Obj_<> enum

        bool operator==(const ObjDesc& o) const { return RawValue == o.RawValue; }
        bool operator==(const void* o) const { return pObjVoid == o; }
        ObjDesc& operator=(const ObjDesc& o)
        {
            pObjVoid = o.pObjVoid;
            pParentVoid = o.pParentVoid;
            ParentDesc = o.ParentDesc;
            ObjType = o.ObjType;
            ParentType = o.ParentType;
            return *this;
        }
    };
    struct HashF
    {
        UPInt  operator() (const ObjDesc& data) const
        {
            return data.RawValue;
        }
        UPInt  operator() (const void* data) const
        {
            union
            {
                const void*     pObjVoid;
                UPInt           RawValue;
            } u;
            u.pObjVoid = data;
            return u.RawValue;
        }

    };
    typedef HashSetUncached<ObjDesc,
        HashF,
        HashF>              ObjHash;
    typedef HashUncached<const ObjDesc*, AmpMovieObjectDesc*,
        HashF>              ConstObjMap;

    struct ObjDescArray : public ArrayCC<ObjDesc>
    {
        volatile UPInt      InsertPos;
        volatile UPInt      CursorPos;

        ObjDescArray() : ArrayCC<ObjDesc>(ObjDesc()), InsertPos(0), CursorPos(0) {}

        void InsertTemp(ObjDesc& od);
        void Clear() { ArrayCC<ObjDesc>::Clear(); InsertPos = CursorPos = 0; }

        void Process(ObjectCollector& c, ASRefCountCollector& asc);
    };
public:
    ObjectCollector();

    void AddToProcess(const AvmDisplayObj* obj, const String& desc);
    void AddToProcess(const RefCountBaseGC<Mem_Stat>* obj, const String& desc);
    void AddToProcess(const GFx::MovieDef* fh, const String& desc);

    void AddToProcess(const AvmDisplayObj* obj, const AvmDisplayObj* parent);
    void AddToProcess(const RefCountBaseGC<Mem_Stat>* obj, const RefCountBaseGC<Mem_Stat>* parent);
    void AddToProcess(const AvmDisplayObj* obj, const RefCountBaseGC<Mem_Stat>* parent);
    void AddToProcess(const RefCountBaseGC<Mem_Stat>* obj, const AvmDisplayObj* parent);

    void AddToProcess(const GFx::FontHandle* fh, const GFx::MovieDef* parent);
    void AddToProcess(const GFx::MovieDef* md, const GFx::FontHandle* parent);
    void AddToProcess(const GFx::MovieDef* md, const GFx::MovieDef* parent, const char* descr = 0);

    //const ObjDesc* Find() const;
    void Dump(UInt32 flags, Log* log, const char* swfName) const;
    void GetTree(Array<Ptr<AmpMovieObjectDesc> >* rootObjects, MemoryHeap* heap);
    void Clear();
    void Process(ASRefCountCollector&);

#ifdef SF_ENABLE_GRAPHVIZ
    void GenerateGraphViz(MovieRoot*, const Hash<VMAbcFile*, Array<ObjDesc> >& abcFiles) const;
#endif
protected:
    void InsertTemp(ObjDesc& od, bool lowPrio);
    void TraceBack(Log* log, const ObjDesc& od, const char* prefix, UInt32 flags, const VMAbcFile* abcFile) const;
    void TraceBack(Log* log, const ObjDesc& od, const char* prefix, UInt32 flags, const GFx::MovieDef* movieDef) const;
    void CollectImports(const ObjDesc& od);
    AmpMovieObjectDesc* CreateAmpDesc(const ObjDesc* currentObj, ConstObjMap& objectsDone, AmpMovieObjectDesc* childDesc, Array<Ptr<AmpMovieObjectDesc> >* rootObjects, MemoryHeap* heap);
public:
    class ASCollector* pASCollector;

private:
    ObjHash             Objects;

    ObjDescArray        TempObjects;
    ObjDescArray        TempObjectsLowPrio;
    bool                CollectingLowPrio;
};
#endif //#ifdef SF_OBJ_COLLECTOR_ENABLED

}}}

#endif //INC_SF_GFx_AS3_ObjCollector_H
