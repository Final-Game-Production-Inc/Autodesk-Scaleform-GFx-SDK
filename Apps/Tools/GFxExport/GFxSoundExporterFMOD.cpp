/**************************************************************************

Filename    :   GFxSoundExporterFMOD.cpp
Content     :   SWF to GFX resource extraction and conversion tool
Created     :   September, 2008
Authors     :   Maxim Didenko

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#define GFX_EXPORT_MAJOR_VERSION    1
#define GFX_EXPORT_MINOR_VERSION    11
#define GFX_EXPORT_VERSION (((GFX_EXPORT_MAJOR_VERSION)<<8)|(GFX_EXPORT_MINOR_VERSION))

#include "GFxExport.h"
#ifdef GFX_ENABLE_SOUND

#include "Kernel/SF_SysFile.h"
#include "Kernel/SF_MsgFormat.h"
using namespace Scaleform;

#include "fmod.hpp"
#include "fmod_errors.h"


#define CHUNKSIZE 4096

FMOD_RESULT F_CALLBACK DecodeOpen(const char *sd, int unicode, unsigned int *filesize, void **handle, void **userdata)
{
    SF_UNUSED2(unicode, handle);
    Sound::AppendableSoundData* psd = (Sound::AppendableSoundData*)sd;
    psd->SeekPos(0);
    *userdata = psd;
    *filesize = 0x0FFFFFFF;
    return FMOD_OK;
}
FMOD_RESULT F_CALLBACK DecodeClose(void *handle, void *userdata)
{
    SF_UNUSED2(userdata, handle);
    Sound::AppendableSoundData* psd = (Sound::AppendableSoundData*)userdata;
    psd->SeekPos(0);
    return FMOD_OK;
}
FMOD_RESULT F_CALLBACK DecodeRead(void *handle, void *buffer, unsigned int sizebytes, unsigned int *bytesread, void *userdata)
{
    SF_UNUSED(handle);
    Sound::AppendableSoundData* psd = (Sound::AppendableSoundData*)userdata;
    *bytesread = psd->GetData((UByte*)buffer, sizebytes);
    return FMOD_OK;
}
FMOD_RESULT F_CALLBACK DecodeSeek(void *handle, unsigned int pos, void *userdata)
{
    SF_UNUSED(handle);
    Sound::AppendableSoundData* psd = (Sound::AppendableSoundData*)userdata;
    if (!psd->SeekPos(pos))
        return FMOD_ERR_FILE_COULDNOTSEEK;
    return FMOD_OK;
}

class GFxSoundExporterFMOD : public GFxSoundExporter
{
public:
    GFxSoundExporterFMOD(FMOD::System* system) : pSystem(system)
    {
    }

    ~GFxSoundExporterFMOD()
    {
    	if (pSystem)
    	{
    		FMOD_RESULT       result;
    		result = pSystem->close();
    		result = pSystem->release();
    	}
    }

    const char* GetDefaultExtension() const { return "wav"; }

    static void WriteWaveHeader(File* dest, UInt32 samplerate, UInt16 channels, UInt16 bits, UInt32 length)
    {
        int block_align = bits/8*channels;
        int avg_bytes_per_sec = samplerate * block_align;

        dest->Write((UByte *)"RIFF",4);
        dest->WriteUInt32(length);
        dest->Write((UByte *)"WAVE",4);
        dest->Write((UByte *)"fmt ",4);
        dest->WriteUInt32(16);  // data header size
        dest->WriteUInt16(1);   // compression (PCM)
        dest->WriteUInt16(channels); 
        dest->WriteUInt32(samplerate); 
        dest->WriteUInt32(avg_bytes_per_sec); 
        dest->WriteUInt16((UInt16)block_align); 
        dest->WriteUInt16(bits); 
        dest->Write((UByte *)"data",4);
        dest->WriteUInt32(length); 
    }

    static void UpdateWaveHeader(File* dest, UInt32 file_size, UInt32 sound_len)
    {
        // new we need to fix "RIFF" chunk data size
        dest->Seek(4,File::Seek_Set);
        dest->WriteUInt32(file_size - 8);
        // and "DATA" chunk data size
        dest->Seek(0x28,File::Seek_Set);
        dest->WriteUInt32(sound_len);

    }
    virtual bool WriteWaveFile(Sound::SoundData* psound, File* dest, GFxExportSoundAttrs* attrs)
    {
        if ((psound->GetFormat() & Sound::SoundData::Sample_Format) == Sound::SoundData::Sample_PCM)
        {
            UInt16 bits = (psound->GetFormat() & 0x7) ? 16 : 8;
            WriteWaveHeader(dest, psound->GetRate(), (UInt16)psound->GetChannelNumber(), bits, psound->GetDataSize());
            dest->Write(psound->GetData(), psound->GetDataSize());
            UpdateWaveHeader(dest, dest->Tell(), psound->GetDataSize());
        }
        else
        {
            FMOD_CREATESOUNDEXINFO exinfo;
            memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
            exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
            int flags = FMOD_OPENMEMORY_POINT | FMOD_OPENONLY | FMOD_ACCURATETIME;
            exinfo.length = psound->GetDataSize();
            FMOD::Sound      *sound;
            FMOD_RESULT       result;
            result = pSystem->createStream((const char*)psound->GetData(), flags, &exinfo, &sound);
            if (result != FMOD_OK)
                return false;

            unsigned int length = 0;
            result = sound->getLength(&length, FMOD_TIMEUNIT_PCM);
            if (result != FMOD_OK)
                return false;
            attrs->SampleCount = length;

            result = sound->getLength(&length, FMOD_TIMEUNIT_PCMBYTES);
            if (result != FMOD_OK)
                return false;
            
            int channels, bits;
            result = sound->getFormat(NULL,NULL,&channels,&bits);
            if (result != FMOD_OK)
                return false;
            attrs->Channels = (UInt16)channels;
            attrs->Bits = (UInt16)bits;

            float frequency;
            result = sound->getDefaults(&frequency, NULL, NULL, NULL);
            if (result != FMOD_OK)
                return false;

            attrs->Format = FileTypeConstants::File_WAVE; // WAVE
            attrs->SampleRate = (UInt32)frequency;

            WriteWaveHeader(dest, attrs->SampleRate, attrs->Channels, attrs->Bits, length);

            UByte* data = (UByte*)SF_ALLOC(CHUNKSIZE, Stat_Sound_Mem);
            unsigned int bytesread = 0, read;
            do
            {
                result = sound->readData((char *)data, CHUNKSIZE, &read);
                if (attrs->Bits == 8)
                {
                    for(unsigned int i = 0; i < read; ++i)
                        data[i] ^= 128;
                }
                dest->Write(data, read);
                bytesread += read;
            }
            while (result == FMOD_OK && read == CHUNKSIZE);
            SF_FREE(data);

            if (bytesread & 1) // the data size should be a multiple of 2
            {
                dest->WriteUByte(0);
                bytesread++;
            }
            UpdateWaveHeader(dest, dest->Tell(), bytesread);

            result = sound->release();
            //ERRCHECK(result);
        }
        return true;
    }

    virtual bool WriteWaveFile(Sound::AppendableSoundData* psound, File* dest, GFxExportSoundAttrs* attrs)
    {
        if ((psound->GetFormat() & Sound::SoundData::Sample_Format) == Sound::SoundData::Sample_PCM)
        {
            UInt16 bits = (psound->GetFormat() & 0x7) ? 16 : 8;
            WriteWaveHeader(dest, psound->GetRate(), (UInt16)psound->GetChannelNumber(), bits, psound->GetDataSize());
            UByte* data = (UByte*)SF_ALLOC(CHUNKSIZE, Stat_Sound_Mem);
            unsigned int bytesread = 0, read;
            do
            {
                read = psound->GetData(data, CHUNKSIZE);
                dest->Write(data, read);
                bytesread += read;
            }
            while (read == CHUNKSIZE);
            SF_FREE(data);
            if (bytesread & 1) // the data size should be a multiple of 2
            {
                dest->WriteUByte(0);
                bytesread++;
            }
            UpdateWaveHeader(dest, dest->Tell(), bytesread);
        }
        else
        {
            FMOD_CREATESOUNDEXINFO exinfo;
            memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
            exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
            exinfo.length = psound->GetDataSize();    
            exinfo.useropen = &DecodeOpen;
            exinfo.userclose= &DecodeClose;
            exinfo.userread = &DecodeRead;
            exinfo.userseek = &DecodeSeek;
            exinfo.decodebuffersize = 1024*8;
            int flags = FMOD_OPENONLY | FMOD_SOFTWARE | FMOD_IGNORETAGS; 
            FMOD::Sound      *sound;
            FMOD_RESULT       result;
            result = pSystem->createStream((const char*)psound, flags, &exinfo, &sound);
            if (result != FMOD_OK)
                return false;

            unsigned int length = 0;
            result = sound->getLength(&length, FMOD_TIMEUNIT_PCM);
            if (result != FMOD_OK)
                return false;
            attrs->SampleCount = length;

            result = sound->getLength(&length, FMOD_TIMEUNIT_PCMBYTES);
            if (result != FMOD_OK)
                return false;

            int channels, bits;
            result = sound->getFormat(NULL,NULL,&channels,&bits);
            if (result != FMOD_OK)
                return false;
            attrs->Channels = (UInt16)channels;
            attrs->Bits = (UInt16)bits;

            float frequency;
            result = sound->getDefaults(&frequency, NULL, NULL, NULL);
            if (result != FMOD_OK)
                return false;

            attrs->Format = FileTypeConstants::File_WAVE; // WAVE
            attrs->SampleRate = (UInt32)frequency;

            WriteWaveHeader(dest, attrs->SampleRate, attrs->Channels, attrs->Bits, length);

            UByte* data = (UByte*)SF_ALLOC(CHUNKSIZE, Stat_Sound_Mem);
            unsigned int bytesread = 0, read;
            do
            {
                result = sound->readData((char *)data, CHUNKSIZE, &read);
                if (attrs->Bits == 8)
                {
                    for(unsigned int i = 0; i < read; ++i)
                        data[i] ^= 128;
                }
                dest->Write(data, read);
                bytesread += read;
            }
            while (result == FMOD_OK && read == CHUNKSIZE);
            SF_FREE(data);

            if (bytesread & 1) // the data size should be a multiple of 2
            {
                dest->WriteUByte(0);
                bytesread++;
            }
            UpdateWaveHeader(dest, dest->Tell(), bytesread);

            result = sound->release();
            //ERRCHECK(result);
        }
        return true;
    }
    FMOD::System*    pSystem;
};

GFxSoundExporter* CreateSoundExporter(GFxExportListener *logger)
{
    FMOD::System     *system;
    //FMOD::Sound      *sound;
    //FMOD::Sound      *sound1;
    FMOD_RESULT       result;
    unsigned int      version;
    String          errorMsg;

    /*
    Create a System object and initialize.
    */
    result = FMOD::System_Create(&system);
    if (result != FMOD_OK)
    {
        Format(errorMsg, "FMOD error! ({0}) {1}", (unsigned)result, FMOD_ErrorString(result));
        logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
        return NULL;
    }

    result = system->getVersion(&version);
    if (result != FMOD_OK)
    {
        Format(errorMsg, "FMOD error! ({0}) {1}", (unsigned)result, FMOD_ErrorString(result));
        logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
        system->release();
        return NULL;
    }

    if (version < FMOD_VERSION)
    {
        Format(errorMsg, "Error!  You are using an old version of FMOD {0:08x}.  This program requires {1:08x}", version, FMOD_VERSION);
        logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
        system->release();
        return NULL;
    }

    result = system->init(10, FMOD_INIT_NORMAL, 0);
    if (result != FMOD_OK)
    {
        Format(errorMsg, "FMOD error! ({0}) {1}", (unsigned)result, FMOD_ErrorString(result));
        logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
        system->release();
        return NULL;
    }

    return new GFxSoundExporterFMOD(system);
}

#endif // SF_NO_SOUND
