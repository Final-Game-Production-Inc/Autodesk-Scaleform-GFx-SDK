/**************************************************************************

Filename    :   GFx_FontManager.cpp
Content     :   Font manager functinality
Created     :   May 18, 2007
Authors     :   Artyom Bolgar, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFx/GFx_FontManager.h"
#include "GFx/GFx_MovieDef.h"
#include "GFx/GFx_FontLib.h"
#include "GFx/GFx_DisplayObject.h"
#include "Kernel/SF_MsgFormat.h"
#include "GFx/GFx_PlayerImpl.h"

namespace Scaleform { namespace GFx {

State* FontManagerStates::GetStateAddRef(State::StateType state) const
{
    if (state == State::State_FontLib)
    {
        if (pFontLib)
            pFontLib->AddRef();
        return pFontLib.GetPtr();
    }
    else if (state == State::State_FontMap)
    {
        if (pFontMap)
            pFontMap->AddRef();
        return pFontMap.GetPtr();
    }
    else if (state == State::State_FontProvider)
    {
        if (pFontProvider)
            pFontProvider->AddRef();
        return pFontProvider.GetPtr();
    }
    if (state == State::State_Translator)
    {
        if (pTranslator)
            pTranslator->AddRef();
        return pTranslator.GetPtr();
    }
    SF_ASSERT(pDelegate);
    return pDelegate->GetStateAddRef(state);
}

UInt8 FontManagerStates::CheckStateChange(FontLib* pfontLib, FontMap* pfontMap,
                                          FontProvider* pfontProvider, Translator* ptranslator)
{
    UInt8 stateChangeFlag = 0u;

    if (pFontLib.GetPtr() != pfontLib)
    {
        stateChangeFlag |= DisplayObjectBase::StateChanged_FontLib;
        pFontLib = pfontLib;
    }
    if (pFontMap.GetPtr() != pfontMap)
    {
        stateChangeFlag |=  DisplayObjectBase::StateChanged_FontMap;
        pFontMap = pfontMap;
    }
    if (pFontProvider.GetPtr() != pfontProvider)
    {
        stateChangeFlag |=  DisplayObjectBase::StateChanged_FontProvider;
        pFontProvider = pfontProvider;
    }
    if (pTranslator.GetPtr() != ptranslator)
    {
        stateChangeFlag |=  DisplayObjectBase::StateChanged_Translator;
        pTranslator = ptranslator;
    }
    return stateChangeFlag;
}

//////////////////////////////////
// FontManager
//
FontManager::FontManager(MovieImpl* movie,
                         MovieDefImpl *pdefImpl,
                         FontManagerStates *pstate)
{
    pMovie        = movie;
    pDefImpl      = pdefImpl;
    pWeakLib      = NULL;
    pState        = pstate;
    commonInit();
}

FontManager::FontManager(ResourceWeakLib *pweakLib,
                         FontManagerStates *pstate)
{
    pMovie        = NULL;
    pDefImpl      = NULL;
    pWeakLib      = pweakLib;
    pState        = pstate;
    commonInit();
}

void FontManager::commonInit()
{
    Ptr<FontData> pfontData = *SF_HEAP_AUTO_NEW(this) FontData();
    Ptr<FontResource> pemptyFont = *SF_HEAP_AUTO_NEW(this) FontResource(pfontData, 0);
    pEmptyFont = *SF_NEW FontHandle(0, pemptyFont->GetFont());
}

FontManager::~FontManager()
{
    // CachedFonts must be empty here.
    SF_ASSERT(CreatedFonts.GetSize() == 0);
}


void FontManager::RemoveFontHandle(FontHandle *phandle)
{
    if (phandle != pEmptyFont)
    {
        unsigned fontCount = (unsigned)CreatedFonts.GetSize();
        SF_UNUSED(fontCount);
        //if (!fontCount)
        //    return;
        CreatedFonts.Remove(phandle);
        // This should not fail but has been seen to do so due to font style mask
        // mismatch. The new operator == which compares pointers resolves that.
        SF_ASSERT(CreatedFonts.GetSize() == fontCount - 1);
    }
}

static const char *StrFlags[] = 
{
    "", "[Bold]", "[Italic]", "[Bold,Italic]", "[Device]", "[Bold,Device]", "[Italic,Device]", "[Bold,Italic,Device]"
};

static inline const char* FontFlagsToString(unsigned matchFontFlags)
{
    if (!matchFontFlags)
        return StrFlags[0];
    if (matchFontFlags & Font::FF_DeviceFont)
    {
        if((matchFontFlags & Font::FF_BoldItalic) == Font::FF_BoldItalic)
            return StrFlags[7];
        if(matchFontFlags & Font::FF_Bold)
            return StrFlags[5];
        if(matchFontFlags & Font::FF_Italic)
            return StrFlags[6];
        return StrFlags[4];
    }
    else
    {
        if((matchFontFlags & Font::FF_BoldItalic) == Font::FF_BoldItalic)
            return StrFlags[3];
        if(matchFontFlags & Font::FF_Bold)
            return StrFlags[1];
        if(matchFontFlags & Font::FF_Italic)
            return StrFlags[2];
    }
    return StrFlags[0];
}

static inline void AddSearchInfo(FontManager::FontSearchPathInfo* psearchInfo, const char* line)
{
    if (!psearchInfo)
        return;
    String sindent("   ");
    for (int i = 0; i < psearchInfo->Indent; ++i)
        psearchInfo->Info += sindent;
    psearchInfo->Info += line;
    psearchInfo->Info += "\n";
}

static inline void AddSearchInfo(FontManager::FontSearchPathInfo* psearchInfo, const char* pfontname, unsigned flags, bool fontlib_installed,
                                 const MovieDefImpl::SearchInfo& resSearchInfo)
{
    char buff[1024];
    StringDataPtr buffData(buff, sizeof(buff));

    if (!psearchInfo)
        return;
    if(resSearchInfo.Status == MovieDefImpl::SearchInfo::FoundInResources)
    {
        Format(buffData, "Movie resource: \"{0}\" {1} found.", pfontname, FontFlagsToString(flags));
        AddSearchInfo(psearchInfo, buff);
        return;
    }
    if(resSearchInfo.Status == MovieDefImpl::SearchInfo::FoundInResourcesNeedFaux)
    {
        Format(buffData, "Movie resource: \"{0}\" {1} found, requires faux", pfontname, FontFlagsToString(flags));
        AddSearchInfo(psearchInfo, buff);
        return;
    }
    if(resSearchInfo.Status == MovieDefImpl::SearchInfo::FoundInResourcesNoGlyphs)
    {
        Format(buffData, "Movie resource: \"{0}\" {1} ref found, requires FontLib/Map/Provider.",
                            pfontname, FontFlagsToString(flags));
        AddSearchInfo(psearchInfo, buff);
        return;
    }

    Format(buffData, "Movie resource: \"{0}\" {1} not found.", pfontname, FontFlagsToString(flags));
    AddSearchInfo(psearchInfo, buff);
    if(resSearchInfo.Status == MovieDefImpl::SearchInfo::FoundInImports || 
       (resSearchInfo.Status == MovieDefImpl::SearchInfo::FoundInImportsFontLib && !fontlib_installed) )
    {
        Format(buffData, "Imports       : \"{0}\" {1} found in \"{2}\".", 
                 pfontname, FontFlagsToString(flags), resSearchInfo.ImportFoundUrl);
        AddSearchInfo(psearchInfo, buff);
        return;
    }
    if(resSearchInfo.Status == MovieDefImpl::SearchInfo::FoundInImportsFontLib)
    {
        Format(buffData, "Imports       : \"{0}\" {1} import delegates to font library.", 
                 pfontname, FontFlagsToString(flags));
        AddSearchInfo(psearchInfo, buff);
        return;
    }
    Format(buffData, "Imports       : \"{0}\" {1} not found.", pfontname, FontFlagsToString(flags));
    AddSearchInfo(psearchInfo, buff);
    if (resSearchInfo.ImportSearchUrls.GetSize() > 0)
    {
        String tmp;
        for(MovieDefImpl::SearchInfo::StringSet::ConstIterator it = resSearchInfo.ImportSearchUrls.Begin(); 
                it != resSearchInfo.ImportSearchUrls.End(); ++it)
        {
            if (it != resSearchInfo.ImportSearchUrls.Begin())
                tmp += ", ";
            tmp += String("\"") + *it + "\"";
        }
        Format(buffData, "              : {0}.", tmp);
        AddSearchInfo(psearchInfo, buff);
    }
    if(resSearchInfo.Status == MovieDefImpl::SearchInfo::FoundInExports)
    {
        Format(buffData, "Exported      : \"{0}\" {1} found.", pfontname, FontFlagsToString(flags));
        AddSearchInfo(psearchInfo, buff);
        return;
    }
    Format(buffData, "Exported      : \"{0}\" {1} not found.", pfontname, FontFlagsToString(flags));
    AddSearchInfo(psearchInfo, buff);
}

static inline void AddSearchInfo(FontManager::FontSearchPathInfo* psearchInfo, const char* str1,
                                 const char* str2, const char* str3)
{
    if (!psearchInfo)
        return;
    StringBuffer buf;
    buf += str1;
    buf += str2;
    buf += str3;
    AddSearchInfo(psearchInfo, buf);
}
static inline void AddSearchInfo(FontManager::FontSearchPathInfo* psearchInfo, const char* str1, const char* str2, 
                                 const char* str3, unsigned flags, const char* str4)
{
    if (!psearchInfo)
        return;
    StringBuffer buf;
    buf += str1;
    buf += str2;
    buf += str3;
    buf += FontFlagsToString(flags);
    buf += str4;
    AddSearchInfo(psearchInfo, buf);
}
static inline void AddSearchInfo(FontManager::FontSearchPathInfo* psearchInfo, const char* str1, const char* str2, const char* str3, 
                                 unsigned flags1, const char* str4, const char* str5, const char* str6, unsigned flags2)
{
    if (!psearchInfo)
        return;
    StringBuffer buf;
    buf += str1;
    buf += str2;
    buf += str3;
    buf += FontFlagsToString(flags1);
    buf += str4;
    buf += str5;
    buf += str6;
    buf += FontFlagsToString(flags2);
    AddSearchInfo(psearchInfo, buf);
}

static inline void AddSearchInfo(FontManager::FontSearchPathInfo* psearchInfo, const char* str1, const char* str2,
                                 const char* str3, const char* str4, const char* str5, unsigned flags)
{
    if (!psearchInfo)
        return;
    StringBuffer buf;
    buf += str1;
    buf += str2;
    buf += str3;
    buf += str4;
    buf += str5;
    buf += FontFlagsToString(flags);
    AddSearchInfo(psearchInfo, buf);
}
// If searchInfo is not NULL then the search algorithm will not check internal cache and any new created FontHandles 
// will not be cached
FontHandle* FontManager::FindOrCreateHandle(const char* pfontName, unsigned matchFontFlags, 
                                            FontResource** ppfoundFont,
                                            FontSearchPathInfo* searchInfo)
{
// DBG
//SF_ASSERT(!(matchFontFlags & Font::FF_BoldItalic));
//if (matchFontFlags & Font::FF_BoldItalic)
//    return 0;

    FontKey       key(pfontName, matchFontFlags);
    FontHandle*   phandle       = 0;
    FontLib*      pfontLib      = pState->pFontLib.GetPtr();
    FontProvider* pfontProvider = pState->pFontProvider.GetPtr();
    FontMap*      pfontMap      = pState->pFontMap.GetPtr();

    // Our matchFontFlag argument may include FF_DeviceFont flag if device preference is
    // requested. The biggest part this is handled correctly by MatchFont used in our
    // CreatedFonts.Get(), pDefImpl->GetFontResource and FontLib lookups. However,
    // if device font match is not found we may need to do a second cycle to find an embedded
    // font instead (default Flash behavior, since DeviceFont is only a hint).
    int indentDif = 0;
    const NodePtr* psetNode = NULL, *pfoundNode = NULL;
    MovieDefImpl::SearchInfo resSearchInfo;
    MovieDefImpl::SearchInfo* presSearchInfo = NULL;
    bool secondLoop                             = false;


#ifndef SF_NO_IME_SUPPORT
    if (pIMECandidateFont && !String::CompareNoCase(pIMECandidateFont->GetFontName(), pfontName))
    {
        pIMECandidateFont->AddRef();
        return pIMECandidateFont;
    }
#endif //#ifdef SF_NO_IME_SUPPORT

retry_font_lookup:
    if (searchInfo)
    {
        indentDif++;
        searchInfo->Indent++;
        presSearchInfo = &resSearchInfo;
    } 
    else
    {
        // If the font has already been created, return it.
        psetNode = CreatedFonts.Get(key);
        if (psetNode)
        {
            // A special case when the font was cached as device font first and when 
            // non-device font was requested with the same style and name: need to check
            // if non-device font exists.            
            if ((matchFontFlags & Font::FF_DeviceFont) == 0 && 
                (psetNode->pNode->GetFontFlags() & Font::FF_DeviceFont) != 0)
            {
                // Check if we already looked for non-device font for
                // this node. If so, just return what we have found.
                if (psetNode->SearchedForNonDeviceFont)
                {
                    psetNode->pNode->AddRef();
                    return psetNode->pNode;
                }
                else
                {
                    // Otherwise, save the node and perform regular search for FontResource.
                    // If we haven't found a non-device font then set the flag 
                    // 'SearchedForNonDeviceFont' to true and return the originally
                    // found node.
                    pfoundNode = psetNode;
                }
            }
            else
            {
                psetNode->pNode->AddRef();
                return psetNode->pNode;
            }
        }
    }

    // Search MovieDef for the desired font and create it if necessary.
    // The resource is not AddRefed here, so don't do extra release.
    FontResource* pfontRes = 
        (pDefImpl) ? pDefImpl->GetFontResource(pfontName, matchFontFlags, presSearchInfo) : NULL;;
    AddSearchInfo(searchInfo, pfontName, matchFontFlags, pState->pFontLib.GetPtr() != NULL, resSearchInfo);

    MovieDef* psrcMovieDef = NULL;
    // check registered fonts
    if (!pfontRes && pMovie)
    {
        pfontRes = pMovie->FindRegisteredFont(pfontName, matchFontFlags, &psrcMovieDef);
        if (!pfontRes)
            AddSearchInfo(searchInfo, "Registered fonts: \"", pfontName, "\"", matchFontFlags, " not found.");
    }

    if (pfontRes)
    {
        // If NotResolve flag for the found font is set we have to search it through 
        // FontLib and FontProvider
        if (pfontRes->GetFont()->IsResolved())
        {
            // If we are looking for bold/italic, and there is an empty font found, 
            // then return NULL to give opportunity to faux bold/italic.
            if ((matchFontFlags & Font::FF_BoldItalic) && !pfontRes->GetFont()->HasVectorOrRasterGlyphs())
            {
                // need to pass the found font out, just in case if font manager
                // fails looking for a regular font for faux bold/italic
                if (ppfoundFont) 
                    *ppfoundFont = pfontRes;
                if (pfoundNode)
                {
                    // going to create a faux bold/italic font. Remove the old device font
                    // hash entry since the old and new entry would have the same key.
                    pfoundNode->pNode->pFontManager = NULL; // unlink fonthandle from manager
                    CreatedFonts.Remove(pfoundNode->pNode);
                }
                return 0;
            }
            // The above lookup can return a font WITH A DIFFERENT name in
            // case export names were used for a search. If that happens,
            // the passed name will be stored and used in FontHandle.
            phandle = SF_NEW FontHandle(searchInfo? NULL : this, pfontRes->GetFont(), pfontName, 0, psrcMovieDef);
        }
    }

    // Have we already found a node in CreateFonts hash?
    if (pfoundNode)
    {
        if (!phandle) // no new handle was still created?
        {
            // Non-device font is not found, but device font was found in CachedFonts
            // above: set the flag 'SearchedForNonDeviceFont' to true and return the originally
            // found node.
            pfoundNode->SearchedForNonDeviceFont = true;
            pfoundNode->pNode->AddRef();
            return pfoundNode->pNode;
        }
        else
        {
            // a new handle was created, font found. Remove the old hash entry since
            // the old and new entry would have the same key.
            pfoundNode->pNode->pFontManager = NULL; // unlink fonthandle from manager
            CreatedFonts.Remove(pfoundNode->pNode);

            // The handle for very same font is already in the hash, 
            // just return the existing handle and delete the new one.
            pfoundNode = CreatedFonts.Get(phandle);
            if (pfoundNode != 0)
            {
                phandle->pFontManager = NULL;
                phandle->Release();
                pfoundNode->pNode->AddRef();
                return pfoundNode->pNode;
            }
        }
    }

    // Lookup an alternative font name for the font. Font flags can
    // also be altered if the font map forces a style.
    const char* plookupFontName = pfontName;
    unsigned    lookupFlags     = matchFontFlags;
    float       scaleFactor     = 1.0f;
    unsigned    overridenFlags  = 0;

    if (!phandle && pfontMap)
    {       
        if (pfontMap->GetFontMapping(&FontMapEntry, pfontName))
        {
            plookupFontName = FontMapEntry.Name.ToCStr();
            lookupFlags     = FontMapEntry.UpdateFontFlags(matchFontFlags);
            scaleFactor     = FontMapEntry.ScaleFactor;
            overridenFlags  = FontMapEntry.GetOverridenFlags();
            if (searchInfo)
                AddSearchInfo(searchInfo, "Applying FontMap: \"", pfontName, "\"  mapped to \"", 
                                            plookupFontName, "\"" , lookupFlags);
            else
            {
                // When we substitute a font name, it is possible that
                // this name already existed - so do a hash lookup again.
                FontKey    lookupKey(plookupFontName, lookupFlags);
                psetNode = CreatedFonts.Get(lookupKey);
                if (psetNode)
                {
                    // If font scale factor of a fount font is not the same as in FontMapEntry we 
                    // need to create a new font handle
                    if (psetNode->pNode->FontScaleFactor != scaleFactor)
                    {
                        phandle = SF_NEW
                            FontHandle(searchInfo? NULL : this, psetNode->pNode->pFont, 
                                          pfontName, matchFontFlags, psetNode->pNode->pSourceMovieDef );
                        phandle->FontScaleFactor = scaleFactor;
                    }
                    else
                    {
                        psetNode->pNode->AddRef();
                        return psetNode->pNode;
                    }
                }

                // Well, we need to check for (pfontName, changed lookupFlags) also,
                // since in the case if a normal font with name "name" is substituted by 
                // the bold one, that substitution will be placed in the hash table as (name, Bold),
                // even though we were (and will be next time!) looking for the (name, 0).
                FontKey    lookupKey2(pfontName, lookupFlags);
                psetNode = CreatedFonts.Get(lookupKey2);
                if (psetNode)
                {
                    // If font scale factor of a fount font is not the same as in FontMapEntry we 
                    // need to create a new font handle
                    if (psetNode->pNode->FontScaleFactor != scaleFactor)
                    {
                        phandle = SF_NEW FontHandle(searchInfo? NULL :this, psetNode->pNode->pFont, 
                            pfontName, matchFontFlags, psetNode->pNode->pSourceMovieDef );
                        phandle->FontScaleFactor = scaleFactor;
                    }
                    else
                    {
                        psetNode->pNode->AddRef();
                        return psetNode->pNode;
                    }
                }
            }
        }
    }

    if (!phandle && pfontLib)
    {
        // Find/create compatible font through FontLib.
        FontLib::FontResult fr;
        if (pfontLib->FindFont(&fr, plookupFontName, lookupFlags,
            pDefImpl, pState, pWeakLib))
        {
            if (overridenFlags)
            {
                // if FauxBold or/and FauxItalic attributes are set (overridenFlags != 0)
                // then need to look for already existing font handle first.
                // If not found - create one with using overridenFlags.
                FontKey    lookupKey(pfontName, overridenFlags);
                psetNode = CreatedFonts.Get(lookupKey);
                if (!psetNode)
                {
                    // Use font handle to hold reference to resource's MovieDef.
                    phandle = SF_NEW FontHandle(searchInfo? NULL :this, fr.GetFontResource()->GetFont(),
                        pfontName, overridenFlags, fr.GetMovieDef());
                    phandle->FontScaleFactor = scaleFactor;
                }
                else
                {
                    psetNode->pNode->AddRef();
                    return psetNode->pNode;
                }
                AddSearchInfo(searchInfo, "Searching FontLib: \"", pfontName, "\" ", overridenFlags, " found.");
            }
            else
            {
                // Use font handle to hold reference to resource's MovieDef.
                phandle = SF_NEW FontHandle(searchInfo? NULL :this, fr.GetFontResource()->GetFont(),
                    pfontName, 0, fr.GetMovieDef());
                phandle->FontScaleFactor = scaleFactor;
                AddSearchInfo(searchInfo, "Searching FontLib: \"", plookupFontName, "\" ", lookupFlags, " found.");
            }
        }
        else
            AddSearchInfo(searchInfo, "Searching FontLib: \"", plookupFontName, "\" ", lookupFlags, " not found.");
    }
    if (!secondLoop && searchInfo && !phandle && !pfontLib && resSearchInfo.Status == MovieDefImpl::SearchInfo::FoundInResourcesNoGlyphs)
        AddSearchInfo(searchInfo, "FontLib not installed.");

    if (!phandle && pfontProvider)
    {
        // Create or lookup a system font if system is available.
        SF_ASSERT(pDefImpl || pWeakLib);
        Ptr<FontResource> pfont = 
            *FontResource::CreateFontResource(plookupFontName, lookupFlags,
            pfontProvider, ((pDefImpl) ? pDefImpl->GetWeakLib() : pWeakLib));
        if (pfont)
        {
            AddSearchInfo(searchInfo, "Searching FontProvider: \"", plookupFontName, "\" ", lookupFlags, " found.");
            phandle = SF_NEW FontHandle(searchInfo? NULL : this, pfont->GetFont(), pfontName);
            phandle->FontScaleFactor = scaleFactor;
        }
        else
            AddSearchInfo(searchInfo, "Searching FontProvider: \"", plookupFontName, "\" ", lookupFlags, " not found.");
    }
    if (!secondLoop && searchInfo && !phandle && !pfontProvider && resSearchInfo.Status == MovieDefImpl::SearchInfo::FoundInResourcesNoGlyphs)
        AddSearchInfo(searchInfo, "FontProvider not installed.");


    // If device font was not found, try fontLib without that flag,
    // as FontLib is allowed to masquerade as a device font source.
    if (!phandle && pfontLib && (lookupFlags & Font::FF_DeviceFont))
    {
        // Find/create compatible font through FontLib.
        unsigned lf = lookupFlags & ~Font::FF_DeviceFont;
        FontLib::FontResult fr;
        if (pfontLib->FindFont(&fr, plookupFontName, lf, pDefImpl, pState))
        {
            // Use font handle to hold reference to resource's MovieDef.
            phandle = SF_NEW FontHandle(searchInfo? NULL : this, fr.GetFontResource()->GetFont(),
                                        pfontName, Font::FF_DeviceFont, fr.GetMovieDef());
            phandle->FontScaleFactor = scaleFactor;
            AddSearchInfo(searchInfo, "Searching FontLib without [Device] flag: \"", plookupFontName, "\" ", lf, " found.");
        }
        else
            AddSearchInfo(searchInfo, "Searching FontLib without [Device] flag: \"", plookupFontName, "\" ", lf, " not found.");
    }


    // If font handle was successfully created, add it to list and return.
    if (phandle)
    {
        if (searchInfo)
            searchInfo->Indent -= indentDif;
        else 
        {
            SF_ASSERT(CreatedFonts.Get(phandle) == 0);
            CreatedFonts.Add(phandle);
        }
        return phandle;
    }

    // If DeviceFont was not found, search for an embedded font.
    if (matchFontFlags & Font::FF_DeviceFont)
    {
        AddSearchInfo(searchInfo, "Searching again without [Device] flag:");
        matchFontFlags &= ~Font::FF_DeviceFont;
        // Must modify key flags value otherwise incorrect multiple copies of the same
        // font would get created and put into a hash (causing waste and crash later).
        key.FontStyle  = matchFontFlags;
        // Do not search font provider in a second pass since failure in
        // the first pass would imply failure in the second one as well.
        pfontProvider = 0;
        pfontLib = 0;
        secondLoop = true;
        goto retry_font_lookup;
    }
    if (searchInfo)
        searchInfo->Indent -= indentDif;
    return 0;
}

FontHandle* FontManager::CreateFontHandle(const char* pfontName, unsigned matchFontFlags, 
                                                bool allowListOfFonts, FontSearchPathInfo* searchInfo)
{
    if (!allowListOfFonts)
        return CreateFontHandleFromName(pfontName, matchFontFlags, searchInfo);
    else
    {
        FontHandle* fh = NULL;
        // traverse through the comma-separated list of fonts in pfontName,
        // until font is created.
        const char* pe = NULL;
        char buf[128];
        do 
        {
            pe = SFstrchr(pfontName, ',');
            const char* curName;
            UPInt len;
            if (pe)
            {
                len = pe - pfontName;
                if (len > sizeof(buf) - 1)
                    continue; // skip too long font name - it doesn't fit to our buffer
                SFstrncpy(buf, sizeof(buf)-1, pfontName, len);
                buf[len] = '\0';
                curName = buf;
                pfontName += len + 1; // skip the name + comma
            }
            else
            {
                curName = pfontName;
                len = SFstrlen(pfontName);
            }
            fh = CreateFontHandleFromName(curName, matchFontFlags, searchInfo);
        } while (!fh && pe != NULL);
        return fh;
    }
}

FontHandle* FontManager::CreateFontHandleFromName(const char* pfontName, unsigned matchFontFlags, 
                                                        FontSearchPathInfo* searchInfo)
{
    int saveIndent = 0;
    if (searchInfo)
    {
        saveIndent = searchInfo->Indent;
        AddSearchInfo(searchInfo, "Searching for font: \"", pfontName, "\" ", matchFontFlags, "");
    }
    FontResource*    pfoundFont = NULL;
    FontHandle*      phandle = FindOrCreateHandle(pfontName, matchFontFlags, &pfoundFont, searchInfo);

    if (!phandle)
    {
        // if Bold or/and Italic was not found, search for a regular font to 
        // make faux bold/italic
        if (matchFontFlags & Font::FF_BoldItalic)
        {
            unsigned newMatchFontFlags = matchFontFlags & (~Font::FF_BoldItalic);
            if (searchInfo)
            {
                searchInfo->Indent++;
                AddSearchInfo(searchInfo, "Searching for font: \"", pfontName, "\" " , newMatchFontFlags, "");
            }
            Ptr<FontHandle> pplainHandle = *FindOrCreateHandle(pfontName, newMatchFontFlags, NULL, searchInfo);
            if (pplainHandle)
            {
                // copy the handle, modify with faux bold/italic flags and add
                // it to a table.
                phandle = SF_NEW FontHandle(*pplainHandle);
                phandle->OverridenFontFlags |= (matchFontFlags & Font::FF_BoldItalic);
                
                // If font handle was successfully created, add it to list and return.
                if (!searchInfo)
                {
                    SF_ASSERT(CreatedFonts.Get(phandle) == 0);
                    CreatedFonts.Add(phandle);
                }
                else
                    AddSearchInfo(searchInfo, "Font \"", pfontName, "\" ", matchFontFlags, " will be generated from \"", pfontName, "\"", newMatchFontFlags);
            }
        }
    }

    // if handle is not created, but the original font (empty) was found - 
    // use it.
    if (searchInfo)
        searchInfo->Indent = saveIndent;
    if (!phandle && pfoundFont)
    {
        AddSearchInfo(searchInfo, "Empty font: \"", pfontName, "\" is created");
        phandle = SF_NEW FontHandle(searchInfo? NULL : this, pfoundFont->GetFont(), pfontName);
        if (!searchInfo)
        {
            SF_ASSERT(CreatedFonts.Get(phandle) == 0);
            CreatedFonts.Add(phandle);
        }
    }
    if(!phandle)
        AddSearchInfo(searchInfo, "Font not found.");
    return phandle;

}

#ifndef SF_NO_IME_SUPPORT
void FontManager::SetIMECandidateFont(FontHandle* pfont)
{
    pIMECandidateFont = pfont;
}
#endif //#ifdef SF_NO_IME_SUPPORT

/*
FontHandle* FontManager::CreateAnyFontHandle(const char* pfontName, FontSearchPathInfo* searchInfo) 
{
    FontHandle* pfontHandle = CreateFontHandle(pfontName, 0, searchInfo);
    if (!pfontHandle)
    {
        pEmptyFont->AddRef();
        AddSearchInfo(searchInfo, "Returning \"EmptyFont\".");
        return pEmptyFont;
    }
    return pfontHandle;    
}
*/

FontHandle* FontManager::GetEmptyFont()
{
    pEmptyFont->AddRef();
    return pEmptyFont;
}

void FontManager::CleanCache()
{
    for(FontSet::Iterator it = CreatedFonts.Begin(); it != CreatedFonts.End(); ++it)
    {
        FontHandle* phandle = it->pNode;
        phandle->pFontManager = NULL;
    }
    CreatedFonts.Clear();
}

void FontManager::CleanCacheFor(MovieDefImpl* pdefImpl)
{
    for(FontSet::Iterator it = CreatedFonts.Begin(); it != CreatedFonts.End(); ++it)
    {
        FontHandle* phandle = it->pNode;
        if (phandle->pSourceMovieDef == pdefImpl)
        {
            phandle->pFontManager = NULL;
            it.Remove();
        }
    }
}


}} // namespace Scaleform::GFx
