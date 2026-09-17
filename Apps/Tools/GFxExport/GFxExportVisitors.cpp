
#include "GFxExportVisitors.h"
#include "GFxExport.h"
#include "GFx/GFx_MovieDef.h"
#include "../../../Src/GFx/GFx_TextFieldDef.h"

// Helper function which strips tags from HTML.
void StripHtmlTags(String *presult, const char *ptext)
{
    UInt32 code;
    presult->Clear();

    while ( (code = UTF8Util::DecodeNextChar(&ptext))!=0 )
    {
        // Skip HTML markup

        // Ampersand
        if (code == '&')
        {
            // &lt;
            // &gt;
            // &amp;
            // &quot;
            code = UTF8Util::DecodeNextChar(&ptext);
            UInt32 decoded = 0;
            switch(code)
            {
            case 'l': decoded = '<'; break;
            case 'g': decoded = '>'; break;
            case 'a': decoded = '&'; break;
            case 'q': decoded = '\"'; break;
            }
            // Skip past ';'
            while (code && ((code = UTF8Util::DecodeNextChar(&ptext)) != ';'))
                ;
            // convert code and proceeded
            code = decoded;                 
        }

        else if (code == '<')
        {
            while ((code = UTF8Util::DecodeNextChar(&ptext))!=0)
            {
                if (code == '>')
                    break;
            }
            continue;
        }

        // Append to string.
        presult->AppendString((const wchar_t*)&code, 1);
    }           
}


void GFxImageExtractorVisitor::Visit(MovieDef* pmovieDef, Resource* presource,
                                     ResourceId rid, const char* pexportName)

{
    SF_UNUSED(pmovieDef);

	Ptr<Render::ImageBase> pimage;
    if (presource->GetResourceUse() == Resource::Use_Bitmap)
    {
        pimage = static_cast<ImageResource*>(presource)->GetImage();
        SF_ASSERT(pimage);

        Exporter.ExtractImage(pimage, rid, Name, DestPath, pexportName, static_cast<ImageResource*>(presource));
    }
}

void GFxFontsVisitor::Visit(MovieDef*, Resource* presource, ResourceId rid, const char*)
{
    if (presource->GetResourceType() == Resource::RT_Font)
    {
        FontResource* pfontResource = static_cast<FontResource*>(presource);
        int fontId = rid.GetIdValue();

        pExporter->ExtractFont(pfontResource, fontId, pResBinding);
    }
}

void GFxFontNameVisitor::Visit(MovieDef*, Resource* presource, ResourceId rid, const char*)
{
    FontResource* pfontResource = static_cast<FontResource*>(presource);

    char buf[100];
    String font;
    font = pfontResource->GetName();
    if (pfontResource->IsBold())
    {
        font += " - Bold";
    }
    else if (pfontResource->IsItalic())
    {
        font += " - Italic";
    }
    SFsprintf(buf, sizeof(buf), ", %d glyphs", pfontResource->GetGlyphShapeCount()); 
    font += buf;
    if (!pfontResource->HasLayout())
    {
        font += ", static only";
    }
    font += " (";
    rid.GenerateIdString(buf);
    font += buf;
    font += ")";

    Fonts.PushBack(font);
}

void GFxFontTexturesVisitor::Visit(ResourceId textureResId, ImageResource* pimageRes)
{
    UInt32 textureId = (textureResId.GetIdValue() & (~ResourceId::IdType_Bit_TypeMask)) | ResourceId::IdType_FontImage;
    FontTexturesHashType::Iterator iter = pTextures->Find(textureId);
    GFxFontTextureDescr::FontDescr fontDescr;
    fontDescr.FontId = FontId;
    fontDescr.pFont  = pFont;
    if (iter != pTextures->End())
    {
        // just add font to already existing GFxFontTextureDescr
        iter->Second.Fonts.PushBack(fontDescr);
    }
    else
    {
        GFxFontTextureDescr ftDescr;
        ftDescr.Fonts.PushBack(fontDescr);
        ftDescr.pTexture = pimageRes->GetImage();
        pTextures->Add(textureId, ftDescr);
    }
}


