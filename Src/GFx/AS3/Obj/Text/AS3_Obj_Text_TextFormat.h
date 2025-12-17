//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Text_TextFormat.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Text_TextFormat_H
#define INC_AS3_Obj_Text_TextFormat_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
#include "Render/Text/Text_Core.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_text
{
    extern const TypeInfo TextFormatTI;
    extern const ClassInfo TextFormatCI;
} // namespace fl_text
namespace fl
{
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo anyTI;
    extern const ClassInfo anyCI;
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
} // namespace fl

namespace ClassTraits { namespace fl_text
{
    class TextFormat;
}}

namespace InstanceTraits { namespace fl_text
{
    class TextFormat;
}}

namespace Classes { namespace fl_text
{
    class TextFormat;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_text
{
    class TextFormat : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_text::TextFormat;
        
    public:
        typedef TextFormat SelfType;
        typedef Classes::fl_text::TextFormat ClassType;
        typedef InstanceTraits::fl_text::TextFormat TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_text::TextFormatTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_text::TextFormat"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        TextFormat(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;
        virtual void AS3Constructor(unsigned argc, const Value* argv);
    public:
        void SetTextFormat(const Render::Text::ParagraphFormat& parafmt, const Render::Text::TextFormat&);
        void GetTextFormat(Render::Text::ParagraphFormat* parafmt, Render::Text::TextFormat* fmt);
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_alignGet, 
            mid_alignSet, 
            mid_blockIndentGet, 
            mid_blockIndentSet, 
            mid_boldGet, 
            mid_boldSet, 
            mid_bulletGet, 
            mid_bulletSet, 
            mid_colorGet, 
            mid_colorSet, 
            mid_fontGet, 
            mid_fontSet, 
            mid_indentGet, 
            mid_indentSet, 
            mid_italicGet, 
            mid_italicSet, 
            mid_kerningGet, 
            mid_kerningSet, 
            mid_leadingGet, 
            mid_leadingSet, 
            mid_leftMarginGet, 
            mid_leftMarginSet, 
            mid_letterSpacingGet, 
            mid_letterSpacingSet, 
            mid_rightMarginGet, 
            mid_rightMarginSet, 
            mid_sizeGet, 
            mid_sizeSet, 
            mid_tabStopsGet, 
            mid_tabStopsSet, 
            mid_targetGet, 
            mid_targetSet, 
            mid_underlineGet, 
            mid_underlineSet, 
            mid_urlGet, 
            mid_urlSet, 
        };
        void alignGet(ASString& result);
        void alignSet(const Value& result, const ASString& value);
        void blockIndentGet(Value& result);
        void blockIndentSet(const Value& result, const Value& value);
        void boldGet(Value& result);
        void boldSet(const Value& result, const Value& value);
        void bulletGet(Value& result);
        void bulletSet(const Value& result, const Value& value);
        void colorGet(Value& result);
        void colorSet(const Value& result, const Value& value);
        void fontGet(ASString& result);
        void fontSet(const Value& result, const ASString& value);
        void indentGet(Value& result);
        void indentSet(const Value& result, const Value& value);
        void italicGet(Value& result);
        void italicSet(const Value& result, const Value& value);
        void kerningGet(Value& result);
        void kerningSet(const Value& result, const Value& value);
        void leadingGet(Value& result);
        void leadingSet(const Value& result, const Value& value);
        void leftMarginGet(Value& result);
        void leftMarginSet(const Value& result, const Value& value);
        void letterSpacingGet(Value& result);
        void letterSpacingSet(const Value& result, const Value& value);
        void rightMarginGet(Value& result);
        void rightMarginSet(const Value& result, const Value& value);
        void sizeGet(Value& result);
        void sizeSet(const Value& result, const Value& value);
        void tabStopsGet(SPtr<Instances::fl::Array>& result);
        void tabStopsSet(const Value& result, Instances::fl::Array* value);
        void targetGet(ASString& result);
        void targetSet(const Value& result, const ASString& value);
        void underlineGet(Value& result);
        void underlineSet(const Value& result, const Value& value);
        void urlGet(ASString& result);
        void urlSet(const Value& result, const ASString& value);

