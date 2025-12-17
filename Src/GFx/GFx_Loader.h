/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_Loader.h
Content     :   Public SWF loading interface for GFxPlayer
Created     :   June 21, 2005
Authors     :   Michael Antonov

Notes       :   Redesigned to use inherited state objects for GFx 2.0.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_SF_GFX_LOADER_H
#define INC_SF_GFX_LOADER_H

#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Log.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_StringHash.h"
#include "Kernel/SF_File.h"
#include "Kernel/SF_System.h"
#include "Kernel/SF_WString.h"

#include "GFx/GFx_Types.h"
//#include "Render/Render_JPEGUtil.h"
#include "GFx/GFx_Resource.h"
#include "GFx/GFx_PlayerStats.h"

#include "Render/Text/Text_WWHelper.h"
#include "GFx/GFx_Tags.h"
#include "GFx/GFx_CharacterDef.h"
//#include "GFx/GFx_ASIMEManager.h" 
#include "Render/Render_Font.h"

namespace Scaleform { 

class   File;
class   KeyModifiers;

namespace Render
{
    class   Font;
    class   Image;
//    class   ImageInfoBase;
    class   Texture;

    namespace Text {
        class StyledText;
    }
}

#ifdef GFX_ENABLE_SOUND
namespace Sound
{
    class   SoundRenderer;
}
#endif

namespace GFx {

// ***** Declared Classes
struct  MovieInfo;
struct  TagInfo;
class   StateBag;
class   Loader;
class   Translator;
class	ASIMEManager;
struct  ExporterInfo;
struct  ImageFileInfo;
class   ImageCreateInfo;

// ***** External Classes
class   MovieDef;
class   MovieDataDef;
class   TestStream;
struct  MemoryParams;
class   MemoryContext;

// Externally declared states:
class   LogState;
class   ExternalInterface;
class   ExtensionContextInterface;
class   MultitouchInterface;
class   VirtualKeyboardInterface;
class   AccelerometerInterface;
class   GeolocationInterface;
class   ImportVisitor;
class   TaskManager;
class   FontLib;
class   IMEManagerBase;
class   ZlibSupportBase;
class   FontCompactorParams;
class   SharedObjectManagerBase;

class   LoadProcess;
class   MovieDefImpl;
class   ASMovieRootBase;
class   MovieImpl;
class   PlaceObject2Tag;
class   PlaceObject3Tag;
class   RemoveObjectTag;
class   RemoveObject2Tag;

class   DisplayObjectBase;
class   InteractiveObject;
class   CharacterDef;
class   ButtonDef;

class   ImageCreator;
class   ImageFileHandlerRegistry;
struct  CharacterCreateInfo;

namespace Text
{
    using Render::Text::StyledText;
    using Render::Text::WordWrapHelper;
}

#ifdef GFX_ENABLE_XML
namespace XML 
{
    class   SupportBase;
}
#endif

#ifdef GFX_ENABLE_SOUND
class   AudioBase;
#endif
#ifdef GFX_ENABLE_VIDEO
namespace Video {
class   VideoBase;
}
#endif

// ***** GFxLoader States

// GFxState - a base class for all Loader/Movie states in GFx. These states are
// accessible through the  GShadedState::SetState/GetStateAddRef functions.

class State :  public RefCountBase<State, Stat_Default_Mem>
{
public:
    enum StateType
    {
        State_None,

        // *** Instance - related states.
        State_Translator,
        State_Log,
        State_ActionControl,    // Verbose Action
        State_UserEventHandler,
        State_FSCommandHandler,
        State_ExternalInterface,
		State_MultitouchInterface,
		State_ExtensionContextInterface,
        State_VirtualKeyboardInterface,
		State_AccelerometerInterface,
		State_GeolocationInterface,

        // *** Loading - related states.
        State_FileOpener,
        State_URLBuilder,
        State_ImageCreator,
        State_ImageFileHandlerRegistry,
        State_ParseControl,     // Verbose Parse
        State_ProgressHandler,
        State_ImportVisitor,
        State_FontPackParams,
        State_FontLib,
        State_DefaultFontLibName,
        State_FontProvider,
        State_FontMap,
        State_TaskManager,
        State_Clipboard,
        State_TextKeyMap,
        State_IMEManager,
        State_XMLSupport,
        State_ZlibSupport,
        State_FontCompactorParams,
        State_ImagePackerParams,
        State_Audio,
        State_Video,
        State_TestStream,
        State_SharedObject,
        State_UrlNavigator,
        State_LocSupport,

        State_AS2Support,
        State_AS3Support
    };

protected:        
    StateType       SType;
public:

    // State Ref-Counting and interaction needs to be thread-safe.
    State(StateType st = State_None) : SType(st) { }
    virtual ~State() { }

    // Use inline to make state lookups more efficient.
    inline StateType   GetStateType() const { return SType;  }
};


// ***** Translator

// Translator class is used for international language translation of dynamic
// text fields. Translation is performed through the Translate virtual function
// which is expected to fill in a buffer with a translated string.

class Translator : public State
{
public:
    enum TranslateCaps
    {
        // Specifies that Translate key can include Flash-HTML tags. If not specified
        // translate will only receive stripped text content (default).
        Cap_ReceiveHtml = 0x1,
        // Forces to strip all trailing new-line symbols before the text is passed
        // to Translate.
        Cap_StripTrailingNewLines = 0x2,

        // Enables support for bidirectional text. Method OnBidirectionalText will be called.
        Cap_BidirectionalText     = 0x4
    };
    // Handling custom word-wrapping. To turn it on you need to enable custom word-wrapping
    // in GetCaps by adding Cap_CustomWordWrapping bit.
    enum WordWrappingTypes
    {
        WWT_Default       = 0,   // OnWordWrapping will not be invoked
        WWT_Asian         = Text::WordWrapHelper::WWT_Asian, // mostly Chinese
        WWT_Prohibition   = Text::WordWrapHelper::WWT_Prohibition, // Prohibits certain chars at start/end of line ("Japanese prohibition rule") 
        WWT_NoHangulWrap  = Text::WordWrapHelper::WWT_NoHangulWrap, // Korean-specific rule
        WWT_Hyphenation   = (Text::WordWrapHelper::WWT_Last<<1), // very simple hyphenation; for demo only
        WWT_Custom        = 0x80, // user defined word-wrapping.

        WWT_Korean        = WWT_Prohibition | WWT_NoHangulWrap,
        WWT_Japanese      = WWT_Prohibition,
        WWT_Chinese       = WWT_Prohibition | WWT_Asian
    };
    unsigned               WWMode; // combination of WWT_- flags

    Translator() : State(State_Translator), WWMode(WWT_Default) { }
    explicit Translator(unsigned wwMode) : State(State_Translator), WWMode(wwMode) { }
  
    // Override this function to specify whether Translate can receive and/or return HTML.
    virtual unsigned GetCaps() const         { return 0; }
    inline  bool     CanReceiveHtml() const  { return (GetCaps() & Cap_ReceiveHtml) != 0; }
    inline  bool     NeedStripNewLines() const { return (GetCaps() & Cap_StripTrailingNewLines) != 0; }
    inline  bool     HandlesCustomWordWrapping() const { return (WWMode != WWT_Default); }
    inline  bool     HandlesBidirectionalText() const { return (GetCaps() & Cap_BidirectionalText) != 0; }


    // This class provides data to and from the 'Translate' method, such as original 
    // text, name of textfield's instance, resulting translated text.
    class TranslateInfo
    {
    protected:
        const wchar_t*  pKey;
        WStringBuffer*  pResult;
        const char*     pInstanceName;
        enum
        {
            Flag_Translated = 0x01,
            Flag_ResultHtml = 0x02,
            Flag_SourceHtml = 0x04
        };
        UInt8               Flags;
    public:
        TranslateInfo():pKey(NULL), pResult(NULL), pInstanceName(NULL), Flags(0) {}

        // *Input methods.
        // Returns 'key' string - the original text value of the textfield being translated.
        const wchar_t*  GetKey() const          { return pKey; }

