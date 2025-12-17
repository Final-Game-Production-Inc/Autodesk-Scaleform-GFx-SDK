//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Text_Font.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Text_Font.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "../AS3_Obj_Array.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_text::Font, Instances::fl_text::Font::mid_fontNameGet, Value> TFunc_Instances_Font_fontNameGet;
typedef ThunkFunc0<Instances::fl_text::Font, Instances::fl_text::Font::mid_fontStyleGet, Value> TFunc_Instances_Font_fontStyleGet;
typedef ThunkFunc0<Instances::fl_text::Font, Instances::fl_text::Font::mid_fontTypeGet, Value> TFunc_Instances_Font_fontTypeGet;
typedef ThunkFunc1<Instances::fl_text::Font, Instances::fl_text::Font::mid_hasGlyphs, bool, const ASString&> TFunc_Instances_Font_hasGlyphs;

template <> const TFunc_Instances_Font_fontNameGet::TMethod TFunc_Instances_Font_fontNameGet::Method = &Instances::fl_text::Font::fontNameGet;
template <> const TFunc_Instances_Font_fontStyleGet::TMethod TFunc_Instances_Font_fontStyleGet::Method = &Instances::fl_text::Font::fontStyleGet;
template <> const TFunc_Instances_Font_fontTypeGet::TMethod TFunc_Instances_Font_fontTypeGet::Method = &Instances::fl_text::Font::fontTypeGet;
template <> const TFunc_Instances_Font_hasGlyphs::TMethod TFunc_Instances_Font_hasGlyphs::Method = &Instances::fl_text::Font::hasGlyphs;

