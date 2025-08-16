/**************************************************************************

Filename    :   Video_CriMvSoundGateway.h
Content     :   Video sound data gateway
Created     :   October 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_VIDEO_CRIMVSOUNDGATEWAY_H
#define INC_GFX_VIDEO_CRIMVSOUNDGATEWAY_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Kernel/SF_Memory.h"
#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#undef new
#endif
#include <cri_xpt.h>
#include <cri_error.h>
#include <cri_movie.h>
#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#define new SF_DEFINE_NEW
#endif

#include "Kernel/SF_MemoryHeap.h"
#include "Video/Video_Video.h"

namespace Scaleform { 

#ifdef GFX_ENABLE_SOUND
namespace Sound {
    class SoundChannel;
}
#endif

namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

class CriMvSound : public RefCountBaseNTS<CriMvSound,Stat_Video_Mem>,
                   public CriMvSoundInterface
{
public:
    // Data types for CRI callbacks
    typedef CriUint32 (*GetFloatPcmDataCallback)(void *obj, CriUint32 nch, CriFloat32 *pcmbuf[], CriUint32 req_nsmpl);
    typedef CriUint32 (*GetSInt16PcmDataCallback)(void *obj, CriUint32 nch, CriSint16 *pcmbuf[], CriUint32 req_nsmpl);

    CriMvSound(MemoryHeap*, VideoSound* psound) : pVSInterface(psound) {}
    virtual ~CriMvSound() {}

    // Implementation of CriMvSoundInterface
    virtual Bool CreateOutput(CriHeap heap, Uint32 channel, Uint32 samplerate);
    virtual void DestroyOutput(void);

    virtual CriMvSoundInterface::PcmFormat GetPcmFormat(void);

    virtual void SetCallbackGetFloat32PcmData(GetFloatPcmDataCallback func, void *obj)
    {
        StreamImpl.CallbackFloat     = func;
        StreamImpl.CallbackFloatData = obj;
    }
    virtual void SetCallbackGetSint16PcmData(GetSInt16PcmDataCallback func, void *obj)
    {
        StreamImpl.CallbackSInt16     = func;
        StreamImpl.CallbackSInt16Data = obj;
    }

    virtual void Start(void);
    virtual void Stop(void);
    virtual CriMvSoundInterface::Status GetStatus(void);
    virtual void Pause(Bool sw);
    virtual void GetTime(Uint64 &count, Uint64 &unit);

#ifdef GFX_ENABLE_SOUND
    virtual Sound::SoundChannel* GetSoundChannel();
#endif

private:
    // This class holds CRI callbacks for the sound, turning them into a simpler
    // PCMStream implementation, which is used by our VideoSound implementation
    // to request video sound data
    class PCMStreamImpl : public VideoSound::PCMStream
    {
    public:
        GetFloatPcmDataCallback   CallbackFloat;
        void*                     CallbackFloatData;
        GetSInt16PcmDataCallback  CallbackSInt16;
        void*                     CallbackSInt16Data;

        PCMStreamImpl()
        {
            CallbackFloat      = 0;
            CallbackFloatData  = 0;
            CallbackSInt16     = 0;
            CallbackSInt16Data = 0;
        }

        // Implement query virtual functions to return data though CRI-installed callbacks
        virtual UInt32  GetDataSInt16(UInt32 nch, SInt16 *pcmbuf[], UInt32 req_nsmpl)
        {
            if (CallbackSInt16)
                return CallbackSInt16(CallbackSInt16Data, nch, pcmbuf, req_nsmpl);
            return 0;
        }        
        virtual UInt32  GetDataFloat(UInt32 nch, float *pcmbuf[], UInt32 req_nsmpl)
        {
            if (CallbackFloat)
                return CallbackFloat(CallbackFloatData, nch, pcmbuf, req_nsmpl);
            return 0;
        }
    };

    Ptr<VideoSound>     pVSInterface;
    PCMStreamImpl       StreamImpl;
};

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

#endif // INC_GFX_VIDEO_CRIMVSOUNDGATEWAY_H
