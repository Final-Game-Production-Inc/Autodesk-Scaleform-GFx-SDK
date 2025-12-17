//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_Bitmap.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_Bitmap.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "AS3_Obj_Display_BitmapData.h"
#include "GFx/AS3/AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_display::Bitmap, Instances::fl_display::Bitmap::mid_bitmapDataGet, SPtr<Instances::fl_display::BitmapData> > TFunc_Instances_Bitmap_bitmapDataGet;
typedef ThunkFunc1<Instances::fl_display::Bitmap, Instances::fl_display::Bitmap::mid_bitmapDataSet, const Value, Instances::fl_display::BitmapData*> TFunc_Instances_Bitmap_bitmapDataSet;
typedef ThunkFunc0<Instances::fl_display::Bitmap, Instances::fl_display::Bitmap::mid_pixelSnappingGet, ASString> TFunc_Instances_Bitmap_pixelSnappingGet;
typedef ThunkFunc1<Instances::fl_display::Bitmap, Instances::fl_display::Bitmap::mid_pixelSnappingSet, const Value, const ASString&> TFunc_Instances_Bitmap_pixelSnappingSet;
typedef ThunkFunc0<Instances::fl_display::Bitmap, Instances::fl_display::Bitmap::mid_smoothingGet, bool> TFunc_Instances_Bitmap_smoothingGet;
typedef ThunkFunc1<Instances::fl_display::Bitmap, Instances::fl_display::Bitmap::mid_smoothingSet, const Value, bool> TFunc_Instances_Bitmap_smoothingSet;

template <> const TFunc_Instances_Bitmap_bitmapDataGet::TMethod TFunc_Instances_Bitmap_bitmapDataGet::Method = &Instances::fl_display::Bitmap::bitmapDataGet;
template <> const TFunc_Instances_Bitmap_bitmapDataSet::TMethod TFunc_Instances_Bitmap_bitmapDataSet::Method = &Instances::fl_display::Bitmap::bitmapDataSet;
template <> const TFunc_Instances_Bitmap_pixelSnappingGet::TMethod TFunc_Instances_Bitmap_pixelSnappingGet::Method = &Instances::fl_display::Bitmap::pixelSnappingGet;
template <> const TFunc_Instances_Bitmap_pixelSnappingSet::TMethod TFunc_Instances_Bitmap_pixelSnappingSet::Method = &Instances::fl_display::Bitmap::pixelSnappingSet;
template <> const TFunc_Instances_Bitmap_smoothingGet::TMethod TFunc_Instances_Bitmap_smoothingGet::Method = &Instances::fl_display::Bitmap::smoothingGet;
template <> const TFunc_Instances_Bitmap_smoothingSet::TMethod TFunc_Instances_Bitmap_smoothingSet::Method = &Instances::fl_display::Bitmap::smoothingSet;

namespace Instances { namespace fl_display
{
    Bitmap::Bitmap(InstanceTraits::Traits& t)
    : Instances::fl_display::DisplayObject(t)
//##protect##"instance::Bitmap::Bitmap()$data"
    , PixelSnapping(Never)
    , Smoothing(GFX_AS3_SMOOTH_BMP_BY_DEFAULT)
//##protect##"instance::Bitmap::Bitmap()$data"
    {
//##protect##"instance::Bitmap::Bitmap()$code"
//##protect##"instance::Bitmap::Bitmap()$code"
    }

