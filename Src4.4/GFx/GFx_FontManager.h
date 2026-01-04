/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_FontManager.h
Content     :   Font manager functionality
Created     :   May 18, 2007
Authors     :   Artyom Bolgar, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_FONTMANAGER_H
#define INC_SF_GFX_FONTMANAGER_H

#include "Kernel/SF_UTF8Util.h"
#include "Kernel/SF_Array.h"
#include "GFx/GFx_FontResource.h"
#include "GFx/GFx_StringHash.h"
#include "GFx/GFx_FontLib.h"
#include "Render/Text/Text_FontManager.h"

namespace Scaleform { namespace GFx {

class FontManager;
class MovieDefImpl;
class MovieImpl;

class FontHandle : public Render::Text::FontHandle
{
    friend class FontManager;    

public:
    // A strong pointer to MovieDefImpl from which the font was created.
    // Font manager needs to hold these pointers for those fonts which came
    // from the FontProvider, since our instance can be the only reference
    // to them. Null for all other cases.
    Ptr<MovieDef>   pSourceMovieDef;

    FontHandle(Render::Text::FontManagerBase *pmanager, Font *pfont,
               const char* pfontName = 0, unsigned overridenFontFlags = 0,
               MovieDef* pdefImpl = 0)
        : Render::Text::FontHandle(pmanager, pfont, pfontName, overridenFontFlags),
          pSourceMovieDef(pdefImpl)
    {
    }
    FontHandle(const FontHandle& f) : Render::Text::FontHandle(f),
        pSourceMovieDef(f.pSourceMovieDef)
    {
    }
    inline ~FontHandle();

    bool operator==(const FontHandle& f) const
    {
        return Render::Text::FontHandle::operator==(f) && 
               pSourceMovieDef == f.pSourceMovieDef;
    }
};


class FontManagerStates : public RefCountBaseNTS<FontManagerStates, StatMV_Other_Mem>, public StateBag
{
public:
    FontManagerStates(StateBag* pdelegate) : pDelegate(pdelegate) {}

    virtual StateBag*   GetStateBagImpl() const { return pDelegate; }

    virtual State*      GetStateAddRef(State::StateType state) const;

    UInt8               CheckStateChange(FontLib*, FontMap*, FontProvider*, Translator*);

    Ptr<FontLib>      pFontLib;
    Ptr<FontMap>      pFontMap;
    Ptr<FontProvider> pFontProvider;
    Ptr<Translator>   pTranslator;

private:
    StateBag*       pDelegate;
};

class FontManager : public Render::Text::FontManagerBase
{
    friend class FontHandle;    
public:
    // Make a hash-set entry that tracks font handle nodes.
    struct FontKey 
    {
        const char*    pFontName;
        unsigned       FontStyle;

        FontKey() : FontStyle(0) {}
        FontKey(const char* pfontName, unsigned style)
            : pFontName(pfontName), FontStyle(style) { }

        static UPInt CalcHashCode(const char* pname, UPInt namelen, unsigned fontFlags)
        {
            UPInt hash = String::BernsteinHashFunctionCIS(pname, namelen);
            return (hash ^ (fontFlags & Font::FF_Style_Mask));
        }
    };
    struct NodePtr
    {
        FontHandle* pNode;

        // This flag is used if pNode represents a device font. 
        // It indicates if search for non-device font was performed or not.
        // If not, and the node with device font was found in CreatedFonts hash,
        // the FontManager will try to search for non-device font. Such situation is
        // possible if the font was cached as device font first and when 
        // non-device font was requested with the same style and name: need to check
        // if non-device font exists.
        mutable bool SearchedForNonDeviceFont;

        NodePtr() { }
        NodePtr(FontHandle* pnode) : pNode(pnode), SearchedForNonDeviceFont(false) { }
        NodePtr(const NodePtr& other) : pNode(other.pNode),
            SearchedForNonDeviceFont(other.SearchedForNonDeviceFont) { }

        // Two nodes are identical if their fonts match.
        bool operator == (const NodePtr& other) const
        {
            if (pNode == other.pNode)
                return 1;
            // For node comparisons we have to handle DeviceFont strictly ALL the time,
            // because it is possible to have BOTH versions of font it the hash table.
            enum { FontFlagsMask = Font::FF_CreateFont_Mask | Font::FF_DeviceFont | Font::FF_BoldItalic};
            unsigned ourFlags   = pNode->GetFontFlags() & FontFlagsMask;
            unsigned otherFlags = other.pNode->GetFontFlags() & FontFlagsMask;
            return ((ourFlags == otherFlags) &&
                !String::CompareNoCase(pNode->GetFontName(), other.pNode->GetFontName()));
        }        

        // Key search uses MatchFont to ensure that DeviceFont flag is handled correctly.
        bool operator == (const FontKey &key) const
        {
            return (Font::MatchFontFlags_Static(pNode->GetFontFlags(), key.FontStyle) &&
                !String::CompareNoCase(pNode->GetFontName(), key.pFontName));
        }
    };

