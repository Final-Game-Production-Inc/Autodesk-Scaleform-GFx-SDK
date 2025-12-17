/**************************************************************************

Filename    :   GFxExport.cpp
Content     :   SWF to GFX resource extraction and conversion tool
Created     :   August, 2006
Authors     :   Artyom Bolgar, Dmitry Polenur, Maxim Didenko

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#define GFX_EXPORT_MAJOR_VERSION    4
#define GFX_EXPORT_MINOR_VERSION    1
#define GFX_EXPORT_VERSION (((GFX_EXPORT_MAJOR_VERSION)<<8)|(GFX_EXPORT_MINOR_VERSION))

#include "GFxExport.h"
#include "GFxExportVisitors.h"
#include "GFx/GFx_PlayerImpl.h"
#include "Kernel/SF_SysFile.h"
#include "Kernel/SF_MsgFormat.h"
//#include "GSound.h"
#include "GFx/Audio/GFx_Sound.h"
#include "GFx/GFx_Audio.h"
#include "GFx/GFx_TextFieldDef.h"
#include "Render/Render_TextureUtil.h"

#define AS3_NO_FLASH // AS3_NO_FLASH should be defined before AS3_Global.h is included
#include "GFx/AS3/AS3_Global.h"

// Standard includes
#ifdef SF_OS_WIN32
#include <windows.h>
#include <winbase.h>
#include <direct.h>
#include "FxWin32DebugPopups.h"

#else
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include "../../../3rdParty/zlib-1.3.1/zlib.h"

#include "Render/ImageFiles/PNG_ImageFile.h"
#include "Render/ImageFiles/DDS_ImageFile.h"
#include "Render/ImageFiles/SIF_ImageFile.h"

#include "GFx/GFx_ImagePacker.h"

#include "ImageFormats/PVRHelper.h"
#include "ImageFormats/DXTHelper.h"
#include "ImageFormats/SIFHelper.h"

using namespace Scaleform;

#define GFX_MAX_TEXTURE_DIMENSION 4096

// ***** Default Font Lib texture generation constants

// The default rendered glyph size can be overridden to trade off
// memory vs. tessellation of large glyphs.
// To set these values use FontPackParams::GetTextureGlyphConfig.


#if defined(SF_CC_MSVC) && (SF_CC_MSVC >= 1400)
#define G_getcwd  _getcwd
#define SFchdir   _chdir
#define G_mkdir   _mkdir
#elif defined(SF_OS_WIN32)
#define G_getcwd  getcwd
#define SFchdir   chdir
#define G_mkdir   mkdir
#else
#define G_getcwd  getcwd
#define SFchdir   chdir
#define G_mkdir(x)   mkdir(x,0777)
#endif

inline char* SFstrlwr(char* str, UPInt numberOfElements)
{
#if defined(SF_MSVC_SAFESTRING)
    _strlwr_s(str, numberOfElements);
    return str;
#elif defined(SF_OS_WIN32)
    SF_UNUSED(numberOfElements);
    return _strlwr(str);
#else
    for (UPInt i = 0; i < numberOfElements && str[i]; i++)
        str[i] = SFtolower(str[i]);
    return str;
#endif
}

inline char* SFstrupr(char* str, UPInt numberOfElements)
{
#if defined(SF_MSVC_SAFESTRING)
    _strupr_s(str, numberOfElements);
    return str;
#elif defined(SF_OS_WIN32)
    SF_UNUSED(numberOfElements);
    return _strupr(str);
#else
    for (UPInt i = 0; i < numberOfElements && str[i]; i++)
        str[i] = SFtoupper(str[i]);
    return str;
#endif
}

class GFxImagePackerInfo : public ImagePacker
{
public:
    HashSet<ImageBase*>            InputImages;
    GFxImagePackerInfo()
    {
    }
    virtual void SetBindData(Scaleform::GFx::MovieDefImpl::BindTaskData* pbinddata) {SF_UNUSED(pbinddata);}
    virtual void AddImageFromResource(ImageResource* presource, const char* pexportname)
    {
        SF_UNUSED(pexportname);
        InputImages.Add(presource->GetImage());
    }
    virtual void AddResource(ResourceDataNode* presNode, ImageResource* presource)
    {
        SF_UNUSED2(presNode, presource);
    }
    virtual void Finish() {}
};

void GFxImageExporterParams::Set
    (UInt32 charId, const GFxDataExporter* pexp, 
     GFxExportImageDescr* pimgDescr, Render::Image* pimage, File* pfile, 
     ImageExportType expType)
{
    CharacterId = charId;
    Exporter   = pexp;
    ImagePtr      = pimage;
    ImageDescr = pimgDescr;
    FilePtr       = pfile;
    Rescale     = (Exporter) ? Exporter->GetRescale() : ExportImageHelper::RescaleNone;
    ExpType     = expType;
}
class ExportImageData //helper for FindIdenticalImage
{
	ImageData Data;
	Ptr<RawImage> pRawImage;
public:
	ExportImageData (Image* psrcimage)
	{
		SF_ASSERT(psrcimage);
		pRawImage = *RawImage::Create(psrcimage->GetFormat(), 1, psrcimage->GetSize(), 0);
		pRawImage->GetImageData(&Data);
		psrcimage->Decode(&Data);
	}
	UPInt ComputeHash() const {return Data.ComputeHash();}
	bool operator == (const ExportImageData& src) const
	{
		if (src.Data.Format != Data.Format)
			return false;
		ImagePlane pl1, pl2;
		src.Data.GetPlane(0, &pl1);
		Data.GetPlane(0, &pl2);
		if (pl1.DataSize != pl2.DataSize )
			return false;
		return memcmp(pl1.pData, pl2.pData, pl1.DataSize) == 0;
	}

private:
	ExportImageData () {}	
};
GFxDataExporter::GFxDataExporter(GFxExportListener *plog, UByte verboseFlags)
: Logger(plog)
{
    PluginHandler.RegisterLog(Logger);

    FxArgDesc options []=
    {
        {"\nUsage: gfxexport [options] [file.swf] [file(s)...]\n", "", FxArgs::Spacer, ""},
        {"",    "FileNames",        FxArgs::StringList | FxArgs::Positional, NULL, "[file.swf] [file(s)...]"},
        {"help",  "Help",           FxArgs::Flag,      "",   "Print command line options."},
        {"platform", "Platform",FxArgs::StringOption, "","Platform specific formats (appropriate dll plugin is required)\n"
        "              3ds        - Nintendo 3DS\n"
        "              iphone     - Apple iPhone/Ipad/Ipod Touch\n"
        "              vita       - Sony Playstation Vita\n"
        "              wiiu       - Nintendo WiiU\n"
        "              x360       - Microsoft XBox 360\n"
        "              android    - Google Android devices\n"
        "              ps3        - Sony Playstation 3\n"
        "              ps4        - Sony Playstation 4\n"
        },

        {"pu",   "PlatformUncompressed",    FxArgs::Flag,      "",   "Use uncompressed platform specific formats"},
        {"i",  "ImagesFormat",      FxArgs::StringOption, "tga", "<format>   Specifies output format for exporting image data\n"
        "              where <format> is one of the following:\n"
        "              TGA - Truevision (Targa or TGA)\n"
        "              DDS - DirectDraw Surface (DDS), using NVidia Texture Tools\n"
        "              PVR - PVR compression format, using PVRTexLib\n"
        "              ETC - Ericsson compression format, using PVRTexLib\n"
        "              ORIG - Original format: JPEG data will be saved as JPG files\n"
        "              MULT - Save DDS+TGA+PVR to support different GPUs \n"
        "              BC7  - DX11 BC7 compression (requires texconv.exe) \n"
        "              and lossless images will be saved as TGAs.\n"
#ifdef SF_TIFF_SUPPORT
        "              TIFF - 32-bit Tagged Image File Format\n"
#endif // SF_TIFF_SUPPORT
        },
        {"s",  "SoundExt",       FxArgs::StringOption, "", "<extension>   Specifies extension for exporting sound file\n"},
        {"strip_images",  "JustStripImages", FxArgs::Flag,      "",   "Just strip images, do not write them in files."},
        {"d",  "OutputRootDir",     FxArgs::StringOption, "", "Destination directory for exported data. If not specified,\n"
        "                files are stored in the directory containing the SWF."},
        {"dll",  "LoadPlugin",      FxArgs::StringList, NULL, "<path> [options] Load the specified DLL plugin.\n"
        "              Options should not be preceded by '-' .\n"},
        {"sd",  "DoCreateSubDirs",  FxArgs::Flag,      "",   "Create subdirectories for each SWF file using the SWF filename."},
        {"c",   "DoCompress",       FxArgs::Flag,      "",   "Write compressed stripped .gfx file(s)."},
        {"modstamp", "ModStamp",    FxArgs::Flag, "",   "Do not run if swf file is older then existing gfx"},
        {"o",   "OutputGfxDir",     FxArgs::StringOption, "",   "Specify the directory to write resulting .gfx files.  The\n"
        "              default is to use the same directory as the generated images.\n"
        "              Note that the .gfx file cannot be used from that directory\n"
        "              without help from a custom FileOpener class."},
        {"p",   "Prefix",           FxArgs::StringOption, "",   "Specifies prefix to add to the name of each exported resource.\n"
        "              By default, the original SWF filename is used as prefix.\n"},
        {"lwr", "ToLowercase",      FxArgs::Flag,      "",   "Force all exported files to have lowercase names."},
        {"upr", "ToUppercase",      FxArgs::Flag,      "",   "Force all exported files to have uppercase names."},
        {"ne",  "NoExportNames",    FxArgs::Flag,      "",   "Do not use export names as file names for images."},
        {"q",   "Quiet",            FxArgs::Flag,      "",   "Quiet mode (suppress all output)."},
        {"x",   "QuietProgress",    FxArgs::Flag,      "",   "Quiet progress mode (suppress progress output)."},
        {"list", "GenFileList",     FxArgs::Flag,      "",   "Write list of generated files into file."},
        {"info", "Info",            FxArgs::Flag,      "",   "Get information about exported images, but do not export them.\n"
        "              Information is provided for the current set of options.\n"},
        {"share_images", "ShareImages",    FxArgs::Flag,      "",   "Try to reuse images in the destination directory.\n"},
        {"replace_images", "ReplaceImages",    FxArgs::Flag,  "",  "Try to substitute embedded images with external ones.\n"},
        {"id",  "ImgSubstDir",     FxArgs::StringOption, "", "Directory with images for substitution."},
        {"rescale",  "Rescale",     FxArgs::StringList, NULL, "<nearest | hi | low | nextlow | mult4> \n"
        "              Rescale image to nearest, next highest or next lowest power\n"
        "              of two or multiple of 4. 'hi' is default for compressed DDS.\n"
        "              Optional filtering for rescaling\n" 
        "              (after mandatory rescale mode option):\n"
        "               Box\n"
        "               Triangle\n"
        "               Quadratic\n"
        "               Cubic   (default)\n"
        "               Catrom\n"
        "               Mitchell\n"
        "               Gaussian\n"
        "               Sinc\n"
        "               Bessel\n"
        "               Hanning\n"
        "               Hamming\n"
        "               Blackman\n"
        "               Kaiser\n"},
        
        {"sq",   "SquareImages",   FxArgs::Flag,    "",   "Force -rescale and -pack options to produce square images\n"},
        {"pack",     "PackImages",   FxArgs::Flag,      "",   "Pack images into large textures.\n"},
        {"packsize", "PackImageSize",FxArgs::IntOption, "1024","Max size of packed textures (-pack).\n"},

		{"ptresize", "PackTextureResize",FxArgs::StringOption, "","Resize packed textures (-pack) <no | p2 | mult4>.\n"
		"              no      - No rescale\n"
		"              p2      - Resize to higher power of 2 (default)\n"
		"              mult4   - Resize to higher multiple of 4\n"
        "              mult128 - Resize to higher multiple of 128\n"},
        
        {"pad", "PadTextures",FxArgs::Flag, "","Pad textures to power of 2 or multiple of 4 (set by ptresize)\n"
        "              this is alias for -pack -packsize 2"},

        {"DDS options ('-i DDS' is specified):","",FxArgs::Spacer,""},
        {"d0",   "UncompressedDDS",  FxArgs::Flag,      "",   "Write uncompressed DDS"},
        {"d1",   "DXT1",             FxArgs::Flag,      "",   NULL},
        {"d1c",  "DXT1Allowed",      FxArgs::Flag,      "",   "Use DXT1 for RGB data without alpha channel"},
        {"d1a",  "DXT1a",            FxArgs::Flag,      "",   "Use DXT1 for RGB data with alpha channel"},      
        {"d3",   "DXT3",             FxArgs::Flag,      "",   "Use DXT3 for RGB data with alpha channel"},
        {"d5",   "DXT5",             FxArgs::Flag,      "",   "Use DXT5 (default) for RGB data with alpha channel"},   
        {"qf",   "quick",            FxArgs::Flag,      "",   "Fast compression method."},
        {"qn",   "quality_normal",   FxArgs::Flag,      "",   "Normal quality compression (default)."},
        {"qp",   "quality_production", FxArgs::Flag,    "",   "Production quality compression."},
        {"qh",   "quality_highest",  FxArgs::Flag,      "",   "Highest quality compression (this can be very slow)."},
		{"mipmap", "MipMap",         FxArgs::Flag,      "",   "Generate mipmaps in DDS files (by default mipmaps are generated only for fonts)."},
        {"mipfilter", "MipFilter",   FxArgs::StringOption, "Box",   "Mipmap filtering options: \n"
        "               Box (default) \n"
        "               Triangle\n"
        "               Kaiser\n"},

        {"PVR options ('-i PVR' is specified):","",FxArgs::Spacer,""},
        {"pv2",   "PVRTC2",          FxArgs::Flag,      "",   "Use 2 bit per pixel compression. \n"},
        {"pv4",   "PVRTC4",          FxArgs::Flag,      "",   "Use 4 bit per pixel compression. \n"},

        {"Font options:","",FxArgs::Spacer,"", ""},
        {"fc",   "CompactFonts",       FxArgs::Flag,         "",  "Compact fonts."},
        {"fcl",  "FontNormalizedSize", FxArgs::IntOption,    "256",  "<size> Set compact fonts normalized size (default size is 256)."},
        {"fcm",  "FontMergeEdges",     FxArgs::Flag,         "",  "Merge edges for compact fonts."},
	    {"fonts", "ExportFonts",       FxArgs::Flag,         "",  "Export font textures."},
        {"fntlst", "ExportFontList",   FxArgs::Flag,         "",  "Export font list and textfield/font map (.fnt-file)."},
        {"fns",  "TextureGlyphNominalSize", FxArgs::IntOption, "48", "<size> Nominal size of texture glyph (in pixels)."},
        {"fpp",  "TextureGlyphPadPixels",   FxArgs::IntOption,  "3", "<n> Space, in pixels, to leave around the individual glyph image."},
        {"fts",  "FontTextureSize",      FxArgs::StringOption, "1024x1024", "<WXH>  The dimensions of the textures that the glyphs get packed\n"
        "              into. Default size is %dx%d. To specify square texture only\n"
        "              one dimension can be specified, e.g.: '-fts 128' is 128x128.\n"
        "              '-fts 512x128' specifies rectangle texture."},
        {"fs",   "UseSeparateFontTextures", FxArgs::Flag,        "",   "Force separate textures for each font.\n" 
        "              By default, fonts share textures."},
        {"strip_font_shapes",   "GlyphsStripped",  FxArgs::Flag, "",   "Do not write font shapes in resulting GFX file."},
        {"fi",  "FontImagesFormat",    FxArgs::StringOption, NULL , "<format>  Specifies output format for font textures\n"
        "              where <format> is one of the following:\n"
        "                TGA8  - 8-bit Targa TrueVision (grayscaled)\n"
        "                TGA24 - 24-bit Targa TrueVision (grayscaled)\n"
        "                TGA32 - 32-bit Targa TrueVision\n"
        "                DDS8  - 8-bit DDS A8\n"
#ifdef SF_TIFF_SUPPORT
        "                TIFF  - 32-bit Tagged Image File (white color,\n"
        "                        alpha channel contains font data)\n"
#endif // SF_TIFF_SUPPORT
        "              By default, if image format (-i option) is TGA then TGA8\n"
        "              is used for font textures; otherwise DDS A8."},
        {"\nDefault edit-textfield (dynamic/input) text report options:","",FxArgs::Spacer,"",""},
        {"det",   "ExportDefaultEditText",      FxArgs::Flag,      "",   "Export list of unique default dynamic/input textfields\n" 
        "              (.det-file; UTF-8)."},
        {"ddt",   "DefaultDynamicTextOnly",     FxArgs::Flag,      "",   "Export list of unique default dynamic textfields\n"
        "              (.ddt-file; UTF-8)."}, 
        
        {"\nFSCommand use report options:","",FxArgs::Spacer,"",""},
        {"fstree",   "FsCommandsAsTree",     FxArgs::Flag,      "",   "Export list of FSCommands as a tree (.fst-file)."},
        {"fslist",   "FsCommandsAsList",     FxArgs::Flag,      "",   "Export list of FSCommands as a sorted list (.fsl-file)."},    
        {"fsparams", "FsCommandsParams",     FxArgs::Flag,      "",   "Try to find parameters for FSCommands."},
        {"ndp",      "NoDebugPopups",        FxArgs::Flag,      "",   "Disable Windows debug popups"},


        {"as3", "ExportAS3code",     FxArgs::Flag,      "",   NULL},   
        {"", "", FxArgs::ArgEnd, "", ""} 
    };

    Arguments.AddDesriptions(options);
    
    // Values will be overwritten by the options even if option was not set in command line 
    Info            = false;
    Quiet           = false;
    QuietProgress   = false;
    DoCompress      = false;
    DoCreateSubDirs = false;
    UncompressedTexture = false;
    DXT1Allowed     = false;
    DXTn            = 0;
    PVRn            = 4;
    GenMipMapLevels = true;
    Quality         = ExportImageHelper::QualityNormal;
    Rescale         = ExportImageHelper::RescaleNone;
    RescaleFilter   = ExportImageHelper::FilterBicubic;
    MipFilter       = ExportImageHelper::nvFilterBox;
    ToUppercase     = false;
    ToLowercase     = false;
    NoExportNames   = false;
    ImagesFormat    = "tga";
    SoundExt    = "";
    ExportSounds    = false;
    JustStripImages = false;
    ShareImages     = false;

    ExportDefaultEditText = false;
    DefaultDynamicTextOnly = false;

    ExportFsCommands = false;
    FsCommandsAsTree = false;
    FsCommandsAsList = false;
    FsCommandsParams = false;

    ExportFonts             = false;
    ExportFontList          = false;
    GlyphsStripped          = false;
    TextureGlyphNominalSize = 48;
    TextureGlyphPadPixels   = 3;
    FontTextureWidth        = 1024;
    FontTextureHeight       = 1024;
    FontImagesFormat        = FileTypeConstants::File_TGA;
    FontImagesBits          = 8;
    UseSeparateFontTextures = false;
    CompactFonts            = false;
    FontNormalizedSize      = 256;
    FontMergeEdges          = false;    

    ExportAS3code           = false;

    Ptr<ImageCreator> pimageCreator = *new ImageCreator();

    bSquareImages = false;
    Ptr<ASSupport> pAS2Support = *new AS2Support();
    ExpLoader.SetAS2Support(pAS2Support);
    Ptr<ASSupport> pAS3Support = *new AS3Support();
    ExpLoader.SetAS3Support(pAS3Support);

	SF::Ptr<GFx::ImageFileHandlerRegistry> pimgReg = *new GFx::ImageFileHandlerRegistry();
	pimgReg->AddHandler(&SF::Render::JPEG::FileReader::Instance);
#if defined (SF_ENABLE_LIBPNG)
	pimgReg->AddHandler(&SF::Render::PNG::FileReader::Instance);
#endif //SF_ENABLE_LIBPNG
	pimgReg->AddHandler(&SF::Render::TGA::FileReader::Instance);
	pimgReg->AddHandler(&SF::Render::DDS::FileReader::Instance);
    pimgReg->AddHandler(&SF::Render::SIF::FileReader::Instance);
	ExpLoader.SetImageFileHandlerRegistry(pimgReg);
#if 0
    ExpLoader.SetImageCreator(pimageCreator);
#endif
    ExpLoader.SetFontPackParams(Ptr<FontPackParams>(*new FontPackParams));


    if (verboseFlags)
    {
        Ptr<ParseControl> pparseControl = *new ParseControl(verboseFlags);
        ExpLoader.SetParseControl(pparseControl);
    }

    class ExportFileOpener : public GFx::FileOpener
    {
    public:
        virtual File* OpenFile(const char* purl, 
            int flags = FileConstants::Open_Read|FileConstants::Open_Buffered, 
            int mode = FileConstants::Mode_ReadWrite)
        {
            SF_UNUSED2(mode, flags);
            // Buffered file wrapper is faster to use because it optimizes seeks.
            return new SysFile(purl);
        }
    };
    Ptr<GFx::FileOpener> pfileOpener = *new ExportFileOpener;
    ExpLoader.SetFileOpener(pfileOpener);
    //?mLoader.SetLog(Ptr<GFxLog>(*new Log()));

    class ExportProgressHandler : public ProgressHandler
    {
        GFxDataExporter* pExporter;
    public:

        ExportProgressHandler(GFxDataExporter* pexporter) : pExporter(pexporter) { }

        virtual void    LoadTagUpdate(const ProgressHandler::TagInfo& info, bool calledFromDefSprite)
        {
            pExporter->LoadTagProgressCallback(info, calledFromDefSprite);
        }

        virtual void    ProgressUpdate(const ProgressHandler::Info& info)
        {
            pExporter->ProgressCallback(info);
        }
    };
    Ptr<ProgressHandler> pprogressHandler = *new ExportProgressHandler(this);
    ExpLoader.SetProgressHandler(pprogressHandler);
}

GFxDataExporter::~GFxDataExporter()
{
    ClearOriginalImageData();

    Hash<int, GFxImageExporter*>::ConstIterator iter = ImageExporters.Begin();
    for(; iter != ImageExporters.End(); ++iter)
    {
        delete iter->Second;
    }
}

File*  GFxDataExporter::FileOpener(const char* url)
{
    return new SysFile(url);
}

String GFxDataExporter::CutPath(const String& filename)
{
    for (int n = (int)filename.GetSize() - 1; n >= 0; --n)
    {
        char c = filename[n];
        if (c == '\\' || c == '/')
        {
            String nopath(filename.ToCStr() + n + 1);
            return nopath;
        }
    }
    return filename;
}

String GFxDataExporter::CutExtension(const String& filename)
{
    String noext(filename);
    for (int n = (int)filename.GetSize() - 1; n >= 0; --n)
    {
        if (filename[n] == '.')
            return noext.Substring(0, n);
    }
    return noext;   
}

void GFxDataExporter::ExtractImage(ImageBase* pimageInfoBase,
                                   ResourceId rid, const char* pswfName, const char* pdestPath, 
                                   const char* pexportName, ImageResource* presource)

{
    SF_ASSERT(pimageInfoBase);
    String errorMsg;

    GFxExportImageDescr   imgDescr;
    ExportImageHelper::RescaleTypes rescale = Rescale;

    if (!Quiet && !Info)
    {
        ++ImagesProcessedCount;
        if (!QuietProgress)
        {            
            Logger->OnProcProgress(GFxExportListener::PROG_Images, int(ImagesProcessedCount*100/TotalImageCount));
        }
    }

    UInt32 characterId = rid.GetIdValue();

    if (presource && presource->GetBaseImageId().GetIdValue())
    {
		SF_ASSERT(pimageInfoBase->GetImageType() == ImageBase::Type_SubImage);
        ResourceId baseimage = presource->GetBaseImageId();
        SubImage* psubimage = static_cast<SubImage*>(pimageInfoBase);

        imgDescr.CharacterId  = characterId;
        imgDescr.pImage   = NULL;
        if (pexportName)
            imgDescr.ExportName = pexportName;
        imgDescr.SwfName      = pswfName;
        imgDescr.UsedRect = psubimage->GetRect();
        imgDescr.BaseImageId = baseimage.GetIdValue();
        imgDescr.IsPackedImage = 0;
        ImageDescriptors.Set(characterId, imgDescr);

        if (ImageDescriptors.Get(baseimage.GetIdValue()))
            return;

        // Fall through to export full image once (it is not visited since it wasn't in the resources list at load)
        // It will be named after the first subimage, but the subimage does not write a file so there is no conflict.
        characterId = baseimage.GetIdValue();
        imgDescr.IsPackedImage = 1;
    }
    else
        imgDescr.IsPackedImage = 0;

    if (pimageInfoBase->GetImageType() == Image::Type_SubImage)
    {
        pimageInfoBase = ((SubImage*)pimageInfoBase)->GetBaseImage();
    }
    //else
    //    SF_ASSERT(pimageInfoBase->GetImageType() == Image::Type_Image);

    Ptr<Render::Image> pimage = 0;
    Image* peimage = static_cast<Image*>(pimageInfoBase);
    SF_ASSERT(peimage);
    if (!PackImages && ReplaceImages && pexportName && pImgSubstProvider)
    {
        pimage =  *pImgSubstProvider->CreateImage(pexportName);
        if (!pimage)
        {
            Format(errorMsg, "Warning: Can't find substitution image for '{0}'.", pexportName);
            Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
        }
        else if ((pimage->GetSize().Width != peimage->GetSize().Width) || (pimage->GetSize().Height != peimage->GetSize().Height) )
        {
            Format(errorMsg, "Warning: Substitution image for '{0}' has different resolution. Embedded image will be used.", pexportName);
            Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);

            pimage = peimage;
        }
    }
    if(!pimage)
        pimage = peimage;
    
    imgDescr.CharacterId  = characterId;
    imgDescr.pImage   = pimage;
    if (pexportName)
        imgDescr.ExportName = pexportName;
    imgDescr.SwfName      = pswfName;
    imgDescr.TargetWidth  = (pimage->GetSize()).Width;
    imgDescr.TargetHeight = (pimage->GetSize()).Height;
    imgDescr.BaseImageId  = 0;

    int imgFormatId;
		GFxImageExporterParams::ImageExportType expType;
    Render::ImageFormat destImageFormat = pimage->GetFormat();
    if (rid.GetIdType() == ResourceId::IdType_FontImage) // DynFontImage (??)
    {
        // font textures
        imgFormatId     = FontImagesFormat;
        switch(FontImagesBits)
        {
        case 24:    destImageFormat = Render::Image_R8G8B8; break;
        case 32:    destImageFormat = Render::Image_R8G8B8A8; break;
        }
        rescale = ExportImageHelper::RescaleNone; // turn of rescale for font textures.
        expType = GFxImageExporterParams::IET_Font;
    }
    else
    {
        imgFormatId     = GetImageFormatId();
        expType = GFxImageExporterParams::IET_Image;
    }

    imgDescr.Format = UInt16(imgFormatId);

    GFxImageExporter* pimageExporter = GetImageExporter(imgFormatId);
    SF_ASSERT(pimageExporter);

    char buf[1024];
    if (pexportName != 0 && !NoExportNames)
    {
        SFsprintf(buf, 1024, "%s", pexportName);
    }
    else
    {
        char idstr[20];
        rid.GenerateIdString(idstr);
        SFsprintf(buf, 1024, "%s_%s", ((Prefix.GetLength() == 0) ? pswfName : Prefix.ToCStr()), idstr);
    }

    GFxImageExporterParams& ieParams = pimageExporter->InitParams(characterId, this, &imgDescr, pimage, 0, expType);
    imgDescr.ImageName = pimageExporter->MakeFileName(buf);
    if (ToLowercase)
        imgDescr.ImageName = imgDescr.ImageName.ToLower();
    else if (ToUppercase)
        imgDescr.ImageName = imgDescr.ImageName.ToUpper();

    String destPath = pimageExporter->MakePath(pdestPath, imgDescr.ImageName);
    if (Info)
    {
        
//        printf("  %-20s ",imgDescr.ImageName.ToCStr());
    }
    ieParams.Rescale = rescale;

    ImageDescriptors.Set(characterId, imgDescr);
    String tempPath = destPath;
    if (ShareImages)
        tempPath += ".$$$";
    else
        FileList.PushBack(tempPath);

    if(JustStripImages && (rid.GetIdType() == 0)) //?
    {
        // only strip images, do not write them on disk
        return;
    }

    if(!Info)
    {
        if (pimageExporter->GetFormatId() == FileTypeConstants::File_MultiFormat)
            tempPath += ".tga";
        Ptr<File> pfile = *new SysFile(tempPath.ToCStr(),
            File::Open_Write | File::Open_Truncate | File::Open_Create);
        if (!pfile->IsValid() || !pfile->IsWritable())
        {
            Format(errorMsg, "Error: Can't open file '{0}' for writing", tempPath.ToCStr());
            Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
            return;
        }
        ieParams.FilePtr = pfile;

        if (ieParams.Rescale != ExportImageHelper::RescaleNone && pimageExporter->MightBeRescaled())
        {
            Ptr<Render::RawImage> pnewimage = *pExportCompressor->Rescale
                (ieParams.ImagePtr, ieParams.Rescale, RescaleFilter,  destImageFormat, bSquareImages);
            if (pnewimage)
                ieParams.ImagePtr = pnewimage;
            else
            {
                Format(errorMsg, "Can't rescale image: '{0}', error: '{1}'", tempPath.ToCStr(), pExportCompressor->GetLastErrorString());
                Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
            }
        }

        if (pimageExporter->NeedToBeConverted(ieParams.ImagePtr, destImageFormat))
        {
			Render::RawImage* pnewimage = RawImage::Create(destImageFormat, 1, ieParams.ImagePtr->GetSize(), 0);
			SF_ASSERT(pnewimage);
            ImageData imgData;
            pnewimage->GetImageData(&imgData);
            ieParams.ImagePtr->Decode(&imgData,GetImageConvertFunc(destImageFormat, ieParams.ImagePtr->GetFormat()));
            ieParams.ImagePtr = *pnewimage;
        }

        pimageExporter->Write(pExportCompressor, Logger);
        pfile->Close();
    }
    else
    {
        if(!SFstrcmp(pimageExporter->GetName(),"TGA"))
        {
            UInt32 w,h;
            ExportImageHelper::CalculateWH(ieParams.ImagePtr->GetSize().Width,ieParams.ImagePtr->GetSize().Height,w,h,ieParams.Rescale);
            UInt32 pitch = (UInt32)Render::ImageData::GetFormatPitch(destImageFormat,w);
            UInt32 size = pitch*h; 
            TotalMemoryCount+=size;

            GFxExportListener::OutputPayload data;
            data.Name = imgDescr.ImageName.ToCStr();
            data.ImageWidth = w;
            data.ImageHeight = h;
            data.ImageEstSize = size;
            data.FilePath = tempPath;
            Logger->OnOutput(GFxExportListener::OUT_ImageFile, data);
        }
        else if (!SFstrcmp(pimageExporter->GetName(),"DDS"))
        {
            UInt32 w,h;
            ExportImageHelper::CalculateWH(ieParams.ImagePtr->GetSize().Width,ieParams.ImagePtr->GetSize().Height,w,h,
                (ieParams.Rescale==ExportImageHelper::RescaleNone) ? (ExportImageHelper::RescaleBiggestPower2) : (ieParams.Rescale));
            //with current implementation of nvtt DDS is always 8888
            UInt32 size = GDXTHelper::CalculateDDSSize(w,h, /*Image::Image_ARGB_8888*/ destImageFormat,DXTn, UncompressedTexture,GenMipMapLevels, DXT1Allowed);
            TotalMemoryCount+=size;

            GFxExportListener::OutputPayload data;
            data.Name = imgDescr.ImageName.ToCStr();
            data.ImageWidth = w;
            data.ImageHeight = h;
            data.ImageEstSize = size;
            data.FilePath = tempPath;
            Logger->OnOutput(GFxExportListener::OUT_ImageFile, data);
        }
        else if (!SFstrcmp(pimageExporter->GetName(),"ORIG"))
        {
            UInt32 w=ieParams.ImagePtr->GetSize().Width, h=ieParams.ImagePtr->GetSize().Height;
            UInt32 pitch = (UInt32)Render::ImageData::GetFormatPitch(destImageFormat,w);
            UInt32 size = pitch*h; 
            TotalMemoryCount+=size; 

            GFxExportListener::OutputPayload data;
            data.Name = imgDescr.ImageName.ToCStr();
            data.ImageWidth = w;
            data.ImageHeight = h;
            data.ImageEstSize = size;
            data.FilePath = tempPath;
            Logger->OnOutput(GFxExportListener::OUT_ImageFile, data);
        }
    }

    if (!Info)
    {
        // Image sharing computations are only possible if we're writing out
        // every image to compare against.
        bool writtenNew = true;
        if (ShareImages)
        {
            GFxExportImageDescr* pdescr = ImageDescriptors.Get(characterId);
            String sharedFilePath; //We want to put also shared files in the FileList
            if (FindIdenticalImage(destPath, tempPath, &sharedFilePath, pdescr, pimageExporter))
            {
                remove(tempPath);
                writtenNew = false;
                imgDescr = *pdescr;
                FileList.PushBack(sharedFilePath);
            }
            else
            {
                remove(destPath.ToCStr());
                rename(tempPath.ToCStr(), destPath.ToCStr());
                FileList.PushBack(destPath);
            }
        }

        if (writtenNew)
        {
            // ModifyFileReference requires that the file exist on disk, which
            // isn't guaranteed if -info is used.
            String rename;
            if (PluginHandler.ModifyFileReference(GFxExportPlugin::File_Image, destPath, imgDescr.ExportName, rename))
            {
                imgDescr.ImageName = rename;
                ImageDescriptors.Set(characterId, imgDescr);
            }

            GFxExportListener::OutputPayload data;
            data.Name = imgDescr.ImageName.ToCStr();
            data.FilePath = destPath;
            Logger->OnOutput(GFxExportListener::OUT_ImageFile, data);

            ++ImagesWrittenCount;
        }
        else
        {
            ++SharedImagesCount;
        }
        
    }
}

