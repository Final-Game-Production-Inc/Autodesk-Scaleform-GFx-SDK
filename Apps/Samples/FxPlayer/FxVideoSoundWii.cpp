/**************************************************************************

Filename    :   FxVideoSoundWii.cpp
Content     :   Sample system based allocator class that can be
                installed in FxPlayer to show overriding allocators.
Created     :   January 15, 2008
Authors     :   Michael Antonov, Andrew Reisse, Maxim Didenko

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxVideoSoundWii.h"

#include "cri_heap.h"

#include "CriSmpMvSound_Wii.h"

/*
#include "GSoundPlayer.h"
class GSoundChannelSDL : public GSoundChannel
{
public:
    virtual GSoundPlayer*  GetSoundPlayer() const                              = 0;
    virtual GSoundSample*  GetSample() const                                   = 0;

    virtual void           Stop()                                              = 0;
    virtual void           Pause(bool pause)                                   = 0;
    virtual bool           IsPlaying() const                                   = 0;
    virtual void           SetPosition(UInt samples)                           = 0;
    virtual void           SetPositionF(Float seconds)                         = 0;
    virtual UInt           GetPosition()                                       = 0;
    virtual Float          GetPositionF()                                      = 0;
    virtual void           Loop(SInt count, UInt start = 0, UInt end = 0)      = 0;
    virtual void           LoopF(SInt count, Float start = 0, Float end = 0)   = 0;
    virtual SInt           GetVolume()                                         = 0;
    virtual void           SetVolume(SInt volume)                              = 0;
};

*/


//////////////////////////////////////////////////////////////////////////
// GFxVideoSoundInterfaceWii


class FxVideoSoundCreatorWiiImpl;
class FxVideoSoundInterfaceWii : public GFxVideoSound
{
public:
    FxVideoSoundInterfaceWii(FxVideoSoundCreatorWiiImpl* parent);
    virtual ~FxVideoSoundInterfaceWii();

    virtual bool CreateOutput(UInt32 channel, UInt32 samplerate);
    virtual void DestroyOutput();

    virtual void SetCallbackPCMDataFunc(PCMDataFunc func, Handle obj);
    virtual void Start();
    virtual void Stop();
    virtual Status GetStatus();

    virtual void Pause(bool sw);
    virtual void GetTime(UInt64* count, UInt64* unit);

    virtual GSoundChannel* GetSoundChannel() { return 0; }


    FxVideoSoundCreatorWiiImpl* pParent;
/*
    GPtr<GSoundChannel> pChannel;
*/
    CriSmpMvSoundWii* pSoundWii;
};

class FxVideoSoundCreatorWiiImpl : public GFxVideoSoundCreator
{
public:
    FxVideoSoundCreatorWiiImpl();
    ~FxVideoSoundCreatorWiiImpl();

    virtual GFxVideoSound* Create(GMemoryHeap* pheap);


    CriHeap heap;
    void *heapwork;
};

#define SIZE_HEAPWORK		(1024 * 512)

FxVideoSoundCreatorWiiImpl::FxVideoSoundCreatorWiiImpl()
{
    heapwork = OSAlloc(SIZE_HEAPWORK);
    criHeap_Initialize();
    heap = criHeap_Create((void*)heapwork, SIZE_HEAPWORK);
    CriSmpMvSoundWii::Initialize();
}

FxVideoSoundCreatorWiiImpl::~FxVideoSoundCreatorWiiImpl()
{
    CriSmpMvSoundWii::Finalize();
    criHeap_Destroy(heap); heap = NULL;
    OSFree(heapwork); heapwork = NULL;
    criHeap_Finalize();
}

GFxVideoSound* FxVideoSoundCreatorWiiImpl::Create(GMemoryHeap* pheap)
{
    return new FxVideoSoundInterfaceWii(this);
}



//////////////////////////////////////////////////////////////////////////

FxVideoSoundInterfaceWii::FxVideoSoundInterfaceWii(FxVideoSoundCreatorWiiImpl* pparent)
    : pParent(pparent)
{
    pSoundWii = CriSmpMvSoundWii::Create(pParent->heap);
}

FxVideoSoundInterfaceWii::~FxVideoSoundInterfaceWii()
{
    pSoundWii->Destroy();
}


bool FxVideoSoundInterfaceWii::CreateOutput(UInt32 channels, UInt32 samplerate)
{
    if (channels == 0)
        return false;

    return pSoundWii->CreateOutput(pParent->heap, channels, samplerate);
}


void FxVideoSoundInterfaceWii::DestroyOutput()
{
    pSoundWii->DestroyOutput();
}


void FxVideoSoundInterfaceWii::SetCallbackPCMDataFunc(PCMDataFunc func, Handle obj)
{
    pSoundWii->SetCallbackGetSint16PcmData(func,obj);
}

void FxVideoSoundInterfaceWii::Start(void)
{
    pSoundWii->Start();
}

void FxVideoSoundInterfaceWii::Stop(void)
{
    pSoundWii->Stop();
}

void FxVideoSoundInterfaceWii::Pause(bool sw)
{
    pSoundWii->Pause(sw);
}

GFxVideoSound::Status FxVideoSoundInterfaceWii::GetStatus(void)
{
    CriMvSoundInterface::Status st = pSoundWii->GetStatus();
    if (st == CriMvSoundInterface::MVEASY_SOUND_STATUS_STOP)
        return Sound_Stopped;
    else if (st == CriMvSoundInterface::MVEASY_SOUND_STATUS_EXEC)
        return Sound_Playing;
    else
        return Sound_Error;
}

void FxVideoSoundInterfaceWii::GetTime(UInt64* count, UInt64* unit)
{	
    pSoundWii->GetTime(*count, *unit);
}

/*
GSoundChannel* GFxVideoSoundInterfaceSDL::GetSoundChannel()
{
    return pChannel;
}
*/
/*
inline Sint16 PcmfToPcm16(Float32 pcmf) {
if ( pcmf > -1.0f && pcmf <=1.0f ) {
return (Sint16)(pcmf*32767.0f);
} else if ( pcmf > 1.0f ) {
return  32767;
} else {
return -32768;
}
}
*/


//////////////////////////////////////////////////////////////////////////

FxVideoSoundCreatorWii::FxVideoSoundCreatorWii()
{
    pImpl = new FxVideoSoundCreatorWiiImpl;
}

FxVideoSoundCreatorWii::~FxVideoSoundCreatorWii()
{
    pImpl->Release();
}

GFxVideoSound* FxVideoSoundCreatorWii::Create(GMemoryHeap* pheap)
{
    return pImpl->Create(pheap);
}
