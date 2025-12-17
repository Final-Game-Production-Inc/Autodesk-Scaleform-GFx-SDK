//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Media_Sound.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Media_Sound_H
#define INC_AS3_Obj_Media_Sound_H

#include "../Events/AS3_Obj_Events_EventDispatcher.h"
//##protect##"includes"
#include "GFx/AS3/Audio/AS3_SoundObject.h"
#include "GFx/AS3/Obj/Media/AS3_Obj_Media_SoundChannel.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_media
{
    extern const TypeInfo SoundTI;
    extern const ClassInfo SoundCI;
    extern const TypeInfo ID3InfoTI;
    extern const ClassInfo ID3InfoCI;
    extern const TypeInfo SoundLoaderContextTI;
    extern const ClassInfo SoundLoaderContextCI;
    extern const TypeInfo SoundTransformTI;
    extern const ClassInfo SoundTransformCI;
} // namespace fl_media
namespace fl
{
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl
namespace fl_net
{
    extern const TypeInfo URLRequestTI;
    extern const ClassInfo URLRequestCI;
} // namespace fl_net

namespace ClassTraits { namespace fl_media
{
    class Sound;
}}

namespace InstanceTraits { namespace fl_media
{
    class Sound;
}}

namespace Classes { namespace fl_media
{
    class Sound;
}}

//##protect##"forward_declaration"
namespace Instances
{
    namespace fl_media
    {
        class ID3Info;
        class SoundChannel;
        class SoundLoaderContext;
        class SoundTransform;
    }
    namespace fl_net
    {
        class URLRequest;
    }
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_media
{
    class Sound : public Instances::fl_events::EventDispatcher
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_media::Sound;
        
    public:
        typedef Sound SelfType;
        typedef Classes::fl_media::Sound ClassType;
        typedef InstanceTraits::fl_media::Sound TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_media::SoundTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_media::Sound"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Sound(InstanceTraits::Traits& t);

//##protect##"instance$methods"
#ifdef GFX_ENABLE_SOUND
        virtual void AS3Constructor(unsigned argc, const Value* argv);
        virtual void InitInstance(bool extCall);
        bool CreateLibraryObject();
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;

    public:
        void DispatchEventOpen();
        void DispatchEventComplete();
        void DispatchEventSoundComplete();
        void DispatchEventIOError();
#endif
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_bytesLoadedGet, 
            mid_bytesTotalGet, 
            mid_id3Get, 
            mid_isBufferingGet, 
            mid_lengthGet, 
            mid_urlGet, 
            mid_close, 
            mid_load, 
            mid_play, 
        };
        void bytesLoadedGet(UInt32& result);
        void bytesTotalGet(SInt32& result);
        void id3Get(SPtr<Instances::fl_media::ID3Info>& result);
        void isBufferingGet(bool& result);
        void lengthGet(Value::Number& result);
        void urlGet(ASString& result);
        void close(const Value& result);
        void load(const Value& result, Instances::fl_net::URLRequest* stream, Instances::fl_media::SoundLoaderContext* context = NULL);
        void play(Value& result, Value::Number startTime = 0.0, SInt32 loops = 0, Instances::fl_media::SoundTransform* sndTransform = NULL);

        // C++ friendly wrappers for AS3 methods.
        UInt32 bytesLoadedGet()
        {
            UInt32 result;
            bytesLoadedGet(result);
            return result;
        }
        SInt32 bytesTotalGet()
        {
            SInt32 result;
            bytesTotalGet(result);
            return result;
        }
        SPtr<Instances::fl_media::ID3Info> id3Get();
        bool isBufferingGet()
        {
            bool result;
            isBufferingGet(result);
            return result;
        }
        Value::Number lengthGet()
        {
            Value::Number result;
            lengthGet(result);
            return result;
        }
        ASString urlGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            urlGet(result);
            return result;
        }
        void close()
        {
            close(Value::GetUndefined());
        }
        void load(Instances::fl_net::URLRequest* stream, Instances::fl_media::SoundLoaderContext* context = NULL)
        {
            load(Value::GetUndefined(), stream, context);
        }
        Value play(Value::Number startTime = 0.0, SInt32 loops = 0, Instances::fl_media::SoundTransform* sndTransform = NULL)
        {
            Value result;
            play(result, startTime, loops, sndTransform);
            return result;
        }

//##protect##"instance$data"
#ifdef GFX_ENABLE_SOUND
        Ptr<SoundObject>   pSoundObject;
        Ptr<SoundResource> pSoundResource;
        MovieDefImpl*      pMovieDef;
        String             SoundURL;
        bool               Streaming;
        SPtr<Instances::fl_media::SoundChannel> pChannel;
#endif
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_media
{
    class Sound : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Sound"; }
#endif
    public:
        typedef Instances::fl_media::Sound InstanceType;

    public:
        Sound(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_media::Sound> MakeInstance(Sound& t)
        {
            return Pickable<Instances::fl_media::Sound>(new(t.Alloc()) Instances::fl_media::Sound(t));
        }
        SPtr<Instances::fl_media::Sound> MakeInstanceS(Sound& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 9 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[14];
        static const Abc::ConstValue dva[3];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_media
{
    class Sound : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Sound"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_media::Sound InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Sound(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

