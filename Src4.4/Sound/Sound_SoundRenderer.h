/**************************************************************************

Filename    :   Sound_SoundRenderer.h
Content     :   Sound renderer interface
Created     :   November, 2008
Authors     :   Andrew Reisse, Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#ifndef INC_GSOUNDRENDERER_H
#define INC_GSOUNDRENDERER_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_SOUND

#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Array.h"

#include "Sound_Sound.h"

namespace Scaleform { namespace Sound {

class   SoundRenderer;
class   SoundSample;
class   SoundInfoBase;
class   SoundInfo;

//
// ***** SoundSample
//
// SoundSample represents a SoundRenderer-specific version of a sound that should be
// associated with an instance of SoundDataBase class which loaded from a SWF file
//
// An instance of this call is created by calling to one of the CreateSample... methods
// of SoundRenderer class. To play a sound sample an instance of SoundChannel should be 
// created by calling to PlaySample method of SoundRenderer. There could be multiple 
// instances of SoundChannel class associated with the same instance of SoundSample class
//
// This class has a custom refcount implementation because care should be taken when is
// detached from SoundRenderer in the multi-threaded environment.
//
class SoundSample : public NewOverrideBase<Stat_Sound_Mem>
{
public:
    SoundSample()           { RefCount.Value = 1; }
    virtual ~SoundSample()  {}

    // Obtains the renderer that created this sound sample
    // GetRenderer can return 0 if the renderer has been released, in that
    // case surviving SoundSample object should be just a stub
    virtual SoundRenderer*  GetSoundRenderer() const    = 0;
    virtual bool            IsDataValid() const         = 0;

    // Obtains the sound duration
    virtual float            GetDuration() const        = 0;
    // Returns the size, in bytes
    virtual int             GetBytesTotal() const       = 0;
    // Returns the number of bytes loaded 
    virtual int             GetBytesLoaded() const      = 0;

private:
    AtomicInt<SInt32> RefCount;

public:
    inline void AddRef()
    {
        RefCount.Increment_NoSync();
    }
    inline void Release()
    {
        if ((RefCount.ExchangeAdd_Acquire(-1) - 1) == 0)
            delete this;
    }
    inline bool AddRef_NotZero()
    {
        while (1)
        {
            SInt32 refCount = RefCount;
            if (refCount == 0)
                return 0;
            if (RefCount.CompareAndSet_NoSync(refCount, refCount+1))
                break;
        }
        return 1;
    }
};

//
// ***** SoundChannel
//
// SoundChannel represents an active (playing) sound and is used to control
// its attributes like volume level, status (paused, stopped).
//
// It is created by calling to PlaySample or AttachAuxStreamer methods of 
// SoundRenderer class. 
//
class SoundChannel : public RefCountBase<SoundChannel,Stat_Sound_Mem>
{
public:
    // Sound volume transformation descriptor
    struct Transform
    {
        float Position;     // Position in seconds
        float LeftVolume;   // Volume for left channel (0.0..1.0)
        float RightVolume;  // Volume for right channel (0.0..1.0)
    };

    // Sound spatial position, orientation, velocity, etc.
    struct Vector
    {
        float X, Y, Z;      // 3D vector
        Vector(): X(0.0f), Y(0.0f), Z(0.0f) {}
    };

    SoundChannel() {}

    // Obtains the renderer that created this sound channel
    // GetRenderer can return 0 if this channel has not been associated with any 
    // SoundRenderers (in case if a separate sound system is used to play a sound
    // track from the video file)
    virtual SoundRenderer* GetSoundRenderer() const	                         = 0;
    // Obtains the sound sample that was used to created this sound channel
    // GetSample can return 0 if this channel has not been associated with any 
    // sound samples (in case if a separate sound system is used to play a sound
    // track from the video file)
    virtual SoundSample*   GetSample() const                                 = 0;

    // Stop the active sound. After calling to this method calls to all methods of 
    // object should not changed the state and attributes of this object.
    virtual void           Stop()                                            = 0;
    // Pause/resume the active sound
    virtual void           Pause(bool pause)                                 = 0;
    // Check if the active sound has been stopped
    virtual bool           IsPlaying() const                                 = 0;
    // Set the playing position in seconds
    virtual void           SetPosition(float seconds)                        = 0;
    // Get the playing position of the active sound 
    virtual float          GetPosition()                                     = 0;
    // Set the sound's loop attributes (number of loops, start and end position in seconds)
    virtual void           Loop(int count, float start = 0, float end = 0)   = 0;
    // Get the current volume level (0.0 ... 1.0)
    virtual float          GetVolume()                                       = 0;
    // Set the current volume level (0.0 ... 1.0)
    virtual void           SetVolume(float volume)                           = 0;
    // Get the current pan (-1.0 ... 0 ... 1.0 )
    virtual float          GetPan()                                          = 0;
    // Set the current pan (-1.0 ... 0 ... 1.0)
    virtual void           SetPan(float volume)                              = 0;
    // Set a volume level transformation vector 
    virtual void           SetTransforms(const Array<Transform>& transforms) = 0;
    // Sets sound spatial information such as position, orientation and velocity
    virtual void           SetSpatialInfo(const Array<Vector> spatinfo[])    = 0;
};

//
// ***** SoundRenderer
//
// SoundRenderer is a base sound renderer class used for playing all sounds in 
// a SWF file; it is essentially a low level interface to the hardware.
//
class SoundRenderer : public RefCountBase<SoundRenderer,Stat_Sound_Mem>
{
public:

    enum SoundRendererCapBits
    {
        Cap_NoMP3        = 0x00000001,  // SoundRenderer CAN NOT play MP3 compressed sound data
        Cap_NoVideoSound = 0x00000002,  // SoundRenderer DOES NOT support video sound playback
        Cap_NoStreaming  = 0x00000004   // SoundRenderer DOES NOT support streaming sound
    };

    // This interface is used to access the audio for a video source
    class AuxStreamer : public RefCountBase<AuxStreamer,Stat_Sound_Mem>
    {
    public:
        // Specifies the format of PCM data which a streamer will be providing.
        enum PCMFormat
        {
            PCM_SInt16,
            PCM_Float
        };
        virtual ~AuxStreamer() {}
        // This method is called then needs the next chunk of audio data from
        // a video source.
        virtual unsigned GetPCMData(UByte* pdata, unsigned datasize) = 0;
        // Set the position (in seconds) in the sound streamer
        virtual bool SetPosition(float /*seconds*/) { return false; }       
    };

    SoundRenderer() { }
    virtual ~SoundRenderer() {}

    // Helper function to query sound renderer capabilities.
    virtual bool            GetRenderCaps(UInt32 *pcaps)                = 0;

    // Returns created objects with a refCount of 1, must be user-released. 
    // The created sound object is associated with the sound data which
    // comes from a separate file (mp3, wav). 
    // streaming parameters specifies if a file should be completely loaded
    // before it starts playing.
    virtual SoundSample*    CreateSampleFromFile(const char* fname, bool streaming) = 0;

    // Convenience creation functions, create & initialize sample at the same time
    // The created sound sample object is associated with the sound data which is 
    // kept in SWF resource library in was loaded during swf file loading process 
    virtual SoundSample*    CreateSampleFromData(SoundDataBase* psd)    = 0;

    // Returns created objects with a refCount of 1, must be user-released. 
    // The created sound associated with the audio stream of a video source. 
    virtual SoundSample*    CreateSampleFromAuxStreamer(
        AuxStreamer*           /* pStreamder */,
        UInt32                 /* channels */,
        UInt32                 /* samplerate */,
        AuxStreamer::PCMFormat /* fmt */) { return NULL; }

    // Create a sound channel object form a given sound sample object.
    // if paused parameter is set to false the sound will not start playing
    // until GSoundChannel::Paused(false) method is called.
    virtual SoundChannel*  PlaySample(SoundSample* ps, bool paused)     = 0;

    // Get the master volume level for the sound renderer
    virtual float           GetMasterVolume()                           = 0;
    // Set the master volume level for all sound created by this renderer
    virtual void            SetMasterVolume(float volume)               = 0;
    
    // Updates internal sound renderer systems. This method should be called periodically
    // in the game main loop
    // Returns time remaining till next Update must be called, in seconds.
    virtual float           Update()                                    = 0;
    // Mutes/unmutes all sounds playing by the sound renderer. 
    virtual void            Mute(bool mute)                             = 0;
};

