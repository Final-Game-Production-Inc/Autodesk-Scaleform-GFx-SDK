//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_InteractiveObject.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_InteractiveObject_H
#define INC_AS3_Obj_Display_InteractiveObject_H

#include "AS3_Obj_Display_DisplayObject.h"
//##protect##"includes"
#include "GFx/GFx_InteractiveObject.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo InteractiveObjectTI;
    extern const ClassInfo InteractiveObjectCI;
} // namespace fl_display
namespace fl
{
    extern const TypeInfo ObjectTI;
    extern const ClassInfo ObjectCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo anyTI;
    extern const ClassInfo anyCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
} // namespace fl

namespace ClassTraits { namespace fl_display
{
    class InteractiveObject;
}}

namespace InstanceTraits { namespace fl_display
{
    class InteractiveObject;
}}

namespace Classes { namespace fl_display
{
    class InteractiveObject;
}}

//##protect##"forward_declaration"
namespace Instances
{
    class NativeMenu;
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_display
{
    class InteractiveObject : public Instances::fl_display::DisplayObject
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_display::InteractiveObject;
        
    public:
        typedef InteractiveObject SelfType;
        typedef Classes::fl_display::InteractiveObject ClassType;
        typedef InstanceTraits::fl_display::InteractiveObject TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::InteractiveObjectTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_display::InteractiveObject"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        InteractiveObject(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;
    public:
        GFx::InteractiveObject* GetIntObj() 
        {
            return static_cast<GFx::InteractiveObject*>(pDispObj.GetPtr());
        }
        ~InteractiveObject();
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_contextMenuGet, 
            mid_contextMenuSet, 
            mid_doubleClickEnabledGet, 
            mid_doubleClickEnabledSet, 
            mid_focusRectGet, 
            mid_focusRectSet, 
            mid_mouseEnabledGet, 
            mid_mouseEnabledSet, 
            mid_tabEnabledGet, 
            mid_tabEnabledSet, 
            mid_tabIndexGet, 
            mid_tabIndexSet, 
        };
        void contextMenuGet(SPtr<Instances::fl::Object>& result);
        void contextMenuSet(const Value& result, const Value& value);
        void doubleClickEnabledGet(bool& result);
        void doubleClickEnabledSet(const Value& result, bool value);
        void focusRectGet(Value& result);
        void focusRectSet(const Value& result, const Value& value);
        void mouseEnabledGet(bool& result);
        void mouseEnabledSet(const Value& result, bool value);
        void tabEnabledGet(bool& result);
        void tabEnabledSet(const Value& result, bool value);
        void tabIndexGet(SInt32& result);
        void tabIndexSet(const Value& result, SInt32 value);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl::Object> contextMenuGet()
        {
            SPtr<Instances::fl::Object> result;
            contextMenuGet(result);
            return result;
        }
        void contextMenuSet(const Value& value)
        {
            contextMenuSet(Value::GetUndefined(), value);
        }
        bool doubleClickEnabledGet()
        {
            bool result;
            doubleClickEnabledGet(result);
            return result;
        }
        void doubleClickEnabledSet(bool value)
        {
            doubleClickEnabledSet(Value::GetUndefined(), value);
        }
        Value focusRectGet()
        {
            Value result;
            focusRectGet(result);
            return result;
        }
        void focusRectSet(const Value& value)
        {
            focusRectSet(Value::GetUndefined(), value);
        }
        bool mouseEnabledGet()
        {
            bool result;
            mouseEnabledGet(result);
            return result;
        }
        void mouseEnabledSet(bool value)
        {
            mouseEnabledSet(Value::GetUndefined(), value);
        }
        bool tabEnabledGet()
        {
            bool result;
            tabEnabledGet(result);
            return result;
        }
        void tabEnabledSet(bool value)
        {
            tabEnabledSet(Value::GetUndefined(), value);
        }
        SInt32 tabIndexGet()
        {
            SInt32 result;
            tabIndexGet(result);
            return result;
        }
        void tabIndexSet(SInt32 value)
        {
            tabIndexSet(Value::GetUndefined(), value);
        }

//##protect##"instance$data"
    protected:
        SPtr<Instances::fl::Object> pContextMenu;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    class InteractiveObject : public fl_display::DisplayObject
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::InteractiveObject"; }
#endif
    public:
        typedef Instances::fl_display::InteractiveObject InstanceType;

    public:
        InteractiveObject(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_display::InteractiveObject> MakeInstance(InteractiveObject& t)
        {
            return Pickable<Instances::fl_display::InteractiveObject>(new(t.Alloc()) Instances::fl_display::InteractiveObject(t));
        }
        SPtr<Instances::fl_display::InteractiveObject> MakeInstanceS(InteractiveObject& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 12 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[18];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_display
{
    class InteractiveObject : public fl_display::DisplayObject
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::InteractiveObject"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_display::InteractiveObject InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        InteractiveObject(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

