/**************************************************************************

Filename    :   Video_Video.h
Content     :   Main GFx video classes declaration
Created     :   October 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_VIDEO_H
#define INC_GFX_VIDEO_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Kernel/SF_Types.h"
#include "Kernel/SF_Memory.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_Threads.h"
#include "Sound/Sound_SoundRenderer.h"
#include "Render/Render_Image.h"

#include "GFx/GFx_Loader.h"
#include "GFx/GFx_VideoBase.h"

namespace Scaleform {

#ifdef GFX_ENABLE_SOUND
namespace Sound {
class SoundChannel;
}
#endif

namespace GFx {

class TaskManager;
class FileOpenerBase;

namespace Video {

class VideoSound;
class VideoImage;

//////////////////////////////////////////////////////////////////////////
//

// VideoPlayer is an internal video playback interface returned by Video::CreateVideoPlayer.
// End users do not need to create or access objects this of this class.

class VideoPlayer : public NewOverrideBase<Stat_Video_Mem>
{
protected:
    virtual ~VideoPlayer() {}

public:
    enum CuePointType
    {
        EventCuePoint,
        NavigationCuePoint
    };

    struct CuePointParam
    {
        String Name;
        String Value;
    };

    struct CuePoint
    {
        CuePointType          Type;
        String                Name;
        UInt32                Time;
        Array<CuePointParam>  Params;
    };

    struct AudioTrackInfo
    {
        int       Index;             // Index of a audio track
        UInt32    SampleRate;        // Sample rate
        UInt32    ChannelsNumber;    // Number of channels in the track (1-mono, 2-stereo, 6-5.1)
        UInt32    TotalSamples;      // total number of channels
    };

    struct VideoInfo
    {
        int                      Width;                     // Width of the video data
        int                      Height;                    // Height of the video data
        UInt32                   FrameRate;                 // Video bit rate
        UInt32                   AudioDataRate;             // Audio sample rate
        UInt32                   TotalFrames;               // Total number of frames of video data
        Array<CuePoint>          CuePoints;                 // List of cue points encoded into the video file
        int                      SubtitleChannelsNumber;    // Total number of subtitle channels
        int                      MaxSubtitleLen;            // Maximum size of a subtitle string
        Array<AudioTrackInfo>    AudioTracks;               // Audio tracks information
    };

    enum Status
    {
        NotInitialized, // Open method has never been called on this object
        Opening,        // Open method is called. Player is getting ready for a movie playing
        Ready,          // Play method can be called on this object to play the loaded movie
        Starting,       // Play method has been called, but the first frame is not received yet
        Playing,        // The loaded movie is playing
        Seeking,        // Seek method has been called
        Stopping,       // Stop method has been called
        Finished,       // The playback reached the last frame of the movie
        Stopped,        // The movie is stopped by calling Stop method
        FileNotFound,   // Requested file is not found
        InternalError   // Internal player error
    };

    // Synchronization object
    class SyncObject :  public RefCountBaseNTS<SyncObject, Stat_Video_Mem>
    {
    public:
        virtual ~SyncObject() {}

        virtual void SetStartFrame(unsigned frame) = 0;
        virtual void Start()                       = 0;
        virtual void Stop()                        = 0;
        virtual void Pause(bool sw)                = 0;

        // Returns the user's controlled time, based on which the video playback will be 
        // synchronized. The time starts out at 0 before Start is called and increases as data
        // is played by the stream. The "count / unit" value indicates time playback
        // in seconds. If the 'unit' is reported as equal to the sample rate, the 'count'
        // should indicate the number of samples played so far.
        virtual void GetTime(UInt64* count, UInt64* unit) = 0;
    };

    // Read Buffer Information
    struct ReadBufferInfo
    {
        UInt32 BufferSize;         // Read Buffer Size
        UInt32 DataSize;           // Data size in the buffer
        UInt32 ReloadThreshold;    // Reload threshold. When DataSize is less the ReloadThreshold
                                   // the next read operation is accrued
    };

    // Destroy this object
    virtual void      Destroy() { delete this; }

    // Open a movie. After the call to this method the movie is not ready to be played.
    // GetStatus() method should be called to obtain the movie status. To start a movie
    // playback Play() method should be called after the status transitioned to Ready state.
    virtual void      Open(const char* url) = 0;

    // Return a movie status.
    virtual Status    GetStatus() = 0;

    // Return a movie's parameters. This method can only be called after the status
    // transitioned to Ready status.
    virtual void      GetVideoInfo(VideoInfo* info) = 0;

    // Start movie playback. This method can only be called after the status
    // transitioned to Ready status.
    virtual void      Play() = 0;

    // Go to specified position. This method can only be called after the status
    // transitioned to Ready status.
    virtual void      Seek(UInt32 pos) = 0;

    // Set the subtitle channel number to play. -1 turn off subtitle
    virtual void      SetSubtitleChannel(int channel) = 0;

    // Get the subtitle channel number. -1 means subtitles are not playing
    virtual int       GetSubtitleChannel() = 0;

    // Returns current playback position (in video frames)
    virtual UInt32    GetPosition() = 0;

    // Create a texture image. This method can only be called after the status transitioned to Ready status
    virtual VideoImage* CreateTexture(Render::TextureManager* ptexman) = 0;

    // Update the texture image (which was create by CreateTexture method) if the next frame is ready
    virtual void      UpdateTexture(VideoImage* pimage, char* subtitle, int subtitleLength) = 0;

    // Is video frame decoded and ready
    virtual bool      IsTextureReady() = 0;

    // Perform decoding
    virtual void      Decode() = 0;

    // Stop playback
    virtual void      Stop() = 0;

    // Pause playback
    virtual void      Pause(bool on_off) = 0;

    // Set video synchronization object
    virtual void      SetSyncObject(SyncObject*) = 0;

    // Retrieve cue points that were triggered since the last call to this method
    virtual void      GetCurrentCuePoints(Array<CuePoint>* cuePoints) = 0;

    // When the movie file has multiple audio tracks (e.g., an English version and
    // a Spanish version), this function sets the audio track to play audio.
    virtual void      SetAudioTrack(int trackIndex) = 0;

    // A secondary, or SubAudio, track is typically used to play a dialog track or
    // sound effects along with a movie.
    virtual void      SetSubAudioTrack(int trackIndex) = 0;

    // Extra audio track, similar to SubAudio track
    virtual void      SetExtraAudioTrack(int trackIndex) = 0;

    // Replaces the center track of 5.1ch audio.
    // This function sets the center channel for replacement.
    // Use this only to change the voice track of 5.1ch music.
    virtual void      ReplaceCenterVoice(int trackIndex) = 0;

    // Sets a looping flag. If set to true the player will not stop 
    // until Stop() method is called
    virtual void      SetLoopFlag(bool flag) = 0;

    // Set the buffering time of streaming
    virtual void      SetBufferTime(float time) = 0;

    // Set the number of internal video buffers
    virtual void      SetNumberOfFramePools(unsigned pools) = 0;

    // Set the re-load timing
    virtual void      SetReloadThresholdTime(float time) = 0;

    // Get input buffer information
    virtual void      GetReadBufferInfo(ReadBufferInfo* info) = 0;

    // Set decoding header timeout in sec
    virtual void      SetDecodeHeaderTimeout(float timeout) = 0;

    // Reads a video file header without playing the file
    static bool SF_CDECL LoadVideoInfo(const char* pfilename, VideoInfo* info, FileOpenerBase* popener);

#ifdef GFX_ENABLE_SOUND
    enum SoundTrack
    {
        MainTrack,  // Main sound track
        SubAudio,   // A secondary, or SubAudio, track
        ExtraAudio  // Extra audio track
    };
    virtual Sound::SoundChannel*
                      GetSoundChannel(SoundTrack track = MainTrack) = 0;
    virtual void      SetSoundSpatialInfo(Array<Sound::SoundChannel::Vector> spatinfo[]) = 0;
#endif
};


//////////////////////////////////////////////////////////////////////////
//

// VideoSoundSystem is an abstract interface providing sound support for Video playback;
// developers can substitute video sound implementation by making their own version of this
// class. Before playing video, an instance of this class needs to be created and installed
// with Video::SetSoundSystem. Typically, a platform-specific sample implementation such as
// VideoSoundSystemDX8 or VideoSoundSystemWii can be used to avoid implementing this interface.

// In the current version of GFx, video sound support is decoupled from the embedded Flash
// sound playback, allowing video to be used without requiring the general sound engine.
// To make this work, video is supported by an independent VideoSoundSystem class that is
// separate from SoundRenderer used in the rest of GFx. This means that to get video sound
// support you only need to implement VideoSoundSystem and VideoSound classes, which are much
// simpler then SoundRenderer. Note that if you already have SoundRenderer implementation,
// you can use it directly to initialize Video, as it provides a superset of functionality.
// In some cases you can also mix two implementations (helpful if a custom video sound class
// provides better streaming support then the general sound engine).
// 
// To have video sound support you need to implement two classes: VideoSoundSystem and
// VideoSound. Typically there is only one instance of VideoSoundSystem installed during video
// initialization. VideoSoundSystem exposes a single method, Create, used to create VideoSound
// objects representing independent video sound streams. Scaleform GFx will call this function
// every time a new video is opened; there can be multiple videos playing at the same time.
//
// After each VideoSound object is created, GFx will call its various functions to instruct
// it to Start and Stop audio output. The actual sound data for the stream is obtained through
// polling of the VideoSound::PCMStream passed to the given sound. Polling is typically done
// by a separate thread maintained by the VideoSoundSystem to service its active sounds.

class VideoSoundSystem : public RefCountBaseNTS<VideoSoundSystem,Stat_Video_Mem>
{
public:
    // Creates a new VideoSoundSystem. Receives a pointer to the memory
    // heap that should be used for all of the internal memory allocations.
    // The memory heap passed is guaranteed to be thread safe.
    VideoSoundSystem(MemoryHeap* pheap) : pHeap(pheap) {}
    virtual ~VideoSoundSystem() {}

    // Creates a VideoSound stream object for a particular video instance; returns
    // null in case of failure. After sound is created, GFx will call its CreateOutput,
    // Start, Stop and other methods to it when it can start polling for data.    
    virtual class VideoSound* Create() = 0;

    virtual void Update() {}

    // Get a pointer to the memory heap used for video sound system allocations.
    MemoryHeap* GetHeap() const { return pHeap; }

protected:
    MemoryHeap* pHeap;
};


//////////////////////////////////////////////////////////////////////////
//

// VideoSound object represents a stream of audio data for a single video file playing within GFx.
// VideoSound objects are created by the VideoSoundSystem::Create function; they are further
// initialized by CreateOutput. After creation, GFx calls Start, Stop, Pause and other functions
// of the sound object to tell it when to start and stop streaming data.
// 
// VideoSound receives its PCM data by polling it from the video player through the PCMStream
// interface passed to the Start function. PCM data can be requested in either 16-bit integer
// or 32-bit floating point format; users specify the desired format by implementing the
// GetPCMFormat function. The actual polling process typically takes place on a separate thread
// created by the sound or VideoSoundSystem.
// 
// The following is a detailed sequence of events that takes place with VideoSound object:
//   1. VideoSound is created by VideoSoundSystem::Create.
//   2. CreateOutput is called to initialize the sound stream, specifying
//      the number of channels and the sample rate.
//   3. Start is called to begin audio streaming. At this point, VideoSound
//      implementation should begin polling PCMStream for audio data.
//   4. Pause is called to temporarily stop and resume playback without losing
//      position within the audio stream.
//   5. Stop completes sound playback; after this function is called PCMStream
//      should no longer be polled.
//   6. DestoyOutput shuts down the sound channel.
//   7. VideoSound destructor is called at some point later on once its
//      reference count goes down to zero.
//
// Throughout its lifetime, VideoSound is responsible for maintaining its Status state
// and reporting it properly from the GetStatus() function. The status starts out in 
// Sound_Stopped state and transitions to Sound_Playing when Start is called.
// The other status states include Sound_Paused and Sound_Error.

class VideoSound : public RefCountBaseNTS<VideoSound,Stat_Video_Mem>
{
public:  
    // Specifies the format of PCM data supported by this VideoSound implementation.
    // A value of this type is returned by GetPCMFormat().
    enum PCMFormat
    {
        PCM_SInt16,
        PCM_Float
    };

    // Describes the current status of the VideoSound object. Implementation
    // should properly transition the status within Start, Stop and Pause calls.
    enum Status
    {
        Sound_Stopped,  // Sound module doesn't output the sound of the video.
        Sound_Playing,  // Sound module is running to output the sound of the video.
        Sound_Paused,   // Sound playback is paused.
        Sound_Error     // An error has occurred.
    };
    
    enum {
        // Maxim number of channels that can be requested for output.
        MaxChannels = 8
    };

    // PCMStream is an abstract interface used by VideoSound implementation to pull
    // sound data from the video player; it is passed as an argument to the Start()
    // function. VideoSound implementor should request sound data through this interface
    // until the Stop() function is called.
    class PCMStream
    {
    public:        
        virtual ~PCMStream() {}

        // Obtains sound data in SInt16 format. This function should only be called
        // if VideoSound::GetPCMFormat() returns PCM_SInt16. The 
        virtual UInt32  GetDataSInt16(UInt32 nch, SInt16 *pcmbuf[], UInt32 samplesRequested) = 0;
        // Obtains sound data in Float format. This function should only be called
        // if VideoSound::GetPCMFormat() returns PCM_Float.
        virtual UInt32  GetDataFloat(UInt32 nch, float *pcmbuf[], UInt32 samplesRequested)   = 0;
    };

    virtual ~VideoSound() {}

    // Initializes sound output to the specified number of channels and sample rate.
    virtual bool           CreateOutput(UInt32 channels, UInt32 samplerate) = 0;
    // Shuts down sound output. Internal hardware sound buffers can be destroyed during
    // this call.
    virtual void           DestroyOutput()                                  = 0;

    // Returns the PCM data format expected by VideoSound implementation. Data of
    // appropriate format is obtained through the PCMStream::GetDataSInt16 and
    // PCMStream::GetDataFloat functions.
    virtual PCMFormat      GetPCMFormat() const                             = 0;

    // Tells VideoSound to begin sound output by polling PCM data from 'pstream'.
    // For success, state returned by GetStatus should transition to Sound_Playing.
    virtual void           Start(PCMStream *pstream)                        = 0;
    // Tells video sound to stop sound output. PCMStream should not be polled
    // for data after this call.
    virtual void           Stop()                                           = 0;
    // Pauses or resumes sound streaming, temporarily halting playback and PCM polling.
    // Sound streaming should be paused if 'paused' argument value is true, it should
    // be resumed otherwise.
    virtual void           Pause(bool paused)                               = 0;
    
    // Returns the current playback Status.
    virtual Status         GetStatus() const                                = 0;

    // Returns the time of user audio playback from the stream, in user specified
    // units. The time starts out at 0 before Start is called and increases as data
    // is played by the stream. The "count / unit" value indicates time playback
    // in seconds. If the 'unit' is reported as equal to the sample rate, the 'count'
    // should indicate the number of samples played so far.
    virtual void           GetTime(UInt64* count, UInt64* unit) const       = 0;

#ifdef GFX_ENABLE_SOUND
    virtual Sound::SoundChannel* GetSoundChannel()                          = 0;
    virtual void SetSpatialInfo(const Array<Sound::SoundChannel::Vector> spatinfo[])
                                                                            = 0;
#endif
};


//////////////////////////////////////////////////////////////////////////
//

// VideoDecoder is an abstract interface which allows to change the way of how 
// the decoding function is performed. 
class VideoDecoder : public RefCountBaseNTS<VideoDecoder,Stat_Video_Mem>
{
public:
    // Add a video player for the decoding queue
    virtual void StartDecoding(VideoPlayer*)     = 0;

    // Request to stop decoding of a specified video player
    virtual void StopDecoding(VideoPlayer*)      = 0;

    // Check if a specified video player is still in the decoding queue
    virtual bool IsDecodingStopped(VideoPlayer*) = 0;

    // Execute decoding of a specified video player, 
    // return false if the decoding finished for a given player
    virtual bool ExecuteDecode(VideoPlayer*)     = 0;

    // Get input buffer information for all video players
    virtual void GetReadBufferInfo(Array<VideoPlayer::ReadBufferInfo>&) = 0;

    // Check if a video system need to perform file read operation
    virtual bool IsIORequired() const            = 0;

    // Pause/resume the entire decoding queue
    virtual void PauseDecoding(bool)             = 0;

#ifdef SF_OS_XBOX360
    virtual void SetUsableProcessors(int) {}
#endif
};


//////////////////////////////////////////////////////////////////////////
//

// VideoReaderConfig is used to create and initialize instances of VideoReader
// interface which are used to access data from video files.

class VideoReader;

class VideoReaderConfig : public RefCountBaseNTS<VideoReaderConfig,Stat_Video_Mem>
{
public:
    typedef VideoBase::ReadCallback ReadCallback;

    VideoReaderConfig(MemoryHeap* pheap, FileOpenerBase* pfileopener) :
        pHeap(pheap), pFileOpener(pfileopener), IOEnabled(true) {}

    MemoryHeap*             GetHeap()       { return pHeap; }
    FileOpenerBase*         GetFileOpener() { return pFileOpener; }

    virtual VideoReader*    CreateReader() = 0;

    virtual void            SetReadCallback(ReadCallback*) {}
#ifdef SF_OS_XBOX360
    virtual void            SetUsableProcessors(int) {}
#endif

    virtual void            DetachReader(VideoReader*);
    void                    AttachReader(VideoReader*);

    virtual void            EnableIO(bool enable) { IOEnabled = enable; }
    bool                    IsIOEnabled() const   { return IOEnabled; }

protected:
    MemoryHeap*             pHeap;
    Ptr<FileOpenerBase>     pFileOpener;
    Array<VideoReader*>     Readers;    
    volatile  bool          IOEnabled;
};


//////////////////////////////////////////////////////////////////////////
//

// Video ActionScript VM support: AS2, AS3 or both

struct VideoVMSupport
{
    AS2VideoSupport* pAS2VSupport;
    AS3VideoSupport* pAS3VSupport;

protected:
    VideoVMSupport(AS2VideoSupport* pas2, AS3VideoSupport* pas3) 
        : pAS2VSupport(pas2), pAS3VSupport(pas3) {}
};

struct VideoVMSupportAS2 : public VideoVMSupport
{
#ifdef GFX_AS2_SUPPORT
    VideoVMSupportAS2() : VideoVMSupport(AS2VideoSupport::CreateInstance(), 0) {}
#else
    VideoVMSupportAS2() : VideoVMSupport(0, 0) {}
#endif
};

struct VideoVMSupportAS3 : public VideoVMSupport
{
#ifdef GFX_AS3_SUPPORT
    VideoVMSupportAS3() : VideoVMSupport(0, AS3VideoSupport::CreateInstance()) {}
#else
    VideoVMSupportAS3() : VideoVMSupport(0, 0) {}
#endif
};

struct VideoVMSupportAll : public VideoVMSupport
{
#if defined(GFX_AS2_SUPPORT) && defined(GFX_AS3_SUPPORT)
    VideoVMSupportAll() : VideoVMSupport(AS2VideoSupport::CreateInstance(), 
                                         AS3VideoSupport::CreateInstance()) {}
#elif defined(GFX_AS2_SUPPORT)
    VideoVMSupportAll() : VideoVMSupport(AS2VideoSupport::CreateInstance(), 0) {}
#elif defined(GFX_AS3_SUPPORT)
    VideoVMSupportAll() : VideoVMSupport(0, AS3VideoSupport::CreateInstance()) {}
#else
    VideoVMSupportAll() : VideoVMSupport(0, 0) {}
#endif
};


//////////////////////////////////////////////////////////////////////////
//

// Video state is used to initialize video playback in GFx; an instance of this object
// should be created at startup time and set on the loader with Loader::SetVideo. If this
// object is not created, video support will not be linked into your application.
//  
// As a part of video initialization, users need to call SetSoundSystem to initialize video
// sound output. The provided sound system object can be either a platform-specific version
// of the sound system, such as VideoSoundSystemXA2 (for XBox360), or a SoundRender
// implementation that implements SoundRenderer::AuxStreamer.

class Video : public VideoBase
{
public:
    Video(const VideoVMSupport& vmSupport,
          Thread::ThreadPriority decodeThreadPriority = Thread::NormalPriority,
          bool autoInit = true);
    ~Video();

    // AS interfaces access
    virtual AS2VideoSupport* GetAS2Support() const { return pAS2VSupport; }
    virtual AS3VideoSupport* GetAS3Support() const { return pAS3VSupport; }

    // Create an instance of a video player
    virtual VideoPlayer*    CreateVideoPlayer(MemoryHeap*, TaskManager*, FileOpenerBase*, Log*);

    // Set a read call back instance to the video system
    virtual void            SetReadCallback(ReadCallback*);

    // Query a video system if it needs to perform any data read operation
    virtual bool            IsIORequired() const;
    // Enable/Disable video read operations
    virtual void            EnableIO(bool enable);

    // SWF file video tags reader
    virtual void            ReadDefineVideoStreamTag(LoadProcess* p, const TagInfo& tagInfo);

    // This method(s) can be called during video initialization and be used
    // to apply some system specific settings for player, decoder and reader
    virtual void            ApplySystemSettings(VideoPlayer*) {}
    virtual void            ApplySystemSettings(VideoDecoder*) {}
    virtual void            ApplySystemSettings(VideoReaderConfig*) {}

    // Texture manager used to create VideoImage
    void                    SetTextureManager(Render::TextureManager* ptexman) { pTextureManager = ptexman; }
    virtual Render::TextureManager* GetTextureManager() const { return pTextureManager; }

    // Memory heap used by video system
    virtual MemoryHeap*     GetHeap() const { return pHeap; }

    // SoundRenderer and VideoSoundSystem based sound
#ifdef GFX_ENABLE_SOUND
    void                    SetSoundSystem(Sound::SoundRenderer* psoundRenderer);
#endif
    void                    SetSoundSystem(VideoSoundSystem* psoundSystem) { pSoundSystem = psoundSystem; }
    VideoSoundSystem*       GetSoundSystem() const { return pSoundSystem; }

    // Decoder and reader
    VideoDecoder*           GetDecoder()      { return pDecoder; }
    VideoReaderConfig*      GetReaderConfig() { return pReaderConfig; }

protected:
    static void             Initialize(bool argbInit = false);
    static void             Finalize();

    static unsigned         Initialized;

    // GFx supports playing multiple videos at the same time. However, there is a limitation
    // of the maximum number of video handles that are available at a time. In general, each
    // video will use one handle and there is an upper limit (VIDEO_HANDLE_MAX) by default.
    // Note: alpha video playback uses two handles.
#ifndef SF_OS_WII
    static const UInt32     VIDEO_HANDLE_MAX = 8;
#else
    static const UInt32     VIDEO_HANDLE_MAX = 4;
#endif
    static void*            pHandleWorkBuf;
    static UInt32           HandleWorkSize;

    Ptr<MemoryHeap>         pHeap;
    Ptr<Render::TextureManager> pTextureManager;

    Ptr<VideoDecoder>       pDecoder;
    Ptr<VideoReaderConfig>  pReaderConfig;
    Ptr<VideoSoundSystem>   pSoundSystem;
    Ptr<ReadCallback>       pReadCallback;
    Thread::ThreadPriority  DecodeThreadPriority;

    AS2VideoSupport*        pAS2VSupport;
    AS3VideoSupport*        pAS3VSupport;
};

}}} // namespace Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

#endif // INC_GFX_VIDEO_H
