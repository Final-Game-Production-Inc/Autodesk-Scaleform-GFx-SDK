//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_DisplayObjectContainer.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_DisplayObjectContainer_H
#define INC_AS3_Obj_Display_DisplayObjectContainer_H

#include "AS3_Obj_Display_InteractiveObject.h"
//##protect##"includes"
#include "GFx/GFx_DisplayObjContainer.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo DisplayObjectContainerTI;
    extern const ClassInfo DisplayObjectContainerCI;
    extern const TypeInfo DisplayObjectTI;
    extern const ClassInfo DisplayObjectCI;
} // namespace fl_display
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
} // namespace fl
namespace fl_text
{
    extern const TypeInfo TextSnapshotTI;
    extern const ClassInfo TextSnapshotCI;
} // namespace fl_text
namespace fl_geom
{
    extern const TypeInfo PointTI;
    extern const ClassInfo PointCI;
} // namespace fl_geom

namespace ClassTraits { namespace fl_display
{
    class DisplayObjectContainer;
}}

namespace InstanceTraits { namespace fl_display
{
    class DisplayObjectContainer;
}}

namespace Classes { namespace fl_display
{
    class DisplayObjectContainer;
}}

//##protect##"forward_declaration"
namespace Instances { namespace fl_text
{
    class TextSnapshot;
}}
//##protect##"forward_declaration"

namespace Instances { namespace fl_display
{
    class DisplayObjectContainer : public Instances::fl_display::InteractiveObject
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_display::DisplayObjectContainer;
        
    public:
        typedef DisplayObjectContainer SelfType;
        typedef Classes::fl_display::DisplayObjectContainer ClassType;
        typedef InstanceTraits::fl_display::DisplayObjectContainer TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::DisplayObjectContainerTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_display::DisplayObjectContainer"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        DisplayObjectContainer(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        ~DisplayObjectContainer();
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_mouseChildrenGet, 
            mid_mouseChildrenSet, 
            mid_numChildrenGet, 
            mid_tabChildrenGet, 
            mid_tabChildrenSet, 
            mid_textSnapshotGet, 
            mid_addChild, 
            mid_addChildAt, 
            mid_areInaccessibleObjectsUnderPoint, 
            mid_contains, 
            mid_getChildAt, 
            mid_getChildByName, 
            mid_getChildIndex, 
            mid_getObjectsUnderPoint, 
            mid_removeChild, 
            mid_removeChildAt, 
            mid_removeChildren, 
            mid_setChildIndex, 
            mid_swapChildren, 
            mid_swapChildrenAt, 
        };
        void mouseChildrenGet(bool& result);
        void mouseChildrenSet(const Value& result, bool value);
        void numChildrenGet(SInt32& result);
        void tabChildrenGet(bool& result);
        void tabChildrenSet(const Value& result, bool value);
        void textSnapshotGet(SPtr<Instances::fl_text::TextSnapshot>& result);
        void addChild(SPtr<Instances::fl_display::DisplayObject>& result, Instances::fl_display::DisplayObject* child);
        void addChildAt(SPtr<Instances::fl_display::DisplayObject>& result, Instances::fl_display::DisplayObject* child, SInt32 index);
        void areInaccessibleObjectsUnderPoint(bool& result, Instances::fl_geom::Point* point);
        void contains(bool& result, Instances::fl_display::DisplayObject* child);
        void getChildAt(SPtr<Instances::fl_display::DisplayObject>& result, SInt32 index);
        void getChildByName(SPtr<Instances::fl_display::DisplayObject>& result, const ASString& name);
        void getChildIndex(SInt32& result, Instances::fl_display::DisplayObject* child);
        void getObjectsUnderPoint(SPtr<Instances::fl::Array>& result, Instances::fl_geom::Point* point);
        void removeChild(Value& result, unsigned argc, const Value* const argv);
        void removeChildAt(SPtr<Instances::fl_display::DisplayObject>& result, SInt32 index);
        void removeChildren(const Value& result, SInt32 beginIndex = 0, SInt32 endIndex = 0x7fffffff);
        void setChildIndex(const Value& result, Instances::fl_display::DisplayObject* child, SInt32 index);
        void swapChildren(const Value& result, Instances::fl_display::DisplayObject* child1, Instances::fl_display::DisplayObject* child2);
        void swapChildrenAt(const Value& result, SInt32 index1, SInt32 index2);

