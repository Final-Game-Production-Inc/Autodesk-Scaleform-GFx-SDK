/**************************************************************************

PublicHeader:   GFx_FontProviderPSVita.h
Filename    :   GFx_FontProviderPSVita.h
Content     :
Created     :   2011
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_FontProviderPSVita_H
#define INC_SF_GFX_FontProviderPSVita_H

#include "Render/FontProvider/Render_FontProviderPSVita.h"
#include "GFx/GFx_Loader.h"

namespace Scaleform { namespace GFx {

typedef Render::ExternalFontPSVita ExternalFontPSVita;

//------------------------------------------------------------------------
// External font provider that supports libpvf. This provider has a number 
// of restriction due to the fact that there is no access to vector glyph shapes.
// 
// So that, the maximal glyph height cannot exceed GlyphCacheParams.MaxSlotHeight. 
// 
// The libpvf uses strange units, probably to be compatible with the older libpgf, 
// which makes it difficult to conform with the Flash font units with its traditional 
// 1024x1024 EM square. 
// 
// The only reasonable way is to tune 2 scaling coefficients when mapping the font:
// metricsScale = 1.5f, advanceScale = 0.5f
// These coefficients correspond well with the TrueType fonts opened by scePvfOpenUserFile, 
// but may need to be adjusted when using system fonts.
// The greater metricsScale is the lower the text will be positioned.
// The greater advanceScale is the bigger letter spacing will appear.
// 
// An example of mapping the TrueType fonts:
//
// String arial = String(GetDefaultFilePath())   + "arial.ttf";
// String times = String(GetDefaultFilePath())   + "times.ttf";
// String georgia = String(GetDefaultFilePath()) + "georgia.ttf";
// Ptr<GFx::FontProviderPSVita> fontProvider = *new GFx::FontProviderPSVita(8, 0);
//
// fontProvider->MapFontToFile("Arial", 0, arial);
// fontProvider->MapFontToFile("Times New Roman", 0, times);
// fontProvider->MapFontToFile("Georgia", 0, georgia);
//
// mLoader.SetFontProvider(fontProvider);
//
// The ld requires -lScePvf_stub
//------------------------------------------------------------------------

class FontProviderPSVita : public FontProvider
{
    typedef Render::Font Font;
    Render::FontProviderPSVita* getFontProvider();

public:
    FontProviderPSVita(unsigned numUserFontEntries=8, int firstId=0);
    FontProviderPSVita(MemoryHeap* heap, unsigned numUserFontEntries=8, int firstId=0);

    void MapSystemFont(const char* fontName, unsigned fontFlags, unsigned fontType, 
                       Render::PSVita_FontStreamType fontStream = Render::PSVita_FontStream_File,
                       float metricsScale = 1.5f, float advanceScale = 0.5f);

    void MapFontToFile(const char* fontName, unsigned fontFlags, const char* fileName, 
                       Render::PSVita_FontStreamType fontStream = Render::PSVita_FontStream_File,
                       float metricsScale = 1.5f, float advanceScale = 0.5f);

    void MapFontToMemory(const char* fontName, unsigned fontFlags, 
                         const char* fontData, unsigned dataSize, 
                         float metricsScale = 1.5f, float advanceScale = 0.5f);

    virtual void     LoadFontNames(StringHash<String>& fontnames);
};

}} // namespace Scaleform { namespace GFx {

#endif
