/**************************************************************************

Filename    :   FontConfigParser.cpp
Content     :   Parsing logic for GFxPlayer font configuration file
Created     :
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FontConfigParser.h"

#include "GFx.h"
#include "GFx_Kernel.h"
#include <stdarg.h>

namespace Scaleform { namespace GFx {
    
// ***** ConfigParser implementation

ConfigParser::ConfigParser(const char* pfilename)
{
    Pos         = 0;
    Line        = 1;

    // if absolute path, then save the parent path
    ParentPath.AppendString(pfilename);
    URLBuilder::ExtractFilePath(&ParentPath);
    if (ParentPath.CompareNoCase(pfilename) == 0)
    {
        ParentPath = "";
    } 

    SysFile file(pfilename);
    if (file.IsValid())
    {
        int flen = 0;
        if ((flen = file.GetLength()) != 0)
        {
            UByte* td = (UByte*) SF_ALLOC(flen, Stat_Default_Mem);
            file.Read(td, flen);

            // PPS: the following converts byte stream to appropriate endianness
            // PPS: only for UTF16
            UInt16* prefix16 = (UInt16*)td;

            if (prefix16[0] == Alg::ByteUtil::BEToSystem((UInt16)0xFFFE)) // little endian
            {
                prefix16++;
                int numChars = (flen / 2) - 1;
                if (sizeof(wchar_t) == 2)
                {
                    UInt16* chars = (UInt16*) SF_ALLOC(numChars * 2,Stat_Default_Mem);
                    Buffer.Resize(numChars);
                    for (int i=0; i < numChars; i++)
                        chars[i] = Alg::ByteUtil::LEToSystem(prefix16[i]);
                    Buffer.SetString( ((const wchar_t*)chars), numChars );
                    SF_FREE(chars);
                }
                else if (sizeof(wchar_t) == 4)
                {
                    UInt32* chars = (UInt32*) SF_ALLOC(numChars * 4, Stat_Default_Mem);
                    Buffer.Resize(numChars);
                    for (int i=0; i < numChars; i++)
                        chars[i] = Alg::ByteUtil::LEToSystem(prefix16[i]);
                    Buffer.SetString( ((const wchar_t*)chars), numChars );
                    SF_FREE(chars);
                }
                else
                {
					#ifndef SF_OS_WIIU
                    SF_DEBUG_ASSERT1(0, "Unsupported wchar_t size (%ld)\n", sizeof(wchar_t));
					#endif
                }
            }

            else if (prefix16[0] == Alg::ByteUtil::BEToSystem((UInt16)0xFEFF)) // big endian
            {
                prefix16++;
                int numChars = (flen / 2) - 1;
                if (sizeof(wchar_t) == 2)
                {
                    UInt16* chars = (UInt16*) SF_ALLOC(numChars * 2, Stat_Default_Mem);
                    Buffer.Resize(numChars);
                    for (int i=0; i < numChars; i++)
                        chars[i] = Alg::ByteUtil::BEToSystem(prefix16[i]);
                    Buffer.SetString( ((const wchar_t*)chars), numChars );
                    SF_FREE(chars);
                }
                else if (sizeof(wchar_t) == 4)
                {
                    UInt32* chars = (UInt32*) SF_ALLOC(numChars * 4, Stat_Default_Mem);
                    Buffer.Resize(numChars);
                    for (int i=0; i < numChars; i++)
                        chars[i] = Alg::ByteUtil::BEToSystem(prefix16[i]);
                    Buffer.SetString( ((const wchar_t*)chars), numChars );
                    SF_FREE(chars);
                }
                else
                {
					#ifndef SF_OS_WIIU
                    SF_DEBUG_ASSERT1(0, "Unsupported wchar_t size (%ld)\n", sizeof(wchar_t));
					#endif
                }            
            }

            else if (flen > 2 && td[0] == 0xEF && td[1] == 0xBB && td[2] == 0xBF)
            {
                // UTF-8 BOM was explicitly specified
                Buffer.SetString( (char*)td + 3, flen - 3);
            }

            else
            {
                // UTF-8 BOM not specified
                Buffer.SetString( (char*)td, flen - 3);
            }

            SF_FREE(td);
        }
    }
}

ConfigParser::~ConfigParser()
{
    
}


ConfigParser::TokenType ConfigParser::NextToken(Token* ptoken)
{
    if (!IsValid() || (Pos >= (int)Buffer.GetLength()))
        return ptoken->Set(Tok_EOF);

    SkipWhitespace();

    // Skip EOF byte, if any.
    if (Buffer[Pos] == 0x1A)
    {
        Pos++;
        if (Pos == (int)Buffer.GetLength())
            return ptoken->Set(Tok_EOF);
    }

    // Handle '\r\n' and just '\n' combinations.
    if ((Buffer[Pos] == '\r') && (Buffer[Pos+1] == '\n'))
    {
        Line++;
        Pos+=2;
        return ptoken->Set(Tok_EOL, &Buffer[Pos-2], 2);
    }

    if (Buffer[Pos] == '\n')
    {
        Line++;
        Pos++;
        return ptoken->Set(Tok_EOL, &Buffer[Pos-1], 1);
    }

    // Process identifiers.
    if (IsFirstIdChar(Buffer[Pos]))
    {
        wchar_t * pstart = &Buffer[Pos];
        unsigned   start = Pos;
        Pos++;

        while((Pos < (int)Buffer.GetLength()) && IsNextIdChar(Buffer[Pos]))
            Pos++;
        return ptoken->Set(Tok_Id, pstart, Pos-start);
    }

    if (IsNumberChar(Buffer[Pos]))
    {
        wchar_t * pstart = &Buffer[Pos];
        unsigned   start = Pos;
        bool sawDot = Buffer[Pos] == L'.';
        Pos++;

        while((Pos < (int)Buffer.GetLength()) && IsNumberChar(Buffer[Pos]))
        {
            if (Buffer[Pos] == L'.')
            {
                if (sawDot)
                    break;
                sawDot = true;
            }
            Pos++;
        }
        return ptoken->Set(Tok_Number, pstart, Pos-start);
    }
    // Process Quoted strings.
    if (Buffer[Pos] == '\"')
    {
        Pos++;
        wchar_t* pstart = &Buffer[Pos];
        unsigned   start  = Pos;

        while((Pos < (int)Buffer.GetLength()) &&
            (Buffer[Pos] != '\"') )
            Pos++;

        if (Buffer[Pos] == '\"')
        {
            Pos++;
            return ptoken->Set(Tok_String, pstart, Pos-start-1);
        }
        else
        {
            // Incorrect.
            ptoken->Set(Tok_Error);
            ptoken->pError = "closing quote \" character expected";
            return ptoken->Type;
        }
    }

    // Report other characters as Tok_Char.
    Pos++;
    return ptoken->Set(Tok_Char, &Buffer[Pos-1], 1);
}

bool    ConfigParser::TokenizeLine(TokenLine* ptokens, unsigned max)
{
    SF_UNUSED(max);
    Token tok;
    ptokens->ErrorIndex = -1;
    ptokens->LineIndex  = Line;
    ptokens->Tokens.Clear();
    
    do {
        NextToken(&tok);
        if ((tok.Type == Tok_Error) && (ptokens->ErrorIndex == -1))
            ptokens->ErrorIndex = (int)ptokens->Tokens.GetSize();
        ptokens->Tokens.PushBack(tok);
    } while(tok.Type > Tok_EOF);

    return (ptokens->Tokens.GetSize() > 1) || (tok.Type != Tok_EOF);
}

bool    ConfigParser::MatchLine(Array<Token> &tokens, const wchar_t* pformat, ...)
{
    va_list vl;
    va_start(vl, pformat);

    unsigned tokenIndex = 0;

    while(*pformat != 0)
    {
        // Must have tokens for format string.
        if (tokenIndex >= tokens.GetSize())
        {
            va_end(vl);
            return 0;
        }

        Token &tok = tokens[tokenIndex];

        switch (*pformat)
        {
        case '%':

            // %i = matching id
            if (*(pformat+1) == 'i')
            {
                if (!tok.IsId(va_arg(vl, const wchar_t*)))
                {
                    va_end(vl);
                    return 0;
                }
            }
            // %s feed back string
            else if (*(pformat+1) == 's')
            {
                if (!tok.IsString())
                {
                    va_end(vl);
                    return 0;
                }
                String* pstr = va_arg(vl, String*);
                *pstr = tok.GetString();
            }

            // %I feed back identifier
            else if (*(pformat+1) == 'I')
            {
                if (!tok.IsId())
                {
                    va_end(vl);
                    return 0;
                }
                String* pstr = va_arg(vl, String*);
                *pstr = tok.GetString();
            }

            // %n feed back number
            else if (*(pformat+1) == 'n')
            {
                if (!tok.IsNumber())
                {
                    va_end(vl);
                    return 0;
                }
                float* pnumber = va_arg(vl, float*);
                *pnumber = tok.GetNumber();
            }

            pformat++;
            break;

            // By default, token must match a character.
        default:
            if (!tok.IsChar(*pformat))
            {
                va_end(vl);
                return 0;
            }
        }

        // Go to next character and 
        pformat++;
        tokenIndex++;
    }

    va_end(vl);

    // Last token must be new line or error.
    if (tokenIndex >= tokens.GetSize())    
        return 0;
    Token &tok = tokens[tokenIndex];
    if ((tok.Type == Tok_EOL) || (tok.Type == Tok_EOF))
        return 1;

    // Errors at the end of line would yield parse error.
    return 0;
}


// ***** Font translation
//
void FontTranslator::AddMapping(const String &key, const String &value)
{
    Mappings.Add(key, value);
}

void FontTranslator::AddMappings(const MappingHash& mappings)
{
    for (MappingHash::ConstIterator it = mappings.Begin(); it != mappings.End(); ++it)
    {
        Mappings.Add(it->First, it->Second);
    }
}

void FontTranslator::Translate(TranslateInfo* ptranslateInfo)
{
    String key;
    key = ptranslateInfo->GetKey();
    String* pvalue = Mappings.Get(key);
    if (pvalue)
    {
        ptranslateInfo->SetResult(pvalue->ToCStr(), pvalue->GetSize());
    }
}


// ***** FontFonfig parsing

// Apply config settings to loader.
void FontConfig::Apply(Loader* ploader)
{
    ploader->SetFontLib(0);
    ploader->SetFontMap(pFontMap.GetPtr());

    // IMPORTANT: to force re-translation it is necessary to set a new 
    // Translator.
    ploader->SetTranslator(pTranslation);

    // Load font SWF/GFX files
    if (FontLibFiles.GetSize())
    {
        Ptr<FontLib> plib = *new FontLib;
        ploader->SetFontLib(plib);
        for(unsigned i=0; i<FontLibFiles.GetSize(); i++)
        {
            //plib->SetSubstitute(FontLibFiles[i].ToCStr()); //?
            Ptr<MovieDef> pdef = *ploader->CreateMovie(FontLibFiles[i]);
            if (pdef)
                plib->AddFontsFrom(pdef, 0);
        }
    }
   
}

FontConfigSet::FontConfigSet()
{
}

// Helper function used to decode font style
static FontMap::MapFontFlags UpdateFontFlags(FontMap::MapFontFlags flags, const String& symbol, unsigned lineIndex)
{    
    FontMap::MapFontFlags newFlags = FontMap::MFF_Original;

    if (!symbol.CompareNoCase("bold"))
        newFlags = FontMap::MFF_Bold;
    else if (!symbol.CompareNoCase("normal"))
        newFlags = FontMap::MFF_Normal;
    else if (!symbol.CompareNoCase("italic"))
        newFlags = FontMap::MFF_Italic;
    else if (!symbol.CompareNoCase("fauxbold"))
        newFlags = FontMap::MFF_FauxBold;
    else if (!symbol.CompareNoCase("fauxitalic"))
        newFlags = FontMap::MFF_FauxItalic;
    else if (!symbol.CompareNoCase("noautofit"))
        newFlags = FontMap::MFF_NoAutoFit;
    else
    {
        fprintf(stderr, "Warning: FontConfig(%d) - unknown map font style '%s'\n",
                        lineIndex, symbol.ToCStr());
        return flags;
    }

    if (flags == FontMap::MFF_Original)
    {
        flags = newFlags;
    }
    else if ( ((flags == FontMap::MFF_Normal) && ((newFlags & FontMap::MFF_BoldItalic) != 0)) ||
              ((newFlags == FontMap::MFF_Normal) && ((flags & FontMap::MFF_BoldItalic) != 0)) )
    {
        // Normal combined with incorrect flag.
        fprintf(stderr, "Warning: FontConfig(%d) - unexpected map font style '%s'\n",
                lineIndex, symbol.ToCStr());
    }
    else
    {
          flags = (FontMap::MapFontFlags) (newFlags | flags);
    }

    return flags;
}


void FontConfigSet::Parse(ConfigParser* pparser)
{
    // Process file with tokens    
    ConfigParser::TokenLine     line;
    Array<Token>&              tokens = line.Tokens;

    String s1, s2, s3, s4;

    Configs.Clear();
    if (!pparser->IsValid())
        return;

    while(pparser->TokenizeLine(&line, 6))
    {
        if (line.HasErrors())
        {
            fprintf(stderr, "Error: FontConfig(%d) - %s\n",
                             line.LineIndex, tokens[line.ErrorIndex].pError);
            break;            
        }

        // [FontConfig] - unnamed
        if (ConfigParser::MatchLine(tokens, L"[%i]", L"FontConfig"))
        {
            Ptr<FontConfig> pfontConfig = *new FontConfig;
            pfontConfig->ConfigName = "Unnamed";
            Configs.PushBack(pfontConfig);
        }
        // [FontConfig "English"]
        else if (ConfigParser::MatchLine(tokens, L"[%i%s]", L"FontConfig", &s1))
        {
            Ptr<FontConfig> pfontConfig = *new FontConfig;
            pfontConfig->ConfigName = s1;
            Configs.PushBack(pfontConfig);
        }

        else
        {
            // Empty lines ok.
            if (ConfigParser::MatchLine(tokens, L""))
                continue;

            if (Configs.GetSize() == 0)
            {
                fprintf(stderr, "Error: FontConfig(%d) - file mappings outside of [FontConfig] section\n",
                                line.LineIndex);
                break;
            }

            FontConfig* pconfig = Configs[Configs.GetSize()-1];
            SF_ASSERT(pconfig);
            float scaleFactor = 1.0f;

            // map "Arial" = "takoma"
            if (ConfigParser::MatchLine(tokens, L"%i%s=%s", L"map", &s1, &s2))
            {
                if (!pconfig->pFontMap)                
                    pconfig->pFontMap = *new FontMap;
                pconfig->pFontMap->MapFont(s1.ToCStr(), s2.ToCStr());
            }

            // map "Arial" = "takoma" Bold
            else if (ConfigParser::MatchLine(tokens, L"%i%s=%s%I", L"map", &s1, &s2, &s3))
            {
                if (!pconfig->pFontMap)                
                    pconfig->pFontMap = *new FontMap;

                FontMap::MapFontFlags mapFlags =
                        UpdateFontFlags(FontMap::MFF_Original, s3, line.LineIndex);
                pconfig->pFontMap->MapFont(s1.ToCStr(), s2.ToCStr(), mapFlags);
            }
            // map "Arial" = "takoma" 1.3
            else if (ConfigParser::MatchLine(tokens, L"%i%s=%s%n", L"map", &s1, &s2, &scaleFactor))
            {
                if (!pconfig->pFontMap)                
                    pconfig->pFontMap = *new FontMap;
                pconfig->pFontMap->MapFont(s1.ToCStr(), s2.ToCStr(), FontMap::MFF_Original, scaleFactor);
            }

            // map "Arial" = "takoma" Bold 1.3
            else if (ConfigParser::MatchLine(tokens, L"%i%s=%s%I%n", L"map", &s1, &s2, &s3, &scaleFactor))
            {
                if (!pconfig->pFontMap)                
                    pconfig->pFontMap = *new FontMap;

                FontMap::MapFontFlags mapFlags =
                    UpdateFontFlags(FontMap::MFF_Original, s3, line.LineIndex);
                pconfig->pFontMap->MapFont(s1.ToCStr(), s2.ToCStr(), mapFlags, scaleFactor);
            }

            // map "Arial" = "takoma" Bold Italic
            else if (ConfigParser::MatchLine(tokens, L"%i%s=%s%I%I", L"map", &s1, &s2, &s3, &s4))
            {
                if (!pconfig->pFontMap)                
                    pconfig->pFontMap = *new FontMap;

                FontMap::MapFontFlags mapFlags =
                           UpdateFontFlags(FontMap::MFF_Original, s3, line.LineIndex);
                mapFlags = UpdateFontFlags(mapFlags, s4, line.LineIndex);
                pconfig->pFontMap->MapFont(s1.ToCStr(), s2.ToCStr(), mapFlags);
            }
            // map "Arial" = "takoma" Bold Italic 1.3
            else if (ConfigParser::MatchLine(tokens, L"%i%s=%s%I%I%n", L"map", &s1, &s2, &s3, &s4, &scaleFactor))
            {
                if (!pconfig->pFontMap)                
                    pconfig->pFontMap = *new FontMap;

                FontMap::MapFontFlags mapFlags =
                    UpdateFontFlags(FontMap::MFF_Original, s3, line.LineIndex);
                mapFlags = UpdateFontFlags(mapFlags, s4, line.LineIndex);
                pconfig->pFontMap->MapFont(s1.ToCStr(), s2.ToCStr(), mapFlags, scaleFactor);
            }

            // fontlib "myfile.swf"
            else if (ConfigParser::MatchLine(tokens, L"%i%s", L"fontlib", &s1))
            {
                // prepend parent path to the filename.
                String path( pparser->ParentPath );
                path += s1;
                pconfig->FontLibFiles.PushBack(path);
            }

            // tr "Good morning" = "Guten Morgen"
            else if (ConfigParser::MatchLine(tokens, L"%i%s=%s", L"tr", &s1, &s2))
            {

                pconfig->pTranslation->AddMapping(s1, s2);
            }            

            else
            {
                // Unsupported line
                fprintf(stderr, "Error: FontConfig(%d) - unexpected statement\n",
                                line.LineIndex);
                break;
            }
        }       
    }
}

}} // Scaleform::GFx

