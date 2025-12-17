/**************************************************************************

Filename    :   GFxExport.h
Content     :   SWF to GFX resource extraction and conversion tool
Created     :   May, 2007
Authors     :   Artyom Bolgar, Dmitry Polenur, Maxim Didenko

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFXEXPORT_H
#define INC_GFXEXPORT_H


//#include "Render/Render_Image.h"
//#include "Render/Render_JPEGUtil.h"
#include "Render/ImageFiles/TGA_ImageFile.h"
#include "Render/ImageFiles/JPEG_ImageFile.h"
#include "Render/ImageFiles/DDS_ImageFile.h"
#include "JPEGUtil.h"

#include "Kernel/SF_Std.h"
#include "Kernel/SF_Types.h"
#include "Kernel/SF_AutoPtr.h"
#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_File.h"
#include "Kernel/SF_String.h"


#include "GFx/GFx_ZLibFile.h"
#include "GFx/GFx_Player.h"
#include "GFx/GFx_Loader.h"
#include "GFx/GFx_Log.h"
#include "GFx/GFx_Stream.h"
#include "GFx/GFx_FontResource.h"
//#include "GFx/GFx_Styles.h"

#include "GFx/Audio/GFx_Sound.h"

#include "FxSettings.h"
#include "ExportImageHelper.h"
#include "GFxExportPluginHandler.h"

using namespace Scaleform;
using namespace GFx;
using namespace Render;

class  GFxDataExporter;
struct GFxFsCommandOrigin;
struct GFxFsCommand;

class GFxExportListener
{
public:
    virtual ~GFxExportListener() {}

    enum OutputType
    {
        OUT_ImageFile,
        OUT_SoundFile,
        OUT_SpriteSoundFile,
        OUT_GFxFile,
        OUT_CFxFile,
        OUT_FontList,
        OUT_EditTextfieldList,
        OUT_DynamicTextfieldList,
        OUT_FileList,
        OUT_AbcFile,
    };
    struct OutputPayload
    {
        String      Name;
        String      FilePath;
        union 
        {
            int         ImageWidth;
            int         GFxTotalImagesWritten;
        };
        union
        {
            int         ImageHeight;
            int         GFxEstMemSize;
        };
        union
        {
            int         ImageEstSize;
            int         GFxEstFileSize;
        };               
        
        OutputPayload() : ImageWidth(0), ImageHeight(0), ImageEstSize(0) {}
    };
    virtual void    OnOutput(OutputType type, const OutputPayload &data) = 0;

    enum ProcMode
    {
        PROC_Swf,
        PROG_Images,
        PROG_Sounds,
        PROG_SpriteSounds,
        PROC_Other,
    };
    struct ProcData
    {
        String  SwfFilename;
        int     WrittenImagesCount;
        int     SharedImagesCount; 
        int     WrittenSoundsCount;
        ProcData() : WrittenImagesCount(0), SharedImagesCount(0), WrittenSoundsCount(0) {}
    };
    virtual void    OnProcBegin(ProcMode type, const ProcData& data)                      = 0;
    virtual void    OnProcProgress(ProcMode type, int progress)     = 0;
    virtual void    OnProcEnd(ProcMode type, const ProcData& data)  = 0;

    enum MessageType
    {
        MSG_Normal,
        MSG_Warning,
        MSG_Error,

        MSG_Version,
    };
    virtual void    OnMessage(MessageType type, const char *msg)    = 0;
};

struct GFxExportImageDescr
{
    Image*				pImage;
    String              ImageName;
    String              ExportName;
    String              SwfName;
    UInt32              CharacterId;
    unsigned            TargetWidth;
    unsigned            TargetHeight;
    UInt16              Format;

    ImageRect           UsedRect;           // Area of BaseImageID used for this resource
    UInt32              BaseImageId;        // CharacterID of packed texture
    bool                IsPackedImage;
};
typedef HashIdentity<UInt32, GFxExportImageDescr>               ExportImagesHashType;

struct GFxImageExporterParams
{
    enum ImageExportType
    {
        IET_Image,
        IET_Font
    };

    UInt32                      CharacterId;
    const GFxDataExporter*      Exporter;
    Ptr<Render::Image>			ImagePtr;
    GFxExportImageDescr*        ImageDescr;
    Ptr<File>                   FilePtr;
    ExportImageHelper::RescaleTypes    Rescale;
    ImageExportType             ExpType;

    GFxImageExporterParams(UInt32 charId = 0, const GFxDataExporter* exp = NULL, 
                           GFxExportImageDescr* imgDescr = NULL, Render::RawImage* image = NULL, 
                           File* file = 0, ImageExportType expType = IET_Image) :
        CharacterId(charId),
        Exporter(exp),
        ImagePtr(image),
        ImageDescr(imgDescr),
        FilePtr(file),
        Rescale(),
        ExpType(expType)
    { 
    }

    void Set(UInt32 charId, const GFxDataExporter* pexp, GFxExportImageDescr* pimgDescr, Render::Image* pimage, File* pfile = 0, ImageExportType expType = IET_Image);
    void Clear() 
    { 
        ImagePtr = 0; 
    }
};


struct GFxFontTextureDescr
{
    struct FontDescr
    {
        int             FontId;
        Ptr<FontResource>   pFont;
        struct Pair
        {
            int IndexInFont;
            int IndexInTexture;

            inline Pair() : IndexInFont(0), IndexInTexture(0) {}
            inline Pair(int i1, int i2) : IndexInFont(i1), IndexInTexture(i2) {}
        };
        Array<Pair> TexGlyphsIndicesMap; // map index of tex glyphs (index_of_tex_glyph_in_font <=> (index in texture))
    };
    Ptr<ImageBase>    pTexture;
    Array<FontDescr>  Fonts;

    Array<Ptr<GFx::TextureGlyph> > TextureGlyphs;
};

typedef HashIdentity<UInt32, GFxFontTextureDescr>               FontTexturesHashType;



struct GFxFontTextureCounter : public RefCountBase<GFxFontTextureCounter, Stat_Default_Mem>
{
    unsigned Counter;

    GFxFontTextureCounter(unsigned c):Counter(c) {}
};
typedef HashIdentity<UInt32, Ptr<GFxFontTextureCounter> >       FontTextureCounterHashType; // TexId -> Counter
typedef HashIdentity<UInt16, FontTextureCounterHashType >       FontTextureUseHashType; // FontId -> Texture Ids + UseCnt

struct JpegDesc
{
    int             TagType;
    UByte*          pData;
    unsigned            DataSize;

    inline JpegDesc() : pData(0) {}
};


class GFxExporterBase
{
public:
    virtual ~GFxExporterBase() {};

    // Makes and returns full name of the file with extension (w/o path)
    virtual String   MakeFileName(const String& nameNoExt)
    {
        return nameNoExt + "." + GetDefaultExtension();
    }
    // Makes and returns full path to the file
    virtual String   MakePathAndAddExt(const String& path, const String& nameNoExt)
    {
        return path + nameNoExt + "." + GetDefaultExtension();
    }
    // Makes and returns full path to the file
    virtual String   MakePath(const String& path, const String& nameWithExt)
    {
        return path + nameWithExt;
    }
    // Returns default extension of the file
    virtual const char* GetDefaultExtension() const = 0;

    virtual void Clear() {}
};

// Interface for plug-in image exporters.
// Each implementation of this interface should also create and store the instance
// of GFxImageExporterParams. This instance should be accessible at least by InitParams() 
// and Write() methods.
class GFxImageExporter : public GFxExporterBase
{
public:
    virtual ~GFxImageExporter() {}

    // Returns short name of format, like "TGA", "DDS", etc
    virtual const char* GetName() const         = 0;
    // Returns description that will be displayed on help screen
    virtual const char* GetDescription() const  = 0;
    // Returns format id. Should be one of the FileTypeConstants::Image_<> constant,
    // or beyond FileTypeConstants::Image_NextAvail
    virtual int         GetFormatId() const     = 0;

    // Initializes and returns GFxImageExporterParams structure
    virtual GFxImageExporterParams& InitParams(UInt32 charId, const GFxDataExporter* pexp, 
                               GFxExportImageDescr* pimgDescr, Render::Image* pimage, File* pfile = 0, 
                               GFxImageExporterParams::ImageExportType expType = GFxImageExporterParams::IET_Image) = 0;

    // Returns true, if image could be rescaled, if -rescale option is used
    virtual bool        MightBeRescaled() const = 0;

    // Returns true, if image need to be converted from the pimage->Format to destFormat.
    virtual bool        NeedToBeConverted(Render::Image* pimage, int destFormat) const = 0;
    
    // Write image to the disk.
    virtual bool        Write(ExportImageHelper* pdxtHepler, GFxExportListener *logger) = 0;

    // Read file and create Image. This method used for sharing images. If this method
    // is not implemented (returns NULL) then images of this format cannot be shared
    virtual Render::Image*     Read(const char* filePath, GFxExportListener *logger) { SF_UNUSED2(filePath, logger); return NULL; }
};

struct GFxExportSoundAttrs
{
    UInt16           Format;
    UInt32           SampleRate;
    UInt16           Bits;
    UInt16           Channels;
    UInt32           SampleCount;
};

#ifdef GFX_ENABLE_SOUND
struct GFxExportSoundDescr
{
    Sound::SoundInfo*   Info;
    String              SoundName;
    String              ExportName;
    String              SwfName;
    GFxExportSoundAttrs Attrs;
    unsigned            StartFrame;
    unsigned            LastFrame;
};

class GFxSoundExporter : public GFxExporterBase
{
public:
    virtual ~GFxSoundExporter() {}

    virtual bool WriteWaveFile(Sound::SoundData*, File*, GFxExportSoundAttrs* ) = 0;
    virtual bool WriteWaveFile(Sound::AppendableSoundData*, File*, GFxExportSoundAttrs* ) = 0;
};
#endif // GFX_ENABLE_SOUND

class ExportLoader : public Loader
{
public:
    bool DoesNeedTag(int tagType) const
    {
        return (tagType <= 1 || Loader::CheckTagLoader(tagType));
    }
};

class GFxDataExporter
{
public:
    //enum PlatformFormat
    //{
    //    PF_Generic,
    //    PF_Xbox360,
    //    PF_PS3,
    //    PF_WII,
    //    PF_Vita,
    //    PF_3DS,
    //    PF_IOS,
    //};

    GFxDataExporter(GFxExportListener* plog, UByte verboseFlags = 0);
    ~GFxDataExporter();

    bool    CreateDestDirectory(const char* path);

    int     Process();
	void    ParseFilename(const char* pfilename);
    int     ParseCommandLine(int argc, char* argv[]);

    int     CollectOriginalImageData(const char* srcFileName);
    void    ClearOriginalImageData();

    void                AddImageExporter(int imgFormatId, GFxImageExporter* pimgExp);
    GFxImageExporter*   GetImageExporter(int imgFormatId) const;

#ifdef GFX_ENABLE_SOUND
    void                SetSoundExporter(GFxSoundExporter*);
    GFxSoundExporter*   GetSoundExporter();
#endif

    void                ShowHelpScreen();

    // static methods
    static void         ShowVersion(GFxExportListener *logger);

    void    ExtractImage(ImageBase* pimageInfoBase, ResourceId rid, const char* pswfName, 
        const char* pdestPath, const char* pexportName, ImageResource* presource = NULL);

    void    ExtractFont(FontResource* pfontResource, int fontId, ResourceBinding* pResBinding);

#ifdef GFX_ENABLE_SOUND
    void    ExtractSound(Sound::SoundInfoBase* psoundData, ResourceId rid, const char* pswfName, 
        const char* pdestPath, const char* pexportName);

    void    ExtractSpriteSound(SoundStreamDef* psoundData, ResourceId rid, const char* pswfName, 
        const char* pdestPath, const char* pexportName);
#endif

    ExportImageHelper::RescaleTypes        GetRescale() const { return Rescale; }
    ExportImageHelper::QualitySetting      GetQuality() const { return Quality; }
    ExportImageHelper::RescaleFilterTypes  GetRescaleFilter() const { return RescaleFilter; }
    ExportImageHelper::MipFilterTypes      GetMipFilter() const { return MipFilter; }
    int                             GetDXTn() const { return DXTn; }
    int                             GetPVRn() const { return PVRn; }
    ImageTarget                     GetPlatform() const {return TargetPlatform;}
    bool                            IsGenMipMapLevels() const { return GenMipMapLevels; }
    bool                            IsUncompressedTexture() const { return UncompressedTexture; }
    bool                            IsDXT1Allowed() const { return DXT1Allowed; }

    const JpegDesc*                 GetJpegDescriptor(int key) const { return JpegDescriptors.Get(key); }

    static void         WriteTagHeader(File* pout, unsigned tag, unsigned len);

    // Removes the leading directory path
    static String    CutPath(const String& filename);
    // Removes the trailing filename extension
    static String    CutExtension(const String& filename);
    ExportImageHelper*              GetDXTCompressor () const {return pDXTCompressor; }
    ExportImageHelper*              GetPVRCompressor () const {return pPVRCompressor; }
    ExportImageHelper*              GetSIFCompressor () const {return pSIFCompressor; }
    bool                            IsInfoMode() const  { return Info; }
    bool                            SquareImages() const { return bSquareImages; }
private:
    FxArgs                          Arguments; 
    Ptr<ExportImageHelper>          pExportCompressor;
    Ptr<ExportImageHelper>          pDXTCompressor;
    Ptr<ExportImageHelper>          pPVRCompressor;
    Ptr<ExportImageHelper>          pSIFCompressor;
    GFxExportPluginHandler          PluginHandler;
    Ptr<MovieDef>                   MovieDefPtr;
    MovieInfo                       ExpMovieInfo;
    ExportLoader                    ExpLoader;
    Hash<int, GFxImageExporter*>    ImageExporters;
    Hash<UPInt, Array<String> >     SharedImagesHash;
    StringHash<int>                 SharedImageNamesSet;
    ImageTarget                     TargetPlatform;
    Ptr<ImageSubstProvider>         pImgSubstProvider;

    FontTexturesHashType        FontTextures;
    FontTextureUseHashType      FontTextureUse;

    Array<String>               FileList; // list of all generated files
    bool                        GenFileList;
    String                      FileListName;

    // Options
    Array<String>               SrcFileNames;
    String                      OutputRootDir;
    String                      OutputGfxDir;
    String                      ImgSubstDir;
    String                      Prefix;
    String                      ImagesFormat;   // TGA, DDS....
    String                      SoundExt;   // WAV, ...
    bool                        ExportSounds;
    bool                        ReplaceImages;
    bool                        Info;           //Information only, files are not written 
    bool                        DoCompress;
    bool                        DoStrip;
    bool                        DoCreateSubDirs;
    bool                        UncompressedTexture;
    bool                        ToUppercase;
    bool                        ToLowercase;
    bool                        NoExportNames;
    bool                        DXT1Allowed;    // for non-alpha RGB data
    int                         DXTn;           // 3 or 5
    int                         PVRn;
    bool                        GenMipMapLevels;
    bool                        Quiet;
    bool                        QuietProgress;
    bool                        bSquareImages;
    ExportImageHelper::QualitySetting      Quality;
    ExportImageHelper::RescaleTypes        Rescale;
    ExportImageHelper::RescaleFilterTypes  RescaleFilter;
    ExportImageHelper::MipFilterTypes      MipFilter;
    bool                        JustStripImages;
    bool                        ShareImages;
    // Edit Textfields 
    bool                        ExportDefaultEditText;
    bool                        DefaultDynamicTextOnly;
    // FS Commands
    bool                        ExportFsCommands;
    bool                        FsCommandsAsTree;
    bool                        FsCommandsAsList;
    bool                        FsCommandsParams;
    // font options
    bool                        ExportFonts;
    bool                        ExportFontList;
    bool                        GlyphsStripped;
    int                         TextureGlyphNominalSize;
    int                         TextureGlyphPadPixels;
    int                         FontTextureWidth;
    int                         FontTextureHeight;
    Loader::FileFormatType      FontImagesFormat;   // TGA, DDS
    UByte                       FontImagesBits;     // 8, 24 or 32
    bool                        UseSeparateFontTextures;
    bool                        CompactFonts;
    unsigned                    FontNormalizedSize;
    bool                        FontMergeEdges;

    bool                        ExportAS3code;

    // image packer
    bool                                PackImages;
    unsigned                            PackImageSize;
    ImagePackParamsBase::SizeOptionType PackImageResize;

    ExportImagesHashType        ImageDescriptors;
    Array<TagInfo>              TagsToBeCopied, TagsToBeRemoved, TagsWithActions, TagsActions3;
    Array<TagInfo>          ActionTags;
    Hash<int,JpegDesc>         JpegDescriptors;
    UInt32                      FirstTagOffset; 
    int                         TotalImageCount;
    int                         ImagesWrittenCount;
    int                         ImagesProcessedCount;
    int                         SharedImagesCount;
    UInt32                      TotalMemoryCount; //used for Info option

#ifdef GFX_ENABLE_SOUND
    typedef HashIdentity<UInt32, GFxExportSoundDescr>                ExportSoundsHashType;

    int                         TotalSoundCount;
    int                         SoundsWrittenCount;
    int                         SoundsProcessedCount;
    int                         StreamSoundsWrittenCount;
    int                         StreamSoundsProcessedCount;

    struct StreamSoundData
    {
        bool                    IsHeadTagSet;
        TagInfo              DefSpriteTag;
        TagInfo              HeadTag;
        Array<TagInfo>      BlockTags;
    };
    Array<StreamSoundData>     StreamSoundsData;

    AutoPtr<GFxSoundExporter>  pSoundExporter;
    ExportSoundsHashType        SoundDescriptors;
#endif

    GFxExportListener                *Logger;

    void                InitializeArguments();

    static File*       FileOpener(const char* url);

    void LoadTagProgressCallback(const ProgressHandler::TagInfo& info, bool calledFromDefSprite);
    void ProgressCallback(const ProgressHandler::Info& info);

    int     Load(const String& fileName);

    int     GetImageFormatId() const;

    bool WriteStrippedSwf(const char* srcFileName, const char* dstFileName, const String& name);
    void    WriteExternalImageTag(File* pout, UInt32 characterId, const GFxExportImageDescr& imgDescr);
	void WriteSubImageTag(File* pout, UInt32 characterId, UInt32 baseCharId, const ImageRect& rect);

    void    WriteGlyphTextureInfo(File* pout, unsigned textureId, const GFxFontTextureDescr& textureDescr);

    void    WriteDefaultEditTextFieldText(MovieDef* pmovieDef, const char* swfFileName, const String& path, const String& name);

    void    WriteFontList(MovieDef* pmovieDef, const char* swfFileName, const String& path, const String& name);

    void    WriteListOfFiles(const String& path, const String& name);

    void    WriteActions3(MovieDef* pmovieDef, const char* swfFileName, const String& path);

#ifdef GFX_ENABLE_SOUND

    void    WriteExternalSoundTag(File* pout, UInt32 characterId, const GFxExportSoundDescr& sndDescr);
    //void    WriteStreamSoundFile(SoundData* psound, const char* pdestPath, const char* name, GFxExportSoundAttrs* sndAttrs, String* soundName);
    void    WriteStreamSoundTag(File* pout, const GFxExportSoundDescr& sndDescr);
    unsigned    GetStreamSoundTagLen(const GFxExportSoundDescr& sndDescr);
#endif

    enum FsCommandsMasks
    {
        FSTree = 1,
        FSList = 2
    };
    void    WriteFsCommands(MovieDef* pmovieDef, const char* swfFileName, const String& path, const String& name, unsigned mask);
    bool FindIdenticalImageInDir(const String& imagePath, const String& filePath, String* presultImagePath, GFxExportImageDescr* pdescr, GFxImageExporter* pimageExporter);
    bool FindIdenticalImage(const String& imagePath, const String& filePath, String* presultImagePath, GFxExportImageDescr* pdescr,  GFxImageExporter* pimageExporter);

    void                DumpFsCommandsAsTree(FILE* fout, Array<GFxFsCommandOrigin>& fscommands, 
        Array<String>& stringHolderArray, int indent);
    void                MakeFsCommandsAsList(Array<GFxFsCommandOrigin>& fscommands, 
        Array<String>& stringHolderArray, 
        Array<String>& cmdList,
        Array<int>& cmdSortedIdx);
    void                LookForFsCommandsInTags(MovieDef* pmovieDef, 
        File* pin, unsigned finalOffset, 
        Array<GFxFsCommandOrigin>& fscommands, 
        Array<String>& stringHolderArray);


};

struct GFxImageExporterFactoryBase
{
    virtual ~GFxImageExporterFactoryBase() { }
    virtual GFxImageExporter* Create() = 0;
};

#define GFX_MAX_NUM_OF_IMAGE_EXPORTERS  20
extern GFxImageExporterFactoryBase* GFxImageExporterFactoriesArray[];
extern unsigned                     GFxImageExporterFactoriesArrayCount; 

template<typename T>
struct ImageExporterFactory : public GFxImageExporterFactoryBase
{
    GFxImageExporter* Create() { return new T(); }
    ImageExporterFactory() { GFxImageExporterFactoriesArray[GFxImageExporterFactoriesArrayCount++] = this; }
};

#define GFX_REGISTER_IMAGE_EXPORTER(exporterClass) ImageExporterFactory<exporterClass> __instance_of_##exporterClass##_factory

#ifdef GFX_ENABLE_SOUND
GFxSoundExporter* CreateSoundExporter(GFxExportListener *logger);
#endif

#endif //INC_GFXEXPORT_H
