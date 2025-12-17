/**************************************************************************

Filename    :   FxSettings.cpp
Content     :   Base FxArgs class implementation. 
Created     :   Feb 18, 2008
Authors     :   Dmitry Polenur, Maxim Didenko

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "FxSettings.h"
#include "GFx_Kernel.h"
#include "Kernel/SF_MsgFormat.h"

FxValue::~FxValue()
{
}

class FxValueString : public FxValue
{
public:
    FxValueString(const char* value) : StrValue(value) { Valid = true; }
    ~FxValueString() {}

    virtual int         GetInt(unsigned = 0)    const  { return (int) atoi(StrValue.ToCStr()); }
    virtual float       GetFloat(unsigned = 0)  const  { return (float) atof(StrValue.ToCStr()); }
    virtual bool        GetBool(unsigned = 0)   const  { return !StrValue.IsEmpty();}
    virtual String     GetString(unsigned = 0) const  { return StrValue;}

protected:
    String StrValue;
};

class FxValueInt : public FxValueString
{
public:
    FxValueInt(const char* value) : FxValueString(value)
    { 
        IntValue = atoi(value);
    }

    FxValueInt (int value) : FxValueString("")
    {
        IntValue = value;
        Format(StrValue, "{0}", IntValue);
    }

    ~FxValueInt() {}

    virtual int        GetInt(unsigned = 0)   const { return IntValue; }
    virtual float      GetFloat(unsigned = 0) const { return (float) IntValue;}
    virtual bool       GetBool(unsigned = 0)  const { return IntValue !=0;}

private:
    int  IntValue;
};

class FxValueFloat : public FxValueString
{
public:
    FxValueFloat (const char* value) : FxValueString(value) 
    {
        FloatValue = (float)atof(value);
    }

    FxValueFloat (float value) : FxValueString("")
    {
        FloatValue = value;
        Format(StrValue, "{0}", FloatValue);
    }

    ~FxValueFloat(){}

    virtual int         GetInt(unsigned = 0)   const  { return (int)FloatValue;}
    virtual float       GetFloat(unsigned = 0) const  { return FloatValue;}
    virtual bool        GetBool(unsigned = 0)  const  { return FloatValue != 0;}

private:
    float FloatValue;
};

class FxValueBool : public FxValueString
{
public:
    FxValueBool(const char* value) : FxValueString(value) 
    {
        if(StrValue == "true")
            BoolValue = true;
        else
        {
            BoolValue = false;
            StrValue  = "false";
        }
    }
    FxValueBool(bool value) : FxValueString("")
    { 
        BoolValue = value;
        if (value)
            StrValue = "true";
        else 
            StrValue = "false";
        Valid = true;
    }
    ~FxValueBool(){} 

    virtual int        GetInt(unsigned = 0)   const  { return (int)BoolValue; }
    virtual float      GetFloat(unsigned = 0) const  { return (float)BoolValue;}
    virtual bool       GetBool(unsigned = 0)  const  { return BoolValue;}

private:
    bool BoolValue;
};

class FxValueList : public FxValue
{
public:
    FxValueList() {}
    ~FxValueList() {}
    virtual int         GetInt(unsigned idx = 0)    const  {SF_ASSERT(idx < GetListSize()); return ValueList[idx]->GetInt(); }
    virtual float       GetFloat(unsigned idx = 0)  const  {SF_ASSERT(idx < GetListSize()); return ValueList[idx]->GetFloat(); }
    virtual bool        GetBool(unsigned idx = 0)   const  {SF_ASSERT(idx < GetListSize()); return ValueList[idx]->GetBool();}
    virtual String   GetString(unsigned idx = 0) const  {SF_ASSERT(idx < GetListSize()); return ValueList[idx]->GetString();}

    virtual size_t      GetListSize() const {return ValueList.GetSize();}
    void                AddValue(FxValue* pvalue) {ValueList.PushBack(*pvalue); }

private:
    Array<Ptr<FxValue> > ValueList;
};

bool FxArgs::HasValue(const char* name) const
{
    return GetValue(name).GetPtr() != NULL;
}

size_t FxArgs::GetListSize( const char* name ) const
{
    if (GetValue(name))
        return GetValue(name)->GetListSize();
    else
        return 0;
}

Ptr<FxValue> FxArgs::GetValue(const char* name) const
{
    HashValue hv(0,0);
    // If name Value with that name does not exist, try to access value of the option with that name
    if (!IdxValues.Get(name, &hv)) 
    {
        return NULL;
    }
    return Values[hv.ValueIdx];
}

void FxArgs::AddDesriptions(const FxArgDesc* pargDescs)
{
    if (!pargDescs)
        return;
    for (int i = 0; pargDescs[i].Flags != FxArgs::ArgEnd; i++)
    {
        if (pargDescs[i].Flags == FxArgs::Spacer)
        {
            Descs.PushBack(pargDescs[i]);
            continue;
        }

        if (pargDescs[i].pName && strlen(pargDescs[i].pName) > 0 && IdxValues.Get(pargDescs[i].pName, NULL))
        {
            fprintf(stderr,"Warning: Argument description \"%s\" already exists. Skipped.\n", pargDescs[i].pName);
            continue;
        }
        if (pargDescs[i].pLongName && strlen(pargDescs[i].pLongName) > 0 && IdxValues.Get(pargDescs[i].pLongName, NULL))
        {
            fprintf(stderr,"Warning: Argument description \"%s\" already exists. Skipped.\n", pargDescs[i].pLongName);
            continue;
        }
        bool added = false;
        if (pargDescs[i].pName && strlen(pargDescs[i].pName) > 0) 
        {
            IdxValues.Set(pargDescs[i].pName, HashValue(Descs.GetSize(), Values.GetSize()));
            added = true;
        }
        if (pargDescs[i].pLongName && strlen(pargDescs[i].pLongName) > 0) 
        {
            IdxValues.Set(pargDescs[i].pLongName, HashValue(Descs.GetSize(), Values.GetSize()));
            added = true;
        }
        if (!added)
        {
            fprintf(stderr,"Warning: Argument description does not have a pName nor a long pName. Skipped.\n");
            continue;
        }
        if (pargDescs[i].Flags & FxArgs::Positional)
            PosValues.PushBack(HashValue(Descs.GetSize(), Values.GetSize()));
        Descs.PushBack(pargDescs[i]);
        Values.PushBack(NULL);
    }
}

inline static bool hasOptionPrefix( char* arg ) 
{
#ifdef SF_OS_WIN32
    if (arg[0] == '/')
    {
        arg[0] = '-';
        return true;
    }
#endif
    return  (arg[0] == '-');

}

static FxValue* CreateValue(const char* cmdline, unsigned flags)
{
    if (!cmdline)
        return NULL;

    switch (flags & 0xF0)
    {
    case FxArgs::IntValue:
        {
            char *ptail = NULL;
            int  intValue = (int)SFstrtol(cmdline, &ptail, 10);
            if (SFstrlen(ptail) > 0)
            {
                fprintf(stderr,"Error: Cannot convert \"%s\" to integer\n", cmdline);
                return NULL;
            }
            else
                return new FxValueInt(intValue);
        }
    case FxArgs::FloatValue:
        {
            char *ptail = NULL;
            float floatValue = (float) SFstrtod(cmdline, &ptail);
            if (SFstrlen(ptail) > 0)
            {
                fprintf(stderr,"Error: Cannot convert \"%s\" to float\n", cmdline);
                return NULL;
            }
            else
                return new FxValueFloat(floatValue);
        }

    case FxArgs::StringValue:
        return new FxValueString(cmdline);
    case FxArgs::BoolValue:
        return new FxValueBool(cmdline);
    default:
        SF_ASSERT(0);
        return NULL;
    }
}

int FxArgs::ParseCommandLine( int argc, char* argv[] )
{
    UInt16 pCounter = 0; // Positional value counter

    ++argv;
    --argc;
    
    while (argc > 0)
    {
        char* cur_arg= argv[0];
        if (hasOptionPrefix(cur_arg)) // Also changes "/" to "-" for Win32;  
        {
            cur_arg++;
#if defined(SF_OS_MAC)
            if (!strncmp(cur_arg, "psn_", 4))
            {
                ++argv;
                --argc;
                continue;
            }
#endif
            HashValue hv(0,0);
            if (!IdxValues.Get(cur_arg, &hv))
            {
                fprintf(stderr, "Unknown option : %s \n",cur_arg);
                return 1;
            }
            unsigned flag = Descs[hv.ArgIdx].Flags;
            switch (flag & 0x0F) // See FxCmdOption::ArgsDescr 
            {
                case FxArgs::ArgNone:
                    if ( (flag & 0xF0) == FxArgs::BoolValue) 
                        Values[hv.ValueIdx] = *new FxValueBool(true);
                    else
                    {
                        fprintf(stderr,"Option \"%s\" must be BoolValue.\n", cur_arg);
                        return 1;
                    }
                    break;
                case FxArgs::ArgRequred:
                    if (--argc <= 0 )
                    {
                       fprintf(stderr,"Option \"%s\" must have arguments.\n", cur_arg); 
                       return 1;
                    }
                    else
                    {
                        Values[hv.ValueIdx] = *CreateValue((++argv)[0], flag);
                        if (!Values[hv.ValueIdx])
                        {
                            fprintf(stderr,"Cannot parse parameter for \"%s\" \n", cur_arg); 
                            return 1;
                        }
                    }
                    break;
                case FxArgs::ArgList:
                    if (argc <= 1 )
                    {
                        fprintf(stderr,"Option \"%s\" must have arguments.\n", cur_arg); 
                        return 1;
                    }
                    else
                    {
                        Ptr<FxValueList> pValueList = *new FxValueList();
                        while (--argc > 0 && !hasOptionPrefix((argv+1)[0]))
                             pValueList->AddValue(CreateValue((++argv)[0], flag));  
                        ++argc; 
                        Values[hv.ValueIdx] = pValueList;                    
                    }
                    break;
                default:
                    SF_ASSERT(0);
            }
        }
        else
        {
            if (pCounter < PosValues.GetSize())
            {
                const FxArgDesc& descr = Descs[PosValues[pCounter].ArgIdx];
                if ((descr.Flags & 0x0F) == FxArgs::ArgList )
                {
                    Ptr<FxValueList> pValueList = *new FxValueList();
                    while (argc-- > 0 && !hasOptionPrefix(argv[0]))
                        pValueList->AddValue(CreateValue((argv++)[0], descr.Flags));  
                    argc+=2;
                    argv--;
                    Values[PosValues[pCounter].ValueIdx] = pValueList;
                }
                else
                    Values[PosValues[pCounter].ValueIdx] = *CreateValue(cur_arg, descr.Flags);
            }
            else
            {
                fprintf(stderr,"Unknown positional value \"%s\"\n", cur_arg);
                return 1;
            }
            pCounter++;
        }
        ++argv;
        --argc;
    }

    for (StringHash<HashValue>::ConstIterator it = IdxValues.Begin(); it != IdxValues.End(); ++it)
    {
        const HashValue& hv = it->Second;
        const FxArgDesc& descr = Descs[hv.ArgIdx];
        if (!Values[hv.ValueIdx])
        {
            if (descr.Flags & FxArgs::Mandatory)
            {
                fprintf(stderr,"Mandatory option \"%s\" or \"%s\" is not found \n", descr.pName, descr.pLongName);
                return 1;
            }
            if (!Values[hv.ValueIdx].GetPtr())
                Values[hv.ValueIdx] = *CreateValue(descr.pDefaultValue, descr.Flags);
        }
    }

    // If Help option was set then return -1 
    Ptr<FxValue> pfxValue = GetValue("Help");
    if (pfxValue) 
        return pfxValue->GetBool() ? -1 : 0;

    return 0;
}

void FxArgs::PrintOptionsHelp() const
{
    int pos = 0;
    for (unsigned i = 0; i < Descs.GetSize(); ++i)
    {
        if (Descs[i].Flags & FxArgs::Spacer) // Spacer
        {
            printf("%s\n", Descs[i].pName);
            continue;
        }
        if (Descs[i].Flags & FxArgs::Positional)
            printf("<pos%d>      : %s\n", ++pos, Descs[i].pHelpString);
        else if (Descs[i].pHelpString)
            printf("-%-10s : %s\n", Descs[i].pName, Descs[i].pHelpString);
    }
}

void FxArgs::PrintArgValues() const
{
    for (unsigned i = 0; i < Descs.GetSize(); i++)
    {
        Ptr<FxValue> pfxValue = GetValue(Descs[i].pLongName);
        if (pfxValue)
        {
            printf("%-30s : %s ", Descs[i].pLongName, pfxValue->GetString().ToCStr());
            if ((Descs[i].Flags & 0x0F) == FxArgs::ArgList)
                for (unsigned j = 1; j < pfxValue->GetListSize(); j++ )
                    printf("%s ", GetString(Descs[i].pLongName, j).ToCStr() );
            printf("\n");
        }
        else
            printf("%-30s : Value is not set \n", Descs[i].pLongName);

    }
}