        // Returns true, if key (returned by 'GetKey()') is HTML. Note, this
        // may happen only if flag Translator::Cap_ReceiveHtml is set in value
        // returned by Translator::GetCaps(). If this flag is not set and textfield 
        // contains HTML then HTML tags will be stripped out and IsKeyHtml() will return
        // 'false'.
        bool            IsKeyHtml() const { return (Flags & Flag_SourceHtml) != 0; }

        // Returns the instance name of the textfield being translated.
        const char*     GetInstanceName() const { return pInstanceName; }
        
        // *Output methods.
        // Sets translated sting as a plain text. Two variants are supported - wide-character 
        // and UTF-8. Length is optional; null-terminated string should be used if it is omitted.
        // 'resultLen' for wide-char variant indicates number of characters (not bytes) in the string.
        void            SetResult(const wchar_t* presultText, UPInt resultLen = SF_MAX_UPINT);
        // 'resultUTF8Len' for UTF-8 variant indicates number of bytes (not characters) in the string.
        void            SetResult(const char* presultTextUTF8, UPInt resultUTF8Len = SF_MAX_UPINT);

        // Sets translated sting as a HTML text. HTML will be parsed before applying to the textfield.
        // Two variants are supported - wide-character and UTF-8. Length is optional; 
        // null-terminated string should be used if it is omitted.
        // 'resultLen' for wide-char variant indicates number of characters (not bytes) in the string.
        void            SetResultHtml(const wchar_t* presultHtml, UPInt resultLen = SF_MAX_UPINT)
        {
            SetResult(presultHtml, resultLen);
            Flags |= Flag_ResultHtml;
        }
        // 'resultLen' for UTF-8 variant indicates number of bytes (not characters) in the string.
        void            SetResultHtml(const char* presultHtmlUTF8, UPInt resultLen = SF_MAX_UPINT)
        {
            SetResult(presultHtmlUTF8, resultLen);
            Flags |= Flag_ResultHtml;
        }
    };

    // Performs lookup of 'ptranslateInfo->GetKey()' string for language translation, 
    // filling in the destination string buffer by calling 'SetResult' or 'SetResultHtml' 
    // method. 'ptranslateInfo' is guaranteed to be not null.
    // If neither 'SetResult' nor 'SetResultHtml' is called then original text will not
    // be changed.
    virtual void Translate(TranslateInfo* ptranslateInfo) { SF_UNUSED(ptranslateInfo); }

    // This struct provides information of the line text to be formatted like the length 
    // of the text, text position etc. It is mainly used by OnWordWrapping 
    // to control word wrapping of the text.
    struct LineFormatDesc
    {
        const wchar_t*  pParaText;              // [in] paragraph text
        UPInt           ParaTextLen;            // [in] length, in chars
        const float*    pWidths;                // [in] array of line widths before char at corresponding index, in pixels, size = NumCharsInLine + 1
        UPInt           LineStartPos;           // [in] text position in paragraph of first char in line
        UPInt           NumCharsInLine;         // [in] count of chars currently in the line        
        float           VisibleRectWidth;       // [in] width of client rect, in pixels
        float           CurrentLineWidth;       // [in] current line width, in pixels
        float           LineWidthBeforeWordWrap;// [in] line width till ProposedWordWrapPoint, in pixels
        float           DashSymbolWidth;        // [in] may be used to calculate hyphenation, in pixels
        enum                                    
        {
            Align_Left      = 0,
            Align_Right     = 1,
            Align_Center    = 2,
            Align_Justify   = 3
        };
        UInt8           Alignment;              // [in] alignment of the line

        UPInt           ProposedWordWrapPoint;  // [in,out] index in the line of proposed word wrap pos,
                                                //          callback may change it to move wordwrap point
        bool            UseHyphenation;         // [out]    callback may set it to indicate to use hyphenation
    };
    // A virtual method, a callback, which is invoked once a necessity of 
    // word-wrapping for any text field is determined. This method is invoked only
    // if custom word-wrapping is turned on by using the GFx::Translator(wwMode) constructor. 
    virtual bool OnWordWrapping(LineFormatDesc* pdesc);

    // A callback that is called on bidirectional text enabled textfields. This method
    // receives original text ('text'/'textLen' parameters) and pre-allocated buffers: 
    //   'newText'  - new re-ordered text should be put there, do not overrun the length (the length is the same as 'textLen')
    //   'indexMap' - a buffer for an array of unsigned integers, that should be filled with new indices of each char. For
    //                example, if original char at index 0 was relocated to index 10, then indexMap[0] = 10.
    //   'mirrorBits'- a buffer for an array of bools (length = 'textLen'), where index is an index of char in 'newText' and the value (true/false)
    //                indicates necessity of the glyph mirroring (for example, for integral sign).
    // Should return 'true' if method was successful and the core should use contents of the buffers; false, if no changes
    // to the textfield should be applied.
    virtual bool OnBidirectionalText(const wchar_t* text, UPInt textLen, wchar_t* newText, 
                                     unsigned* indexMap, bool* mirrorBits);
};


// ***** ActionControl

// Controls verbosity and runtime behavior of ActionScript.
class ActionControl : public State
{
protected:
    unsigned        ActionFlags;

public:

    // ActionScript execution control flags.
    enum ActionControlFlags
    {
        Action_Verbose              = 0x01,
        Action_ErrorSuppress        = 0x02,
        Action_LogRootFilenames     = 0x04, //Display filename for root movie 
        Action_LogChildFilenames    = 0x08, //Display filename for child movie
        Action_LogAllFilenames      = 0x04 | 0x08, // Display filenames for all movies
        Action_LongFilenames        = 0x10  //Display full path
    };

    ActionControl(unsigned actionFlags = Action_LogChildFilenames)
        : State(State_ActionControl),  ActionFlags(actionFlags)
    { }       
    
    inline void     SetActionFlags(unsigned actionFlags)   { ActionFlags = actionFlags; }
    inline unsigned GetActionFlags() const             { return ActionFlags; }

    // Enables verbose output to log when executing ActionScript. Useful
    // for debugging, 0 by default. Not available if GFx is compiled
    // with the GFX_AS2_VERBOSE / GFX_AS3_VERBOSE options not defined for
    // ActionScript 2 and 3, respectively.
    inline void    SetVerboseAction(bool verboseAction)
    {
        ActionFlags = (ActionFlags & ~(Action_Verbose)) | (verboseAction ? Action_Verbose : 0);
    }
    // Turn off/on log for ActionScript errors..
    inline void    SetActionErrorSuppress(bool suppressErrors)
    {
        ActionFlags = (ActionFlags & ~(Action_ErrorSuppress)) | (suppressErrors ? Action_ErrorSuppress : 0);
    } 
    // Print filename with ActionScript messages
    inline void    SetLogRootFilenames(bool logRootFilenames)
    {
        ActionFlags = (ActionFlags & ~(Action_LogRootFilenames)) | (logRootFilenames ? Action_LogRootFilenames : 0);
    } 

    inline void    SetLogChildFilenames(bool logChildFilenames)
    {
        ActionFlags = (ActionFlags & ~(Action_LogChildFilenames)) | (logChildFilenames ? Action_LogChildFilenames : 0);
    } 

    inline void    SetLogAllFilenames(bool logAllFilenames)
    {
        ActionFlags = (ActionFlags & ~(Action_LogAllFilenames)) | (logAllFilenames ? Action_LogAllFilenames : 0);
    } 

    inline void    SetLongFilenames(bool longFilenames)
    {
        ActionFlags = (ActionFlags & ~(Action_LongFilenames)) | (longFilenames ? Action_LongFilenames : 0);
    } 

};



// ***** UserEventHandler

// User-insallable interface to handle events fired FROM the player.
// These events include requests to show/hide mouse, etc.

class UserEventHandler : public State
{
public:
    
    UserEventHandler() : State(State_UserEventHandler) { }

    virtual void HandleEvent(class Movie* pmovie, const class Event& event) = 0;
};

// ***** TestStream
// Handles saving and reproducing events (timers, random etc.)

class TestStream : public State
{
public:
    TestStream ()  : State(State_TestStream) {}
    virtual bool GetParameter (const char* parameter, String* value) = 0;
    virtual bool SetParameter (const char* parameter, const char* value) = 0;
    enum
    {
        Record,
        Play
    } TestStatus;
};


// ***** FSCommandHandler

// ActionScript embedded in a movie can use the built-in fscommand() method
// to send data back to the host application. This method can be used to
// inform the application about button presses, menu selections and other
// UI events. To receive this data developer need to register a handler
// with SetFSCommandCallback.

// If specified, this handler gets called when ActionScript executes
// the fscommand() statement. The handler receives the Movie*
// interface for the movie that the script is embedded in and two string
// arguments passed by the script to fscommand().


class FSCommandHandler : public State
{
public:
    FSCommandHandler() : State(State_FSCommandHandler) { }