    struct NodePtrHashOp
    {                
        // Hash code is computed based on a state key.
        UPInt  operator() (const NodePtr& other) const
        {            
            FontHandle* pnode = other.pNode;
            const char* pfontName = pnode->GetFontName();
            return (UPInt) FontKey::CalcHashCode(pfontName, SFstrlen(pfontName), 
                pnode->GetFont()->GetFontFlags() | pnode->GetFontStyle());
        }
        UPInt  operator() (const FontHandle* pnode) const
        {
            SF_ASSERT(pnode != 0);            
            const char* pfontName = pnode->GetFontName();
            return (UPInt) FontKey::CalcHashCode(pfontName, SFstrlen(pfontName), 
                pnode->GetFont()->GetFontFlags() | pnode->GetFontStyle());
        }
        UPInt  operator() (const FontKey& data) const
        {
            return (UPInt) FontKey::CalcHashCode(data.pFontName, SFstrlen(data.pFontName), data.FontStyle);
        }
    };

    // State hash
    typedef HashSetLH<NodePtr, NodePtrHashOp, NodePtrHashOp> FontSet;
protected:

    // Keep a hash of all allocated nodes, so that we can find one when necessary.
    // Nodes automatically remove themselves when all their references die.
    FontSet                 CreatedFonts;

    // A list of MovieDataDef pointers to movies
    // that serve as a source for fonts
    Array<Ptr<MovieDataDef> >  FontMovies;

    // MovieDefImpl which this font manager is associated with. We look up
    // fonts here by default before considering pFontProvider. Note that this
    // behavior changes if device flags are passed.
    MovieDefImpl*           pDefImpl;
    // pWeakLib might be set and used in the case if pDefImpl == NULL
    ResourceWeakLib*        pWeakLib;
    // Shared state for the instance, comes from GFxMovieImpl. Shouldn't need 
    // to AddRef since nested sprite (and font manager) should die before MovieRoot.    
    FontManagerStates*      pState;


    // Empty fake font returned if requested font is not found.
    Ptr<FontHandle>         pEmptyFont; 

    // FontMap Entry temporary; here to avoid extra constructor/destructor calls.
    FontMap::MapEntry       FontMapEntry;

    // a pointer back to movieImpl to search within registered fonts.
    MovieImpl*              pMovie;

#ifndef SF_NO_IME_SUPPORT
    Ptr<FontHandle>      pIMECandidateFont;
#endif //#ifdef SF_NO_IME_SUPPORT
    
    FontHandle*             FindOrCreateHandle(const char* pfontName, unsigned matchFontFlags, 
                                                FontResource** ppfoundFont, FontSearchPathInfo* searchInfo);

    FontHandle*             CreateFontHandleFromName(const char* pfontName, unsigned matchFontFlags, 
                                                      FontSearchPathInfo* searchInfo);
private:
    void commonInit();
public:
    FontManager(MovieImpl* movie, MovieDefImpl *pdefImpl, FontManagerStates* pState);
    FontManager(ResourceWeakLib *pweakLib, FontManagerStates* pState);
    ~FontManager();


    // Returns font by name and style. If a non-NULL searchInfo passed to the method
    // it assumes that it was called only for a diagnostic purpose and in this case font
    // will not be searched in the internal cache and a created font handle will not be cached.
    virtual FontHandle*     CreateFontHandle(const char* pfontName, unsigned matchFontFlags, 
                                             bool allowListOfFonts = true, FontSearchPathInfo* searchInfo = NULL); 

	// Helper function to remove handle from CreatedFonts when it dies.
	void                        RemoveFontHandle(FontHandle *phandle);

    // Returns any font with the font name 'pfontName' or the first one
    // in the hash.
    //FontHandle*          CreateAnyFontHandle(const char* pfontName, FontSearchPathInfo* searchInfo = NULL);

    virtual FontHandle*     GetEmptyFont();

    // Clean internal cache. This method is called from Advance method of 
    // MovieRoot then it detects that FontLib, FontMap, FontProvider or Translator 
    // is changed
    virtual void            CleanCache();
    void                    CleanCacheFor(MovieDefImpl* pdefImpl);

    void                    SetIMECandidateFont(FontHandle* pfont);

    MovieDefImpl*           GetDefImpl() const { return pDefImpl; }
#if defined(SF_BUILD_DEBUG) || defined(SF_BUILD_DEBUGOPT)
    // Required for AS3::ObjectCollector only. Don't use for anything else!
    const FontSet&          GetCreatedFonts() const { return CreatedFonts; }
#endif
};


// Font handle inlined after font manager because it uses manager's APIs.
inline FontHandle::~FontHandle()
{        
    // Remove from hash.
    if (pFontManager)
        static_cast<FontManager*>(pFontManager)->RemoveFontHandle(this);

    // Must release pFont before its containing MovieDefImpl destructor,
    // since otherwise we'd be accessing a dying heap.
    pFont = 0;
}

}} // namespace Scaleform::GFx

#endif //INC_SF_GFX_FONTMANAGER_H
