/**************************************************************************

Filename    :   FontConfigParser.h
Content     :   Parsing logic for GFxPlayer font configuration file
Created     :
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_FontConfigParser_H
#define INC_FontConfigParser_H

#include "GFx_Kernel.h"
#include "GFx.h"

// ***** Font configuration settings
namespace Scaleform { namespace GFx {
    
class ConfigParser
{
    WStringBuffer Buffer;

    int     Pos;
    unsigned    Line;

    inline void SkipWhitespace()
    {
        while ((Pos < (int)Buffer.GetLength()) && 
            ((Buffer[Pos] == L' ') || (Buffer[Pos] == L'\t')))
            Pos++;
    }

    static inline bool IsFirstIdChar(const wchar_t c)
    {
        return ((c >= L'a') && (c <= L'z')) ||
            ((c >= L'A') && (c <= L'Z')) ||
            (c == L'_');
    }

    static inline bool IsNextIdChar(const wchar_t c)
    {
        return IsFirstIdChar(c) ||
            ((c >= L'0') && (c <= L'9'));
    }
    static inline bool IsNumberChar(const wchar_t c)
    {
        return ((c >= L'0') && (c <= L'9')) || c == L'.';
    }

public:

    String ParentPath;

    ConfigParser(const char* pfilename);
    ~ConfigParser();

    bool    IsValid() const { return Buffer.GetLength() != 0; }


    enum TokenType
    {
        // Order relied on by TokenizeLine.
        Tok_Error,  // Error
        Tok_EOL,    // End of line
        Tok_EOF,    // End of file

        Tok_Char,   // Control character
        Tok_String, // Quoted string
        Tok_Id,     // Identifier
        Tok_Number  // Number
    };

    struct Token
    {
        TokenType   Type;
        const wchar_t* pData;
        unsigned    Length;
        const char* pError;

        // *** Initialization
        TokenType        Set(TokenType t, const wchar_t* pdata = 0, unsigned length = 0)
        {
            pError = "";
            Type = t;
            pData = pdata;
            Length = length;
            return t;
        }

        // *** State/Type query

        String   GetString() const   
        { 
            if (pData)
            {
                String str;

                // Strip newlines because Flash multiline text uses \r instead of \n
                for (unsigned i=0; i < Length; i++)
                {
                    if (pData[i] != '\n')
                        str.AppendChar(pData[i]);
                }

                return str;
            }
            else
            {
                return String(); 
            }
        }
        
        wchar_t        GetChar() const     { return pData ? pData[0] : 0; }

        float          GetNumber() const
        {
            String s1;
            s1.AppendString(pData, Length);
            return (float)atof(s1.ToCStr());
        }

        bool        IsId() const        { return (Type == Tok_Id); }
        bool        IsString() const    { return (Type == Tok_String); }
        bool        IsNumber() const    { return (Type == Tok_Number); }

        bool        IsId(const wchar_t* pid) const
        {
            String s1, s2;
            s1.AppendString(pData, Length);
            s2.AppendString(pid);
            return ((Type == Tok_Id) &&
                Length == s2.GetLength() &&
                !s1.CompareNoCase(s2));
        }

        bool        IsChar(const wchar_t c) const
        {
            return ((Type == Tok_Char) && GetChar() == c);
        }

    };

    // Reads next token and returns its type. Used by TokenizeLine
    TokenType   NextToken(Token* ptoken);

    struct TokenLine
    {
        // Tokens for the line, including new line in the end
        Array<Token>      Tokens;
        // Index of Tok_Error, if any. -1 for no errors.
        int                ErrorIndex;
        // Line index that we tokenized.
        unsigned           LineIndex;

        bool    HasErrors() { return ErrorIndex != -1; }
    };

    // Tokenize a line, returning 1 if successful and 0 for eof.
    bool        TokenizeLine(TokenLine* pline, unsigned max);

    // Matches a token array based on format signature, returns 1 if match found 0 otherwise.
    static bool MatchLine(Array<Token> &tokens, const wchar_t* pformat, ...);
};


/* *** Font configuration file structure:

[FontConfig "English"]
fontlib "fonts_english1.swf"
fontlib "fonts_english2.swf"

[FontConfig "Chinese"]
fontlib "fonts_chinese.swf"
map "Arial" = "Chinese Font Name"

*/


class FontTranslator : public Translator
{
public:    
    typedef Hash<String, String, String::HashFunctor> MappingHash;

    virtual unsigned    GetCaps() const         { return Cap_StripTrailingNewLines; }

    // Add a mapping between two strings
    void AddMapping(const String &key, const String &value);

    // Add a table of mappings.
    void AddMappings(const MappingHash& mappings);

    // Callback
    virtual void Translate(TranslateInfo* ptranslateInfo);
private:
    MappingHash Mappings;
};

class FontConfig : public RefCountBase<FontConfig, Stat_Default_Mem>
{
public:
    enum ProviderType
    {
        Provider_Undefined,
        Provider_None,
        Provider_Win32,
        Provider_FT2
    };

    String              ConfigName;
    ProviderType        Provider;

    Array<String>       FontLibFiles;
    Ptr<FontMap>        pFontMap;

    Ptr<FontTranslator> pTranslation;

    FontConfig()
    {
        Provider = Provider_Undefined;
        pTranslation = *new FontTranslator();
    }

    // Apply config settings to loader.
    void Apply(Loader* ploader);
    //void Apply(GFxMovie* pmovie, Loader* ploader);

};

class FontConfigSet
{
public:

    typedef ConfigParser::Token     Token;
    typedef ConfigParser::TokenLine TokenLine;

    Array<Ptr<FontConfig> >   Configs;

    FontConfigSet();
    
    // Parse font configuration set.
    void Parse(ConfigParser* pparser);

    // Array-style access
    unsigned GetSize() const { return (unsigned)Configs.GetSize(); }
    FontConfig* operator [] (unsigned index) { return Configs[index]; }
};

}}

#endif

