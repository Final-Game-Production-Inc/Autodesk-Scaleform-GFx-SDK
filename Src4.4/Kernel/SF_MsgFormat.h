/**************************************************************************

PublicHeader:   None
Filename    :   SF_MsgFormat.h
Content     :   Formatting of strings
Created     :   January 26, 2009
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_MsgFormat_H
#define INC_SF_Kernel_MsgFormat_H

#include <stdarg.h>

#include "SF_String.h"
#include "SF_Array.h"
#include "SF_StringHash.h"
#include "SF_Locale.h"
#include "SF_StackMemPool.h"


namespace Scaleform {

////////////////////////////////////////////////////////////////////////////////
// Forward declaration.

// ***** FmtInfo
// Formatter traits class.
template <typename T> struct FmtInfo;

class MsgFormat;


////////////////////////////////////////////////////////////////////////////////

// ***** FmtResource

// Base class for all formatting resources.
// Main purpose of this class is to declare method Reflect, which "reflects"/returns
// set of available attributes.

class FmtResource
{
public:
    virtual ~FmtResource() {};

public:
    struct TAttrs 
    {
        TAttrs() : Num(0), Attrs(NULL) {}

        unsigned Num; // Number of attributes.
        UPInt*   Attrs; // Array of UPInt values.
    };

public:
    // attrs - pointer to a structure, which containts size on an array of 
    //         attributes and a pointer to the array.
    // Return number of attributes.
    virtual unsigned Reflect(const TAttrs*& attrs) const;
};

// ***** Formatter

// Abstract base class for all formatters.
// Formatter can be used as a standalone object and as a part of MsgFormat.
// Formatters can be organized into trees.
class Formatter : public FmtResource
{
public:
    enum requirement_t 
    {
        rtNone = 0,      // No requirements.
        rtPrevStr = 1,   // Previous string.
        rtPrevStrSS = 2, // Sentence separator. It is used to prevent cyclic dependencies.
        rtNextStr = 4,   // Next string.
        rtParent = 8     // Parent formatter.
    };
    // Set of requirements.
    typedef UInt8 requirements_t;

    // Position of a parent formatter.
    enum ParentRef 
    {
        prNone, // No parent formatter.
        prPrev, // Previous. Relative position.
        prNext, // Next. Relative position.
        prPos   // Absolute position (from the left).
    };

public:
    Formatter()
    : pParentFmt(NULL)
    , IsConverted(false)
    {
    }
    Formatter(MsgFormat& f)
    : pParentFmt(&f)
    , IsConverted(false)
    {
    }
    virtual ~Formatter();

public:
    // Parse formatting string.
    virtual void Parse(const StringDataPtr& str);
    // Convert value.
    virtual void Convert() = 0;
    // Retrieve conversion result.
    virtual StringDataPtr GetResult() const = 0;
    // Retrieve size of a converted value.
    // May be called only after Convert().
    virtual UPInt GetSize() const = 0;
    // Retrieve formatter's requirements. 
    virtual requirements_t GetRequirements() const;
    // Check conversion status for optimization purposes.
    bool Converted() const { return IsConverted; }

public:
    // Dependency-related ...

    // Prev string ...
    virtual void SetPrevStr(const StringDataPtr& /*ptr*/);
    virtual StringDataPtr GetPrevStr() const;

    // Next string ...
    virtual void SetNextStr(const StringDataPtr& /*ptr*/);
    virtual StringDataPtr GetNextStr() const;

    // Parent formatter ...
    virtual ParentRef GetParentRef() const;
    virtual unsigned char GetParentPos() const;
    virtual void SetParent(const FmtResource&);

protected:
    bool HasParent() const
    {
        return pParentFmt != NULL;
    }
    // Non-const version is required for formatter substitution.
    MsgFormat& GetParent()
    {
        SF_ASSERT(pParentFmt);
        return *pParentFmt;
    }
    const MsgFormat& GetParent() const
    {
        SF_ASSERT(pParentFmt);
        return *pParentFmt;
    }

    // Parent formatter ...
    // Set relative position.
    virtual void SetParentRef(ParentRef);
    // Set absolute position of a parent formatter.
    virtual void SetParentPos(unsigned char);

    // Set IsConverted flag.
    void SetConverted(bool value = true)
    {
        IsConverted = value;
    }

private:
    Formatter& operator = (const Formatter&);

private:
    MsgFormat*  pParentFmt;
    bool        IsConverted;
};

////////////////////////////////////////////////////////////////////////////////

// ***** StrFormatter

// String formatter. Implementation of Formatter, which serves as a proxy 
// for string data types.

class StrFormatter : public Formatter
{
public:
    StrFormatter(const char* v);
    StrFormatter(const StringDataPtr& v);
    StrFormatter(const String& v);

    StrFormatter(MsgFormat& f, const char* v);
    StrFormatter(MsgFormat& f, const StringDataPtr& v);
    StrFormatter(MsgFormat& f, const String& v);

public:
    virtual void        Parse(const StringDataPtr& str);
    virtual void        Convert();
    virtual StringDataPtr GetResult() const;
    virtual UPInt       GetSize() const;

private:
    StrFormatter& operator = (const StrFormatter&);

private:
    const StringDataPtr    Value;
};

template <>
struct FmtInfo<const char*>
{
    typedef StrFormatter formatter;
};

template <>
struct FmtInfo<char*>
{
    typedef StrFormatter formatter;
};

template <>
struct FmtInfo<const unsigned char*>
{
    typedef StrFormatter formatter;
};