void GFxTextureGlyphVisitor::Visit(unsigned index, GFx::TextureGlyph* ptextureGlyph)
{
    if (ptextureGlyph->pImage == TextureDescr.pTexture->GetAsImage())
    {
        unsigned indexInTexture = (unsigned)TextureDescr.TextureGlyphs.GetSize();
        TextureDescr.TextureGlyphs.PushBack(ptextureGlyph);

        FntDescr.TexGlyphsIndicesMap.PushBack(GFxFontTextureDescr::FontDescr::Pair(index, indexInTexture));
        ++NumGlyphsInFont;
    }
}

void GFxTextFieldVisitor::Visit(MovieDef*, Resource* presource, ResourceId rid, const char*)
{
    if (rid == ResourceId(CharacterDef::CharId_EmptyTextField))
        return;
    TextFieldDef* dyntxtresource = (TextFieldDef*)presource;

    String tmp;
    if (dyntxtresource->IsHtml())
    {
        String tmp1;
        StripHtmlTags(&tmp1, dyntxtresource->DefaultText.ToCStr());
        tmp += tmp1;
    }
    else
    {
        tmp += dyntxtresource->DefaultText.ToCStr();
    }
    String tmp2;
    tmp2 = "\"";
    tmp2 += tmp;
    tmp2 += "\" - ";

    char buf[100];
    dyntxtresource->FontId.GenerateIdString(buf);
    tmp2 += buf;
    AddUniqueTextToArray(tmp2);
}

void GFxTextFieldVisitor::AddUniqueTextToArray(const String& txt)
{
    // find the string in the array
    for (unsigned i=0; i < Text.GetSize(); i++)
    {
        if (txt.CompareNoCase(Text[i]) == 0)
            return;
    }
    Text.PushBack(txt);
}


void GFxEditTextFieldVisitor::Visit(MovieDef*, Resource* presource, ResourceId, const char*)
{
    TextFieldDef* dyntxtresource = (TextFieldDef*)presource;

    if (!DynOnly || dyntxtresource->IsReadOnly())
    {

        // TODO: Only add unique text

        if (dyntxtresource->IsHtml())
        {
            String tmp;
            StripHtmlTags(&tmp, dyntxtresource->DefaultText.ToCStr());
            AddUniqueTextToArray(tmp);
        }
        else
        {
            AddUniqueTextToArray(dyntxtresource->DefaultText);
        }
    }
}

void GFxEditTextFieldVisitor::AddUniqueTextToArray(const String& txt)
{
    // find the string in the array
    for (unsigned i=0; i < Text.GetSize(); i++)
    {
        if (txt.CompareNoCase(Text[i]) == 0)
            return;
    }
    Text.PushBack(txt);
}


void GFxImageSoundCounterVisitor::Visit(MovieDef*, Resource* presource, ResourceId, const char*)
{
    if (presource->GetResourceType() == Resource::RT_Image && 
        presource->GetResourceUse()  == Resource::Use_Bitmap)
        ++ImageCount;
    if (presource->GetResourceType() == Resource::RT_SoundSample)
        ++SoundCount;
}


#ifdef GFX_ENABLE_SOUND
void GFxSoundExtractorVisitor::Visit(MovieDef* pmovieDef, Resource* presource,
                                     ResourceId rid, const char* pexportName)
{
    SF_UNUSED(pmovieDef);
    Ptr<Sound::SoundInfoBase> psoundInfo = static_cast<SoundResource*>(presource)->GetSoundInfo();
    Exporter.ExtractSound(psoundInfo, rid, Name, DestPath, pexportName);
}


void GFxSpriteSoundExtractorVisitor::Visit(MovieDef* pmovieDef, Resource* presource,
                                           ResourceId rid, const char* pexportName)
{
    SoundStreamDef* pstreamSound = NULL;
    if (rid == CharacterDef::CharId_EmptyMovieClip)
    {
        Scaleform::GFx::MovieDefImpl* pdefImpl = static_cast<Scaleform::GFx::MovieDefImpl*>(pmovieDef);
        pstreamSound = pdefImpl->GetDataDef()->GetSoundStream();
    }
    else 
    {
        TimelineDef* ptimelineDef = static_cast<TimelineDef*>(presource);
        pstreamSound = ptimelineDef->GetSoundStream();
    }
    if(pstreamSound )
        Exporter.ExtractSpriteSound(pstreamSound,rid,Name, DestPath, pexportName);
}
#endif