    virtual void Callback(Movie* pmovie, const char* pcommand, const char* parg) = 0;
};


// ***** FileOpenerBase

// A callback interface that is used for opening files.
// Must return either an open file or 0 value for failure.
// The caller of the function is responsible for Releasing
// the File interface.

class FileOpenerBase : public State
{
public:

    FileOpenerBase();

    // Override to opens a file using user-defined function and/or File class.
    // The default implementation uses buffer-wrapped SysFile.
    // The 'purl' should be encoded as UTF-8 to support international file names.
    virtual File* OpenFile(const char* purl, 
                           int flags = FileConstants::Open_Read|FileConstants::Open_Buffered, 
                           int mode = FileConstants::Mode_ReadWrite) = 0;

    // Returns last modified date/time required for file change detection.
    // Can be implemented to return 0 if no change detection is desired.
    // The 'purl' should be encoded as UTF-8 to support international file names.
    virtual SInt64      GetFileModifyTime(const char* purl) = 0;

    // Open file with customizable log, by relying on OpenFile.
    // If not null, log will receive error messages on failure.
    // The 'purl' should be encoded as UTF-8 to support international file names.
    virtual File* OpenFileEx(const char* purl, Log *plog, 
                             int flags = FileConstants::Open_Read|FileConstants::Open_Buffered, 
                             int mode = FileConstants::Mode_ReadWrite) = 0;    
};

// ***** FileOpener
// Default implementation of FileOpenerBase interface.

class FileOpener : public FileOpenerBase
{
public:

    // Override to opens a file using user-defined function and/or File class.
    // The default implementation uses buffer-wrapped SysFile, but only
    // if SF_ENABLE_SYSFILE is defined.
    // The 'purl' should be encoded as UTF-8 to support international file names.
    virtual File* OpenFile(const char* purl, 
                           int flags = FileConstants::Open_Read|FileConstants::Open_Buffered, 
                           int mode = FileConstants::Mode_ReadWrite);

    // Returns last modified date/time required for file change detection.
    // Can be implemented to return 0 if no change detection is desired.
    // Default implementation checks file time if SF_ENABLE_SYSFILE is defined.
    // The 'purl' should be encoded as UTF-8 to support international file names.
    virtual SInt64 GetFileModifyTime(const char* purl);

    // Open file with customizable log, by relying on OpenFile.
    // If not null, log will receive error messages on failure.
    // The 'purl' should be encoded as UTF-8 to support international file names.
    virtual File* OpenFileEx(const char* purl, Log *plog, 
        int flags = FileConstants::Open_Read|FileConstants::Open_Buffered, 
        int mode = FileConstants::Mode_ReadWrite);
};



// ***** URLBuilder

// URLBuilder class is responsible for building a filename path
// used for loading objects. If can also be overridden if filename
// or extension substitution is desired.

class URLBuilder : public State
{
public:
     
    URLBuilder() : State(State_URLBuilder) { } 

    enum FileUse
    {
        File_Regular,
        File_Import,
        File_ImageImport,
        File_LoadMovie,
        File_LoadVars,
        File_LoadXML,
        File_LoadCSS,
        File_Sound
    };

    enum UrlMethod
    {
        Url_Method_None,
        Url_Method_Get,
        Url_Method_Post,
        Url_Method_Put,
        Url_Method_Delete,
    };

    struct LocationInfo
    {
        FileUse         Use;
        String         FileName;
        String         ParentPath;

        LocationInfo(FileUse use, const String& filename = "", const String& parentPath = "")
            : Use(use), FileName(filename), ParentPath(parentPath) { }
    };

    // Builds a new filename based on the provided filename and a parent path.
    // Uses IsPathAbsolute internally do determine whether the FileName is absolute.
    virtual void        BuildURL(String *ppath, const LocationInfo& loc);

    // Static helper function used to determine if the path is absolute.
    // Relative paths should be concatenated in TranslateFilename,
    // while absolute ones are used as-is.
    static bool  SF_CDECL IsPathAbsolute(const char *putf8str);
    // Modifies path to not include the filename, leaves trailing '/'.
    static bool  SF_CDECL ExtractFilePath(String *ppath);

    // Handle 'http://', 'file://' prefixes and the like.
    static bool  SF_CDECL IsProtocol(const String& path);
#ifdef SF_ENABLE_HTTP_LOADING
    static bool SF_CDECL SendURLRequest(Array<UByte>* bytes, const String& path, UrlMethod method = Url_Method_Get, const char* data = NULL, int dataLength = 0, const Array<String>* headers = NULL, const char* contentType = NULL);
#endif

    // Default implementation used by BuildURL.
    friend class LoadStates;    
    static void SF_CDECL DefaultBuildURL(String *ppath, const LocationInfo& loc); 
private:
#ifdef SF_ENABLE_HTTP_LOADING
    struct PutData
    {
        const char* Data;
        size_t DataLeft;
    };
    static size_t HttpWriteData(char* buffer, size_t size, size_t nmemb, void* userp);
    static size_t HttpReadData(void* ptr, size_t size, size_t nmemb, void* userp);
#endif 
};

// ***** ParseControl

class ParseControl : public State
{
protected:
    unsigned ParseFlags;
public:    

    // Verbose constants, determine what gets displayed during parsing
    enum VerboseParseConstants
    {
        VerboseParseNone        = 0x00,
        VerboseParse            = 0x01,
        VerboseParseAction      = 0x02,
        VerboseParseShape       = 0x10,
        VerboseParseMorphShape  = 0x20,
        VerboseParseAllShapes   = VerboseParseShape | VerboseParseMorphShape,
        VerboseParseAll         = VerboseParse | VerboseParseAction | VerboseParseAllShapes
    };

    ParseControl(unsigned parseFlags = VerboseParseNone)
        : State(State_ParseControl), ParseFlags(parseFlags)
    { }

    inline void     SetParseFlags(unsigned parseFlags)   { ParseFlags = parseFlags; }
    inline unsigned GetParseFlags() const            { return ParseFlags; }

    bool    IsVerboseParse() const              { return (ParseFlags & VerboseParse) != 0; }
    bool    IsVerboseParseShape() const         { return (ParseFlags & VerboseParseShape) != 0; }
    bool    IsVerboseParseMorphShape() const    { return (ParseFlags & VerboseParseMorphShape) != 0; }
    bool    IsVerboseParseAction() const        { return (ParseFlags & VerboseParseAction) != 0; }

};


// ***** GFxLoadProgressHandler

// This state is used to get a progress during the file loading
// If the state is set its methods get called when the certain 
// events happened during file loading.

class ProgressHandler : public State
{
public:    
    ProgressHandler() : State(State_ProgressHandler) { }

    struct Info
    {
        String   FileUrl;      // the name of the file which the information is related to
        unsigned BytesLoaded;  // the number of bytes that have been loaded
        unsigned TotalBytes;   // the total file size
        unsigned FrameLoading; // the frame number which is being loaded
        unsigned TotalFrames;  // the total number on frames in the file.

        Info(const String& fileUrl, unsigned bytesLoaded, unsigned totalBytes, 
             unsigned frameLoading, unsigned totalFrames)
            : FileUrl(fileUrl), BytesLoaded(bytesLoaded), TotalBytes(totalBytes), 
                                FrameLoading(frameLoading), TotalFrames(totalFrames) {}
    };
    // The method is called periodically called to indicate who many bytes have 
    // already been loaded and what frame is being loaded now.
    virtual void    ProgressUpdate(const Info& info) = 0;   

    struct TagInfo
    {
        String  FileUrl;       // the name of the file which the information is related to
        int     TagType;       // the tag type id
        int     TagOffset;     // the tag offset in the file
        int     TagLength;     // the tag length
        int     TagDataOffset; 

