/**************************************************************************

Filename    :   Video_VideoSoundSystemWiiU.cpp
Content     :   Video sound system implementation for WiiU
Created     :   February 2012
Authors     :   Vladislav Merker

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Video/Video_VideoSoundSystemWiiU.h"

#ifdef GFX_ENABLE_VIDEO

#include "Video/Video_SystemSoundInterface.h"
#include "Kernel/SF_HeapNew.h"

#include <cafe.h>
#include <cafe/os.h>
#include <cafe/ai.h>
#include <cafe/ax.h>
#include <cafe/mix.h>


namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

class VideoSoundWiiU : public VideoSoundStream
{
public:
    VideoSoundWiiU(MemoryHeap* pheap);
    virtual ~VideoSoundWiiU() { DestroyOutput(); }

    virtual bool CreateOutput(UInt32 channel, UInt32 samplerate);
    virtual void DestroyOutput();

    virtual PCMFormat GetPCMFormat() const;
    virtual void Start(PCMStream* pstream);
    virtual void Stop();
    virtual Status GetStatus() const;

    virtual void SetVolume(float volume);
    virtual float GetVolume();

    virtual void Pause(bool sw);
    virtual void GetTime(UInt64* count, UInt64* unit) const;

    static int SoundThreadFunc(int intArg, void *ptrArg);
    static void SoundAxCallback(void);
    static void* VoiceDropCallback(AXVPB* voice);

    void ExecuteAudioFrame(void);
    UInt32 GetSoundData(UInt32 nch, SInt16 *sample[], UInt32 nsmpl);
    void CleanupOutputResources(void);

    static UInt32 SampleToByte(UInt32 nsmpl) { return nsmpl * sizeof(SInt16); }
    static UInt32 ByteToSample(UInt32 nbyte) { return nbyte / sizeof(SInt16); }

    MemoryHeap* pHeap;
    PCMStream*  pPCMStream;

    static const UInt32 BUF_NSMPL = 144*10;
    static const UInt32 MAX_NCH = 6;
    static const SInt32 SOUND_MAX_HN = 4;

    AXVPB *axvoice[MAX_NCH];        //  AX Voice
    UInt32 buf_nsmpl;               //  Buffer size (SAMPLE)
    UInt32 rp;                      //  Read pointer (SAMPLE)
    UInt32 wp;                      //  Write Pointer (SAMPLE)
    UInt32 ndata;                   //  Number of samples in the buffer
    UInt32 nch;                     //  Number of channels
    float  vol;

    UInt32 size_output_buffer;
    SInt16 *output_buffer[MAX_NCH]; // 64 byte align

    SInt32      idx_handle_table;
    bool        is_available_ax_voice;
    OSMutex     mutex_sound_hn;

    Status      stat;
    UInt32      out_channels;
    UInt32      samplerate;
    bool        pause_flag;
    bool        command_pause;
    bool        command_resume;
    UInt64      total_play_nsmpls;
	
    UInt64      total_recv_nsmpls;
    UInt8       exec_flag;
    bool        voice_drop_flag;

    struct Globals : public RefCountBase<Globals, Stat_Video_Mem>
    {
        OSThread SoundThread;
        OSMutex  mutex;
        bool     SoundThread_flag;
        bool     SoundThread_actflag;
        bool     SoundThread_endflag;
        AXUserCallback OrigAxCb;
        VideoSoundWiiU* SoundPlayers[SOUND_MAX_HN];
        void* ThreadStack;

        Globals();
        ~Globals();
    };

    Ptr<Globals> pGlobals;
};

static Ptr<VideoSoundWiiU::Globals> pSoundGlobals = 0;

//////////////////////////////////////////////////////////////////////////
//

VideoSound::PCMFormat VideoSoundWiiU::GetPCMFormat() const
{
    return PCM_SInt16;
}

int VideoSoundWiiU::SoundThreadFunc(int intArg, void *ptrArg)
{
    SF_UNUSED2(intArg, ptrArg);

    for (;;) {
        if (pSoundGlobals->SoundThread_actflag == FALSE) {
            break;
        }

        OSLockMutex(&pSoundGlobals->mutex);
        for(int i=0; i < 4; i++)
            if (pSoundGlobals->SoundPlayers[i])
                pSoundGlobals->SoundPlayers[i]->ExecuteAudioFrame();
        OSUnlockMutex(&pSoundGlobals->mutex);

        MIXUpdateSettings();
    }
    pSoundGlobals->SoundThread_endflag = TRUE;

    return 0;
}

VideoSoundWiiU::Globals::Globals()
{
    SF_ASSERT(pSoundGlobals.GetPtr() == 0);
    pSoundGlobals = *this;

    ThreadStack = SF_MEMALIGN(8192, 32, Stat_Video_Mem);

    pSoundGlobals->SoundThread_flag = OSCreateThread(
        &pSoundGlobals->SoundThread, SoundThreadFunc, 0, NULL,
        (void*)((UInt32)pSoundGlobals->ThreadStack + 8192), 8192, 12, 0);

    SF_ASSERT(pSoundGlobals->SoundThread_flag);

    pSoundGlobals->SoundThread_actflag = TRUE;
    pSoundGlobals->SoundThread_endflag = FALSE;
    OSInitMutex(&pSoundGlobals->mutex);
    pSoundGlobals->OrigAxCb = AXRegisterCallback(SoundAxCallback);

    for (int i = 0; i < SOUND_MAX_HN; i++)
        pSoundGlobals->SoundPlayers[i] = NULL;
}

VideoSoundWiiU::Globals::~Globals()
{
    SF_ASSERT(this == pSoundGlobals);

    AXRegisterCallback(pSoundGlobals->OrigAxCb);

    if (pSoundGlobals->SoundThread_flag)
	{
        pSoundGlobals->SoundThread_actflag = FALSE;

        while (pSoundGlobals->SoundThread_endflag == FALSE) {
            OSResumeThread(&pSoundGlobals->SoundThread);
            OSSleepMilliseconds(1);
        }
        OSJoinThread(&pSoundGlobals->SoundThread, NULL);
        pSoundGlobals->SoundThread_flag = FALSE;
    }

    pSoundGlobals = 0;
    SF_FREE(ThreadStack);
}

void VideoSoundWiiU::SoundAxCallback(void)
{
    MIXUpdateSettings();

    if (pSoundGlobals->SoundThread_endflag == FALSE) {
        OSResumeThread(&pSoundGlobals->SoundThread);
    }

    if (pSoundGlobals->OrigAxCb != NULL)
	{
        (pSoundGlobals->OrigAxCb)();
    }
}

VideoSoundWiiU::VideoSoundWiiU(MemoryHeap* pheap)
{
    pHeap = pheap;

    is_available_ax_voice = FALSE;
    stat = Sound_Stopped;
    voice_drop_flag = FALSE;
    vol = 1.0f;
    if (!pSoundGlobals)
        pSoundGlobals = new Globals;
    pPCMStream = NULL;
    OSInitMutex(&this->mutex_sound_hn);
    idx_handle_table = -1;
    this->nch = 0;
    this->buf_nsmpl = 0;
    this->rp = 0;
    this->wp = 0;
    this->ndata = 0;
    this->total_play_nsmpls = 0;
    this->pause_flag = FALSE;
    this->voice_drop_flag = FALSE;
    this->exec_flag = 0;
    this->samplerate = 0;
    this->vol = 0;
}

bool VideoSoundWiiU::CreateOutput(UInt32 channel, UInt32 samplerate)
{
    SInt32 i;

    if (channel == 0) {
        return FALSE;
    }
    if (channel > MAX_NCH) {
        return FALSE;
    }

    OSLockMutex(&pSoundGlobals->mutex);
    this->idx_handle_table = -1;
    for(i = 0; i<SOUND_MAX_HN; i++) {
        if(pSoundGlobals->SoundPlayers[i] == NULL){
            this->idx_handle_table = i;
            break;
        }
    }
    if (this->idx_handle_table == -1) {
        OSReport("Error: No available VideoSoundSound handle.\n");
        OSUnlockMutex(&pSoundGlobals->mutex);
        return FALSE;
    }

    this->nch = channel;
    this->buf_nsmpl = BUF_NSMPL;
    this->rp = 0;
    this->wp = 0;
    this->ndata = 0;
    this->total_play_nsmpls = 0;
    this->pause_flag = FALSE;
    this->voice_drop_flag = FALSE;
    this->exec_flag = 0;
    this->samplerate = samplerate;
    this->vol = 1.0f;
 
    for (i = 0; i < this->nch; i++) {
        AXPBSRC     src;
        AXPBOFFSET  offset;
        UInt32      start_addr, end_addr, cur_addr;
        UInt32      src_bits;
        UInt8       pan;

        this->size_output_buffer = BUF_NSMPL * sizeof(SInt16) + 63;
        this->output_buffer[i] = (SInt16*)SF_HEAP_MEMALIGN(pHeap, (SInt32)this->size_output_buffer, 64, Stat_Video_Mem);
        if (this->output_buffer[i] == NULL) {
            this->CleanupOutputResources();
            OSUnlockMutex(&pSoundGlobals->mutex);
            return FALSE;
        }

        // Acquire AX voice
        this->axvoice[i] = AXAcquireVoice(AX_PRIORITY_NODROP, (AXVoiceCallback)VoiceDropCallback, (UInt32)this);
        if (this->axvoice[i] == NULL) {
            this->CleanupOutputResources();
            OSUnlockMutex(&pSoundGlobals->mutex);
            return FALSE;
        }

        if (this->nch == 1) {
            // MONAURAL (CENTER)
            pan = 64;
        } else if (this->nch == 2) {
            // STEREO
            if (i == 0) {
                // LEFT
                pan = 0;
            } else {
                // RIGHT
                pan = 127;
            }
        } else {
            // 5.1ch ? // Not support
            pan = 64;
        }

        // Initialize Mixer channel
        MIXInitChannel(this->axvoice[i], 
            MIX_MODE_AUXA_PREFADER|MIX_MODE_AUXB_PREFADER|MIX_MODE_AUXC_PREFADER,   //  mode 
            0,      //  vol
            -960,   //  aux A
            -960,   //  aux B
            -960,   //  aux C
            pan,    //  panpot
            127,    //  suround pan
            0       //  main
            );

        // Set sampling frequency
        src_bits = (u32)(0x00010000 * ((f32)samplerate / AX_IN_SAMPLES_PER_SEC));
        src.ratioHi = (u16)(src_bits >> 16);
        src.ratioLo = (u16)(src_bits & 0xFFFF);
        src.currentAddressFrac = 0;
        src.last_samples[0] = 0;
        src.last_samples[1] = 0;
        src.last_samples[2] = 0;
        src.last_samples[3] = 0;
        AXSetVoiceSrcType(this->axvoice[i], AX_SRC_TYPE_LINEAR);    // SRC type
        AXSetVoiceSrc(this->axvoice[i], &src);                      // initial SRC settings

        // Set Voice buffer
        offset.format   = AX_PB_FORMAT_PCM16;
        offset.loopFlag = AXPBADDR_LOOP_ON;
        start_addr = (UInt32)this->output_buffer[i];
        end_addr = start_addr + SampleToByte(BUF_NSMPL) - 2;
        cur_addr = start_addr;
        offset.loopOffset = start_addr;
        offset.endOffset  = end_addr;
        offset.currentOffset = cur_addr;
        offset.samples = this->output_buffer[i];
        AXSetVoiceOffsets(this->axvoice[i], &offset);               // input addressing
    }

    this->is_available_ax_voice = TRUE;
    // Register sound handle to sound thread.
    pSoundGlobals->SoundPlayers[this->idx_handle_table] = this;

    OSUnlockMutex(&pSoundGlobals->mutex);

    return TRUE;
}

void VideoSoundWiiU::DestroyOutput(void)
{
    UInt32  chno;

    this->Stop();

    OSLockMutex(&pSoundGlobals->mutex);
    // Detach sound handle to sound thread.
    pSoundGlobals->SoundPlayers[this->idx_handle_table] = NULL;
    this->is_available_ax_voice = FALSE;
    OSUnlockMutex(&pSoundGlobals->mutex);

    // Release all MIX
    for (chno=0; chno<this->nch; chno++) {
        if ( this->axvoice[chno] != NULL ) {
            MIXReleaseChannel(this->axvoice[chno]);
        }
    }

    // Rlease another resources
    this->CleanupOutputResources();

    return;
}

void VideoSoundWiiU::CleanupOutputResources(void)
{
    UInt32  chno, i;

    // Release all AX voice without MIX
    for (chno = 0; chno < this->nch; chno++) {
        if (this->axvoice[chno] != NULL) {
            AXFreeVoice(this->axvoice[chno]);
            this->axvoice[chno] = NULL;
        }
    }

    for (i = 0; i < this->nch; i++) {
        if (this->output_buffer[i] != NULL) {
            SF_FREE(this->output_buffer[i]);
            this->output_buffer[i] = NULL;
        }
    }

    this->nch = 0;
    this->buf_nsmpl = 0;
    this->rp = 0;
    this->wp = 0;
    this->ndata = 0;
    this->total_play_nsmpls = 0;
    this->pause_flag = FALSE;
    this->voice_drop_flag = FALSE;
    this->exec_flag = 0;
    this->samplerate = 0;
    this->vol = 0;
}

void VideoSoundWiiU::SetVolume(float volume)
{
    float vol_norm;
    float vol_db;
    int vol_mb;
    int i;

    // Normalize volume
    if (volume <= 0.0f) {
        vol_norm = 0.0f;
    } else if (volume > 1.0f) {
        vol_norm = 1.0f;
    } else {
        vol_norm = volume;
    };
    this->vol = vol_norm;

    if (vol_norm == 0.0f) {
        vol_mb = -960;
    } else {
        // Convert to dB from volume
        vol_db = (float)(20.0f * log10(vol_norm));
        // Convert to mB from dB for MIX
        vol_mb = (int)(vol_db * 10);
    }

    // Set volume to MIX
    for (i = 0; i < this->nch; i++) {
        // This setting will be updated by MIXUpdateSettings()
        MIXSetInput(this->axvoice[i], vol_mb);
    }

    return;
}

float VideoSoundWiiU::GetVolume(void)
{
    return (this->vol);
}

void VideoSoundWiiU::ExecuteAudioFrame(void)
{
    UInt32 wsmpl;
    UInt32 free_blk1, free_blk2;

    if (this->is_available_ax_voice != TRUE) {
        return;
    }

    OSLockMutex(&this->mutex_sound_hn);
    if ( this->stat == Sound_Stopped ) {
        OSUnlockMutex(&this->mutex_sound_hn);
        return;
    }

    // Update read pointer
    if ( (this->stat == Sound_Playing) && (this->exec_flag == 1) ) {
        UInt32 cpos, tmp;
        UInt32 play_nsmpls;

        tmp = *((UInt32*)&this->axvoice[0]->offsets.samples);
        cpos = (UInt32)tmp - (UInt32)(*((UInt32*)&this->output_buffer[0]));

        if ( cpos >= this->rp ) {
            // |----R*****************C**|
            play_nsmpls = cpos - this->rp;
        } else {
            // |****C------R*************|
            play_nsmpls = this->buf_nsmpl - (this->rp - cpos);
        }
        this->rp = cpos;
        this->ndata -= play_nsmpls;
        this->total_play_nsmpls += play_nsmpls;
    }

    // Calculate total free size
    if (this->rp > this->wp) {
        //    data       blk1     data    
        // |**********W-------R**********|
        free_blk1 = this->rp - this->wp;
        free_blk2 = 0;
    } else {
        //     blk2      data     blk1
        // |----------R*******W----------|
        free_blk1 = this->buf_nsmpl - this->wp;
        free_blk2 = this->rp;
    }

    if ( pPCMStream ) {
        SInt16 *sample[MAX_NCH];
        UInt32 chno;
        UInt32 wsmpl1, wsmpl2;

        // Buffers for Free Block 1 (it is just write pointer)
        for (chno = 0; chno < this->nch; chno++) {
            sample[chno] = &this->output_buffer[chno][this->wp];
        }
        //  Get PCM data to temporary buffer from CRI
        wsmpl1 = this->GetSoundData(this->nch, sample, free_blk1);
        wsmpl = wsmpl1;

        if (wsmpl1 == free_blk1 && free_blk2 > 0) {
            // One more getting PCM for Free Block 2 (it is just top pointer)
            for (chno = 0; chno < this->nch; chno++) {
                sample[chno] = &this->output_buffer[chno][0];
            }
            //  Get PCM data to temporary buffer from CRI
            wsmpl2 = this->GetSoundData(this->nch, sample, free_blk2);
            wsmpl += wsmpl2;
        }

        for (chno=0; chno<this->nch; chno++) {
            DCFlushRange((void*)this->output_buffer[chno], this->size_output_buffer);
        }

    } else {
        UInt32  chno;
        for (chno=0; chno<this->nch; chno++) {
            memset(this->output_buffer[chno], 0, SampleToByte(BUF_NSMPL));
        }
        wsmpl = free_blk1 + free_blk2;
    }

    // Update write pointer
    this->ndata += wsmpl;
    this->wp = (this->wp + wsmpl) % this->buf_nsmpl;

    if ( this->exec_flag == 0 ) {
        if ( this->stat == Sound_Playing ) {
            if ( this->ndata >= BUF_NSMPL ) {
                UInt32  chno;
                for (chno=0; chno<this->nch; chno++) {
                    if ( this->axvoice[chno] != NULL ) {
                        AXSetVoiceState(this->axvoice[chno], AX_PB_STATE_RUN);
                    }
                }
                this->exec_flag = 1;
            }
        }
    }

    OSUnlockMutex(&this->mutex_sound_hn);
}

void VideoSoundWiiU::Start(PCMStream* pstream)
{
    OSLockMutex(&this->mutex_sound_hn);
    this->stat = Sound_Playing;
    this->pause_flag = FALSE;
    this->total_recv_nsmpls = 0;
    this->total_play_nsmpls = 0;
    pPCMStream = pstream;
    OSUnlockMutex(&this->mutex_sound_hn);

    return;
}

void VideoSoundWiiU::Stop(void)
{
    UInt32  chno;

    OSLockMutex(&this->mutex_sound_hn);
    pPCMStream = 0;

    this->stat = Sound_Stopped;
    this->exec_flag = 0;
    for (chno=0; chno<this->nch; chno++) {
        if ( this->axvoice[chno] != NULL ) {
            AXSetVoiceState(this->axvoice[chno], AX_PB_STATE_STOP);
        }
    }
    OSUnlockMutex(&this->mutex_sound_hn);

    this->total_recv_nsmpls = 0;

    return;
}

VideoSoundWiiU::Status VideoSoundWiiU::GetStatus(void) const
{
    if (this->voice_drop_flag == TRUE) {
        return Sound_Error;
    }

    return this->stat;
}

void VideoSoundWiiU::Pause(bool sw)
{
    UInt32 chno;

    if (this->is_available_ax_voice != TRUE) {
        return;
    }

    OSLockMutex(&this->mutex_sound_hn);
    if (this->pause_flag != sw) {
        this->pause_flag = sw;
        if (this->pause_flag == 1) {
            this->stat = Sound_Stopped;
            for (chno=0; chno<this->nch; chno++) {
                if ( this->axvoice[chno] != NULL ) {
                    AXSetVoiceState(this->axvoice[chno], AX_PB_STATE_STOP);
                }
            }
        } else {
            this->stat = Sound_Playing;
            for (chno=0; chno<this->nch; chno++) {
                if ( this->axvoice[chno] != NULL ) {
                    AXSetVoiceState(this->axvoice[chno], AX_PB_STATE_RUN);
                }
            }
        }
    }
    OSUnlockMutex(&this->mutex_sound_hn);

    return;
}

void VideoSoundWiiU::GetTime(UInt64* count, UInt64* unit) const
{
    *count = this->total_play_nsmpls;
    *unit = this->samplerate;

    return;
}

UInt32 VideoSoundWiiU::GetSoundData(UInt32 nch, SInt16 *sample[], UInt32 nsmpl)
{
    UInt32 recv_nsmpl = 0;

    // Mono, Stereo or 5.1ch
    if (pPCMStream) {
        recv_nsmpl = pPCMStream->GetDataSInt16(nch, sample, nsmpl);
    }

    this->total_recv_nsmpls += recv_nsmpl;
    return recv_nsmpl;
}

void* VideoSoundWiiU::VoiceDropCallback(AXVPB* voice)
{
    UInt32  chno;
    VideoSoundWiiU* mvsound = (VideoSoundWiiU*)voice->userContext;

    // Detach sound handle to sound thread.
    pSoundGlobals->SoundPlayers[mvsound->idx_handle_table] = NULL;
    mvsound->is_available_ax_voice = FALSE;

    // Release MIX of dropped voice
    MIXReleaseChannel(voice);
    // Search dropped voice and clear pointer
    for (chno = 0; chno < mvsound->nch; chno++) {
        if (mvsound->axvoice[chno] == voice) {
            mvsound->axvoice[chno] = NULL;
            break;
        }
    }

    // Release another voices at the same time
    for (chno = 0; chno < mvsound->nch; chno++) {
        if (mvsound->axvoice[chno] != NULL) {
            AXSetVoiceState(mvsound->axvoice[chno], AX_PB_STATE_STOP);
            MIXReleaseChannel(mvsound->axvoice[chno]);
            AXFreeVoice(mvsound->axvoice[chno]);
            mvsound->axvoice[chno] = NULL;
        }
    }

    mvsound->voice_drop_flag = TRUE;

    return NULL;
}

//////////////////////////////////////////////////////////////////////////
//
static bool Initialized = 0;

VideoSoundSystemWiiU::VideoSoundSystemWiiU(bool init, MemoryHeap* pheap) :
    VideoSoundSystem(pheap)
{
    if (init && !Initialized)
    {
        Initialized = 1;
        AIInit(NULL);
        AXInit();
        MIXInit();
    }
}

VideoSound* VideoSoundSystemWiiU::Create()
{
    return new VideoSoundWiiU(GetHeap());
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO
