/**************************************************************************

Filename    :   FxSettings.h
Content     :   Base FxArgs class declaration. 
Created     :   Feb 18, 2008
Authors     :   Dmitry Polenur, Maxim Didenko

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_FxSettings_H
#define INC_FxSettings_H
#include "GFx_Kernel.h"
#include "GFx.h"

namespace SF = Scaleform;
using namespace Scaleform;
using namespace Render;
using namespace GFx;

class FxValue :  public RefCountBase<FxValue, Stat_Default_Mem>
{
public:
    FxValue() : Valid(false){}
    virtual ~FxValue();

    bool                IsValue()      const {return Valid;}

    virtual int         GetInt(unsigned = 0)       const = 0;
    virtual float       GetFloat(unsigned = 0)     const = 0;
    virtual bool        GetBool(unsigned = 0)      const = 0;
    virtual String     GetString(unsigned = 0)    const = 0;

    // Used in FxValueList, for other FxValue types it will return 1 if value is set and 0 otherwise,
    // so scalar values can be used as list with one element
    virtual UPInt       GetListSize() const {return (UPInt)Valid;}

protected:
    bool Valid;
};

struct FxArgDesc
{  
    const char* pName;
    const char* pLongName;
    unsigned    Flags;
    const char* pDefaultValue;
    const char* pHelpString;
};

class FxArgs 
{
public:

    enum FxArgDescrFlags
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

    void AddDesriptions(const FxArgDesc* pargDescs);

    void PrintOptionsHelp() const;
    void PrintArgValues() const;

    int ParseCommandLine(int argc, char* argv[]);

    
    // *** Value / Argument access

    bool          HasValue(const char* name)    const;

    SF::Ptr<FxValue> GetValue(const char* name)    const; 
    SF::Ptr<FxValue> operator[] (const char* name) const { return GetValue(name); }

    // If value does not exist SF_ASSERT will be triggered (misspelled name is the most likely cause) 
    int         GetInt(const char* name, int idx = 0) const     { SF_ASSERT(GetValue(name)); return GetValue(name)->GetInt(idx); }
    bool        GetBool(const char* name, int idx = 0) const    { SF_ASSERT(GetValue(name)); return GetValue(name)->GetBool(idx); }
    String     GetString(const char* name, int idx = 0) const  { SF_ASSERT(GetValue(name)); return GetValue(name)->GetString(idx); }
    float       GetFloat(const char* name, int idx = 0) const   { SF_ASSERT(GetValue(name)); return GetValue(name)->GetFloat(idx); }

    // Used only in FxValueList, SF_ASSERT will be triggered for other types
    UPInt       GetListSize(const char* name) const;


private:
    Array<FxArgDesc>           Descs;
    Array<SF::Ptr<FxValue> >      Values;
    struct HashValue
    {
        HashValue(UPInt argIdx = 0, UPInt valueIdx = 0) : ArgIdx(argIdx), ValueIdx(valueIdx) {}
        UPInt  ArgIdx;
        UPInt  ValueIdx;
    };
    StringHash<HashValue>    IdxValues;
    Array<HashValue>           PosValues;
};


#endif //INC_FxSettings_H