        TagInfo(const String& fileUrl, int tagType, int tagOffset, int tagLength, int tagDataOffset)
            : FileUrl(fileUrl), TagType(tagType), TagOffset(tagOffset), 
                                TagLength(tagLength), TagDataOffset(tagDataOffset) {}
    };
    // The method is called the a tag has just been load from the file.
    virtual void    LoadTagUpdate(const TagInfo& info, bool calledFromDefSprite) { SF_UNUSED2(info, calledFromDefSprite); }
};

// Forward Declarations
class FontResource;

// ***** FontPackParams

// FontPackParams contains the font rendering configuration options 
// necessary to generate glyph textures. By setting TextureConfig, users can
// control the size of glyphs and textures which are generated.
class FontPackParams : public State
{
public:
    struct TextureConfig
    {
        // The nominal size of the final anti-aliased glyphs stored in
        // the texture.  This parameter controls how large the very
        // largest glyphs will be in the texture; most glyphs will be
        // considerably smaller.  This is also the parameter that
        // controls the trade off between texture RAM usage and
        // sharpness of large text. Default is 48.
        int     NominalSize;
     
        // How much space to leave around the individual glyph image.
        // This should be at least 1.  The bigger it is, the smoother
        // the boundaries of minified text will be, but the more texture
        // space is wasted.
        int     PadPixels;
        
        // The dimensions of the textures that the glyphs get packed into.
        // Default size is 1024.
        int     TextureWidth, TextureHeight;

        TextureConfig() : 
            NominalSize(48), 
            PadPixels(3), 
            TextureWidth(1024), 
            TextureHeight(1024)
        {}
    };

public:
    FontPackParams() : 
        State(State_FontPackParams),
        SeparateTextures(false),
        GlyphCountLimit(0)
    {}

    // Size (in TWIPS) of the box that the glyph should stay within.
    // this *should* be 1024, but some glyphs in some fonts exceed it!
    static const float          GlyphBoundBox;

    // Obtain/modify variables that control how glyphs are packed in textures.
    void            GetTextureConfig(TextureConfig* pconfig) { *pconfig = PackTextureConfig; }
    // Set does some bound checking to it is implemented in the source file.
    void            SetTextureConfig(const TextureConfig& config);

    bool            GetUseSeparateTextures() const    { return SeparateTextures; }
    void            SetUseSeparateTextures(bool flag) { SeparateTextures = flag; }

    int             GetGlyphCountLimit() const        { return GlyphCountLimit; }
    void            SetGlyphCountLimit(int lim)       { GlyphCountLimit = lim;  }

    float           GetDrawGlyphScale(int nominalGlyphHeight);    

    // Return the pixel height of text, such that the
    // texture glyphs are sampled 1-to-1 texels-to-pixels.
    // I.E. the height of the glyph box, in texels.
    static float    SF_STDCALL GetTextureGlyphMaxHeight(const FontResource* f);

    // State type
    virtual StateType   GetStateType() const { return State_FontPackParams; }

private:
    // Variables that control how glyphs are packed in textures.
    TextureConfig   PackTextureConfig;

    // Use separate textures for each font. Default is False.
    bool            SeparateTextures;

    // The limit of the number of glyphs in a font. 0 means "no limit".
    // If the limit is exceeded the glyphs are not packed. In this 
    // case the dynamic cache may be used if enabled. Default is 0.
    int             GlyphCountLimit;
};

// ***** FontCompactorParams
//
// FontCompactorParams contains the font compactor configuration options 
// There are two parameters that can be set on the contractor of this class
// 1. NominalSize - The nominal glyph size that corresponds with the input 
//    coordinates. Small nominal size results in smaller total data size, 
//    but less accurate glyphs. For example, NominalSize=256 can typically 
//    save about 25% of memory, compared with NominalSize=1024, but really 
//    big glyphs, like 500 pixels may have visual inaccuracy. For typical game 
//    UI text NominalSize=256 is still good enough. In SWF it is typically 1024.
// 2. MergeContours - A Boolean flag that tells, whether or not the 
//    FontCompactor should merge the same contours and glyphs. When merging 
//    the data can be more compact and save 10-70% of memory, depending on fonts.
//    But if the font contains too many glyphs, the hash table may consume 
//    additional memory, 12 (32-bit) or 16 (64-bit) bytes per each unique path, 
//    plus 12 (32-bit) or 16 (64-bit) bytes per each unique glyph.
class FontCompactorParams : public State
{
public:
    FontCompactorParams(unsigned nominalSize = 256, bool mergeContours = true)
        : State(State_FontCompactorParams), NominalSize(nominalSize), MergeContours(mergeContours) {}

    ~FontCompactorParams() {}

    unsigned GetNominalSize() const { return NominalSize; }
    bool     NeedMergeContours() const { return MergeContours; }
private:
    unsigned NominalSize;
    bool     MergeContours;
};


// ***** FontProvider

// An external interface to handle device/system fonts

class FontProvider : public State
{
public:
    FontProvider(Render::FontProvider* fp = NULL) : State(State_FontProvider), pFontProvider(fp) {}
    virtual ~FontProvider() {}
    
    // fontFlags style as described by Font::FontFlags
    virtual Render::Font* CreateFont(const char* name, unsigned fontFlags) 
    {
        return pFontProvider->CreateFont(name, fontFlags);
    }

    //void EnumerateFonts interface...
    virtual void LoadFontNames(StringHash<String>& fontnames)
    {
        pFontProvider->LoadFontNames(fontnames);
    }

protected:
    Ptr<Render::FontProvider> pFontProvider;
};

// ***** FontMap

// Font map defines a set of font name substitutions that apply fonts
// looked up/create from FontLib and FontProvider. By installing a
// map you can change fonts used by SWF/GFX file to refer to alternative
// fonts. For example, if the file makes use of the "Helvetica" font while
// your system only provides "Arial", you can install it as a substitution
// by doing the following:
//    Ptr<FontMap> pfontMap = *new FontMap;
//    pfontMap->AddFontMap("Helvetica", "Arial");
//    loader.SetFontMap(Map);

class FontMap : public State
{
    class FontMapImpl *pImpl;
public:
    FontMap();
    virtual ~FontMap();

    enum MapFontFlags
    {
        // Default state,
        MFF_Original        = 0x0010,
        MFF_NoAutoFit       = 0x0020,

        // Other font flag combinations, indicate that a specific font type
        // will be looked up. The must match up with constants in Font.
        MFF_Normal          = 0x0000,
        MFF_Italic          = 0x0001,
        MFF_Bold            = 0x0002,
        MFF_BoldItalic      = MFF_Italic | MFF_Bold,

        MFF_FauxItalic      = 0x0004,
        MFF_FauxBold        = 0x0008,
        MFF_FauxBoldItalic  = MFF_FauxItalic | MFF_FauxBold
    };

    class MapEntry
    {
    public:
        String         Name;
        float           ScaleFactor;
        MapFontFlags    Flags;

        MapEntry():ScaleFactor(1.f) { Flags = MFF_Original; }
        MapEntry(const String& name, MapFontFlags flags, float scaleFactor = 1.0f) : 
            Name(name), ScaleFactor(scaleFactor), Flags(flags) { }
        MapEntry(const MapEntry &src) : Name(src.Name), ScaleFactor(src.ScaleFactor), Flags(src.Flags) { }        
        MapEntry& operator = (const MapEntry &src)
        { Name = src.Name; ScaleFactor = src.ScaleFactor; Flags = src.Flags; return *this; }

        // Updates font flags generating new ones based on local MapFontFlags.
        unsigned UpdateFontFlags(unsigned originalFlags)
        {
            return (Flags == MFF_Original) ? originalFlags :
                   ((originalFlags & ~MFF_BoldItalic) | (Flags & ~MFF_FauxBoldItalic));
        }
        unsigned GetOverridenFlags() const
        {
            return (((Flags & MFF_FauxBoldItalic) ? Flags >> 2 : 0) & MFF_BoldItalic) | (Flags & MFF_NoAutoFit);
        }
    };

