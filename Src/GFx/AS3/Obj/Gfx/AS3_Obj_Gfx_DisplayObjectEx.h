//##protect##"disclaimer"
/**********************************************************************

Filename    :   .h
Content     :   
Created     :   May, 2012
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Gfx_DisplayObjectEx_H
#define INC_AS3_Obj_Gfx_DisplayObjectEx_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
#include "../Display/AS3_Obj_Display_DisplayObject.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_gfx
{
    extern const TypeInfo DisplayObjectExTI;
    extern const ClassInfo DisplayObjectExCI;
} // namespace fl_gfx
namespace fl_display
{
    extern const TypeInfo DisplayObjectTI;
    extern const ClassInfo DisplayObjectCI;
} // namespace fl_display
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
} // namespace fl

namespace ClassTraits { namespace fl_gfx
{
    class DisplayObjectEx;
}}

namespace InstanceTraits { namespace fl_gfx
{
    class DisplayObjectEx;
}}

namespace Classes { namespace fl_gfx
{
    class DisplayObjectEx;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_gfx
{
    class DisplayObjectEx : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::DisplayObjectEx"; }
#endif
    public:
        typedef Classes::fl_gfx::DisplayObjectEx ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        DisplayObjectEx(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { ThunkInfoNum = 8 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[20];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_gfx
{
    class DisplayObjectEx : public Class
    {
        friend class ClassTraits::fl_gfx::DisplayObjectEx;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_gfx::DisplayObjectExTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::DisplayObjectEx"; }
#endif
    public:
        typedef DisplayObjectEx SelfType;
        typedef DisplayObjectEx ClassType;
        
    private:
        DisplayObjectEx(ClassTraits::Traits& t);
       
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
            mid_disableBatching, 
            mid_isBatchingDisabled, 
            mid_setRendererString, 
            mid_getRendererString, 
            mid_setRendererFloat, 
            mid_getRendererFloat, 

			// Scaleform 4.5.32 Update
            mid_setInvertedMask,
            mid_getInvertedMask
        };
        void disableBatching(const Value& result, Instances::fl_display::DisplayObject* o, bool b);
        void isBatchingDisabled(bool& result, Instances::fl_display::DisplayObject* o);
        void setRendererString(const Value& result, Instances::fl_display::DisplayObject* o, const ASString& s);
        void getRendererString(ASString& result, Instances::fl_display::DisplayObject* o);
        void setRendererFloat(const Value& result, Instances::fl_display::DisplayObject* o, Value::Number f);
        void getRendererFloat(Value::Number& result, Instances::fl_display::DisplayObject* o);

        // Scaleform 4.5.32 Update
        void setInvertedMask(const Value& result, Instances::fl_display::DisplayObject* o, bool b);
        void getInvertedMask(bool& result, Instances::fl_display::DisplayObject* o);

        // C++ friendly wrappers for AS3 methods.
        void disableBatching(Instances::fl_display::DisplayObject* o, bool b)
        {
            disableBatching(Value::GetUndefined(), o, b);
        }
        bool isBatchingDisabled(Instances::fl_display::DisplayObject* o)
        {
            bool result;
            isBatchingDisabled(result, o);
            return result;
        }
        void setRendererString(Instances::fl_display::DisplayObject* o, const ASString& s)
        {
            setRendererString(Value::GetUndefined(), o, s);
        }
        ASString getRendererString(Instances::fl_display::DisplayObject* o)
        {
            ASString result(GetStringManager().CreateEmptyString());
            getRendererString(result, o);
            return result;
        }
        void setRendererFloat(Instances::fl_display::DisplayObject* o, Value::Number f)
        {
            setRendererFloat(Value::GetUndefined(), o, f);
        }
        Value::Number getRendererFloat(Instances::fl_display::DisplayObject* o)
        {
            Value::Number result;
            getRendererFloat(result, o);
            return result;
        }

        // 4.5.32 Adding
        void setInvertedMask(Instances::fl_display::DisplayObject* o, bool b)
        {
            disableBatching(Value::GetUndefined(), o, b);
        }
        bool getInvertedMask(Instances::fl_display::DisplayObject* o)
        {
            bool result;
            getInvertedMask(result, o);
            return result;
        }

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

