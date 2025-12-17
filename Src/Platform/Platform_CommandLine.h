/**************************************************************************

Filename    :   Platform_CommandLine.h
Content     :   Command line argument support through Args class.
Created     :   Feb 18, 2008
Authors     :   Dmitry Polenur, Maxim Didenko

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Platform_CommandLine_H
#define INC_SF_Platform_CommandLine_H

#if defined(SF_OS_WINMETRO)
	#define Platform SFPlatform
#endif

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_String.h"
#include "Kernel/SF_StringHash.h"

namespace Scaleform { namespace Platform {


//------------------------------------------------------------------------

class ArgValue :  public RefCountBase<ArgValue, Stat_Default_Mem>
{
public:
    ArgValue() : Valid(false) {}
    virtual ~ArgValue();

    bool            IsValue() const { return Valid; }

    virtual int     GetInt(unsigned = 0) const = 0;
    virtual float   GetFloat(unsigned = 0) const = 0;
    virtual bool    GetBool(unsigned = 0) const = 0;
    virtual String  GetString(unsigned = 0)  const = 0;

    // Used in ValueList, for other ArgValue types it will return 1 if value is set and 0 otherwise,
    // so scalar values can be used as list with one element
    virtual UPInt   GetListSize() const { return (UPInt)Valid; }

protected:
    bool Valid;
};

//------------------------------------------------------------------------

struct ArgDesc
{  
    const char* pName;
    const char* pLongName;
    unsigned    Flags;
    const char* pDefaultValue;
    const char* pHelpString;
};


//------------------------------------------------------------------------
// ***** Args

// Args class describes a full set of arguments read in from command line.
// It also maintains a set of argument descriptions used to interpret the
// arguments and look them up based on a name.
//
// The class is used as follows:
//  1. A set of argument descriptions are added by calling AddDesriptions.
//  2. Command line is parsed by calling ParseCommandLine.
//  3. Named and/or indexed argument values are accessed through GetValue,
//     GetString, GetInt and similar accessors.

class Args
{
public:
    enum ArgDescFlags
    {
        ArgEnd             = 0x0000, // Used for the last option
        ArgRequred         = 0x0001,
        ArgNone            = 0x0002,
        ArgList            = 0x0004,

        BoolValue           = 0x0010,
        IntValue            = 0x0020,
        FloatValue          = 0x0040,
        StringValue         = 0x0080,

        Flag                = ArgNone    | BoolValue,
        IntOption           = ArgRequred | IntValue,
        FloatOption         = ArgRequred | FloatValue,
        StringOption        = ArgRequred | StringValue,

        StringList          = ArgList | StringValue,

        Mandatory           = 0x0100,
        Positional          = 0x0200,

        Spacer              = 0x0400, // Do not create value, used just to print help
    };

    // *** Initialization

    void AddDesriptions(const ArgDesc* pargDescs);

    void PrintOptionsHelp() const;
    void PrintArgValues() const;

    enum ParseResult
    {
        Parse_Success,
        Parse_Help,
        Parse_Error
    };

    ParseResult   ParseCommandLine(int argc, char* argv[]);

    
    // *** Value / Argument access

    bool          HasValue(const char* name)    const;

    Ptr<ArgValue> GetValue(const char* name)    const; 
    Ptr<ArgValue> operator[] (const char* name) const { return GetValue(name); }

    // If value does not exist SF_ASSERT will be triggered (misspelled name is the most likely cause) 
    int         GetInt(const char* name, int idx = 0) const     { SF_ASSERT(GetValue(name)); return GetValue(name)->GetInt(idx); }
    bool        GetBool(const char* name, int idx = 0) const    { SF_ASSERT(GetValue(name)); return GetValue(name)->GetBool(idx); }
    String      GetString(const char* name, int idx = 0) const  { SF_ASSERT(GetValue(name)); return GetValue(name)->GetString(idx); }
    float       GetFloat(const char* name, int idx = 0) const   { SF_ASSERT(GetValue(name)); return GetValue(name)->GetFloat(idx); }

    // Used only in FxValueList, SF_ASSERT will be triggered for other types
    UPInt       GetListSize(const char* name) const;

private:
    struct HashValue
    {
        HashValue(UPInt argIdx = 0, UPInt valueIdx = 0)
            : ArgIdx(argIdx), ValueIdx(valueIdx) { }
        UPInt  ArgIdx;
        UPInt  ValueIdx;
    };

    Array<ArgDesc>          Descs;
    Array<Ptr<ArgValue> >   Values;
    StringHash<HashValue>   IdxValues;
    Array<HashValue>        PosValues;
};


}} // Scaleform::Platform

#endif // INC_SF_Platform_CommandLine_H
