/**************************************************************************

Filename    :   Sound_Sound.h
Content     :   Sound samples
Created     :   November, 2008
Authors     :   Andrew Reisse, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SOUND_H
#define INC_SOUND_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_SOUND

#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"

namespace Scaleform { namespace Sound {

// 
// ***** SoundDataBase
//
// The base class for representing sound data in GFx.
// This class does not hold any sound data nor it provides a pointer
// to the sound data because its derivatives can hold only a file name of
// a file with sound data
//
class SoundDataBase : public RefCountBaseNTS<SoundDataBase,Stat_Sound_Mem>
{
public:
    enum SampleFormat
    {
        Sample_None     = 0,
        Sample_8        = 0x001,  // each sample takes 8 bit
        Sample_16       = 0x002,  // each sample takes 16 bit
        Sample_Stereo   = 0x008,  // stereo sound data
        Sample_PCM      = 0x100,  // uncompressed PCM sound data 
        Sample_MP3      = 0x200,  // compressed MP3 sound data
        Sample_ADPCM    = 0x300,  // compressed ADPCM sound data
        Sample_Format   = 0x700,
        Sample_Stream   = 0x1000, // streaming sound data
        Sample_File     = 0x2000  // sound data comes from a file
    };

    SoundDataBase(unsigned format, unsigned rate, unsigned length)
        : Format(format), Rate(rate), Length(length), SeekSample(0) {}

    inline unsigned GetSampleSize() const      { return (Format & 0x7) * ((Format & Sample_Stereo) ? 2 : 1); }
    inline bool     IsDataCompressed() const   { return Format >= Sample_MP3; }
    inline unsigned GetFormat() const          { return Format; }
    inline unsigned GetChannelNumber() const   { return Format & Sample_Stereo ? 2 : 1; }

    inline unsigned GetRate() const            { return Rate; }
    inline unsigned GetRawSize() const         { return Length * GetSampleSize(); }
    inline float    GetDuration() const        { return float(Length) / float(Rate); }

    inline bool     IsStreamSample() const     { return (Format & Sample_Stream) != 0; }
    inline bool     IsFileSample() const       { return (Format & Sample_File) != 0; }
    inline unsigned GetSampleCount() const     { return Length; }

    inline void     SetSeekSample(unsigned sample) { SeekSample = sample; }
    inline unsigned GetSeekSample() const          { return SeekSample; }

protected:
    unsigned    Format;     // format flags
    unsigned    Rate;       // sample rate
    unsigned    Length;     // number of samples
    unsigned    SeekSample; // number of samples that should be skipped from the beginning 
                            // of the sound data when it stars playing
};

//
// ***** SoundData class
//
// This class holds sound data that was loaded from a SWF file
// It is used for SWF event sound where data is loaded all at once.
//
class SoundData : public SoundDataBase
{
public:
    SoundData(unsigned format, unsigned rate, unsigned lenght, unsigned dsize);
    ~SoundData();

    UByte*      GetData() const     { return pData; }
    unsigned    GetDataSize() const { return DataSize; }

protected:
    UByte*      pData;
    unsigned    DataSize;
};

//
// ***** AppendableSoundData class
//
// This class is used for representing streaming sound data from a SWF file
// This type of a SWF sound is not loaded all at once. It comes in chunks during 
// loading of SWF frames. Each SWF frame has its own sound data chunk.
//
class AppendableSoundData : public SoundDataBase
{
public:
    AppendableSoundData(unsigned format, unsigned rate);
    ~AppendableSoundData();

    UByte*      LockDataForAppend(unsigned length, unsigned dsize);
    void        UnlockData();

    unsigned    GetData(UByte* buff, unsigned dsize);
    bool        SeekPos(unsigned pos);
    unsigned    GetDataSize() const { return DataSize; }

protected:
    struct DataChunk : public NewOverrideBase<Stat_Sound_Mem>
    {
        DataChunk(unsigned capacity);
        ~DataChunk();

        DataChunk* pNext;
        UByte*     pData;
        unsigned   DataSize;
        unsigned   StartSample;
        unsigned   SampleCount;
    };

    DataChunk*  pFirstChunk;
    DataChunk*  pFillChunk;
    DataChunk*  pReadChunk;
    unsigned    DataSize;
    unsigned    ReadPos;
    Lock        ChunkLock;
};

//
// ***** SoundFile class
//
// This class sound data that is referenced by a file name
// It is used for loading sound data that was extracted from a SWF file
// by gfxexport utility.
//
class SoundFile : public SoundDataBase
{
public:
    SoundFile(const char* fname, unsigned rate, unsigned length, bool streaming);
    ~SoundFile();

    const char* GetFileName() const { return pFileName; }

protected:
    char* pFileName;
};

}} // Scaleform::Sound

#endif // GFX_ENABLE_SOUND

#endif // INC_SOUND_H