template <>
struct FmtInfo<unsigned char*>
{
    typedef StrFormatter formatter;
};

template <>
struct FmtInfo<const signed char*>
{
    typedef StrFormatter formatter;
};

template <>
struct FmtInfo<signed char*>
{
    typedef StrFormatter formatter;
};

template <int N>
struct FmtInfo<const char[N]>
{
    typedef StrFormatter formatter;
};

template <int N>
struct FmtInfo<char[N]>
{
    typedef StrFormatter formatter;
};

template <int N>
struct FmtInfo<const unsigned char[N]>
{
    typedef StrFormatter formatter;
};

template <int N>
struct FmtInfo<unsigned char[N]>
{
    typedef StrFormatter formatter;
};

template <int N>
struct FmtInfo<const signed char[N]>
{
    typedef StrFormatter formatter;
};

template <int N>
struct FmtInfo<signed char[N]>
{
    typedef StrFormatter formatter;
};

template <>
struct FmtInfo<const StringDataPtr>
{
    typedef StrFormatter formatter;
};

template <>
struct FmtInfo<StringDataPtr>
{
    typedef StrFormatter formatter;
};

template <>
struct FmtInfo<const String>
{
    typedef StrFormatter formatter;
};

template <>
struct FmtInfo<String>
{
    typedef StrFormatter formatter;
};

template <>
struct FmtInfo<const StringLH>
{
    typedef StrFormatter formatter;
};

template <>
struct FmtInfo<StringLH>
{
    typedef StrFormatter formatter;
};

template <>
struct FmtInfo<const StringDH>
{
    typedef StrFormatter formatter;
};

template <>
struct FmtInfo<StringDH>
{
    typedef StrFormatter formatter;
};


////////////////////////////////////////////////////////////////////////////////

// ***** BoolFormatter

// Bool formatter. Implementation of Formatter, which can transform boolean value
// into a string. By default true is transformed into string "true", and false 
// into "false" accordingly. This default behavior can be changed by using a build-in
// switch formatter. For example:
//
//   Format(sink, "{0:sw:yes:no}", true);
//   Format(sink, "{0:sw:yes:no}", false);
//
// where "sw" is a argument, which enables "switch-formatter". Next two arguments are
// string values corresponding to true and false.

class BoolFormatter : public Formatter
{
public:
    BoolFormatter(MsgFormat& f, bool v);

public:
    virtual void        Parse(const StringDataPtr& str);
    virtual void        Convert();
    virtual StringDataPtr GetResult() const;
    virtual UPInt       GetSize() const;

private:
    BoolFormatter& operator = (const BoolFormatter&);

private:
    bool            Value:1;
    bool            SwitchStmt:1;
    StringDataPtr   result;
};

template <>
struct FmtInfo<bool>
{
    typedef BoolFormatter formatter;
};


////////////////////////////////////////////////////////////////////////////////

// ***** NumericBase
// Base class for LongFormatter and DoubleFormatter.
// It just keeps data common to its children.
class NumericBase
{
public:
    NumericBase();

public:
    char*   ToCStr() const { return ValueStr; }

public:
    void    SetPrecision(unsigned prec = 1)
    {
        Precision = prec;
    }
    void    SetWidth(unsigned width = 1)
    {
        Width = width;
    }
    // In case of hexadecimal and octal numbers it also means "show prefix".
    void    SetShowSign(bool flag = true)
    {
        ShowSign = flag;
    }
    void    SetPrefixChar(char prefix = ' ')
    {
        PrefixChar = prefix;
    }
    void    SetSeparatorChar(char sep = ',')
    {
        SeparatorChar = sep;
    }
    void    SetBigLetters(bool flag = true)
    {
        BigLetters = flag;
    }
    void    SetBlankPrefix(bool flag = true)
    {
        BlankPrefix = flag;
    }
    void    SetAlignLeft(bool flag = true)
    {
        AlignLeft = flag;
    }
    void    SetSharpSign(bool flag = true)
    {
        SharpSign = flag;
    }

protected:
    void    ReadPrintFormat(StringDataPtr token);
    void    ReadWidth(StringDataPtr token);

    void    ULongLong2String(char* buff, UInt64 value, bool separator, unsigned base = 10);
    void    ULong2String(char* buff, UInt32 value, bool separator, unsigned base = 10);

protected:
    unsigned Precision:5;
    unsigned Width:5;
    char     PrefixChar:7;
    char     SeparatorChar:7;
    bool     ShowSign:1;
    bool     BigLetters:1;
    bool     BlankPrefix:1;
    bool     AlignLeft:1; // Required for sprintf().
    bool     SharpSign:1; // Required for sprintf().

    char*    ValueStr;
};

////////////////////////////////////////////////////////////////////////////////

// ***** LongFormatter

// Long formatter. This class is capable of formatting of integer data types.
// This class can be used either with Format or as a standalone formatter.
// It also has a built-in switch formatter. Example:
//
//   Format(sink, "{0:sw:0:zero:1:one:2:two:many}", int_value);
//
// Where argument "sw" enables switch formatter. Arguments of the switch formatter
// should follow the format below.
//
// sw:(int_value:str_value)+(:default_str_value)?
//
// For each pair "int_value:str_value" int_value will be replaced with the str_value.
// If a value does not correspond to any of int_value, it will be replaced with 
// a default string value "default_str_value".
class LongFormatter : public Formatter, public NumericBase, public String::InitStruct
{
public:
    LongFormatter(int v);
    LongFormatter(unsigned int v);

