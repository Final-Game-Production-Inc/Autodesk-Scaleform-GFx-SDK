//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_NetStream.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Net_NetStream_H
#define INC_AS3_Obj_Net_NetStream_H

#include "../Events/AS3_Obj_Events_EventDispatcher.h"
//##protect##"includes"
#ifdef GFX_ENABLE_VIDEO
#include "GFx/AS3/AS3_MovieRoot.h"
#include "Video/AS3/AS3_VideoProviderNetStream.h"
#endif
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_net
{
    extern const TypeInfo NetStreamTI;
    extern const ClassInfo NetStreamCI;
} // namespace fl_net
namespace fl
{
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo ObjectTI;
    extern const ClassInfo ObjectCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl
namespace fl_media
{
    extern const TypeInfo SoundTransformTI;
    extern const ClassInfo SoundTransformCI;
    extern const TypeInfo MicrophoneTI;
    extern const ClassInfo MicrophoneCI;
    extern const TypeInfo CameraTI;
    extern const ClassInfo CameraCI;
} // namespace fl_media

namespace ClassTraits { namespace fl_net
{
    class NetStream;
}}

namespace InstanceTraits { namespace fl_net
{
    class NetStream;
}}

namespace Classes { namespace fl_net
{
    class NetStream;
}}

//##protect##"forward_declaration"
namespace Instances
{
    namespace fl_media
    {
        class SoundTransform;
        class Microphone;
        class Camera;
    }
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_net
{
    class NetStream : public Instances::fl_events::EventDispatcher
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_net::NetStream;
        
    public:
        typedef NetStream SelfType;
        typedef Classes::fl_net::NetStream ClassType;
        typedef InstanceTraits::fl_net::NetStream TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_net::NetStreamTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_net::NetStream"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        NetStream(InstanceTraits::Traits& t);

//##protect##"instance$methods"
#ifdef GFX_ENABLE_VIDEO
        virtual void AS3Constructor(unsigned argc, const Value* argv);
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;
    public:
        ~NetStream();
        VideoProviderNetStream* GetVideoProvider() { return pVideoProvider; }

        class EventHandlers: public NetStreamInterface
        {
            Instances::fl_net::NetStream* pNetStream;
        public:
            EventHandlers(Instances::fl_net::NetStream* p) : pNetStream(p) { SF_ASSERT(pNetStream); }
            virtual void DispatchNetStatus(const String& code, const String& level);
            virtual void ExecuteOnMataData(GFx::Video::VideoPlayer::VideoInfo* pinfo);
            virtual void ExecuteOnCuePoint(GFx::Video::VideoPlayer::CuePoint* pcuepoint);
            virtual void ExecuteOnSubtitle(const String& subtitle);
        };
#endif
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_bufferLengthGet, 
            mid_bufferTimeGet, 
            mid_bufferTimeSet, 
            mid_bytesLoadedGet, 
            mid_bytesTotalGet, 
            mid_checkPolicyFileGet, 
            mid_checkPolicyFileSet, 
            mid_clientGet, 
            mid_clientSet, 
            mid_currentFPSGet, 
            mid_liveDelayGet, 
            mid_objectEncodingGet, 
            mid_soundTransformGet, 
            mid_soundTransformSet, 
            mid_timeGet, 
            mid_attachAudio, 
            mid_attachCamera, 
            mid_close, 
            mid_pause, 
            mid_play, 
            mid_publish, 
            mid_receiveAudio, 
            mid_receiveVideo, 
            mid_receiveVideoFPS, 
            mid_resume, 
            mid_seek, 
            mid_send, 
            mid_setDRMAuthenticationCredentials, 
            mid_togglePause, 
            mid_loopGet, 
            mid_loopSet, 
            mid_currentFrameGet, 
            mid_audioTrackGet, 
            mid_audioTrackSet, 
            mid_subAudioTrackGet, 
            mid_subAudioTrackSet, 
            mid_voiceTrackGet, 
            mid_voiceTrackSet, 
            mid_subtitleTrackGet, 
            mid_subtitleTrackSet, 
            mid_subSoundTransformGet, 
            mid_subSoundTransformSet, 
            mid_reloadThresholdTimeSet, 
            mid_numberOfFramePoolsSet, 
            mid_openTimeoutSet, 
        };
        void bufferLengthGet(Value::Number& result);
        void bufferTimeGet(Value::Number& result);
        void bufferTimeSet(const Value& result, Value::Number value);
        void bytesLoadedGet(UInt32& result);
        void bytesTotalGet(UInt32& result);
        void checkPolicyFileGet(bool& result);
        void checkPolicyFileSet(const Value& result, bool value);
        void clientGet(SPtr<Instances::fl::Object>& result);
        void clientSet(const Value& result, const Value& value);
        void currentFPSGet(Value::Number& result);
        void liveDelayGet(Value::Number& result);
        void objectEncodingGet(UInt32& result);
        void soundTransformGet(SPtr<Instances::fl_media::SoundTransform>& result);
        void soundTransformSet(const Value& result, Instances::fl_media::SoundTransform* value);
        void timeGet(Value::Number& result);
        void attachAudio(const Value& result, Instances::fl_events::EventDispatcher* microphone);
        void attachCamera(const Value& result, Instances::fl_events::EventDispatcher* theCamera, SInt32 snapshotMilliseconds = -1);
        void close(const Value& result, bool flag = false);
        void pause(const Value& result);
        void play(Value& result, unsigned argc, const Value* const argv);
        void publish(const Value& result, const ASString& name, const ASString& type);
        void receiveAudio(const Value& result, bool flag);
        void receiveVideo(const Value& result, bool flag);
        void receiveVideoFPS(const Value& result, Value::Number FPS);
        void resume(const Value& result);
        void seek(const Value& result, Value::Number offset);
        void send(Value& result, unsigned argc, const Value* const argv);
        void setDRMAuthenticationCredentials(const Value& result, const ASString& userName, const ASString& password, const ASString& type);
        void togglePause(const Value& result);
        void loopGet(bool& result);
        void loopSet(const Value& result, bool flag);
        void currentFrameGet(Value::Number& result);
        void audioTrackGet(Value::Number& result);
        void audioTrackSet(const Value& result, Value::Number track);
        void subAudioTrackGet(Value::Number& result);
        void subAudioTrackSet(const Value& result, Value::Number track);
        void voiceTrackGet(Value::Number& result);
        void voiceTrackSet(const Value& result, Value::Number track);
        void subtitleTrackGet(Value::Number& result);
        void subtitleTrackSet(const Value& result, Value::Number track);
        void subSoundTransformGet(SPtr<Instances::fl_media::SoundTransform>& result);
        void subSoundTransformSet(const Value& result, Instances::fl_media::SoundTransform* value);
        void reloadThresholdTimeSet(const Value& result, Value::Number time);
        void numberOfFramePoolsSet(const Value& result, Value::Number pools);
        void openTimeoutSet(const Value& result, Value::Number time);