//
// ***** SoundInfoBase
//
// SoundInfoBase represents a sound within GFxPlayer
//
class SoundInfoBase : public RefCountBaseNTS<SoundInfoBase,Stat_Sound_Mem>
{
public:
    virtual unsigned    GetLength() const                           = 0;
    virtual unsigned    GetRate() const                             = 0;

    // Obtains the SoundSample pointer from the data,
    // for a given sound renderer. Optionally create the SoundSample
    virtual SoundSample* GetSoundSample(SoundRenderer* prenderer)   = 0;

    // SetSoundData is a part of the interface but it does not need to be implemented
    // in versions of SoundInfoBase which do not support initialization with a SoundData.
    // If supported, the class can AddRef to the Sound to keep its data.
    // The default '.gfx' file loading procedure will use this method to
    // initialize Sound contents after the base file has been loaded.
    virtual bool        SetSoundData(SoundDataBase* pSound) { SF_UNUSED(pSound); return 0; }

    // Returns true, if the instance can be played.
    virtual bool        IsPlayable() const { return false; }

    virtual void        ReleaseResource()                           = 0;
};

//
// ***** SoundInfo
//
// SoundInfoBase version that keeps sound sample data within allocated GSoundData object.
//
class SoundInfo : public SoundInfoBase
{
protected:
    // Source sound data for the SoundSample
    Ptr<SoundDataBase>  pSound;
    Ptr<SoundSample>    pSoundSample;

public:
    explicit SoundInfo(SoundDataBase* psound = 0) : pSound(psound) {}
    virtual ~SoundInfo();

    // Simple accessors
    SoundDataBase*  GetSound() const { return pSound; }

    // SoundInfoBase implementation
    unsigned        GetLength() const { return pSound->GetSampleCount(); }
    unsigned        GetRate() const; 

    SoundSample*    GetSoundSample(SoundRenderer* prenderer);    
    bool            SetSoundData(SoundDataBase* pSound);

    virtual bool    IsPlayable() const { return pSound.GetPtr() != 0; }

    virtual void    ReleaseResource();
};

}} // Scaleform::Sound

#endif // GFX_ENABLE_SOUND

#endif
