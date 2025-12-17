//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Text_StyleSheet.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Text_StyleSheet_H
#define INC_AS3_Obj_Text_StyleSheet_H

#include "../Events/AS3_Obj_Events_EventDispatcher.h"
//##protect##"includes"
#include "GFx/Text/Text_StyleSheet.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_text
{
    extern const TypeInfo StyleSheetTI;
    extern const ClassInfo StyleSheetCI;
    extern const TypeInfo TextFormatTI;
    extern const ClassInfo TextFormatCI;
} // namespace fl_text
namespace fl
{
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
    extern const TypeInfo ObjectTI;
    extern const ClassInfo ObjectCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_text
{
    class StyleSheet;
}}

namespace InstanceTraits { namespace fl_text
{
    class StyleSheet;
}}

namespace Classes { namespace fl_text
{
    class StyleSheet;
}}

//##protect##"forward_declaration"
namespace Instances 
{
    namespace fl_text
    {
        class TextFormat;
    }
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_text
{
    class StyleSheet : public Instances::fl_events::EventDispatcher
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_text::StyleSheet;
        
    public:
        typedef StyleSheet SelfType;
        typedef Classes::fl_text::StyleSheet ClassType;
        typedef InstanceTraits::fl_text::StyleSheet TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_text::StyleSheetTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_text::StyleSheet"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        StyleSheet(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_styleNamesGet, 
            mid_clear, 
            mid_getStyle, 
            mid_parseCSS, 
            mid_setStyle, 
            mid_transform, 
        };
        void styleNamesGet(SPtr<Instances::fl::Array>& result);
        void clear(const Value& result);
        void getStyle(SPtr<Instances::fl::Object>& result, const ASString& styleName);
        void parseCSS(const Value& result, const ASString& CSSText);
        void setStyle(const Value& result, const ASString& styleName, const Value& styleObject);
        void transform(SPtr<Instances::fl_text::TextFormat>& result, const Value& formatObject);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl::Array> styleNamesGet();
        void clear()
        {
            clear(Value::GetUndefined());
        }
        SPtr<Instances::fl::Object> getStyle(const ASString& styleName)
        {
            SPtr<Instances::fl::Object> result;
            getStyle(result, styleName);
            return result;
        }
        void parseCSS(const ASString& CSSText)
        {
            parseCSS(Value::GetUndefined(), CSSText);
        }
        void setStyle(const ASString& styleName, const Value& styleObject)
        {
            setStyle(Value::GetUndefined(), styleName, styleObject);
        }
        SPtr<Instances::fl_text::TextFormat> transform(const Value& formatObject);

//##protect##"instance$data"

#ifdef GFX_ENABLE_CSS
        Text::StyleManager* GetTextStyleManager() { return &CSS; }

    protected:
        Text::StyleManager   CSS;
#endif

//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_text
{
    class StyleSheet : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::StyleSheet"; }
#endif
    public:
        typedef Instances::fl_text::StyleSheet InstanceType;

    public:
        StyleSheet(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_text::StyleSheet> MakeInstance(StyleSheet& t)
        {
            return Pickable<Instances::fl_text::StyleSheet>(new(t.Alloc()) Instances::fl_text::StyleSheet(t));
        }
        SPtr<Instances::fl_text::StyleSheet> MakeInstanceS(StyleSheet& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 6 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[11];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_text
{
    class StyleSheet : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::StyleSheet"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_text::StyleSheet InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        StyleSheet(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

