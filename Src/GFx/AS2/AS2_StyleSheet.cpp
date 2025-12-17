/**************************************************************************

Filename    :   AS2_StyleSheet.cpp
Content     :   StyleSheet (Textfield.StyleSheet) object functionality
Created     :   May 20, 2008
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/AS2/AS2_AvmTextField.h"
#include "Kernel/SF_File.h"
#include "GFx/GFx_PlayerImpl.h"

#include "GFx/AS2/AS2_StyleSheet.h"
#include "GFx/AS2/AS2_ArrayObject.h"
#include "GFx/AS2/AS2_AsBroadcaster.h"
#include "GFx/AS2/AS2_TextFormat.h"
#include "GFx/AS2/AS2_MovieRoot.h"

namespace Scaleform { namespace GFx { namespace AS2 {

#ifdef GFX_ENABLE_CSS
// ********************************************************************

//
// StyleSheet object's file loader implementations
// An instance of this object is sent to the GFx load queue for
// processing. It is thread safe, and therefore allows files
// to be loaded and processed asynchronously.
// This object performs both file loading and parsing
//
class CSSFileLoaderAndParserImpl : public ASCSSFileLoader
{
    enum StreamType
    {
        STREAM_UTF8,
        STREAM_UTF16,
    };

    StreamType                  Type;
    UByte*                      pFileData;
    int                         FileSize;

public:
    CSSFileLoaderAndParserImpl() : Type(STREAM_UTF8), pFileData(0), FileSize(0) {}
    virtual         ~CSSFileLoaderAndParserImpl();

    //
    // File loader implementation
    //
    void            Load(const String& filename, FileOpener* pfo);
    void            Init(Environment* penv, Object* pTarget);
};


//
// Destructor
//
CSSFileLoaderAndParserImpl::~CSSFileLoaderAndParserImpl()
{
    if (pFileData != NULL)
        SF_FREE(pFileData);
}

//
// Loads and parses the file and returns a GFxXMLDocument object
//
void CSSFileLoaderAndParserImpl::Load( const String& filename, FileOpener* pfo )
{
    // Could be on a separate thread here if thread support is enabled.

    Ptr<File> pFile;
    Array<UByte> bytes;
    if (URLBuilder::IsProtocol(filename))
    {
#ifdef SF_ENABLE_HTTP_LOADING
        if (URLBuilder::SendURLRequest(&bytes, filename) && !bytes.IsEmpty())
        {
            pFile = *SF_NEW MemoryFile(filename, bytes.GetDataPtr(), (int)bytes.GetSize());
        }
#endif
    }
    else
    {
        pFile = *pfo->OpenFile(filename);
    }
    if (pFile && pFile->IsValid())
    {
        if ((FileSize = pFile->GetLength()) != 0)
        {
            // Load the file into memory
            pFileData = (UByte*) SF_ALLOC(FileSize+2, StatMV_Text_Mem);
            pFile->Read(pFileData, FileSize);
            pFileData[FileSize] = pFileData[FileSize+1] = 0;
        }
    }
}

//
// Parse the CSS1 stream depending on its encoding
//
void CSSFileLoaderAndParserImpl::Init(Environment* penv, Object* pTarget)
{
    // Back on main thread at this point. Check if target is ok.
    StyleSheetObject* pASObj = static_cast<StyleSheetObject*>(pTarget);

    if ( !pFileData )
    {
        pASObj->NotifyOnLoad(penv, false);
    }
    else
    {
        wchar_t*            wcsptr = NULL;
        UByte*              ptextData = pFileData;
        int                 textLength = FileSize;

        // the following converts byte stream to appropriate endianness
        // for UTF16/UCS2 (wide char format)
        UInt16* prefix16 = (UInt16*)pFileData;
        if (prefix16[0] == Alg::ByteUtil::BEToSystem((UInt16)0xFFFE)) // little endian
        {
            prefix16++;
            ptextData = (UByte*)prefix16;
            textLength = (FileSize / 2) - 1;
            Type = STREAM_UTF16;
            if (sizeof(wchar_t) == 2)
            {
                for (int i=0; i < textLength; i++)
                    prefix16[i] = (wchar_t)Alg::ByteUtil::LEToSystem(prefix16[i]);
            }
            else
            {
                // special case: create an aux buffer to hold the data
                wcsptr = (wchar_t*) SF_ALLOC(textLength * sizeof(wchar_t), StatMV_Text_Mem);
                for (int i=0; i < textLength; i++)
                    wcsptr[i] = (wchar_t)Alg::ByteUtil::LEToSystem(prefix16[i]);
                ptextData = (UByte*)wcsptr;
            }
        }
        else if (prefix16[0] == Alg::ByteUtil::BEToSystem((UInt16)0xFEFF)) // big endian
        {
            prefix16++;
            ptextData = (UByte*)prefix16;
            textLength = (FileSize / 2) - 1;
            Type = STREAM_UTF16;
            if (sizeof(wchar_t) == 2)
            {
                for (int i=0; i < textLength; i++)
                    prefix16[i] = Alg::ByteUtil::BEToSystem(prefix16[i]);
            }
            else
            {
                wcsptr = (wchar_t*) SF_ALLOC(textLength * sizeof(wchar_t), StatMV_Text_Mem);
                for (int i=0; i < textLength; i++)
                    wcsptr[i] = Alg::ByteUtil::BEToSystem(prefix16[i]);
                ptextData = (UByte*)wcsptr;
            }
        }
        else if (FileSize > 2 && pFileData[0] == 0xEF && pFileData[1] == 0xBB && pFileData[2] == 0xBF)
        {
            // UTF-8 BOM explicitly specified
            ptextData += 3;
            textLength -= 3;
        }

        // else we assume encoding is ASCII or UTF8

        bool ret = false;
        switch (Type)
        {
        case STREAM_UTF16:
            ret = pASObj->CSS.ParseCSS((wchar_t*)ptextData, textLength);
            break;
        default:
            ret = pASObj->CSS.ParseCSS((char*)ptextData, textLength);
        }              

        // Cleanup
        if (wcsptr)
            SF_FREE(wcsptr);
        SF_FREE(pFileData);
        pFileData = NULL;

        pASObj->NotifyOnLoad( penv, ret );
    }
}


// ********************************************************************


StyleSheetObject::StyleSheetObject(Environment* penv)
: Object(penv)
{
    Set__proto__(penv->GetSC(), penv->GetPrototype(ASBuiltin_StyleSheet));

    AsBroadcaster::Initialize(penv->GetSC(), this);
    AsBroadcaster::AddListener(penv, this, this);
}


// 
// Invoked when a XML.load() or XML.sendAndLoad() operation has ended
//
void StyleSheetObject::NotifyOnLoad(Environment* penv, bool success)
{
    // Flash Doc: Invoked when a load() operation has completed. If the 
    // StyleSheet loaded successfully, the success parameter is true. If 
    // the document was not received, or if an error occurred in receiving 
    // the response from the server, the success parameter is false.
    if (success)
        CSS.SetState(Text::StyleManager::LoadingFinished);
    else
        CSS.SetState(Text::StyleManager::Error);
    penv->Push(success);
    AsBroadcaster::BroadcastMessage(penv, this, 
        penv->CreateConstString("onLoad"), 1, penv->GetTopIndex());
    penv->Drop1();    
}


////////////////////////////////////////////////
//
static const NameFunction GAS_StyleSheetFunctionTable[] = 
{
    { "clear", StyleSheetProto::Clear },
    { "getStyle", StyleSheetProto::GetStyle },
    { "getStyleNames", StyleSheetProto::GetStyleNames },
    { "load", StyleSheetProto::Load },
    { "parseCSS", StyleSheetProto::ParseCSS },
    { "setStyle", StyleSheetProto::SetStyle },
    { "transform", StyleSheetProto::Transform },
    { 0, 0 }
};

StyleSheetProto::StyleSheetProto(ASStringContext* psc, Object* pprototype, const FunctionRef& constructor) : 
    Prototype<StyleSheetObject>(psc, pprototype, constructor)
{
    InitFunctionMembers(psc, GAS_StyleSheetFunctionTable);
}


//
// StyleSheet.Clear() : Void
//
void    StyleSheetProto::Clear(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, StyleSheet);
    StyleSheetObject* pthis = (StyleSheetObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;    

    // Flash Doc: Removes all styles from the specified StyleSheet object.

    pthis->CSS.ClearStyles();
}


//
// StyleSheet.getStyle(name:String) : Object
//
void    StyleSheetProto::GetStyle(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, StyleSheet);
    StyleSheetObject* pthis = (StyleSheetObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;    

    // Flash Doc: Returns a copy of the style object associated with the specified 
    // style (name). If there is no style object associated with name, the method 
    // returns null.

    if (fn.NArgs < 1)
    {
        fn.Result->SetNull();
        return;
    }
    
    ASString stylename = fn.Arg(0).ToString(fn.Env);
    const Text::Style* pstyle = NULL;
    if (stylename.GetSize() > 0 && stylename[0] == '.')
        pstyle = pthis->CSS.GetStyle(Text::StyleManager::CSS_Class, stylename.ToCStr()+1, stylename.GetSize()-1);
    else
        pstyle = pthis->CSS.GetStyle(Text::StyleManager::CSS_Tag, stylename.ToCStr(), stylename.GetSize());
    if (!pstyle)
    {
        fn.Result->SetNull();
        return;
    }

    Ptr<Object> pobj = *SF_HEAP_NEW(fn.Env->GetHeap()) Object(fn.Env);
    ASStringContext* psc = fn.Env->GetSC();
    // fill in all set properties
    const Text::TextFormat& tf = pstyle->mTextFormat;
    const Text::ParagraphFormat& pf = pstyle->mParagraphFormat;

    // color
    if (tf.IsColorSet())
    {
        String colorStr;
        // create html color string
        colorStr.AppendChar('#');
        Color color = tf.GetColor();
        const char* refstr = "0123456789ABCDEF";
        UByte comp = color.GetRed();
        colorStr.AppendChar(refstr[comp >> 4]);
        colorStr.AppendChar(refstr[comp & 0x0F]);
        comp = color.GetGreen();
        colorStr.AppendChar(refstr[comp >> 4]);
        colorStr.AppendChar(refstr[comp & 0x0F]);
        comp = color.GetBlue();
        colorStr.AppendChar(refstr[comp >> 4]);
        colorStr.AppendChar(refstr[comp & 0x0F]);
        pobj->SetMember(fn.Env, psc->CreateString("color"), psc->CreateString(colorStr));
    }

    // display
    // TODO

    // fontFamily
    if (tf.IsFontListSet())
        pobj->SetMember(fn.Env, psc->CreateString("fontFamily"),
            psc->CreateString(tf.GetFontList()));

    // fontSize
    if (tf.IsFontSizeSet())
        pobj->SetMember(fn.Env, psc->CreateString("fontSize"),
        (Number)tf.GetFontSize());

    // fontStyle
    if (tf.IsItalicSet())
        pobj->SetMember(fn.Env, psc->CreateString("fontStyle"), 
        tf.IsItalic() ? psc->CreateString("italic") : psc->CreateString("normal"));

    // fontWeight
    if (tf.IsBoldSet())
        pobj->SetMember(fn.Env, psc->CreateString("fontWeight"), 
        tf.IsBold() ? psc->CreateString("bold") : psc->CreateString("normal"));

    // kerning
    if (tf.IsKerningSet())
        pobj->SetMember(fn.Env, psc->CreateString("kerning"), 
        tf.IsKerning() ? psc->CreateString("true") : psc->CreateString("false"));

    // letterSpacing
    if (tf.IsLetterSpacingSet())
        pobj->SetMember(fn.Env, psc->CreateString("letterSpacing"),
        (Number)tf.GetLetterSpacing());

    // marginLeft
    if (pf.IsLeftMarginSet())
        pobj->SetMember(fn.Env, psc->CreateString("marginLeft"),
        (Number)pf.GetLeftMargin());

    // marginRight
    if (pf.IsRightMarginSet())
        pobj->SetMember(fn.Env, psc->CreateString("marginRight"),
        (Number)pf.GetRightMargin());

    // textAlign
    if (pf.IsAlignmentSet())
        pobj->SetMember( fn.Env, psc->CreateString("textAlign"), 
            pf.IsLeftAlignment() ? psc->CreateString("left") : 
            pf.IsCenterAlignment() ? psc->CreateString("center") :
            pf.IsRightAlignment() ? psc->CreateString("right") : 
            psc->CreateString("justify") );

    // textDecoration
    if (tf.IsUnderlineSet())
        pobj->SetMember(fn.Env, psc->CreateString("textDecoration"), 
        tf.IsUnderline() ? psc->CreateString("underline") : psc->CreateString("none"));

    // textIndent
    if (pf.IsIndentSet())
        pobj->SetMember(fn.Env, psc->CreateString("textIndent"),
        (Number)pf.GetIndent());

    fn.Result->SetAsObject(pobj);
}


//
// StyleSheet.getStyleNames() : Array
//
void    StyleSheetProto::GetStyleNames(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, StyleSheet);
    StyleSheetObject* pthis = (StyleSheetObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;    

    // Flash Doc: Returns an array that contains the names (as strings) of all 
    // of the styles registered in this style sheet.

    Ptr<ArrayObject> parr = *static_cast<ArrayObject*>(
        fn.Env->OperatorNew(fn.Env->GetBuiltin(ASBuiltin_Array)));
    ASStringContext* psc = fn.Env->GetSC();
    String temp;
    for (Text::StyleHash::ConstIterator iter = pthis->CSS.GetStyles().Begin(); 
            iter != pthis->CSS.GetStyles().End(); ++iter)
    {
        temp.Clear();
        if (iter->First.Type == Text::StyleManager::CSS_Class)
            temp.AppendChar('.');
        temp += iter->First.Value;
        parr->PushBack( psc->CreateString(temp) );
    }
    fn.Result->SetAsObject(parr);
}


//
// StyleSheet.load(url:String) : Boolean
//
void    StyleSheetProto::Load(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, StyleSheet);
    StyleSheetObject* pthis = (StyleSheetObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;    

    // Flash Doc: Starts loading the CSS file into the StyleSheet. The load 
    // operation is asynchronous; use the onLoad() callback handler to determine 
    // when the file has finished loading. The CSS file must reside in the same 
    // domain as the SWF file that is loading it.

    if (fn.NArgs == 0)
    {
        fn.Result->SetBool(false);
        return;
    }
    ASString urlStr(fn.Arg(0).ToString(fn.Env));

    pthis->CSS.SetState(Text::StyleManager::Loading);
    // new ?! MA: Is this the right heap for loader?
    // PPS: Create the loader in the global heap
    Ptr<ASCSSFileLoader> pLoader = 
        *new CSSFileLoaderAndParserImpl();
    fn.Env->GetAS2Root()->AddCssLoadQueueEntry(pthis, pLoader, urlStr.ToCStr(), 
        LoadQueueEntry::LM_None);
    fn.Result->SetBool(true);
}


//
// StyleSheet.parseCSS(cssText:String) : Boolean
//
void    StyleSheetProto::ParseCSS(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, StyleSheet);
    StyleSheetObject* pthis = (StyleSheetObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;    

    // Flash Doc: Parses the CSS in cssText and loads the StyleSheet with it. 
    // If a style in cssText is already in the StyleSheet, the StyleSheet 
    // retains its properties, and only the ones in cssText are added or changed. 

    if (fn.NArgs < 1)
    {
        fn.Result->SetBool(false);
        return;
    }

    Value csstxt = fn.Arg(0);
    ASString cssstr = csstxt.ToString(fn.Env);
    fn.Result->SetBool( pthis->CSS.ParseCSS( cssstr.ToCStr(), cssstr.GetSize() ) );
}


// 
// Member visitor to collate style properties
//
struct CSSStringBuilder : public Object::MemberVisitor
{
    Environment* pEnv;
    String& Dest;
    CSSStringBuilder(Environment* penv, String& dest) 
        : pEnv(penv), Dest(dest) {}
    CSSStringBuilder& operator=( const CSSStringBuilder& x )
    {
        pEnv = x.pEnv;
        Dest = x.Dest;
        return *this;
    }
    void Visit(const ASString& name, const Value& val, UByte flags)
    {
        SF_UNUSED(flags);
        // translate from Flash prop to CSS1 prop
        if (name == "fontFamily")               Dest += "font-family";
        else if (name == "fontSize")            Dest += "font-size";
        else if (name == "fontStyle")           Dest += "font-style";
        else if (name == "fontWeight")          Dest += "font-weight";
        else if (name == "letterSpacing")       Dest += "letter-spacing";
        else if (name == "marginLeft")          Dest += "margin-left";
        else if (name == "marginRight")         Dest += "margin-right";
        else if (name == "textAlign")           Dest += "text-align";
        else if (name == "textDecoration")      Dest += "text-decoration";
        else if (name == "textIndent")          Dest += "text-indent";
        else                                    Dest += name.ToCStr();
        Dest += ":";
        Dest += val.ToString(pEnv).ToCStr();
        Dest += ";";        
    }
};

//
// StyleSheet.setStyle(name:String, style:Object) : Void
//
void    StyleSheetProto::SetStyle(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, StyleSheet);
    StyleSheetObject* pthis = (StyleSheetObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;    

    // Flash Doc: Adds a new style with the specified name to the StyleSheet 
    // object. If the named style does not already exist in the StyleSheet, 
    // it is added. If the named style already exists in the StyleSheet, it 
    // is replaced. If the style parameter is null, the named style is removed. 

    if (fn.NArgs < 1)
        return;

    if (fn.NArgs < 2 || fn.Arg(1).IsNull())
    {
        // remove style
        ASString stylename = fn.Arg(0).ToString(fn.Env);
        if (stylename.GetSize() > 0)
        {
            if (stylename[0] == '.')
                pthis->CSS.ClearStyle(Text::StyleManager::CSS_Class, stylename.ToCStr());
            else
                pthis->CSS.ClearStyle(Text::StyleManager::CSS_Tag, stylename.ToCStr());
        }
        return;
    }
    else
    {
        // replace style
        ASString stylename = fn.Arg(0).ToString(fn.Env);
        Object* pobj = fn.Arg(1).ToObject(fn.Env);
        if (!pobj)
        {
            SF_DEBUG_WARNING(1, "Style is not an Object");
            return;
        }
        // build a css string with given info
        String cssstr;
        cssstr.AppendString(stylename.ToCStr());
        cssstr.AppendChar('{');
        CSSStringBuilder propvis(fn.Env, cssstr);
        pobj->VisitMembers(fn.Env->GetSC(), &propvis, 0);
        cssstr.AppendChar('}');
        // parse it using the textstylemanager
        pthis->CSS.ParseCSS(cssstr.ToCStr(), cssstr.GetSize());
    }
}


// 
// Member visitor to collate style properties
//
struct CSSTextFormatLoader : public Object::MemberVisitor
{
    Environment* pEnv;
    TextFormatObject* pTFO;
    CSSTextFormatLoader(Environment* penv, TextFormatObject* dest) 
        : pEnv(penv), pTFO(dest) {}
    CSSTextFormatLoader& operator=( const CSSTextFormatLoader& x )
    {
        pEnv = x.pEnv;
        pTFO = x.pTFO;
        return *this;
    }
    void Visit(const ASString& name, const Value& val, UByte flags)
    {
        SF_UNUSED(flags);
        // switch on valid props
        ASString valstr = val.ToString(pEnv);
        const char* pstr = valstr.ToCStr();
        char* temp = NULL;
        unsigned sz = valstr.GetSize();
        if (name == "color")
        {
            UInt32 colval = SFstrtol(pstr+1, &temp, 16);
            pTFO->mTextFormat.SetColor32(colval);
        }
        else if (name == "display")
        {
            // TODO: Not supported currently
        }
        else if (name == "fontFamily")
        {
            pTFO->mTextFormat.SetFontList(pstr, sz);
        }
        else if (name == "fontSize")            
        {
            float num = (float)SFstrtod(pstr, &temp);
            pTFO->mTextFormat.SetFontSize(num);
        }
        else if (name == "fontStyle")           
        {
            if ( !SFstrncmp("normal", pstr, (sz < 4) ? sz : 4) ) 
                pTFO->mTextFormat.SetItalic(false);        
            else if ( !SFstrncmp("italic", pstr, (sz < 9) ? sz : 9) )
                pTFO->mTextFormat.SetItalic(true);        
        }
        else if (name == "fontWeight")          
        {
            if ( !SFstrncmp("normal", pstr, (sz < 6) ? sz : 6) ) 
                pTFO->mTextFormat.SetBold(false);
            else if ( !SFstrncmp("bold", pstr, (sz < 4) ? sz : 4) )
                pTFO->mTextFormat.SetBold(true);
        }
        else if (name == "kerning")
        {
            if ( !SFstrncmp("false", pstr, (sz < 5) ? sz : 5) )
                pTFO->mTextFormat.SetKerning(false);        
            else if ( !SFstrncmp("true", pstr, (sz < 4) ? sz : 4) ) 
                pTFO->mTextFormat.SetKerning(true);        
        }
        else if (name == "letterSpacing")       
        {
            float num = (float)SFstrtod(pstr, &temp);
            pTFO->mTextFormat.SetLetterSpacing(num);
        }
        else if (name == "marginLeft")          
        {
            float num = (float)SFstrtod(pstr, &temp);
            pTFO->mParagraphFormat.SetLeftMargin((unsigned)num);
        }
        else if (name == "marginRight")         
        {
            float num = (float)SFstrtod(pstr, &temp);
            pTFO->mParagraphFormat.SetRightMargin((unsigned)num);
        }
        else if (name == "textAlign")           
        {
            if ( !SFstrncmp("left", pstr, (sz < 4) ? sz : 4) ) 
                pTFO->mParagraphFormat.SetAlignment( Text::ParagraphFormat::Align_Left );
            else if ( !SFstrncmp("center", pstr, (sz < 6) ? sz : 6) )
                pTFO->mParagraphFormat.SetAlignment( Text::ParagraphFormat::Align_Center );
            else if ( !SFstrncmp("right", pstr, (sz < 5) ? sz : 5) ) 
                pTFO->mParagraphFormat.SetAlignment( Text::ParagraphFormat::Align_Right );
            else if ( !SFstrncmp("justify", pstr, (sz < 7) ? sz : 7) )
                pTFO->mParagraphFormat.SetAlignment( Text::ParagraphFormat::Align_Justify );
        }
        else if (name == "textDecoration")      
        {
            if ( !SFstrncmp("none", pstr, (sz < 4) ? sz : 4) ) 
                pTFO->mTextFormat.SetUnderline(false);        
            else if ( !SFstrncmp("underline", pstr, (sz < 9) ? sz : 9) )
                pTFO->mTextFormat.SetUnderline(true);        
        }
        else if (name == "textIndent")          
        {
            float num = (float)SFstrtod(pstr, &temp);
            pTFO->mParagraphFormat.SetIndent((int)num);
        }
    }
};

//
// StyleSheet.transform(style:Object) : TextFormat
//
void    StyleSheetProto::Transform(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, StyleSheet);
    StyleSheetObject* pthis = (StyleSheetObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;    

    // Flash Doc: Extends the CSS parsing capability. Advanced developers can 
    // override this method by extending the StyleSheet class.
    // (NOTE: By default this method returns a TextFormat object with
    // the style properties)

    if (fn.NArgs < 1)
        return;

    Object* pobj = fn.Arg(0).ToObject(fn.Env);
    if (!pobj)
        return;

    Ptr<TextFormatObject> ptfo = *static_cast<TextFormatObject*>(
        fn.Env->OperatorNew(fn.Env->GetBuiltin(ASBuiltin_TextFormat)));
    
    CSSTextFormatLoader tfl(fn.Env, ptfo);
    pobj->VisitMembers(fn.Env->GetSC(), &tfl, 0);

    fn.Result->SetAsObject(ptfo);
}


////////////////////////////////////////////////
//
StyleSheetCtorFunction::StyleSheetCtorFunction(ASStringContext *psc) :
    CFunctionObject(psc, GlobalCtor)
{
    SF_UNUSED(psc);
}

void StyleSheetCtorFunction::GlobalCtor(const FnCall& fn)
{
    Ptr<StyleSheetObject> ab;
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_StyleSheet && !fn.ThisPtr->IsBuiltinPrototype())
        ab = static_cast<StyleSheetObject*>(fn.ThisPtr);
    else
        ab = *SF_HEAP_NEW(fn.Env->GetHeap()) StyleSheetObject(fn.Env);
    
    fn.Result->SetAsObject(ab.GetPtr());
}

Object* StyleSheetCtorFunction::CreateNewObject(Environment* penv) const
{
    return SF_HEAP_NEW(penv->GetHeap()) StyleSheetObject(penv);
}

FunctionRef StyleSheetCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) StyleSheetCtorFunction(&sc));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) StyleSheetProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_StyleSheet, proto);
    
    FunctionRef textFieldCtor = static_cast<TextFieldProto*>(pgc->GetPrototype(ASBuiltin_TextField))->GetConstructor();
    textFieldCtor->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_StyleSheet), Value(ctor)); 

    return ctor;
}

#else

void GASStyleSheet_DummyFunction() {}   // Exists to quelch compiler warning

#endif // GFX_ENABLE_CSS

}}} // SF::GFx::AS2
