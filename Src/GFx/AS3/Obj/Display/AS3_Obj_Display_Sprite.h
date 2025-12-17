//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_Sprite.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_Sprite_H
#define INC_AS3_Obj_Display_Sprite_H

#include "AS3_Obj_Display_DisplayObjectContainer.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo SpriteTI;
    extern const ClassInfo SpriteCI;
    extern const TypeInfo DisplayObjectTI;
    extern const ClassInfo DisplayObjectCI;
    extern const TypeInfo GraphicsTI;
    extern const ClassInfo GraphicsCI;
} // namespace fl_display
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
} // namespace fl
namespace fl_media
{
    extern const TypeInfo SoundTransformTI;
    extern const ClassInfo SoundTransformCI;
} // namespace fl_media
namespace fl_geom
{
    extern const TypeInfo RectangleTI;
    extern const ClassInfo RectangleCI;
} // namespace fl_geom

namespace ClassTraits { namespace fl_display
{
    class Sprite;
}}

namespace InstanceTraits { namespace fl_display
{
    class Sprite;
}}

namespace Classes { namespace fl_display
{
    class Sprite;
}}

//##protect##"forward_declaration"
namespace Instances
{
    namespace fl_display
    {
        class Graphics;
    }
    namespace fl_media
    {
        class SoundTransform;
    }
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_display
{
    class Sprite : public Instances::fl_display::DisplayObjectContainer
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_display::Sprite;
        
    public:
        typedef Sprite SelfType;
        typedef Classes::fl_display::Sprite ClassType;
        typedef InstanceTraits::fl_display::Sprite TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::SpriteTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_display::Sprite"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Sprite(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        ~Sprite();

        virtual void InitInstance(bool extCall);
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;
        virtual GFx::DisplayObject* CreateStageObject();

        virtual void        AS3Constructor(unsigned argc, const Value* argv);
    public:
        virtual bool        HasFrameScript() const { return false; }
        virtual bool        GetFrameScript(unsigned, Value*) { return false; }
        virtual bool        IsMovieClip() const { return false; }
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_buttonModeGet, 
            mid_buttonModeSet, 
            mid_dropTargetGet, 
            mid_graphicsGet, 
            mid_hitAreaGet, 
            mid_hitAreaSet, 
            mid_soundTransformGet, 
            mid_soundTransformSet, 
            mid_useHandCursorGet, 
            mid_useHandCursorSet, 
            mid_startDrag, 
            mid_stopDrag, 
            mid_startTouchDrag, 
            mid_stopTouchDrag, 
        };
        void buttonModeGet(bool& result);
        void buttonModeSet(const Value& result, bool value);
        void dropTargetGet(SPtr<Instances::fl_display::DisplayObject>& result);
        void graphicsGet(SPtr<Instances::fl_display::Graphics>& result);
        void hitAreaGet(SPtr<Instances::fl_display::Sprite>& result);
        void hitAreaSet(const Value& result, Instances::fl_display::Sprite* value);
        void soundTransformGet(SPtr<Instances::fl_media::SoundTransform>& result);
        void soundTransformSet(const Value& result, Instances::fl_media::SoundTransform* value);
        void useHandCursorGet(bool& result);
        void useHandCursorSet(const Value& result, bool value);
        void startDrag(const Value& result, bool lockCenter = false, Instances::fl_geom::Rectangle* bounds = NULL);
        void stopDrag(const Value& result);
        void startTouchDrag(const Value& result, SInt32 touchPointID, bool lockCenter = false, Instances::fl_geom::Rectangle* bounds = NULL);
        void stopTouchDrag(const Value& result, SInt32 touchPointID);

        // C++ friendly wrappers for AS3 methods.
        bool buttonModeGet()
        {
            bool result;
            buttonModeGet(result);
            return result;
        }
        void buttonModeSet(bool value)
        {
            buttonModeSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_display::DisplayObject> dropTargetGet();
        SPtr<Instances::fl_display::Graphics> graphicsGet();
        SPtr<Instances::fl_display::Sprite> hitAreaGet()
        {
            SPtr<Instances::fl_display::Sprite> result;
            hitAreaGet(result);
            return result;
        }
        void hitAreaSet(Instances::fl_display::Sprite* value)
        {
            hitAreaSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_media::SoundTransform> soundTransformGet();
        void soundTransformSet(Instances::fl_media::SoundTransform* value)
        {
            soundTransformSet(Value::GetUndefined(), value);
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
        void startDrag(bool lockCenter = false, Instances::fl_geom::Rectangle* bounds = NULL)
        {
            startDrag(Value::GetUndefined(), lockCenter, bounds);
        }
        void stopDrag()
        {
            stopDrag(Value::GetUndefined());
        }
        void startTouchDrag(SInt32 touchPointID, bool lockCenter = false, Instances::fl_geom::Rectangle* bounds = NULL)
        {
            startTouchDrag(Value::GetUndefined(), touchPointID, lockCenter, bounds);
        }
        void stopTouchDrag(SInt32 touchPointID)
        {
            stopTouchDrag(Value::GetUndefined(), touchPointID);
        }

//##protect##"instance$data"
        SPtr<Graphics> pGraphics;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    class Sprite : public fl_display::DisplayObjectContainer
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Sprite"; }
#endif
    public:
        typedef Instances::fl_display::Sprite InstanceType;

    public:
        Sprite(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_display::Sprite> MakeInstance(Sprite& t)
        {
            return Pickable<Instances::fl_display::Sprite>(new(t.Alloc()) Instances::fl_display::Sprite(t));
        }
        SPtr<Instances::fl_display::Sprite> MakeInstanceS(Sprite& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 14 };
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
    class Sprite : public fl_display::DisplayObjectContainer
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Sprite"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_display::Sprite InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Sprite(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

