/**************************************************************************

PublicHeader:   AMP
Filename    :   Amp_Message.h
Content     :   Messages sent back and forth to AMP
Created     :   December 2009
Authors     :   Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

// This file includes a number of different classes
// All of them derive from GFxMessage and represent the types of messages
// sent back and forth between AMP and the GFx application

#ifndef INCLUDE_GFX_AMP_MESSAGE_H
#define INCLUDE_GFX_AMP_MESSAGE_H

#include "GFxConfig.h"
#ifdef SF_ENABLE_STATS

#include "Amp_ProfileFrame.h"
#include "Kernel/SF_AmpInterface.h"

namespace Scaleform {
namespace GFx {
namespace AMP {

class AmpStream;
class MsgHandler;

// Generic message that includes only the type
// Although not abstract, this class is intended as a base class for concrete messages
class Message : public RefCountBase<Message, StatAmp_Message>, public ListNode<Message>
{
public:
    // For backwards compatibility, add new platforms to the end of this list
    // and don't delete any obsolete platforms
    enum PlatformType
    {
        PlatformOther = 0,

        PlatformWindows,
        PlatformMac,
        PlatformLinux,

        PlatformXbox360,
        PlatformPs3,
        PlatformWii,
        Platform3DS,

        PlatformAndroid,
        PlatformIphone,

        PlatformNgp,
        PlatformWiiU
    };

    virtual ~Message() { }

    // Message serialization methods
    virtual void            Read(File& str);
    virtual void            Write(File& str) const;

    // Type name for message reconstruction after serialization
    virtual String          GetMessageName() const = 0;

    // version for backwards-compatibility
    void        SetVersion(UInt32 version)                  { Version = version; }
    UInt32      GetVersion() const                          { return Version; }
    UByte       GetGFxVersion() const                       { return GFxVersion; }

    // Compression
    virtual bool    Compress(Array<UByte>& compressedData) const;
    virtual bool    Uncompress(Array<UByte>& uncompressedData) { SF_UNUSED(uncompressedData); return false; }

    // IP address information
    virtual void    SetPeerName(const char* name) { SF_UNUSED(name); }
    virtual void    SetPeerAddress(UInt32 address) { SF_UNUSED(address); }

    // Version for backwards compatibility
    static UInt32       GetLatestVersion()          { return Version_Latest; }

    // Helper read/write methods
    static void         ReadString(File& inFile, String* str);
    static void         WriteString(File& outFile, const String& str);
    static String       MsgTypeToMsgName(UInt32 msgType); 
    static PlatformType GetLocalPlatform();

protected:

    // enum used only for compatibility with older versions
    enum MessageType
    {
        Msg_None,
        Msg_Heartbeat,

        // Server to client only
        Msg_Log,
        Msg_CurrentState,
        Msg_ProfileFrame,
        Msg_SwdFile,
        Msg_SourceFile,

        // Client to server only
        Msg_SwdRequest,
        Msg_SourceRequest,
        Msg_AppControl,

        // Broadcast
        Msg_Port,

        // Image
        Msg_ImageRequest,
        Msg_ImageData,
        Msg_FontRequest,
        Msg_FontData,

        Msg_Compressed,
        Msg_InitState,
        Msg_ObjectsReportRequest,
        Msg_ObjectsReport,

        Num_Messages
    };

    enum VersionType
    {
        Version_Latest = 42
    };

    UInt32                      Version;
    UByte                       GFxVersion;

    // Protected constructor because Message is intended to be used as a base class only
    Message();

    virtual MessageType     GetMessageType() const { return Msg_None; } 
};


// Heartbeat message
// Used for connection verification
// A lack of received messages for a long time period signifies lost connection
// Not handled by the message handler, but at a lower level by the thread manager
class MessageHeartbeat : public Message
{
public:
    static String           GetStaticTypeName() { return "Heartbeat"; }

    virtual String          GetMessageName() const        { return GetStaticTypeName(); }

protected:
    // enum used only for compatibility with older versions
    virtual MessageType     GetMessageType() const { return Msg_Heartbeat; } 
};

// InitState message
// Used to start AMP server in the specified state
class MessageInitState : public Message
{
public:
    static String           GetStaticTypeName() { return "InitState"; }
    MessageInitState(UInt32 initStateFlags = 0, UInt32 initProfileLevel = 0);

    // Message overrides
    virtual void            Read(File& str);
    virtual void            Write(File& str) const;
    virtual String          GetMessageName() const        { return GetStaticTypeName(); }

    // Accessors
    UInt32                  GetInitStateFlags() const;
    UInt32                  GetInitProfileLevel() const;

protected:
    UInt32                  InitStateFlags;
    UInt32                  InitProfileLevel;

    // enum used only for compatibility with older versions
    virtual MessageType     GetMessageType() const { return Msg_InitState; } 
};

// Log message
// Sent from server to client
class MessageLog : public Message
{
public:
    static String           GetStaticTypeName()         { return "Log"; }
    MessageLog(const String& logText = "", UInt32 logCategory = 0, const UInt64 timeStamp = 0);

    // Message overrides
    virtual void            Read(File& str);
    virtual void            Write(File& str) const;
    virtual String          GetMessageName() const        { return GetStaticTypeName(); }

    // Accessors
    const StringLH&         GetLogText() const;
    UInt32                  GetLogCategory() const;
    const StringLH&         GetTimeStamp() const;
    void                    SetLog(const String& logText, UInt32 logCategory, const UInt64 timeStamp);

protected:
    StringLH                LogText;
    UInt32                  LogCategory;
    StringLH                TimeStamp;

    // enum used only for compatibility with older versions
    virtual MessageType     GetMessageType() const { return Msg_Log; } 
};

// Message describing AMP current state
// Sent from server to client
// The state is stored in the base class member
class MessageCurrentState : public Message
{
public:
    static String           GetStaticTypeName()        { return "CurrentState"; }
    MessageCurrentState(const ServerState* state = NULL);

    // Message overrides
    virtual void            Read(File& str);
    virtual void            Write(File& str) const;
    virtual bool            ShouldQueue() const     { return false; }
    virtual String          GetMessageName() const        { return GetStaticTypeName(); }

    // Accessors
    const ServerState*      GetCurrentState() const   { return State; }

protected:
    Ptr<ServerState>    State;

    // enum used only for compatibility with older versions
    virtual MessageType     GetMessageType() const { return Msg_CurrentState; } 
};

// Message describing a single frame's profiling results
// Sent by server to client every frame
class MessageProfileFrame : public Message
{
public:
    static String           GetStaticTypeName()        { return "ProfileFrame"; }
    MessageProfileFrame(Ptr<ProfileFrame> frameInfo = NULL);

    // Message overrides
    virtual String          GetMessageName() const        { return GetStaticTypeName(); }
    virtual void            Read(File& str);
    virtual void            Write(File& str) const;

    // Data Accessor
    const ProfileFrame*     GetFrameInfo() const;

protected:

    Ptr<ProfileFrame>       FrameInfo;

    // enum used only for compatibility with older versions
    virtual MessageType     GetMessageType() const { return Msg_ProfileFrame; } 
};


// Message with the contents of a SWD file
// The contents are not parsed but are sent in the form of an array of bytes
// Sent by server to client upon request
// The base class member holds the corresponding SWF file handle, generated by the server
class MessageSwdFile : public Message
{
public:
    static String           GetStaticTypeName()        { return "SwdFile"; }
    MessageSwdFile(UInt32 swfHandle = 0, UByte* bufferData = NULL, unsigned bufferSize = 0, 
                                                                   const char* filename = "");

    // Message overrides
    virtual String          GetMessageName() const        { return GetStaticTypeName(); }
    virtual void            Read(File& str);
    virtual void            Write(File& str) const;

    // Accessors
    UInt32                  GetHandle() const;
    const ArrayLH<UByte>&   GetFileData() const;
    const char*             GetFilename() const;

protected:
    UInt32                  Handle;
    ArrayLH<UByte>          FileData;
    StringLH                Filename;

    // enum used only for compatibility with older versions
    virtual MessageType     GetMessageType() const { return Msg_SwdFile; } 
};

// Message containing ActionScript source code
// Sent by server to client upon request
class MessageSourceFile : public Message
{
public:
    static String           GetStaticTypeName()        { return "SourceFile"; }
    MessageSourceFile(UInt64 fileHandle = 0, UByte* bufferData = NULL, 
                      unsigned bufferSize = 0, const char* filename = "");

    // Message overrides
    virtual String          GetMessageName() const        { return GetStaticTypeName(); }
    virtual void            Read(File& str);
    virtual void            Write(File& str) const;

    // Accessors
    UInt64                  GetFileHandle() const;
    const ArrayLH<UByte>&   GetFileData() const;
    const char*             GetFilename() const;

protected:

    UInt64                  FileHandle;
    ArrayLH<UByte>          FileData;
    StringLH                Filename;

    // enum used only for compatibility with older versions
    virtual MessageType     GetMessageType() const { return Msg_SourceFile; } 
};

// Message containing Objects report
// Sent by server to client upon request
class MessageObjectsReport : public Message
{
public:
    static String           GetStaticTypeName()        { return "ObjectsReport"; }
    MessageObjectsReport(const char* objectsReport = NULL, Ptr<AmpMovieObjectDesc> root = NULL, UInt32 movieHandle = 0);

    // Message overrides
    virtual String          GetMessageName() const        { return GetStaticTypeName(); }
    virtual void            Read(File& str);
    virtual void            Write(File& str) const;

    // Accessors
    const char*                                 GetObjectsReport() const;
    const Ptr<AmpMovieObjectDesc>               GetObjectsRoot() const;
    UInt32                                      GetMovieHandle() const;
protected:

    StringLH                            ObjectsReport;
    Ptr<AmpMovieObjectDesc>             ObjectsRoot;
    UInt32                              MovieHandle;


    // enum used only for compatibility with older versions
    virtual MessageType     GetMessageType() const { return Msg_ObjectsReport; } 
};



// Sent by client to server
// Base class holds the SWF handle
class MessageSwdRequest : public Message
{
public:
    static String           GetStaticTypeName()        { return "SwdRequest"; }
    MessageSwdRequest(UInt32 swfHandle = 0, bool requestContents = false);

    // Message overrides
    virtual String          GetMessageName() const        { return GetStaticTypeName(); }
    virtual void            Read(File& str);
    virtual void            Write(File& str) const;

    // Accessors
    UInt32                  GetHandle() const;
    bool                    IsRequestContents() const;

protected:
    UInt32                  Handle;
    // may just request filename for a given SWF handle, or the entire SWD file
    bool                    RequestContents; 

    // enum used only for compatibility with older versions
    virtual MessageType     GetMessageType() const { return Msg_SwdRequest; } 
};

// Requests a source file corresponding to a given handle
// Sent by client to server
class MessageSourceRequest : public Message
{
public:
    static String           GetStaticTypeName()        { return "SourceRequest"; }
    MessageSourceRequest(UInt64 handle = 0, bool requestContents = false);

    // Message overrides
    virtual String          GetMessageName() const        { return GetStaticTypeName(); }
    virtual void            Read(File& str);
    virtual void            Write(File& str) const;

    // Accessors
    UInt64                  GetFileHandle() const;
    bool                    IsRequestContents() const;

protected:

    UInt64                  FileHandle;
    bool                    RequestContents; 

    // enum used only for compatibility with older versions
    virtual MessageType     GetMessageType() const { return Msg_SourceRequest; } 
};

// Requests an objects report
// Sent by client to server
class MessageObjectsReportRequest : public Message
{
public:
    static String           GetStaticTypeName()        { return "ObjectsReportRequest"; }
    MessageObjectsReportRequest();
    MessageObjectsReportRequest(UInt32 movieHandle, bool shortFilenames, bool noCircularReferences, bool suppressOverallStats, bool addressesForAnonymObjsOnly, bool suppressMovieDefsStats, bool noEllipsis);

    // Message overrides
    virtual String          GetMessageName() const        { return GetStaticTypeName(); }
    virtual void            Read(File& str);
    virtual void            Write(File& str) const;

    // Accessors
    UInt32                  GetMovieHandle() const;
    bool                    IsShortFilenames() const;
    bool                    IsNoCircularReferences() const;
    bool                    IsSuppressOverallStats() const;
    bool                    IsAddressesForAnonymObjsOnly() const;
    bool                    IsSuppressMovieDefsStats() const;
    bool                    IsNoEllipsis() const;

protected:

    UInt32                  MovieHandle;
    bool                    ShortFilenames; 
    bool                    NoCircularReferences; 
    bool                    SuppressOverallStats; 
    bool                    AddressesForAnonymObjsOnly; 
    bool                    SuppressMovieDefsStats; 
    bool                    NoEllipsis; 

    // enum used only for compatibility with older versions
    virtual MessageType     GetMessageType() const { return Msg_ObjectsReportRequest; } 
};


// Message that controls the GFx application in some way
// Normally sent by client to server
// Server sends this to client to signify supported capabilities
// Control information stored as bits in the base class member
class MessageAppControl : public Message
{
public:
    static String           GetStaticTypeName()        { return "AppControl"; }
    MessageAppControl(UInt32 flags = 0);

    // Message overrides
    virtual String          GetMessageName() const        { return GetStaticTypeName(); }
    virtual void            Read(File& str);
    virtual void            Write(File& str) const;

    // Accessors
    UInt32                  GetFlags() const;
    bool                    IsToggleWireframe() const;
    void                    SetToggleWireframe(bool wireframeToggle);
    bool                    IsTogglePause() const;
    void                    SetTogglePause(bool pauseToggle);
    bool                    IsToggleAmpRecording() const;
    void                    SetToggleAmpRecording(bool recordingToggle);
    bool                    IsToggleOverdraw() const;
    void                    SetToggleOverdraw(bool overdrawToggle);
    bool                    IsToggleBatch() const;
    void                    SetToggleBatch(bool batchToggle);
    bool                    IsToggleBlending() const;
    void                    SetToggleBlending(bool blendingToggle);
    bool                    IsToggleTextureDensity() const;
    void                    SetToggleTextureDensity(bool texDensityToggle);
    bool                    IsToggleInstructionProfile() const;
    void                    SetToggleInstructionProfile(bool instToggle);
    bool                    IsToggleFastForward() const;
    void                    SetToggleFastForward(bool ffToggle);
    bool                    IsToggleAaMode() const;
    void                    SetToggleAaMode(bool aaToggle);
    bool                    IsToggleStrokeType() const;
    void                    SetToggleStrokeType(bool strokeToggle);
    bool                    IsRestartMovie() const;
    void                    SetRestartMovie(bool restart);
    bool                    IsNextFont() const;
    void                    SetNextFont(bool next);
    bool                    IsCurveToleranceUp() const;
    void                    SetCurveToleranceUp(bool up);
    bool                    IsCurveToleranceDown() const;
    void                    SetCurveToleranceDown(bool down);
    bool                    IsDebugPause() const;
    void                    SetDebugPause(bool debug);
    bool                    IsDebugStep() const;
    void                    SetDebugStep(bool debug);
    bool                    IsDebugStepIn() const;
    void                    SetDebugStepIn(bool debug);
    bool                    IsDebugStepOut() const;
    void                    SetDebugStepOut(bool debug);
    bool                    IsDebugNextMovie() const;
    void                    SetDebugNextMovie(bool debug);
    bool                    IsToggleMemReport() const;
    void                    SetToggleMemReport(bool toggle);
    const StringLH&         GetLoadMovieFile() const;
    void                    SetLoadMovieFile(const char* fileName);
    SInt32                  GetProfileLevel() const;
    void                    SetProfileLevel(SInt32 profileLevel);
    SInt32                  GetBatchChange() const;
    void                    SetBatchChange(SInt32 change);

protected:

    enum OptionFlags
    {
        OF_ToggleWireframe              = 0x00000001,
        OF_TogglePause                  = 0x00000002,
        OF_ToggleAmpRecording           = 0x00000004,
        OF_ToggleOverdraw               = 0x00000008,
        OF_ToggleInstructionProfile     = 0x00000010,
        OF_ToggleFastForward            = 0x00000020,
        OF_ToggleAaMode                 = 0x00000040,
        OF_ToggleStrokeType             = 0x00000080,
        OF_RestartMovie                 = 0x00000100,
        OF_NextFont                     = 0x00000200,
        OF_CurveToleranceUp             = 0x00000400,
        OF_CurveToleranceDown           = 0x00000800,
        OF_DebugPause                   = 0x00002000,
        OF_DebugStep                    = 0x00004000,
        OF_DebugStepIn                  = 0x00008000,
        OF_DebugStepOut                 = 0x00010000,
        OF_DebugNextMovie               = 0x00020000,
        OF_ToggleMemReport              = 0x00040000,
        OF_ToggleBatch                  = 0x00080000,
        OF_ToggleBlending               = 0x00100000,
        OF_ToggleTextureDensity         = 0x00200000,
    };

    UInt32                  OptionBits;
    StringLH                LoadMovieFile;
    SInt32                  ProfileLevel;
    SInt32                  BatchChange;

    // enum used only for compatibility with older versions
    virtual MessageType     GetMessageType() const { return Msg_AppControl; } 
};

// Message containing the server listening port
// Broadcast via UDP
// The server IP address is known from the origin of the UDP packet
class MessagePort : public Message
{
public:
    static String           GetStaticTypeName()        { return "Port"; }

    MessagePort(UInt32 port = 0, const char* appName = NULL, const char* FileName = NULL);

    // Message overrides
    virtual String      GetMessageName() const        { return GetStaticTypeName(); }
    virtual void        Read(File& str);
    virtual void        Write(File& str) const;

    // Accessors
    UInt32              GetPort() const;
    UInt32              GetAddress() const;
    const StringLH&     GetPeerName() const;
    PlatformType        GetPlatform() const;
    const StringLH&     GetAppName() const;
    const StringLH&     GetFileName() const;
    virtual void        SetPeerName(const char* name);
    virtual void        SetPeerAddress(UInt32 address);
    void                SetPlatform(PlatformType platform);


protected:
    UInt32              Port;
    UInt32              Address;
    StringLH            PeerName;
    PlatformType        Platform;
    StringLH            AppName;
    StringLH            FileName;

    // enum used only for compatibility with older versions
    virtual MessageType     GetMessageType() const { return Msg_Port; } 
};

// Sent by client to server
// Base class holds the Image ID
class MessageImageRequest : public Message
{
public:
    static String       GetStaticTypeName()        { return "ImageRequest"; }
    MessageImageRequest(UInt32 imageId = 0);

    // Message overrides
    virtual void        Read(File& str);
    virtual void        Write(File& str) const;
    virtual String      GetMessageName() const        { return GetStaticTypeName(); }

    // Accessor
    UInt32              GetImageId() const;

protected:
    UInt32              ImageId;

    // enum used only for compatibility with older versions
    virtual MessageType     GetMessageType() const { return Msg_ImageRequest; } 
};

// Message with the contents of an image
// Sent by server to client upon request
// The base class member holds the corresponding image ID
class MessageImageData : public Message
{
public:
    static String           GetStaticTypeName()        { return "ImageData"; }
    MessageImageData(UInt32 imageId = 0);
    virtual ~MessageImageData();

    // Message overrides
    virtual String          GetMessageName() const        { return GetStaticTypeName(); }
    virtual void            Read(File& str);
    virtual void            Write(File& str) const;

    // Accessors
    UInt32                  GetImageId() const;
    AmpStream*              GetImageData() const;
    void                    SetImageData(AmpStream* imageData);
    bool                    IsPngFormat() const;
    void                    SetPngFormat(bool pngFormat);

protected:
    UInt32                  ImageId;
    AmpStream*              ImageDataStream;
    bool                    PngFormat;

    // enum used only for compatibility with older versions
    virtual MessageType     GetMessageType() const { return Msg_ImageData; } 
};

// Sent by client to server
// Base class holds the Image ID
class MessageFontRequest : public Message
{
public:
    static String           GetStaticTypeName()        { return "FontRequest"; }
    MessageFontRequest(UInt32 fontId = 0);

    // Message overrides
    virtual String          GetMessageName() const        { return GetStaticTypeName(); }
    virtual void            Read(File& str);
    virtual void            Write(File& str) const;

    // Accessors
    UInt32                  GetFontId() const;

protected:
    UInt32                  FontId;

    // enum used only for compatibility with older versions
    virtual MessageType     GetMessageType() const { return Msg_FontRequest; } 
};

// Message with the contents of an image
// Sent by server to client upon request
// The base class member holds the corresponding image ID
class MessageFontData : public Message
{
public:
    static String           GetStaticTypeName()        { return "FontData"; }
    MessageFontData(UInt32 fontId = 0);
    virtual ~MessageFontData();

    // Message overrides
    virtual String          GetMessageName() const        { return GetStaticTypeName(); }
    virtual void            Read(File& str);
    virtual void            Write(File& str) const;

    // Accessors
    UInt32                  GetFontId() const;
    UInt32                  GetNumTextures() const;
    void                    SetNumTextures(UInt32 numTextures);
    AmpStream*              GetImageData() const;
    void                    SetImageData(AmpStream* imageData);

protected:
    UInt32                  FontId;
    UInt32                  NumTextures;
    AmpStream*              FontDataStream;

    // enum used only for compatibility with older versions
    virtual MessageType     GetMessageType() const { return Msg_FontData; } 
};

// Compressed Message
// Can be any other message type once uncompressed
class MessageCompressed : public Message
{
public:
    static String           GetStaticTypeName()        { return "Compressed"; }

    // Message overrides
    virtual String          GetMessageName() const        { return GetStaticTypeName(); }
    virtual void            Read(File& str);
    virtual void            Write(File& str) const;

    virtual bool            Uncompress(Array<UByte>& uncompressedData);

    // Accessors
    void                    AddCompressedData(UByte* data, UPInt dataSize);
    const ArrayLH<UByte>&   GetCompressedData() const;

private:
    ArrayLH<UByte>          CompressedData;

    // enum used only for compatibility with older versions
    virtual MessageType     GetMessageType() const { return Msg_Compressed; } 
};


} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif

#endif

