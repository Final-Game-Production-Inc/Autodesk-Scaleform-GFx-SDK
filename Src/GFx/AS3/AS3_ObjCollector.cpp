/**************************************************************************

Filename    :   AS3_ObjCollector.cpp
Content     :   A collector of objects to track object dependencies
Created     :   
Authors     :   Artem Bolgar

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_ObjCollector.h"
#include "AS3_MovieRoot.h"
#include "AS3_IntervalTimer.h"

namespace Scaleform {
namespace GFx {
namespace AS3 {

namespace SF = Scaleform;

#ifdef SF_OBJ_COLLECTOR_ENABLED
static String TreatFileName(const Scaleform::String& fileName, UInt32 flags)
{
    if (flags & Movie::Report_ShortFileNames)
    {
        UPInt len = fileName.GetLength();
        for (SPInt i = (SPInt)len - 1; i >= 0; --i)
        {
            if (fileName[UPInt(i)] == '\\' || fileName[UPInt(i)] == '/' || fileName[UPInt(i)] == ':')
            {
                return String(fileName.ToCStr() + i + 1);
            }
        }
    }
    return fileName;
}

void ObjectCollector::ObjDescArray::InsertTemp(ObjDesc& od)
{
    SF_ASSERT(CursorPos <= InsertPos);

    if (CursorPos > 0 && InsertPos == GetSize())
    {
        RemoveMultipleAt(0, CursorPos);
        InsertPos = (InsertPos - CursorPos);
        CursorPos = 0;
    }
    InsertAt(InsertPos++, od);
}

void ObjectCollector::CollectImports(const ObjDesc& od)
{
    struct Visitor : public MovieDef::ImportVisitor
    {
        ObjectCollector& C;
        Visitor(ObjectCollector& c):C(c) {}

        virtual void    Visit(MovieDef* pparentDef, MovieDef* pimportDef,
            const char* )
        {
            C.AddToProcess(pimportDef, pparentDef, "imports");    
        }
    private:
        Visitor& operator=(const Visitor&) { return *this; }
    } visitor(*this);
    const_cast<GFx::MovieDef*>(od.pMovieDef)->VisitImportedMovies(&visitor);
}

void ObjectCollector::ObjDescArray::Process(ObjectCollector& c, ASRefCountCollector& asc)
{
    // Processing TempObjects array, executing either AvmDisplayObj::CollectChildren or
    // RefCountBaseGC<Mem_Stat>::ForEachChild_GC (in conjunction with passed ASRefCountCollector)
    // filling up Objects hash table. The algorithm is non recursive, it uses TempObjects
    // array to avoid recursion.
    while (CursorPos < InsertPos)
    {
        ObjDesc od = At(CursorPos++);

        if (!c.Objects.Get(od))
        {
            c.Objects.Add(od);

            if (od.ObjType == ObjDesc::Obj_DispObj)
            {
                od.pDispObj->CollectChildren(c);
            }
            else if (od.ObjType == ObjDesc::Obj_AS3Obj)
            {
                asc.CollectChildren(od.pAS3Obj);
            }
            else if (od.ObjType == ObjDesc::Obj_MovieDef)
            {
                // collect imports
                c.CollectImports(od);
            }
            else if (od.ObjType == ObjDesc::Obj_FontHandle)
            {
                if (od.pFontHandle->pSourceMovieDef)
                {
                    c.AddToProcess(od.pFontHandle->pSourceMovieDef, od.pFontHandle);
                }
            }
            //else
            //    SF_ASSERT(0); //?
        }
    }
    Clear();
}

ObjectCollector::ObjectCollector() 
{
    CollectingLowPrio = false;
}

void ObjectCollector::InsertTemp(ObjDesc& od, bool lowPrio)
{
    if (!lowPrio)
        TempObjects.InsertTemp(od);
    else
        TempObjectsLowPrio.InsertTemp(od);
}

void ObjectCollector::Clear()
{
    pASCollector = NULL;
    TempObjects.Clear();
    TempObjectsLowPrio.Clear();
    Objects.Clear();
}

void ObjectCollector::Process(ASRefCountCollector& asc)
{
    CollectingLowPrio = false;
    TempObjects.Process(*this, asc);

    CollectingLowPrio = true;
    TempObjectsLowPrio.Process(*this, asc);
    CollectingLowPrio = false;
}

void ObjectCollector::AddToProcess(const RefCountBaseGC<Mem_Stat>* obj, const String& desc)
{
    if (!obj) return;
    ObjDesc od;
    od.ObjType = ObjDesc::Obj_AS3Obj;
    od.pAS3Obj = obj;
    od.pDispObjParent = NULL;
    od.ParentDesc = desc;
    od.ParentType = ObjDesc::Obj_DispObj;
    InsertTemp(od, CollectingLowPrio);
}

void ObjectCollector::AddToProcess(const AvmDisplayObj* obj, const String& desc)
{
    if (!obj) return;
    ObjDesc od;
    od.ObjType = ObjDesc::Obj_DispObj;
    od.pDispObj = obj;
    od.pDispObjParent = NULL;
    od.ParentDesc = desc;
    od.ParentType = ObjDesc::Obj_DispObj;
    InsertTemp(od, CollectingLowPrio);
}

void ObjectCollector::AddToProcess(const RefCountBaseGC<Mem_Stat>* obj, const AvmDisplayObj* parent)
{
    if (!obj) return;
    ObjDesc od;
    od.ObjType = ObjDesc::Obj_AS3Obj;
    od.pAS3Obj = obj;
    od.pDispObjParent = parent;
    od.ParentType = ObjDesc::Obj_DispObj;
    InsertTemp(od, CollectingLowPrio);
}

void ObjectCollector::AddToProcess(const AvmDisplayObj* obj, const AvmDisplayObj* parent)
{
    if (!obj) return;
    ObjDesc od;
    od.ObjType = ObjDesc::Obj_DispObj;
    od.pDispObj = obj;
    od.pDispObjParent = parent;
    od.ParentType = ObjDesc::Obj_DispObj;
    InsertTemp(od, CollectingLowPrio);
}

void ObjectCollector::AddToProcess(const RefCountBaseGC<Mem_Stat>* obj, const RefCountBaseGC<Mem_Stat>* parent)
{
    if (!obj) return;
    ObjDesc od;
    od.ObjType = ObjDesc::Obj_AS3Obj;
    od.pAS3Obj = obj;
    od.pAS3ObjParent = parent;
    od.ParentType = ObjDesc::Obj_AS3Obj;
    InsertTemp(od, CollectingLowPrio);
}

void ObjectCollector::AddToProcess(const AvmDisplayObj* obj, const RefCountBaseGC<Mem_Stat>* parent)
{
    if (!obj) return;
    ObjDesc od;
    od.ObjType = ObjDesc::Obj_DispObj;
    od.pDispObj = obj;
    od.pAS3ObjParent = parent;
    od.ParentType = ObjDesc::Obj_AS3Obj;
    InsertTemp(od, CollectingLowPrio);
}

void ObjectCollector::AddToProcess(const GFx::MovieDef* md, const String& desc)
{
    if (!md) return;
    ObjDesc od;
    od.ObjType = ObjDesc::Obj_MovieDef;
    od.pMovieDef = md;
    od.pDispObjParent = NULL;
    od.ParentDesc = desc;
    od.ParentType = ObjDesc::Obj_DispObj;
    InsertTemp(od, CollectingLowPrio);
}

void ObjectCollector::AddToProcess(const GFx::FontHandle* fh, const GFx::MovieDef* parent)
{
    if (!fh) return;
    ObjDesc od;
    od.ObjType = ObjDesc::Obj_FontHandle;
    od.pFontHandle = fh;
    od.pMovieDefParent = parent;
    od.ParentType = ObjDesc::Obj_MovieDef;
    InsertTemp(od, CollectingLowPrio);
}

void ObjectCollector::AddToProcess(const GFx::MovieDef* md, const GFx::MovieDef* parent, const char* descr)
{
    if (!md) return;
    ObjDesc od;
    od.ObjType = ObjDesc::Obj_MovieDef;
    od.pMovieDef = md;
    od.pMovieDefParent = parent;
    if (descr)
        od.ParentDesc = descr;
    od.ParentType = ObjDesc::Obj_MovieDef;
    InsertTemp(od, CollectingLowPrio);
}

void ObjectCollector::AddToProcess(const GFx::MovieDef* md, const GFx::FontHandle* parent)
{
    if (!md) return;
    ObjDesc od;
    od.ObjType = ObjDesc::Obj_MovieDef;
    od.pMovieDef = md;
    od.pFontHandleParent = parent;
    od.ParentType = ObjDesc::Obj_FontHandle;
    InsertTemp(od, CollectingLowPrio);
}

void ObjectCollector::TraceBack(Log* log, const ObjDesc& od, const char* prefix, UInt32 flags,
                                const VMAbcFile* abcFile) const
{
    const ObjDesc* cur = &od;
    if (flags & Movie::Report_NoCircularReferences && abcFile)
    {
        // figure out, do we have the VMAbcFile in the chain of parents.
        // if so, skip the whole chain.

        for(int i = 0; cur ; ++i)
        {
            if (i > 0 && cur->ObjType == ObjDesc::Obj_AS3Obj)
            {
                if (strcmp(cur->pAS3Obj->GetAS3ObjectType(), "VMAbcFile") == 0)
                {
                    if (cur->pAS3Obj->GetAS3ObjectName() == abcFile->GetAbcFile().GetName())
                        return;
                }
            }
            // find a parental ObjDesc
            if (cur->pParentVoid)
                cur = Objects.GetAlt(cur->pParentVoid);
            else
                break;
        }
    }
    cur = &od;
    for(int i = 0; cur ; ++i)
    {
        if (i)
            log->LogMessage(" <- ");
        else
            log->LogMessage("%s", prefix);
        if (cur->ObjType == ObjDesc::Obj_DispObj)
        {
            ASString s = cur->pDispObj->GetName();
            if ((flags & Movie::Report_AddressesForAnonymObjsOnly) && !s.IsEmpty())
                log->LogMessage("%s", cur->pDispObj->GetObjectTypeName());
            else
                log->LogMessage("%s@%p", cur->pDispObj->GetObjectTypeName(), cur->pDispObj);
            if (!s.IsEmpty())
            {
                log->LogMessage("(%s)", s.ToCStr());
            }
        }
        else if (cur->ObjType == ObjDesc::Obj_AS3Obj)
        {
            Scaleform::String s = cur->pAS3Obj->GetAS3ObjectName();
            if ((flags & Movie::Report_AddressesForAnonymObjsOnly) && !s.IsEmpty())
                log->LogMessage("%s", cur->pAS3Obj->GetAS3ObjectType());
            else
                log->LogMessage("%s@%p", cur->pAS3Obj->GetAS3ObjectType(), cur->pAS3Obj);
            if (!s.IsEmpty())
            {
                if (strcmp(cur->pAS3Obj->GetAS3ObjectType(), "VMAbcFile") == 0 ||
                    strcmp(cur->pAS3Obj->GetAS3ObjectType(), "Instances::fl::GlobalObjectScript") == 0)
                {
                    // Treat name of the file for VMAbcFile.
                    log->LogMessage("(%s)", TreatFileName(s, flags).ToCStr());
                }
                else
                    log->LogMessage("(%s)", s.ToCStr());
            }
        }

        // find a parental ObjDesc
        if (cur->pParentVoid)
            cur = Objects.GetAlt(cur->pParentVoid);
        else
        {
            if (!cur->ParentDesc.IsEmpty())
                log->LogMessage(" <- %s", cur->ParentDesc.ToCStr());
            break;
        }

    }
    log->LogMessage("\n");
}

void ObjectCollector::TraceBack(Log* log, const ObjDesc& od, const char* prefix, UInt32 flags,
                                const GFx::MovieDef* movieDef) const
{
    const ObjDesc* cur = &od;
    cur = &od;
    if (od.ObjType == ObjDesc::Obj_DispObj || od.ObjType == ObjDesc::Obj_AS3Obj)
    {
        TraceBack(log, od, prefix, flags, static_cast<const VMAbcFile*>(0));
        return;
    }
    for(int i = 0, j = 0; cur ; ++i)
    {
        if (j)
            log->LogMessage(" <- ");
        else if (!i)
        {
            log->LogMessage("%s", prefix);
            if (od.pMovieDef == movieDef)
            {
                if (cur->pParentVoid)
                    cur = Objects.GetAlt(cur->pParentVoid);
                else
                    break;
                continue; // skip
            }
        }
        ++j;
        if (cur->ObjType == ObjDesc::Obj_FontHandle)
        {
            String s = cur->pFontHandle->GetFontName();
            unsigned fontFl = cur->pFontHandle->GetFontFlags();
            if (fontFl & GFx::FontHandle::FF_Bold)
                s += " Bold";
            if (fontFl & GFx::FontHandle::FF_Italic)
                s += " Italic";
            if ((flags & Movie::Report_AddressesForAnonymObjsOnly) && !s.IsEmpty())
                log->LogMessage("Font");
            else
                log->LogMessage("Font@%p", cur->pFontHandle);
            if (!s.IsEmpty())
            {
                log->LogMessage("(%s)", s.ToCStr());
            }
        }
        else if (cur->ObjType == ObjDesc::Obj_MovieDef)
        {
            String s = TreatFileName(cur->pMovieDef->GetFileURL(), flags);
            if ((flags & Movie::Report_AddressesForAnonymObjsOnly) && !s.IsEmpty())
                log->LogMessage("MovieDef");
            else
                log->LogMessage("MovieDef@%p", cur->pMovieDef);
            if (!s.IsEmpty())
            {
                log->LogMessage("(%s)", s.ToCStr());
            }
        }

        // find a parental ObjDesc
        if (cur->pParentVoid)
            cur = Objects.GetAlt(cur->pParentVoid);
        else
        {
            if (!cur->ParentDesc.IsEmpty())
                log->LogMessage(" <- %s", cur->ParentDesc.ToCStr());
            break;
        }
    }
    log->LogMessage("\n");
}

#ifdef SF_ENABLE_GRAPHVIZ
void ObjectCollector::GenerateGraphViz(MovieRoot* root, const Hash<VMAbcFile*, Array<ObjDesc> >& abcFiles) const
{
    const char* url = root->GetMovieDefImpl()->GetFileURL();
    String fn(url);
    fn += ".vg";
    FILE* f = fopen(fn.ToCStr(), "w");
    if (f)
    {
        fprintf(f, "digraph G {\n");

        Array<Scaleform::String> subgraphs;

        // generate subgraphs
        for (Hash<VMAbcFile*, Array<ObjDesc> >::ConstIterator it = abcFiles.Begin(); it != abcFiles.End(); ++it)
        {
            const char* abcFileName = it->First->GetAbcFile().GetName().ToCStr();
            SPInt l = (SPInt)strlen(abcFileName) - 1;
            while(l > 0 && abcFileName[l] != '\\' && abcFileName[l] != '/') --l;
            fprintf(f, "   subgraph %s {\n", abcFileName + l);
            subgraphs.PushBack(String(abcFileName + l));
            
            const Array<ObjDesc>& objs = it->Second;
            // find an object belonging to this VMAbcFile
            for (UPInt j = 0; j < objs.GetSize(); ++j)
            {
                TraceBack(objs[j], "       ");
            }
        }

        for (UPInt i = 0; i < subgraphs.GetSize(); ++i)
        {
            fprintf(f, "   start -> %s\n", subgraphs[i].ToCStr());
        }

        fprintf(f, "   start [shape=Mdiamond];\n");
        //fprintf(f, "   end [shape=Msquare];\n");
        fprintf(f, "}"); 
        fclose(f);
    }
}
#endif

class SortingLog : public SF::Log
{
    SF::ArrayCPP<SF::StringBuffer> Strings;
    unsigned CurIndex;
    Ptr<SF::Log> pLog;
public:
    SortingLog(SF::Log* plog):CurIndex(0), pLog(plog) {}

    // We override this function in order to do custom logging.
    virtual void    LogMessageVarg(SF::LogMessageId messageId, const char* pfmt, va_list argList)
    {
        char buffer[MaxLogBufferMessageSize];
        FormatLog(buffer, sizeof(buffer), messageId, pfmt, argList);

        // find new-line symbols (if any)
        UPInt i = 0, li = 0;
        for (; buffer[i] != '\0'; ++i)
        {
            if (buffer[i] == '\n')
            {
                EnsureStringExists(CurIndex);
                SF::StringBuffer& sbuf = Strings[CurIndex++];
                sbuf.AppendString(buffer + li, i - li + 1);
                sbuf.AppendChar(0);
                li = i + 1;
            }
        }
        if (li < i)
        {
            EnsureStringExists(CurIndex);
            SF::StringBuffer& sbuf = Strings[CurIndex];
            sbuf.AppendString(buffer + li, i - li);
            li = i + 1;
        }
    }
    void EnsureStringExists(unsigned curIndex)
    {
        while (Strings.GetSize() <= curIndex)
            Strings.PushBack(SF::StringBuffer(""));
    }

    static bool StringBuffsLess(const StringBuffer& c1, const StringBuffer& c2)
    {
        // compare strings in reverse (from end to begining)
        int c1i = (int)c1.GetSize() - 1;
        int c2i = (int)c2.GetSize() - 1;
        int res = c1i - c2i;
        for (; c1i >= 0 && c2i >= 0; --c1i, --c2i)
        {
            if (c1[c1i] != c2[c2i])
            {
                res = (int)c1[c1i] - (int)c2[c2i];
                break;
            }
        }

        return res < 0;
    }
    void TreatAndFlush(UInt32 flags)
    {
        // sort the strings
        Alg::QuickSortSafe(Strings, StringBuffsLess);
        
        const StringBuffer* prev = NULL;
        for (UPInt i = 0; i < Strings.GetSize(); ++i)
        {
            const StringBuffer& cur = Strings[i];
            if (prev)
            {
                // compare previous with current, starting from the end 
                // (in order to find a common part for ellipsis).
                int ci = (int)cur.GetSize() - 1;
                int pi = (int)prev->GetSize() - 1;
                int lastArrow = 0;
                for (; pi >= 0 && ci >= 0 && (*prev)[pi] == cur[ci]; --pi, --ci)
                {
                    if (ci > 0 && cur[ci] == '-' && cur[ci - 1] == '<')
                        lastArrow = ci + 1;
                }
                if (pi == ci && pi < 0) // duplicate found, don't display
                    continue;

                if (!(flags & Movie::Report_NoEllipsis) && lastArrow > 0)
                {
                    // find next arrow
                    UPInt uci;
                    for (uci = lastArrow; uci < cur.GetSize(); ++uci)
                    {
                        if (uci + 1 < cur.GetSize() && cur[uci] == '<' && cur[uci + 1] == '-')
                            break;
                    }
                    StringBuffer sbuf(cur);
                    if (uci + 1 < cur.GetSize())
                    {
                        sbuf.Resize(uci + 2);
                        sbuf.AppendString(" ...\n");
                    }
                    pLog->LogMessage("%s", sbuf.ToCStr());
                }
                else
                    pLog->LogMessage("%s", cur.ToCStr());
            }
            else
                pLog->LogMessage("%s", cur.ToCStr());
            prev = &cur;
        }
        Strings.Clear();
    }

    static bool StringBuffsLess2(const StringBuffer& c1, const StringBuffer& c2)
    {
        return SFstrcmp(c1.ToCStr(), c2.ToCStr()) < 0;
    }
    void Flush()
    {
        // sort the strings
        Alg::QuickSortSafe(Strings, StringBuffsLess2);

        for (UPInt i = 0; i < Strings.GetSize(); ++i)
        {
            const StringBuffer& cur = Strings[i];
            pLog->LogMessage("%s", cur.ToCStr());
        }
        Strings.Clear();
    }

};

void ObjectCollector::Dump(UInt32 flags, 
                           Log* log,  
                           const char* swfName) const
{
    // collect statistics first
    unsigned dispObjCnt = 0;
    unsigned as3objCnt = 0;
    unsigned as3DispObjCnt = 0;
    unsigned as3ClassesCnt = 0;
    unsigned as3UserClassesCnt = 0;
    Hash<const VMAbcFile*, Array<ObjDesc> > abcFileNames;
#ifdef SF_OBJECTS_REPORT_OTHERS
    Array<ObjDesc> otherObjects;
#endif
    StringHash<int> as3objsByTypes;
    Hash<const MovieDef*, Array<ObjDesc> > movieDefs;
    for(ObjHash::ConstIterator it = Objects.Begin(); it != Objects.End(); ++it)
    {
        const ObjDesc& od = *it;
        switch(od.ObjType)
        {
            case ObjDesc::Obj_DispObj:
                ++dispObjCnt;
                //printf("$ %s\n", od.pDispObj->GetDispObj()->GetResourceMovieDef()->GetFileURL());
                break;
            case ObjDesc::Obj_AS3Obj:
            {
                ++as3objCnt;

                // save in obj-by-type hash
                String type(od.pAS3Obj->GetAS3ObjectType());
                int* pcnt = as3objsByTypes.Get(type);
                if (pcnt)
                    ++(*pcnt);
                else
                {
                    int cnt = 1;
                    as3objsByTypes.Set(type, cnt); 
                }

                const VMAbcFile* vmabc = NULL;
                const Instances::fl_display::DisplayObject* pas3DispObj = NULL;
                if (od.pAS3Obj->IsAS3Object())
                {
                    const AS3::Object* obj = static_cast<const AS3::Object*>(od.pAS3Obj);
                    if (obj->GetTraits().IsClassTraits())
                    {
                        ++as3ClassesCnt;
                        if (obj->GetTraits().IsUserDefined())
                            ++as3UserClassesCnt;
                    }
                    else if (obj->GetTraitsType() >= Traits_DisplayObject_Begin && obj->GetTraitsType() < Traits_DisplayObject_End)
                    {
                        ++as3DispObjCnt;
                        pas3DispObj = static_cast<const Instances::fl_display::DisplayObject*>(obj);
                        //printf("@ %s\n", static_cast<const Instances::fl_display::DisplayObject*>(obj)->pDispObj->GetResourceMovieDef()->GetFileURL());
                    }
                    vmabc = obj->GetTraits().GetFilePtr();
                }
                else
                {
                    if (!vmabc && strcmp(od.pAS3Obj->GetAS3ObjectType(), "VMAbcFile") == 0)
                        vmabc = static_cast<const VMAbcFile*>(od.pAS3Obj);
                }
                if (vmabc)
                {
                    Array<ObjDesc>* aobj = abcFileNames.Get(vmabc);
                    if (aobj)
                    {
                        aobj->PushBack(od);
                    }
                    else
                    {
                        Array<ObjDesc> ao;
                        ao.PushBack(od);
                        abcFileNames.Set(vmabc, ao);
                    }
                }
                else if (pas3DispObj && pas3DispObj->pDispObj)
                {
                    // AS3 obj w/o Abc: hopefully it has a moviedef...
                    Array<ObjDesc>* aobj = movieDefs.Get(pas3DispObj->pDispObj->GetResourceMovieDef());
                    if (aobj)
                    {
                        aobj->PushBack(od);
                    }
                    else
                    {
                        Array<ObjDesc> ao;
                        ao.PushBack(od);
                        movieDefs.Set(pas3DispObj->pDispObj->GetResourceMovieDef(), ao);
                    }
                }
#ifdef SF_OBJECTS_REPORT_OTHERS
                else
                {
                    otherObjects.PushBack(od);
                }
#endif
                break;
            }
            case ObjDesc::Obj_MovieDef:
            {
                Array<ObjDesc>* aobj = movieDefs.Get(od.pMovieDef);
                if (aobj)
                {
                    aobj->PushBack(od);
                }
                else
                {
                    Array<ObjDesc> ao;
                    ao.PushBack(od);
                    movieDefs.Set(od.pMovieDef, ao);
                }
                break;
            }
            case ObjDesc::Obj_FontHandle:
                {
                    if (od.pFontHandle->pSourceMovieDef)
                    {
                        Array<ObjDesc>* aobj = movieDefs.Get(od.pFontHandle->pSourceMovieDef);
                        if (aobj)
                        {
                            aobj->PushBack(od);
                        }
                        else
                        {
                            Array<ObjDesc> ao;
                            ao.PushBack(od);
                            movieDefs.Set(od.pFontHandle->pSourceMovieDef, ao);
                        }
                    }
                    break;
                }
            default: SF_ASSERT(0);
        }
    }
#ifdef SF_ENABLE_GRAPHVIZ
    GenerateGraphViz(root, abcFileNames);
#endif

    if (!(flags & Movie::Report_SuppressOverallStats))
    {
        log->LogMessage("Total number of objects:    %lu\n", (unsigned long)Objects.GetSize());
        log->LogMessage("   DisplayObjects:          %d\n", dispObjCnt);
        log->LogMessage("   AS3 Objects:             %d\n", as3objCnt);
        log->LogMessage("      Display Objects:      %d\n", as3DispObjCnt);
        log->LogMessage("      Classes:              %d\n", as3ClassesCnt);
        log->LogMessage("         UserDefined:       %d\n", as3UserClassesCnt);
    }

    log->LogMessage("\nAbc Files:                %lu\n", (unsigned long)abcFileNames.GetSize());
    for (Hash<const VMAbcFile*, Array<ObjDesc> >::ConstIterator it = abcFileNames.Begin(); it != abcFileNames.End(); ++it)
    {
        String abcFileName = TreatFileName(it->First->GetAbcFile().GetName().ToCStr(), flags);
        if (swfName)
        {
            // using strstr here, because abcFileName may contain extra stuff at the end (such as
            // ABC block name in []). Not a perfect solution, of course, but should work in most cases.
            if (strstr(abcFileName.ToCStr(), TreatFileName(swfName, flags).ToCStr()) == NULL)
                continue;
        }

        log->LogMessage("\n----------------\n\n");
        log->LogMessage("   %s, %s@%p\n", abcFileName.ToCStr(), it->First->GetAS3ObjectType(), it->First);
        
        Ptr<SF::Log> savedLog = log;
        Ptr<SortingLog> sortLog = *new SortingLog(log);
        log = sortLog;

        // find objects belonging to this VMAbcFile
        for (UPInt j = 0; j < it->Second.GetSize(); ++j)
        {
            TraceBack(log, it->Second[j], "       ", flags, it->First);
        }
        sortLog->TreatAndFlush(flags);
        log = savedLog;
    }

#ifdef SF_OBJECTS_REPORT_OTHERS
    log->LogMessage("\nOther objects:\n");
    log->LogMessage("\n----------------\n\n");
    {
        Ptr<SF::Log> savedLog = log;
        Ptr<SortingLog> sortLog = *new SortingLog(log);
        log = sortLog;

        // find objects belonging to this VMAbcFile
        for (UPInt j = 0; j < otherObjects.GetSize(); ++j)
        {
            TraceBack(log, otherObjects[j], "       ", flags, static_cast<VMAbcFile*>(NULL));
        }
        sortLog->TreatAndFlush(flags);
        log = savedLog;
    }
#endif

    if (!(flags & Movie::Report_SuppressMovieDefsStats) && movieDefs.GetSize() > 1)
    {
        log->LogMessage("\nMovieDefs:                %lu", (unsigned long)movieDefs.GetSize());
        for (Hash<const MovieDef*, Array<ObjDesc> >::ConstIterator it = movieDefs.Begin(); it != movieDefs.End(); ++it)
        {
            if (!it->First)
                continue;
            String fileName = TreatFileName(it->First->GetFileURL(), flags);

            log->LogMessage("\n----------------\n\n");
            log->LogMessage("   %s @%p\n", fileName.ToCStr(), it->First);

            Ptr<SF::Log> savedLog = log;
            Ptr<SortingLog> sortLog = *new SortingLog(log);
            log = sortLog;

            // find objects belonging to this VMAbcFile
            for (UPInt j = 0; j < it->Second.GetSize(); ++j)
            {
                TraceBack(log, it->Second[j], "       ", flags, it->First);
            }
            sortLog->TreatAndFlush(flags);
            log = savedLog;
        }
    }

    if (!(flags & Movie::Report_SuppressOverallStats))
    {
        log->LogMessage("\nAS3 objs, %lu\n", (unsigned long)as3objsByTypes.GetSize());
        Ptr<SF::Log> savedLog = log;
        Ptr<SortingLog> sortLog = *new SortingLog(log);
        log = sortLog;
        for (StringHash<int>::ConstIterator it = as3objsByTypes.Begin(); it != as3objsByTypes.End(); ++it)
        {
            UPInt l = it->First.GetLength();
            log->LogMessage("   %s", it->First.ToCStr());
            char buf[61];
            UPInt fillLen = 1;
            if (l < 60)
                fillLen = 60 - l;
            memset(buf, ' ', fillLen);
            buf[fillLen] = 0;
            log->LogMessage("%s%d\n", buf, it->Second);
        }
        sortLog->Flush();
        log = savedLog;
    }

#if 0
    for(ObjHash::ConstIterator it = Objects.Begin(); it != Objects.End(); ++it)
    {
        if (it->ObjType == ObjDesc::Obj_AS3Obj)
        {
            const ObjDesc& od = *it;
            const RefCountBaseGC<Mem_Stat>* p = od.pAS3Obj;
            if (p->IsAS3Object())
            {
//                const AS3::Object* obj = static_cast<const AS3::Object*>(p);
//                VMAbcFile* vmabc = obj->GetTraits().GetFilePtr();
//                if (vmabc)
//                    ;//printf("%s\n", vmabc->GetAbcFile().GetName().ToCStr());
            }
        }
    }
#endif
    return;
}

AmpMovieObjectDesc* ObjectCollector::CreateAmpDesc(const ObjDesc* currentObj, ConstObjMap& objectsDone, AmpMovieObjectDesc* childDesc, Array<Ptr<AmpMovieObjectDesc> >* rootObjects, MemoryHeap* heap)
{
    bool foundExisting = false;
    Ptr<AmpMovieObjectDesc> ampDesc;
    AmpMovieObjectDesc** ampDescFound = objectsDone.Get(currentObj);
    if (ampDescFound != NULL)
    {
        foundExisting = true;
        ampDesc = *ampDescFound;
    }
    else
    {
        ampDesc = *SF_HEAP_NEW(heap) AmpMovieObjectDesc();
        switch (currentObj->ObjType)
        {
        case ObjDesc::Obj_AS3Obj:
            ampDesc->Description = currentObj->pAS3Obj->GetAS3ObjectName().ToCStr();
            if (ampDesc->Description.IsEmpty())
            {
                ampDesc->Description = currentObj->pAS3Obj->GetAS3ObjectType();
            }
            else
            {
                ampDesc->Description.AppendChar('(');
                ampDesc->Description.AppendString(currentObj->pAS3Obj->GetAS3ObjectType());
                ampDesc->Description.AppendChar(')');
            }
            break;
        case ObjDesc::Obj_DispObj:
            ampDesc->Description = currentObj->pDispObj->GetName().ToCStr();
            if (ampDesc->Description.IsEmpty())
            {
                ampDesc->Description = currentObj->pDispObj->GetObjectTypeName();
            }
            else
            {
                ampDesc->Description.AppendChar('(');
                ampDesc->Description.AppendString(currentObj->pDispObj->GetObjectTypeName());
                ampDesc->Description.AppendChar(')');
            }
            break;
        case ObjDesc::Obj_FontHandle:
            ampDesc->Description = currentObj->pFontHandle->GetFontName();
            if (ampDesc->Description.IsEmpty())
            {
                ampDesc->Description = "Font";
            }
            if (currentObj->pFontHandle->GetFontFlags() & GFx::FontHandle::FF_Bold)
            {
                ampDesc->Description += " Bold";
            }
            if (currentObj->pFontHandle->GetFontFlags() & GFx::FontHandle::FF_Italic)
            {
                ampDesc->Description += " Italic";
            }
            break;
        case ObjDesc::Obj_MovieDef:
            ampDesc->Description = currentObj->pMovieDef->GetFileURL();
            if (ampDesc->Description.IsEmpty())
            {
                ampDesc->Description = "MovieDef";
            }
            break;
        default:
            SF_ASSERT(false);
            break;
        }
    }

    if (childDesc != NULL)
    {
        ampDesc->Children.PushBack(childDesc);
    }

    if (foundExisting)
    {
        return NULL;
    }

    objectsDone.Add(currentObj, ampDesc);

    AmpMovieObjectDesc* parentDesc = NULL;
    if (currentObj->pParentVoid != NULL)
    {
        const ObjDesc* parentObj = Objects.GetAlt(currentObj->pParentVoid);
        SF_ASSERT(parentObj != NULL);
        if (parentObj != NULL)
        {
            parentDesc = CreateAmpDesc(parentObj, objectsDone, ampDesc, rootObjects, heap);
        }
    }
    else
    {
        AmpMovieObjectDesc* rootDesc = NULL;
        for (UPInt i = 0; i < rootObjects->GetSize(); ++i)
        {
            if (rootObjects->At(i)->Description == currentObj->ParentDesc)
            {
                rootDesc = rootObjects->At(i);
                break;
            }
        }
        if (rootDesc == NULL)
        {
            rootDesc = SF_HEAP_NEW(heap) AmpMovieObjectDesc();
            rootDesc->Description = currentObj->ParentDesc;
            parentDesc = rootDesc;
        }

        rootDesc->Children.PushBack(ampDesc);
    }
    return parentDesc;
}


void ObjectCollector::GetTree(Array<Ptr<AmpMovieObjectDesc> >* rootObjects, MemoryHeap* heap)
{
    ConstObjMap objectsDone;
    for (ObjHash::ConstIterator it = Objects.Begin(); it != Objects.End(); ++it)
    {
        const ObjDesc& od = *it;
        AmpMovieObjectDesc* rootObj = CreateAmpDesc(&od, objectsDone, NULL, rootObjects, heap);
        if (rootObj != NULL)
        {
            rootObjects->PushBack(*rootObj);
        }
    }
}


//////////////////////////////////////////////////////////////////////////

void AvmDisplayObj::CollectChildren(class ObjectCollector& oc) const
{
    if (pAS3CollectiblePtr)
    {
        // the display obj holds AS3 object strongly; i.e. the child is the AS3 obj
        // and the parent is 'this' display obj.
        oc.AddToProcess(pAS3CollectiblePtr, this);
    }
    if (pAS3RawPtr)
    {
        // the display obj holds AS3 object weakly; meaning, it is a child of the
        // AS3 obj
        oc.AddToProcess(this, pAS3RawPtr);
    }
}

void AvmDisplayObjContainer::CollectChildren(class ObjectCollector& oc) const
{
    AvmInteractiveObj::CollectChildren(oc);

    const DisplayObjContainer* dobjParent = GetDisplayObjContainer();
    const DisplayList& dl = dobjParent->GetDisplayList();
    for (SPInt i = 0, n = (SPInt)dl.GetCount(); i < n; ++i)
    {
        DisplayObjectBase* pch = dl.GetDisplayObject(i);
        if (pch->IsScriptableObject())
        {
            oc.AddToProcess(ToAvmDisplayObj(pch->CharToScriptableObject_Unsafe()), this);
            ToAvmDisplayObj(pch->CharToScriptableObject_Unsafe())->CollectChildren(oc);
        }
    }
}

class ASCollector : public ASRefCountCollector
{
public:
    ObjectCollector& ObjCollector;
    String           CurrentDesc;

    ASCollector(ObjectCollector& c);
};

static void CollectorFunction(RefCountCollector<Mem_Stat>* prcc, 
                              const RefCountBaseGC<Mem_Stat>** pchild 
                              SF_DEBUG_ARG(const RefCountBaseGC<Mem_Stat>& owner))
{
    ASCollector* c = static_cast<ASCollector*>(prcc);
    if (&owner != NULL)
        c->ObjCollector.AddToProcess(*pchild, &owner);
    else if (!c->CurrentDesc.IsEmpty())
        c->ObjCollector.AddToProcess(*pchild, c->CurrentDesc);
    else
        SF_ASSERT(0); // WTF?
//     if (strcmp((*pchild)->GetAS3ObjectType(), "VMAbcFile") == 0)
//     {
//         printf("&&& %s\n", (*pchild)->GetAS3ObjectName().ToCStr());
//         printf("");
//         //const VMAbcFile* vmabc = static_cast<const VMAbcFile*>(od.pAS3Obj);
//     }

    if ((*pchild)->IsAS3Object())
    {
        const AS3::Object* obj = static_cast<const AS3::Object*>(*pchild);
        BuiltinTraitsType objType = obj->GetTraitsType();
        // make sure the object is a DisplayObject...
        if (obj->GetTraits().IsInstanceTraits() && 
            objType >= Traits_DisplayObject_Begin && objType < Traits_DisplayObject_End)
        {
            const Instances::fl_display::DisplayObject* das3obj = 
                static_cast<const Instances::fl_display::DisplayObject*>(obj);
            // check if GFx::DisplayObject holds the das3obj strongly or weakly. 
            // This will define, who is an object and who is a parent.
            // If there is no collectible (strong) reference from GFx::DisplayObject to AS3 object then
            // the AS3 object is an owner of the GFx::DisplayObject.
            if (!ToAvmDisplayObj(das3obj->pDispObj)->GetCollectibleAS3Ptr())
            {
                c->ObjCollector.AddToProcess(ToAvmDisplayObj(das3obj->pDispObj), das3obj);
            }
        }
    }
}
ASCollector::ASCollector(ObjectCollector& c) : ObjCollector(c) 
{
    CollectorFunc = CollectorFunction;
}

void IntervalTimer::Collect(ObjectCollector& c)
{
    ASCollector& asc = *c.pASCollector;
    c.AddToProcess(TimerObj, "IntervalTimer.TimerObj");
    asc.CurrentDesc = "IntervalTimer.Function";
    AS3::ForEachChild_GC(&asc, Function, CollectorFunction 
                         SF_DEBUG_ARG(*reinterpret_cast<const RefCountBaseGC<Mem_Stat>*>((const char*)1 - 1)));
    for (UPInt j = 0; j < Params.GetSize(); ++j)
    {
        asc.CurrentDesc = "IntervalTimer.Params";
        AS3::ForEachChild_GC(&asc, Params[j], CollectorFunction 
                            SF_DEBUG_ARG(*reinterpret_cast<const RefCountBaseGC<Mem_Stat>*>((const char*)1 - 1)));
    }
    asc.CurrentDesc = "";
}

void ASRefCountCollector::CollectRoots(ObjectCollector& c)
{
    ASCollector& asc = *c.pASCollector;
    asc.CurrentDesc = "GC.Root";
    c.CollectingLowPrio = true;

    for (UPInt g = 0; g < sizeof(Roots)/sizeof(Roots[0]); ++g)
    {
        const RefCountBaseGC<Mem_Stat>* cur = Roots[g].pRootHead;
        while (cur)
        {
            if (cur->IsRoot())
                c.AddToProcess(cur, "GC.Root");
            cur = cur->pNextRoot;
        }
    }

    asc.CurrentDesc = "";
    c.CollectingLowPrio = false;
}

void VM::Collect(ObjectCollector& c)
{
    ASCollector& asc = *c.pASCollector;
//     for (UPInt i = 0; i < GlobalObjects.GetSize(); ++i)
//     {
//         c.AddToProcess(GlobalObjects[i].GetPtr(), "VM.GlobalObjects");
//     }

    asc.CurrentDesc = "VM.GlobalObjectValue";
    AS3::ForEachChild_GC(&asc, GlobalObjectValue, CollectorFunction 
                         SF_DEBUG_ARG(*reinterpret_cast<const RefCountBaseGC<Mem_Stat>*>((const char*)1 - 1)));
    asc.CurrentDesc = "VM.ExceptionObj";
    AS3::ForEachChild_GC(&asc, ExceptionObj, CollectorFunction 
                         SF_DEBUG_ARG(*reinterpret_cast<const RefCountBaseGC<Mem_Stat>*>((const char*)1 - 1)));

    asc.CurrentDesc = "VM.Registers";
    for (UPInt i = 0; i < RegisterFile.GetSize(); ++i)
    {
        AS3::ForEachChild_GC(&asc, RegisterFile[i], CollectorFunction 
                             SF_DEBUG_ARG(*reinterpret_cast<const RefCountBaseGC<Mem_Stat>*>((const char*)1 - 1)));
    }

    asc.CurrentDesc = "VM.ScopeStack";
    for (UPInt i = 0; i < ScopeStack.GetSize(); ++i)
    {
        AS3::ForEachChild_GC(&asc, ScopeStack[i], CollectorFunction 
                             SF_DEBUG_ARG(*reinterpret_cast<const RefCountBaseGC<Mem_Stat>*>((const char*)1 - 1)));
    }

    asc.CurrentDesc = "";//"sas";
    //? CallFrame?

    Array<SPtr<const Instances::fl::Namespace> > multinames;
    SystemDomain->GetMultinames(multinames);
    for (UPInt i = 0; i < multinames.GetSize(); ++i)
    {
        c.AddToProcess(multinames[i].GetPtr(), "VM.ClassTraitsSet.Key");
    }

    c.AddToProcess(PublicNamespace.GetPtr(), "VM");
    c.AddToProcess(AS3Namespace.GetPtr(), "VM");
    c.AddToProcess(VectorNamespace.GetPtr(), "VM");

    c.AddToProcess(TraitsClassClass.GetPtr(), "VM");
    c.AddToProcess(TraitsObject.GetPtr(), "VM");
    c.AddToProcess(TraitsNamespace.GetPtr(), "VM");
    c.AddToProcess(TraitsFunction.GetPtr(), "VM");
    //c.AddToProcess(TraitsMethodClosure.GetPtr(), "VM");
    c.AddToProcess((RefCountBaseGC<Mem_Stat>*)TraitsBoolean.GetPtr(), "VM");
    c.AddToProcess((RefCountBaseGC<Mem_Stat>*)TraitsNumber.GetPtr(), "VM");
    c.AddToProcess((RefCountBaseGC<Mem_Stat>*)TraitsInt.GetPtr(), "VM");
    c.AddToProcess((RefCountBaseGC<Mem_Stat>*)TraitsUint.GetPtr(), "VM");
    c.AddToProcess((RefCountBaseGC<Mem_Stat>*)TraitsString.GetPtr(), "VM");
    c.AddToProcess((RefCountBaseGC<Mem_Stat>*)TraitsArray.GetPtr(), "VM");
    c.AddToProcess((RefCountBaseGC<Mem_Stat>*)TraitsByteArray.GetPtr(), "VM");
    c.AddToProcess((RefCountBaseGC<Mem_Stat>*)TraitsCatch.GetPtr(), "VM");
    c.AddToProcess((RefCountBaseGC<Mem_Stat>*)TraitsVector.GetPtr(), "VM");
    c.AddToProcess((RefCountBaseGC<Mem_Stat>*)TraitsVector_int.GetPtr(), "VM");
    c.AddToProcess((RefCountBaseGC<Mem_Stat>*)TraitsVector_uint.GetPtr(), "VM");
    c.AddToProcess((RefCountBaseGC<Mem_Stat>*)TraitsVector_Number.GetPtr(), "VM");
    c.AddToProcess((RefCountBaseGC<Mem_Stat>*)TraitsVector_String.GetPtr(), "VM");
    c.AddToProcess((RefCountBaseGC<Mem_Stat>*)TraitsQName.GetPtr(), "VM");

#ifdef GFX_ENABLE_XML
    const XMLSupport& xmls = GetXMLSupport();
    if (xmls.IsEnabled())
    {
        c.AddToProcess((RefCountBaseGC<Mem_Stat>*)xmls.GetClassTraitsXML(), "VM");
        c.AddToProcess((RefCountBaseGC<Mem_Stat>*)xmls.GetClassTraitsXMLList(), "VM");
    }
#endif

    c.AddToProcess(TraitsNull.GetPtr(), "VM");
    c.AddToProcess(TraitsVoid.GetPtr(), "VM");

    c.AddToProcess(NoFunctionTraits.GetPtr(), "VM");

    // CPP GlobalObject ...
    c.AddToProcess((RefCountBaseGC<Mem_Stat>*)TraitaGlobalObject.GetPtr(), "VM.TraitaGlobalObject");
    c.AddToProcess((RefCountBaseGC<Mem_Stat>*)GlobalObject.GetPtr(), "VM.GlobalObjectCPP");
}

void ASVM::Collect(ObjectCollector& c)
{
    VM::Collect(c);
    c.AddToProcess(GraphicsClass.GetPtr(), "ASVM");
    c.AddToProcess(TransformClass.GetPtr(), "ASVM");
    c.AddToProcess(MatrixClass.GetPtr(), "ASVM");
    c.AddToProcess(Matrix3DClass.GetPtr(), "ASVM");
    c.AddToProcess(PerspectiveProjectionClass.GetPtr(), "ASVM");
    c.AddToProcess(ColorTransformClass.GetPtr(), "ASVM");
    c.AddToProcess(EventClass.GetPtr(), "ASVM");
    c.AddToProcess(MouseEventClass.GetPtr(), "ASVM");
    c.AddToProcess(MouseEventExClass.GetPtr(), "ASVM");
    c.AddToProcess(KeyboardEventClass.GetPtr(), "ASVM");
    c.AddToProcess(KeyboardEventExClass.GetPtr(), "ASVM");
#if defined(GFX_ENABLE_ANALOG_GAMEPAD)
    c.AddToProcess(GamePadAnalogEventClass.GetPtr(), "ASVM");
#endif
    c.AddToProcess(FocusEventClass.GetPtr(), "ASVM");
    c.AddToProcess(FocusEventExClass.GetPtr(), "ASVM");
    c.AddToProcess(TextEventClass.GetPtr(), "ASVM");
    c.AddToProcess(TextEventExClass.GetPtr(), "ASVM");
    c.AddToProcess(TimerEventClass.GetPtr(), "ASVM");
    c.AddToProcess(ProgressEventClass.GetPtr(), "ASVM");
#ifdef SF_AS3_VERSION_AIR
    c.AddToProcess(OutputProgressEventClass.GetPtr(), "ASVM");
#endif
#if defined(GFX_MULTITOUCH_SUPPORT_ENABLE)
    c.AddToProcess(TouchEventClass.GetPtr(), "ASVM");
    c.AddToProcess(GestureEventClass.GetPtr(), "ASVM");
    c.AddToProcess(TransformGestureEventClass.GetPtr(), "ASVM");
    c.AddToProcess(PressAndTapGestureEventClass.GetPtr(), "ASVM");
#endif
    c.AddToProcess(StageOrientationEventClass.GetPtr(), "ASVM");
    c.AddToProcess(AppLifecycleEventClass.GetPtr(), "ASVM");
    c.AddToProcess(PointClass.GetPtr(), "ASVM");
    c.AddToProcess(RectangleClass.GetPtr(), "ASVM");
    c.AddToProcess(TextFormatClass.GetPtr(), "ASVM");
    c.AddToProcess(EventDispatcherClass.GetPtr(), "ASVM");
    c.AddToProcess(Vector3DClass.GetPtr(), "ASVM");

    GetGC().CollectRoots(c);
}

void EventChains::Collect(ObjectCollector& c) const 
{
    ASCollector& asc = *c.pASCollector;
    for (ChainHash::ConstIterator it = Chains.Begin(); it != Chains.End(); ++it)
    {
        const AutoPtr<Chain>& ch = it->Second;
        int eventId = it->First;
        char buf[256];
        SFsprintf(buf, sizeof(buf), "EventListener(0x%x)", eventId);
        asc.CurrentDesc = buf;
        for (UPInt i = 0, n = ch->GetSize(); i < n; ++i)
        {
            const Ptr<DisplayObject>& dobj = (*ch)[i];
            if (dobj)
            {
                c.AddToProcess(ToAvmDisplayObj(dobj.GetPtr()), buf);
            }
        }
        asc.CurrentDesc = "";
    }
}

MovieRoot::CollectorProcessHelper::CollectorProcessHelper(MovieRoot* mov) : MovRoot(mov)
{
    // Suspend the GC to avoid OnExceedLimit to interfere with the obj collection.
    // MAKE SURE GC IS RE-ENABLED AT THE END, NO 'return's are allowed without it!
    MovRoot->SuspendGC(true);

    ASCollector asc(MovRoot->Collector);
    MovRoot->Collector.pASCollector = &asc;
    //Collector.Clear();

    // Add all elgible objs held by MovieRoot/MovieImpl
    // MouseState
    for (unsigned i = 0; i < GFX_MAX_MICE_SUPPORTED; ++i)
    {
        if (MovRoot->mMouseState[i].LastMouseOverObj)
        {
            char s[256];
            SFsprintf(s, sizeof(s), "MouseState[%d].LastOverObj", i);
            MovRoot->Collector.AddToProcess(ToAvmDisplayObj(MovRoot->mMouseState[i].LastMouseOverObj.GetPtr()), s);
        }
        for (unsigned j = 0; j < MovRoot->mMouseState[i].RolloverStack.GetSize(); ++j)
        {
            if (MovRoot->mMouseState[i].RolloverStack[j])
            {
                char s[256];
                SFsprintf(s, sizeof(s), "MouseState[%d].RollOverStack[%d]", i, j);
                MovRoot->Collector.AddToProcess(ToAvmDisplayObj(MovRoot->mMouseState[i].RolloverStack[j].GetPtr()), s);
            }
        }
    }

    MovRoot->Collector.AddToProcess(ToAvmDisplayObj(MovRoot->GetStage()), "MovieRoot");

    // Timers
    MovieImpl* movie = MovRoot->GetMovieImpl();
    for (UPInt i = 0; i < movie->IntervalTimers.GetSize(); ++i)
    {
        IntervalTimer* te = static_cast<IntervalTimer*>(movie->IntervalTimers[i].GetPtr());
        te->Collect(MovRoot->Collector);
    }

    // Event chains (RENDER, EnterFrame, Activate/Deactivate, etc)
    MovRoot->mEventChains.Collect(MovRoot->Collector);

    if (MovRoot->pAVM)
    {
        MovRoot->pAVM->Collect(MovRoot->Collector);
    }

    // Collect fonts
    GFx::MovieDefRootNode *pdefNode = movie->RootMovieDefNodes.GetFirst();
    while(!movie->RootMovieDefNodes.IsNull(pdefNode))
    {        
        const GFx::FontManager::FontSet& fonts = pdefNode->pFontManager->GetCreatedFonts();
        for (GFx::FontManager::FontSet::ConstIterator it = fonts.Begin();
            it != fonts.End(); ++it)
        {
            const GFx::FontHandle* fh = it->pNode;
            MovRoot->Collector.AddToProcess(fh, pdefNode->pDefImpl);
            if (fh->pSourceMovieDef)
                MovRoot->Collector.AddToProcess(fh->pSourceMovieDef, fh);
        }
        pdefNode = pdefNode->pNext;
    }

    // Collect imports
    MovRoot->Collector.AddToProcess(movie->GetMovieDefImpl(), "Main Movie");

    MovRoot->Collector.Process(asc);
}

MovieRoot::CollectorProcessHelper::~CollectorProcessHelper()
{
    MovRoot->Collector.Clear();

    // re-enable GC
    MovRoot->SuspendGC(false);
}


void MovieRoot::CollectObjects(UInt32 flags, 
                               Log* log,  
                               const char* swfName)
{
    CollectorProcessHelper collectorProcess(this);

    Collector.Dump(flags, (log) ? log : GetLog(), swfName);
}

void MovieRoot::CollectObjects(Array<Ptr<AmpMovieObjectDesc> >* rootObjects, MemoryHeap* heap)
{
    CollectorProcessHelper collectorProcess(this);
    Collector.GetTree(rootObjects, heap);
}

void MovieRoot::PrintObjectsReport(UInt32 flags, 
                                   Log* log,  
                                   const char* swfName)
{
    if (!GetMovieImpl()->IsShutdowning())
        CollectObjects(flags, log, swfName);
}

void MovieRoot::GetObjectsTree(Array<Ptr<AmpMovieObjectDesc> >* rootObjects, MemoryHeap* heap)
{
    if (!GetMovieImpl()->IsShutdowning())
    {
        CollectObjects(rootObjects, heap);
    }
}

#else

void MovieRoot::PrintObjectsReport(UInt32 flags, 
                                   Log* log,  
                                   const char* swfName)
{
    SF_UNUSED3(flags, log, swfName);
}

void MovieRoot::GetObjectsTree(Array<Ptr<AmpMovieObjectDesc> >* rootObjects, MemoryHeap* heap)
{
    SF_UNUSED2(rootObjects, heap);
}


#endif //#ifdef SF_OBJ_COLLECTOR_ENABLED

}}}