void GFxDataExporter::ExtractFont(FontResource* pfontResource, int fontId, ResourceBinding* pResBinding)
{

    GFxFontTexturesVisitor texturesVisitor(FontTextures, fontId, pfontResource);
    TextureGlyphData* ptextGlyphData = static_cast<TextureGlyphData*>(pfontResource->GetFont()->GetTextureGlyphData());
    if (ptextGlyphData == NULL)
    {
        ptextGlyphData = (TextureGlyphData*)pfontResource->GetFont()->GetTextureGlyphData();
    }
    if (ptextGlyphData)
    {
        ptextGlyphData->VisitTextures(&texturesVisitor, pResBinding);
    }
}


bool GFxDataExporter::FindIdenticalImage( const String& imagePath, const String& filePath, String* presultImagePath, GFxExportImageDescr* pdescr,  GFxImageExporter* pimageExporter )
{
    if ( FindIdenticalImageInDir(imagePath, filePath, presultImagePath, pdescr, pimageExporter) )
        return true;
    return false;
}

bool GFxDataExporter::FindIdenticalImageInDir( const String& imagePath, const String& filePath, String* presultImagePath, GFxExportImageDescr* pdescr, GFxImageExporter* pimageExporter )
{
    SF_ASSERT(pimageExporter);

    Ptr<Render::Image> pimage = *pimageExporter->Read(filePath.ToCStr(), Logger);
    if (pimage)
    {
		ExportImageData srcimgData(pimage);
        UPInt hashCode = srcimgData.ComputeHash();
        Array<String>* pfileNamesArr = SharedImagesHash.Get(hashCode);

        if (pfileNamesArr)
        {
            for (UPInt i = 0, n = pfileNamesArr->GetSize(); i < n; ++i)
            {
                String& fname = (*pfileNamesArr)[i];
                Ptr<Render::Image> pcmpimage = *pimageExporter->Read(fname, Logger);
				ExportImageData cmpimgData(pcmpimage);
                if (pcmpimage)
                {
                    if (srcimgData == cmpimgData)
                    {
                        *presultImagePath = fname;
                        pdescr->ImageName = CutPath(fname);
                        String newname = fname;
                        if (PluginHandler.ModifyFileReference(GFxExportPlugin::File_Image, fname, pdescr->ExportName, newname) )
                        {
                            pdescr->ImageName = newname;
                        }
                        return true;
                    }
                }
            }
        }
        const char* pext = strrchr(imagePath.ToCStr(), '.');
        if (pext != NULL)
        {
            ++pext;
        }
        else
            pext = pimageExporter->GetDefaultExtension();
        SF_ASSERT(pext);

        String wildCard = imagePath;
        const char* pslash1 = strrchr(imagePath, '\\');
        const char* pslash2 = strrchr(imagePath, '/');
        if (pslash1 > pslash2)
            wildCard = wildCard.Substring(0, pslash1 - imagePath + 1);
        else if (pslash2)
            wildCard = wildCard.Substring(0, pslash2 - imagePath + 1);
        else
            wildCard = "./"; // current path
        String path = wildCard;
        wildCard += "*.";
        wildCard += pext;

#ifdef SF_OS_WIN32
        // search the path
        WIN32_FIND_DATAA ffd;
        HANDLE fh = ::FindFirstFileA(wildCard.ToCStr(), &ffd);
        if (fh!=INVALID_HANDLE_VALUE)
        {
            do
            {
                // skip subdirectories
                if (!(ffd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
                {
                    const char *pFileName = ffd.cFileName;
#else
        DIR *fh = opendir(path.ToCStr());
        if (fh)
        {
            struct dirent *ffd;
            while ((ffd = readdir(fh)) != NULL)
            {
#ifndef __CYGWIN__
                if (ffd->d_type == DT_REG)
#else       
                if (0) //? how to detect directory in cygwin?
#endif // __CYGWIN__
                {
                    const char *pFileName = ffd->d_name;
#endif
                    // add the file to the list
                    String fname = path;
                    fname += pFileName;
                    if (fname == filePath || SharedImageNamesSet.Get(fname) != NULL)
                        continue;

                    Ptr<Render::Image> pcmpimage = *pimageExporter->Read(fname, Logger);
                    if (pcmpimage)
                    {
						ExportImageData cmpimgData(pcmpimage);
                        UPInt hashCodeUP = cmpimgData.ComputeHash();
                        Array<String>* pfileNamesArrUP = SharedImagesHash.Get(hashCodeUP);

                        // cache hash-filename assoc.
                        if (pfileNamesArrUP)
                        {
                            pfileNamesArrUP->PushBack(fname);
                        }
                        else
                        {
                            Array<String> fileNamesArr;
                            fileNamesArr.PushBack(fname);
                            SharedImagesHash.Add(hashCodeUP, fileNamesArr);
                        }
                        SharedImageNamesSet.Set(fname, 0);

                        if (srcimgData == cmpimgData)
                        {
                            *presultImagePath = fname;
                            pdescr->ImageName = CutPath(fname);
                            String newname = fname;
                            if ( PluginHandler.ModifyFileReference(GFxExportPlugin::File_Image, fname, pdescr->ExportName, newname) )
                            {
                                pdescr->ImageName = newname;
                            }
                            return true;
                        }
                    }
#ifdef SF_OS_WIN32
                }
            } while (::FindNextFileA(fh,&ffd));
            ::FindClose(fh);
        }
#else
                }
            }
        }
#endif
    }
    return false;
}

#ifdef GFX_ENABLE_SOUND

void GFxDataExporter::ExtractSound(Sound::SoundInfoBase* psoundInfoBase, ResourceId rid, const char* pswfName, 
                                   const char* pdestPath, const char* pexportName)
{
    SF_ASSERT(psoundInfoBase);
    String errorMsg;

    GFxExportSoundDescr   sndDescr;
    Sound::SoundInfo* psoundInfo = static_cast<Sound::SoundInfo*>(psoundInfoBase);

    if (!Quiet && !Info)
    {
        ++SoundsProcessedCount;
        if (!QuietProgress)
        {
            Logger->OnProcProgress(GFxExportListener::PROG_Sounds, int(SoundsProcessedCount*100/TotalSoundCount));
        }
    }

    UInt32 characterId = rid.GetIdValue();

    Ptr<Sound::SoundData> psound = (Sound::SoundData*)psoundInfo->GetSound();
    SF_ASSERT(psound);

    sndDescr.Info   = psoundInfo;
    if (pexportName)
        sndDescr.ExportName = pexportName;
    sndDescr.SwfName      = pswfName;

    GFxSoundExporter* psoundExporter = GetSoundExporter();
    SF_ASSERT(psoundExporter);

    char buf[1024];
    if (pexportName != 0 && !NoExportNames)
    {
        SFsprintf(buf, 1024, "%s", pexportName);
    }
    else
    {
        char idstr[20];
        rid.GenerateIdString(idstr, sizeof(idstr)-1, 'S');
        SFsprintf(buf, 1024, "%s_%s", ((Prefix.GetLength() == 0) ? pswfName : Prefix.ToCStr()), idstr);
    }

    sndDescr.SoundName = psoundExporter->MakeFileName(buf);
    if (ToLowercase)
        sndDescr.SoundName = sndDescr.SoundName.ToLower();
    else if (ToUppercase)
        sndDescr.SoundName = sndDescr.SoundName.ToUpper();

    String destPath = psoundExporter->MakePath(pdestPath, sndDescr.SoundName);

    String tempPath = destPath;
    /*
    if (ShareImages)
        tempPath += ".$$$";

    if(JustStripImages && (rid.GetIdType() == 0)) //?
    {
        // only strip images, do not write them on disk
        return;
    }
    */

//    if (Info)
    {
        GFxExportListener::OutputPayload data;
        data.Name = sndDescr.SoundName.ToCStr();
        data.FilePath = tempPath;
        Logger->OnOutput(GFxExportListener::OUT_SoundFile, data);
    }

    if(!Info)
    {
        Ptr<File> pfile = *new SysFile(tempPath.ToCStr(),
            File::Open_Write | File::Open_Truncate | File::Open_Create);
        if (!pfile->IsValid() || !pfile->IsWritable())
        {            
            Format(errorMsg, "Error: Can't open file '{0}' for writing", tempPath.ToCStr());
            Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
            return;
        }
        FileList.PushBack(tempPath);

        psoundExporter->WriteWaveFile(psound, pfile, &sndDescr.Attrs);
        pfile->Close();
        ++SoundsWrittenCount;
    }
    SoundDescriptors.Set(characterId, sndDescr);
}

void GFxDataExporter::ExtractSpriteSound(SoundStreamDef* pData, ResourceId rid, const char* pswfName, 
                                         const char* pdestPath, const char* pexportName)
{
    String errorMsg;

    if (!Quiet && !Info)
    {
        ++StreamSoundsProcessedCount;
        if (!QuietProgress)
        {
            Logger->OnProcProgress(GFxExportListener::PROG_SpriteSounds, StreamSoundsProcessedCount);
        }
    }

    GFxExportSoundDescr   sndDescr;
    UInt32 characterId = rid.GetIdValue();

    SoundStreamDefImpl* pssData = (SoundStreamDefImpl*)pData;
    if (pssData->pSoundInfo)
    {
        Ptr<Sound::AppendableSoundData> psound = (Sound::AppendableSoundData*)pssData->pSoundInfo->GetSound();
        if (psound && psound->GetDataSize() > 0)
        {
            sndDescr.Info   = pssData->pSoundInfo;
            sndDescr.StartFrame = pssData->StartFrame;
            sndDescr.LastFrame = pssData->LastFrame;

            if (pexportName)
                sndDescr.ExportName = pexportName;
            sndDescr.SwfName      = pswfName;

            GFxSoundExporter* psoundExporter = GetSoundExporter();
            SF_ASSERT(psoundExporter);

            char buf[1024];
            if (pexportName != 0 && !NoExportNames)
            {
                SFsprintf(buf, 1024, "%s_streamsound", pexportName);
            }
            else
            {
                char idstr[20];
                rid.GenerateIdString(idstr, sizeof(idstr)-1, 'S');
                SFsprintf(buf, 1024, "%s_%s_streamsound", ((Prefix.GetLength() == 0) ? pswfName : Prefix.ToCStr()), idstr);
            }

            sndDescr.SoundName = psoundExporter->MakeFileName(buf);
            if (ToLowercase)
                sndDescr.SoundName = sndDescr.SoundName.ToLower();
            else if (ToUppercase)
                sndDescr.SoundName = sndDescr.SoundName.ToUpper();

            String destPath = psoundExporter->MakePath(pdestPath, sndDescr.SoundName);

            String tempPath = destPath;
            /*
            if (ShareImages)
            tempPath += ".$$$";

            if(JustStripImages && (rid.GetIdType() == 0)) //?
            {
            // only strip images, do not write them on disk
            return;
            }
            */

//            if (Info)
            {
                GFxExportListener::OutputPayload data;
                data.Name = sndDescr.SoundName.ToCStr();
                data.FilePath = tempPath;
                Logger->OnOutput(GFxExportListener::OUT_SpriteSoundFile, data);
            }

            if(!Info)
            {
                Ptr<File> pfile = *new SysFile(tempPath.ToCStr(),
                    File::Open_Write | File::Open_Truncate | File::Open_Create);
                if (!pfile->IsValid() || !pfile->IsWritable())
                {
                    Format(errorMsg, "Error: Can't open file '{0}' for writing", tempPath.ToCStr());
                    Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                    return;
                }
                FileList.PushBack(tempPath);

                psoundExporter->WriteWaveFile(psound, pfile, &sndDescr.Attrs);
                pfile->Close();
                ++StreamSoundsWrittenCount;
            }
        }
    }

    SoundDescriptors.Set(characterId, sndDescr);

}
#endif

int GFxDataExporter::Load(const String& fileName)
{
    String errorMsg;

    FirstTagOffset = 0;

    // Get info about the width & height of the movie.
    if (!ExpLoader.GetMovieInfo(fileName, &ExpMovieInfo))
    {
        Format(errorMsg, "Error: Failed to get info about '{0}'", fileName.ToCStr());
        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
        return 0;
    }

    // Do not allow stripped files as input, since they would cause a crash.
    if (ExpMovieInfo.IsStripped())
    {
        Format(errorMsg, "Error: '{0}' is already stripped, gfxexport requires SWF files", fileName.ToCStr());
        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
        return 0;
    }


    Ptr<FontPackParams> pfontPackParams = ExpLoader.GetFontPackParams();
    SF_ASSERT(pfontPackParams);
    FontPackParams::TextureConfig textureGlyphCfg;
    textureGlyphCfg.TextureWidth     = FontTextureWidth;
    textureGlyphCfg.TextureHeight    = FontTextureHeight;
    textureGlyphCfg.NominalSize      = TextureGlyphNominalSize;
    textureGlyphCfg.PadPixels        = TextureGlyphPadPixels;

    pfontPackParams->SetTextureConfig(textureGlyphCfg);
    pfontPackParams->SetUseSeparateTextures(UseSeparateFontTextures);

    TextureGlyphData::TextureConfig actualCfg;
    pfontPackParams->GetTextureConfig(&actualCfg);
    if (actualCfg.NominalSize != TextureGlyphNominalSize)
    {
        TextureGlyphNominalSize = actualCfg.NominalSize;

        Format(errorMsg, "Warning: Glyph size is corrected to {0}", TextureGlyphNominalSize);
        Logger->OnMessage(GFxExportListener::MSG_Warning, errorMsg);
    }

    class GFxExportImgSubstProvider : public ImageSubstProvider
    {
        String ImageDir;
    public:
        GFxExportImgSubstProvider(const String imageDir, GFxExportListener *logger): ImageDir(imageDir), Logger(logger)
        {
        }
        virtual Image* CreateImage(const char* pname) 
        {
            if(!pname)
                return NULL;
            String filePath = ImageDir + pname;
            const char* pext = strstr(pname, ".");
            if (!pext) //if no "." in the Linkage ID, assume png file
                filePath += ".png";
            //GString filePath = ImageDir + GString(pname, UPInt(pext - pname));
            Ptr<File> pin = *new SysFile(filePath, File::Open_Read);
            if (MatchFileExtension(filePath.ToCStr(), "png"))
            {
#if defined (SF_ENABLE_LIBPNG)
                return Render::PNG::FileReader::ReadImage(pin);
            }
#else
				String errorMsgPNG;
                Format(errorMsgPNG, "Error: PNG support has been disabled.");
                Logger->OnMessage(GFxExportListener::MSG_Error, errorMsgPNG);
				return NULL;
			}
#endif //SF_ENABLE_LIBPNG
            else if (MatchFileExtension(filePath.ToCStr(), "tga"))
                return Render::TGA::FileReader::ReadImage(pin);
            else
            {
                String errorMsg;
                Format(errorMsg, "Error: Unexpected image file extension '{0}'", pname);
                Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                return NULL;
            }
        }
    private:
        GFxExportListener    *Logger;
        bool    MatchFileExtension(const char *pname, const char *pext)
        {
            size_t nameLen = strlen(pname);
            size_t extLen  = strlen(pext);
            if (nameLen <= extLen)
                return 0;
            return (SFstricmp(pname + (nameLen - extLen), pext) == 0);
        }
    };
    if (ReplaceImages)
        pImgSubstProvider = *new GFxExportImgSubstProvider(ImgSubstDir, Logger);


    if (PackImages)
    {
        Ptr<ImagePackParams> ppacker = *new ImagePackParams();
        ImagePackParams::TextureConfig config;
        ppacker->GetTextureConfig(&config);
        config.TextureHeight = config.TextureWidth = PackImageSize;
        config.Square = bSquareImages || (GetImageFormatId() == FileTypeConstants::File_PVR);
        config.SizeOptions = PackImageResize;
        ppacker->SetTextureConfig(config);

        ExpLoader.SetImagePackParams(ppacker);
    }

    // Load the actual new movie and create an instance.
    // Don't use library: this will ensure that the memory is released.

    // Disable import loading; import files are processed individually.
    unsigned loadFlags = Loader::LoadDisableImports;

    MovieDefPtr = *ExpLoader.CreateMovie(fileName, loadFlags | Loader::LoadKeepBindData);
    if (!MovieDefPtr)
    {
        Format(errorMsg, "Error: Failed to create a movie from '{0}'", fileName.ToCStr());
        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
        return 0;
    }

    return 1;
}
void GFxDataExporter::ProgressCallback(const ProgressHandler::Info& info)
{
    GFxDataExporter* pthis = this;

    if (!pthis->Quiet && !pthis->Info)
    {
        if (!pthis->QuietProgress)
        {
            Logger->OnProcProgress(GFxExportListener::PROC_Swf, int(info.BytesLoaded*100/info.TotalBytes));
        }
    }

}
void GFxDataExporter::LoadTagProgressCallback(const ProgressHandler::TagInfo& info, bool calledFromDefSprite)
{
    TagInfo tagInfo;
    tagInfo.TagDataOffset = info.TagDataOffset;
    tagInfo.TagLength     = info.TagLength;
    tagInfo.TagOffset     = info.TagOffset;
    tagInfo.TagType       = (TagType)info.TagType;

    GFxDataExporter* pthis = this;
    // save offset of the very first tag
    if (pthis->FirstTagOffset == 0)
    {
        pthis->FirstTagOffset = tagInfo.TagOffset;
    }

#ifdef GFX_ENABLE_SOUND
    if (ExportSounds && tagInfo.TagType == 39 )  // DefineSprite
    {
        bool add_tag = true;
        if (pthis->StreamSoundsData.GetSize() > 0)
        {
            StreamSoundData& ssd = *pthis->StreamSoundsData.Last();
            if (!ssd.IsHeadTagSet)
            {
                ssd.DefSpriteTag = tagInfo;
                add_tag = false;
            }
        } 
        if (add_tag)
        {
            StreamSoundData ssd;
            ssd.DefSpriteTag = tagInfo;
            ssd.IsHeadTagSet = false;
            pthis->StreamSoundsData.PushBack(ssd);
        }
        pthis->TagsToBeRemoved.PushBack(tagInfo);
    }
#endif

    if (tagInfo.TagType == 12 ||  // DoAction
        tagInfo.TagType == 59 ||  // DoInitAction
        tagInfo.TagType == 7  ||  // DefineButton
        tagInfo.TagType == 34  || // DefineButton2
        tagInfo.TagType == 39  || // DefineSprite
        tagInfo.TagType == 26  || // PlaceObject2
        tagInfo.TagType == 70)    // PlaceObject3
        pthis->TagsWithActions.PushBack(tagInfo);

    if (tagInfo.TagType == 12 ||  // DoAction
        tagInfo.TagType == 59)  // DoInitAction
        pthis->ActionTags.PushBack(tagInfo);

    if (tagInfo.TagType == 82)  // DoAbc
        pthis->TagsActions3.PushBack(tagInfo);

    if (tagInfo.TagType == 6 ||     // DefineBits
        tagInfo.TagType == 8 ||     // JPEGTables
        tagInfo.TagType == 21 ||    // DefineBitsJPEG2
        tagInfo.TagType == 35 ||    // DefineBitsJPEG3
        tagInfo.TagType == 20 ||    // DefineBitsLossless
        tagInfo.TagType == 36 ||    // DefineBitsLossless2
#ifdef GFX_ENABLE_SOUND
        (ExportSounds && tagInfo.TagType == 14) ||    // DefineSound
#endif
        tagInfo.TagType == 0)       // End
    {
        if (!(tagInfo.TagType == 0 && calledFromDefSprite))
            pthis->TagsToBeRemoved.PushBack(tagInfo);
    }
#ifdef GFX_ENABLE_SOUND
    else if (ExportSounds && 
             (tagInfo.TagType == 18 ||    // SoundStreamHead
              tagInfo.TagType == 45))     // SoundStreamHead2
    {
        if (calledFromDefSprite)
        {
            SF_ASSERT(pthis->StreamSoundsData.GetSize() > 0);
            StreamSoundData& ssd = *pthis->StreamSoundsData.Last();
            SF_ASSERT(!ssd.IsHeadTagSet);
            ssd.IsHeadTagSet = true;
            ssd.HeadTag = tagInfo;
            //pthis->TagsToBeRemoved.PushBack(ssd.DefSpriteTag);           
        }
        else
        {
            pthis->TagsToBeRemoved.PushBack(tagInfo);
        }
    } 
    else if (ExportSounds && tagInfo.TagType == 19)    // SoundStreamBlock
    {
        if (calledFromDefSprite)
        {
            SF_ASSERT(pthis->StreamSoundsData.GetSize() > 0);
            StreamSoundData& ssd = *pthis->StreamSoundsData.Last();
            SF_ASSERT(ssd.IsHeadTagSet);
            ssd.BlockTags.PushBack(tagInfo);
        }
        else 
        {
            pthis->TagsToBeRemoved.PushBack(tagInfo);
        }
    }
#endif
    else if ((ExportFonts || GlyphsStripped || CompactFonts) && (
        tagInfo.TagType == 10 ||    // DefineFont
        tagInfo.TagType == 48 ||    // DefineFont2
        tagInfo.TagType == 75))     // DefineFont3
    {
        pthis->TagsToBeRemoved.PushBack(tagInfo);
    }
    // Do not remove tags if they are inside the sprite and ExportSounds == true,
    // because in this case the sprite's tag might be fully re-written because of
    // sound.
    else if (!(ExportSounds && calledFromDefSprite) && PluginHandler.WantsTag(tagInfo.TagType) )
    {
        pthis->TagsToBeRemoved.PushBack(tagInfo);
    }
    else
    {
        // schedule removal for all tags w/o loaders
        // Do not remove tags if they are inside the sprite and ExportSounds == true,
        // because in this case the sprite's tag might be fully re-written because of
        // sound.
        if (!(ExportSounds && calledFromDefSprite) && !pthis->ExpLoader.DoesNeedTag(tagInfo.TagType))
        {
            //printf("Going to remove tag = %d\n", tagInfo.TagType);
            pthis->TagsToBeRemoved.PushBack(tagInfo);
        }
    }
}


int GFxDataExporter::GetImageFormatId() const
{
    const String format = ImagesFormat;
    Hash<int, GFxImageExporter*>::ConstIterator iter = ImageExporters.Begin();
    for(; iter != ImageExporters.End(); ++iter)
    {
        if (format.CompareNoCase(iter->Second->GetName()) == 0)
            return iter->First;
    }
    return FileTypeConstants::File_Unknown;
}

void GFxDataExporter::WriteTagHeader(File* pout, unsigned tag, unsigned len)
{
    SF_ASSERT(tag < 1024);
    if (len < 63)
    {
        // short form of tag
        pout->WriteUInt16(UInt16((tag << 6) | (len & 0x3F)));
    }
    else
    {
        // long form of tag
        pout->WriteUInt16(UInt16(tag << 6) | 0x3F);
        pout->WriteUInt32(UInt32(len));
    }
}

void GFxDataExporter::WriteExternalImageTag(File* pout, UInt32 characterId, const GFxExportImageDescr& imgDescr)
{
    // utilizes the tag 1009 (unused in normal SWF): the format is as follows:
    // Header           RECORDHEADER    1009
    // CharacterID      UI32
    // BitmapFormat     UI16            0 - Default, as in 1001 tag
    //                                  1 - TGA
    //                                  2 - DDS
    // TargetWidth      UI16
    // TargetHeight     UI16
    // ExportNameLen    UI8
    // ExportName       UI8[ExportNameLen]
    // FileNameLen      UI8             with extension
    // FileName         UI8[FileNameLen]

    if (imgDescr.BaseImageId)
    {
        WriteSubImageTag(pout, characterId, imgDescr.BaseImageId, imgDescr.UsedRect);
        return;
    }

    UByte len = UByte(4 + 2 + 2 + 2 + 1 + imgDescr.ExportName.GetSize() + 1 + imgDescr.ImageName.GetSize());
    WriteTagHeader(pout, 1009, len);
    pout->WriteUInt32(characterId);
    pout->WriteUInt16(imgDescr.Format);
    //pout->WriteUInt16((JustStripImages)?0:imgDescr.Format);
    pout->WriteUInt16(UInt16(imgDescr.TargetWidth));
    pout->WriteUInt16(UInt16(imgDescr.TargetHeight));

    pout->WriteUByte(UByte(imgDescr.ExportName.GetSize()));
    pout->Write((const UByte*)imgDescr.ExportName.ToCStr(), (int)imgDescr.ExportName.GetSize());

    pout->WriteUByte(UByte(imgDescr.ImageName.GetSize()));
    pout->Write((const UByte*)imgDescr.ImageName.ToCStr(), (int)imgDescr.ImageName.GetSize());
}

void GFxDataExporter::WriteSubImageTag( File* pout, UInt32 characterId, UInt32 baseCharId, const ImageRect& rect )
{
    // utilizes the tag 1008 (unused in normal SWF): the format is as follows:
    // Header           RECORDHEADER    1008
    // CharacterID      UI16
    // ImageCharacterID UI16
    // Left             UI16            Area of ImageCharacter to use for this resource
    // Top              UI16
    // Right            UI16
    // Bottom           UI16

    UByte len = UByte(2 * 6);
    WriteTagHeader(pout, 1008, len);
    pout->WriteUInt16(UInt16(characterId));
    pout->WriteUInt16(UInt16(baseCharId));
    pout->WriteUInt16(UInt16(rect.x1));
    pout->WriteUInt16(UInt16(rect.y1));
    pout->WriteUInt16(UInt16(rect.x2));
    pout->WriteUInt16(UInt16(rect.y2));
}


void GFxDataExporter::WriteGlyphTextureInfo(File* pout, unsigned textureId, const GFxFontTextureDescr& textureDescr)
{
   Ptr<FontPackParams> pfontPackParams = ExpLoader.GetFontPackParams();
    SF_ASSERT(pfontPackParams);
    const GFxExportImageDescr* descr = ImageDescriptors.Get(textureId);

    // Glyphs' texture info tags
    // utilizes the tag 1002 (unused in normal SWF): the format is as follows:
    // Header           RECORDHEADER    1002
    // TextureID        UI32            Texture ID
    // TextureFormat    UI16            0 - Default, as in 1001 tag
    //                                  1 - TGA
    //                                  2 - DDS
    // FileNameLen      UI8             name of file with texture's image (without extension)
    // FileName         UI8[FileNameLen]
    // TextureWidth     UI16
    // TextureHeight    UI16
    // PadPixels        UI8             
    // NominalGlyphSz   UI16            Nominal height of glyphs
    // NumTexGlyphs     UI16            Number of texture glyphs
    // TexGlyphs        TEXGLYPH[NumTexGlyphs]
    // NumFonts         UI16            Number of fonts using this texture
    // Fonts            FONTINFO[NumFonts]  Font infos
    //
    // FONTINFO
    // FontId           UI16
    // NumGlyphs        UI16            Number of texture glyphs in the font from the current texture
    // GlyphIndices     GLYPHIDX[NumGlyphs] Mapping of font glyph's indices to textures' ones (TexGlyphs)
    //
    // GLYPHIDX
    // IndexInFont      UI16            Index in font
    // IndexInTexture   UI16            Index in texture
    //
    // TEXGLYPH:
    // UvBoundsLeft     FLOAT
    // UvBoundsTop      FLOAT
    // UvBoundsRight    FLOAT
    // UvBoundsBottom   FLOAT
    // UvOriginX        FLOAT
    // UvOriginY        FLOAT

    // count number of texture glyphs
    const unsigned numTextureGlyphs = (unsigned)textureDescr.TextureGlyphs.GetSize();
    unsigned i, n;

    // short form of tag
    UInt32 len = UInt32(4 + 2 + 1 + descr->ImageName.GetSize() + 2 + 2 + 1 + 2 + 2 + (numTextureGlyphs * 6 * 4) + 2 + textureDescr.Fonts.GetSize()*(2 + 2) + numTextureGlyphs*(2+2));
    WriteTagHeader(pout, 1002, len);

    pout->WriteUInt32(UInt32(textureId));           // UI32 - textureId

    pout->WriteUInt16(UInt16(FontImagesFormat));
    pout->WriteUByte(UByte(descr->ImageName.GetSize()));
    pout->Write((const UByte*)descr->ImageName.ToCStr(), (int)descr->ImageName.GetSize());

    pout->WriteUInt16(UInt16(textureDescr.pTexture->GetSize().Width));
    pout->WriteUInt16(UInt16(textureDescr.pTexture->GetSize().Height));

    FontPackParams::TextureConfig texGlyphCfg;
    pfontPackParams->GetTextureConfig(&texGlyphCfg);

    pout->WriteUInt8 (UInt8 (texGlyphCfg.PadPixels));
    pout->WriteUInt16(UInt16(texGlyphCfg.NominalSize));

    pout->WriteUInt16(UInt16(numTextureGlyphs));
    for (i = 0, n = numTextureGlyphs; i < n; ++i)
    {
        Ptr<TextureGlyph> ptextureGlyph = textureDescr.TextureGlyphs[i];

        // save TEXGLYPH
        pout->WriteFloat(ptextureGlyph->UvBounds.x1);
        pout->WriteFloat(ptextureGlyph->UvBounds.y1);
        pout->WriteFloat(ptextureGlyph->UvBounds.x2);
        pout->WriteFloat(ptextureGlyph->UvBounds.y2);

        pout->WriteFloat(ptextureGlyph->UvOrigin.x);
        pout->WriteFloat(ptextureGlyph->UvOrigin.y);
    }

    pout->WriteUInt16(UInt16(textureDescr.Fonts.GetSize()));   // UI16 - NumFonts
    for (i = 0, n = (unsigned)textureDescr.Fonts.GetSize(); i < n; ++i)
    {
        // save FONTINFO
        const GFxFontTextureDescr::FontDescr& fntDescr = textureDescr.Fonts[i];
        pout->WriteUInt16(UInt16(fntDescr.FontId));
        pout->WriteUInt16(UInt16(fntDescr.TexGlyphsIndicesMap.GetSize()));

        for(unsigned ui = 0, un = (unsigned)fntDescr.TexGlyphsIndicesMap.GetSize(); ui < un; ++ui)
        {
            const GFxFontTextureDescr::FontDescr::Pair& assoc = fntDescr.TexGlyphsIndicesMap[ui];
            pout->WriteUInt16(UInt16(assoc.IndexInFont));
            pout->WriteUInt16(UInt16(assoc.IndexInTexture));
        }
    }
}
#ifdef GFX_ENABLE_SOUND
void GFxDataExporter::WriteExternalSoundTag(File* pout, UInt32 characterId, const GFxExportSoundDescr& sndDescr)
{
    // utilizes the tag 1006 (unused in normal SWF): the format is as follows:
    // Header           RECORDHEADER    1006
    // CharacterID      UI16
    // SoundFormat      UI16            0 - WAV
    // Bits             UI16
    // Channels         UI16
    // SampleRate       UI32
    // SampleCount      UI32
    // SeekSample       UI32
    // ExportNameLen    UI8
    // ExportName       UI8[ExportNameLen]
    // FileNameLen      UI8             with extension
    // FileName         UI8[FileNameLen]

    // determine long or short form of tag
    UByte len = UByte(2 + 2 + 2 + 2 + 4 + 4 + 4 + 1 + sndDescr.ExportName.GetSize() + 1 + sndDescr.SoundName.GetSize());
    WriteTagHeader(pout, Tag_DefineExternalSound, len);
    pout->WriteUInt16(UInt16(characterId));
    pout->WriteUInt16(sndDescr.Attrs.Format); // format wav
    pout->WriteUInt16(sndDescr.Attrs.Bits);
    pout->WriteUInt16(sndDescr.Attrs.Channels);
    pout->WriteUInt32(sndDescr.Attrs.SampleRate);
    pout->WriteUInt32(sndDescr.Attrs.SampleCount);
    pout->WriteUInt32(sndDescr.Info->GetSound()->GetSeekSample());
    pout->WriteUByte(UByte(sndDescr.ExportName.GetSize()));
    pout->Write((const UByte*)sndDescr.ExportName.ToCStr(), (int)sndDescr.ExportName.GetSize());

    pout->WriteUByte(UByte(sndDescr.SoundName.GetSize()));
    pout->Write((const UByte*)sndDescr.SoundName.ToCStr(), (int)sndDescr.SoundName.GetSize());
}

unsigned GFxDataExporter::GetStreamSoundTagLen(const GFxExportSoundDescr& sndDescr)
{
    // utilizes the tag 1007 (unused in normal SWF): the format is as follows:
    // Header           RECORDHEADER    1007
    // SoundFormat      UI16            0 - WAV
    // Bits             UI16
    // Channels         UI16
    // SampleRate       UI32
    // SampleCount      UI32
    // SeekSample       UI32
    // StartFrame       UI32
    // LastFrame        UI32
    // FileNameLen      UI8             with extension
    // FileName         UI8[FileNameLen]

    return 2 + 2 + 2 + 4 + 4 + 4 + 4 + 4 + 1 + unsigned(sndDescr.SoundName.GetSize());
}
void GFxDataExporter::WriteStreamSoundTag(File* pout, const GFxExportSoundDescr& sndDescr) 
{
    // utilizes the tag 1007 (unused in normal SWF): the format is as follows:
    // Header           RECORDHEADER    1007
    // SoundFormat      UI16            0 - WAV
    // Bits             UI16
    // Channels         UI16
    // SampleRate       UI32
    // SampleCount      UI32
    // SeekSample       UI32
    // StartFrame       UI32
    // LastFrame        UI32
    // FileNameLen      UI8             with extension
    // FileName         UI8[FileNameLen]

    // determine long or short form of tag
    //UByte len = UByte(2 + 2 + 2 + 4 + 4 + 4 + 4 + 4 + 1 + sndDescr.SoundName.GetSize());
    WriteTagHeader(pout, Tag_DefineExternalStreamSound, GetStreamSoundTagLen(sndDescr));
    pout->WriteUInt16(sndDescr.Attrs.Format); // format wav
    pout->WriteUInt16(sndDescr.Attrs.Bits);
    pout->WriteUInt16(sndDescr.Attrs.Channels);
    pout->WriteUInt32(sndDescr.Attrs.SampleRate);
    pout->WriteUInt32(sndDescr.Attrs.SampleCount);
    pout->WriteUInt32(sndDescr.Info->GetSound()->GetSeekSample());
    pout->WriteUInt32(sndDescr.StartFrame);
    pout->WriteUInt32(sndDescr.LastFrame);
    pout->WriteUByte(UByte(sndDescr.SoundName.GetSize()));
    pout->Write((const UByte*)sndDescr.SoundName.ToCStr(), (int)sndDescr.SoundName.GetSize());
}
#endif // SF_NO_SOUND

bool GFxDataExporter::WriteStrippedSwf( const char* srcFileName, const char* dstFileName, const String& name )
{
    String errorMsg;
    bool success = false;
    UInt32 totalLen;
    UInt32 prevOffset;
    int format;
    UInt32 flags;
    unsigned tlen;
    int hdrsize;
    Ptr<File> pin = *new SysFile(srcFileName, File::Open_Read);

    if (!pin || !pin->IsValid())
    {
        Format(errorMsg, "Error: Can't open source file '{0}' to read from", srcFileName);
        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
        return false;
    }

    // load header
    UInt32 header          = pin->ReadUInt32();
    UInt32 fileLength      = pin->ReadUInt32();
    UByte  version         = UByte((header >> 24) & 255);
    bool   compressed      = (header & 255) == 'C';
    if (compressed)
    {
        pin = *new ZLibFile(pin);
    }

    Ptr<File> pout = *new SysFile(dstFileName,
        File::Open_Write | File::Open_Truncate | File::Open_Create);
    if (!pout || !pout->IsWritable())
    {
        Format(errorMsg, "Error: Can't open destination file '{0}' to write to", dstFileName);
        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
        goto cleanup;
    }
    FileList.PushBack(dstFileName);

    // write new header
    static const UByte gfxHeader[] = {'G', 'F', 'X' };
    static const UByte cfxHeader[] = {'C', 'F', 'X' };
    if (!DoCompress)
        pout->Write(gfxHeader, 3);
    else
        pout->Write(cfxHeader, 3);
    pout->WriteUByte(version);
    pout->WriteUInt32(fileLength); // will need to be modified later

    // copy remaining part of header
    hdrsize = FirstTagOffset - pin->Tell();
    if (pout->CopyFromStream(pin, hdrsize) == -1)
    {
        Format(errorMsg, "Error: Failed to copy {0} bytes from {1} to {2}",
                         hdrsize, pin->GetFilePath(), pout->GetFilePath());
        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
        goto cleanup;
    }

    // write extra-tag with helper info
    // Utilizes the tag 1000 (unused in normal SWF): the format is as follows:
    // Header           RECORDHEADER    1000
    // Tool ver         UI16            Version 1.00 will be encoded as 0x100 value
    // Flags            UI32            Version 1.10 (0x10A) and above - flags
    //      Bit0 - Contains glyphs' textures info (tags 1002)
    //      Bit1 - Glyphs are stripped from DefineFont tags
    //      Bit2 - Indicates gradients' images were exported
    // BitmapsFormat    UI16            1 - TGA
    //                                  2 - DDS
    // PrefixLen        UI8
    // Prefix           UI8[PrefixLen]
    // SwfNameLen       UI8
    // SwfName          UI8[SwfNameLen]
    tlen = 2 + 4 + 2 + 1 + 1 + (unsigned)Prefix.GetSize() + (unsigned)name.GetSize() 
             + 2 + 4 * (unsigned)ActionTags.GetSize();
    WriteTagHeader(pout, 1000, tlen);

    pout->WriteUInt16(GFX_EXPORT_VERSION);

    flags = 0;
    if (ExportFonts)     flags |= ExporterInfo::EXF_GlyphTexturesExported;
    if (GlyphsStripped)  flags |= ExporterInfo::EXF_GlyphsStripped;
    pout->WriteUInt32(flags);

    format = GetImageFormatId();
    pout->WriteUInt16(UInt16(format));
    pout->WriteUByte(UByte(Prefix.GetSize()));
    pout->Write((const UByte*)Prefix.ToCStr(), (int)Prefix.GetSize());
    pout->WriteUByte(UByte(name.GetSize()));
    pout->Write((const UByte*)name.ToCStr(), (int)name.GetSize());

    // Necessary for AMP to match GFX file with SWD
    pout->WriteUInt16(UInt16(ActionTags.GetSize()));
    for (UPInt i = 0; i < ActionTags.GetSize(); ++i)
    {
        pout->WriteUInt32(ActionTags[i].TagDataOffset);
    }
    /////////////////

    if (PackImages)
    {
        ExportImagesHashType::Iterator iter = ImageDescriptors.Begin();
        for (; iter != ImageDescriptors.End(); ++iter)
        {
            const GFxExportImageDescr& imgDescr = iter->Second;
            if (imgDescr.IsPackedImage)
                WriteExternalImageTag(pout, imgDescr.CharacterId, imgDescr);
        }
    }

    prevOffset = pin->Tell();
    for (UPInt i = 0, n = TagsToBeRemoved.GetSize(); i < n; ++i)
    {
        const TagInfo& tag = TagsToBeRemoved[i];

        int size = tag.TagOffset - prevOffset;
        if (pout->CopyFromStream(pin, size) == -1)
        {
            Format(errorMsg, "Error: Failed to copy {0} bytes from {1} to {2}",
                 size, pin->GetFilePath(), pout->GetFilePath());
            Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
            goto cleanup;
        }
        prevOffset = tag.TagOffset;

        // skip tag header
        pin->Seek(tag.TagDataOffset);

        UInt32 characterId = ~0u;
        // read character id, if exists
        if (tag.TagType == 6 ||     // DefineBits
            tag.TagType == 21 ||    // DefineBitsJPEG2
            tag.TagType == 35 ||    // DefineBitsJPEG3
            tag.TagType == 20 ||    // DefineBitsLossless
            tag.TagType == 36)      // DefineBitsLossless2
        {
            characterId = pin->ReadUInt16();
        }
        else if (tag.TagType == Tag_DefineFont ||
            tag.TagType == Tag_DefineFont2 ||
            tag.TagType == Tag_DefineFont3)
        {
            UInt16 fontId = pin->ReadUInt16();
            if (GlyphsStripped)
            {
                if (tag.TagType == Tag_DefineFont)    // DefineFont
                {
                    UInt16 offset0 = pin->ReadUInt16();
                    unsigned len = 2 + 2 + ((offset0 > 0) ? 2 : 0);
                    WriteTagHeader(pout, tag.TagType, len);

                    pout->WriteUInt16(fontId);
                    pout->WriteUInt16(offset0); // first offset, indicates num of glyphs
                    if (offset0 > 0) pout->WriteUInt16(0); // end indicator
                }
                else
                {
                    UByte  uflags   = pin->ReadUByte();
                    UByte  langCode = pin->ReadUByte(); // Language code
                    UByte  nameLen  = pin->ReadUByte();
                    UByte  uname[256];
                    if (pin->Read(uname, nameLen) != nameLen)
                    {
                        Format(errorMsg, "Error: Can't read from '{0}'", pin->GetFilePath());
                        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                        pin->Seek(tag.TagOffset); // revert back
                        continue;
                    }
                    UInt16 numGlyphs = pin->ReadUInt16();
                    if (numGlyphs == 0) // do not change font tags with no glyphs
                    {
                        pin->Seek(tag.TagOffset); // revert back
                        continue;
                    }

                    int tableBase    = pin->Tell();

                    // write null as the first entry of offset table - means there are no
                    // offsets in it. Skip offset table in source SWF
                    pin->Skip(((uflags & 0x8) ? 4 : 2) * numGlyphs); // take into account wide or not wide offsets

                    UInt32 codeTableOffset;
                    if (uflags & 0x8) // wide offsets
                    {
                        codeTableOffset = pin->ReadUInt32();
                    }
                    else
                    {
                        codeTableOffset = pin->ReadUInt16();
                    }
                    // skip shapes
                    pin->Seek(tableBase + codeTableOffset);

                    // now we need to calculate the size of the remaining part of DefineFont2/3 record:
                    // it will be written into the new tag without any modification
                    int remainingLen = (tag.TagDataOffset + tag.TagLength) - pin->Tell();
                    SF_ASSERT(remainingLen >= 0);

                    // now we can start writing a new tag. Calculate its length first.
                    unsigned len = 2 + 1 + 1 + 1 + nameLen + 2 /* numglyphs */ + 2 /* off */ + 2 /* code off */ + remainingLen;

                    WriteTagHeader(pout, tag.TagType, len);

                    // reset "wide offset" flag
                    uflags &= (~0x8);

                    pout->WriteUInt16(fontId);
                    pout->WriteUByte(uflags);
                    pout->WriteUByte(langCode);
                    pout->WriteUByte(nameLen);
                    if (pout->Write(uname, nameLen) != nameLen)
                    {
                        Format(errorMsg, "Error: Can't write to '{0}'", pout->GetFilePath());
                        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                        goto cleanup;
                    }
                    pout->WriteUInt16(numGlyphs);
                    pout->WriteUInt16(0); // offset table
                    pout->WriteUInt16(4); // code table offset is always 4 with stripped shapes
                    if (pout->CopyFromStream(pin, remainingLen) == -1)
                    {
                        Format(errorMsg, "Error: Failed to copy {0} bytes from {1} to {2}",
                             remainingLen, pin->GetFilePath(), pout->GetFilePath());
                        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                        goto cleanup;
                    }
                }
            }
            else
            {
                // no stripping
                if (tag.TagType == Tag_DefineFont || !CompactFonts )
                {
                    // just copy the tag unchanged
                    pin->Seek(tag.TagOffset);
                    if (pout->CopyFromStream(pin, tag.TagLength + (tag.TagDataOffset - tag.TagOffset)) == -1)
                    {
                        Format(errorMsg, "Error: Failed to copy {0} bytes from {1} to {2}",
                             size, pin->GetFilePath(), pout->GetFilePath());
                        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                        goto cleanup;
                    }
                }
                else 
                {
                    FontDataCompactedSwf*   pfontData = (FontDataCompactedSwf*) ((SF::GFx::MovieDefImpl*)MovieDefPtr.GetPtr())->GetDataDef()->pData->GetFontData(ResourceId(fontId));
                    if (pfontData)
                    {
                        const FontDataCompactedSwf::ContainerType& container = pfontData->GetContainer(); 
                        Array<UInt8> data;
                        data.Resize(container.GetSize());
                        for(unsigned ai = 0; ai < data.GetSize(); ++ai)
                            data[ai] = container.ValueAt(ai);
                        WriteTagHeader(pout, Tag_DefineCompactedFont, (unsigned)data.GetSize()+2);
                        pout->WriteUInt16(fontId);
                        pout->Write(&data[0], (int)data.GetSize());                           
                    }
                }
            }
            // check, can we write glyph texture info or not yet
            if (ExportFonts)
            {
                FontTextureCounterHashType* ptexCounter = FontTextureUse.Get(fontId);
                if (ptexCounter)
                {
                    FontTextureCounterHashType::Iterator it = ptexCounter->Begin();
                    for (; it != ptexCounter->End(); ++it)
                    {
                        SF_ASSERT(it->Second->Counter > 0);
                        --it->Second->Counter;
                        if (it->Second->Counter == 0)
                        {
                            const GFxFontTextureDescr* ptextureDescr = FontTextures.Get(it->First);
                            if (ptextureDescr)
                            {
                                WriteGlyphTextureInfo(pout, it->First, *ptextureDescr);
                            }
                        }
                    }
                }
            }
        }
#ifdef GFX_ENABLE_SOUND

        else if (ExportSounds && tag.TagType == Tag_DefineSound)
        {
            UInt16 soundId = pin->ReadUInt16();
            const GFxExportSoundDescr* pdescr = SoundDescriptors.Get(soundId);
            SF_ASSERT(pdescr);
            WriteExternalSoundTag(pout, soundId, *pdescr);
        }
        else if (ExportSounds && (tag.TagType == Tag_SoundStreamHead || tag.TagType == Tag_SoundStreamHead2))
        {
            const GFxExportSoundDescr* pdescr = SoundDescriptors.Get(CharacterDef::CharId_EmptyMovieClip);
            //SF_ASSERT(pdescr);
            if (pdescr && pdescr->Info)
                WriteStreamSoundTag(pout, *pdescr);
        }
        else if (ExportSounds && tag.TagType == Tag_Sprite)
        {
            StreamSoundData* ssd = 0;
            for(UPInt ui = 0; ui < StreamSoundsData.GetSize(); ++ui)
            {
                if (StreamSoundsData[ui].IsHeadTagSet && StreamSoundsData[ui].DefSpriteTag.TagOffset == tag.TagOffset 
                    && StreamSoundsData[ui].DefSpriteTag.TagLength == tag.TagLength && StreamSoundsData[ui].DefSpriteTag.TagDataOffset == tag.TagDataOffset)
                {
                    ssd = &StreamSoundsData[ui];
                    break;
                }
            }
            if (ssd)
            {
                unsigned spriteId = pin->ReadUInt16();
                const GFxExportSoundDescr* pdescr = SoundDescriptors.Get(spriteId);
                if (!pdescr || !pdescr->Info)
                {
                    // just remove and empty sound data
                    SF_ASSERT(ssd->BlockTags.GetSize() == 0);
                    unsigned len = tag.TagLength - (ssd->HeadTag.TagLength + (ssd->HeadTag.TagDataOffset - ssd->HeadTag.TagOffset));
                    WriteTagHeader(pout, Tag_Sprite, len);
                    pout->WriteUInt16(UInt16(spriteId));
                    // copy everything before StreamSoundTag
                    // -2 is because spriteid is already written
                    if (pout->CopyFromStream(pin, ssd->HeadTag.TagOffset - tag.TagDataOffset - 2) == -1)
                    {
                        Format(errorMsg, "Error: Failed to copy {0} bytes from {1} to {2}",
                            size, pin->GetFilePath(), pout->GetFilePath());
                        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                        goto cleanup;
                    }
                    // copy everything after StreamSoundTag
                    int endtag = ssd->HeadTag.TagDataOffset + ssd->HeadTag.TagLength;
                    pin->Seek(endtag);
                    if (pout->CopyFromStream(pin, tag.TagLength + tag.TagDataOffset - endtag) == -1)
                    {
                        Format(errorMsg, "Error: Failed to copy {0} bytes from {1} to {2}",
                            size, pin->GetFilePath(), pout->GetFilePath());
                        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                        goto cleanup;
                    }
                }
                else
                {
                    unsigned minusLen = 0;
                    for (UPInt ui = 0; i < ssd->BlockTags.GetSize(); ++ ui)
                        minusLen += ssd->BlockTags[ui].TagLength + ssd->BlockTags[ui].TagDataOffset - ssd->BlockTags[ui].TagOffset;
                    minusLen += ssd->HeadTag.TagLength + ssd->HeadTag.TagDataOffset - ssd->HeadTag.TagOffset;
                    unsigned plusLen = GetStreamSoundTagLen(*pdescr);
                    plusLen += plusLen < 63 ? 2 : 6; // added header size
                    unsigned taglen = tag.TagLength + plusLen - minusLen;
                    WriteTagHeader(pout, Tag_Sprite, taglen);
                    pout->WriteUInt16(UInt16(spriteId));
                    // copy everything before StreamSoundTag
                    // -2 is because spriteid is already written
                    if (pout->CopyFromStream(pin, ssd->HeadTag.TagOffset - tag.TagDataOffset - 2) == -1)
                    {
                        Format(errorMsg, "Error: Failed to copy {0} bytes from {1} to {2}",
                            size, pin->GetFilePath(), pout->GetFilePath());
                        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                        goto cleanup;
                    }
                    WriteStreamSoundTag(pout, *pdescr);
                    const TagInfo* ltag = &ssd->HeadTag;
                    int endtag = ltag->TagDataOffset + ltag->TagLength;
                    
                    UPInt ui = 0;
                    while(ui< ssd->BlockTags.GetSize())
                    {
                        pin->Seek(endtag);
                        if (pout->CopyFromStream(pin, ssd->BlockTags[ui].TagOffset - endtag) == -1)
                        {
                            Format(errorMsg, "Error: Failed to copy {0} bytes from {1} to {2}",
                                size, pin->GetFilePath(), pout->GetFilePath());
                            Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                            goto cleanup;
                        }
                        ltag = &ssd->BlockTags[ui++];
                        endtag = ltag->TagDataOffset + ltag->TagLength;
                    }
                    pin->Seek(endtag);
                    if (pout->CopyFromStream(pin, tag.TagLength + tag.TagDataOffset - endtag) == -1)
                    {
                        Format(errorMsg, "Error: Failed to copy {0} bytes from {1} to {2}",
                            size, pin->GetFilePath(), pout->GetFilePath());
                        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                        goto cleanup;
                    }

                }
            }
            else
            {
                // just copy the tag unchanged
                pin->Seek(tag.TagOffset);
                if (pout->CopyFromStream(pin, tag.TagLength + (tag.TagDataOffset - tag.TagOffset)) == -1)
                {
                    Format(errorMsg, "Error: Failed to copy {0} bytes from {1} to {2}",
                        size, pin->GetFilePath(), pout->GetFilePath());
                    Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                    goto cleanup;
                }
            }
        }
#endif // SF_NO_SOUND

        else if ( PluginHandler.WantsTag(tag.TagType) )
        {
            PluginHandler.HandleTag(tag, pin, pout);
        }


        // skip tag content
        pin->Seek(tag.TagDataOffset + tag.TagLength);
        prevOffset = tag.TagDataOffset + tag.TagLength;

        if (characterId != ~0u)
        {
            // write new tag
            const GFxExportImageDescr* pdescr = ImageDescriptors.Get(characterId);
            if (!pdescr)
            {
                Format(errorMsg, "Error: Can't find image descriptor for '{0}'", characterId);
                Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                goto cleanup;
            }
            WriteExternalImageTag(pout, characterId, *pdescr);
        }
    }
    // we don't need to finish copying since the last tag in TagsToBeRemoved
    // should be an end-tag.

    pin->Close();

    // write end tag
    pout->WriteUInt16(UInt16((0 << 6) | (0 & 0x3F)));

    // correct FileLength field in SWF header
    totalLen = pout->Tell();
    pout->Seek(4);
    pout->WriteUInt32(totalLen);

    // force out file to be closed
    pout->Close();

    if (DoCompress)
    {
        String tmpFileName = dstFileName;
        tmpFileName += "###";
        // read file into a buffer
        int lenToCompress = int(totalLen - 8);
        // compress just written file starting from offset 8
        Ptr<File> piln = *new SysFile(dstFileName, File::Open_Read);
        if (!piln || !piln->IsValid())
        {
            Format(errorMsg, "Error: Can't open source file '{0}' to read from", dstFileName);
            Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
            goto cleanup;
        }


        Ptr<File> fpout = *new SysFile(tmpFileName,
            File::Open_Write | File::Open_Truncate | File::Open_Create);
        if (!fpout || !fpout->IsWritable())
        {
            Format(errorMsg, "Error: Can't open destination temporary file '{0}' to write to", tmpFileName.ToCStr());
            Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
            goto compressed_cleanup;
        }

        // Copy the header bits
        if (fpout->CopyFromStream(piln, 8) == -1)
        {
            Format(errorMsg, "Error: Failed to copy {0} bytes from {1} to {2}\n",
                 8, piln->GetFilePath(), fpout->GetFilePath());
            Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
            goto compressed_cleanup;
        }



        z_stream zstream;
        memset(&zstream, 0, sizeof(zstream));
        deflateInit(&zstream, 9);

        while (lenToCompress > 0)
        {
            UByte buffer[4096];
            UByte outBuff[8192];

            int toRead = Alg::Min(lenToCompress, int(sizeof(buffer)));
            if (pin->Read(buffer, toRead) != toRead)
            {
                Format(errorMsg, "Error: Can't read from file '{0}'", dstFileName);
                Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                break;
            }
            lenToCompress -= toRead;

            // compress the buffer
            zstream.next_in = buffer;
            zstream.avail_in = sizeof(buffer);
            int ret;
            do {
                zstream.next_out = outBuff;
                zstream.avail_out = sizeof(outBuff);
                ret = deflate(&zstream, ((lenToCompress == 0) ? Z_FINISH : Z_NO_FLUSH));

                int toWrite = int(sizeof(outBuff) - zstream.avail_out);
                if (pout->Write(outBuff, toWrite) != toWrite)
                {
                    Format(errorMsg, "Error: Can't write in file '{0}'", tmpFileName.ToCStr());
                    Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                    lenToCompress = 0;
                    goto compressed_cleanup;
                }
            } while(zstream.avail_out == 0 && ret != Z_STREAM_END);
        }
        deflateEnd(&zstream);
        success = true;
compressed_cleanup:
        if (pout)
            pout->Close();
        if(pin)
            pin->Close();
        
        // rename temp file into dstFileName
        remove(dstFileName);
        if (!success)
        {
            remove (tmpFileName);
            return false;
        }
        else
            rename(tmpFileName, dstFileName);
    }
    success = true;
cleanup:
    if (!success)
    {
        if (pout)
            pout->Close();
        if(pin)
            pin->Close();
        remove (dstFileName);

    }
    return success;
}

bool GFxDataExporter::CreateDestDirectory(const char* path)
{
    String errorMsg;

    // get cur dir
    char buf[1024];
    if (G_getcwd(buf, sizeof(buf)) == 0)
        return false;

    // check path existence
    if (SFchdir(path) == 0)
    {
        // path exists
        if (SFchdir(buf) != 0)
        {
            Format(errorMsg, "Error: Can't change directory to '{0}'", buf);
            Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
            return false;
        }
        return true;
    }

    // create path
    SPInt len = strlen(path);
    char  curDir[1024];
    SPInt curDirStart = 0;
    SPInt pathStart = 0;

    #if defined(SF_OS_WIN32)
    // Check if absolute path is used
    if (path[1] == ':' && (path[2]=='/' || path[2]=='\\'))
    {
        SFstrncpy(curDir, 1024, path, 3);
        if (SFchdir(curDir) == 0)
            curDirStart = pathStart = 3;
        else 
            return false;
    }
    #endif

    for (SPInt i = pathStart; i < len; i++)
    {
        if ((path[i]=='/' || path[i]=='\\') && (curDirStart != i) )
        {
            SFstrncpy(curDir, 1024, path + curDirStart, i - curDirStart);
            if (SFchdir(curDir) != 0) // Part of the path may already exist
            {
                if (G_mkdir(curDir) == 0)
                {
                    if (SFchdir(curDir) != 0)
                    {
                        Format(errorMsg, "Error: Can't change directory to '{0}'", curDir);
                        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                    }
                }
                else
                    return false;
            }
            curDirStart = i + 1;
        }
    }
   
    return SFchdir(buf) == 0;
}

int GFxDataExporter::CollectOriginalImageData(const char* srcFileName)
{
    String errorMsg;

    Ptr<File> pin = *new SysFile(srcFileName, File::Open_Read);
    if (!pin || !pin->IsValid())
    {
        Format(errorMsg, "Error: Can't open source file '{0}' to read from", srcFileName);
        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
        return 0;
    }

    // load header
    UInt32 header          = pin->ReadUInt32();
    pin->ReadUInt32(); //fileLength
    bool   compressed      = (header & 255) == 'C';
    if (compressed)
    {
        pin = *new ZLibFile(pin);
    }

    for (UPInt i = 0, n = TagsToBeRemoved.GetSize(); i < n; ++i)
    {
        const TagInfo& tag = TagsToBeRemoved[i];

        // skip tag header
        pin->Seek(tag.TagDataOffset);

        int characterId = -1;
        // read character id, if exists
        if (tag.TagType == 6 ||     // DefineBits
            tag.TagType == 21 ||    // DefineBitsJPEG2
            tag.TagType == 35 ||    // DefineBitsJPEG3
            tag.TagType == 20 ||    // DefineBitsLossless
            tag.TagType == 36)      // DefineBitsLossless2
        {
            characterId = pin->ReadUInt16();
        }

        if (characterId >= 0 && (tag.TagType == 6 || tag.TagType == 21 || tag.TagType == 35))
        {
            // save original JPEG data
            JpegDesc jpegDesc;
            jpegDesc.TagType = tag.TagType;
            if (tag.TagType == 35)
            {
                jpegDesc.DataSize = pin->ReadUInt32();
            }
            else
            {
                jpegDesc.DataSize = tag.TagDataOffset + tag.TagLength - pin->Tell();
            }
            jpegDesc.pData = (UByte*)SF_ALLOC(jpegDesc.DataSize, Stat_Default_Mem);
            if (pin->Read(jpegDesc.pData, int(jpegDesc.DataSize)) != int(jpegDesc.DataSize))
            {
                Format(errorMsg, "Error: Failed to read {0} bytes from {1}",
                    jpegDesc.DataSize, pin->GetFilePath());
                Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                SF_FREE(jpegDesc.pData);
                return 0;
            }
            JpegDescriptors.Set(characterId, jpegDesc);
        }
        else if (tag.TagType == 8)
        {
            // special case for JPEGTables
            JpegDesc jpegDesc;
            jpegDesc.TagType = tag.TagType;
            jpegDesc.DataSize = tag.TagDataOffset + tag.TagLength - pin->Tell();
            if (jpegDesc.DataSize > 0)
            {
                jpegDesc.pData = (UByte*)SF_ALLOC(jpegDesc.DataSize, StatGroup_Default);
                if (pin->Read(jpegDesc.pData, int(jpegDesc.DataSize)) != int(jpegDesc.DataSize))
                {
                    Format(errorMsg, "Error: Failed to read {0} bytes from {1}",
                        jpegDesc.DataSize, pin->GetFilePath());
                    Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                    SF_FREE(jpegDesc.pData);
                    return 0;
                }
            }
            else
                jpegDesc.pData = NULL;
            JpegDescriptors.Set(-1, jpegDesc);
        }
        pin->Seek(tag.TagDataOffset + tag.TagLength);
    }
    return 1;
}

void GFxDataExporter::ClearOriginalImageData()
{
    Hash<int,JpegDesc>::ConstIterator i = JpegDescriptors.Begin();

    for(; i != JpegDescriptors.End(); ++i)
    {
        const JpegDesc& desc = i->Second;
        SF_FREE(desc.pData);
    }
    JpegDescriptors.Clear();
}

int GFxDataExporter::Process()
{
    String errorMsg;
#ifdef GFX_ENABLE_SOUND
            if (ExportSounds)
                SetSoundExporter(CreateSoundExporter(Logger));
#endif
    if (CompactFonts)
    {
        SF_ASSERT(FontNormalizedSize > 0);
        ExpLoader.SetFontCompactorParams(Ptr<FontCompactorParams>(*new FontCompactorParams(FontNormalizedSize, FontMergeEdges)) );
    }
    if (!Quiet) ShowVersion(Logger);
    
    int format = GetImageFormatId();
    if (format == FileTypeConstants::File_Unknown)
    {
        Format(errorMsg, "Unknown image format: {0}. Using default TGA format instead...",
            ImagesFormat.ToCStr());
        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
        ImagesFormat = "tga";
    }
    if (format == FileTypeConstants::File_PVR || format == FileTypeConstants::File_ETC)
    {
        pExportCompressor = *SF_NEW GPVRHelper;
    }
    else if (format == FileTypeConstants::File_DDS || format == FileTypeConstants::File_TGA || format == FileTypeConstants::File_GNF)
    {
        pExportCompressor = *SF_NEW GDXTHelper;
    }
    else if (format == FileTypeConstants::File_SIF)
    {
        pExportCompressor = *SF_NEW GSIFHelper;
    }
    else if (format == FileTypeConstants::File_GTX)
    {
        pDXTCompressor = *SF_NEW GDXTHelper;
        pDXTCompressor->CalculateResizeFilter(RescaleFilter);
    }
    else if (format == FileTypeConstants::File_DDSBC)
    {
        pDXTCompressor = *SF_NEW GDXTHelper;
        pDXTCompressor->CalculateResizeFilter(RescaleFilter);
        UncompressedTexture = true; //for pDXTCompressor
        TargetPlatform = ImageTarget_DX11;
    }
    else if (format == FileTypeConstants::File_GXT)
    {
        pPVRCompressor = *SF_NEW GPVRHelper;
        pPVRCompressor->CalculateResizeFilter(RescaleFilter);
    }
    else if (format == FileTypeConstants::File_MultiFormat)
    {
        pDXTCompressor = *SF_NEW GDXTHelper;
        pPVRCompressor = *SF_NEW GPVRHelper;
        pSIFCompressor = *SF_NEW GSIFHelper;
        pDXTCompressor->CalculateResizeFilter(RescaleFilter);
        pPVRCompressor->CalculateResizeFilter(RescaleFilter);
        pSIFCompressor->CalculateResizeFilter(RescaleFilter);
        TargetPlatform = ImageTarget_Adreno; //for SIF file
    }
    if (pExportCompressor)
        pExportCompressor->CalculateResizeFilter(RescaleFilter);

    String destPath;
    if (OutputRootDir.GetSize() != 0)
    {
        destPath = OutputRootDir;
        if (OutputRootDir[OutputRootDir.GetSize() - 1] != '\\' &&
            OutputRootDir[OutputRootDir.GetSize() - 1] != '/')
            destPath += "/";
    }
    
    if (OutputGfxDir.GetSize() != 0)
    {
        if (OutputGfxDir[OutputGfxDir.GetSize() - 1] != '\\' &&
            OutputGfxDir[OutputGfxDir.GetSize() - 1] != '/')
            OutputGfxDir += "/";

        if (!CreateDestDirectory(OutputGfxDir.ToCStr()))
        {
            Format(errorMsg, "Failed to create or open destination path '{0}'", OutputGfxDir.ToCStr());
            Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
            return 0;
        }
    }
    
    if ( !PluginHandler.Initialize() )
    {
        return 0;
    }

    for (UPInt i = 0, n = SrcFileNames.GetSize(); i < n; ++i)
    {
        // Filename with extension
        String fname = GFxDataExporter::CutPath(SrcFileNames[i]);
        // Filename without extension
        String name  = GFxDataExporter::CutExtension(fname);

        

        String path;

        // If the destination path was not set and the SWF is in a
        // subdirectory, use the SWFs subdirectory as the destination path
        if (destPath.GetSize() != 0)
        {
            path = destPath;
        }
        else if (fname.GetLength() != SrcFileNames[i].GetLength())
        {
            path = SrcFileNames[i];
            path.Remove(path.GetLength()-fname.GetLength(), fname.GetLength());
        }

        // Put the extracted files into a sub
        // directory with the same name as the SWF
        if (DoCreateSubDirs)
        {
            path += name;
            path += "/";
        }

        if (path.GetLength() && !CreateDestDirectory(path))
        {
            Format(errorMsg, "Failed to create or open destination path '{0}'", path.ToCStr());
            Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
            continue;
        }


        String fullPath = OutputGfxDir;
        if ( fullPath.GetSize() == 0 )
        {
            fullPath = path;
        }

        // save .gfx file
        char buf[1024];
        SFsprintf(buf, 1024, "%s.gfx", name.ToCStr());

        // Change the exported filename case if specified
        if (ToLowercase)
            SFstrlwr(buf, sizeof(buf));
        else if (ToUppercase)
            SFstrupr(buf, sizeof(buf));

        fullPath += buf;
        if (Arguments.GetBool("ModStamp"))
        {
            FileOpenerBase* pfileopener = ExpLoader.GetFileOpener();
            SF_ASSERT(pfileopener);
            SInt64  gfxModifyTime  = pfileopener->GetFileModifyTime(fullPath.ToCStr());

            if (gfxModifyTime != -1 && gfxModifyTime > pfileopener->GetFileModifyTime(SrcFileNames[i].ToCStr()))
            {
                Format(errorMsg, "Existing {0} is newer then swf file, skipping... ", fullPath.ToCStr());
                Logger->OnMessage(GFxExportListener::MSG_Normal, errorMsg);
                continue;
            }
        }

        if (!Load(SrcFileNames[i]))
            continue;


        if (!Quiet)
        {
            GFxExportListener::ProcData data;
            data.SwfFilename = SrcFileNames[i];
            Logger->OnProcBegin(GFxExportListener::PROC_Swf, data);
        }

        int sformat = GetImageFormatId();
        if (sformat == FileTypeConstants::File_Original)
        {
            // original format is requested. Need to traverse through original source SWF,
            // scratch out image data. JPEG data will be saved as JPG. JPEG with alpha
            // will be saved as JPEG + TGA, lossless images - TGA
            CollectOriginalImageData(SrcFileNames[i]);
        }

        PluginHandler.BeginFile(SrcFileNames[i].ToCStr());

        TotalImageCount     = 0;
        ImagesWrittenCount  = 0;
        ImagesProcessedCount= 0;
        SharedImagesCount   = 0;
        TotalMemoryCount    = 0;

#ifdef GFX_ENABLE_SOUND
        TotalSoundCount     = 0;
        SoundsWrittenCount  = 0;
        SoundsProcessedCount= 0;

        StreamSoundsWrittenCount  = 0;
        StreamSoundsProcessedCount= 0;
#endif // SF_NO_SOUND


        GFxImageSoundCounterVisitor cntVisitor;

        unsigned visitMask = MovieDef::ResVisit_AllLocalImages;
#ifdef GFX_ENABLE_SOUND
        visitMask |= MovieDef::ResVisit_Sounds;
#endif // SF_NO_SOUND

        MovieDefPtr->VisitResources(&cntVisitor, visitMask);
        TotalImageCount = cntVisitor.ImageCount;

#ifdef GFX_ENABLE_SOUND
        TotalSoundCount = cntVisitor.SoundCount;
#endif // SF_NO_SOUND

        if (Info)
        {
            Format(errorMsg, "Images                    : {0}", TotalImageCount);
            Logger->OnMessage(GFxExportListener::MSG_Normal, errorMsg);
#ifdef GFX_ENABLE_SOUND
            if (ExportSounds)
            {
                Format(errorMsg, "Sounds                    : {0}", TotalSoundCount);
                Logger->OnMessage(GFxExportListener::MSG_Normal, errorMsg);
            }
#endif
        }

        {
            GFxExportListener::ProcData data;
            data.SwfFilename = SrcFileNames[i];
            Logger->OnProcBegin(GFxExportListener::PROG_Images, data);

            if (ExportFonts)
            {
                // collect font textures.
                Scaleform::GFx::MovieDefImpl* pmovieDefImpl = static_cast<Scaleform::GFx::MovieDefImpl*>(MovieDefPtr.GetPtr());

                GFxFontsVisitor fontVisitor(this, &pmovieDefImpl->GetResourceBinding());
                MovieDefPtr->VisitResources(&fontVisitor, MovieDef::ResVisit_Fonts);

                if (Info)
                {
                    Format(errorMsg, "Font textures             : {0}", (int)FontTextures.GetSize());
                    Logger->OnMessage(GFxExportListener::MSG_Normal, errorMsg);
                }


                TotalImageCount += (int)FontTextures.GetSize();

                HashIdentity<unsigned, Ptr<GFxFontTextureCounter> > TexId2Counter; // TexId -> Cnt
                { // collect number of fonts per each texture
                    FontTexturesHashType::Iterator iter = FontTextures.Begin();
                    for(; iter != FontTextures.End(); ++iter)
                    {
                        GFxFontTextureDescr& textureDescr = iter->Second;
                        Ptr<GFxFontTextureCounter> pcnt = *new GFxFontTextureCounter((unsigned)textureDescr.Fonts.GetSize());
                        TexId2Counter.Set(iter->First, pcnt);
                    }
                }

                FontTexturesHashType::Iterator iter = FontTextures.Begin();
                for(; iter != FontTextures.End(); ++iter)
                {
                    GFxFontTextureDescr& textureDescr = iter->Second;

                    UPInt ui, un;
                    for (ui = 0, un = textureDescr.Fonts.GetSize(); ui < un; ++ui)
                    {
                        GFxFontTextureDescr::FontDescr& fntDescr = textureDescr.Fonts[ui];
                        Ptr<FontResource> pfont = fntDescr.pFont;

                        // so, build the table of used textures, by fonts
                        FontTextureCounterHashType* ptexCounter = FontTextureUse.Get((UInt16)fntDescr.FontId);
                        if (!ptexCounter)
                        {
                            FontTextureUse.Set((UInt16)fntDescr.FontId, FontTextureCounterHashType());
                            ptexCounter = FontTextureUse.Get((UInt16)fntDescr.FontId);
                        }
                        SF_ASSERT(ptexCounter);
                        Ptr<GFxFontTextureCounter>* ppcnt = TexId2Counter.Get(iter->First);
                        SF_ASSERT(ppcnt);
                        ptexCounter->Set(iter->First, *ppcnt);

                        // visit each texture glyph to gather all texture glyphs
                        // also creates map for each font to translate font's texture glyph
                        // indices into the texture's ones.
                        GFxTextureGlyphVisitor texGlyphVisitor(textureDescr, fntDescr, &pmovieDefImpl->GetResourceBinding());
                        TextureGlyphData* ptextGlyphData = static_cast<TextureGlyphData*>(pfont->GetFont()->GetTextureGlyphData());
                        if (ptextGlyphData == NULL)
                            ptextGlyphData = (TextureGlyphData*)pfont->GetFont()->GetTextureGlyphData();
                        if (ptextGlyphData)
                            ptextGlyphData->VisitTextureGlyphs(&texGlyphVisitor);
                    }
                }
            }

            if (!Quiet && !Info) 
            {
                Format(errorMsg, "Processing {0} images - ", TotalImageCount);
                Logger->OnMessage(GFxExportListener::MSG_Normal, errorMsg);
            }

            // Extract all images to the path
            if (TotalImageCount > 0)
            {
                if (Info)
                {
                    Logger->OnMessage(GFxExportListener::MSG_Normal, "Images: ");
                }

                GFxImageExtractorVisitor visitor(*this, name, path);
                MovieDefPtr->VisitResources(&visitor, visitMask);

                if (FontTextures.GetSize() > 0)
                {
                    if (Info)
                    {
                        Logger->OnMessage(GFxExportListener::MSG_Normal, "Font Textures: ");
                    }

                    // write font images
                    FontTexturesHashType::ConstIterator it = FontTextures.Begin();
                    for(; it != FontTextures.End(); ++it)
                    {
                        UInt32 id = it->First;
                        const GFxFontTextureDescr& ftDescr = it->Second;
                        ExtractImage(ftDescr.pTexture, ResourceId(id), name, path, NULL);
                        if (Info)
                        {
                            int nfonts=(int)ftDescr.Fonts.GetSize();
                            for (int fi=0;fi<nfonts;fi++)
                            {
                                Format(errorMsg, "    Font: {0:-15s}  {1:9d} Glyph Shapes", ftDescr.Fonts[fi].pFont->GetName(), ftDescr.Fonts[fi].pFont->GetGlyphShapeCount());
                                Logger->OnMessage(GFxExportListener::MSG_Normal, errorMsg);
                            }
                            //printf("Number of fonts : %d",nfonts);
                        }

                    }
                }
            }
            else
            {
                if (!Quiet && !Info) 
                {
                    Logger->OnMessage(GFxExportListener::MSG_Normal, "100%");
                }
            }

            data.WrittenImagesCount = ImagesWrittenCount;
            data.SharedImagesCount = SharedImagesCount;
            Logger->OnProcEnd(GFxExportListener::PROG_Images, data);
        }

        if (!Quiet && !Info) 
        {
            Format(errorMsg, "Total images written: {0}", ImagesWrittenCount);
            Logger->OnMessage(GFxExportListener::MSG_Normal, errorMsg);
            if (SharedImagesCount)
            {
                Format(errorMsg, "Total images shared: {0}", SharedImagesCount);
                Logger->OnMessage(GFxExportListener::MSG_Normal, errorMsg);
            }
        }


#ifdef GFX_ENABLE_SOUND
        if (ExportSounds)
        {
            GFxExportListener::ProcData data;
            data.SwfFilename = SrcFileNames[i];
            Logger->OnProcBegin(GFxExportListener::PROG_Sounds, data);

            if (!Quiet && !Info) 
            {
                Format(errorMsg, "Processing {0} sounds - ", TotalSoundCount);
                Logger->OnMessage(GFxExportListener::MSG_Normal, errorMsg);
            }
            if (TotalSoundCount > 0)
            {
                GFxSoundExtractorVisitor visitor(*this, name, path);
                MovieDefPtr->VisitResources(&visitor, MovieDef::ResVisit_Sounds | MovieDef::ResVisit_NestedMovies);
            }
            else
            {
                if (!Quiet && !Info) 
                    Logger->OnProcProgress(GFxExportListener::PROG_Sounds, 100);
            }
            if (!Quiet && !Info) 
            {
                Format(errorMsg, "Total sounds written: {0}", SoundsWrittenCount);
                Logger->OnMessage(GFxExportListener::MSG_Normal, errorMsg);
            }
            
            data.WrittenSoundsCount = SoundsWrittenCount;
            Logger->OnProcEnd(GFxExportListener::PROG_Sounds, data);
            Logger->OnProcBegin(GFxExportListener::PROG_SpriteSounds, data);

            if (!Quiet && !Info) 
            {
                Logger->OnMessage(GFxExportListener::MSG_Normal, "Processing stream sounds - ");
            }

            {
                GFxSpriteSoundExtractorVisitor visitor(*this, name, path);
                MovieDefPtr->VisitResources(&visitor, MovieDef::ResVisit_Sprite | MovieDef::ResVisit_NestedMovies);
            }

            if (!Quiet && !Info) 
            {
                Logger->OnProcProgress(GFxExportListener::PROG_SpriteSounds, 100);
            }
            if (!Quiet && !Info) 
            {
                Format(errorMsg, "Total stream sounds written: {0}", StreamSoundsWrittenCount);
                Logger->OnMessage(GFxExportListener::MSG_Normal, errorMsg);
            }

            data.WrittenSoundsCount = StreamSoundsWrittenCount;
            Logger->OnProcEnd(GFxExportListener::PROG_SpriteSounds, data);
        }
#endif // SF_NO_SOUND

        {
            GFxExportListener::ProcData data;
            data.SwfFilename = SrcFileNames[i];
            Logger->OnProcBegin(GFxExportListener::PROC_Other, data);

            if (ExportFsCommands)
            {
                unsigned mask = ((FsCommandsAsTree) ? FSTree : 0) | ((FsCommandsAsList) ? FSList : 0);
                WriteFsCommands(MovieDefPtr, SrcFileNames[i], path, name, mask);
            }

            if (ExportDefaultEditText)
            {
                WriteDefaultEditTextFieldText(MovieDefPtr, SrcFileNames[i], path, name);
            }

            if (ExportFontList)
            {
                WriteFontList(MovieDefPtr, SrcFileNames[i], path, name);
            }

            WriteListOfFiles(path, name);

            if (ExportAS3code)
            {
                WriteActions3(MovieDefPtr, SrcFileNames[i], path);
            }

            Logger->OnProcEnd(GFxExportListener::PROC_Other, data);
        }

        // Create the stripped SWF file and save it as a GFX file
        {
            if (!Info)
            {
                if (!WriteStrippedSwf(SrcFileNames[i], fullPath, name))
                    return false;
                if (!Quiet) 
                {
                    GFxExportListener::OutputPayload data;
                    data.FilePath = fullPath;
                    Logger->OnOutput(DoCompress ? GFxExportListener::OUT_CFxFile : GFxExportListener::OUT_GFxFile, data);
                }
            }
            else 
            {
                String fp = fullPath;
                fullPath+="$$$";
                if (!WriteStrippedSwf(SrcFileNames[i], fullPath, name))
                    return false;
                Ptr<File> ptmp = *new SysFile(fullPath,File::Open_Read);
                int size = ptmp->SeekToEnd();
                
                GFxExportListener::OutputPayload data;
                data.FilePath = fp;
                data.GFxTotalImagesWritten = ImagesWrittenCount;
                data.GFxEstMemSize = TotalMemoryCount;
                data.GFxEstFileSize = size;
                Logger->OnOutput(DoCompress ? GFxExportListener::OUT_CFxFile : GFxExportListener::OUT_GFxFile, data);
                
                ptmp->Close();
                remove(fullPath);
            }
        }

        PluginHandler.EndFile();

        // clear all the stuff here, since heap will be released anyway.
        FontTextures.Clear();
        FontTextureUse.Clear();
        TagsToBeCopied.Resize(0);
        TagsToBeRemoved.Resize(0); 
        TagsWithActions.Resize(0); 
        TagsActions3.Resize(0);
        ActionTags.Resize(0);
        FileList.Resize(0);

        ImageDescriptors.Clear();
        SharedImagesHash.Clear();
        SharedImageNamesSet.Clear();
        ClearOriginalImageData();

        Hash<int, GFxImageExporter*>::ConstIterator iter = ImageExporters.Begin();
        for(; iter != ImageExporters.End(); ++iter)
        {
            iter->Second->Clear();
        }
#ifdef GFX_ENABLE_SOUND
        SoundDescriptors.Clear();
        StreamSoundsData.Resize(0);
#endif   
        MovieDefPtr = NULL;

        if (!Quiet)
        {
            GFxExportListener::ProcData data;
            data.SwfFilename = SrcFileNames[i];
            Logger->OnProcEnd(GFxExportListener::PROC_Swf, data);
        }
    }
    //if (!Quiet) printf("\n");
    PluginHandler.Shutdown();

    return 1;
}

void    GFxDataExporter::WriteFontList(MovieDef* pmovieDef, const char* swfFileName, const String& path, const String& name)
{
    String errorMsg;

    if (!Quiet) 
    {
        Format(errorMsg, "Looking for fonts in '{0}'", swfFileName);
        Logger->OnMessage(GFxExportListener::MSG_Normal, errorMsg);
    }

    Array<String> fonts;
    Array<String> text;

    GFxFontNameVisitor fontVisitor(fonts);
    GFxTextFieldVisitor tfVisitor(text);

    // visit the resources
    pmovieDef->VisitResources(&fontVisitor, MovieDef::ResVisit_Fonts);
    pmovieDef->VisitResources(&tfVisitor, MovieDef::ResVisit_EditTextFields);

    // setup output file
    char buf[1024];
    SFsprintf(buf, 1024, "%s.fnt", name.ToCStr());
    if (ToLowercase)
        SFstrlwr(buf, sizeof(buf));
    else if (ToUppercase)
        SFstrupr(buf, sizeof(buf));
    String fullPath = path;
    fullPath += buf;    
     
    // write out data in UTF8
    FILE* fout;
#if defined(SF_CC_MSVC) && (SF_CC_MSVC >= 1400)
    fout = NULL;
    fopen_s(&fout, fullPath, "w");
#else
    fout = fopen(fullPath, "w");
#endif // defined(SF_CC_MSVC) && (SF_CC_MSVC >= 1400)
    if (!fout)
    {
        Format(errorMsg, "Error: Can't open destination file '{0}' to write to", fullPath.ToCStr());
        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
        return;
    }

    // write-out UTF8 BOM
    fprintf(fout, "%c%c%c", 0xEF, 0xBB, 0xBF);
    for (unsigned i=0; i < fonts.GetSize(); i++)
    {
        String& t = fonts[i];

        if (t.GetLength() > 0)
            fprintf(fout, "%s\n", fonts[i].ToCStr());
    }
    fprintf(fout, "\n\nTextfield default text (font id)\n\n");
    for (unsigned i=0; i < text.GetSize(); i++)
    {
        String& t = text[i];

        if (t.GetLength() > 0)
            fprintf(fout, "%s\n", text[i].ToCStr());
    }

    fclose(fout);
    //if (!Quiet) printf("\n");

    if (!Quiet)
    {
        GFxExportListener::OutputPayload data;
        data.FilePath = fullPath;
        Logger->OnOutput(GFxExportListener::OUT_FontList, data);
    }
}


void    GFxDataExporter::WriteDefaultEditTextFieldText(MovieDef* pmovieDef, const char* swfFileName, const String& path, const String& name)
{
    String errorMsg;

    if (!Quiet) 
    {
        Format(errorMsg, "Looking for dynamic textfields in '{0}'", swfFileName);
        Logger->OnMessage(GFxExportListener::MSG_Normal, errorMsg);
    }

    Array<String> text;

    // resource visitor
    GFxEditTextFieldVisitor dtfVisitor(text, DefaultDynamicTextOnly);

    // visit the resources
    pmovieDef->VisitResources(&dtfVisitor, MovieDef::ResVisit_EditTextFields);

    // setup output file
    char buf[1024];
    if (DefaultDynamicTextOnly)
        SFsprintf(buf, 1024, "%s.ddt", name.ToCStr());
    else
        SFsprintf(buf, 1024, "%s.det", name.ToCStr());
    if (ToLowercase)
        SFstrlwr(buf, sizeof(buf));
    else if (ToUppercase)
        SFstrupr(buf, sizeof(buf));
    String fullPath = path;
    fullPath += buf;    

    // write out data in UTF8
    FILE* fout;
#if defined(SF_CC_MSVC) && (SF_CC_MSVC >= 1400)
    fout = NULL;
    fopen_s(&fout, fullPath, "w");
#else
    fout = fopen(fullPath, "w");
#endif // defined(SF_CC_MSVC) && (SF_CC_MSVC >= 1400)
    if (!fout)
    {
        Format(errorMsg, "Error: Can't open destination file '{0}' to write to", fullPath.ToCStr());
        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
        return;
    }

    // write-out UTF8 BOM
    fprintf(fout, "%c%c%c", 0xEF, 0xBB, 0xBF);
    for (unsigned i=0; i < text.GetSize(); i++)
    {
        const String& t = text[i];

        if (t.GetLength() > 0)
            fprintf(fout, "\"%s\"\n", t.ToCStr());
    }

    fclose(fout);

    //if (!Quiet) printf("\n");

    if (!Quiet) 
    {
        GFxExportListener::OutputPayload data;
        data.FilePath = fullPath;
        Logger->OnOutput(DefaultDynamicTextOnly ? GFxExportListener::OUT_DynamicTextfieldList : GFxExportListener::OUT_EditTextfieldList, data);
    }
}

void    GFxDataExporter::WriteListOfFiles(const String& path, const String& name)
{
    String errorMsg;

    if (GenFileList)
    {
        String fullPath;
        if (FileListName.IsEmpty())
        {
            // assemble
            char buf[1024];
            SFsprintf(buf, 1024, "%s.lst", name.ToCStr());
            fullPath = path;
            fullPath += buf;    
        }
        else
            fullPath = FileListName;

        // write out data in UTF8
        FILE* fout;
#if defined(SF_CC_MSVC) && (SF_CC_MSVC >= 1400)
        fout = NULL;
        fopen_s(&fout, fullPath, "w");
#else
        fout = fopen(fullPath, "w");
#endif // defined(SF_CC_MSVC) && (SF_CC_MSVC >= 1400)
        if (!fout)
        {
            Format(errorMsg, "Error: Can't open destination file '{0}' to write to", fullPath.ToCStr());
            Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
            return;
        }

        // write-out UTF8 BOM
        //fprintf(fout, "%c%c%c", 0xEF, 0xBB, 0xBF);
        for (UPInt i=0; i < FileList.GetSize(); i++)
        {
            const String& t = FileList[i];

            if (t.GetLength() > 0)
                fprintf(fout, "%s\n", t.ToCStr());
        }

        fclose(fout);

        if (!Quiet) 
        {
            GFxExportListener::OutputPayload data;
            data.FilePath = fullPath;
            Logger->OnOutput(GFxExportListener::OUT_FileList, data);
        }
    }
}

void GFxDataExporter::WriteActions3(MovieDef* pmovieDef, const char* swfFileName, const String& path)
{
    String errorMsg;

    SF_UNUSED(pmovieDef);

    Ptr<File> pinFile = *new SysFile(swfFileName, File::Open_Read);
    if (!pinFile || !pinFile->IsValid())
    {
        Format(errorMsg, "Error: Can't open source file '{0}' to read from", swfFileName);
        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
        return;
    }

    // load header
    UInt32 header          = pinFile->ReadUInt32();
    pinFile->ReadUInt32(); // fileLength
    bool   compressed      = (header & 255) == 'C';
    if (compressed)
    {
        pinFile = *new ZLibFile(pinFile);
    }

    if (!pinFile || !pinFile->IsValid())
    {
        Format(errorMsg, "Error: Can't read from source file '{0}'", pinFile->GetFilePath());
        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
        return;
    }

    Stream in(pinFile, Memory::GetGlobalHeap(), 0, 0); 
    Stream* pin = &in;
    UByte* pbuf = 0;
    UPInt bufSize = 0;

    for (UPInt i = 0, n = TagsActions3.GetSize(); i < n; ++i)
    {
        const TagInfo& tag = TagsActions3[i];
        pin->SetPosition(tag.TagDataOffset);
        
        pin->ReadU32(); // flags
        char* pstr = pin->ReadString(Memory::GetGlobalHeap());

        char name[256];
        if (!pstr || *pstr == 0)
            SFsprintf(name, sizeof(name), "%s_%d.abc", swfFileName, i);
        else
            SFsprintf(name, sizeof(name), "%s_%s.abc", swfFileName, pstr);
        
        SPInt remainingLen = (tag.TagDataOffset + tag.TagLength) - pin->Tell();
        SF_ASSERT(remainingLen >= 0);
        if (!pbuf)
        {
            pbuf = (UByte*)SF_ALLOC(remainingLen, Stat_Default_Mem);
            bufSize = (UPInt)remainingLen;
        }
        else if (remainingLen > (SPInt)bufSize)
        {
            pbuf = (UByte*)SF_REALLOC(pbuf, remainingLen, Stat_Default_Mem);
            bufSize = (UPInt)remainingLen;
        }
        pin->ReadToBuffer(pbuf, (unsigned)remainingLen);

        String fname = path;
        fname += name;
        FILE* fout;
#if defined(SF_CC_MSVC) && (SF_CC_MSVC >= 1400)
        fout = NULL;
        fopen_s(&fout, fname, "wb");
#else
        fout = fopen(fname, "w");
#endif // defined(SF_CC_MSVC) && (SF_CC_MSVC >= 1400)
        if (!fout)
        {
            Format(errorMsg, "Error: Can't open destination file '{0}' to write to", fname.ToCStr());
            Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
            return;
        }

        if (fwrite(pbuf, 1, bufSize, fout) != bufSize)
        {
            Format(errorMsg, "Error: Can't write to destination file '{0}' to write to", fname.ToCStr());
            Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
        }

        fclose(fout);
        SF_FREE(pstr);

        if (!Quiet) 
        {
            GFxExportListener::OutputPayload data;
            data.FilePath = fname;
            Logger->OnOutput(GFxExportListener::OUT_AbcFile, data);            
        }
    }
    SF_FREE(pbuf);
    //if (!Quiet) printf("\n");
/*
    for (unsigned curMask = 1; curMask <= mask; curMask <<= 1)
    {
        if (curMask & mask)
        {
            String fname = path;
            fname += name;
            if (curMask == FSList)
            {
                fname += ".fsl";
                if (!Quiet) 
                {
                    Logger->OnProcessFSCommands(fname.ToCStr(), false );//bool isTree
                    //printf("Saving list of fscommands to '%s'", fname.ToCStr());
                }
            }
            else
            {
                fname += ".fst";
                if (!Quiet) 
                {
                    Logger->OnProcessFSCommands(fname.ToCStr(), true );//bool isTree
                    //printf("Saving tree of fscommands to '%s'", fname.ToCStr());
                }
            }

            FILE* fout;
#if defined(SF_CC_MSVC) && (SF_CC_MSVC >= 1400)
            fout = NULL;
            fopen_s(&fout, fname, "w");
#else
            fout = fopen(fname, "w");
#endif // defined(SF_CC_MSVC) && (SF_CC_MSVC >= 1400)
            if (!fout)
            {
                Format(errorMsg, "Error: Can't open destination file '{0}' to write to", fname.ToCStr());
                Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                return;
            }

            if (curMask == FSTree)
            {
                DumpFsCommandsAsTree(fout, fscommands, stringHolderArray, 0);
            }
            if (curMask == FSList)
            {
                Array<String>  cmdList;
                Array<int>        cmdSortedIdx;
                MakeFsCommandsAsList(fscommands, stringHolderArray, cmdList, cmdSortedIdx);
                for (UPInt i = 0, n = cmdSortedIdx.GetSize(); i < n; ++i)
                {
                    fprintf(fout, "%s\n", cmdList[cmdSortedIdx[i]].ToCStr());
                }
            }

            fclose(fout);
            //if (!Quiet) printf("\n");
        }
    }*/
}


static int ParseRescaleFilterType(const char* pfilterName)
{
    if (SFstricmp(pfilterName, "box") == 0)
        return ExportImageHelper::FilterBox;
    else if (SFstricmp(pfilterName, "triangle") == 0)
        return ExportImageHelper::FilterTriangle;
    else if (SFstricmp(pfilterName, "quadratic") == 0)
        return ExportImageHelper::FilterQuadratic;
    else if (SFstricmp(pfilterName, "cubic") == 0)
        return ExportImageHelper::FilterBicubic;
    else if (SFstricmp(pfilterName, "catrom") == 0)
        return ExportImageHelper::FilterCatrom;
    else if (SFstricmp(pfilterName, "mitchell") == 0)
        return ExportImageHelper::FilterMitchell;
    else if (SFstricmp(pfilterName, "gaussian") == 0)
        return ExportImageHelper::FilterGaussian;
    else if (SFstricmp(pfilterName, "sinc") == 0)
        return ExportImageHelper::FilterSinc;
    else if (SFstricmp(pfilterName, "bessel") == 0)
        return ExportImageHelper::FilterBessel;
    else if (SFstricmp(pfilterName, "hanning") == 0)
        return ExportImageHelper::FilterHanning;
    else if (SFstricmp(pfilterName, "hamming") == 0)
        return ExportImageHelper::FilterHamming;
    else if (SFstricmp(pfilterName, "blackman") == 0)
        return ExportImageHelper::FilterBlackman;
    else if (SFstricmp(pfilterName, "kaiser") == 0)
        return ExportImageHelper::FilterKaiser;
    return -1;
}

static int ParseMipFilterType(const char* pfilterName)
{
    if (SFstricmp(pfilterName, "box") == 0)
        return ExportImageHelper::nvFilterBox;
    else if (SFstricmp(pfilterName, "triangle") == 0)
        return ExportImageHelper::nvFilterTriangle;
    else if (SFstricmp(pfilterName, "kaiser") == 0)
        return ExportImageHelper::nvFilterKaiser;
    return -1;
}

static unsigned GetIntLog2(UInt32 value)
{ 
    // Binary search - decision tree (5 tests, rarely 6)
    return
        ((value <= 1U<<15) ?
        ((value <= 1U<<7) ?
        ((value <= 1U<<3) ?
        ((value <= 1U<<1) ? ((value <= 1U<<0) ? 0 : 1) : ((value <= 1U<<2) ? 2 : 3)) :
        ((value <= 1U<<5) ? ((value <= 1U<<4) ? 4 : 5) : ((value <= 1U<<6) ? 6 : 7))) :
        ((value <= 1U<<11) ?
        ((value <= 1U<<9) ? ((value <= 1U<<8) ? 8 : 9) : ((value <= 1U<<10) ? 10 : 11)) :
        ((value <= 1U<<13) ? ((value <= 1U<<12) ? 12 : 13) : ((value <= 1U<<14) ? 14 : 15)))) :
        ((value <= 1U<<23) ?
        ((value <= 1U<<19) ?
        ((value <= 1U<<17) ? ((value <= 1U<<16) ? 16 : 17) : ((value <= 1U<<18) ? 18 : 19)) :
        ((value <= 1U<<21) ? ((value <= 1U<<20) ? 20 : 21) : ((value <= 1U<<22) ? 22 : 23))) :
        ((value <= 1U<<27) ?
        ((value <= 1U<<25) ? ((value <= 1U<<24) ? 24 : 25) : ((value <= 1U<<26) ? 26 : 27)) :
        ((value <= 1U<<29) ? ((value <= 1U<<28) ? 28 : 29) : ((value <= 1U<<30) ? 30 : 
        ((value <= 1U<<31) ? 31 : 32))))));
}

static UInt32 GetNextPow2(UInt32 value)
{ 
    return (1U<<GetIntLog2(value));
}

void GFxDataExporter::ParseFilename( const char* pfilename )
{
#ifdef SF_OS_WIN32
	const char* pch = pfilename;
	bool    hasWildcards = 0;
	SPInt   lastDirPos = -1;
	while (*pch && !hasWildcards)
	{
		switch (*pch)
		{
		case '{':
		case '[':
		case '?':
		case '*':
			hasWildcards=1;
			break;
		case '/':
		case '\\':
			lastDirPos=pch-pfilename;
			break;
		}
		pch++;
	}

	if (!hasWildcards)
	{
		// add the single filename
		SrcFileNames.PushBack(pfilename);
	}
	else
	{
		// search the path
		WIN32_FIND_DATAA ffd;
		HANDLE fh = ::FindFirstFileA(pfilename, &ffd);
		if (fh!=INVALID_HANDLE_VALUE)
		{
			do
			{
				// skip subdirectories
				if (!(ffd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
				{
					// add the file to the list
					String fname(pfilename,lastDirPos+1);
					fname += ffd.cFileName;
					SrcFileNames.PushBack(fname);
				}
			} while (::FindNextFileA(fh,&ffd));
			::FindClose(fh);
		}
	}
#else
	SrcFileNames.PushBack(pfilename);
#endif
}

int GFxDataExporter::ParseCommandLine(int argc, char* argv[])
{
    if (Arguments.ParseCommandLine(argc, argv))
    {
        ShowVersion(Logger);
        Arguments.PrintOptionsHelp();
        return 0;
    }

    InitializeArguments();
    return 1;
}

void    GFxDataExporter::InitializeArguments()
{    
    String errorMsg;

    bool fontImagesFmtSpecified = false;
    //bool gradImagesFmtSpecified = false;

    if (Arguments.GetString("NoDebugPopups"))
    {
#ifdef SF_OS_WIN32
    SuppressDiagPopupMessages();
#endif
    }
    

    int nfilenames = (int) Arguments.GetListSize("FileNames");
    for (int i = 0; i < nfilenames; i++)
    {
        const char* pfilename = Arguments.GetString("FileNames", i);
		ParseFilename(pfilename);
    }
    OutputGfxDir        = Arguments.GetString("OutputGfxDir");
    OutputRootDir       = Arguments.GetString("OutputRootDir");
    Prefix              = Arguments.GetString("Prefix");
    Info                = Arguments.GetBool("Info");
    Quiet               = Arguments.GetBool("Quiet");
    QuietProgress       = Arguments.GetBool("QuietProgress");
    DoCompress          = Arguments.GetBool("DoCompress");
    DoCreateSubDirs     = Arguments.GetBool("DoCreateSubDirs");
    UncompressedTexture     = Arguments.GetBool("UncompressedDDS");
    DXT1Allowed         = Arguments.GetBool("DXT1Allowed");
    String platform = Arguments.GetString("Platform").ToLower();
    
    bool pu = Arguments.GetBool("PlatformUncompressed");

    if (platform == "3ds")
    {
        TargetPlatform = ImageTarget_3DS;
        ImagesFormat = "SIF";
    }
    else if (platform == "ps3")
    {
        TargetPlatform = ImageTarget_PS3;
        if (pu)
        {
            ImagesFormat = "SIF";
        }
        else
        {
            ImagesFormat = "DDS";
            DXTn = 5;
        }
    }
    else if (platform == "psvita")
    {
        TargetPlatform = ImageTarget_PSVita;
        ImagesFormat = "GXT";
    }
    else if (platform == "wiiu")
    {
        TargetPlatform = ImageTarget_WiiU;
        ImagesFormat = "GTX";
        if (pu)
        {
            UncompressedTexture = true;
        }
        else
        {
            DXTn = 5;
        }
    }
    else if (platform == "xbox360")
    {
        TargetPlatform = ImageTarget_X360; 
        ImagesFormat = "SIF";
        UncompressedTexture = false;
        if (pu) 
        {
            UncompressedTexture = true;
        }
        else
        {
            DXTn = 5;
        }
    }
    else if (platform == "iphone")
    {
        if (pu) 
        {
            ImagesFormat = "tga";
        }
        else
        {
            ImagesFormat = "PVR";
        }   
    }
    else if (platform == "wii")
    {
        TargetPlatform = ImageTarget_Wii;
        if (pu)
        {
            ImagesFormat = "SIF";
        }
        else
        {
            ImagesFormat = "DDS";
            DXTn = 1;
            DXT1Allowed = true;
        }

    }
    else if (platform == "android")
    {
        TargetPlatform = ImageTarget_Adreno; //for SIF file
        ImagesFormat = "MULT";
        DXT1Allowed = true;
    }
    else if (platform == "ps4")
    {
        TargetPlatform = ImageTarget_Orbis;
        ImagesFormat = "GNF";
        UncompressedTexture = pu;
    }
    else 
        ImagesFormat    = Arguments.GetString("ImagesFormat").ToLower();

    ReplaceImages       = Arguments.GetBool("ReplaceImages");
    ImgSubstDir         = Arguments.GetString("ImgSubstDir");

    if (!ImgSubstDir.IsEmpty() && ImgSubstDir[ImgSubstDir.GetSize() - 1] != '\\' &&
        ImgSubstDir[ImgSubstDir.GetSize() - 1] != '/')
        ImgSubstDir += "/";



    if (Arguments.GetBool("DXT1"))
    {
        DXT1Allowed = true;
        Logger->OnMessage(GFxExportListener::MSG_Warning, "-d1 option is deprecated, please use -d1c instead");
    }
    
    if (!DXTn)
    {
        if (Arguments.GetBool("DXT1a"))
        {
            DXTn = 1;
            DXT1Allowed = true;
        }
        else if (Arguments.GetBool("DXT5"))
            DXTn = 5;
        else if (Arguments.GetBool("DXT3"))
            DXTn = 3;
    }


   if (Arguments.GetBool("pv2"))
   {
        PVRn = 2;
   }
   else if (Arguments.GetBool("pv4"))
   {
       PVRn = 4;
   }

    GenMipMapLevels     = Arguments.GetBool("MipMap");

    ToUppercase         = Arguments.GetBool("ToUppercase");
    ToLowercase         = Arguments.GetBool("ToLowercase");
    NoExportNames       = Arguments.GetBool("NoExportNames");
    ShareImages         = Arguments.GetBool("ShareImages");
    JustStripImages     = Arguments.GetBool("JustStripImages");
    SoundExt        = Arguments.GetString("SoundExt").ToLower();
    if (!SoundExt.IsEmpty())
        ExportSounds = true;
    GenFileList         = Arguments.GetBool("GenFileList");

    TextureGlyphNominalSize = Arguments.GetInt("TextureGlyphNominalSize");
    TextureGlyphPadPixels   = Arguments.GetInt("TextureGlyphPadPixels");


    {  // Get Font Texture size 
        const char* pfontTextureSize = Arguments.GetString("FontTextureSize").ToCStr();
        char* endp = 0;
        unsigned long n = strtoul(pfontTextureSize, &endp, 10);
        if (n < GFX_FONT_CACHE_TEXTURE_SIZE_MINIMUM) n = GFX_FONT_CACHE_TEXTURE_SIZE_MINIMUM;
        FontTextureWidth = int(GetNextPow2(UInt32(n)));
        if (FontTextureWidth != int(n))
        {
            Format(errorMsg, "Warning: Texture width should be a power of 2. Setting to {0}", FontTextureWidth);
            Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
        }
        if (*endp == 0)
            FontTextureHeight = FontTextureWidth;
        else if (*endp == 'x' || *endp == 'X' || *endp == '*' || *endp == '-' || *endp == ',')
        {
            ++endp;
            n = strtoul(endp, &endp, 10);
            if (n < GFX_FONT_CACHE_TEXTURE_SIZE_MINIMUM) n = GFX_FONT_CACHE_TEXTURE_SIZE_MINIMUM;
            if (*endp == 0)
            {
                FontTextureHeight = int(GetNextPow2(UInt32(n)));
                if (FontTextureHeight != int(n))
                {
                    Format(errorMsg, "Warning: Texture height should be a power of 2. Setting to {0}", FontTextureHeight);
                    Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                }
            }
            else
                FontTextureHeight = FontTextureWidth;
        }
        if (FontTextureWidth > GFX_MAX_TEXTURE_DIMENSION)
        {
            FontTextureWidth = GFX_MAX_TEXTURE_DIMENSION;
            Format(errorMsg, "Warning: Font texture width is too big. Resetting to {0}", FontTextureWidth);
            Logger->OnMessage(GFxExportListener::MSG_Warning, errorMsg);
        }
        if (FontTextureHeight > GFX_MAX_TEXTURE_DIMENSION)
        {
            FontTextureHeight = GFX_MAX_TEXTURE_DIMENSION;
            Format(errorMsg, "Warning: Font texture height is too big. Resetting to {0}", FontTextureHeight);
            Logger->OnMessage(GFxExportListener::MSG_Warning, errorMsg);
        }
    }

    if (Arguments.HasValue("FontImagesFormat"))
    {
        String fontImagesFormatStr = Arguments.GetString("FontImagesFormat").ToLower();
        if (fontImagesFormatStr == "tga" || fontImagesFormatStr == "tga8")
        {
            FontImagesFormat = FileTypeConstants::File_TGA;
            FontImagesBits = 8;
        }
        else if (fontImagesFormatStr == "tga24")
        {
            FontImagesFormat = FileTypeConstants::File_TGA;
            FontImagesBits = 24;
        }
        else if (fontImagesFormatStr == "tga32")
        {
            FontImagesFormat = FileTypeConstants::File_TGA;
            FontImagesBits = 32;
        }
        else if (fontImagesFormatStr == "dds" || fontImagesFormatStr == "dds8")
        {
            FontImagesFormat = FileTypeConstants::File_DDS;
            FontImagesBits = 8;
        }
#ifdef SF_TIFF_SUPPORT
        else if (fontImagesFormatStr == "tiff")
        {
            FontImagesFormat = FileTypeConstants::File_TIFF;
            FontImagesBits = 8;
        }
#endif // SF_TIFF_SUPPORT
        else
        {
            Format(errorMsg, "Unknown font image format: {0}. Using default TGA8 format instead...",
                fontImagesFormatStr.ToCStr());
            Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
        }
        fontImagesFmtSpecified = true;
    }

    ExportDefaultEditText = Arguments.GetBool("ExportDefaultEditText");
    if (Arguments.GetBool("DefaultDynamicTextOnly"))
    {
        ExportDefaultEditText = true;
        DefaultDynamicTextOnly = true;
    }

    if (Arguments.GetBool("FsCommandsAsTree"))
    {
        ExportFsCommands = true;
        FsCommandsAsTree = true;
    }
    else if (Arguments.GetBool("FsCommandsAsList"))
    {
        ExportFsCommands = true;
        FsCommandsAsList = true;
    }

    FsCommandsParams        = Arguments.GetBool("FsCommandsParams");
    ExportFonts             = Arguments.GetBool("ExportFonts");
    ExportFontList          = Arguments.GetBool("ExportFontList");
    GlyphsStripped          = Arguments.GetBool("GlyphsStripped");
    UseSeparateFontTextures = Arguments.GetBool("UseSeparateFontTextures");
    CompactFonts            = Arguments.GetBool("CompactFonts");
    FontNormalizedSize      = Arguments.GetInt("FontNormalizedSize");
    FontMergeEdges          = Arguments.GetBool("FontMergeEdges");

    ExportAS3code           = Arguments.GetBool("ExportAS3code");

    if (ExportFonts && CompactFonts)
    {
        Logger->OnMessage(GFxExportListener::MSG_Error, "Error: Compact Fonts can not be exported.");
        ExportFonts = false;
    }
    if (GlyphsStripped && CompactFonts)
    {
        Logger->OnMessage(GFxExportListener::MSG_Error, "Error:  strip_font_shapes is not compatible with Compact Fonts");
        GlyphsStripped = false;
    }

#if 0
    ExportGradients         = Arguments.GetBool("ExportGradients");

    if (Arguments.HasValue("GradientSize"))
    {
        unsigned sz = Arguments.GetInt("GradientSize");
        GradientSize = unsigned(GetNextPow2(UInt32(sz)));

        if (GradientSize != sz)
        {
            Format(errorMsg, "Warning: Gradients' size should be a power of 2. Setting to {0}", GradientSize);
            Logger->OnMessage(GFxExportListener::MSG_Warning, errorMsg);
        }
        if (GradientSize > GFX_MAX_TEXTURE_DIMENSION)
        {
            GradientSize = GFX_MAX_TEXTURE_DIMENSION;
            Format(errorMsg, "Warning: Gradient texture dimensions are too big. Resetting to {0}", GradientSize);
            Logger->OnMessage(GFxExportListener::MSG_Warning, errorMsg);
        }
    }

    if (Arguments.HasValue("GradientImagesFormat"))
    {
        String gradImagesFormatStr = Arguments.GetString("GradientImagesFormat").ToLower();
        if (gradImagesFormatStr == "tga" || gradImagesFormatStr == "tga32")
        {
            GradientImagesFormat = FileTypeConstants::File_TGA;
            GradientImagesBits = 32;
        }
        else if (gradImagesFormatStr == "dds32")
        {
            GradientImagesFormat = FileTypeConstants::File_DDS;
            GradientImagesBits = 32;
        }
        else if (gradImagesFormatStr == "dds")
        {
            GradientImagesFormat = FileTypeConstants::File_DDS;
            GradientImagesBits = 0;
        }
#ifdef SF_TIFF_SUPPORT
        else if (gradImagesFormatStr == "tiff")
        {
            GradientImagesFormat = FileTypeConstants::File_TIFF;
            GradientImagesBits = 32;
        }
#endif // SF_TIFF_SUPPORT
        else
        {
            Format(errorMsg, "Unknown gradient image format: {0}. Using default TGA format instead...",
                gradImagesFormatStr.ToCStr());
            Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
        }
        gradImagesFmtSpecified = true;
    }
#endif

    if (!fontImagesFmtSpecified)
    {
        if (GetImageFormatId() == FileTypeConstants::File_TGA)
        {
            FontImagesFormat = FileTypeConstants::File_TGA;
            FontImagesBits = 8;
        }
        else if (GetImageFormatId() == FileTypeConstants::File_DDS)
        {
            FontImagesFormat = FileTypeConstants::File_DDS;
            FontImagesBits = 8;
        }
    }
    //if (!gradImagesFmtSpecified)
    //{
    //    if (GetImageFormatId() == FileTypeConstants::File_TGA)
    //    {
    //        GradientImagesFormat = FileTypeConstants::File_TGA;
    //        GradientImagesBits = 32;
    //    }
    //    else if (GetImageFormatId() == FileTypeConstants::File_DDS)
    //    {
    //        GradientImagesFormat = FileTypeConstants::File_DDS;
    //        GradientImagesBits = 32;
    //    }
    //}

    PackImages = Arguments.GetBool("PackImages");
    PackImageSize = Arguments.GetInt("PackImageSize");
    bSquareImages = Arguments.GetBool("SquareImages");
	const char* resizeName = Arguments.GetString("PackTextureResize", 0).ToCStr();
	if (SFstricmp(resizeName, "no") == 0)
		PackImageResize = ImagePackParams::PackSize_1;
	else if (SFstricmp(resizeName, "mult4") == 0)
		PackImageResize = ImagePackParams::PackSize_4;
    else if (SFstricmp(resizeName, "mult128") == 0)
        PackImageResize = ImagePackParams::PackSize_128;
	else 
		PackImageResize = ImagePackParams::PackSize_PowerOf2;

    if (Arguments.GetBool("PadTextures"))
    {
        PackImages = true;
        PackImageSize = 2;
    }

    if (Arguments.GetBool("quick"))
        Quality = ExportImageHelper::QualityFastest;
    else if (Arguments.GetBool("quality_normal"))
        Quality = ExportImageHelper::QualityNormal;
    else if (Arguments.GetBool("quality_production"))
        Quality = ExportImageHelper::QualityProduction;
    else if (Arguments.GetBool("quality_highest"))
        Quality = ExportImageHelper::QualityHighest;

	int listSize = (int) Arguments.GetListSize("Rescale");
	if (listSize > 0)
	{

		const char* prescaleName = Arguments.GetString("Rescale", 0).ToCStr();
		if (SFstricmp(prescaleName, "nearest") == 0)
			Rescale = ExportImageHelper::RescaleNearestPower2;
		else if (SFstricmp(prescaleName, "hi") == 0)
			Rescale = ExportImageHelper::RescaleBiggestPower2;
		else if (SFstricmp(prescaleName, "low") == 0)
			Rescale = ExportImageHelper::RescaleSmallestPower2;
		else if (SFstricmp(prescaleName, "nextlow") == 0)
			Rescale = ExportImageHelper::RescaleNextSmallestPower2;
		else if (SFstricmp(prescaleName, "mult4") == 0)
		{
			Rescale = ExportImageHelper::RescalePreScale;
		}
		else
		{
            Format(errorMsg, "Error: Invalid rescale mode is specified: '{0}'", prescaleName);
            Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
		}

		if ( listSize > 1)
		{
			const char* pfilter = Arguments.GetString("Rescale", 1).ToCStr();
			int f = ParseRescaleFilterType(pfilter);
			if (f >= 0)
				RescaleFilter = (ExportImageHelper::RescaleFilterTypes)f;
			else //could be filename
			{
				const char* pstr = strrchr(pfilter, '.');
				if (pstr && String::CompareNoCase(pstr, ".swf") == 0)
					ParseFilename(pfilter);
				else
                {
                    Format(errorMsg, "Error: Invalid rescale filtering is specified: '{0}'", Arguments.GetString("Rescale", 1).ToCStr());
                    Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                }
			}
		}
		for (int i = 2; i < listSize;i++) //consider everything else filename
			ParseFilename(Arguments.GetString("Rescale", i).ToCStr());
	}

    //if (PackImages && (Rescale != ExportImageHelper::RescaleNone))
    //{
    //    Rescale = ExportImageHelper::RescaleNone;
    //    Logger->OnMessage(GFxExportListener::MSG_Error, "-rescale option is not compatible with -pack option and will be ignored"
    //        "\n use -ptresize for atlas texture size options");

    //}

    int f = ParseMipFilterType(Arguments.GetString("MipFilter").ToCStr());
    if (f >= 0)
        MipFilter = (ExportImageHelper::MipFilterTypes)f;
    else
    {
        Format(errorMsg, "Error: Invalid mipmap filtering is specified: '{0}'", Arguments.GetString("MipMap").ToCStr());
        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
    }

    // if file format is DDS, rescale mode - mult4 then mipmap levels cannot be generated by DXTLib. 
    // The DXTLib crashes in this case.
    if (GetImageFormatId() == FileTypeConstants::File_DDS && Rescale == ExportImageHelper::RescalePreScale && GenMipMapLevels)
    {
        Logger->OnMessage(GFxExportListener::MSG_Warning, "Warning: Deminesions should be power of 2 for generation of mipmaps.");
        GenMipMapLevels = false;
    }

    if ( Arguments.GetListSize("LoadPlugin") > 0)
    {
        PluginHandler.LoadPluginLibrary(Arguments.GetString("LoadPlugin", 0).ToCStr());
        for(unsigned i = 1; i < Arguments.GetListSize("LoadPlugin"); i++ )
            PluginHandler.ParseCommandlineArg(Arguments.GetString("LoadPlugin", i).ToCStr());
    }
}

void              GFxDataExporter::AddImageExporter(int imgFormatId, GFxImageExporter* pimgExp)
{
    ImageExporters.Set(imgFormatId, pimgExp);
}

GFxImageExporter* GFxDataExporter::GetImageExporter(int imgFormatId) const
{
    GFxImageExporter* ptr = 0;
    ImageExporters.Get(imgFormatId, &ptr);
    return ptr;
}

#ifdef GFX_ENABLE_SOUND

void              GFxDataExporter::SetSoundExporter(GFxSoundExporter* exporter)
{
    pSoundExporter.Reset(exporter);
    if (exporter)
        ExpLoader.SetAudio(Ptr<Audio>(*new Audio(NULL)));
    else
        ExpLoader.SetAudio(NULL);
}
GFxSoundExporter* GFxDataExporter::GetSoundExporter()
{
    return pSoundExporter.GetPtr();
}
#endif

void GFxDataExporter::ShowVersion(GFxExportListener *logger)
{
    String msg;
    Format(msg, "GFxExport v{0}.{1:2.2d} for SDK v{2}.{3}.{4}, (c) 2006-2011 Scaleform Corporation",
        (GFX_EXPORT_VERSION>>8), (GFX_EXPORT_VERSION&0xFF),
        GFX_MAJOR_VERSION, GFX_MINOR_VERSION, GFX_BUILD_VERSION);
    logger->OnMessage(GFxExportListener::MSG_Version, msg);

    logger->OnMessage(GFxExportListener::MSG_Version, "This program uses:");
    logger->OnMessage(GFxExportListener::MSG_Version, "\tNVIDIA Texture Tools 2, (c) 2007 NVIDIA Corporation");
    logger->OnMessage(GFxExportListener::MSG_Version, "\tPVRTexLib (c) 2010, Imagination Technologies Ltd.");

#ifdef SF_TIFF_SUPPORT
    logger->OnMessage("\tTIFF Image exporter extension, (c) 2008 Crytek");
#endif // SF_TIFF_SUPPORT
}

void GFxDataExporter::ShowHelpScreen()
{
    ShowVersion(Logger);
    Arguments.PrintOptionsHelp();
    PluginHandler.PrintUsage();
}