    // Adds a font mapping; font names expressed in UTF8.
    // A function will fail if a mapping for this font already exists,
    // overwriting a map is not allowed.
    bool            MapFont(const char* pfontName, const char* pnewFontName,
                            MapFontFlags mff = MFF_Original, float scaleFactor = 1.0f);
    // Adds a font mapping; font names expressed in Unicode.
    bool            MapFont(const wchar_t* pfontName, const wchar_t* pnewFontName,
                            MapFontFlags mff = MFF_Original, float scaleFactor = 1.0f);
    
    // Obtains a font mapping, filling in MapEntry if succeeds.
    bool            GetFontMapping(MapEntry* pentry, const char *pfontName);
};

// ***** TextClipboard
// The default implementation of text clipboard for textfields. This
// implementation is platform independent. To make a platform specific
// implementation - inherit from this class and overload the OnTextStore
// virtual method. 'GetText' virtual method can be overridden too, in order
// to retrieve data from the system clipboard (by default, it just returns 
// internally stored data).

class Clipboard : public State
{
public:
    enum Formats // as in flash.desktop.ClipboardFormats
    {
        Format_None             = 0,
        Format_Text             = 0x1,
        Format_RichText         = 0x2,
        Format_Bitmap           = 0x4,
        Format_FileList         = 0x8,
        Format_FilePromise_List = 0x10,
        Format_Html             = 0x20,
        Format_URL              = 0x40
    };
protected:
    WStringBuffer       PlainText;
    Text::StyledText*   pStyledText;
protected:
    void SetPlainText(const wchar_t* ptext, UPInt len);
    void SetStyledText(class Text::StyledText*);
    void ReleaseStyledText();
public:
    Clipboard();
    virtual ~Clipboard();

    virtual UInt32 GetSupportedFormats() {return 0;} //Formats supported by Clipboard implementation
    virtual UInt32 GetAvailableFormats() {return 0;} //Formats currently available in the Clipboard
    virtual bool   HasFormat(Formats format) { SF_UNUSED(format); return false;}
    // ** plain text storing
    // SetText methods save plain text inside the clipboard. OnTextStore callback
    // will be called to save data in system clipboard.
    void SetText(const String&);
    void SetText(const wchar_t* ptext, UPInt len = SF_MAX_UPINT);
    // GetText returns a plain string stored in the clipboard. This method can be
    // overridden to retrieve data from system clipboard. PlainText member can be
    // used for the storage.
    virtual const WStringBuffer& GetText();

    // ** rich text storing
    // SetTextAndStyledText method saves both plain and rich text inside the clipboard. 
    // OnTextStore callback will be called to save data in system clipboard (but only
    // for plain text; however full source customers can use pStyledText member to 
    // retrieve full rich text information).
    void SetTextAndStyledText(const wchar_t* ptext, UPInt len, class Text::StyledText*);

    // This method returns rich text info stored in the clipboard.
    virtual class Text::StyledText* GetStyledText();

    // This method returns true if clipboard stores rich text and false if plain.
    // As a result, either SetTextAndStyledText or SetText methods will be used.
    virtual bool ContainsRichText() const { return pStyledText != NULL; }

    // A virtual callback that is called when GFx stores plain text data in
    // the clipboard. This callback maybe used to store the data in a system
    // clipboard.
    virtual void OnTextStore(const wchar_t* ptext, UPInt len) { SF_UNUSED2(ptext, len); }
    virtual void Clear() {}
    virtual void ClearData(Formats format) {SF_UNUSED(format);}
};

#ifdef GFX_ENABLE_TEXT_INPUT
// ***** TextKeyMap
// Text keyboard map that specifies functionality of each keystroke in a textfield.
// This state might be useful to re-define the default key layout on different
// platforms.

class TextKeyMap : public State
{
public:
    enum KeyAction
    {
        KeyAct_None,
        KeyAct_ToggleSelectionMode,
        KeyAct_EnterSelectionMode,
        KeyAct_LeaveSelectionMode,
        KeyAct_Left,
        KeyAct_Right,
        KeyAct_Up,
        KeyAct_Down,
        KeyAct_PageUp,
        KeyAct_PageDown,
        KeyAct_LineHome,
        KeyAct_LineEnd,
        KeyAct_PageHome,
        KeyAct_PageEnd,
        KeyAct_DocHome,
        KeyAct_DocEnd,
        KeyAct_Backspace,
        KeyAct_Delete,
        KeyAct_Return,
        KeyAct_Copy,
        KeyAct_Paste,
        KeyAct_Cut,
        KeyAct_SelectAll
    };
    enum KeyState
    {
        State_Down,
        State_Up
    };
    struct KeyMapEntry
    {
        KeyAction           Action;
        unsigned            KeyCode;
        unsigned            SpecKeysPressed;
        KeyState            mState;

        KeyMapEntry():KeyCode(SF_MAX_UINT) {}
        KeyMapEntry(KeyAction ka, unsigned kc, UInt8 specKeys = 0, KeyState ks = State_Down):
        Action(ka), KeyCode(kc), SpecKeysPressed(specKeys), mState(ks) {}
    };
    Array<KeyMapEntry>     Map;

    TextKeyMap();

    TextKeyMap* InitWindowsKeyMap();
    TextKeyMap* InitMacKeyMap();

    void AddKeyEntry(const KeyMapEntry&);

    const KeyMapEntry* FindFirstEntry(unsigned keyCode) const;
    const KeyMapEntry* FindNextEntry(const KeyMapEntry*) const;

    const KeyMapEntry* Find(unsigned keyCode, const KeyModifiers& specKeys, KeyState state) const;
};
#else
class TextKeyMap : public State
{
public: 
    TextKeyMap() { }

    TextKeyMap* InitWindowsKeyMap() { return this; }
    TextKeyMap* InitMacKeyMap()     { return this; }
};
#endif // SF_NO_TEXT_INPUT_SUPPORT

// ***** ImagePacker
// Pack images into large textures. This is intended to be used within GFxExport for producing
// .gfx files that reference packed textures. It can also be used in a player, but causes:
// - All movie loading will wait for completion
// - More memory will be used for images as the original image data from the SWF file remains

class ImagePacker;


// Used in the packer to substitute embedded images
class ImageSubstProvider : public RefCountBase<ImageSubstProvider, Stat_Default_Mem>
{
public:
    virtual Render::Image* CreateImage(const char* pname) = 0;
};

class ImagePackParamsBase : public State
{
public:
    enum SizeOptionType
    {
        PackSize_1,
        PackSize_4,             // width and height of textures expanded to multiple of 4
        PackSize_PowerOf2,
        PackSize_128
    };

    struct TextureConfig
    {
        int             TextureWidth, TextureHeight; // Max size of one texture. If a smaller image is produced it will follow SizeOptions
        SizeOptionType  SizeOptions;                 // If a renderer is set on the loader, renderer caps will be used instead.
        bool            Square;                      // Required for some hardware
        TextureConfig() : 
            TextureWidth(1024), 
            TextureHeight(1024),
            SizeOptions(PackSize_1),
            Square(false)
        {}
    };

public:
    ImagePackParamsBase() : 
      State(State_ImagePackerParams)
      {}

      // Obtain/modify variables that control how images are packed in textures.
      void            GetTextureConfig(TextureConfig* pconfig) const { *pconfig = PackTextureConfig; }
      void            SetTextureConfig(const TextureConfig& config);

      // State type
      virtual StateType   GetStateType() const { return State_ImagePackerParams; }

	  virtual ImagePacker*  Begin(ResourceId* pIdGen, ImageCreator* pic, ImageCreateInfo* pici) const = 0;

private:
    // Variables that control how glyphs are packed in textures.
    TextureConfig   PackTextureConfig;
};


class ImagePackParams : public ImagePackParamsBase
{
public:
	virtual ImagePacker*  Begin(ResourceId* pIdGen, ImageCreator* pic, ImageCreateInfo* pici) const;
};


// ***** ZlibSupportBase
// The purpose of this interface is to provide ZLIB decompresser to the loader

class Stream;
class ZlibSupportBase : public State
{
public:
    ZlibSupportBase() : State(State_ZlibSupport) {}