        // C++ friendly wrappers for AS3 methods.
        bool mouseChildrenGet()
        {
            bool result;
            mouseChildrenGet(result);
            return result;
        }
        void mouseChildrenSet(bool value)
        {
            mouseChildrenSet(Value::GetUndefined(), value);
        }
        SInt32 numChildrenGet()
        {
            SInt32 result;
            numChildrenGet(result);
            return result;
        }
        bool tabChildrenGet()
        {
            bool result;
            tabChildrenGet(result);
            return result;
        }
        void tabChildrenSet(bool value)
        {
            tabChildrenSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_text::TextSnapshot> textSnapshotGet();
        SPtr<Instances::fl_display::DisplayObject> addChild(Instances::fl_display::DisplayObject* child);
        SPtr<Instances::fl_display::DisplayObject> addChildAt(Instances::fl_display::DisplayObject* child, SInt32 index);
        bool areInaccessibleObjectsUnderPoint(Instances::fl_geom::Point* point)
        {
            bool result;
            areInaccessibleObjectsUnderPoint(result, point);
            return result;
        }
        bool contains(Instances::fl_display::DisplayObject* child)
        {
            bool result;
            contains(result, child);
            return result;
        }
        SPtr<Instances::fl_display::DisplayObject> getChildAt(SInt32 index);
        SPtr<Instances::fl_display::DisplayObject> getChildByName(const ASString& name);
        SInt32 getChildIndex(Instances::fl_display::DisplayObject* child)
        {
            SInt32 result;
            getChildIndex(result, child);
            return result;
        }
        SPtr<Instances::fl::Array> getObjectsUnderPoint(Instances::fl_geom::Point* point);
        SPtr<Instances::fl_display::DisplayObject> removeChildAt(SInt32 index);
        void removeChildren(SInt32 beginIndex = 0, SInt32 endIndex = 0x7fffffff)
        {
            removeChildren(Value::GetUndefined(), beginIndex, endIndex);
        }
        void setChildIndex(Instances::fl_display::DisplayObject* child, SInt32 index)
        {
            setChildIndex(Value::GetUndefined(), child, index);
        }
        void swapChildren(Instances::fl_display::DisplayObject* child1, Instances::fl_display::DisplayObject* child2)
        {
            swapChildren(Value::GetUndefined(), child1, child2);
        }
        void swapChildrenAt(SInt32 index1, SInt32 index2)
        {
            swapChildrenAt(Value::GetUndefined(), index1, index2);
        }

//##protect##"instance$data"
        GFx::DisplayObjContainer* GetDisplayObjContainer() const 
        { 
            SF_ASSERT(pDispObj && pDispObj->IsDisplayObjContainer());
            return pDispObj->CharToDisplayObjContainer_Unsafe(); 
        }
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    class DisplayObjectContainer : public fl_display::InteractiveObject
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::DisplayObjectContainer"; }
#endif
    public:
        typedef Instances::fl_display::DisplayObjectContainer InstanceType;

    public:
        DisplayObjectContainer(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_display::DisplayObjectContainer> MakeInstance(DisplayObjectContainer& t)
        {
            return Pickable<Instances::fl_display::DisplayObjectContainer>(new(t.Alloc()) Instances::fl_display::DisplayObjectContainer(t));
        }
        SPtr<Instances::fl_display::DisplayObjectContainer> MakeInstanceS(DisplayObjectContainer& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 20 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[40];
        static const Abc::ConstValue dva[1];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_display
{
    class DisplayObjectContainer : public fl_display::InteractiveObject
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::DisplayObjectContainer"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_display::DisplayObjectContainer InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        DisplayObjectContainer(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