        // C++ friendly wrappers for AS3 methods.
        ASString alignGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            alignGet(result);
            return result;
        }
        void alignSet(const ASString& value)
        {
            alignSet(Value::GetUndefined(), value);
        }
        Value blockIndentGet()
        {
            Value result;
            blockIndentGet(result);
            return result;
        }
        void blockIndentSet(const Value& value)
        {
            blockIndentSet(Value::GetUndefined(), value);
        }
        Value boldGet()
        {
            Value result;
            boldGet(result);
            return result;
        }
        void boldSet(const Value& value)
        {
            boldSet(Value::GetUndefined(), value);
        }
        Value bulletGet()
        {
            Value result;
            bulletGet(result);
            return result;
        }
        void bulletSet(const Value& value)
        {
            bulletSet(Value::GetUndefined(), value);
        }
        Value colorGet()
        {
            Value result;
            colorGet(result);
            return result;
        }
        void colorSet(const Value& value)
        {
            colorSet(Value::GetUndefined(), value);
        }
        ASString fontGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            fontGet(result);
            return result;
        }
        void fontSet(const ASString& value)
        {
            fontSet(Value::GetUndefined(), value);
        }
        Value indentGet()
        {
            Value result;
            indentGet(result);
            return result;
        }
        void indentSet(const Value& value)
        {
            indentSet(Value::GetUndefined(), value);
        }
        Value italicGet()
        {
            Value result;
            italicGet(result);
            return result;
        }
        void italicSet(const Value& value)
        {
            italicSet(Value::GetUndefined(), value);
        }
        Value kerningGet()
        {
            Value result;
            kerningGet(result);
            return result;
        }
        void kerningSet(const Value& value)
        {
            kerningSet(Value::GetUndefined(), value);
        }
        Value leadingGet()
        {
            Value result;
            leadingGet(result);
            return result;
        }
        void leadingSet(const Value& value)
        {
            leadingSet(Value::GetUndefined(), value);
        }
        Value leftMarginGet()
        {
            Value result;
            leftMarginGet(result);
            return result;
        }
        void leftMarginSet(const Value& value)
        {
            leftMarginSet(Value::GetUndefined(), value);
        }
        Value letterSpacingGet()
        {
            Value result;
            letterSpacingGet(result);
            return result;
        }
        void letterSpacingSet(const Value& value)
        {
            letterSpacingSet(Value::GetUndefined(), value);
        }
        Value rightMarginGet()
        {
            Value result;
            rightMarginGet(result);
            return result;
        }
        void rightMarginSet(const Value& value)
        {
            rightMarginSet(Value::GetUndefined(), value);
        }
        Value sizeGet()
        {
            Value result;
            sizeGet(result);
            return result;
        }
        void sizeSet(const Value& value)
        {
            sizeSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl::Array> tabStopsGet();
        void tabStopsSet(Instances::fl::Array* value)
        {
            tabStopsSet(Value::GetUndefined(), value);
        }
        ASString targetGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            targetGet(result);
            return result;
        }
        void targetSet(const ASString& value)
        {
            targetSet(Value::GetUndefined(), value);
        }
        Value underlineGet()
        {
            Value result;
            underlineGet(result);
            return result;
        }
        void underlineSet(const Value& value)
        {
            underlineSet(Value::GetUndefined(), value);
        }
        ASString urlGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            urlGet(result);
            return result;
        }
        void urlSet(const ASString& value)
        {
            urlSet(Value::GetUndefined(), value);
        }

//##protect##"instance$data"
        // original values
        Value                   mAlign;
        Value                   mBlockIndent;
        Value                   mBullet;
        Value                   mBold;
        Value                   mColor;
        Value                   mFont;
        Value                   mItalic;
        Value                   mIndent;
        Value                   mKerning;
        Value                   mLeading;
        Value                   mLeftMargin;
        Value                   mRightMargin;
        Value                   mLetterSpacing;
        Value                   mSize;
        SPtr<Instances::fl::Array>  mTabStops;
        Value                   mTarget;
        Value                   mUnderline;
        Value                   mUrl;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_text
{
    class TextFormat : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::TextFormat"; }
#endif
    public:
        typedef Instances::fl_text::TextFormat InstanceType;

    public:
        TextFormat(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_text::TextFormat> MakeInstance(TextFormat& t)
        {
            return Pickable<Instances::fl_text::TextFormat>(new(t.Alloc()) Instances::fl_text::TextFormat(t));
        }
        SPtr<Instances::fl_text::TextFormat> MakeInstanceS(TextFormat& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 36 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[54];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_text
{
    class TextFormat : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::TextFormat"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_text::TextFormat InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        TextFormat(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