    LongFormatter(long v);
    LongFormatter(unsigned long v);

#if !defined(SF_64BIT_POINTERS) || defined(SF_OS_WIN32)
    // SInt64 and UInt64 on PS2 are equivalent to long and unsigned long.
    LongFormatter(SInt64 v);
    LongFormatter(UInt64 v);
#endif

    LongFormatter(MsgFormat& f, int v);
    LongFormatter(MsgFormat& f, unsigned int v);

    LongFormatter(MsgFormat& f, long v);
    LongFormatter(MsgFormat& f, unsigned long v);

#if !defined(SF_64BIT_POINTERS) || defined(SF_OS_WIN32)
    // SInt64 and UInt64 on PS2 are equivalent to long and unsigned long.
    LongFormatter(MsgFormat& f, SInt64 v);
    LongFormatter(MsgFormat& f, UInt64 v);
#endif

public:
    virtual void        Parse(const StringDataPtr& str);
    virtual void        Convert();
    virtual StringDataPtr GetResult() const;
    virtual UPInt       GetSize() const;
    virtual void        InitString(char* pbuffer, UPInt size) const;

public:
    LongFormatter& SetPrecision(unsigned prec = 1)
    {
        SF_ASSERT(prec <= 20);
        NumericBase::SetPrecision(prec);
        return *this;
    }
    LongFormatter& SetWidth(unsigned width = 1)
    {
        SF_ASSERT(width <= 20);
        NumericBase::SetWidth(width);
        return *this;
    }
    LongFormatter& SetShowSign(bool flag = true)
    {
        NumericBase::SetShowSign(flag);
        return *this;
    }

    // base should be in range [2..16].
    LongFormatter& SetBase(unsigned base)
    {
        SF_ASSERT(base >= 2 && base <= 16);
        Base = base;
        return *this;
    }
    LongFormatter& SetPrefixChar(char prefix = ' ')
    {
        NumericBase::SetPrefixChar(prefix);
        return *this;
    }
    LongFormatter& SetSeparatorChar(char sep = ',')
    {
        NumericBase::SetSeparatorChar(sep);
        return *this;
    }
    LongFormatter& SetBigLetters(bool flag = true)
    {
        NumericBase::SetBigLetters(flag);
        return *this;
    }

private:
    void AppendSignCharLeft(bool negative);
    LongFormatter& operator = (const LongFormatter&);

    UPInt GetSizeInternal() const
    {
        return Buff + sizeof(Buff) - 1 - ValueStr;
    }

private:
    unsigned        Base:5;
    const bool      SignedValue:1;
    bool            IsLongLong:1;

    const SInt64    Value;
    // Big-enough to keep signed long long with separators plus zero terminator.
    // Should be increased for Unicode separators.
    char            Buff[29]; 
};

template <>
struct FmtInfo<char>
{
    typedef LongFormatter formatter;
};

template <>
struct FmtInfo<unsigned char>
{
    typedef LongFormatter formatter;
};

template <>
struct FmtInfo<signed char>
{
    typedef LongFormatter formatter;
};

template <>
struct FmtInfo<short>
{
    typedef LongFormatter formatter;
};

template <>
struct FmtInfo<unsigned short>
{
    typedef LongFormatter formatter;
};

template <>
struct FmtInfo<int>
{
    typedef LongFormatter formatter;
};

template <>
struct FmtInfo<unsigned int>
{
    typedef LongFormatter formatter;
};

template <>
struct FmtInfo<long>
{
    typedef LongFormatter formatter;
};

template <>
struct FmtInfo<unsigned long>
{
    typedef LongFormatter formatter;
};

#if !defined(SF_64BIT_POINTERS) || defined(SF_OS_WIN32)
template <>
struct FmtInfo<SInt64>
{
    typedef LongFormatter formatter;
};

template <>
struct FmtInfo<UInt64>
{
    typedef LongFormatter formatter;
};
#endif

////////////////////////////////////////////////////////////////////////////////
//#define INTERNAL_D2A

// ***** DoubleFormatter

// Double formatter. This class is capable of formatting of numeric data types with 
// floating point.
class DoubleFormatter : public Formatter, public NumericBase, public String::InitStruct
{
public:
    enum PresentationType { FmtDecimal, FmtScientific, FmtSignificant };
    
public:
    DoubleFormatter(double v);
    DoubleFormatter(MsgFormat& f, double v);

public:
    virtual void        Parse(const StringDataPtr& str);
    virtual void        Convert();
    virtual StringDataPtr GetResult() const;
    virtual UPInt       GetSize() const;
    virtual void        InitString(char* pbuffer, UPInt size) const;

public:
    DoubleFormatter& SetType(PresentationType ptype)
    {
        Type = ptype;
        return *this;
    }
    DoubleFormatter& SetPrecision(unsigned prec = 1)
    {
        NumericBase::SetPrecision(prec);
        return *this;
    }
    DoubleFormatter& SetWidth(unsigned width = 1)
    {
        NumericBase::SetWidth(width);
        return *this;
    }
    DoubleFormatter& SetShowSign(bool flag = true)
    {
        NumericBase::SetShowSign(flag);
        return *this;
    }
    DoubleFormatter& SetPrefixChar(char prefix = ' ')
    {
        NumericBase::SetPrefixChar(prefix);
        return *this;
    }
    DoubleFormatter& SetSeparatorChar(char sep = ',')
    {
        NumericBase::SetSeparatorChar(sep);
        return *this;
    }
    DoubleFormatter& SetBigLetters(bool flag = true)
    {
        NumericBase::SetBigLetters(flag);
        return *this;
    }
    DoubleFormatter& SetAlignLeft(bool flag = true)
    {
        NumericBase::SetAlignLeft(flag);
        return *this;
    }

private:
    DoubleFormatter& operator = (const DoubleFormatter&);