    virtual File*   CreateZlibFile(File* in) = 0;
    virtual void    InflateWrapper(Stream* pinStream, void* buffer, int BufferBytes) = 0;
};  

// ***** ZlibSupport
// Default implementation of ZlibSupportBase interface
// The instance of this class is set on the loader contractor
// If the application does not need ZLIB support the loader constructor
// should be called with the second parameter set to NULL 

class ZlibSupport : public ZlibSupportBase
{
public:
    virtual File*   CreateZlibFile(File* in);
    virtual void    InflateWrapper(Stream* pinStream, void* buffer, int BufferBytes);
};

// ***** ASSupport
// Interface for ActionScript support
class ASSupport : public State
{
public:
    ASSupport(StateType st) : State(st) {}

    virtual MovieImpl* CreateMovie(MemoryContext*) =0;
	virtual Ptr<GFx::ASIMEManager> CreateASIMEManager() = 0;
    virtual MemoryContext* CreateMemoryContext(const char* heapName, const MemoryParams& memParams, 
                                               bool debugHeap) = 0;

    // Creates a new instance of a character, ownership is passed to creator (character is
    // not automatically added to parent as a child; that is responsibility of the caller).
    // Default behavior is to create a GenericCharacter.
    //  - pbindingImpl argument provides the MovieDefImpl object with binding tables
    //    necessary for the character instance to function. Since in the new implementation
    //    loaded data is separated from instances, CharacterDef will not know what
    //    what MovieDefImpl it is associated with, so this data must be passed.
    //    This value is significant when creating GFxSprite objects taken from an import,
    //    since their MovieDefImpl is different from pparent->GetResourceMovieDef().
    virtual DisplayObjectBase* CreateCharacterInstance
        (MovieImpl* proot,
        const CharacterCreateInfo& ccinfo, 
        InteractiveObject* pparent, 
        ResourceId rid, 
        CharacterDef::CharacterDefType type = CharacterDef::Unknown) = 0;

    virtual void  DoActions() = 0;

    virtual UByte GetAVMVersion() const =0;

    virtual PlaceObject2Tag* AllocPlaceObject2Tag(LoadProcess*, UPInt dataSz, UInt8 swfVer) =0;
    virtual PlaceObject3Tag* AllocPlaceObject3Tag(LoadProcess*, UPInt dataSz) =0;
    virtual RemoveObjectTag* AllocRemoveObjectTag(LoadProcess*) =0;
    virtual RemoveObject2Tag* AllocRemoveObject2Tag(LoadProcess*) =0;

    virtual void DoActionLoader(LoadProcess* p, const TagInfo& tagInfo) =0;
    virtual void DoInitActionLoader(LoadProcess* p, const TagInfo& tagInfo) =0;

    virtual void ReadButtonActions
        (LoadProcess* p, ButtonDef* pbuttonDef, TagType tagType) =0;
    virtual void ReadButton2ActionConditions
        (LoadProcess* p, ButtonDef* pbuttonDef, TagType tagType) =0;
};


// ***** UrlNavigator
// Interface for url navigation. Intended to open URLs in current OS/environment from AS3
class UrlNavigator : public State
{
public:
    UrlNavigator() : State(State_UrlNavigator) {}

    virtual void NavigateToUrl(const String& url) = 0;
};


// ***** GFxStateBag

// GFxStateBag collects accessor methods for properties that can be
// inherited from GFxLoader to MovieDef, and then to Movie.
// These properties can be overwritten in any one of above mentioned instances.

class StateBag : public FileTypeConstants
{
protected:
    virtual StateBag* GetStateBagImpl() const { return 0; }

public:
    virtual ~StateBag () { }
    
    // General state access. We need to pass state type to SetState so that we know
    // which state is being cleared when null is passed.    
    virtual void        SetState(State::StateType state, State* pstate)
    {
        StateBag* p = GetStateBagImpl(); 
        SF_ASSERT((pstate == 0) ? 1 : (pstate->GetStateType() == state));
        if (p) p->SetState(state, pstate);
    }
    // We always need to return AddRef'ed state value from GetState to remain
    // thread safe, since otherwise another thread might overwrite the state
    // killing it prematurely and causing Get to return a bad pointer.
    virtual State*      GetStateAddRef(State::StateType state) const
    {
        StateBag* p = GetStateBagImpl();
        return p ? p->GetStateAddRef(state) : 0;
    }

    // Fills in a set of states with one call.
    // Initial array pstates must contain null pointers.
    virtual void        GetStatesAddRef(State** pstateList,
                                        const State::StateType *pstates, unsigned count) const
    {
        StateBag* p = GetStateBagImpl();
        if (p) p->GetStatesAddRef(pstateList, pstates, count);
    }

    // *** Inlines for convenient state access.
#ifdef GFX_ENABLE_SOUND
    inline void                 SetAudio(AudioBase* ptr);
    inline Ptr<AudioBase>       GetAudio() const;
    inline Sound::SoundRenderer* GetSoundRenderer() const;
#endif // SF_NO_SOUND

    // Sets an interface that will be used for logging - implemented in GFxLog.h.
    void                        SetLog(Log *plog);
    Ptr<Log>                    GetLog() const;
    void                        SetLogState(LogState *plog);
    Ptr<LogState>               GetLogState() const;

    // Set an interface that will be used to translate dynamic text field for international languages.
    inline void                 SetTranslator(Translator *ptr)  { SetState(State::State_Translator, ptr); }
    inline Ptr<Translator>      GetTranslator() const           { return *(Translator*)GetStateAddRef(State::State_Translator); }

    // Sets loadMovie interface used for images.
    inline void                 SetImageFileHandlerRegistry(ImageFileHandlerRegistry* pmil);
    inline Ptr<ImageFileHandlerRegistry> GetImageFileHandlerRegistry() const;

    // Set ActionScript control states.
    inline void                 SetActionControl(ActionControl* pac) { SetState(State::State_ActionControl, pac); }
    inline Ptr<ActionControl>   GetActionControl() const             { return *(ActionControl*)GetStateAddRef(State::State_ActionControl); }

    // UserEventHander - used to control events in 
    inline void                 SetUserEventHandler(UserEventHandler* pri) { SetState(State::State_UserEventHandler, pri); }
    inline Ptr<UserEventHandler> GetUserEventHandler() const            { return *(UserEventHandler*)GetStateAddRef(State::State_UserEventHandler); }

    // Sets loadMovie interface used for images.
    inline void                 SetFSCommandHandler(FSCommandHandler* ps){ SetState(State::State_FSCommandHandler, ps); }
    inline Ptr<FSCommandHandler> GetFSCommandHandler() const            { return *(FSCommandHandler*)GetStateAddRef(State::State_FSCommandHandler); }

    //Set TestStream for recording end reproducing events in Tester
    inline void                 SetTestStream(TestStream* pts)        { SetState(State::State_TestStream, pts); }
    inline Ptr<TestStream>      GetTestStream() const                 { return *(TestStream*)GetStateAddRef(State::State_TestStream); }

    // Sets the external interface used - implemented in GFxPlayer.h.
    inline void                 SetExternalInterface(ExternalInterface* p);
    inline Ptr<ExternalInterface> GetExternalInterface() const;

	// Sets the extension context interface used - implemented in GFxPlayer.h.
    inline void					SetExtensionContextInterface(ExtensionContextInterface* p);
    inline Ptr<ExtensionContextInterface> GetExtensionContextInterface() const;

    // Sets the multitouch interface used - implemented in GFxPlayer.h.
    inline void                 SetMultitouchInterface(MultitouchInterface* p);
    inline Ptr<MultitouchInterface> GetMultitouchInterface() const;

    // Sets the virtual keyboard interface used - implemented in GFxPlayer.h.
    inline void                 SetVirtualKeyboardInterface(VirtualKeyboardInterface* p);
    inline Ptr<VirtualKeyboardInterface> GetVirtualKeyboardInterface() const;

    // Sets the accelerometer interface used - implemented in GFxPlayer.h.
    inline void                 SetAccelerometerInterface(AccelerometerInterface* p);
    inline Ptr<AccelerometerInterface> GetAccelerometerInterface() const;

	// Sets the geolocation interface used - implemented in GFxPlayer.h.
    inline void                 SetGeolocationInterface(GeolocationInterface* p);
    inline Ptr<GeolocationInterface> GetGeolocationInterface() const;