    void Bitmap::bitmapDataGet(SPtr<Instances::fl_display::BitmapData>& result)
    {
//##protect##"instance::Bitmap::bitmapDataGet()"
        SF_UNUSED1(result);
        result = pBitmapData;
//##protect##"instance::Bitmap::bitmapDataGet()"
    }
    void Bitmap::bitmapDataSet(const Value& result, Instances::fl_display::BitmapData* value)
    {
//##protect##"instance::Bitmap::bitmapDataSet()"
        SF_UNUSED2(result, value);
        pBitmapData = value;
        AvmBitmap* abmp = GetAvmBitmap();
        if (abmp)
        {
            if (pBitmapData)
                abmp->SetResourceMovieDef(pBitmapData->pDefImpl);
            else
                abmp->SetResourceMovieDef(NULL);
        }
        NotifyBitmapDataChanged();
//##protect##"instance::Bitmap::bitmapDataSet()"
    }
    void Bitmap::pixelSnappingGet(ASString& result)
    {
//##protect##"instance::Bitmap::pixelSnappingGet()"
        SF_UNUSED1(result);
        switch (PixelSnapping)
        {
        case Never:     result = "never"; break;
        case Always:    result = "always"; break;
        case Auto:      result = "auto"; break;
        default: SF_ASSERT(0);
        }
//##protect##"instance::Bitmap::pixelSnappingGet()"
    }
    void Bitmap::pixelSnappingSet(const Value& result, const ASString& value)
    {
//##protect##"instance::Bitmap::pixelSnappingSet()"
        SF_UNUSED2(result, value);
        PixelSnapping = String2PixelSnapping(value.ToCStr());
        NotifyBitmapDataChanged();
//##protect##"instance::Bitmap::pixelSnappingSet()"
    }
    void Bitmap::smoothingGet(bool& result)
    {
//##protect##"instance::Bitmap::smoothingGet()"
        SF_UNUSED1(result);
        result = Smoothing;
//##protect##"instance::Bitmap::smoothingGet()"
    }
    void Bitmap::smoothingSet(const Value& result, bool value)
    {
//##protect##"instance::Bitmap::smoothingSet()"
        SF_UNUSED2(result, value);
        Smoothing = value;
        NotifyBitmapDataChanged();
//##protect##"instance::Bitmap::smoothingSet()"
    }

    SPtr<Instances::fl_display::BitmapData> Bitmap::bitmapDataGet()
    {
        SPtr<Instances::fl_display::BitmapData> result;
        bitmapDataGet(result);
        return result;
    }
//##protect##"instance$methods"
    void Bitmap::NotifyBitmapDataChanged()
    {
        if (pDispObj)
        {
            GetAvmBitmap()->RecreateRenderNode();
        }
    }

    GFx::ImageResource* Bitmap::GetImageResource() const
    {
        if (pBitmapData)
            return pBitmapData->GetImageResource();
        return NULL;
    }

    void Bitmap::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        DisplayObject::ForEachChild_GC(prcc, op);
        AS3::ForEachChild_GC<BitmapData, Mem_Stat>(prcc, pBitmapData, op SF_DEBUG_ARG(*this));
    }

    void Bitmap::AS3Constructor(unsigned argc, const Value* argv)
    {
        if (argc >= 1)
        {
            if (GetVM().IsOfType(argv[0], "flash.display.BitmapData", GetVM().GetCurrentAppDomain()))
            {
                pBitmapData = static_cast<BitmapData*>(argv[0].GetObject());
                AvmBitmap* abmp = GetAvmBitmap();
                if (abmp)
                {
                    if (pBitmapData)
                        abmp->SetResourceMovieDef(pBitmapData->pDefImpl);
                    else
                        abmp->SetResourceMovieDef(NULL);
                }
            }
            if (argc >= 2)
            {
                const Value& v = argv[1];
                ASString str = GetVM().GetStringManager().CreateEmptyString();

                if (!v.Convert2String(str))
                    return;

                PixelSnapping = String2PixelSnapping(str.ToCStr());

                if (argc >= 3)
                {
                    Smoothing = argv[2].Convert2Boolean();
                }
            }
            NotifyBitmapDataChanged();
        }
    }