    void AppendSignCharLeft(bool negative, bool show_sign = false);

#ifdef INTERNAL_D2A
    // In case of underflow it can switch to scientific format.
    void DecimalFormat(Double v);
    void ScientificFormat();
#endif

    UPInt GetSizeInternal() const
    {
        return Buff + sizeof(Buff) - 1 - ValueStr;
    }

private:
    PresentationType    Type;
    const double        Value;
    UPInt               Len;

    char                Buff[347 + 1]; 
};

template <>
struct FmtInfo<float>
{
    typedef DoubleFormatter formatter;
};

template <>
struct FmtInfo<double>
{
    typedef DoubleFormatter formatter;
};

////////////////////////////////////////////////////////////////////////////////
class ResouceProvider;

////////////////////////////////////////////////////////////////////////////////

// ***** ResourceFormatter

// Formatter, which is responsible for formatting of localization resources.

class ResourceFormatter : public Formatter
{
public:
    class ValueType
    {
    public:
        ValueType(UPInt rc);
        ValueType(UPInt rc, const ResouceProvider& provider);
        // int version of RC is supposed to be used with NULL argument only.
        // It is interpreted as a string pointer.
        explicit ValueType(int rc);
        // int version of RC is supposed to be used with NULL argument only.
        // It is interpreted as a string pointer.
        explicit ValueType(int rc, const ResouceProvider& provider);
        ValueType(const char* rc);
        ValueType(const char* rc, const ResouceProvider& provider);

    public:
        bool IsStrResource() const
        {
            return IsString;
        }
        const char* GetStrResource() const
        {
            SF_ASSERT(IsStrResource());
            return Resource.RStr;
        }
        UPInt GetLongResource() const
        {
            SF_ASSERT(!IsStrResource());
            return Resource.RLong;
        }

    public:
        const ResouceProvider* GetResouceProvider() const
        {
            return RC_Provider;
        }
        void SetResouceProvider(const ResouceProvider* rp)
        {
            RC_Provider = rp;
        }

    private:
        ValueType& operator = (const ValueType&);

    private:
        union ResourceType
        {
            const char*         RStr;
            UPInt               RLong;
        };

        ResourceType            Resource;
        const bool              IsString;
        const ResouceProvider*  RC_Provider;
    };

public:
    ResourceFormatter(MsgFormat& f, const ValueType& v);
    virtual ~ResourceFormatter();

public:
    virtual void            Parse(const StringDataPtr& str);
    virtual void            Convert();
    virtual StringDataPtr   GetResult() const;
    virtual UPInt           GetSize() const;

private:
    StringDataPtr MakeString(const TAttrs& attrs) const;

private:
    ResourceFormatter& operator = (const ResourceFormatter&);

private:
    const ValueType         Value;
    const ResouceProvider*  pRP;

    StringDataPtr           Result;
};

template <>
struct FmtInfo<ResourceFormatter::ValueType>
{
    typedef ResourceFormatter formatter;
};

////////////////////////////////////////////////////////////////////////////////

// ***** ResouceProvider

// Localization resource provider.

class ResouceProvider : public NewOverrideBase<Stat_Default_Mem>
{
public:
    ResouceProvider(const ResouceProvider* next = NULL) : nextRP(next) {}
    virtual ~ResouceProvider() {}

public:
    virtual unsigned      Reflect(const ResourceFormatter::ValueType& /*rc*/, const FmtResource::TAttrs** attrs) const = 0;
    virtual StringDataPtr MakeString(const ResourceFormatter::ValueType& /*rc*/, const FmtResource::TAttrs& /*attrs*/) const = 0;

protected:
    const ResouceProvider* GetNext() const { return nextRP; }

private:
    const ResouceProvider* nextRP;
};


////////////////////////////////////////////////////////////////////////////////

// ***** FormatterFactory

// Abstract base class for all formatter factories.
class FormatterFactory
{
public:
    virtual ~FormatterFactory() {}

public:
    struct Args
    {
        Args(MsgFormat&f, const StringDataPtr& n, const ResourceFormatter::ValueType& v) 
            : Fmt(f), Name(n), Value(v)
        {
        }

        MsgFormat&                             Fmt;
        const StringDataPtr&                   Name;
        const ResourceFormatter::ValueType&    Value;

    private:
        Args& operator = (const Args&);
    };

public:
    virtual Formatter* MakeFormatter(const Args& args) const = 0;
};

////////////////////////////////////////////////////////////////////////////////

// ***** LocaleProvider

// Formatter factory, which is also aware of locale. This class is supposed to
// provide all information related to localization.

class LocaleProvider : public FormatterFactory
{
public:
    virtual ~LocaleProvider() {}

public:
    virtual const Locale&       GetLocale() const = 0;
    virtual ResouceProvider*    GetDefaultRCProvider() const = 0;
};


////////////////////////////////////////////////////////////////////////////////

// ***** MsgFormat

