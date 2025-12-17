/**************************************************************************

Filename    :   GFx_Loader.cpp
Content     :   GFxPlayer loader implementation
Created     :   June 30, 2005
Authors     :

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_File.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_UTF8Util.h"

#include "GFx/GFx_LoaderImpl.h"

#include "Render/Render_Image.h"
//#include "Render/Render_ImageInfo.h"
#include "GFx/GFx_ImageResource.h"

#include "Kernel/SF_HeapNew.h"
#include "GFx/AMP/Amp_Server.h"

#ifdef SF_ENABLE_HTTP_LOADING
#include "curl/curl.h"
#endif



void    GFx_ValidateEvaluation();

namespace Scaleform { namespace GFx {

// ***** URLBuilder implementation

// Default TranslateFilename implementation.
void    URLBuilder::BuildURL(String *ppath, const LocationInfo& loc)
{
    DefaultBuildURL(ppath, loc);  
}  

void    URLBuilder::DefaultBuildURL(String *ppath, const LocationInfo& loc)
{
    // Determine the file name we should use base on a relative path.
    if (IsPathAbsolute(loc.FileName))
    {
        *ppath = loc.FileName;
    }
    else
    {  
        // If not absolute, concatenate image path to the relative parent path.

        UPInt length = loc.ParentPath.GetSize();
        if (length > 0)
        {
            *ppath = loc.ParentPath;
            UInt32 cend = loc.ParentPath[length - 1];

            if ((cend != '\\') && (cend != '/'))
            {
                *ppath += "/";
            }
            *ppath += loc.FileName;
        }
        else
        {
            *ppath = loc.FileName;
        }
    }
}

// Static helper function used to determine if the path is absolute.
bool    URLBuilder::IsPathAbsolute(const char *putf8str)
{

    // Absolute paths can star with:
    //  - protocols:        'file://', 'http://'
    //  - windows drive:    'c:\'
    //  - UNC share name:   '\\share'
    //  - unix root         '/'

    // On the other hand, relative paths are:
    //  - directory:        'directory/file'
    //  - this directory:   './file'
    //  - parent directory: '../file'
    // 
    // For now, we don't parse '.' or '..' out, but instead let it be concatenated
    // to string and let the OS figure it out. This, however, is not good for file
    // name matching in library/etc, so it should be improved.

    if (!putf8str || !*putf8str)
        return true; // Treat empty strings as absolute.    

    UInt32 charVal = UTF8Util::DecodeNextChar(&putf8str);

    // Fist character of '/' or '\\' means absolute path.
    if ((charVal == '/') || (charVal == '\\'))
        return true;

    while (charVal != 0)
    {
        // Treat a colon followed by a slash as absolute.
        if (charVal == ':')
        {
            charVal = UTF8Util::DecodeNextChar(&putf8str);
            // Protocol or windows drive. Absolute.
            if ((charVal == '/') || (charVal == '\\'))
                return true;
        }
        else if ((charVal == '/') || (charVal == '\\'))
        {
            // Not a first character (else 'if' above the loop would have caught it).
            // Must be a relative path.
            break;
        }

        charVal = UTF8Util::DecodeNextChar(&putf8str);
    }

    // We get here for relative paths.
    return false;
}

// Modifies path to not include the filename, leaves trailing '/'.
bool    URLBuilder::ExtractFilePath(String *ppath)
{
    // And strip off the actual file name.
    SPInt length  = (SPInt)ppath->GetLength();
    SPInt i       = length-1;
    for (; i>=0; i--)
    {
        UInt32 charVal = ppath->GetCharAt(i);

        // The trailing name will end in either '/' or '\\',
        // so just clip it off at that point.
        if ((charVal == '/') || (charVal == '\\'))
        {
            *ppath = ppath->Substring(0, i+1);
            break;
        }
    }

    // Technically we can have extra logic somewhere for paths,
    // such as enforcing protocol and '/' only based on flags,
    // but we keep it simple for now.
    return (i >= 0);
}

bool URLBuilder::IsProtocol(const String& path)
{
    String protocol = path.GetProtocol();

    return (protocol == "http://" || protocol == "file://" || protocol == "https://");
}

#ifdef SF_ENABLE_HTTP_LOADING
bool URLBuilder::SendURLRequest(Array<UByte>* bytes, const String& path, UrlMethod method, const char* data, int dataLength, const Array<String>* headers, const char* contentType)
{
    CURL* curlHandle = curl_easy_init();
    if (curlHandle == NULL)
    {
        return false;
    }
    PutData putData;

    curl_easy_setopt(curlHandle, CURLOPT_URL, path.ToCStr());
    curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, HttpWriteData);
    curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, bytes);
    curl_easy_setopt(curlHandle, CURLOPT_USERAGENT, "Scaleform-agent/1.0");

//    SF_ASSERT(curl_version_info(CURLVERSION_NOW)->features & CURL_VERSION_SSL);
    curl_easy_setopt(curlHandle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curlHandle, CURLOPT_SSL_VERIFYHOST, 0L);

    if (method == Url_Method_Post)
    {
        if (data != NULL && dataLength > 0)
        {
            curl_easy_setopt(curlHandle, CURLOPT_POSTFIELDS, data);
        }
        else
        {
            curl_easy_setopt(curlHandle, CURLOPT_POST, 1L);
        }
    }
    else if (method == Url_Method_Put)
    {
        curl_easy_setopt(curlHandle, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curlHandle, CURLOPT_PUT, 1L);
        if (data != NULL && dataLength > 0)
        {
            putData.Data = data;
            putData.DataLeft = static_cast<size_t>(dataLength);
            curl_easy_setopt(curlHandle, CURLOPT_READDATA, &putData);
            curl_easy_setopt(curlHandle, CURLOPT_READFUNCTION, HttpReadData);
            curl_easy_setopt(curlHandle, CURLOPT_INFILESIZE_LARGE, (curl_off_t)dataLength);
        }
    }
    curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, bytes);

    struct curl_slist* curlHeaders = NULL;
    if (contentType != NULL && !String(contentType).IsEmpty())
    {
        String contentFull("Content-Type: ");
        contentFull += contentType;
        curlHeaders = curl_slist_append(curlHeaders, contentFull.ToCStr()); 
    }
    if (headers != NULL)
    {
        for (UPInt i = 0; i < headers->GetSize(); ++i)
        {
            curlHeaders = curl_slist_append(curlHeaders, headers->At(i).ToCStr()); 
        }
    }
    if (curlHeaders != NULL)
    {
        curl_easy_setopt(curlHandle, CURLOPT_HTTPHEADER, curlHeaders);
    }

    CURLcode res = curl_easy_perform(curlHandle);
    curl_easy_cleanup(curlHandle);
    return (res == CURLE_OK);
}

size_t URLBuilder::HttpWriteData(char* buffer, size_t size, size_t nmemb, void* userp)
{
    Array<UByte>* bytes = (Array<UByte>*)userp;

    UPInt sizeIncrease = size * nmemb;
    if (sizeIncrease > 0)
    {
        bytes->Append((UByte*)buffer, sizeIncrease);
    }
    return sizeIncrease;
}

size_t URLBuilder::HttpReadData(void* ptr, size_t size, size_t nmemb, void* userp)
{
    size_t retcode = size * nmemb;
    PutData* putData = static_cast<PutData*>(userp);

    if (putData->DataLeft >= retcode)
    {
        memcpy(ptr, putData->Data, retcode);
        putData->DataLeft -= retcode;
    }
    else
    {
        memcpy(ptr, putData->Data, putData->DataLeft);
        retcode = putData->DataLeft;
        putData->DataLeft = 0;
    }
    return retcode;
}


#endif


// ***** GFxStateBag implementation


// Implementation that allows us to override the log.
File*      StateBag::OpenFileEx(const char *pfilename, Log *plog)
{
    Ptr<FileOpenerBase> popener = GetFileOpener();
    if (!popener)
    {             
        // Don't even have a way to open the file.
        if (plog)
            plog->LogError(
            "Loader failed to open '%s', FileOpener not installed",
            pfilename);
        return 0;
    }

    return popener->OpenFileEx(pfilename, plog);
}

// Opens a file using the specified callback.
File*      StateBag::OpenFile(const char *pfilename)
{
    return OpenFileEx(pfilename, GetLog());
}


// ***** Loader implementation

Loader::Loader(const Loader::LoaderConfig& config)
{
    InitLoader(config);
}

Loader::Loader(const Ptr<FileOpenerBase>& pfileOpener, 
                     const Ptr<ZlibSupportBase>& pzlib
                     )
{
    LoaderConfig config(0, pfileOpener, pzlib/*, pjpeg*/);
    InitLoader(config);
}

