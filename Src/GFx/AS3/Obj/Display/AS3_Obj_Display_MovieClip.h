//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_MovieClip.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_MovieClip_H
#define INC_AS3_Obj_Display_MovieClip_H

#include "AS3_Obj_Display_Sprite.h"
//##protect##"includes"
#include "GFx/GFx_Sprite.h"
//#include "GFx/AS3/AS3_AvmSprite.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo MovieClipTI;
    extern const ClassInfo MovieClipCI;
    extern const TypeInfo SceneTI;
    extern const ClassInfo SceneCI;
} // namespace fl_display
namespace fl
{
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo anyTI;
    extern const ClassInfo anyCI;
    extern const TypeInfo ObjectTI;
    extern const ClassInfo ObjectCI;
} // namespace fl

namespace ClassTraits { namespace fl_display
{
    class MovieClip;
}}

namespace InstanceTraits { namespace fl_display
{
    class MovieClip;
}}

namespace Classes { namespace fl_display
{
    class MovieClip;
}}

//##protect##"forward_declaration"
namespace Instances
{
    namespace fl_display
    {
        class Scene;
    }
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_display
{
    class MovieClip : public Instances::fl_display::Sprite
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_display::MovieClip;
        
    public:
        typedef MovieClip SelfType;
        typedef Classes::fl_display::MovieClip ClassType;
        typedef InstanceTraits::fl_display::MovieClip TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::MovieClipTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_display::MovieClip"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        MovieClip(InstanceTraits::Traits& t);

//##protect##"instance$methods"
       virtual void         ForEachChild_GC(Collector* prcc, GcOp op) const;
       
       SPtr<Instances::fl_display::Scene> CreateFakeScene();
       const MovieDataDef::SceneInfo*   GetSceneInfo(unsigned frameNumber) const;
       const MovieDataDef::SceneInfo*   GetSceneInfoByName(const ASString& name) const;
       bool                 GetLabeledFrame(GFx::Sprite* spr, const Value& frame, const Value& scene, unsigned* targetFrame);

    public:
        virtual bool        HasFrameScript() const;
        virtual bool        GetFrameScript(unsigned frame, Value* pmethod);
        virtual bool        IsMovieClip() const { return true; }

//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_currentFrameGet, 
            mid_currentFrameLabelGet, 
            mid_currentLabelGet, 
            mid_currentLabelsGet, 
            mid_currentSceneGet, 
            mid_enabledGet, 
            mid_enabledSet, 
            mid_framesLoadedGet, 
            mid_scenesGet, 
            mid_totalFramesGet, 
            mid_trackAsMenuGet, 
            mid_trackAsMenuSet, 
            mid_addFrameScript, 
            mid_gotoAndPlay, 
            mid_gotoAndStop, 
            mid_nextFrame, 
            mid_nextScene, 
            mid_play, 
            mid_prevFrame, 
            mid_prevScene, 
            mid_stop, 
        };
        void currentFrameGet(SInt32& result);
        void currentFrameLabelGet(ASString& result);
        void currentLabelGet(ASString& result);
        void currentLabelsGet(SPtr<Instances::fl::Array>& result);
        void currentSceneGet(SPtr<Instances::fl_display::Scene>& result);
        void enabledGet(bool& result);
        void enabledSet(const Value& result, bool value);
        void framesLoadedGet(SInt32& result);
        void scenesGet(SPtr<Instances::fl::Array>& result);
        void totalFramesGet(SInt32& result);
        void trackAsMenuGet(bool& result);
        void trackAsMenuSet(const Value& result, bool value);
        void addFrameScript(Value& result, unsigned argc, const Value* const argv);
        void gotoAndPlay(const Value& result, const Value& frame, const Value& scene = Value::GetNull());
        void gotoAndStop(const Value& result, const Value& frame, const Value& scene = Value::GetNull());
        void nextFrame(const Value& result);
        void nextScene(const Value& result);
        void play(const Value& result);
        void prevFrame(const Value& result);
        void prevScene(const Value& result);
        void stop(const Value& result);

