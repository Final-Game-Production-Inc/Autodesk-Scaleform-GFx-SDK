//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Media_Video.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Media_Video_H
#define INC_AS3_Obj_Media_Video_H

#include "../Display/AS3_Obj_Display_DisplayObject.h"
//##protect##"includes"
#include "GFx/GFx_DisplayObject.h"
#ifdef GFX_ENABLE_VIDEO
#include "Video/Video_VideoCharacter.h"
#endif
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_media
{
    extern const TypeInfo VideoTI;
    extern const ClassInfo VideoCI;
    extern const TypeInfo CameraTI;
    extern const ClassInfo CameraCI;
} // namespace fl_media
namespace fl
{
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
} // namespace fl
namespace fl_net
{
    extern const TypeInfo NetStreamTI;
    extern const ClassInfo NetStreamCI;
} // namespace fl_net

namespace ClassTraits { namespace fl_media
{
    class Video;
}}

namespace InstanceTraits { namespace fl_media
{
    class Video;
}}

namespace Classes { namespace fl_media
{
    class Video;
}}

//##protect##"forward_declaration"
namespace Instances
{
    namespace fl_media
    {
        class Camera;
    }
    namespace fl_net
    {
        class NetStream;
    }
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_media
{
    class Video : public Instances::fl_display::DisplayObject
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_media::Video;
        
    public:
        typedef Video SelfType;
        typedef Classes::fl_media::Video ClassType;
        typedef InstanceTraits::fl_media::Video TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_media::VideoTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_media::Video"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Video(InstanceTraits::Traits& t);

//##protect##"instance$methods"
#ifdef GFX_ENABLE_VIDEO
        virtual void AS3Constructor(unsigned argc, const Value* argv);
        virtual void InitInstance(bool extCall = false);
        virtual GFx::DisplayObject* CreateStageObject();

        GFx::Video::VideoCharacter* GetVideoCharacter() const 
        {
            SF_ASSERT(pDispObj && pDispObj->GetType() == CharacterDef::Video);
            return static_cast<GFx::Video::VideoCharacter*>(pDispObj.GetPtr()); 
        }
#endif
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_deblockingGet, 
            mid_deblockingSet, 
            mid_smoothingGet, 
            mid_smoothingSet, 
            mid_videoHeightGet, 
            mid_videoWidthGet, 
            mid_attachCamera, 
            mid_attachNetStream, 
            mid_clear, 
        };
        void deblockingGet(SInt32& result);
        void deblockingSet(const Value& result, SInt32 value);
        void smoothingGet(bool& result);
        void smoothingSet(const Value& result, bool value);
        void videoHeightGet(SInt32& result);
        void videoWidthGet(SInt32& result);
        void attachCamera(const Value& result, Instances::fl_events::EventDispatcher* camera);
        void attachNetStream(const Value& result, Instances::fl_net::NetStream* netStream);
        void clear(const Value& result);

        // C++ friendly wrappers for AS3 methods.
        SInt32 deblockingGet()
        {
            SInt32 result;
            deblockingGet(result);
            return result;
        }
        void deblockingSet(SInt32 value)
        {
            deblockingSet(Value::GetUndefined(), value);
        }
        bool smoothingGet()
        {
            bool result;
            smoothingGet(result);
            return result;
        }
        void smoothingSet(bool value)
        {
            smoothingSet(Value::GetUndefined(), value);
        }
        SInt32 videoHeightGet()
        {
            SInt32 result;
            videoHeightGet(result);
            return result;
        }
        SInt32 videoWidthGet()
        {
            SInt32 result;
            videoWidthGet(result);
            return result;
        }
        void attachCamera(Instances::fl_events::EventDispatcher* camera)
        {
            attachCamera(Value::GetUndefined(), camera);
        }
        void attachNetStream(Instances::fl_net::NetStream* netStream)
        {
            attachNetStream(Value::GetUndefined(), netStream);
        }
        void clear()
        {
            clear(Value::GetUndefined());
        }

//##protect##"instance$data"
#ifdef GFX_ENABLE_VIDEO
        Value::Number Width;
        Value::Number Height;
#endif
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_media
{
    class Video : public fl_display::DisplayObject
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Video"; }
#endif
    public:
        typedef Instances::fl_media::Video InstanceType;

    public:
        Video(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_media::Video> MakeInstance(Video& t)
        {
            return Pickable<Instances::fl_media::Video>(new(t.Alloc()) Instances::fl_media::Video(t));
        }
        SPtr<Instances::fl_media::Video> MakeInstanceS(Video& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 9 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[13];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_media
{
    class Video : public fl_display::DisplayObject
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Video"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_media::Video InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Video(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