// Class, which parses formatting string, applies formatter's values, and
// puts final formatted string into a Sink.
//
// Sink can be of three data types:
//   1) String. Resulting string will be copied into String.
//   2) StringBuffer. Resulting string will be added to StringBuffer.
//   3) StringDataPtr. This data type represents a buffer. Resulting string will 
//        be copied into the buffer.
class MsgFormat : String::InitStruct
{
public:
    class Sink
    {
        friend class MsgFormat;

    public:
        Sink(String& str) 
            : Type(tStr), SinkData(str) {}
        Sink(StringBuffer& buffer) 
            : Type(tStrBuffer), SinkData(buffer) {}
        Sink(const StringDataPtr& strData) 
            : Type(tDataPtr), SinkData(strData) {}
        template <typename T, int N> 
        Sink(const T (&v)[N])
            : Type(tDataPtr), SinkData(StringDataPtr(v, N)) {}

    private:
        enum DataType {tStr, tStrBuffer, tDataPtr};

        struct StrDataType
        {
            const char*     pStr;
            UPInt           Size;
        };

        union SinkDataType
        {
        public:
            SinkDataType(String& str)
                : pStr(&str)
            {
            }
            SinkDataType(StringBuffer& buffer)
                : pStrBuffer(&buffer)
            {
            }
            SinkDataType(const StringDataPtr& strData)
            {
                DataPtr.pStr = strData.ToCStr();
                DataPtr.Size = strData.GetSize();
            }

        public:
            String*         pStr;
            StringBuffer*   pStrBuffer;
            StrDataType     DataPtr;
        };

        DataType            Type;
        SinkDataType        SinkData;
    };

public:
    MsgFormat(const Sink& r);
    MsgFormat(const Sink& r, const LocaleProvider& loc);
    ~MsgFormat();

public:
    typedef UInt8 ArgNumType;
    typedef StackMemPool<> MemoryPoolType;

    // Parse formatting string in "Scaleform" format. 
    // Split a string onto tokens, which can be of two types:
    // text and parameter. For parameters retrieve an argument number.
    void Parse(const char* fmt);

    // Parse formatting string in "sprintf" format.
    // This function creates all necessary formatters.
    void FormatF(const StringDataPtr& fmt, va_list argList);

    // Replace an old formatter with the new one.
    // Argument "allocated" should be true if new formatter is dynamically allocated.
    // Return value: true on success.
    bool ReplaceFormatter(Formatter* oldf, Formatter* newf, bool allocated);

    ArgNumType GetFirstArgNum() const
    {
        return FirstArgNum;
    }
    void SetFirstArgNum(ArgNumType num)
    {
        FirstArgNum = num;
    }

    // Get current locale provider if any.
    const LocaleProvider* GetLocaleProvider() const
    {
        return pLocaleProvider;
    }

    // Get memory pool, which can be used by formatters.
    MemoryPoolType& GetMemoryPool()
    {
        return MemPool;
    }

    // Get formatted string size.
    UPInt GetStrSize() const
    {
        return StrSize;
    }

public:
    UInt8 GetEscapeChar() const
    {
        return EscapeChar;
    }
    void SetEscapeChar(UInt8 c)
    {
        EscapeChar = c;
    }

public:
    // "D" stands for "Dynamic".

    // Create a binding for one argument.
    // This function will allocate a formatter only if this argument is mentioned in a
    // formatting string.
    template <typename T>
    void FormatD1(const T& v)
    {
        while (NextFormatter())
        {
            Bind(new (GetMemoryPool()) typename FmtInfo<T>::formatter(*this, v), true);
        }

        ++FirstArgNum;
    }

    void FinishFormatD();

private:
    enum ERecType {eStrType, eParamStrType, eFmtType};

    struct str_ptr
    {
        const char*     Str;
        unsigned char   Len;
        unsigned char   ArgNum;
    };

    struct fmt_ptr
    {
        Scaleform::Formatter* Formatter;
        bool            Allocated;
    };

    union fmt_value
    {
        str_ptr         String;
        fmt_ptr         Formatter;
    };

    class fmt_record
    {
    public:
        fmt_record(ERecType type, const fmt_value& value)
        : RecType(type)
        , RecValue(value)
        {
        }

        ERecType GetType() const
        {
            return RecType;
        }
        const fmt_value& GetValue() const
        {
            return RecValue;
        }

    private:
        ERecType    RecType;
        fmt_value   RecValue;
    };

private:
    // Function with side effects.
    // Calculate next DataInd, which is supposed to be used by Bind().
    bool NextFormatter();
    // Bind a formatter at DataInd,
    void Bind(Formatter* formatter, const bool allocated);
    void BindNonPos();
    void MakeString();
    void Evaluate(UPInt ind);

    void AddStringRecord(const StringDataPtr& str);
    void AddFormatterRecord(Formatter* f, bool allocated);

    // InitStruct-related.
    virtual void InitString(char* pbuffer, UPInt size) const;

private:
    MsgFormat& operator = (const MsgFormat&);

private:
    // Minimalistic implementation of Array, which allocates first SS elements
    // on stack.
    template <typename T, UPInt SS = 8, class DA = Array<T> >
    class StackArray
    {
    public:
        typedef T       ValueType;
    
    public:
        StackArray()
            : Size(0)
        {
        }
        
        UPInt GetSize() const { return Size; }

        void PushBack(const ValueType& val)
        {
            if (Size < SS)
                * reinterpret_cast<ValueType*>(StaticArray + Size * sizeof(T)) = val;
            else
                DynamicArray.PushBack(val);

            ++Size;
        }