        // C++ friendly wrappers for AS3 methods.
        SInt32 currentFrameGet()
        {
            SInt32 result;
            currentFrameGet(result);
            return result;
        }
        ASString currentFrameLabelGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            currentFrameLabelGet(result);
            return result;
        }
        ASString currentLabelGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            currentLabelGet(result);
            return result;
        }
        SPtr<Instances::fl::Array> currentLabelsGet();
        SPtr<Instances::fl_display::Scene> currentSceneGet();
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
        SInt32 framesLoadedGet()
        {
            SInt32 result;
            framesLoadedGet(result);
            return result;
        }
        SPtr<Instances::fl::Array> scenesGet();
        SInt32 totalFramesGet()
        {
            SInt32 result;
            totalFramesGet(result);
            return result;
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
        void gotoAndPlay(const Value& frame, const Value& scene = Value::GetNull())
        {
            gotoAndPlay(Value::GetUndefined(), frame, scene);
        }
        void gotoAndStop(const Value& frame, const Value& scene = Value::GetNull())
        {
            gotoAndStop(Value::GetUndefined(), frame, scene);
        }
        void nextFrame()
        {
            nextFrame(Value::GetUndefined());
        }
        void nextScene()
        {
            nextScene(Value::GetUndefined());
        }
        void play()
        {
            play(Value::GetUndefined());
        }
        void prevFrame()
        {
            prevFrame(Value::GetUndefined());
        }
        void prevScene()
        {
            prevScene(Value::GetUndefined());
        }
        void stop()
        {
            stop(Value::GetUndefined());
        }

//##protect##"instance$data"
        // This class contains associations of frame numbers with corresponding
        // ActionScript method to be called when appropriate frame is executed.
        // Contains a sorted by frame number array of descriptors along with
        // a bitset that indicates presence of action at the corresponding frame
        // (for quick check, before going to do sorted array search).
        class FrameScript
        {
        public:
            struct Descr
            {
                Value    Method;
                unsigned Frame;  // number of frame

                Descr():Frame(0) {}
                Descr(unsigned f, const Value& frMethod):Method(frMethod), Frame(f) {}
                Descr(const Descr& o): Method(o.Method), Frame(o.Frame) {}

                bool operator < (const Descr& f) const { return Frame < f.Frame; }
            };
        private:
            UByte*  pData;
            UInt16  DescrCnt; // number of Descr elements
            UInt16  FrameCnt; // total number of frames (pData[DescrCnt - 1].Number + 1))

            Descr*  GetDescs() const                        { return reinterpret_cast<Descr*>(pData + 4*((FrameCnt + 31)/32)); }
            UPInt   GetDescSize(unsigned newDescrCnt) const { return sizeof(Descr) * newDescrCnt + 4*((FrameCnt + 31)/32); }
        public:
            FrameScript() : pData(NULL), DescrCnt(0), FrameCnt(0) {}
            ~FrameScript();

            // initial setup
            void Set(MemoryHeap* pheap, unsigned totalFrames, unsigned cnt, const Descr*);

            // gets descr by frame number. Frame number is 0-based
            const Descr* Get(unsigned frameNum);

            // Release all records. Necessary to do before AVM shutdown.
            void Clear();

            bool IsEmpty() const { return DescrCnt == 0; }

            void ForEachChild_GC(Collector* prcc, GcOp op
                                 SF_DEBUG_ARG(const MovieClip& owner)) const;
        };

        FrameScript mFrameScript;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    class MovieClip : public fl_display::Sprite
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::MovieClip"; }
#endif
    public:
        typedef Instances::fl_display::MovieClip InstanceType;

    public:
        MovieClip(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_display::MovieClip> MakeInstance(MovieClip& t)
        {
            return Pickable<Instances::fl_display::MovieClip>(new(t.Alloc()) Instances::fl_display::MovieClip(t));
        }
        SPtr<Instances::fl_display::MovieClip> MakeInstanceS(MovieClip& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 21 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[27];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_display
{
    class MovieClip : public fl_display::Sprite
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::MovieClip"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_display::MovieClip InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        MovieClip(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

