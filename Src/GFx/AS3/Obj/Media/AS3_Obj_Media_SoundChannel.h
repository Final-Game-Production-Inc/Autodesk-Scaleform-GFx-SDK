//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Media_SoundChannel.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Media_SoundChannel_H
#define INC_AS3_Obj_Media_SoundChannel_H

#include "../Events/AS3_Obj_Events_EventDispatcher.h"
//##protect##"includes"
#include "GFx/AS3/Audio/AS3_SoundObject.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_media
{
    extern const TypeInfo SoundChannelTI;
    extern const ClassInfo SoundChannelCI;
    extern const TypeInfo SoundTransformTI;
    extern const ClassInfo SoundTransformCI;
} // namespace fl_media
namespace fl
{
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
} // namespace fl

namespace ClassTraits { namespace fl_media
{
    class SoundChannel;
}}

namespace InstanceTraits { namespace fl_media
{
    class SoundChannel;
}}

namespace Classes { namespace fl_media
{
    class SoundChannel;
}}

//##protect##"forward_declaration"
namespace Instances
{
    namespace fl_media
    {
        class SoundTransform;
    }
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_media
{
    class SoundChannel : public Instances::fl_events::EventDispatcher
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_media::SoundChannel;
        
    public:
        typedef SoundChannel SelfType;
        typedef Classes::fl_media::SoundChannel ClassType;
        typedef InstanceTraits::fl_media::SoundChannel TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_media::SoundChannelTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_media::SoundChannel"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        SoundChannel(InstanceTraits::Traits& t);

//##protect##"instance$methods"
#ifdef GFX_ENABLE_SOUND
    public:
        void DispatchEventSoundComplete();
#endif
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_leftPeakGet, 
            mid_positionGet, 
            mid_rightPeakGet, 
            mid_soundTransformGet, 
            mid_soundTransformSet, 
            mid_stop, 
        };
        void leftPeakGet(Value::Number& result);
        void positionGet(Value::Number& result);
        void rightPeakGet(Value::Number& result);
        void soundTransformGet(SPtr<Instances::fl_media::SoundTransform>& result);
        void soundTransformSet(const Value& result, Instances::fl_media::SoundTransform* value);
        void stop(const Value& result);

        // C++ friendly wrappers for AS3 methods.
        Value::Number leftPeakGet()
        {
            Value::Number result;
            leftPeakGet(result);
            return result;
        }
        Value::Number positionGet()
        {
            Value::Number result;
            positionGet(result);
            return result;
        }
        Value::Number rightPeakGet()
        {
            Value::Number result;
            rightPeakGet(result);
            return result;
        }
        SPtr<Instances::fl_media::SoundTransform> soundTransformGet();
        void soundTransformSet(Instances::fl_media::SoundTransform* value)
        {
            soundTransformSet(Value::GetUndefined(), value);
        }
        void stop()
        {
            stop(Value::GetUndefined());
        }

//##protect##"instance$data"
#ifdef GFX_ENABLE_SOUND
        Ptr<SoundObject> pSoundObject;
#endif
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_media
{
    class SoundChannel : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::SoundChannel"; }
#endif
    public:
        typedef Instances::fl_media::SoundChannel InstanceType;

    public:
        SoundChannel(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_media::SoundChannel> MakeInstance(SoundChannel& t)
        {
            return Pickable<Instances::fl_media::SoundChannel>(new(t.Alloc()) Instances::fl_media::SoundChannel(t));
        }
        SPtr<Instances::fl_media::SoundChannel> MakeInstanceS(SoundChannel& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 6 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[7];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_media
{
    class SoundChannel : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::SoundChannel"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_media::SoundChannel InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        SoundChannel(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