// Create a new loader, copying it's library and states.
Loader::Loader(const Loader& src)
{
    GFx_ValidateEvaluation();

    // Create new LoaderImpl with copied states.
    pImpl = SF_NEW LoaderImpl(src.pImpl);
    // Copy strong resource lib reference.
    pStrongResourceLib = src.pStrongResourceLib;
    if (pStrongResourceLib)
        pStrongResourceLib->AddRef();

    SF_DEBUG_ERROR(!pImpl, "Loader::Loader failed to initialize properly, low memory");
}


Loader::~Loader()
{
    SF_AMP_CODE(AmpServer::GetInstance().RemoveLoader(this);)
    if (pImpl)
        pImpl->Release();
    if (pStrongResourceLib)
        pStrongResourceLib->Release();

}

void Loader::InitLoader(const LoaderConfig& cfg)
{
    GFx_ValidateEvaluation();
       
    bool debugHeap = (cfg.DefLoadFlags & LoadDebugHeap) ? true : false;

    DefLoadFlags       = cfg.DefLoadFlags;
    pStrongResourceLib = SF_NEW ResourceLib(debugHeap);

    if ((pImpl = SF_NEW LoaderImpl(pStrongResourceLib, debugHeap))!=0)
    {
        SetFileOpener(cfg.pFileOpener);
        SetParseControl(Ptr<ParseControl>(*SF_NEW ParseControl(ParseControl::VerboseParseNone)));
        SetZlibSupport(cfg.pZLibSupport);
    }    

#ifdef SF_AMP_SERVER
    if (!debugHeap)
    {
        AmpServer::GetInstance().AddLoader(this);
    }
#endif

    SF_DEBUG_ERROR(!pImpl || !pStrongResourceLib,
        "Loader::Loader failed to initialize properly, low memory");
}


