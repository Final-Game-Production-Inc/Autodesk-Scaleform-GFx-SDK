/**************************************************************************

Filename    :   AS2_TextSnapshot.cpp
Content     :   TextSnapshot reference class for ActionScript 2.0
Created     :   12/10/2008
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"

#ifdef GFX_AS_ENABLE_TEXTSNAPSHOT

#include "GFx/AS2/AS2_TextSnapshot.h"

#include "GFx/GFx_Sprite.h"

#include "GFx/AS2/AS2_ArrayObject.h"

namespace Scaleform { namespace GFx { namespace AS2 {

TextSnapshotObject::TextSnapshotObject(Environment* penv)
: Object(penv)
{
    Set__proto__(penv->GetSC(), penv->GetPrototype(ASBuiltin_TextSnapshot));
}

TextSnapshotObject::~TextSnapshotObject()
{

}

void TextSnapshotObject::Process(Sprite* pmovieClip)
{
    pmovieClip->GetTextSnapshot(&SnapshotData);
}

// --------------------------------------------------------------------

const NameFunction TextSnapshotProto::FunctionTable[] = 
{
    { "findText", &TextSnapshotProto::FindText },
    { "getCount", &TextSnapshotProto::GetCount },
    { "getSelected", &TextSnapshotProto::GetSelected },
    { "getSelectedText", &TextSnapshotProto::GetSelectedText },
    { "getText", &TextSnapshotProto::GetText },
    { "getTextRunInfo", &TextSnapshotProto::GetTextRunInfo },
    { "hitTestTextNearPos", &TextSnapshotProto::HitTestTextNearPos },
    { "setSelectColor", &TextSnapshotProto::SetSelectColor },
    { "setSelected", &TextSnapshotProto::SetSelected },
    { 0, 0 }
};


TextSnapshotProto::TextSnapshotProto(ASStringContext *psc, Object* prototype, const FunctionRef& constructor) :
Prototype<TextSnapshotObject>(psc, prototype, constructor)
{
    InitFunctionMembers(psc, FunctionTable, PropFlags::PropFlag_ReadOnly | PropFlags::PropFlag_DontDelete);
}


//
// findText (TextSnapshot.findText method)
//
// public findText(startIndex:Number, textToFind:String, caseSensitive:Boolean) : Number
//
// Searches the specified TextSnapshot object and returns the position 
// of the first occurrence of textToFind found at or after startIndex. 
// If textToFind is not found, the method returns -1.
//
void    TextSnapshotProto::FindText(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, TextSnapshot);
    TextSnapshotObject* pthis = (TextSnapshotObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    if (fn.NArgs < 3)
        return;

    UInt32      start = fn.Arg(0).ToUInt32(fn.Env);
    ASString   text = fn.Arg(1).ToString(fn.Env);
    bool        bcaseSensitive = fn.Arg(2).ToBool(fn.Env);

    String     query(text.ToCStr(), text.GetSize());
    fn.Result->SetInt( pthis->GetData().FindText(start, query, bcaseSensitive) );
}

//
// getCount (TextSnapshot.getCount method)
//
// public getCount() : Number
// 
// Returns the number of characters in a TextSnapshot object.
//
void    TextSnapshotProto::GetCount(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, TextSnapshot);
    TextSnapshotObject* pthis = (TextSnapshotObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    fn.Result->SetUInt((unsigned)pthis->GetData().GetCharCount());
}


//
// getSelected (TextSnapshot.getSelected method)
//
// public getSelected(start:Number, [end:Number]) : Boolean
//
// Returns a Boolean value that specifies whether a TextSnapshot 
// object contains selected text in the specified range. To search 
// all characters, pass a value of 0 for start, and TextSnapshot.getCount() 
// (or any very large number) for end. To search a single character, pass 
// the end parameter a value that is one greater than the start parameter.
//
void    TextSnapshotProto::GetSelected(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, TextSnapshot);
    TextSnapshotObject* pthis = (TextSnapshotObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    if (fn.NArgs < 1)
        return;

    UInt32  start = fn.Arg(0).ToUInt32(fn.Env);
    UInt32  end = (fn.NArgs > 1) ? fn.Arg(1).ToUInt32(fn.Env) : 
        UInt32(pthis->GetData().GetCharCount());

    if (end <= start)
        end = start + 1;

    fn.Result->SetBool(pthis->GetData().IsSelected(start, end));
}


//
// getSelectedText (TextSnapshot.getSelectedText method)
//
// public getSelectedText([includeLineEndings:Boolean]) : String
// 
// Returns a string that contains all the characters specified by the 
// corresponding TextSnapshot.setSelected() method. If no characters 
// are specified (by the TextSnapshot.setSelected() method), an empty 
// string is returned. If you pass true for includeLineEndings, newline 
// characters are inserted in the return string, and the return string 
// might be longer than the input range. If includeLineEndings is false 
// or omitted, the method returns the selected text without adding any 
// characters.
//
void    TextSnapshotProto::GetSelectedText(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, TextSnapshot);
    TextSnapshotObject* pthis = (TextSnapshotObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    bool    binclLineEndings = (fn.NArgs > 0) ? fn.Arg(0).ToBool(fn.Env) : false;

    String   selectedText = pthis->GetData().GetSelectedText(binclLineEndings);
    fn.Result->SetString(fn.Env->CreateString(selectedText));
}


//
// getText (TextSnapshot.getText method)
//
// public getText(start:Number, end:Number, [includeLineEndings:Boolean]) : String
//
// Returns a string that contains all the characters specified by the 
// start and end parameters. If no characters are specified, the method 
// returns an empty string. 
// 
// To return all characters, pass a value of 0 for start, and TextSnapshot.getCount() 
// (or any very large number) for end. To return a single character, pass a value of 
// start +1 for end. 
// 
// If you pass true for includeLineEndings, newline characters are inserted in the 
// return string where deemed appropriate, and the return string might be longer than 
// the input range. If includeLineEndings is false or omitted, the method returns the 
// selected text without adding any characters.
// 
void    TextSnapshotProto::GetText(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, TextSnapshot);
    TextSnapshotObject* pthis = (TextSnapshotObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    if (fn.NArgs < 2)
        return;

    UInt32  start = fn.Arg(0).ToUInt32(fn.Env);
    UInt32  end = fn.Arg(1).ToUInt32(fn.Env);
    bool    bincLineEndings = (fn.NArgs < 3) ? false : fn.Arg(2).ToBool(fn.Env);

    if (end <= start)
        end = start + 1;

    String   subStr = pthis->GetData().GetSubString(start, end, bincLineEndings);
    fn.Result->SetString(fn.Env->CreateString(subStr));
}



class GASTextSnapshotGlyphVisitor : public StaticTextSnapshotData::GlyphVisitor
{
public:
    GASTextSnapshotGlyphVisitor(Environment* penv, ArrayObject* parr)
        : pEnv(penv), pArrayObj(parr) {}

    void    OnVisit()
    {
        // Create new object
        Ptr<Object> pobj = *SF_HEAP_NEW(pEnv->GetHeap()) Object(pEnv);

        // Fill object
        Value val;
        val.SetUInt((unsigned)GetRunIndex());
        pobj->SetMember(pEnv, pEnv->CreateConstString("indexInRun"), val);
        pobj->SetMember(pEnv, pEnv->CreateConstString("font"), pEnv->CreateString(GetFontName()));
        // Setting unsigned below produces a negative number..??
        val.SetNumber((float)GetColor().ToColor32());
        pobj->SetMember(pEnv, pEnv->CreateConstString("color"), val);
        val.SetNumber(GetHeight());
        pobj->SetMember(pEnv, pEnv->GetBuiltin(ASBuiltin_height), val);
        val.SetBool(IsSelected());
        pobj->SetMember(pEnv, pEnv->CreateConstString("selected"), val);

        const Render::Matrix2F& mat = GetMatrix();
        val.SetNumber(TwipsToPixels(mat.Sx()));
        pobj->SetMember(pEnv, pEnv->CreateString("matrix_a"), val);
        val.SetNumber(TwipsToPixels(mat.Shy()));
        pobj->SetMember(pEnv, pEnv->CreateString("matrix_b"), val);
        val.SetNumber(TwipsToPixels(mat.Shx()));
        pobj->SetMember(pEnv, pEnv->CreateString("matrix_c"), val);
        val.SetNumber(TwipsToPixels(mat.Sy()));
        pobj->SetMember(pEnv, pEnv->CreateString("matrix_d"), val);
        val.SetNumber(TwipsToPixels(mat.Tx()));
        pobj->SetMember(pEnv, pEnv->CreateString("matrix_tx"), val);
        val.SetNumber(TwipsToPixels(mat.Ty()));
        pobj->SetMember(pEnv, pEnv->CreateString("matrix_ty"), val);

        const Render::PointF& bl = GetCorners().BottomLeft();
        const Render::PointF& br = GetCorners().BottomRight();
        const Render::PointF& tl = GetCorners().TopLeft();
        const Render::PointF& tr = GetCorners().TopRight();
        val.SetNumber(TwipsToPixels(bl.x));
        pobj->SetMember(pEnv, pEnv->CreateString("corner0x"), val);
        val.SetNumber(TwipsToPixels(bl.y));
        pobj->SetMember(pEnv, pEnv->CreateString("corner0y"), val);
        val.SetNumber(TwipsToPixels(br.x));
        pobj->SetMember(pEnv, pEnv->CreateString("corner1x"), val);
        val.SetNumber(TwipsToPixels(br.y));
        pobj->SetMember(pEnv, pEnv->CreateString("corner1y"), val);
        val.SetNumber(TwipsToPixels(tr.x));
        pobj->SetMember(pEnv, pEnv->CreateString("corner2x"), val);
        val.SetNumber(TwipsToPixels(tr.y));
        pobj->SetMember(pEnv, pEnv->CreateString("corner2y"), val);
        val.SetNumber(TwipsToPixels(tl.x));
        pobj->SetMember(pEnv, pEnv->CreateString("corner3x"), val);
        val.SetNumber(TwipsToPixels(tl.y));
        pobj->SetMember(pEnv, pEnv->CreateString("corner3y"), val);

        // Add to array
        pArrayObj->PushBack(Value(pobj));
    }

private:
    Environment*     pEnv;
    ArrayObject*     pArrayObj;
};

//
// getTextRunInfo (TextSnapshot.getTextRunInfo method)
//
// public getTextRunInfo(beginIndex:Number, endIndex:Number) : Array
//
// Returns an array of objects that contains information about a run 
// of text. Each object corresponds to one character in the range of 
// characters specified by the two method parameters. 
//
// Note: Using the getTextRunInfo() method for a large range of text 
// can return a large object. Macromedia recommends limiting the text 
// range defined by the beginIndex and endIndex parameters. 
//
void    TextSnapshotProto::GetTextRunInfo(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, TextSnapshot);
    TextSnapshotObject* pthis = (TextSnapshotObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    if (fn.NArgs < 2)
        return;

    UInt32  start = fn.Arg(0).ToUInt32(fn.Env);
    UInt32  end = fn.Arg(1).ToUInt32(fn.Env);

    Ptr<ArrayObject> parr = *SF_HEAP_NEW(fn.Env->GetHeap()) ArrayObject(fn.Env);

    //
    // Returns
    // Array - An array of objects in which each object contains 
    // information about a specific character in the specified range. 
    // Each object contains the following properties: 
    //
    // indexInRun - A zero-based integer index of the character 
    //              (relative to the entire string rather than the 
    //              selected run of text). 
    // selected - A Boolean value that indicates whether the character 
    //            is selected true; false otherwise. 
    // font     - The name of the character's font. 
    // color    - The combined alpha and color value of the character. 
    //            The first two hexidecimal digits represent the alpha 
    //            value, and the remaining digits represent the color 
    //            value. (The example includes a method for converting 
    //            decimal values to hexidecimal values.) 
    // height   - The height of the character, in pixels. 
    // matrix_a, matrix_b, matrix_c, matrix_d, matrix_tx, and matrix_ty - 
    //          The values of a matrix that define the geometric 
    //          transformation on the character. Normal, upright text 
    //          always has a matrix of the form [1 0 0 1 x y], where x 
    //          and y are the position of the character within the 
    //          parent movie clip, regardless of the height of the text. 
    //          The matrix is in the parent movie clip coordinate system, 
    //          and does not include any transformations that may be on 
    //          that movie clip itself (or its parent). 
    // corner0x, corner0y, corner1x, corner1y, corner2x, corner2y, 
    // corner3x, and corner3y - 
    //          The corners of the bounding box of the character, based 
    //          on the coordinate system of the parent movie clip. These 
    //          values are only available if the font used by the 
    //          character is embedded in the SWF file. 
    //
    GASTextSnapshotGlyphVisitor visitor(fn.Env, parr);
    pthis->GetData().Visit(&visitor, start, end);

    fn.Result->SetAsObject(parr);
}


//
// hitTestTextNearPos (TextSnapshot.hitTestTextNearPos method)
//
// public hitTestTextNearPos(x:Number, y:Number, [closeDist:Number]) : Number
//
// Lets you determine which character within a TextSnapshot object is 
// on or near the specified x, y coordinates of the movie clip containing 
// the text in the TextSnapshot object. 
// 
// If you omit or pass a value of 0 for closeDist, the location 
// specified by the x, y coordinates must lie inside the bounding box 
// of the TextSnapshot object. 
//
// This method works correctly only with fonts that include character 
// metric information; however, by default, Macromedia Flash does not 
// include this information for static text fields. Therefore, the 
// method might return -1 instead of an index value. To ensure that an 
// index value is returned, you can force the Flash authoring tool to 
// include the character metric information for a font. To do this, 
// add a dynamic text field that uses that font, select Character 
// Options for that dynamic text field, and then specify that font 
// outlines should be embedded for at least one character. (It doesn't 
// matter which characters you specify, nor whether they are the 
// characters used in the static text fields.)
//
void    TextSnapshotProto::HitTestTextNearPos(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, TextSnapshot);
    TextSnapshotObject* pthis = (TextSnapshotObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    if (fn.NArgs < 2)
        return;

    float   x = (float)fn.Arg(0).ToNumber(fn.Env);
    float   y = (float)fn.Arg(1).ToNumber(fn.Env);
    float   closeDist = (fn.NArgs > 2) ? (float)fn.Arg(2).ToNumber(fn.Env) : 0;

    // Returns the char Index in the snapshot, if cursor is over
    // static text char
    int idx = pthis->GetData().HitTestTextNearPos(float(PixelsToTwips(x)), 
                        float(PixelsToTwips(y)), (float)(PixelsToTwips(closeDist)));
    fn.Result->SetNumber(Number(idx));
}


//
// setSelectColor (TextSnapshot.setSelectColor method)
//
// public setSelectColor(color:Number) : Void
//
// Specifies the color to use when highlighting characters that were 
// selected with the TextSnapshot.setSelected() method. The color is 
// always opaque; you can't specify a transparency value. 
//
// This method works correctly only with fonts that include character 
// metric information; however, by default, Macromedia Flash does not 
// include this information for static text fields. Therefore, the 
// method might return -1 instead of an index value. To ensure that an 
// index value is returned, you can force the Flash authoring tool to 
// include the character metric information for a font. To do this, 
// add a dynamic text field that uses that font, select Character 
// Options for that dynamic text field, and then specify that font 
// outlines should be embedded for at least one character. (It doesn't 
// matter which characters you specify, nor if they are the characters 
// used in the static text fields.)
//
void    TextSnapshotProto::SetSelectColor(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, TextSnapshot);
    TextSnapshotObject* pthis = (TextSnapshotObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    if (fn.NArgs < 1)
        return;

    UInt32  c32 = fn.Arg(0).ToUInt32(fn.Env);

    Color color(c32);
    color.SetAlpha(255);
    pthis->SetSelectColor(color);
}


//
// setSelected (TextSnapshot.setSelected method)
//
// public setSelected(start:Number, end:Number, select:Boolean) : Void
//
// Specifies a range of characters in a TextSnapshot object to be 
// selected or not. Characters that are selected are drawn with a 
// colored rectangle behind them, matching the bounding box of the 
// character. The color of the bounding box is defined by 
// TextSnapshot.setSelectColor(). 
//
// To select or deselect all characters, pass a value of 0 for start 
// and TextSnapshot.getCount() (or any very large number) for end. To 
// specify a single character, pass a value of start + 1 for end. 
// 
// Because characters are individually marked as selected, you can 
// call this method multiple times to select multiple characters; that 
// is, using this method does not deselect other characters that have 
// been set by this method.
// 
// This method works correctly only with fonts that include character 
// metric information; by default, Flash does not include this 
// information for static text fields. Therefore, text that is selected 
// might not appear to be selected onscreen. To ensure that all selected 
// text appears to be selected, you can force the Flash authoring tool 
// to include the character metric information for a font. To do this, 
// in the library, include the font used by the static text field, and 
// in Linkage options for the font, select Export for ActionScript.
//
void    TextSnapshotProto::SetSelected(const FnCall &fn)
{
    CHECK_THIS_PTR(fn, TextSnapshot);
    TextSnapshotObject* pthis = (TextSnapshotObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    if (fn.NArgs < 3)
        return;

    UInt32  start = fn.Arg(0).ToUInt32(fn.Env);
    UInt32  end = fn.Arg(1).ToUInt32(fn.Env);
    bool    bselect = fn.Arg(2).ToBool(fn.Env);

    if (end <= start)
        end = start + 1;

    pthis->GetData().SetSelected(start, end, bselect);
}


// --------------------------------------------------------------------


TextSnapshotCtorFunction::TextSnapshotCtorFunction(ASStringContext* psc)
: CFunctionObject(psc, NULL)
{
    SF_UNUSED(psc);
}


FunctionRef TextSnapshotCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef ctor(*SF_HEAP_NEW(pgc->GetHeap()) TextSnapshotCtorFunction(&sc));
    Ptr<TextSnapshotProto> proto = 
        *SF_HEAP_NEW(pgc->GetHeap()) TextSnapshotProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_TextSnapshot, proto);
    pgc->pGlobal->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_TextSnapshot), Value(ctor));
    return ctor;
}

}}} //SF::GFx::AS2

#endif  // SF_NO_FXPLAYER_AS_TEXTSNAPSHOT