        ValueType& operator [] (UPInt index)
        {
            if (index < SS)
                return * reinterpret_cast<ValueType*>(StaticArray + index * sizeof(T));

            return DynamicArray[index - SS];
        }
        const ValueType& operator [] (UPInt index) const
        {
            if (index < SS)
                return * reinterpret_cast<const ValueType*>(StaticArray + index * sizeof(T));

            return DynamicArray[index - SS];
        }

    private:
        UPInt   Size;
        DA      DynamicArray;
        char    StaticArray[SS * sizeof(T)];
    };

private:
    // StackArray, which allocates first 16 elements on stack.
    typedef StackArray<fmt_record, 16, ArrayPOD<fmt_record> > DataType;

    UInt8                   EscapeChar;
    UInt8                   FirstArgNum;
    UInt16                  NonPosParamNum;
    UInt16                  UnboundFmtrInd;
    UPInt                   StrSize;
    SPInt                   DataInd;
    const LocaleProvider*   pLocaleProvider;
    const Sink              Result;

    DataType                Data;
    MemoryPoolType          MemPool;
};


////////////////////////////////////////////////////////////////////////////////
// Formatter type traits.

template <typename T>
struct FmtInfo
{
};

////////////////////////////////////////////////////////////////////////////////

// ***** Format

// Format is supposed to replace the sprintf function. 
// It doesn't not to declare argument's data types in a formatted string.

// 0
inline
UPInt Format(
    const MsgFormat::Sink& result, 
    const char* fmt
    )
{
    MsgFormat parsed_format(result);

    parsed_format.Parse(fmt);
    parsed_format.FinishFormatD();
    return parsed_format.GetStrSize();
}

inline
UPInt Format(
    const MsgFormat::Sink& result, 
    const LocaleProvider& loc,
    const char* fmt
    )
{
    MsgFormat parsed_format(result, loc);

    parsed_format.Parse(fmt);
    parsed_format.FinishFormatD();
    return parsed_format.GetStrSize();
}

////////////////////////////////////////////////////////////////////////////////
// Format is developed as a list of FormatD1 calls in order to produce smaller
// code with CodeWarrior for Wii.

// 1 ...
template <typename T1>
inline
UPInt Format(
    const MsgFormat::Sink& result, 
    const char* fmt, 
    const T1& v1
    )
{
    MsgFormat parsed_format(result);

    parsed_format.Parse(fmt);
    parsed_format.FormatD1(v1);
    parsed_format.FinishFormatD();
    return parsed_format.GetStrSize();
}

template <typename T1>
inline
UPInt Format(
    const MsgFormat::Sink& result, 
    const LocaleProvider& loc,
    const char* fmt, 
    const T1& v1
    )
{
    MsgFormat parsed_format(result, loc);

    parsed_format.Parse(fmt);
    parsed_format.FormatD1(v1);
    parsed_format.FinishFormatD();
    return parsed_format.GetStrSize();
}

// 2 ...
template <typename T1, typename T2>
inline
UPInt Format(
    const MsgFormat::Sink& result, 
    const char* fmt, 
    const T1& v1, 
    const T2& v2
    )
{
    MsgFormat parsed_format(result);

    parsed_format.Parse(fmt);
    parsed_format.FormatD1(v1);
    parsed_format.FormatD1(v2);
    parsed_format.FinishFormatD();
    return parsed_format.GetStrSize();
}

template <typename T1, typename T2>
inline
UPInt Format(
    const MsgFormat::Sink& result, 
    const LocaleProvider& loc,
    const char* fmt, 
    const T1& v1, 
    const T2& v2
    )
{
    MsgFormat parsed_format(result, loc);

    parsed_format.Parse(fmt);
    parsed_format.FormatD1(v1);
    parsed_format.FormatD1(v2);
    parsed_format.FinishFormatD();
    return parsed_format.GetStrSize();
}

// 3 ...
template <typename T1, typename T2, typename T3>
inline
UPInt Format(
    const MsgFormat::Sink& result, 
    const char* fmt, 
    const T1& v1, 
    const T2& v2,
    const T3& v3
    )
{
    MsgFormat parsed_format(result);

    parsed_format.Parse(fmt);
    parsed_format.FormatD1(v1);
    parsed_format.FormatD1(v2);
    parsed_format.FormatD1(v3);
    parsed_format.FinishFormatD();
    return parsed_format.GetStrSize();
}

template <typename T1, typename T2, typename T3>
inline
UPInt Format(
    const MsgFormat::Sink& result, 
    const LocaleProvider& loc,
    const char* fmt, 
    const T1& v1, 
    const T2& v2,
    const T3& v3
    )
{
    MsgFormat parsed_format(result, loc);

    parsed_format.Parse(fmt);
    parsed_format.FormatD1(v1);
    parsed_format.FormatD1(v2);
    parsed_format.FormatD1(v3);
    parsed_format.FinishFormatD();
    return parsed_format.GetStrSize();
}

// 4 ...
template <typename T1, typename T2, typename T3, typename T4>
inline
UPInt Format(
    const MsgFormat::Sink& result, 
    const char* fmt, 
    const T1& v1, 
    const T2& v2,
    const T3& v3,
    const T4& v4
    )
{
    MsgFormat parsed_format(result);

    parsed_format.Parse(fmt);
    parsed_format.FormatD1(v1);
    parsed_format.FormatD1(v2);
    parsed_format.FormatD1(v3);
    parsed_format.FormatD1(v4);
    parsed_format.FinishFormatD();
    return parsed_format.GetStrSize();
}