    // Installs a callback function that is always used by GFxLoader
    // for opening various files based on a path or url string.
    // If not specified, the system file input will be used
    inline  void                SetFileOpener(FileOpenerBase *ptr) { SetState(State::State_FileOpener, ptr); }
    inline  Ptr<FileOpenerBase> GetFileOpener() const              { return *(FileOpenerBase*)GetStateAddRef(State::State_FileOpener); }

    // Open file with customizable log. If not null, log will receive error messages on failure.
    File*                       OpenFileEx(const char *pfilename, Log *plog);
    // Opens a file using the specified callback, same as above but uses local log.
    File*                       OpenFile(const char *pfilename);
    
    // File name translation.
    inline void                 SetURLBuilder(URLBuilder *ptr)     { SetState(State::State_URLBuilder, ptr); }
    inline Ptr<URLBuilder>      GetURLBuilder() const              { return *(URLBuilder*)GetStateAddRef(State::State_URLBuilder); }

    
    // Sets the callback function that will be used to create/initalize images
    // during loading. If no function is set, Render::ImageInfo will be used.
    inline void                 SetImageCreator(ImageCreator *ptr);
    inline Ptr<ImageCreator>    GetImageCreator() const;

    // Sets the callback function that will be used to create/initalize images
    // during loading. If no function is set, Render::ImageInfo will be used.
    inline void                 SetParseControl(ParseControl *ptr) { SetState(State::State_ParseControl, ptr); }
    inline Ptr<ParseControl> GetParseControl() const               { return *(ParseControl*)GetStateAddRef(State::State_ParseControl); }

    inline void                 SetProgressHandler(ProgressHandler *ptr) { SetState(State::State_ProgressHandler, ptr); }
    inline Ptr<ProgressHandler> GetProgressHandler() const              { return *(ProgressHandler*)GetStateAddRef(State::State_ProgressHandler); }

    // Sets a default visitor that is used when after files are loaded.
    inline void                 SetImportVisitor(ImportVisitor *ptr);
    inline Ptr<ImportVisitor>   GetImportVisitor() const;

    // GFxStateBag access for FontFontPackParams
    inline void                 SetFontPackParams(FontPackParams *ptr)   { SetState(State::State_FontPackParams, ptr); }
    inline Ptr<FontPackParams>  GetFontPackParams() const                { return *(FontPackParams*) GetStateAddRef(State::State_FontPackParams); }

    inline void                 SetFontLib(FontLib* pfl);
    inline Ptr<FontLib>         GetFontLib() const;

    void                        SetDefaultFontLibName(const char* defaultFontLib);
    const char*                 GetDefaultFontLibName() const;

    inline void                 SetFontProvider(FontProvider *ptr)       { SetState(State::State_FontProvider, ptr); }
    inline Ptr<FontProvider>    GetFontProvider() const                    { return *(FontProvider*) GetStateAddRef(State::State_FontProvider); }

    inline void                 SetFontMap(FontMap *ptr)                 { SetState(State::State_FontMap, ptr); }
    inline Ptr<FontMap>         GetFontMap() const                          { return *(FontMap*) GetStateAddRef(State::State_FontMap); }

    //@TODO: temporarily disabled, will be re-enabled in later releases.
     inline void                 SetImagePackParams(ImagePackParams *ptr) { SetState(State::State_ImagePackerParams, ptr); }
     inline Ptr<ImagePackParams> GetImagePackerParams() const          { return *(ImagePackParams*) GetStateAddRef(State::State_ImagePackerParams); }


    // Sets a task manager - used to issue I/O and general processing tasks on other threads.
    inline void                 SetTaskManager(TaskManager *ptr);
    inline Ptr<TaskManager>     GetTaskManager() const;

    inline void                 SetClipboard(Clipboard *ptr) { SetState(State::State_Clipboard, ptr); }
    inline Ptr<Clipboard>       GetClipboard() const             { return *(Clipboard*) GetStateAddRef(State::State_Clipboard); }

    inline void                 SetTextKeyMap(TextKeyMap *ptr)       { SetState(State::State_TextKeyMap, ptr); }
    inline Ptr<TextKeyMap>      GetTextKeyMap() const                { return *(TextKeyMap*) GetStateAddRef(State::State_TextKeyMap); }

    inline void                 SetIMEManager(IMEManagerBase *ptr);
    inline Ptr<IMEManagerBase>  GetIMEManager() const;

#ifdef GFX_ENABLE_XML
    inline void                  SetXMLSupport(XML::SupportBase *ptr);
    inline Ptr<XML::SupportBase> GetXMLSupport() const;
#endif

    inline void                 SetZlibSupport(ZlibSupportBase *ptr) { SetState(State::State_ZlibSupport, ptr); }
    inline Ptr<ZlibSupportBase> GetZlibSupport() const              { return *(ZlibSupportBase*) GetStateAddRef(State::State_ZlibSupport); }

    inline void                 SetFontCompactorParams(FontCompactorParams *ptr) { SetState(State::State_FontCompactorParams, ptr); }
    inline Ptr<FontCompactorParams> GetFontCompactorParams() const  { return *(FontCompactorParams*) GetStateAddRef(State::State_FontCompactorParams); }

#ifdef GFX_ENABLE_VIDEO
    // defined in VideoBase.h
    inline void                 SetVideo(Video::VideoBase* ptr);
    inline Ptr<Video::VideoBase> GetVideo() const;
#endif
#ifdef GFX_AS2_ENABLE_SHAREDOBJECT
    inline void                 SetSharedObjectManager(SharedObjectManagerBase *ptr);
    inline Ptr<SharedObjectManagerBase> GetSharedObjectManager() const;
#endif

    inline void                 SetUrlNavigator(UrlNavigator *ptr)  { SetState(State::State_UrlNavigator, ptr); }
    inline Ptr<UrlNavigator>    GetUrlNavigator() const             { return *(UrlNavigator*) GetStateAddRef(State::State_UrlNavigator); }

    inline void                 SetAS2Support(ASSupport *ptr) { SetState(State::State_AS2Support, ptr); }
    inline Ptr<ASSupport>       GetAS2Support() const         { return *(ASSupport*) GetStateAddRef(State::State_AS2Support); }

    inline void                 SetAS3Support(ASSupport *ptr) { SetState(State::State_AS3Support, ptr); }
    inline Ptr<ASSupport>       GetAS3Support() const         { return *(ASSupport*) GetStateAddRef(State::State_AS3Support); }
}; 

// ***** GFxLoader

//  This is the main public interface that loads SWF Files,
//  and maintains the loader state.

struct MovieInfo
{
    // Flag definitions for Flags field.
    enum SWFFlagConstants
    {
        SWF_Compressed  = 0x0001,
        SWF_Stripped    = 0x0010
    };

    unsigned    Version;
    unsigned    Flags;
    int         Width, Height;
    float       FPS;
    unsigned    FrameCount;
    unsigned    TagCount;   

    // Extra exporter info
    UInt16      ExporterVersion; // Example: Version 1.10 will be encoded as 0x10A.
    UInt32      ExporterFlags;   // Same content as ExporterInfo::Flags.

    void    Clear()
    {
        Version = 0;
        Flags   = 0;
        Width = Height = 0;
        FrameCount = TagCount = 0;
        FPS = 0.0f;   
        ExporterVersion = 0;
        ExporterFlags   = 0;
    }

    bool    IsStripped() const      { return (Flags & SWF_Stripped) != 0;  }
    bool    IsCompressed() const    { return (Flags & SWF_Compressed) != 0;  }
};


// Information provided to the stripper tool when converting '.swf' file to '.gfx'.

struct ExporterInfo
{
    FileTypeConstants::FileFormatType Format;    // Format used for images.
    const char*     pPrefix;                    // Prefix assigned to extracted image files.
    const char*     pSWFName;                   // Original SWF file name from which '.gfx' was created.
    UInt16          Version;                    // Export tool version.