        // C++ friendly wrappers for AS3 methods.
        Value::Number bufferLengthGet()
        {
            Value::Number result;
            bufferLengthGet(result);
            return result;
        }
        Value::Number bufferTimeGet()
        {
            Value::Number result;
            bufferTimeGet(result);
            return result;
        }
        void bufferTimeSet(Value::Number value)
        {
            bufferTimeSet(Value::GetUndefined(), value);
        }
        UInt32 bytesLoadedGet()
        {
            UInt32 result;
            bytesLoadedGet(result);
            return result;
        }
        UInt32 bytesTotalGet()
        {
            UInt32 result;
            bytesTotalGet(result);
            return result;
        }
        bool checkPolicyFileGet()
        {
            bool result;
            checkPolicyFileGet(result);
            return result;
        }
        void checkPolicyFileSet(bool value)
        {
            checkPolicyFileSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl::Object> clientGet()
        {
            SPtr<Instances::fl::Object> result;
            clientGet(result);
            return result;
        }
        void clientSet(const Value& value)
        {
            clientSet(Value::GetUndefined(), value);
        }
        Value::Number currentFPSGet()
        {
            Value::Number result;
            currentFPSGet(result);
            return result;
        }
        Value::Number liveDelayGet()
        {
            Value::Number result;
            liveDelayGet(result);
            return result;
        }
        UInt32 objectEncodingGet()
        {
            UInt32 result;
            objectEncodingGet(result);
            return result;
        }
        SPtr<Instances::fl_media::SoundTransform> soundTransformGet();
        void soundTransformSet(Instances::fl_media::SoundTransform* value)
        {
            soundTransformSet(Value::GetUndefined(), value);
        }
        Value::Number timeGet()
        {
            Value::Number result;
            timeGet(result);
            return result;
        }
        void attachAudio(Instances::fl_events::EventDispatcher* microphone)
        {
            attachAudio(Value::GetUndefined(), microphone);
        }
        void attachCamera(Instances::fl_events::EventDispatcher* theCamera, SInt32 snapshotMilliseconds = -1)
        {
            attachCamera(Value::GetUndefined(), theCamera, snapshotMilliseconds);
        }
        void close(bool flag = false)
        {
            close(Value::GetUndefined(), flag);
        }
        void pause()
        {
            pause(Value::GetUndefined());
        }
        void publish(const ASString& name, const ASString& type)
        {
            publish(Value::GetUndefined(), name, type);
        }
        void receiveAudio(bool flag)
        {
            receiveAudio(Value::GetUndefined(), flag);
        }
        void receiveVideo(bool flag)
        {
            receiveVideo(Value::GetUndefined(), flag);
        }
        void receiveVideoFPS(Value::Number FPS)
        {
            receiveVideoFPS(Value::GetUndefined(), FPS);
        }
        void resume()
        {
            resume(Value::GetUndefined());
        }
        void seek(Value::Number offset)
        {
            seek(Value::GetUndefined(), offset);
        }
        void setDRMAuthenticationCredentials(const ASString& userName, const ASString& password, const ASString& type)
        {
            setDRMAuthenticationCredentials(Value::GetUndefined(), userName, password, type);
        }
        void togglePause()
        {
            togglePause(Value::GetUndefined());
        }
        bool loopGet()
        {
            bool result;
            loopGet(result);
            return result;
        }
        void loopSet(bool flag)
        {
            loopSet(Value::GetUndefined(), flag);
        }
        Value::Number currentFrameGet()
        {
            Value::Number result;
            currentFrameGet(result);
            return result;
        }
        Value::Number audioTrackGet()
        {
            Value::Number result;
            audioTrackGet(result);
            return result;
        }
        void audioTrackSet(Value::Number track)
        {
            audioTrackSet(Value::GetUndefined(), track);
        }
        Value::Number subAudioTrackGet()
        {
            Value::Number result;
            subAudioTrackGet(result);
            return result;
        }
        void subAudioTrackSet(Value::Number track)
        {
            subAudioTrackSet(Value::GetUndefined(), track);
        }
        Value::Number voiceTrackGet()
        {
            Value::Number result;
            voiceTrackGet(result);
            return result;
        }
        void voiceTrackSet(Value::Number track)
        {
            voiceTrackSet(Value::GetUndefined(), track);
        }
        Value::Number subtitleTrackGet()
        {
            Value::Number result;
            subtitleTrackGet(result);
            return result;
        }
        void subtitleTrackSet(Value::Number track)
        {
            subtitleTrackSet(Value::GetUndefined(), track);
        }
        SPtr<Instances::fl_media::SoundTransform> subSoundTransformGet();
        void subSoundTransformSet(Instances::fl_media::SoundTransform* value)
        {
            subSoundTransformSet(Value::GetUndefined(), value);
        }
        void reloadThresholdTimeSet(Value::Number time)
        {
            reloadThresholdTimeSet(Value::GetUndefined(), time);
        }
        void numberOfFramePoolsSet(Value::Number pools)
        {
            numberOfFramePoolsSet(Value::GetUndefined(), pools);
        }
        void openTimeoutSet(Value::Number time)
        {
            openTimeoutSet(Value::GetUndefined(), time);
        }

//##protect##"instance$data"
#ifdef GFX_ENABLE_VIDEO
        Ptr<VideoProviderNetStream>  pVideoProvider;
        Ptr<NetStreamInterface>      pEventHandlers;
        bool Paused;

        float    BufferTime;
        float    ReloadThresholdTime;
        unsigned NumberOfFramePools;
        float    OpenTimeout;

        bool     LoopFlag;
        int      AudioTrack;
        int      SubAudioTrack;
        int      VoiceTrack;
        int      SubtitleTrack;

        Value::Number SoundVolume;
        Value::Number SubSoundVolume;

        Value    OnMetaDataFunc;
        Value    OnCuePointFunc;
        Value    OnSubtitleFunc;
#endif
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_net
{
    class NetStream : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::NetStream"; }
#endif
    public:
        typedef Instances::fl_net::NetStream InstanceType;

    public:
        NetStream(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_net::NetStream> MakeInstance(NetStream& t)
        {
            return Pickable<Instances::fl_net::NetStream>(new(t.Alloc()) Instances::fl_net::NetStream(t));
        }
        SPtr<Instances::fl_net::NetStream> MakeInstanceS(NetStream& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 45 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[72];
        static const Abc::ConstValue dva[3];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_net
{
    class NetStream : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::NetStream"; }
#endif
    public:
        typedef Classes::fl_net::NetStream ClassType;
        typedef InstanceTraits::fl_net::NetStream InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        NetStream(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { ThunkInfoNum = 1 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[1];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_net
{
    class NetStream : public Class
    {
        friend class ClassTraits::fl_net::NetStream;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_net::NetStreamTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::NetStream"; }
#endif
    public:
        typedef NetStream SelfType;
        typedef NetStream ClassType;
        
    private:
        NetStream(ClassTraits::Traits& t);
       
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
            mid_resetDRMVouchers, 
        };
        void resetDRMVouchers(const Value& result);

        // C++ friendly wrappers for AS3 methods.
        void resetDRMVouchers()
        {
            resetDRMVouchers(Value::GetUndefined());
        }

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

