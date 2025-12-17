//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Gfx_FocusManager.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Gfx_FocusManager_H
#define INC_AS3_Obj_Gfx_FocusManager_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
#include "../Display/AS3_Obj_Display_InteractiveObject.h"
#include "../Display/AS3_Obj_Display_MovieClip.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_gfx
{
    extern const TypeInfo FocusManagerTI;
    extern const ClassInfo FocusManagerCI;
} // namespace fl_gfx
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
} // namespace fl
namespace fl_display
{
    extern const TypeInfo InteractiveObjectTI;
    extern const ClassInfo InteractiveObjectCI;
    extern const TypeInfo DisplayObjectContainerTI;
    extern const ClassInfo DisplayObjectContainerCI;
    extern const TypeInfo SpriteTI;
    extern const ClassInfo SpriteCI;
} // namespace fl_display

namespace ClassTraits { namespace fl_gfx
{
    class FocusManager;
}}

namespace InstanceTraits { namespace fl_gfx
{
    class FocusManager;
}}

namespace Classes { namespace fl_gfx
{
    class FocusManager;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_gfx
{
    class FocusManager : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::FocusManager"; }
#endif
    public:
        typedef Classes::fl_gfx::FocusManager ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        FocusManager(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { ThunkInfoNum = 16 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[41];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_gfx
{
    class FocusManager : public Class
    {
        friend class ClassTraits::fl_gfx::FocusManager;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_gfx::FocusManagerTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::FocusManager"; }
#endif
    public:
        typedef FocusManager SelfType;
        typedef FocusManager ClassType;
        
    private:
        FocusManager(ClassTraits::Traits& t);
       
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
            mid_alwaysEnableArrowKeysSet, 
            mid_alwaysEnableArrowKeysGet, 
            mid_disableFocusKeysSet, 
            mid_disableFocusKeysGet, 
            mid_moveFocus, 
            mid_findFocus, 
            mid_setFocus, 
            mid_getFocus, 
            mid_numFocusGroupsGet, 
            mid_setFocusGroupMask, 
            mid_getFocusGroupMask, 
            mid_setControllerFocusGroup, 
            mid_getControllerFocusGroup, 
            mid_getControllerMaskByFocusGroup, 
            mid_getModalClip, 
            mid_setModalClip, 
        };
        void alwaysEnableArrowKeysSet(const Value& result, bool enable);
        void alwaysEnableArrowKeysGet(bool& result);
        void disableFocusKeysSet(const Value& result, bool disable);
        void disableFocusKeysGet(bool& result);
        void moveFocus(SPtr<Instances::fl_display::InteractiveObject>& result, const ASString& keyToSimulate, Instances::fl_display::InteractiveObject* startFromMovie, bool includeFocusEnabledChars, UInt32 controllerIdx);
        void findFocus(SPtr<Instances::fl_display::InteractiveObject>& result, const ASString& keyToSimulate, Instances::fl_display::DisplayObjectContainer* parentMovie, bool loop, Instances::fl_display::InteractiveObject* startFromMovie, bool includeFocusEnabledChars, UInt32 controllerIdx);
        void setFocus(const Value& result, Instances::fl_display::InteractiveObject* obj, UInt32 controllerIdx = 0);
        void getFocus(SPtr<Instances::fl_display::InteractiveObject>& result, UInt32 controllerIdx = 0);
        void numFocusGroupsGet(UInt32& result);
        void setFocusGroupMask(const Value& result, Instances::fl_display::InteractiveObject* obj, UInt32 mask);
        void getFocusGroupMask(UInt32& result, Instances::fl_display::InteractiveObject* obj);
        void setControllerFocusGroup(bool& result, UInt32 controllerIdx, UInt32 focusGroupIdx);
        void getControllerFocusGroup(UInt32& result, UInt32 controllerIdx);
        void getControllerMaskByFocusGroup(UInt32& result, UInt32 focusGroupIdx);
        void getModalClip(SPtr<Instances::fl_display::Sprite>& result, UInt32 controllerIdx = 0);
        void setModalClip(const Value& result, Instances::fl_display::Sprite* mc, UInt32 controllerIdx = 0);

        // C++ friendly wrappers for AS3 methods.
        void alwaysEnableArrowKeysSet(bool enable)
        {
            alwaysEnableArrowKeysSet(Value::GetUndefined(), enable);
        }
        bool alwaysEnableArrowKeysGet()
        {
            bool result;
            alwaysEnableArrowKeysGet(result);
            return result;
        }
        void disableFocusKeysSet(bool disable)
        {
            disableFocusKeysSet(Value::GetUndefined(), disable);
        }
        bool disableFocusKeysGet()
        {
            bool result;
            disableFocusKeysGet(result);
            return result;
        }
        SPtr<Instances::fl_display::InteractiveObject> moveFocus(const ASString& keyToSimulate, Instances::fl_display::InteractiveObject* startFromMovie, bool includeFocusEnabledChars, UInt32 controllerIdx);
        SPtr<Instances::fl_display::InteractiveObject> findFocus(const ASString& keyToSimulate, Instances::fl_display::DisplayObjectContainer* parentMovie, bool loop, Instances::fl_display::InteractiveObject* startFromMovie, bool includeFocusEnabledChars, UInt32 controllerIdx);
        void setFocus(Instances::fl_display::InteractiveObject* obj, UInt32 controllerIdx = 0)
        {
            setFocus(Value::GetUndefined(), obj, controllerIdx);
        }
        SPtr<Instances::fl_display::InteractiveObject> getFocus(UInt32 controllerIdx = 0);
        UInt32 numFocusGroupsGet()
        {
            UInt32 result;
            numFocusGroupsGet(result);
            return result;
        }
        void setFocusGroupMask(Instances::fl_display::InteractiveObject* obj, UInt32 mask)
        {
            setFocusGroupMask(Value::GetUndefined(), obj, mask);
        }
        UInt32 getFocusGroupMask(Instances::fl_display::InteractiveObject* obj)
        {
            UInt32 result;
            getFocusGroupMask(result, obj);
            return result;
        }
        bool setControllerFocusGroup(UInt32 controllerIdx, UInt32 focusGroupIdx)
        {
            bool result;
            setControllerFocusGroup(result, controllerIdx, focusGroupIdx);
            return result;
        }
        UInt32 getControllerFocusGroup(UInt32 controllerIdx)
        {
            UInt32 result;
            getControllerFocusGroup(result, controllerIdx);
            return result;
        }
        UInt32 getControllerMaskByFocusGroup(UInt32 focusGroupIdx)
        {
            UInt32 result;
            getControllerMaskByFocusGroup(result, focusGroupIdx);
            return result;
        }
        SPtr<Instances::fl_display::Sprite> getModalClip(UInt32 controllerIdx = 0);
        void setModalClip(Instances::fl_display::Sprite* mc, UInt32 controllerIdx = 0)
        {
            setModalClip(Value::GetUndefined(), mc, controllerIdx);
        }

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