    // Export flag bits.
    enum ExportFlagConstants
    {
        EXF_GlyphTexturesExported       = 0x01,
        EXF_GradientTexturesExported    = 0x02,
        EXF_GlyphsStripped              = 0x10
    };
    unsigned        ExportFlags;                // Export flags described by ExportFlagConstants.
};


// GFxLoader provides the following functionality:
//   - collection of states describing loading / execution characteristics
//   - a resource library referenced during loading
//   - a task manager used for threaded loading, if any
//   - An API used to load movies based on the local states

class Loader : public StateBag
{
    // Hidden loader implementation and context pointer.
    class LoaderImpl*   pImpl;
    ResourceLib*        pStrongResourceLib;
    unsigned            DefLoadFlags;
protected:
    virtual StateBag* GetStateBagImpl() const;
    virtual bool CheckTagLoader(int tagType) const;
public:

    // Constant flags that affect loading implementation. These are most typically
    // passed to CreateMovie, but can also be specified as a part of GFxLoader
    // constructor (in which cases, the specified flags are automatically
    // applied with 'or' operator to every CreateMovie call).

    enum LoadConstants
    {
        // *** General Loading Flags.

        // Default.
        LoadAll             = 0x00000000,

        // Wait until all of the loading has completed before returning MovieDef.
        LoadWaitCompletion  = 0x00000001,
        LoadWaitFrame1      = 0x00000002,

        // Specified ordered binding, where resources are loaded after main data file.
        // If not specified, resource loading will be interleaved with resource and import binding.
        LoadOrdered         = 0x00000010,

        // This flag should be set if CreateMovie is called not on the main thread
        // We need this because in this case we need to postpone textures creation 
        // until the renderer needs them on the main thread
        LoadOnThread        = 0x00000040,

        // Set to preserve Creator bind data.
        // If set image/resource data used for creation will be kept so that
        // it can be re-bound with a different creator instance if necessary.
        // Takes precedence over ImageLoader::IsKeepingImageData
        LoadKeepBindData    = 0x00000080,

        // Set this flag to allow images to be loaded into root MovieDef;
        // file formats will be detected automatically.
        LoadImageFiles      = 0x00010000,
        // Set to disable loading of SWF files; only GFX content is allowed
        LoadDisableSWF      = 0x00080000,

        // Disable loading of imports. Files loaded without imports may not
        // be functional. This is primarily used for 'gfxexport' tool.
        LoadDisableImports  = 0x00100000,

        // Disable logging error message if requested file is not found
        LoadQuietOpen       = 0x00200000,


        // *** GFxLoader  Constructor only

        // Marks all loaded/created heaps with UserDebug flag, so that they can
        // be subtracted from the memory reports.
        LoadDebugHeap       = 0x10000000
    };


// Define optional library constructor macros. These are defined conditionally
// to force library linking only when used.
#ifdef SF_ENABLE_ZLIB
    #define GFX_LOADER_NEW_ZLIBSUPPORT *new ZlibSupport    
#else
    #define GFX_LOADER_NEW_ZLIBSUPPORT ((ZlibSupport*)0)
#endif

    // A structure wrapping together different loader parameters into one,
    // including default loading flags and optional link-dependent states.
    // If certain states, such as ZlibSupport are not specified here,
    // they are similarly not linked into the application.    
    struct LoaderConfig
    {
        unsigned                 DefLoadFlags;
        Ptr<FileOpenerBase>  pFileOpener;
        Ptr<ZlibSupportBase> pZLibSupport;
        
        LoaderConfig(unsigned loadFlags = 0,
                     const Ptr<FileOpenerBase>& pfileOpener = *new FileOpener,
                     const Ptr<ZlibSupportBase>& pzlib = GFX_LOADER_NEW_ZLIBSUPPORT
                     )
            : DefLoadFlags(loadFlags), 
              pFileOpener(pfileOpener), pZLibSupport(pzlib)
        { }
        LoaderConfig(const LoaderConfig& src)
            : DefLoadFlags(src.DefLoadFlags), pFileOpener(src.pFileOpener),
            pZLibSupport(src.pZLibSupport)
        { }
    };


    // GFxLoader Constructors.
    // Default initializers create a number of optional libraries which can
    // be excluded from build if set to 0 instead.
    Loader(const LoaderConfig& config);

    Loader(const Ptr<FileOpenerBase>& pfileOpener = *new FileOpener,
              const Ptr<ZlibSupportBase>& zlib = GFX_LOADER_NEW_ZLIBSUPPORT
              );

    // Create a new loader, copying it's library and states.
    Loader(const Loader& src);

    SF_EXPORT virtual ~Loader();

    
    // *** Task Manager

    // Specify task manager used for threaded loading, if any.


    // *** Library management

    // ResourceLib is a single heterogeneous library, containing image loaded movies,
    // images, gradients and other resources identified by a key. A default library is
    // created by GFxLoader constructor; however, users can change it and/or force library 
    // sharing across loaders by using these methods.
    void            SetResourceLib(ResourceLib *plib);
    ResourceLib*    GetResourceLib() const;


    // *** SWF File Loading

    // Obtains information about SWF file and checks for its availability.
    // Return 1 if the info was obtained successfully (or was null, but SWF file existed),
    // or 0 if it did not exist. 
    // The 'pfilename' should be encoded as UTF-8 to support international file names.
    bool GetMovieInfo(const char* pfilename, MovieInfo *pinfo, bool getTagCount = 0, 
                      unsigned loadConstants = LoadAll);

    // Loads a movie definition, user must release the movie definition.
    // If LoadImports, LoadBitmaps or LoadFontShapes was not specified,
    // user may need to resolve those components manually.
    // The 'pfilename' should be encoded as UTF-8 to support international file names.
    MovieDef* CreateMovie(const char* pfilename, unsigned loadConstants = LoadAll, UPInt memoryArena = 0); 
    

    // Unpins all resources held in the library.
    void            UnpinAll();

    // Cancel all loadings
    void            CancelLoading(); 

    class LoaderImpl* GetLoaderImpl() const { return pImpl; }
private:
     void           InitLoader(const LoaderConfig& config);
};


// ***** System Initialization class

// System initialization must take place before any other GFx objects, such as the loader
// are created; this is done my calling System::Init(). Similarly, System::Destroy must be
// called before program exist for proper clenup. Both of these tasks can be achieved by
// simply creating System object first, allowing its constructor/destructor do the work.
// Note that if an instance of System is created, Init/Destroy calls should not be made
// explicitly.

// One of the main things System initialization does is establish the GFx memory heap
// to use for allocations. Before that is done, allocations will fail and/or generate errors.

class System : public Scaleform::System
{
public:

    // System has two version of constructors, so that you can specify:
    //  - No arguments; uses default sys alloc and root heap.
    //  - Custom SysAlloc implementation with default root heap settings.
    //  - Custom root heap settings with default SysAlloc implementation.
    //  - Both custom root heap settings and custom SysAlloc implementation.
    System(SysAllocBase* psysAlloc = SF_SYSALLOC_DEFAULT_CLASS::InitSystemSingleton());
    System(const MemoryHeap::HeapDesc& rootHeapDesc,
           SysAllocBase* psysAlloc = SF_SYSALLOC_DEFAULT_CLASS::InitSystemSingleton());
    ~System();


    // Initializes System core, setting the global heap that is needed for GFx
    // memory allocations. Users can override memory heap implementation by passing
    // a different memory heap here.   
    SF_EXPORT static void SF_CDECL Init(const MemoryHeap::HeapDesc& rootHeapDesc,
        SysAllocBase* psysAlloc = SF_SYSALLOC_DEFAULT_CLASS::InitSystemSingleton());

    SF_EXPORT static void SF_CDECL Init(
        SysAllocBase* psysAlloc = SF_SYSALLOC_DEFAULT_CLASS::InitSystemSingleton())
    {
        Init(MemoryHeap::RootHeapDesc(), psysAlloc);
    }

    SF_EXPORT static void SF_CDECL Destroy();
};


}} // Scaleform::GFx


#ifdef SF_BUILD_DEBUG
extern "C" int GFx_Compile_without_SF_BUILD_DEBUG;
#else
extern "C" int GFx_Compile_with_SF_BUILD_DEBUG;
#endif

SF_INLINE Scaleform::GFx::FileOpenerBase::FileOpenerBase() : State(State_FileOpener)
{
#ifdef SF_BUILD_DEBUG
    GFx_Compile_without_SF_BUILD_DEBUG = 0;
#else
    GFx_Compile_with_SF_BUILD_DEBUG = 0;
#endif
}


#endif
