//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Desktop_Clipboard.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Desktop_Clipboard.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../AS3_Obj_Array.h"
#include "AS3_Obj_Desktop_ClipboardFormats.h"
#include "../../AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
    struct FormatPair
    {
        GFx::Clipboard::Formats Format;
        const char* AS3Name;
    };
    FormatPair FormatMap[] =
    {
        {GFx::Clipboard::Format_Text, "air:text"},
        {GFx::Clipboard::Format_RichText, "air:rtf"},
        {GFx::Clipboard::Format_Bitmap, "air:bitmap"},
        {GFx::Clipboard::Format_FileList, "air:file list"},
        {GFx::Clipboard::Format_FilePromise_List, "air:file promise list"},
        {GFx::Clipboard::Format_Html, "air:html"},
        {GFx::Clipboard::Format_URL, "air:url"},
        {GFx::Clipboard::Format_None, 0}
    };

    //static const char* GetAS3Format(GFx::Clipboard::Formats format)
    //{
    //    FormatPair* pf = FormatMap;
    //    while (pf->AS3Name)
    //    {
    //        if (pf->Format == format)
    //            return pf->AS3Name;
    //    }
    //    return 0;
    //}

    static GFx::Clipboard::Formats GetLoaderFormat(const char* name)
    {
        FormatPair* pf = FormatMap;
        while (pf->Format != GFx::Clipboard::Format_None)
        {
            if (!SFstrcmp(name, pf->AS3Name))
                return pf->Format;
            ++pf;
        }
        return GFx::Clipboard::Format_None;
    }
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_desktop::Clipboard, Instances::fl_desktop::Clipboard::mid_formatsGet, SPtr<Instances::fl::Array> > TFunc_Instances_Clipboard_formatsGet;
typedef ThunkFunc0<Instances::fl_desktop::Clipboard, Instances::fl_desktop::Clipboard::mid_clear, const Value> TFunc_Instances_Clipboard_clear;
typedef ThunkFunc1<Instances::fl_desktop::Clipboard, Instances::fl_desktop::Clipboard::mid_clearData, const Value, const ASString&> TFunc_Instances_Clipboard_clearData;
typedef ThunkFunc2<Instances::fl_desktop::Clipboard, Instances::fl_desktop::Clipboard::mid_getData, Value, const ASString&, const ASString&> TFunc_Instances_Clipboard_getData;
typedef ThunkFunc1<Instances::fl_desktop::Clipboard, Instances::fl_desktop::Clipboard::mid_hasFormat, bool, const ASString&> TFunc_Instances_Clipboard_hasFormat;
typedef ThunkFunc3<Instances::fl_desktop::Clipboard, Instances::fl_desktop::Clipboard::mid_setData, bool, const ASString&, const Value&, bool> TFunc_Instances_Clipboard_setData;
typedef ThunkFunc3<Instances::fl_desktop::Clipboard, Instances::fl_desktop::Clipboard::mid_setDataHandler, bool, const ASString&, const Value&, bool> TFunc_Instances_Clipboard_setDataHandler;

template <> const TFunc_Instances_Clipboard_formatsGet::TMethod TFunc_Instances_Clipboard_formatsGet::Method = &Instances::fl_desktop::Clipboard::formatsGet;
template <> const TFunc_Instances_Clipboard_clear::TMethod TFunc_Instances_Clipboard_clear::Method = &Instances::fl_desktop::Clipboard::clear;
template <> const TFunc_Instances_Clipboard_clearData::TMethod TFunc_Instances_Clipboard_clearData::Method = &Instances::fl_desktop::Clipboard::clearData;
template <> const TFunc_Instances_Clipboard_getData::TMethod TFunc_Instances_Clipboard_getData::Method = &Instances::fl_desktop::Clipboard::getData;
template <> const TFunc_Instances_Clipboard_hasFormat::TMethod TFunc_Instances_Clipboard_hasFormat::Method = &Instances::fl_desktop::Clipboard::hasFormat;
template <> const TFunc_Instances_Clipboard_setData::TMethod TFunc_Instances_Clipboard_setData::Method = &Instances::fl_desktop::Clipboard::setData;
template <> const TFunc_Instances_Clipboard_setDataHandler::TMethod TFunc_Instances_Clipboard_setDataHandler::Method = &Instances::fl_desktop::Clipboard::setDataHandler;

