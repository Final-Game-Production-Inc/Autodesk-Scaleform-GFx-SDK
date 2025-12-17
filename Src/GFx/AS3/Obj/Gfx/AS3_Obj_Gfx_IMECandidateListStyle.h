//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Gfx_IMECandidateListStyle.h
Content     :   
Created     :   Jan, 2011
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Gfx_IMECandidateListStyle_H
#define INC_AS3_Obj_Gfx_IMECandidateListStyle_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_gfx
{
    extern const TypeInfo IMECandidateListStyleTI;
    extern const ClassInfo IMECandidateListStyleCI;
} // namespace fl_gfx

namespace ClassTraits { namespace fl_gfx
{
    class IMECandidateListStyle;
}}

namespace InstanceTraits { namespace fl_gfx
{
    class IMECandidateListStyle;
}}

namespace Classes { namespace fl_gfx
{
    class IMECandidateListStyle;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_gfx
{
    class IMECandidateListStyle : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_gfx::IMECandidateListStyle;
        
    public:
        typedef IMECandidateListStyle SelfType;
        typedef Classes::fl_gfx::IMECandidateListStyle ClassType;
        typedef InstanceTraits::fl_gfx::IMECandidateListStyle TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_gfx::IMECandidateListStyleTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_gfx::IMECandidateListStyle"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        IMECandidateListStyle(InstanceTraits::Traits& t);

//##protect##"instance$methods"
	public:
		inline void set_textColor(SInt32 _textColor) {textColor = _textColor; }
		inline void set_selectedTextColor(SInt32 _selectedTextColor) {selectedTextColor = _selectedTextColor; }
		inline void set_fontSize(SInt32 _fontSize) { fontSize = _fontSize; }
		inline void set_backgroundColor(SInt32 _bcColor) { backgroundColor = _bcColor; }
		inline void set_selectedBackgroundColor(SInt32 _bcColor) { selectedBackgroundColor = _bcColor; }
		inline void set_indexBackgroundColor(SInt32 _indexBkColor) { indexBackgroundColor = _indexBkColor; }
		inline void set_selectedIndexBackgroundColor(SInt32 _selectedIndexBkColor) { selectedIndexBackgroundColor = _selectedIndexBkColor; }
		inline void set_readingWindowTextColor(SInt32 _readingWindowTxtColor) { readingWindowTextColor = _readingWindowTxtColor; }
		inline void set_readinwWindowBackgroundColor(SInt32 _readingWindowBkColor) { readingWindowBackgroundColor = _readingWindowBkColor; }
		inline void set_readingWindowFontSize(SInt32 _readingWindowFontSize) { readingWindowFontSize = _readingWindowFontSize; }

//##protect##"instance$methods"

    public:
        // AS3 API members.
        SInt32 textColor;
        SInt32 selectedTextColor;
        SInt32 fontSize;
        SInt32 backgroundColor;
        SInt32 selectedBackgroundColor;
        SInt32 indexBackgroundColor;
        SInt32 selectedIndexBackgroundColor;
        SInt32 readingWindowTextColor;
        SInt32 readingWindowBackgroundColor;
        SInt32 readingWindowFontSize;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_gfx
{
    class IMECandidateListStyle : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::IMECandidateListStyle"; }
#endif
    public:
        typedef Instances::fl_gfx::IMECandidateListStyle InstanceType;

    public:
        IMECandidateListStyle(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_gfx::IMECandidateListStyle> MakeInstance(IMECandidateListStyle& t)
        {
            return Pickable<Instances::fl_gfx::IMECandidateListStyle>(new(t.Alloc()) Instances::fl_gfx::IMECandidateListStyle(t));
        }
        SPtr<Instances::fl_gfx::IMECandidateListStyle> MakeInstanceS(IMECandidateListStyle& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 10 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_gfx
{
    class IMECandidateListStyle : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::IMECandidateListStyle"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_gfx::IMECandidateListStyle InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        IMECandidateListStyle(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

