/**************************************************************************

Filename    :   GFxImageExporters.cpp
Content     :   SWF to GFX resource extraction and conversion tool
Created     :   October, 2006
Authors     :   Artyom Bolgar, Dmitry Polenur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxExport.h"
#include "Kernel/SF_SysFile.h"
#include "Kernel/SF_MsgFormat.h"
#include "Render/ImageFiles/TGA_ImageFile.h"
#include "Render/ImageFiles/JPEG_ImageFile.h"
#include "Render/ImageFiles/DDS_ImageFile.h"
#include "Render/ImageFiles/PVR_ImageFile.h"
#include "Render/ImageFiles/SIF_ImageFile.h"

#include "ImageFormats/DXTHelper.h"
#include "ImageFormats/PVRHelper.h"
#include "ImageFormats/SIFHelper.h"

#include "ConsoleSupport.h"

class GFxTGAExporter : public GFxImageExporter
{
    GFxImageExporterParams Params;
public:
    const char* GetName() const         { return "TGA"; }
    const char* GetDescription() const  { return "Truevision (Targa or TGA)"; }
    int         GetFormatId() const     { return FileTypeConstants::File_TGA; }
    const char* GetDefaultExtension() const { return "tga"; }

    GFxImageExporterParams& InitParams
        (UInt32 charId, const GFxDataExporter* pexp, GFxExportImageDescr* pimgDescr, Render::Image* pimage, File* pfile = 0,
         GFxImageExporterParams::ImageExportType expType = GFxImageExporterParams::IET_Image)
    {
        Params.Set(charId, pexp, pimgDescr, pimage, pfile, expType);
        return Params;
    }

    bool        MightBeRescaled() const { return true; }
    bool        NeedToBeConverted(Render::Image* pimage, int destFormat) const
    {
        return (destFormat != pimage->GetFormat());
    }

    bool        Write(ExportImageHelper * pcompressor, GFxExportListener *logger )
    {
        SF_UNUSED(pcompressor);
        //if (!Params.ImagePtr->WriteTga(Params.FilePtr))
		if(!Render::TGA::FileWriter::WriteImage(Params.FilePtr, Params.ImagePtr))
        {
            String errorMsg;
            Format(errorMsg, "Error: Can't write to '{0}' file", Params.FilePtr->GetFilePath());
            logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
            return false;
        }
        return true;
    }

    // Read file and create Image. This method used for sharing images. If this method
    // is not implemented (returns NULL) then images of this format cannot be shared
    Render::Image*     Read(const char* filePath, GFxExportListener *logger)
    {
        Ptr<File> pin = *new SysFile(filePath, File::Open_Read);
        if (!pin || !pin->IsValid())
        {
            String errorMsg;
            Format(errorMsg, "Error: Can't open source file '{0}' to read from", filePath);
            logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
            return NULL;
        }
		return Render::TGA::FileReader::ReadImage(pin);
    }

    virtual void Clear() { Params.Clear(); }
};
GFX_REGISTER_IMAGE_EXPORTER(GFxTGAExporter);

class GFxDDSExporter : public GFxImageExporter
{
    struct GFxDDSImageExporterParams : public GFxImageExporterParams
    {
        bool                        UncompressedDDS;
    } Params;
public:
    const char* GetName() const         { return "DDS"; }
    const char* GetDescription() const  { return "DirectDraw Surface (DDS)"; }
    int         GetFormatId() const     { return FileTypeConstants::File_DDS; }
    const char* GetDefaultExtension() const { return "dds"; }

    GFxImageExporterParams& InitParams
        (UInt32 charId, const GFxDataExporter* pexp, GFxExportImageDescr* pimgDescr, Render::Image* pimage, File* pfile = 0,
         GFxImageExporterParams::ImageExportType expType = GFxImageExporterParams::IET_Image)
    {
        Params.Set(charId, pexp, pimgDescr, pimage, pfile, expType);
        Params.UncompressedDDS = pexp->IsUncompressedTexture();

        if (ResourceId(Params.CharacterId).GetIdType() == ResourceId::IdType_GradientImage)
        {
            {
                // check, is it for linear gradient (size 256x1) or not.
                // if linear - do not use compressed DDS, since this will 
                // resize gradient up to 256x4.
                if (Params.ImageDescr->TargetHeight == 1)
                    Params.UncompressedDDS = true;
                else
                    Params.UncompressedDDS = false;
            }
        }

        return Params;
    }

    bool        MightBeRescaled() const { return false; }
    bool        NeedToBeConverted(Render::Image*, int) const { return false; }

    bool        Write(ExportImageHelper*  pcompressor, GFxExportListener *logger)
    {
        DXTSettings settings;
        settings.DoGenMipMap = Params.Exporter->IsGenMipMapLevels();
        settings.IsDxt1Allowed = Params.Exporter->IsDXT1Allowed();
        settings.IsUncompressed = Params.UncompressedDDS;
        settings.MipFilter = Params.Exporter->GetMipFilter();
        settings.Quality = Params.Exporter->GetQuality();
        settings.Rescale = Params.Rescale;
        settings.RescaleFilter = Params.Exporter->GetRescaleFilter();
        settings.DxtN = Params.Exporter->GetDXTn();
        settings.bSquare = Params.Exporter->SquareImages();
        if (!pcompressor->CompressToFile(Params.ImagePtr, Params.FilePtr, &settings, logger))
        {
            String errorMsg;
            Format(errorMsg, "Error: Can't compress image '{0}' to DDS, error: '{1}'", Params.FilePtr->GetFilePath(), pcompressor->GetLastErrorString());
            logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
            return false;
        }
        return true;
    }

    // Read file and create Image. This method used for sharing images. If this method
    // is not implemented (returns NULL) then images of this format cannot be shared
    Render::Image*     Read(const char* filePath, GFxExportListener *logger)
    {
        Ptr<File> pin = *new SysFile(filePath, File::Open_Read);
        if (!pin || !pin->IsValid())
        {
            String errorMsg;
            Format(errorMsg, "Error: Can't open source file '{0}' to read from", filePath);
            logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
            return NULL;
        }
        return Render::DDS::FileReader::ReadImage(pin);
    }

    virtual void Clear() { Params.Clear(); }
};
GFX_REGISTER_IMAGE_EXPORTER(GFxDDSExporter);

class GFxPVRExporter : public GFxImageExporter
{
    struct GFxPVRImageExporterParams : public GFxImageExporterParams
    {
        bool Uncompressed;
    } Params;
public:
    const char* GetName() const         { return "PVR"; }
    const char* GetDescription() const  { return "PVR"; }
    int         GetFormatId() const     { return FileTypeConstants::File_PVR; }
    const char* GetDefaultExtension() const { return "pvr"; }

    GFxImageExporterParams& InitParams
        (UInt32 charId, const GFxDataExporter* pexp, GFxExportImageDescr* pimgDescr, Render::Image* pimage, File* pfile = 0,
        GFxImageExporterParams::ImageExportType expType = GFxImageExporterParams::IET_Image)
    {
        Params.Set(charId, pexp, pimgDescr, pimage, pfile, expType);
        Params.Uncompressed = false;
        if (ResourceId(Params.CharacterId).GetIdType() == ResourceId::IdType_GradientImage)
        {
            {
                // check, is it for linear gradient (size 256x1) or not.
                // if linear - do not use compressed DDS, since this will 
                // resize gradient up to 256x4.
                if (Params.ImageDescr->TargetHeight == 1)
                    Params.Uncompressed = true;
                else
                    Params.Uncompressed = false;
            }
        }

        return Params;
    }

    bool        MightBeRescaled() const { return false; }
    bool        NeedToBeConverted(Render::Image*, int) const { return false; }

    bool        Write(ExportImageHelper*  pcompressor, GFxExportListener *logger)
    {
        PVRSettings settings;
        settings.bSquare = true;
        settings.DoGenMipMap = Params.Exporter->IsGenMipMapLevels();
        settings.IsUncompressed = Params.Uncompressed;
        settings.Rescale = Params.Rescale;
        settings.RescaleFilter = Params.Exporter->GetRescaleFilter();
        settings.PVRn = Params.Exporter->GetPVRn();
        settings.Format = PVRSettings::FormatPVRTC;
        if (!pcompressor->CompressToFile(Params.ImagePtr, Params.FilePtr, &settings, logger))
        {
            String errorMsg;
            Format(errorMsg, "Error: Can't compress image '{0}' to DDS, error: '{1}'", Params.FilePtr->GetFilePath(), pcompressor->GetLastErrorString());
            logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
            return false;
        }
        return true;
    }

    // Read file and create Image. This method used for sharing images. If this method
    // is not implemented (returns NULL) then images of this format cannot be shared
    Render::Image*     Read(const char* filePath, GFxExportListener *logger)
    {
        Ptr<File> pin = *new SysFile(filePath, File::Open_Read);
        if (!pin || !pin->IsValid())
        {
            String errorMsg;
            Format(errorMsg, "Error: Can't open source file '{0}' to read from", filePath);
            logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
            return NULL;
        }
        return Render::PVR::FileReader::ReadImage(pin);
    }

    virtual void Clear() { Params.Clear(); }
};
GFX_REGISTER_IMAGE_EXPORTER(GFxPVRExporter);



class GFxSIFExporter : public GFxImageExporter
{
    struct GFxSIFImageExporterParams : public GFxImageExporterParams
    {
        bool Uncompressed;
        ImageTarget Platform;
    } Params;
public:
    const char* GetName() const         { return "SIF"; }
    const char* GetDescription() const  { return "SIF"; }
    int         GetFormatId() const     { return FileTypeConstants::File_SIF; }
    const char* GetDefaultExtension() const { return "sif"; }

    GFxImageExporterParams& InitParams
        (UInt32 charId, const GFxDataExporter* pexp, GFxExportImageDescr* pimgDescr, Render::Image* pimage, File* pfile = 0,
        GFxImageExporterParams::ImageExportType expType = GFxImageExporterParams::IET_Image)
    {
        Params.Set(charId, pexp, pimgDescr, pimage, pfile, expType);
        Params.Uncompressed = pexp->IsUncompressedTexture();
        if (ResourceId(Params.CharacterId).GetIdType() == ResourceId::IdType_GradientImage)
        {
            {
                if (Params.ImageDescr->TargetHeight == 1)
                    Params.Uncompressed = true;
                else
                    Params.Uncompressed = false;
            }
        }
        Params.Platform = pexp->GetPlatform();

        return Params;
    }

    bool        MightBeRescaled() const { return false; }
    bool        NeedToBeConverted(Render::Image*, int) const { return false; }

    bool        Write(ExportImageHelper*  pcompressor, GFxExportListener *logger)
    {
        SF_UNUSED(pcompressor);
        SIFSettings settings;
        settings.bSquare = Params.Exporter->SquareImages();
        settings.DoGenMipMap = Params.Exporter->IsGenMipMapLevels();
        settings.IsUncompressed = Params.Uncompressed;
        settings.Rescale = Params.Rescale;
        settings.RescaleFilter = Params.Exporter->GetRescaleFilter();
        settings.Target = Params.Exporter->GetPlatform();
        
        if (!pcompressor->CompressToFile(Params.ImagePtr, Params.FilePtr, &settings, logger))
        {
            String errorMsg;
            Format(errorMsg, "Error: Can't write image '{0}' to SIF, error: '{1}'", Params.FilePtr->GetFilePath(), pcompressor->GetLastErrorString());
            logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
            return false;
        }
        return true;
    }

    // Read file and create Image. This method used for sharing images. If this method
    // is not implemented (returns NULL) then images of this format cannot be shared
    Render::Image*     Read(const char* filePath, GFxExportListener *logger)
    {
        SF_UNUSED2(filePath, logger);
        return NULL;
    }

    virtual void Clear() { Params.Clear(); }
};
GFX_REGISTER_IMAGE_EXPORTER(GFxSIFExporter);

class GFxETCExporter : public GFxImageExporter
{
    struct GFxETCImageExporterParams : public GFxImageExporterParams
    {
        bool Uncompressed;
    } Params;
public:
    const char* GetName() const         { return "ETC"; }
    const char* GetDescription() const  { return "ETC"; }
    int         GetFormatId() const     { return FileTypeConstants::File_ETC; }
    const char* GetDefaultExtension() const { return "pvr"; }

    GFxImageExporterParams& InitParams
        (UInt32 charId, const GFxDataExporter* pexp, GFxExportImageDescr* pimgDescr, Render::Image* pimage, File* pfile = 0,
        GFxImageExporterParams::ImageExportType expType = GFxImageExporterParams::IET_Image)
    {
        Params.Set(charId, pexp, pimgDescr, pimage, pfile, expType);
        Params.Uncompressed = false;
        if (ResourceId(Params.CharacterId).GetIdType() == ResourceId::IdType_GradientImage)
        {
            {
                // check, is it for linear gradient (size 256x1) or not.
                // if linear - do not use compressed DDS, since this will 
                // resize gradient up to 256x4.
                if (Params.ImageDescr->TargetHeight == 1)
                    Params.Uncompressed = true;
                else
                    Params.Uncompressed = false;
            }
        }

        return Params;
    }

    bool        MightBeRescaled() const { return false; }
    bool        NeedToBeConverted(Render::Image*, int) const { return false; }

    bool        Write(ExportImageHelper*  pcompressor, GFxExportListener *logger)
    {
        PVRSettings settings;
        settings.DoGenMipMap = Params.Exporter->IsGenMipMapLevels();
        settings.IsUncompressed = Params.Uncompressed;
        settings.Rescale = Params.Rescale;
        settings.RescaleFilter = Params.Exporter->GetRescaleFilter();
        settings.Format = PVRSettings::FormatETC;
        if (!pcompressor->CompressToFile(Params.ImagePtr, Params.FilePtr, &settings, logger))
        {
            String errorMsg;
            Format(errorMsg, "Error: Can't compress image '{0}' to ETC, error: '{1}'", Params.FilePtr->GetFilePath(), pcompressor->GetLastErrorString());
            logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
            return false;
        }
        return true;
    }
    // Read file and create Image. This method used for sharing images. If this method
    // is not implemented (returns NULL) then images of this format cannot be shared
    Render::Image*     Read(const char* filePath, GFxExportListener *logger)
    {
        Ptr<File> pin = *new SysFile(filePath, File::Open_Read);
        if (!pin || !pin->IsValid())
        {
            String errorMsg;
            Format(errorMsg, "Error: Can't open source file '{0}' to read from", filePath);
            logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
            return NULL;
        }
        return Render::PVR::FileReader::ReadImage(pin);
    }

    virtual void Clear() { Params.Clear(); }
};
GFX_REGISTER_IMAGE_EXPORTER(GFxETCExporter);

class GFxORIGExporter : public GFxImageExporter
{
    struct GFxOrigImageExporterParams : public GFxImageExporterParams
    {
        String                  PathNoExt;
        const JpegDesc*         pJpegDesc;
    } Params;
public:
    const char* GetName() const         { return "ORIG"; }
    const char* GetDescription() const  { return "Original format: JPEG data will be saved as JPG files\n"
        "                         and lossless images will be saved as TGAs.\n"; }
    int         GetFormatId() const     { return FileTypeConstants::File_Original; }
    const char* GetDefaultExtension() const 
    { 
        if (Params.ImageDescr == NULL || Params.ImageDescr->Format == FileTypeConstants::File_JPEG)
        {
            return "jpg";
        }
        else
        {
            return "tga";
        }
        //return "jpg"; 
    }

    GFxImageExporterParams& InitParams
        (UInt32 charId, const GFxDataExporter* pexp, GFxExportImageDescr* pimgDescr, Render::Image* pimage, File* pfile = 0,
         GFxImageExporterParams::ImageExportType expType = GFxImageExporterParams::IET_Image)
    {
        Params.Set(charId, pexp, pimgDescr, pimage, pfile, expType);
        Params.pJpegDesc = Params.Exporter->GetJpegDescriptor(Params.CharacterId);
        if (Params.pJpegDesc)
        {
            if (Params.pJpegDesc->TagType != 35)
                Params.ImageDescr->Format = FileTypeConstants::File_JPEG;
            else
                Params.ImageDescr->Format = FileTypeConstants::File_TGA;
        }
        else
        {
            Params.ImageDescr->Format = FileTypeConstants::File_TGA;
            Params.pJpegDesc = 0;
        }
        return Params;
    }
    bool        MightBeRescaled() const { return false; }
    bool        NeedToBeConverted(Render::Image*, int) const { return false; }

    String   MakeFileName(const String& nameNoExt)
    {
        return nameNoExt + ((Params.ImageDescr->Format == FileTypeConstants::File_JPEG) ? ".jpg" : ".tga");
    }
    virtual String   MakePath(const String& path, const String& nameWithExt)
    {
        String p = path + nameWithExt;
        Params.PathNoExt = GFxDataExporter::CutExtension(p);
        return p;
    }

    bool        Write(ExportImageHelper * pcompressor, GFxExportListener *logger)
    {
        SF_UNUSED(pcompressor);
        if (Params.pJpegDesc)
        {
            if (Params.pJpegDesc->TagType == 35)
            {
                // if JPEG was used with alpha, we need to save the 32-bit TGA as main
                // image and only then JPEG.
                //if (!Params.ImagePtr->WriteTga(Params.FilePtr))
				if (!Render::TGA::FileWriter::WriteImage(Params.FilePtr, Params.ImagePtr))
                {
                    String errorMsg;
                    Format(errorMsg, "Error: Can't write to '{0}' file", Params.FilePtr->GetFilePath());
                    logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                    return false;
                }
                Params.FilePtr->Close();

                // now need to reopen file as jpg and write data to it
                String destPath;
                destPath = Params.PathNoExt;
                destPath += ".jpg";
                Params.FilePtr = *new SysFile(destPath.ToCStr(),
                    File::Open_Write | File::Open_Truncate | File::Open_Create);
                if (!Params.FilePtr->IsValid() || !Params.FilePtr->IsWritable())
                {
                    String errorMsg;
                    Format(errorMsg, "Error: Can't open file '{0}' for writing", destPath.ToCStr());
                    logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                    return false;
                }
            }

            // check if tagType == 6 and we have parsed tagType == 8 (JPEGTables) then
            // write these tables first.
            if (Params.pJpegDesc->TagType == 6)
            {
                const JpegDesc* pjpeg8Desc = Params.Exporter->GetJpegDescriptor(-1);
                if (pjpeg8Desc)
                {
                    if (Params.FilePtr->Write(pjpeg8Desc->pData, int(pjpeg8Desc->DataSize)) != int(pjpeg8Desc->DataSize))
                    {
                        String errorMsg;
                        Format(errorMsg, "Error: Can't write to '{0}' file", Params.FilePtr->GetFilePath());
                        logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                        return false;
                    }
                }
            }
            if (Params.FilePtr->Write(Params.pJpegDesc->pData, int(Params.pJpegDesc->DataSize)) != int(Params.pJpegDesc->DataSize))
            {
                String errorMsg;
                Format(errorMsg, "Error: Can't write to '{0}' file", Params.FilePtr->GetFilePath());
                logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                return false;
            }
            String destPath = Params.FilePtr->GetFilePath();
            Params.FilePtr->Close(); // close file and re-open for reading
            Params.FilePtr = *new SysFile(destPath.ToCStr(), File::Open_Read);
            if (!Params.FilePtr->IsValid())
            {
                String errorMsg;
                Format(errorMsg, "Error: Can't open file '{0}' for reading", destPath.ToCStr());
                logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                return false;
            }

            Ptr<JPEGSystem> pjpegSystem = *JPEGSystem::CreateDefaultSystem();
            if (!pjpegSystem)  return false;
            // original format
            // Write original JPEG data without re-compression
            // First of all, re-read JPEG by jpeglib without decompression....
            JPEGInput* pjin = pjpegSystem->CreateSwfJpeg2HeaderOnly(Params.FilePtr);
            if (!pjin) return false;

            pjin->StartRawImage();
            void* prawData;
            pjin->ReadRawData(&prawData);
            Params.FilePtr->Close(); // close file and re-open for writing

            Params.FilePtr = *new SysFile(destPath.ToCStr(),
                File::Open_Write | File::Open_Truncate | File::Open_Create);
            if (!Params.FilePtr->IsValid() || !Params.FilePtr->IsWritable())
            {
                String errorMsg;
                Format(errorMsg, "Error: Can't open file '{0}' for writing", destPath.ToCStr());
                logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                return false;
            }

            // write JPEG data back w/o re-compression
            JPEGOutput*    pjout = pjpegSystem->CreateOutput(Params.FilePtr);
            pjout->CopyCriticalParams(pjin->GetCInfo());
            pjout->WriteRawData(prawData);
            delete pjout;

            pjin->FinishImage();
            delete pjin;

            Params.FilePtr->Close();
        }
        else
        {
            //if (!Params.ImagePtr->WriteTga(Params.FilePtr))
			if (!Render::TGA::FileWriter::WriteImage(Params.FilePtr, Params.ImagePtr))
            {
                String errorMsg;
                Format(errorMsg, "Error: Can't write to '{0}' file", Params.FilePtr->GetFilePath());
                logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                return false;
            }
        }
        return true;
    }

    // Read file and create Image. This method used for sharing images. If this method
    // is not implemented (returns NULL) then images of this format cannot be shared
    Render::Image*     Read(const char* filePath, GFxExportListener *logger)
    {
        String path (filePath);
        String ext = path.GetExtension();
        if (ext == ".$$$")
        {
            path.StripExtension();
            ext = path.GetExtension();
        }

        Ptr<File> pin = *new SysFile(filePath, File::Open_Read);
        if (!pin || !pin->IsValid())
        {
            String errorMsg;
            Format(errorMsg, "Error: Can't open source file '{0}' to read from", filePath);
            logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
            return NULL;
        }
        if (ext == ".jpg")
            return Render::JPEG::FileReader::ReadImage(pin);//Render::Image::ReadTga(pin);
        else 
            return Render::TGA::FileReader::ReadImage(pin);
    }

    virtual void Clear() { Params.Clear(); }
};
GFX_REGISTER_IMAGE_EXPORTER(GFxORIGExporter);

class GFxMultExporter : public GFxImageExporter
{
    struct GFxMultImageExporterParams : public GFxImageExporterParams
    {
        String                  PathNoExt;
        bool                    Uncompressed;
    } Params;
    GFxImageExporter* pDDSexporter;
    GFxImageExporter* pPVRexporter;
    GFxImageExporter* pSIFexporter;
    GFxImageExporterParams* pDDSParams;
    GFxImageExporterParams* pPVRParams;
    GFxImageExporterParams* pSIFParams;

public:

    const char* GetName() const         { return "Mult"; }
    const char* GetDescription() const  { return "Saves file in TGA, DXT, PVR formats\n";}
    int         GetFormatId() const     { return FileTypeConstants::File_MultiFormat; }

    String   MakeFileName(const String& nameNoExt)
    {
        return nameNoExt;
    }
    const char* GetDefaultExtension() const 
    { 
        return ""; 
    }

    GFxImageExporterParams& InitParams
        (UInt32 charId, const GFxDataExporter* pexp, GFxExportImageDescr* pimgDescr, Render::Image* pimage, File* pfile = 0,
        GFxImageExporterParams::ImageExportType expType = GFxImageExporterParams::IET_Image)
    {
        Params.Set(charId, pexp, pimgDescr, pimage, pfile, expType);
        Params.Uncompressed = false;
        if (ResourceId(Params.CharacterId).GetIdType() == ResourceId::IdType_GradientImage)
        {
            {
                // check, is it for linear gradient (size 256x1) or not.
                // if linear - do not use compressed DDS, since this will 
                // resize gradient up to 256x4.
                if (Params.ImageDescr->TargetHeight == 1)
                    Params.Uncompressed = true;
                else
                    Params.Uncompressed = false;
            }
        }

        pDDSexporter = Params.Exporter->GetImageExporter(FileTypeConstants::File_DDS);
        pDDSParams = &pDDSexporter->InitParams(charId, pexp, pimgDescr, pimage, pfile, expType);
        pPVRexporter = Params.Exporter->GetImageExporter(FileTypeConstants::File_PVR);
        pPVRParams = &pPVRexporter->InitParams(charId, pexp, pimgDescr, pimage, pfile, expType);
        pSIFexporter = Params.Exporter->GetImageExporter(FileTypeConstants::File_SIF);
        pSIFParams = &pSIFexporter->InitParams(charId, pexp, pimgDescr, pimage, pfile, expType);
        return Params;
    }
    bool        MightBeRescaled() const { return false; }
    bool        NeedToBeConverted(Render::Image*, int) const { return false; }

    //String   MakeFileName(const String& nameNoExt)
    //{
    //    return nameNoExt + ((Params.ImageDescr->Format == FileTypeConstants::File_JPEG) ? ".jpg" : ".tga");
    //}
    virtual String   MakePath(const String& path, const String& nameWithExt)
    {
        String p = path + nameWithExt;
        Params.PathNoExt = GFxDataExporter::CutExtension(p);
        return p;
    }

    bool        Write(ExportImageHelper * pcompressor, GFxExportListener *logger)
    {
        SF_UNUSED(pcompressor);
        {
            pDDSParams->Rescale = pPVRParams->Rescale =  Params.Rescale;
            pDDSParams->ImagePtr = pPVRParams->ImagePtr =  Params.ImagePtr;
            //if (!Params.ImagePtr->WriteTga(Params.FilePtr))
            if (!Render::TGA::FileWriter::WriteImage(Params.FilePtr, Params.ImagePtr))
            {
                String errorMsg;
                Format(errorMsg, "Error: Can't write to '{0}' file", Params.FilePtr->GetFilePath());
                logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                return false;
            }
            Ptr<File> pfile = *new SysFile((Params.PathNoExt + ".dds").ToCStr(),
                File::Open_Write | File::Open_Truncate | File::Open_Create);
            pDDSParams->FilePtr = pfile;
            pDDSexporter->Write(Params.Exporter->GetDXTCompressor(), logger);
            pfile->Close();
            pfile = *new SysFile((Params.PathNoExt + ".pvr").ToCStr(),
                File::Open_Write | File::Open_Truncate | File::Open_Create);

            pPVRParams->FilePtr = pfile;
            pPVRexporter->Write(Params.Exporter->GetPVRCompressor(), logger);
#ifdef SF_OS_WIN32
            pfile = *new SysFile((Params.PathNoExt + ".sif").ToCStr(),
                File::Open_Write | File::Open_Truncate | File::Open_Create);

            pSIFParams->FilePtr = pfile;
            pSIFexporter->Write(Params.Exporter->GetSIFCompressor(), logger);
#endif
        }
        return true;
    }

    // Read file and create Image. This method used for sharing images. If this method
    // is not implemented (returns NULL) then images of this format cannot be shared
    Render::Image*     Read(const char* filePath, GFxExportListener *logger)
    {
        SF_UNUSED2(filePath, logger);
        //// check for extension first
        //const char* pext = strrchr(filePath, '.');
        //if (pext != NULL)
        //{
        //    ++pext;
        //}
        //Ptr<File> pin = *new SysFile(filePath, File::Open_Read);
        //if (!pin || !pin->IsValid())
        //{
        //    fprintf(stderr, "Error: Can't open source file '%s' to read from\n", filePath);
        //    return NULL;
        //}
        //if (SFstricmp(pext, "tga") == 0)
        //    return Render::TGA::FileReader::ReadImage(pin);//Render::Image::ReadTga(pin);
        //else if (SFstricmp(pext, "dds") == 0)
        //{
        //    //Ptr<JPEGSystem> pjpegSystem = *Render::JPEGSystem::CreateDefaultSystem();
        //    //if (pjpegSystem)
        //    return Render::DDS::FileReader::ReadImage(pin);
        //}
        //else if (SFstricmp(pext, "pvr") == 0)
        //{
        //    //Ptr<JPEGSystem> pjpegSystem = *Render::JPEGSystem::CreateDefaultSystem();
        //    //if (pjpegSystem)
        //    return Render::PVR::FileReader::ReadImage(pin);
        //}
        return NULL;
    }

    virtual void Clear() { Params.Clear(); }
};
GFX_REGISTER_IMAGE_EXPORTER(GFxMultExporter);

class GFxGTXExporter : public GFxImageExporter //WiiU
{
    struct GFxGTXImageExporterParams : public GFxImageExporterParams
    {
        String                  PathNoExt;
        bool                    Uncompressed;
    } Params;
    GFxImageExporter* pDDSexporter;
    GFxImageExporterParams* pDDSParams;

public:

    const char* GetName() const         { return "gtx"; }
    const char* GetDescription() const  { return "Saves file GTX\n";}
    int         GetFormatId() const     { return FileTypeConstants::File_GTX; }

    String   MakeFileName(const String& nameNoExt)
    {
        return nameNoExt + ".gtx";
    }
    const char* GetDefaultExtension() const 
    { 
        return "gtx"; 
    }

    GFxImageExporterParams& InitParams
        (UInt32 charId, const GFxDataExporter* pexp, GFxExportImageDescr* pimgDescr, Render::Image* pimage, File* pfile = 0,
        GFxImageExporterParams::ImageExportType expType = GFxImageExporterParams::IET_Image)
    {
        Params.Set(charId, pexp, pimgDescr, pimage, pfile, expType);
        Params.Uncompressed = false;
        if (ResourceId(Params.CharacterId).GetIdType() == ResourceId::IdType_GradientImage)
        {
            {
                // check, is it for linear gradient (size 256x1) or not.
                // if linear - do not use compressed DDS, since this will 
                // resize gradient up to 256x4.
                if (Params.ImageDescr->TargetHeight == 1)
                    Params.Uncompressed = true;
                else
                    Params.Uncompressed = false;
            }
        }
        pDDSexporter = Params.Exporter->GetImageExporter(FileTypeConstants::File_DDS);
        pDDSParams = &pDDSexporter->InitParams(charId, pexp, pimgDescr, pimage, pfile, expType);
        return Params;
    }
    bool        MightBeRescaled() const { return false; }
    bool        NeedToBeConverted(Render::Image*, int) const { return false; }


    virtual String   MakePath(const String& path, const String& nameWithExt)
    {
        String p = path + nameWithExt;
        Params.PathNoExt = GFxDataExporter::CutExtension(p);
        return p;
    }

    bool        Write(ExportImageHelper * pcompressor, GFxExportListener *logger)
    {
#if defined (SF_OS_WIN32)
        SF_UNUSED(pcompressor);
        {
            GFxExportConsolePlugin* pexporter = FindConsole(ImageTarget_WiiU);
            if (!pexporter)
            {
                String errorMsg;
                Format(errorMsg, "Error: Can't find console plugin\n");
                logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                return false;
            }
            pDDSParams->Rescale  =  Params.Rescale;
            pDDSParams->ImagePtr =  Params.ImagePtr;
            //if (!Params.ImagePtr->WriteTga(Params.FilePtr))

            Ptr<File> pfile = *new SysFile((Params.PathNoExt + ".dds").ToCStr(),
                File::Open_Write | File::Open_Truncate | File::Open_Create);
            pDDSParams->FilePtr = pfile;
            pDDSexporter->Write(Params.Exporter->GetDXTCompressor(), logger);
            pfile->Close();
            Params.FilePtr->Close();
            SIFSettings settings;
            pexporter->ConvertFile(Params.PathNoExt, settings);

        }
        return true;
#else
        return false;
#endif
    }

    // Read file and create Image. This method used for sharing images. If this method
    // is not implemented (returns NULL) then images of this format cannot be shared
    Render::Image*     Read(const char* filePath, GFxExportListener *logger)
    {
        SF_UNUSED2(filePath, logger);
        return NULL;
    }

    virtual void Clear() { Params.Clear(); }
};
GFX_REGISTER_IMAGE_EXPORTER(GFxGTXExporter);

class GFxGXTExporter : public GFxImageExporter //Sony Vita
{
    struct GFxGXTImageExporterParams : public GFxImageExporterParams
    {
        String                  PathNoExt;
        bool                    Uncompressed;
    } Params;
    GFxImageExporter* pPVRexporter;
    GFxImageExporterParams* pPVRParams;

public:

    const char* GetName() const         { return "GXT"; }
    const char* GetDescription() const  { return "Saves file GXT\n";}
    int         GetFormatId() const     { return FileTypeConstants::File_GXT; }

    String   MakeFileName(const String& nameNoExt)
    {
        return nameNoExt + ".gxt";
    }
    const char* GetDefaultExtension() const 
    { 
        return "gxt"; 
    }

    GFxImageExporterParams& InitParams
        (UInt32 charId, const GFxDataExporter* pexp, GFxExportImageDescr* pimgDescr, Render::Image* pimage, File* pfile = 0,
        GFxImageExporterParams::ImageExportType expType = GFxImageExporterParams::IET_Image)
    {
        Params.Set(charId, pexp, pimgDescr, pimage, pfile, expType);
        Params.Uncompressed = false;
        if (ResourceId(Params.CharacterId).GetIdType() == ResourceId::IdType_GradientImage)
        {
            {
                // check, is it for linear gradient (size 256x1) or not.
                // if linear - do not use compressed PVR, since this will 
                // resize gradient up to 256x4.
                if (Params.ImageDescr->TargetHeight == 1)
                    Params.Uncompressed = true;
                else
                    Params.Uncompressed = false;
            }
        }
        pPVRexporter = Params.Exporter->GetImageExporter(FileTypeConstants::File_PVR);
        pPVRParams = &pPVRexporter->InitParams(charId, pexp, pimgDescr, pimage, pfile, expType);
        return Params;
    }
    bool        MightBeRescaled() const { return false; }
    bool        NeedToBeConverted(Render::Image*, int) const { return false; }


    virtual String   MakePath(const String& path, const String& nameWithExt)
    {
        String p = path + nameWithExt;
        Params.PathNoExt = GFxDataExporter::CutExtension(p);
        return p;
    }

    bool        Write(ExportImageHelper * pcompressor, GFxExportListener *logger)
    {
#if defined (SF_OS_WIN32)
        SF_UNUSED(pcompressor);
        {
            GFxExportConsolePlugin* pconsoleplugin = FindConsole(ImageTarget_PSVita);
            if (!pconsoleplugin)
            {
                String errorMsg;
                Format(errorMsg, "Error: Can't find console plugin\n");
                logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                return false;
            }
            pPVRParams->Rescale  =  Params.Rescale;
            pPVRParams->ImagePtr =  Params.ImagePtr;
            //if (!Params.ImagePtr->WriteTga(Params.FilePtr))

            Ptr<File> pfile = *new SysFile((Params.PathNoExt + ".pvr").ToCStr(),
                File::Open_Write | File::Open_Truncate | File::Open_Create);
            pPVRParams->FilePtr = pfile;
            pPVRexporter->Write(Params.Exporter->GetPVRCompressor(), logger);
            pfile->Close();
            Params.FilePtr->Close();
            SIFSettings settings;
            pconsoleplugin->ConvertFile(Params.PathNoExt, settings);

        }
        return true;
#else
        return false;
#endif
    }

    // Read file and create Image. This method used for sharing images. If this method
    // is not implemented (returns NULL) then images of this format cannot be shared
    Render::Image*     Read(const char* filePath, GFxExportListener *logger)
    {
        SF_UNUSED2(filePath, logger);
        return NULL;
    }

    virtual void Clear() { Params.Clear(); }
};
GFX_REGISTER_IMAGE_EXPORTER(GFxGXTExporter);

class GFxDX11Exporter : public GFxImageExporter //BC7
{
    struct GFxDX11ImageExporterParams : public GFxImageExporterParams
    {
        String                  PathNoExt;
        bool                    Uncompressed;
    } Params;
    GFxImageExporter* pDDSexporter;
    GFxImageExporterParams* pDDSParams;

public:

    const char* GetName() const         { return "BC7"; }
    const char* GetDescription() const  { return "Saves file in DX1 block compressed format\n";}
    int         GetFormatId() const     { return FileTypeConstants::File_DDSBC; }

    String   MakeFileName(const String& nameNoExt)
    {
        return nameNoExt + ".dds";
    }
    const char* GetDefaultExtension() const 
    { 
        return "dds"; 
    }

    GFxImageExporterParams& InitParams
        (UInt32 charId, const GFxDataExporter* pexp, GFxExportImageDescr* pimgDescr, Render::Image* pimage, File* pfile = 0,
        GFxImageExporterParams::ImageExportType expType = GFxImageExporterParams::IET_Image)
    {
        Params.Set(charId, pexp, pimgDescr, pimage, pfile, expType);
        Params.Uncompressed = false;
        if (ResourceId(Params.CharacterId).GetIdType() == ResourceId::IdType_GradientImage)
        {
            {
                // check, is it for linear gradient (size 256x1) or not.
                // if linear - do not use compressed DDS, since this will 
                // resize gradient up to 256x4.
                if (Params.ImageDescr->TargetHeight == 1)
                    Params.Uncompressed = true;
                else
                    Params.Uncompressed = false;
            }
        }
        pDDSexporter = Params.Exporter->GetImageExporter(FileTypeConstants::File_DDS);
        pDDSParams = &pDDSexporter->InitParams(charId, pexp, pimgDescr, pimage, pfile, expType);
        return Params;
    }
    bool        MightBeRescaled() const { return false; }
    bool        NeedToBeConverted(Render::Image*, int) const { return false; }


    virtual String   MakePath(const String& path, const String& nameWithExt)
    {
        String p = path + nameWithExt;
        Params.PathNoExt = GFxDataExporter::CutExtension(p);
        return p;
    }

    bool        Write(ExportImageHelper * pcompressor, GFxExportListener *logger)
    {
#if defined (SF_OS_WIN32)
        SF_UNUSED(pcompressor);
        {
            GFxExportConsolePlugin* pexporter = FindConsole(ImageTarget_DX11);
            if (!pexporter)
            {
                String errorMsg;
                Format(errorMsg, "Error: Can't find DX11 plugin\n");
                logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                return false;
            }
            if (Params.Rescale == ExportImageHelper::RescaleNone)
                ///pDDSParams->Rescale = ExportImageHelper::RescalePreScale;
                pDDSParams->Rescale = ExportImageHelper::RescaleNearestPower2;
            else
                pDDSParams->Rescale  =  Params.Rescale;
            pDDSParams->ImagePtr =  Params.ImagePtr;
            //if (!Params.ImagePtr->WriteTga(Params.FilePtr))

            pDDSParams->FilePtr = Params.FilePtr;
            pDDSexporter->Write(Params.Exporter->GetDXTCompressor(), logger);
            Params.FilePtr->Close();
            SIFSettings settings;
            if (Params.ImageDescr->TargetHeight >=4 &&  Params.ImageDescr->TargetWidth >=4)
                pexporter->ConvertFile(Params.PathNoExt, settings);

        }
        return true;
#else
        return false;
#endif
    }

    // Read file and create Image. This method used for sharing images. If this method
    // is not implemented (returns NULL) then images of this format cannot be shared
    Render::Image*     Read(const char* filePath, GFxExportListener *logger)
    {
        SF_UNUSED2(filePath, logger);
        return NULL;
    }

    virtual void Clear() { Params.Clear(); }
};
GFX_REGISTER_IMAGE_EXPORTER(GFxDX11Exporter);

class GFxGNFExporter : public GFxImageExporter //PS4
{
    struct GFxGNFImageExporterParams : public GFxImageExporterParams
    {
        String                  PathNoExt;
        bool                    Uncompressed;
    } Params;
    //GFxImageExporter* pDDSexporter;
    //GFxImageExporterParams* pDDSParams;

public:

    const char* GetName() const         { return "GNF"; }
    const char* GetDescription() const  { return "Saves file to PS4 GNF format \n";}
    int         GetFormatId() const     { return FileTypeConstants::File_GNF; }

    String   MakeFileName(const String& nameNoExt)
    {
        return nameNoExt + ".GNF";
    }
    const char* GetDefaultExtension() const 
    { 
        return "GNF"; 
    }

    GFxImageExporterParams& InitParams
        (UInt32 charId, const GFxDataExporter* pexp, GFxExportImageDescr* pimgDescr, Render::Image* pimage, File* pfile = 0,
        GFxImageExporterParams::ImageExportType expType = GFxImageExporterParams::IET_Image)
    {
        Params.Set(charId, pexp, pimgDescr, pimage, pfile, expType);
        Params.Uncompressed = pexp->IsUncompressedTexture();
        if (ResourceId(Params.CharacterId).GetIdType() == ResourceId::IdType_GradientImage)
        {
            {
                // check, is it for linear gradient (size 256x1) or not.
                // if linear - do not use compressed DDS, since this will 
                // resize gradient up to 256x4.
                if (Params.ImageDescr->TargetHeight == 1)
                    Params.Uncompressed = true;
                else
                    Params.Uncompressed = false;
            }
        }
        //pDDSexporter = Params.Exporter->GetImageExporter(FileTypeConstants::File_DDS);
        //pDDSParams = &pDDSexporter->InitParams(charId, pexp, pimgDescr, pimage, pfile, expType);
        return Params;
    }
    bool        MightBeRescaled() const { return true; }
    bool        NeedToBeConverted(Render::Image* pimage, int destFormat) const
    {
        return (destFormat != pimage->GetFormat());
    }


    virtual String   MakePath(const String& path, const String& nameWithExt)
    {
        String p = path + nameWithExt;
        Params.PathNoExt = GFxDataExporter::CutExtension(p);
        return p;
    }

    bool        Write(ExportImageHelper * pcompressor, GFxExportListener *logger)
    {
#if defined (SF_OS_WIN32)
        SF_UNUSED(pcompressor);
        {
            GFxExportConsolePlugin* pexporter = FindConsole(ImageTarget_Orbis);
            if (!pexporter)
            {
                String errorMsg;
                Format(errorMsg, "Error: Can't find console plugin\n");
                logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                return false;
            }
            //pDDSParams->Rescale  =  Params.Rescale;
            //pDDSParams->ImagePtr =  Params.ImagePtr;
            ////if (!Params.ImagePtr->WriteTga(Params.FilePtr))

            Ptr<File> pfile = *new SysFile((Params.PathNoExt + ".tga").ToCStr(),
                File::Open_Write | File::Open_Truncate | File::Open_Create);
            //pDDSParams->FilePtr = pfile;
            //pDDSexporter->Write(Params.Exporter->GetDXTCompressor(), logger);
            if (!Render::TGA::FileWriter::WriteImage(pfile, Params.ImagePtr))
            {
                String errorMsg;
                Format(errorMsg, "Error: Can't write to '{0}' file", Params.FilePtr->GetFilePath());
                logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                return false;
            }
            pfile->Close();
            Params.FilePtr->Close();
            CompressionSettinns settings;
            settings.IsUncompressed = Params.Uncompressed;
            settings.DoGenMipMap = Params.Exporter->IsGenMipMapLevels();
            pexporter->ConvertFile(Params.PathNoExt, settings);

        }
        return true;
#else
        return false;
#endif
    }

    // Read file and create Image. This method used for sharing images. If this method
    // is not implemented (returns NULL) then images of this format cannot be shared
    Render::Image*     Read(const char* filePath, GFxExportListener *logger)
    {
        SF_UNUSED2(filePath, logger);
        return NULL;
    }

    virtual void Clear() { Params.Clear(); }
};
GFX_REGISTER_IMAGE_EXPORTER(GFxGNFExporter);
