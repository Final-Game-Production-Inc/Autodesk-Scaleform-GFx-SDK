//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Gfx_Extensions.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Gfx_Extensions_H
#define INC_AS3_Obj_Gfx_Extensions_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
#include "../Text/AS3_Obj_Text_TextField.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_gfx
{
    extern const TypeInfo ExtensionsTI;
    extern const ClassInfo ExtensionsCI;
} // namespace fl_gfx
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl
namespace fl_display
{
    extern const TypeInfo DisplayObjectTI;
    extern const ClassInfo DisplayObjectCI;
} // namespace fl_display
namespace fl_geom
{
    extern const TypeInfo RectangleTI;
    extern const ClassInfo RectangleCI;
} // namespace fl_geom

namespace ClassTraits { namespace fl_gfx
{
    class Extensions;
}}

namespace InstanceTraits { namespace fl_gfx
{
    class Extensions;
}}

namespace Classes { namespace fl_gfx
{
    class Extensions;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_gfx
{
    class Extensions : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Extensions"; }
#endif
    public:
        typedef Classes::fl_gfx::Extensions ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Extensions(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 4 };
        static const MemberInfo mi[MemberInfoNum];
        enum { ThunkInfoNum = 13 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[26];
        static const Abc::ConstValue dva[3];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_gfx
{
    class Extensions : public Class
    {
        friend class ClassTraits::fl_gfx::Extensions;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_gfx::ExtensionsTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Extensions"; }
#endif
    public:
        typedef Extensions SelfType;
        typedef Extensions ClassType;
        
    private:
        Extensions(ClassTraits::Traits& t);
       
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
            mid_enabledSet, 
            mid_enabledGet, 
            mid_noInvisibleAdvanceSet, 
            mid_noInvisibleAdvanceGet, 
            mid_getTopMostEntity, 
            mid_getMouseTopMostEntity, 
            mid_setMouseCursorType, 
            mid_getMouseCursorType, 
            mid_getEdgeAAMode, 
            mid_setEdgeAAMode, 
            mid_numControllersGet, 
            mid_visibleRectGet, 
            mid_isScaleformGet, 
        };
        void enabledSet(const Value& result, bool value);
        void enabledGet(bool& result);
        void noInvisibleAdvanceSet(const Value& result, bool value);
        void noInvisibleAdvanceGet(bool& result);
        void getTopMostEntity(SPtr<Instances::fl_display::DisplayObject>& result, Value::Number x, Value::Number y, bool testAll = true);
        void getMouseTopMostEntity(SPtr<Instances::fl_display::DisplayObject>& result, bool testAll = true, UInt32 mouseIndex = 0);
        void setMouseCursorType(const Value& result, const ASString& cursor, UInt32 mouseIndex);
        void getMouseCursorType(ASString& result, UInt32 mouseIndex = 0);
        void getEdgeAAMode(UInt32& result, Instances::fl_display::DisplayObject* dispObj);
        void setEdgeAAMode(const Value& result, Instances::fl_display::DisplayObject* dispObj, UInt32 mode);
        void numControllersGet(UInt32& result);
        void visibleRectGet(SPtr<Instances::fl_geom::Rectangle>& result);
        void isScaleformGet(bool& result);

        // C++ friendly wrappers for AS3 methods.
        void enabledSet(bool value)
        {
            enabledSet(Value::GetUndefined(), value);
        }
        bool enabledGet()
        {
            bool result;
            enabledGet(result);
            return result;
        }
        void noInvisibleAdvanceSet(bool value)
        {
            noInvisibleAdvanceSet(Value::GetUndefined(), value);
        }
        bool noInvisibleAdvanceGet()
        {
            bool result;
            noInvisibleAdvanceGet(result);
            return result;
        }
        SPtr<Instances::fl_display::DisplayObject> getTopMostEntity(Value::Number x, Value::Number y, bool testAll = true);
        SPtr<Instances::fl_display::DisplayObject> getMouseTopMostEntity(bool testAll = true, UInt32 mouseIndex = 0);
        void setMouseCursorType(const ASString& cursor, UInt32 mouseIndex)
        {
            setMouseCursorType(Value::GetUndefined(), cursor, mouseIndex);
        }
        ASString getMouseCursorType(UInt32 mouseIndex = 0)
        {
            ASString result(GetStringManager().CreateEmptyString());
            getMouseCursorType(result, mouseIndex);
            return result;
        }
        UInt32 getEdgeAAMode(Instances::fl_display::DisplayObject* dispObj)
        {
            UInt32 result;
            getEdgeAAMode(result, dispObj);
            return result;
        }
        void setEdgeAAMode(Instances::fl_display::DisplayObject* dispObj, UInt32 mode)
        {
            setEdgeAAMode(Value::GetUndefined(), dispObj, mode);
        }
        UInt32 numControllersGet()
        {
            UInt32 result;
            numControllersGet(result);
            return result;
        }
        SPtr<Instances::fl_geom::Rectangle> visibleRectGet();
        bool isScaleformGet()
        {
            bool result;
            isScaleformGet(result);
            return result;
        }

    public:
        // AS3 API members.
        const UInt32 EDGEAA_INHERIT;
        const UInt32 EDGEAA_ON;
        const UInt32 EDGEAA_OFF;
        const UInt32 EDGEAA_DISABLE;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