template <typename T1, typename T2, typename T3, typename T4>
inline
UPInt Format(
    const MsgFormat::Sink& result, 
    const LocaleProvider& loc,
    const char* fmt, 
    const T1& v1, 
    const T2& v2,
    const T3& v3,
    const T4& v4
    )
{
    MsgFormat parsed_format(result, loc);

    parsed_format.Parse(fmt);
    parsed_format.FormatD1(v1);
    parsed_format.FormatD1(v2);
    parsed_format.FormatD1(v3);
    parsed_format.FormatD1(v4);
    parsed_format.FinishFormatD();
    return parsed_format.GetStrSize();
}

// 5 ...
template <typename T1, typename T2, typename T3, typename T4, typename T5>
inline
UPInt Format(
    const MsgFormat::Sink& result, 
    const char* fmt, 
    const T1& v1, 
    const T2& v2,
    const T3& v3,
    const T4& v4,
    const T5& v5
    )
{
    MsgFormat parsed_format(result);

    parsed_format.Parse(fmt);
    parsed_format.FormatD1(v1);
    parsed_format.FormatD1(v2);
    parsed_format.FormatD1(v3);
    parsed_format.FormatD1(v4);
    parsed_format.FormatD1(v5);
    parsed_format.FinishFormatD();
    return parsed_format.GetStrSize();
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
inline
UPInt Format(
    const MsgFormat::Sink& result, 
    const LocaleProvider& loc,
    const char* fmt, 
    const T1& v1, 
    const T2& v2,
    const T3& v3,
    const T4& v4,
    const T5& v5
    )
{
    MsgFormat parsed_format(result, loc);

    parsed_format.Parse(fmt);
    parsed_format.FormatD1(v1);
    parsed_format.FormatD1(v2);
    parsed_format.FormatD1(v3);
    parsed_format.FormatD1(v4);
    parsed_format.FormatD1(v5);
    parsed_format.FinishFormatD();
    return parsed_format.GetStrSize();
}

// 6 ...
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
inline
UPInt Format(
    const MsgFormat::Sink& result, 
    const char* fmt, 
    const T1& v1, 
    const T2& v2,
    const T3& v3,
    const T4& v4,
    const T5& v5,
    const T6& v6
    )
{
    MsgFormat parsed_format(result);

    parsed_format.Parse(fmt);
    parsed_format.FormatD1(v1);
    parsed_format.FormatD1(v2);
    parsed_format.FormatD1(v3);
    parsed_format.FormatD1(v4);
    parsed_format.FormatD1(v5);
    parsed_format.FormatD1(v6);
    parsed_format.FinishFormatD();
    return parsed_format.GetStrSize();
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
inline
UPInt Format(
    const MsgFormat::Sink& result, 
    const LocaleProvider& loc,
    const char* fmt, 
    const T1& v1, 
    const T2& v2,
    const T3& v3,
    const T4& v4,
    const T5& v5,
    const T6& v6
    )
{
    MsgFormat parsed_format(result, loc);

    parsed_format.Parse(fmt);
    parsed_format.FormatD1(v1);
    parsed_format.FormatD1(v2);
    parsed_format.FormatD1(v3);
    parsed_format.FormatD1(v4);
    parsed_format.FormatD1(v5);
    parsed_format.FormatD1(v6);
    parsed_format.FinishFormatD();
    return parsed_format.GetStrSize();
}

// 7 ...
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
inline
UPInt Format(
    const MsgFormat::Sink& result, 
    const char* fmt, 
    const T1& v1, 
    const T2& v2,
    const T3& v3,
    const T4& v4,
    const T5& v5,
    const T6& v6,
    const T7& v7
    )
{
    MsgFormat parsed_format(result);

    parsed_format.Parse(fmt);
    parsed_format.FormatD1(v1);
    parsed_format.FormatD1(v2);
    parsed_format.FormatD1(v3);
    parsed_format.FormatD1(v4);
    parsed_format.FormatD1(v5);
    parsed_format.FormatD1(v6);
    parsed_format.FormatD1(v7);
    parsed_format.FinishFormatD();
    return parsed_format.GetStrSize();
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
inline
UPInt Format(
    const MsgFormat::Sink& result, 
    const LocaleProvider& loc,
    const char* fmt, 
    const T1& v1, 
    const T2& v2,
    const T3& v3,
    const T4& v4,
    const T5& v5,
    const T6& v6,
    const T7& v7
    )
{
    MsgFormat parsed_format(result, loc);

    parsed_format.Parse(fmt);
    parsed_format.FormatD1(v1);
    parsed_format.FormatD1(v2);
    parsed_format.FormatD1(v3);
    parsed_format.FormatD1(v4);
    parsed_format.FormatD1(v5);
    parsed_format.FormatD1(v6);
    parsed_format.FormatD1(v7);
    parsed_format.FinishFormatD();
    return parsed_format.GetStrSize();
}

// 8 ...
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
inline
UPInt Format(
    const MsgFormat::Sink& result, 
    const char* fmt, 
    const T1& v1, 
    const T2& v2,
    const T3& v3,
    const T4& v4,
    const T5& v5,
    const T6& v6,
    const T7& v7,
    const T8& v8
    )
{
    MsgFormat parsed_format(result);

    parsed_format.Parse(fmt);
    parsed_format.FormatD1(v1);
    parsed_format.FormatD1(v2);
    parsed_format.FormatD1(v3);
    parsed_format.FormatD1(v4);
    parsed_format.FormatD1(v5);
    parsed_format.FormatD1(v6);
    parsed_format.FormatD1(v7);
    parsed_format.FormatD1(v8);
    parsed_format.FinishFormatD();
    return parsed_format.GetStrSize();
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
inline
UPInt Format(
    const MsgFormat::Sink& result, 
    const LocaleProvider& loc,
    const char* fmt, 
    const T1& v1, 
    const T2& v2,
    const T3& v3,
    const T4& v4,
    const T5& v5,
    const T6& v6,
    const T7& v7,
    const T8& v8
    )
{
    MsgFormat parsed_format(result, loc);

    parsed_format.Parse(fmt);
    parsed_format.FormatD1(v1);
    parsed_format.FormatD1(v2);
    parsed_format.FormatD1(v3);
    parsed_format.FormatD1(v4);
    parsed_format.FormatD1(v5);
    parsed_format.FormatD1(v6);
    parsed_format.FormatD1(v7);
    parsed_format.FormatD1(v8);
    parsed_format.FinishFormatD();
    return parsed_format.GetStrSize();
}

// 9 ...
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
inline
UPInt Format(
    const MsgFormat::Sink& result, 
    const char* fmt, 
    const T1& v1, 
    const T2& v2,
    const T3& v3,
    const T4& v4,
    const T5& v5,
    const T6& v6,
    const T7& v7,
    const T8& v8,
    const T9& v9
    )
{
    MsgFormat parsed_format(result);

    parsed_format.Parse(fmt);
    parsed_format.FormatD1(v1);
    parsed_format.FormatD1(v2);
    parsed_format.FormatD1(v3);
    parsed_format.FormatD1(v4);
    parsed_format.FormatD1(v5);
    parsed_format.FormatD1(v6);
    parsed_format.FormatD1(v7);
    parsed_format.FormatD1(v8);
    parsed_format.FormatD1(v9);
    parsed_format.FinishFormatD();
    return parsed_format.GetStrSize();
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
inline
UPInt Format(
    const MsgFormat::Sink& result, 
    const LocaleProvider& loc,
    const char* fmt, 
    const T1& v1, 
    const T2& v2,
    const T3& v3,
    const T4& v4,
    const T5& v5,
    const T6& v6,
    const T7& v7,
    const T8& v8,
    const T9& v9
    )
{
    MsgFormat parsed_format(result, loc);

    parsed_format.Parse(fmt);
    parsed_format.FormatD1(v1);
    parsed_format.FormatD1(v2);
    parsed_format.FormatD1(v3);
    parsed_format.FormatD1(v4);
    parsed_format.FormatD1(v5);
    parsed_format.FormatD1(v6);
    parsed_format.FormatD1(v7);
    parsed_format.FormatD1(v8);
    parsed_format.FormatD1(v9);
    parsed_format.FinishFormatD();
    return parsed_format.GetStrSize();
}

// 10 ...
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
inline
UPInt Format(
    const MsgFormat::Sink& result, 
    const char* fmt, 
    const T1& v1, 
    const T2& v2,
    const T3& v3,
    const T4& v4,
    const T5& v5,
    const T6& v6,
    const T7& v7,
    const T8& v8,
    const T9& v9,
    const T10& v10
    )
{
    MsgFormat parsed_format(result);

    parsed_format.Parse(fmt);
    parsed_format.FormatD1(v1);
    parsed_format.FormatD1(v2);
    parsed_format.FormatD1(v3);
    parsed_format.FormatD1(v4);
    parsed_format.FormatD1(v5);
    parsed_format.FormatD1(v6);
    parsed_format.FormatD1(v7);
    parsed_format.FormatD1(v8);
    parsed_format.FormatD1(v9);
    parsed_format.FormatD1(v10);
    parsed_format.FinishFormatD();
    return parsed_format.GetStrSize();
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
inline
UPInt Format(
    const MsgFormat::Sink& result, 
    const LocaleProvider& loc,
    const char* fmt, 
    const T1& v1, 
    const T2& v2,
    const T3& v3,
    const T4& v4,
    const T5& v5,
    const T6& v6,
    const T7& v7,
    const T8& v8,
    const T9& v9,
    const T10& v10
    )
{
    MsgFormat parsed_format(result, loc);

    parsed_format.Parse(fmt);
    parsed_format.FormatD1(v1);
    parsed_format.FormatD1(v2);
    parsed_format.FormatD1(v3);
    parsed_format.FormatD1(v4);
    parsed_format.FormatD1(v5);
    parsed_format.FormatD1(v6);
    parsed_format.FormatD1(v7);
    parsed_format.FormatD1(v8);
    parsed_format.FormatD1(v9);
    parsed_format.FormatD1(v10);
    parsed_format.FinishFormatD();
    return parsed_format.GetStrSize();
}

////////////////////////////////////////////////////////////////////////////////

// ***** SPrintF

// This function behaves identically to sprintf.
// Return value: size of resulting string.
UPInt SPrintF(const MsgFormat::Sink& result, const char* fmt, ...);

////////////////////////////////////////////////////////////////////////////////

// ***** ReadInteger

// Utility function, which tries to read an integer.
// It returns read integer value on success, or defaultValue otherwise.
// It will trim string if an integer was successfully read.
int   ReadInteger(StringDataPtr& str, int defaultValue, char separator = ':');

template <typename T>
String AsString(const T& v)
{
    String sink;
    Format(sink, "{0}", v);
    return sink;
}

} // Scaleform

#endif //INC_SF_Kernel_MsgFormat_H