    void Bitmap::InitInstance(bool extCall)
    {
        if (!extCall)
        {
            // create stage display object if the movieclip was created by
            // "new".
            Bitmap::CreateStageObject();
            SF_ASSERT(pDispObj);
        }
    }
    GFx::DisplayObject* Bitmap::CreateStageObject()
    {
        // CreateStageObject is called when the object is instantiated by
        // ActionScript 'new' operator. There are several different scenarios possible:
        //  1) There are no library symbols linked to the bitmap (new Bitmap):
        //      no BitmapData is created.
        //  2) The bitmap was put into a library and linked with a class:
        //      in this case the class is linked to a BitmapData, not to a bitmap itself; therefore
        //      Bitmap will be created as 'new Bitmap(new MyBitmapData())' (see 1st item)
        //  3) The bitmap was embedded via [Embed] directive with assigned class. In this case
        //      the class is linked to the actual Bitmap, so we need to create a proper ActionScript
        //      class and a proper BitmapData.
        if (!pDispObj)
        {
            ASVM& asvm = static_cast<ASVM&>(GetVM());
            MovieRoot* proot = asvm.GetMovieRoot();

            MovieDefImpl* pdefImpl = asvm.GetResourceMovieDef(this);

            if (pdefImpl)
            {
                // it is possible that there is no corresponding stage object
                // for the class; use empty movie clip then.
                CharacterCreateInfo ccinfo;
                ccinfo.pCharDef = NULL;

                if (!FindLibarySymbol(&ccinfo, pdefImpl))
                {
                    // not found... Let check if there are symbols linked to the same class in other
                    // moviedefs. Seems like Flash does it.
                    if (!proot->FindLibrarySymbolInAllABCs(this, &ccinfo))
                    {
                        ccinfo.pCharDef = NULL;
                        ccinfo.pResource = NULL;
                    }
                }

                pDispObj = *static_cast<GFx::DisplayObject*>(proot->GetASSupport()->CreateCharacterInstance(
                    proot->GetMovieImpl(), ccinfo, NULL, 
                    ResourceId(), CharacterDef::Bitmap));
                AvmDisplayObj* pAvmObj = ToAvmDisplayObj(pDispObj);
                pAvmObj->AssignAS3Obj(this);
                pAvmObj->SetAppDomain(GetInstanceTraits().GetAppDomain());

                if (ccinfo.pResource && ccinfo.pResource->GetResourceType() == Resource::RT_Image)
                {
                    // create a BitmapData right here if Resource was found.

                    Value params[] = { Value(0.f), Value(0.f) };
                    SPtr<Instances::fl_display::BitmapData> bmpData;
                    if (asvm.ConstructBuiltinObject(bmpData, "flash.display.BitmapData", 2, params))
                    {
                      if (bmpData->CreateLibraryObject(static_cast<ImageResource*>(ccinfo.pResource), ccinfo.pBindDefImpl))
                         SetBitmapData(bmpData);
                    }
                }
            }
        }
        return pDispObj;
    }

    Bitmap::PixelSnappingType Bitmap::String2PixelSnapping(const char* str)
    {
        if (SFstrcmp(str, "never") == 0)
            return Never;
        else if (SFstrcmp(str, "always") == 0)
            return Always;
        else if (SFstrcmp(str, "auto") == 0)
            return Auto;
        else
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eInvalidEnumError, GetVM() SF_DEBUG_ARG("pixelSnapping")));
        }
        return Never;
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    // const UInt16 Bitmap::tito[Bitmap::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 7, 
    // };
    const TypeInfo* Bitmap::tit[9] = {
        &AS3::fl_display::BitmapDataTI, 
        NULL, &AS3::fl_display::BitmapDataTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
    };
    const ThunkInfo Bitmap::ti[Bitmap::ThunkInfoNum] = {
        {TFunc_Instances_Bitmap_bitmapDataGet::Func, &Bitmap::tit[0], "bitmapData", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Bitmap_bitmapDataSet::Func, &Bitmap::tit[1], "bitmapData", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Bitmap_pixelSnappingGet::Func, &Bitmap::tit[3], "pixelSnapping", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Bitmap_pixelSnappingSet::Func, &Bitmap::tit[4], "pixelSnapping", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Bitmap_smoothingGet::Func, &Bitmap::tit[6], "smoothing", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Bitmap_smoothingSet::Func, &Bitmap::tit[7], "smoothing", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
    };

    Bitmap::Bitmap(VM& vm, const ClassInfo& ci)
    : fl_display::DisplayObject(vm, ci)
    {
//##protect##"InstanceTraits::Bitmap::Bitmap()"
        SetTraitsType(Traits_Bitmap);
//##protect##"InstanceTraits::Bitmap::Bitmap()"

    }

    void Bitmap::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<Bitmap&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_display
{

    Bitmap::Bitmap(VM& vm, const ClassInfo& ci)
    : fl_display::DisplayObject(vm, ci)
    {
//##protect##"ClassTraits::Bitmap::Bitmap()"
        SetTraitsType(Traits_Bitmap);
//##protect##"ClassTraits::Bitmap::Bitmap()"

    }

    Pickable<Traits> Bitmap::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Bitmap(vm, AS3::fl_display::BitmapCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::BitmapCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_display
{
    const TypeInfo BitmapTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_display::Bitmap::InstanceType),
        0,
        0,
        InstanceTraits::fl_display::Bitmap::ThunkInfoNum,
        0,
        "Bitmap", "flash.display", &fl_display::DisplayObjectTI,
        TypeInfo::None
    };

    const ClassInfo BitmapCI = {
        &BitmapTI,
        ClassTraits::fl_display::Bitmap::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_display::Bitmap::ti,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