StateBag* Loader::GetStateBagImpl() const
{
    return pImpl;
}

bool Loader::CheckTagLoader(int tagType) const 
{ 
    return (pImpl) ? pImpl->CheckTagLoader(tagType) : 0;        
};

// Resource library management.
void               Loader::SetResourceLib(ResourceLib *plib)
{
    if (pImpl)
    {
        if (plib)
            plib->AddRef();
        if (pStrongResourceLib)
            pStrongResourceLib->Release();
        pStrongResourceLib      = plib;
        pImpl->SetWeakResourceLib(plib->GetWeakLib());
    }
}

ResourceLib*    Loader::GetResourceLib() const
{
    return pStrongResourceLib;
}


// *** Movie Loading

// Movie Loading APIs just delegate to LoaderImpl after some error checking.

bool    Loader::GetMovieInfo(const char *pfilename, MovieInfo *pinfo,
                                bool getTagCount, unsigned loadConstants)
{
    if (!pfilename || pfilename[0]==0)
    {
        SF_DEBUG_WARNING(1, "Loader::GetMovieInfo - passed filename is empty, no file to load");
        return 0;
    }
    if (!pinfo)
    {
        SF_DEBUG_WARNING(1, "Loader::GetMovieInfo - passed GFxMovieInfo pointer is NULL");
        return 0;
    }

    return pImpl ? pImpl->GetMovieInfo(pfilename, pinfo, getTagCount, loadConstants | DefLoadFlags) : 0;
}


MovieDef*    Loader::CreateMovie(const char *pfilename, unsigned loadConstants, UPInt memoryArena)
{
    if (!pfilename || pfilename[0]==0)
    {
        SF_DEBUG_WARNING(1, "Loader::CreateMovie - passed filename is empty, no file to load");
        return 0;
    }
    return pImpl ? pImpl->CreateMovie(pfilename, loadConstants | DefLoadFlags, memoryArena) : 0;
}

/*
MovieDef*    Loader::CreateMovie(File *pfile, unsigned loadConstants)
{
if (!pfile || !pfile->IsValid())
{
SF_DEBUG_WARNING(1, "Loader::CreateMovie - passed file is not valid, no file to load");
return 0;
}
return pImpl ? pImpl->CreateMovie(pfile, 0, loadConstants) : 0;
}
*/


// Unpins all resources held in the library
void Loader::UnpinAll()
{
    if (pStrongResourceLib)    
        pStrongResourceLib->UnpinAll();    
}

void Loader::CancelLoading()
{
    pImpl->CancelLoading();
}


void ImagePackParamsBase::SetTextureConfig(const TextureConfig& config)
{
     PackTextureConfig = config;
}

//////////////////////////////////////////////////////////////////////////

System::System(SysAllocBase* psysAlloc) : Scaleform::System(psysAlloc)
{
    SF_AMP_CODE(AMP::Server::Init();)
}    

System::System(const MemoryHeap::HeapDesc& rootHeapDesc, SysAllocBase* psysAlloc) 
    : Scaleform::System(rootHeapDesc, psysAlloc)
{
    SF_AMP_CODE(AMP::Server::Init();)
}


System::~System()
{
    SF_AMP_CODE(AMP::Server::Uninit();)
}

// Initializes GFxSystem core, setting the global heap that is needed for GFx
// memory allocations.
void System::Init(const MemoryHeap::HeapDesc& rootHeapDesc, SysAllocBase* psysAlloc)
{
    Scaleform::System::Init(rootHeapDesc, psysAlloc);

    SF_AMP_CODE(AMP::Server::Init();)

}

void System::Destroy()
{
    SF_AMP_CODE(AMP::Server::Uninit();)
    Scaleform::System::Destroy();
}


}

} // Scaleform::GFx

// Validate Check - Eval License Reader code was moved to GFx_LoaderUtil.cpp so that 
// other engines could more easily replace it with their own validation checking code.


extern "C" {
#ifdef SF_BUILD_DEBUG
    int GFx_Compile_without_SF_BUILD_DEBUG;
#else
    int GFx_Compile_with_SF_BUILD_DEBUG;
#endif
}