namespace Instances { namespace fl_text
{
    Font::Font(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
//##protect##"instance::Font::Font()$data"
    , fontName(t.GetStringManager().CreateEmptyString())
    , fontStyle(t.GetStringManager().CreateEmptyString())
    , fontType(t.GetStringManager().CreateEmptyString())
//##protect##"instance::Font::Font()$data"
    {
//##protect##"instance::Font::Font()$code"
//##protect##"instance::Font::Font()$code"
    }

    void Font::fontNameGet(Value& result)
    {
//##protect##"instance::Font::fontNameGet()"
        SF_UNUSED1(result);
        if (pFont)
        {
            result = GetVM().GetStringManager().CreateString(pFont->GetName());
        }
        else if (fontName.GetLength() > 0)
            result = fontName;
        else
            result.SetNull();
//##protect##"instance::Font::fontNameGet()"
    }
    void Font::fontStyleGet(Value& result)
    {
//##protect##"instance::Font::fontStyleGet()"
        SF_UNUSED1(result);
        if (pFont)
        {
            unsigned st = pFont->GetFontStyleFlags();
            if (st & GFx::Font::FF_BoldItalic)
                result = GetVM().GetStringManager().CreateConstString("boldItalic");
            else if (st & GFx::Font::FF_Bold)
                result = GetVM().GetStringManager().CreateConstString("bold");
            else if (st & GFx::Font::FF_Italic)
                result = GetVM().GetStringManager().CreateConstString("italic");
            else
                result = GetVM().GetStringManager().CreateConstString("regular");
        }
        else if (fontStyle.GetLength() > 0)
            result = fontStyle;
        else
            result.SetNull();
//##protect##"instance::Font::fontStyleGet()"
    }
    void Font::fontTypeGet(Value& result)
    {
//##protect##"instance::Font::fontTypeGet()"
        SF_UNUSED1(result);
        if (pFont)
        {
            if (pFont->IsDeviceFont())
                result = GetVM().GetStringManager().CreateConstString("device");
            else
                result = GetVM().GetStringManager().CreateConstString("embedded");
        }
        else if (fontType.GetLength() > 0)
            result = fontType;
        else
            result.SetNull();
//##protect##"instance::Font::fontTypeGet()"
    }
    void Font::hasGlyphs(bool& result, const ASString& str)
    {
//##protect##"instance::Font::hasGlyphs()"
        SF_UNUSED2(result, str);
        if (pFont)
        {
            const char* pbuf = str.ToCStr();
            const char* pebuf = pbuf + str.GetSize();
            UInt32 c;
            result = true;
            while (pbuf < pebuf)
            {
                c = UTF8Util::DecodeNextChar(&pbuf);
                if (pFont->GetGlyphIndex((UInt16)c) < 0)
                {
                    result = false;
                    break;
                }
            }
        }
        else
            result = false;
//##protect##"instance::Font::hasGlyphs()"
    }

//##protect##"instance$methods"
    void Font::InitInstance(bool extCall)
    {
        if (!extCall)
        {
            // create stage display object if the movieclip was created by
            // "new".
            Font::CreateLibraryObject();
        }
    }

    bool Font::CreateLibraryObject()
    {
        if (!pFont)
        {
            ASVM& asvm = static_cast<ASVM&>(GetVM());
            MovieDefImpl* pdefImpl = asvm.GetResourceMovieDef(this);

            if (pdefImpl && GetTraits().IsUserDefined())
            {
                // here we should get fully qualified name of class, with package, 
                // for example: example_fla.Symbol1. 
                ASString className = GetInstanceTraits().GetQualifiedName(Traits::qnfWithDot);

                ResourceBindData    resBindData;
                if (!asvm.GetMovieImpl()->FindExportedResource(pdefImpl, &resBindData, className.ToCStr()))
                {
                    MovieRoot* proot = asvm.GetMovieRoot();
                    if (proot->GetLogState())
                        proot->GetLogState()->LogScriptWarning("Attaching a font with class '%s' failed",
                        className.ToCStr());
                    return false;
                }

                if (resBindData.pResource)
                {
                    if (resBindData.pResource->GetResourceType() & Resource::RT_Font)
                    {
                        pFont = static_cast<FontResource*>(resBindData.pResource.GetPtr())->GetFont();
                    }
                }
            }
        }
        return false;
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_text
{
    // const UInt16 Font::tito[Font::ThunkInfoNum] = {
    //    0, 1, 2, 3, 
    // };
    const TypeInfo* Font::tit[5] = {
        NULL, 
        NULL, 
        NULL, 
        &AS3::fl::BooleanTI, &AS3::fl::StringTI, 
    };
    const ThunkInfo Font::ti[Font::ThunkInfoNum] = {
        {TFunc_Instances_Font_fontNameGet::Func, &Font::tit[0], "fontName", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Font_fontStyleGet::Func, &Font::tit[1], "fontStyle", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Font_fontTypeGet::Func, &Font::tit[2], "fontType", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Font_hasGlyphs::Func, &Font::tit[3], "hasGlyphs", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

    Font::Font(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::Font::Font()"
//##protect##"InstanceTraits::Font::Font()"

    }

    void Font::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<Font&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_text
{
    Font::Font(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::Font::Font()"
//##protect##"class_::Font::Font()"
    }
    void Font::enumerateFonts(SPtr<Instances::fl::Array>& result, bool enumerateDeviceFonts)
    {
//##protect##"class_::Font::enumerateFonts()"
        SF_UNUSED2(result, enumerateDeviceFonts);

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        MovieImpl* proot = asvm.GetMovieImpl();
        MovieDefImpl* pmoviedef = static_cast<MovieDefImpl*>(proot->GetMovieDef());

        // get fonts from ..
        HashSet<Ptr<Render::Font> > fonts;
        // fontresource
        struct FontsVisitor : public MovieDef::ResourceVisitor 
        { 
            HashSet<Ptr<Render::Font> >& Fonts;

            FontsVisitor(HashSet<Ptr<Render::Font> >& fonts) : Fonts(fonts) {}

            virtual void Visit(MovieDef*, Resource* presource, ResourceId, const char*) 
            { 
                if (presource->GetResourceType() == Resource::RT_Font)
                {
                    FontResource* pfontResource = static_cast<FontResource*>(presource);
                    Fonts.Set(pfontResource->GetFont());
                }
            }
        private:
            FontsVisitor& operator=(const FontsVisitor&) { return *this; }
        } fontsVisitor(fonts);
        pmoviedef->VisitResources(&fontsVisitor, MovieDef::ResVisit_Fonts);
        
        // registered fonts
        proot->LoadRegisteredFonts(fonts);

        // fontlib
        FontLib* pfontlib = proot->GetFontLib();
        if (pfontlib)
        {
            pfontlib->LoadFonts(fonts);
        }

        // returns an array of Font
        Class* fontClass = asvm.GetClass("flash.text.Font", asvm.GetCurrentAppDomain());
        SF_ASSERT(fontClass);

        Pickable<Instances::fl::Array> retVal = asvm.MakeArray();

        for (HashSet<Ptr<Render::Font> >::ConstIterator iter = fonts.Begin(); 
             iter != fonts.End(); ++iter)
        {
            // Create a Font object
            SPtr<Instances::fl_text::Font> font;
            asvm.ConstructInstance(font, fontClass, 0, NULL);
            SF_ASSERT(font);
            font->pFont = *iter;

            retVal->PushBack(Value(font));
        }

        if (enumerateDeviceFonts)
        {
            // fontprovider
            FontProvider* pfontprovider = proot->GetFontProvider();
            if (pfontprovider)
            {
                StringHash<Scaleform::String> deviceFontNames;
                pfontprovider->LoadFontNames(deviceFontNames);
                for (StringHash<Scaleform::String>::ConstIterator iter = deviceFontNames.Begin(); 
                     iter != deviceFontNames.End(); ++iter)
                {
                    SPtr<Instances::fl_text::Font> font;
                    asvm.ConstructInstance(font, fontClass, 0, NULL);
                    SF_ASSERT(font);
                    font->fontName = iter->First;

                    retVal->PushBack(Value(font));
                }
            }
        }

        result = retVal;
//##protect##"class_::Font::enumerateFonts()"
    }
    void Font::registerFont(const Value& result, Class* font)
    {
//##protect##"class_::Font::registerFont()"
        SF_UNUSED2(result, font);
        if (font && font->GetClassTraits().IsOfType(GetClassTraits()))
        {
            ASVM& asvm = static_cast<ASVM&>(GetVM());
            Font* fontClass = static_cast<Font*>(font);
            ASString className = fontClass->GetInstanceTraits().GetQualifiedName(Traits::qnfWithDot);
            
            VMAbcFile* vmabc = fontClass->GetTraits().GetFilePtr();
            if (vmabc)
            {
                const ASVM::AbcFileWithMovieDef& abc = 
                    static_cast<const ASVM::AbcFileWithMovieDef&>(vmabc->GetAbcFile());
                MovieDefImpl* pdefImpl = abc.pDefImpl;

                ResourceBindData    resBindData;
                if (asvm.GetMovieImpl()->FindExportedResource(pdefImpl, &resBindData, className.ToCStr()))
                {
                    if (resBindData.pResource)
                    {
                        if (resBindData.pResource->GetResourceType() & Resource::RT_Font)
                        {
                            FontResource* pfont = static_cast<FontResource*>(resBindData.pResource.GetPtr());
                            MovieImpl* proot = asvm.GetMovieImpl();

                            proot->RegisterFont(pdefImpl, pfont);
                        }
                    }
                }
            }
        }
        else
        {  
            // font should be a Font class
            // Error: Error #2136: The SWF file %%% contains invalid data.
            Scaleform::String name = "unknown";
            if (font)
            {
                VMAbcFile* vmabc = font->GetTraits().GetFilePtr();
                if (vmabc)
                {
                    name = vmabc->GetAbcFile().GetName();
                }
            }
            GetVM().ThrowArgumentError(VM::Error(VM::eSWFHasInvalidData, GetVM()
                SF_DEBUG_ARG(StringDataPtr(name)) 
                ));
        }
        //WARN_NOT_IMPLEMENTED("class_::Font::registerFont()");
//##protect##"class_::Font::registerFont()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc1<Classes::fl_text::Font, Classes::fl_text::Font::mid_enumerateFonts, SPtr<Instances::fl::Array>, bool> TFunc_Classes_Font_enumerateFonts;
typedef ThunkFunc1<Classes::fl_text::Font, Classes::fl_text::Font::mid_registerFont, const Value, Class*> TFunc_Classes_Font_registerFont;

template <> const TFunc_Classes_Font_enumerateFonts::TMethod TFunc_Classes_Font_enumerateFonts::Method = &Classes::fl_text::Font::enumerateFonts;
template <> const TFunc_Classes_Font_registerFont::TMethod TFunc_Classes_Font_registerFont::Method = &Classes::fl_text::Font::registerFont;

namespace ClassTraits { namespace fl_text
{
    // const UInt16 Font::tito[Font::ThunkInfoNum] = {
    //    0, 2, 
    // };
    const TypeInfo* Font::tit[4] = {
        &AS3::fl::ArrayTI, &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::ClassTI, 
    };
    const ThunkInfo Font::ti[Font::ThunkInfoNum] = {
        {TFunc_Classes_Font_enumerateFonts::Func, &Font::tit[0], "enumerateFonts", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {TFunc_Classes_Font_registerFont::Func, &Font::tit[2], "registerFont", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

    Font::Font(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::Font::Font()"
//##protect##"ClassTraits::Font::Font()"

    }

    Pickable<Traits> Font::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Font(vm, AS3::fl_text::FontCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_text::FontCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_text
{
    const TypeInfo FontTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_text::Font::InstanceType),
        ClassTraits::fl_text::Font::ThunkInfoNum,
        0,
        InstanceTraits::fl_text::Font::ThunkInfoNum,
        0,
        "Font", "flash.text", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo FontCI = {
        &FontTI,
        ClassTraits::fl_text::Font::MakeClassTraits,
        ClassTraits::fl_text::Font::ti,
        NULL,
        InstanceTraits::fl_text::Font::ti,
        NULL,
    };
}; // namespace fl_text


}}} // namespace Scaleform { namespace GFx { namespace AS3