namespace Instances { namespace fl_desktop
{
    Clipboard::Clipboard(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
//##protect##"instance::Clipboard::Clipboard()$data"
//##protect##"instance::Clipboard::Clipboard()$data"
    {
//##protect##"instance::Clipboard::Clipboard()$code"
//##protect##"instance::Clipboard::Clipboard()$code"
    }

    void Clipboard::formatsGet(SPtr<Instances::fl::Array>& result)
    {
//##protect##"instance::Clipboard::formatsGet()"
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        result = GetVM().MakeArray();
        UInt32 formats = asvm.GetMovieImpl()->GetLoaderImpl()->GetClipboard()->GetAvailableFormats();
        FormatPair* pf = FormatMap;
        while (pf->Format != GFx::Clipboard::Format_None)
        {
            if (formats & pf->Format)
                result->PushBack(Value(GetVM().GetStringManager().CreateConstString(pf->AS3Name)));
            ++pf;
        }

//##protect##"instance::Clipboard::formatsGet()"
    }
    void Clipboard::clear(const Value& result)
    {
//##protect##"instance::Clipboard::clear()"
        SF_UNUSED(result);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        asvm.GetMovieImpl()->GetLoaderImpl()->GetClipboard()->Clear();
//##protect##"instance::Clipboard::clear()"
    }
    void Clipboard::clearData(const Value& result, const ASString& format)
    {
//##protect##"instance::Clipboard::clearData()"
        SF_UNUSED(result);
        ASVM& asvm = static_cast<ASVM&>(GetVM());

        asvm.GetMovieImpl()->GetLoaderImpl()->GetClipboard()->ClearData(GetLoaderFormat(format.ToCStr()));      

//##protect##"instance::Clipboard::clearData()"
    }
    void Clipboard::getData(Value& result, const ASString& format, const ASString& transferMode)
    {
//##protect##"instance::Clipboard::getData()"
        SF_UNUSED(transferMode);
        SF_UNUSED(result);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        GFx::Clipboard::Formats lf = GetLoaderFormat(format.ToCStr());
        switch (lf)
        { //Flash behavior is inconsistent for rtf and html, for now will treat them as a text
        case GFx::Clipboard::Format_Text:
        case GFx::Clipboard::Format_RichText:
        case GFx::Clipboard::Format_Html:
            {
                ASString text = GetVM().GetStringManager().CreateString(String(asvm.GetMovieImpl()->GetLoaderImpl()->GetClipboard()->GetText().ToWStr()));
                result = text; 
            }
        default:
            break;
        }

//##protect##"instance::Clipboard::getData()"
    }
    void Clipboard::hasFormat(bool& result, const ASString& format)
    {
//##protect##"instance::Clipboard::hasFormat()"
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        result = asvm.GetMovieImpl()->GetLoaderImpl()->GetClipboard()->HasFormat(GetLoaderFormat(format.ToCStr()));
//##protect##"instance::Clipboard::hasFormat()"
    }
    void Clipboard::setData(bool& result, const ASString& format, const Value& data, bool serializable)
    {
//##protect##"instance::Clipboard::setData()"
        SF_UNUSED(serializable);
        result = false;
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        GFx::Clipboard::Formats lf = GetLoaderFormat(format.ToCStr());
        switch (lf)
        { //Flash behavior is inconsistent for rtf and html, for now will treat them as a text
        case GFx::Clipboard::Format_Text:
        case GFx::Clipboard::Format_RichText:
        case GFx::Clipboard::Format_Html:
            {
                if (data.IsString())
                    asvm.GetMovieImpl()->GetLoaderImpl()->GetClipboard()->SetText(String(data.AsString().ToCStr()));
                result = true;
            }
        default:
            break;
        }
//##protect##"instance::Clipboard::setData()"
    }
    void Clipboard::setDataHandler(bool& result, const ASString& format, const Value& handler, bool serializable)
    {
//##protect##"instance::Clipboard::setDataHandler()"
        SF_UNUSED4(result, format, handler, serializable);
        WARN_NOT_IMPLEMENTED("instance::Clipboard::setDataHandler()");
//##protect##"instance::Clipboard::setDataHandler()"
    }

    SPtr<Instances::fl::Array> Clipboard::formatsGet()
    {
        SPtr<Instances::fl::Array> result;
        formatsGet(result);
        return result;
    }
//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_desktop
{
    // const UInt16 Clipboard::tito[Clipboard::ThunkInfoNum] = {
    //    0, 1, 2, 4, 7, 9, 13, 
    // };
    const TypeInfo* Clipboard::tit[17] = {
        &AS3::fl::ArrayTI, 
        NULL, 
        NULL, &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, &AS3::fl::StringTI, &AS3::fl::ObjectTI, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, &AS3::fl::StringTI, &AS3::fl::FunctionTI, &AS3::fl::BooleanTI, 
    };
    const Abc::ConstValue Clipboard::dva[3] = {
        {Abc::CONSTANT_Utf8, 2}, 
        {Abc::CONSTANT_True, 0}, 
        {Abc::CONSTANT_True, 0}, 
    };
    const ThunkInfo Clipboard::ti[Clipboard::ThunkInfoNum] = {
        {TFunc_Instances_Clipboard_formatsGet::Func, &Clipboard::tit[0], "formats", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Clipboard_clear::Func, &Clipboard::tit[1], "clear", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Clipboard_clearData::Func, &Clipboard::tit[2], "clearData", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Clipboard_getData::Func, &Clipboard::tit[4], "getData", NULL, Abc::NS_Public, CT_Method, 1, 2, 0, 1, &Clipboard::dva[0]},
        {TFunc_Instances_Clipboard_hasFormat::Func, &Clipboard::tit[7], "hasFormat", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Clipboard_setData::Func, &Clipboard::tit[9], "setData", NULL, Abc::NS_Public, CT_Method, 2, 3, 0, 1, &Clipboard::dva[1]},
        {TFunc_Instances_Clipboard_setDataHandler::Func, &Clipboard::tit[13], "setDataHandler", NULL, Abc::NS_Public, CT_Method, 2, 3, 0, 1, &Clipboard::dva[2]},
    };

    Clipboard::Clipboard(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::Clipboard::Clipboard()"
//##protect##"InstanceTraits::Clipboard::Clipboard()"

    }

    void Clipboard::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<Clipboard&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_desktop
{
    Clipboard::Clipboard(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::Clipboard::Clipboard()"
//##protect##"class_::Clipboard::Clipboard()"
    }
    void Clipboard::generalClipboardGet(SPtr<Instances::fl_desktop::Clipboard>& result)
    {
//##protect##"class_::Clipboard::generalClipboardGet()"
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        SPtr<Instances::fl_desktop::Clipboard>&  cl = reinterpret_cast<SPtr<Instances::fl_desktop::Clipboard>& >(asvm.GeneralClipboard);
        if (!cl)
        {
            if (!GetVM().ConstructBuiltinObject(cl, "flash.desktop.Clipboard"))
                return;
        }
        result = cl;
//##protect##"class_::Clipboard::generalClipboardGet()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc0<Classes::fl_desktop::Clipboard, Classes::fl_desktop::Clipboard::mid_generalClipboardGet, SPtr<Instances::fl_desktop::Clipboard> > TFunc_Classes_Clipboard_generalClipboardGet;

template <> const TFunc_Classes_Clipboard_generalClipboardGet::TMethod TFunc_Classes_Clipboard_generalClipboardGet::Method = &Classes::fl_desktop::Clipboard::generalClipboardGet;

namespace ClassTraits { namespace fl_desktop
{
    // const UInt16 Clipboard::tito[Clipboard::ThunkInfoNum] = {
    //    0, 
    // };
    const TypeInfo* Clipboard::tit[1] = {
        &AS3::fl_desktop::ClipboardTI, 
    };
    const ThunkInfo Clipboard::ti[Clipboard::ThunkInfoNum] = {
        {TFunc_Classes_Clipboard_generalClipboardGet::Func, &Clipboard::tit[0], "generalClipboard", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
    };

    Clipboard::Clipboard(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::Clipboard::Clipboard()"
//##protect##"ClassTraits::Clipboard::Clipboard()"

    }

    Pickable<Traits> Clipboard::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Clipboard(vm, AS3::fl_desktop::ClipboardCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_desktop::ClipboardCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_desktop
{
    const TypeInfo ClipboardTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_desktop::Clipboard::InstanceType),
        ClassTraits::fl_desktop::Clipboard::ThunkInfoNum,
        0,
        InstanceTraits::fl_desktop::Clipboard::ThunkInfoNum,
        0,
        "Clipboard", "flash.desktop", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo ClipboardCI = {
        &ClipboardTI,
        ClassTraits::fl_desktop::Clipboard::MakeClassTraits,
        ClassTraits::fl_desktop::Clipboard::ti,
        NULL,
        InstanceTraits::fl_desktop::Clipboard::ti,
        NULL,
    };
}; // namespace fl_desktop


}}} // namespace Scaleform { namespace GFx { namespace AS3

