//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Text_TextSnapshot.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Text_TextSnapshot_H
#define INC_AS3_Obj_Text_TextSnapshot_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
#include "GFx/GFx_Sprite.h"
#include "GFx/GFx_StaticText.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_text
{
    extern const TypeInfo TextSnapshotTI;
    extern const ClassInfo TextSnapshotCI;
} // namespace fl_text
namespace fl
{
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
} // namespace fl

namespace ClassTraits { namespace fl_text
{
    class TextSnapshot;
}}

namespace InstanceTraits { namespace fl_text
{
    class TextSnapshot;
}}

namespace Classes { namespace fl_text
{
    class TextSnapshot;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_text
{
    class TextSnapshot : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_text::TextSnapshot;
        
    public:
        typedef TextSnapshot SelfType;
        typedef Classes::fl_text::TextSnapshot ClassType;
        typedef InstanceTraits::fl_text::TextSnapshot TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_text::TextSnapshotTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_text::TextSnapshot"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        TextSnapshot(InstanceTraits::Traits& t);

//##protect##"instance$methods"
    public:
#ifdef GFX_AS_ENABLE_TEXTSNAPSHOT
        void                    ProcessSnapshot(GFx::Sprite* sprite) { sprite->GetTextSnapshot(&SnapshotData); }
        StaticTextSnapshotData& GetSnapshotData() { return SnapshotData; }
#endif // GFX_AS_ENABLE_TEXTSNAPSHOT
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_charCountGet, 
            mid_findText, 
            mid_getSelected, 
            mid_getSelectedText, 
            mid_getText, 
            mid_getTextRunInfo, 
            mid_hitTestTextNearPos, 
            mid_setSelectColor, 
            mid_setSelected, 
        };
        void charCountGet(SInt32& result);
        void findText(SInt32& result, SInt32 beginIndex, const ASString& textToFind, bool caseSensitive);
        void getSelected(bool& result, SInt32 beginIndex, SInt32 endIndex);
        void getSelectedText(ASString& result, bool includeLineEndings = false);
        void getText(ASString& result, SInt32 beginIndex, SInt32 endIndex, bool includeLineEndings = false);
        void getTextRunInfo(SPtr<Instances::fl::Array>& result, SInt32 beginIndex, SInt32 endIndex);
        void hitTestTextNearPos(Value::Number& result, Value::Number x, Value::Number y, Value::Number maxDistance = 0.0);
        void setSelectColor(const Value& result, UInt32 hexColor = 0xFFFF00);
        void setSelected(const Value& result, SInt32 beginIndex, SInt32 endIndex, bool select);

        // C++ friendly wrappers for AS3 methods.
        SInt32 charCountGet()
        {
            SInt32 result;
            charCountGet(result);
            return result;
        }
        SInt32 findText(SInt32 beginIndex, const ASString& textToFind, bool caseSensitive)
        {
            SInt32 result;
            findText(result, beginIndex, textToFind, caseSensitive);
            return result;
        }
        bool getSelected(SInt32 beginIndex, SInt32 endIndex)
        {
            bool result;
            getSelected(result, beginIndex, endIndex);
            return result;
        }
        ASString getSelectedText(bool includeLineEndings = false)
        {
            ASString result(GetStringManager().CreateEmptyString());
            getSelectedText(result, includeLineEndings);
            return result;
        }
        ASString getText(SInt32 beginIndex, SInt32 endIndex, bool includeLineEndings = false)
        {
            ASString result(GetStringManager().CreateEmptyString());
            getText(result, beginIndex, endIndex, includeLineEndings);
            return result;
        }
        SPtr<Instances::fl::Array> getTextRunInfo(SInt32 beginIndex, SInt32 endIndex);
        Value::Number hitTestTextNearPos(Value::Number x, Value::Number y, Value::Number maxDistance = 0.0)
        {
            Value::Number result;
            hitTestTextNearPos(result, x, y, maxDistance);
            return result;
        }
        void setSelectColor(UInt32 hexColor = 0xFFFF00)
        {
            setSelectColor(Value::GetUndefined(), hexColor);
        }
        void setSelected(SInt32 beginIndex, SInt32 endIndex, bool select)
        {
            setSelected(Value::GetUndefined(), beginIndex, endIndex, select);
        }

//##protect##"instance$data"
    protected:
#ifdef GFX_AS_ENABLE_TEXTSNAPSHOT
        StaticTextSnapshotData SnapshotData;
#endif // GFX_AS_ENABLE_TEXTSNAPSHOT
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_text
{
    class TextSnapshot : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::TextSnapshot"; }
#endif
    public:
        typedef Instances::fl_text::TextSnapshot InstanceType;

    public:
        TextSnapshot(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_text::TextSnapshot> MakeInstance(TextSnapshot& t)
        {
            return Pickable<Instances::fl_text::TextSnapshot>(new(t.Alloc()) Instances::fl_text::TextSnapshot(t));
        }
        SPtr<Instances::fl_text::TextSnapshot> MakeInstanceS(TextSnapshot& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 9 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[27];
        static const Abc::ConstValue dva[2];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_text
{
    class TextSnapshot : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::TextSnapshot"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_text::TextSnapshot InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        TextSnapshot(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

