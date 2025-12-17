//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_SimpleButton.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_SimpleButton_H
#define INC_AS3_Obj_Display_SimpleButton_H

#include "AS3_Obj_Display_InteractiveObject.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo SimpleButtonTI;
    extern const ClassInfo SimpleButtonCI;
    extern const TypeInfo DisplayObjectTI;
    extern const ClassInfo DisplayObjectCI;
} // namespace fl_display
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
} // namespace fl
namespace fl_media
{
    extern const TypeInfo SoundTransformTI;
    extern const ClassInfo SoundTransformCI;
} // namespace fl_media

namespace ClassTraits { namespace fl_display
{
    class SimpleButton;
}}

namespace InstanceTraits { namespace fl_display
{
    class SimpleButton;
}}

namespace Classes { namespace fl_display
{
    class SimpleButton;
}}

//##protect##"forward_declaration"
class AvmButton;

namespace Instances
{
    namespace fl_media
    {
        class SoundTransform;
    }
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_display
{
    class SimpleButton : public Instances::fl_display::InteractiveObject
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_display::SimpleButton;
        
    public:
        typedef SimpleButton SelfType;
        typedef Classes::fl_display::SimpleButton ClassType;
        typedef InstanceTraits::fl_display::SimpleButton TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::SimpleButtonTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_display::SimpleButton"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        SimpleButton(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        AvmButton*                  GetAvmButton() const;
        virtual void                AS3Constructor(unsigned argc, const Value* argv);
        virtual void                InitInstance(bool extCall = false);
        virtual GFx::DisplayObject* CreateStageObject();
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_downStateGet, 
            mid_downStateSet, 
            mid_enabledGet, 
            mid_enabledSet, 
            mid_hitTestStateGet, 
            mid_hitTestStateSet, 
            mid_overStateGet, 
            mid_overStateSet, 
            mid_soundTransformGet, 
            mid_soundTransformSet, 
            mid_trackAsMenuGet, 
            mid_trackAsMenuSet, 
            mid_upStateGet, 
            mid_upStateSet, 
            mid_useHandCursorGet, 
            mid_useHandCursorSet, 
        };
        void downStateGet(SPtr<Instances::fl_display::DisplayObject>& result);
        void downStateSet(const Value& result, Instances::fl_display::DisplayObject* value);
        void enabledGet(bool& result);
        void enabledSet(const Value& result, bool value);
        void hitTestStateGet(SPtr<Instances::fl_display::DisplayObject>& result);
        void hitTestStateSet(const Value& result, Instances::fl_display::DisplayObject* value);
        void overStateGet(SPtr<Instances::fl_display::DisplayObject>& result);
        void overStateSet(const Value& result, Instances::fl_display::DisplayObject* value);
        void soundTransformGet(SPtr<Instances::fl_media::SoundTransform>& result);
        void soundTransformSet(const Value& result, Instances::fl_media::SoundTransform* value);
        void trackAsMenuGet(bool& result);
        void trackAsMenuSet(const Value& result, bool value);
        void upStateGet(SPtr<Instances::fl_display::DisplayObject>& result);
        void upStateSet(const Value& result, Instances::fl_display::DisplayObject* value);
        void useHandCursorGet(bool& result);
        void useHandCursorSet(const Value& result, bool value);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl_display::DisplayObject> downStateGet();
        void downStateSet(Instances::fl_display::DisplayObject* value)
        {
            downStateSet(Value::GetUndefined(), value);
        }
        bool enabledGet()
        {
            bool result;
            enabledGet(result);
            return result;
        }
        void enabledSet(bool value)
        {
            enabledSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_display::DisplayObject> hitTestStateGet();
        void hitTestStateSet(Instances::fl_display::DisplayObject* value)
        {
            hitTestStateSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_display::DisplayObject> overStateGet();
        void overStateSet(Instances::fl_display::DisplayObject* value)
        {
            overStateSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_media::SoundTransform> soundTransformGet();
        void soundTransformSet(Instances::fl_media::SoundTransform* value)
        {
            soundTransformSet(Value::GetUndefined(), value);
        }
        bool trackAsMenuGet()
        {
            bool result;
            trackAsMenuGet(result);
            return result;
        }
        void trackAsMenuSet(bool value)
        {
            trackAsMenuSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_display::DisplayObject> upStateGet();
        void upStateSet(Instances::fl_display::DisplayObject* value)
        {
            upStateSet(Value::GetUndefined(), value);
        }
        bool useHandCursorGet()
        {
            bool result;
            useHandCursorGet(result);
            return result;
        }
        void useHandCursorSet(bool value)
        {
            useHandCursorSet(Value::GetUndefined(), value);
        }

//##protect##"instance$data"
        SPtr<DisplayObject> OverStateObject;
        SPtr<DisplayObject> UpStateObject;
        SPtr<DisplayObject> DownStateObject;
        SPtr<DisplayObject> HitTestStateObject;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    class SimpleButton : public fl_display::InteractiveObject
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::SimpleButton"; }
#endif
    public:
        typedef Instances::fl_display::SimpleButton InstanceType;

    public:
        SimpleButton(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_display::SimpleButton> MakeInstance(SimpleButton& t)
        {
            return Pickable<Instances::fl_display::SimpleButton>(new(t.Alloc()) Instances::fl_display::SimpleButton(t));
        }
        SPtr<Instances::fl_display::SimpleButton> MakeInstanceS(SimpleButton& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 16 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[24];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_display
{
    class SimpleButton : public fl_display::InteractiveObject
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::SimpleButton"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_display::SimpleButton InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        SimpleButton(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

