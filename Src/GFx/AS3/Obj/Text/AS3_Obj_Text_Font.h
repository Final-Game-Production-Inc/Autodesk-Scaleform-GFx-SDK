//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Text_Font.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Text_Font_H
#define INC_AS3_Obj_Text_Font_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
#include "GFx/GFx_FontResource.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_text
{
    extern const TypeInfo FontTI;
    extern const ClassInfo FontCI;
} // namespace fl_text
namespace fl
{
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo ClassTI;
    extern const ClassInfo ClassCI;
} // namespace fl

namespace ClassTraits { namespace fl_text
{
    class Font;
}}

namespace InstanceTraits { namespace fl_text
{
    class Font;
}}

namespace Classes { namespace fl_text
{
    class Font;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_text
{
    class Font : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_text::Font;
        
    public:
        typedef Font SelfType;
        typedef Classes::fl_text::Font ClassType;
        typedef InstanceTraits::fl_text::Font TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_text::FontTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_text::Font"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Font(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        friend class Classes::fl_text::Font;

        virtual void InitInstance(bool extCall);
        virtual bool CreateLibraryObject();
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_fontNameGet, 
            mid_fontStyleGet, 
            mid_fontTypeGet, 
            mid_hasGlyphs, 
        };
        void fontNameGet(Value& result);
        void fontStyleGet(Value& result);
        void fontTypeGet(Value& result);
        void hasGlyphs(bool& result, const ASString& str);

        // C++ friendly wrappers for AS3 methods.
        Value fontNameGet()
        {
            Value result;
            fontNameGet(result);
            return result;
        }
        Value fontStyleGet()
        {
            Value result;
            fontStyleGet(result);
            return result;
        }
        Value fontTypeGet()
        {
            Value result;
            fontTypeGet(result);
            return result;
        }
        bool hasGlyphs(const ASString& str)
        {
            bool result;
            hasGlyphs(result, str);
            return result;
        }

//##protect##"instance$data"
    protected:
        Ptr<Render::Font>       pFont;

        // the following members will be used only if pFont is NULL
        ASString fontName, fontStyle, fontType;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_text
{
    class Font : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Font"; }
#endif
    public:
        typedef Instances::fl_text::Font InstanceType;

    public:
        Font(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_text::Font> MakeInstance(Font& t)
        {
            return Pickable<Instances::fl_text::Font>(new(t.Alloc()) Instances::fl_text::Font(t));
        }
        SPtr<Instances::fl_text::Font> MakeInstanceS(Font& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 4 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[5];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_text
{
    class Font : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Font"; }
#endif
    public:
        typedef Classes::fl_text::Font ClassType;
        typedef InstanceTraits::fl_text::Font InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Font(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { ThunkInfoNum = 2 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[4];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_text
{
    class Font : public Class
    {
        friend class ClassTraits::fl_text::Font;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_text::FontTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Font"; }
#endif
    public:
        typedef Font SelfType;
        typedef Font ClassType;
        
    private:
        Font(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_enumerateFonts, 
            mid_registerFont, 
        };
        void enumerateFonts(SPtr<Instances::fl::Array>& result, bool enumerateDeviceFonts = false);
        void registerFont(const Value& result, Class* font);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl::Array> enumerateFonts(bool enumerateDeviceFonts = false);
        void registerFont(Class* font)
        {
            registerFont(Value::GetUndefined(), font);
        }

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

