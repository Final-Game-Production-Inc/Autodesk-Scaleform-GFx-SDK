/**************************************************************************

Filename    :   AS3_Marshalling.h
Content     :   
Created     :   Dec, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_AS3_MARSHALLING_H
#define INC_AS3_MARSHALLING_H

namespace Scaleform { namespace GFx { namespace AS3
{

///////////////////////////////////////////////////////////////////////////
template <typename T>
struct Clean2
{
    typedef T type;
};

template <typename T>
struct Clean2<const T>
{
    typedef T type;
};

template <typename T>
struct Clean2<T&>
{
    typedef T type;
};

template <typename T>
struct Clean2<const T&>
{
    typedef T type;
};

template <typename T>
struct Clean2<T*>
{
    typedef T type;
};

template <typename T>
struct Clean2<const T*>
{
    typedef T type;
};

///////////////////////////////////////////////////////////////////////////
template <typename T>
SF_INLINE
typename Clean<T>::type DefaultValue(StringManager&)
{
    return NULL;
}

template <>
SF_INLINE
void DefaultValue<void>(StringManager&)
{
}

template <>
SF_INLINE
bool DefaultValue<bool>(StringManager&)
{
    return false;
}
#if !defined(SF_CC_GNU) && !defined(SF_CC_SNC) && !defined(SF_OS_WII) && !defined(SF_CC_ARM) && !defined(SF_CC_GHS) && !defined(SF_CC_CLANG)
template <>
SF_INLINE
int DefaultValue<int>(StringManager&)
{
    return 0;
}
#endif
template <>
SF_INLINE
SInt32 DefaultValue<SInt32>(StringManager&)
{
    return 0;
}
#if !defined(SF_CC_GNU) && !defined(SF_CC_SNC) && !defined(SF_OS_WII) && !defined(SF_CC_ARM) && !defined(SF_CC_GHS) && !defined(SF_CC_CLANG)
template <>
SF_INLINE
unsigned int DefaultValue<unsigned int>(StringManager&)
{
    return 0;
}
#endif
template <>
SF_INLINE
UInt32 DefaultValue<UInt32>(StringManager&)
{
    return 0;
}

template <>
SF_INLINE
Value::Number DefaultValue<Value::Number>(StringManager&)
{
    return NumberUtil::NaN();
}
template <>
SF_INLINE
Value::Number DefaultValue<const Value::Number&>(StringManager&)
{
    return NumberUtil::NaN();
}

template <>
SF_INLINE
ASStringNode* DefaultValue<ASStringNode*>(StringManager& sm)
{
    return sm.GetEmptyStringNode();
}

// !!! Do not delete this !!!
template <>
SF_INLINE
String DefaultValue<String>(StringManager&)
{
    return String();
}
template <>
SF_INLINE
String DefaultValue<const String&>(StringManager&)
{
    return String();
}

template <>
SF_INLINE
ASString DefaultValue<ASString>(StringManager& sm)
{
    // No problem with RefCount.
    return sm.CreateEmptyString();
}
template <>
SF_INLINE
ASString DefaultValue<const ASString&>(StringManager& sm)
{
    // No problem with RefCount.
    return sm.CreateEmptyString();
}

template <>
SF_INLINE
Value DefaultValue<Value>(StringManager&)
{
    return Value::GetUndefined();
}
template <>
SF_INLINE
Value DefaultValue<const Value&>(StringManager&)
{
    return Value::GetUndefined();
}

template <>
SF_INLINE
AS3::Object* DefaultValue<AS3::Object*>(StringManager&)
{
    return NULL;
}

template <>
SF_INLINE
Instances::Function* DefaultValue<Instances::Function*>(StringManager&)
{
    return NULL;
}

///////////////////////////////////////////////////////////////////////////
namespace Impl
{

    // C - class
    // M - method id
    // N - arg number
    // T - arg type
    template <class C, int M, int N, typename T>
    SF_INLINE
    typename Clean<T>::type GetMethodDefArg(AS3::StringManager& sm)
    {
        return DefaultValue<T>(sm);
    }

}

///////////////////////////////////////////////////////////////////////////////
namespace Impl
{

    ///////////////////////////////////////////////////////////////////////////
    // Convert<FROM,TO> is a class specialization used to convert Value to
    // is component primitive types and back. This class is used to implement the
    // Convert() template function used for reading argument values Thunks.
    // Typically Value -> type conversions are used for reading arguments,
    // while type -> Value conversion is used for the return type.
    //
    // Specializations such as <Value, ASString>, <Value, unsigned> are defined for
    // each primitive type; furthermore, additional conversions are defined in each
    // each object implementation header, converting Value to Instances::ObjectType
    // through a static cast from Instances::fl::Object.

    template <typename FROM, typename TO>
    class Convert
    {
    };

    template <>
    class Convert<Value, Value>
    {
    public:
        typedef Value TO;
        typedef Value FROM;

    public:
        SF_INLINE
        Convert(TO& to, const FROM& from)
        {
            to.Assign(from);
        }
    };

    template <>
    class Convert<Value, String>
    {
    public:
        typedef String TO;
        typedef Value FROM;

    public:
        SF_INLINE
        Convert(TO& to, const FROM& from)
        {
            StringBuffer buf;
            if (from.Convert2String(buf))
                to.AssignString(buf.ToCStr(), buf.GetSize());
        }
    };

    template <>
    class Convert<Value, ASString>
    {
    public:
        typedef ASString TO;
        typedef Value FROM;

    public:
        SF_INLINE
        Convert(TO& to, const FROM& from)
        {
            if (from.IsNull())
                to.SetNull();
            else
                from.Convert2String(to).DoNotCheck();
        }
    };

    template <>
    class Convert<Value, bool>
    {
    public:
        typedef bool TO;
        typedef Value FROM;

    public:
        SF_INLINE
        Convert(TO& to, const FROM& from)
        {
            to = from.Convert2Boolean();
        }
    };

#if !defined(SF_CC_GNU) && !defined(SF_CC_SNC) && !defined(SF_CC_ARM) && !defined(SF_CC_GHS) && !defined(SF_CC_CLANG)
    template <>
    class Convert<Value, int>
    {
    public:
        typedef int TO;
        typedef Value FROM;

    public:
        SF_INLINE
        Convert(TO& to, const FROM& from)
        {
            SInt32 v;
            if (from.Convert2Int32(v))
                to = v;
        }
    };
#endif

    template <>
    class Convert<Value, SInt32>
    {
    public:
        typedef SInt32 TO;
        typedef Value FROM;

    public:
        SF_INLINE
        Convert(TO& to, const FROM& from)
        {
            from.Convert2Int32(to).DoNotCheck();
        }
    };

#if !defined(SF_CC_GNU) && !defined(SF_CC_SNC) && !defined(SF_CC_ARM) && !defined(SF_CC_GHS) && !defined(SF_CC_CLANG)
    template <>
    class Convert<Value, unsigned int>
    {
    public:
        typedef unsigned int TO;
        typedef Value FROM;

    public:
        SF_INLINE
        Convert(TO& to, const FROM& from)
        {
            UInt32 v;
            if (from.Convert2UInt32(v))
                to = v;
        }
    };
#endif

    template <>
    class Convert<Value, UInt32>
    {
    public:
        typedef UInt32 TO;
        typedef Value FROM;

    public:
        SF_INLINE
        Convert(TO& to, const FROM& from)
        {
            from.Convert2UInt32(to).DoNotCheck();
        }
    };

    template <>
    class Convert<Value, Value::Number>
    {
    public:
        typedef Value::Number TO;
        typedef Value FROM;

    public:
        SF_INLINE
        Convert(TO& to, const FROM& from)
        {
            from.Convert2Number(to).DoNotCheck();
        }
    };

    template <class T>
    class Convert<Value, T>
    {
    public:
        typedef T TO;
        typedef Value FROM;

    public:
        SF_INLINE
        Convert(TO& to, const FROM& from)
        {
            // This *if* statement is required only in case when REPORT_NOT_IMPLEMENTED
            // is not defined.
            if (from.IsUndefined())
                to = TO();
            else
                to = (TO)(from.GetObject());
        }
    };

    template <class T>
    class Convert<Value, SPtr<T> >
    {
    public:
        typedef SPtr<T> TO;
        typedef Value FROM;

    public:
        SF_INLINE
        Convert(TO& to, const FROM& from)
        {
            // This *if* statement is required only in case when REPORT_NOT_IMPLEMENTED
            // is not defined.
            if (from.IsUndefined())
                to = TO();
            else
                to.SetPtr(static_cast<T*>(from.GetObject()));
        }
    };

    template <typename FROM>
    class Convert<FROM, Value>
    {
    public:
        typedef Value TO;

    public:
        SF_INLINE
        Convert(TO& to, FROM from)
        {
            to.Assign(from);
        }
    };

    template <>
    class Convert<bool, Value>
    {
    public:
        typedef Value TO;
        typedef bool FROM;

    public:
        SF_INLINE
        Convert(TO& to, const FROM& from)
        {
            to.SetBool(from);
        }
    };

    template <>
    class Convert<SInt32, Value>
    {
    public:
        typedef Value TO;
        typedef SInt32 FROM;

    public:
        SF_INLINE
        Convert(TO& to, const FROM& from)
        {
            to.SetSInt32(from);
        }
    };

    template <>
    class Convert<UInt32, Value>
    {
    public:
        typedef Value TO;
        typedef UInt32 FROM;

    public:
        SF_INLINE
        Convert(TO& to, const FROM& from)
        {
            to.SetUInt32(from);
        }
    };

    template <>
    class Convert<Value::Number, Value>
    {
    public:
        typedef Value TO;
        typedef Value::Number FROM;

    public:
        SF_INLINE
        Convert(TO& to, const FROM& from)
        {
            to.SetNumber(from);
        }
    };

    template <>
    class Convert<ASString, Value>
    {
    public:
        typedef Value TO;
        typedef ASString FROM;

    public:
        SF_INLINE
        Convert(TO& to, const FROM& from)
        {
            to.Assign(from);
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // Unsafe version of Convert.

    template <typename FROM, typename TO>
    class ConvertUnsafe
    {
    };

    template <>
    class ConvertUnsafe<Value, Value>
    {
    public:
        typedef Value TO;
        typedef Value FROM;

    public:
        SF_INLINE
        ConvertUnsafe(TO& to, const FROM& from)
        {
            to.AssignUnsafe(from);
        }
    };

    template <>
    class ConvertUnsafe<Value, String>
    {
    public:
        typedef String TO;
        typedef Value FROM;

    public:
        SF_INLINE
        ConvertUnsafe(TO& to, const FROM& from)
        {
            StringBuffer buf;
            if (from.Convert2String(buf))
                to.AssignString(buf.ToCStr(), buf.GetSize());
        }
    };

    template <>
    class ConvertUnsafe<Value, ASString>
    {
    public:
        typedef ASString TO;
        typedef Value FROM;

    public:
        SF_INLINE
        ConvertUnsafe(TO& to, const FROM& from)
        {
            if (from.IsNull())
                to.SetNull();
            else
                from.Convert2String(to).DoNotCheck();
        }
    };

    template <>
    class ConvertUnsafe<Value, bool>
    {
    public:
        typedef bool TO;
        typedef Value FROM;

    public:
        SF_INLINE
        ConvertUnsafe(TO& to, const FROM& from)
        {
            to = from.Convert2Boolean();
        }
    };

#if !defined(SF_CC_GNU) && !defined(SF_CC_SNC) && !defined(SF_CC_ARM) && !defined(SF_CC_GHS) && !defined(SF_CC_CLANG)
    template <>
    class ConvertUnsafe<Value, int>
    {
    public:
        typedef int TO;
        typedef Value FROM;

    public:
        SF_INLINE
        ConvertUnsafe(TO& to, const FROM& from)
        {
            SInt32 v;
            if (from.Convert2Int32(v))
                to = v;
        }
    };
#endif

    template <>
    class ConvertUnsafe<Value, SInt32>
    {
    public:
        typedef SInt32 TO;
        typedef Value FROM;

    public:
        SF_INLINE
        ConvertUnsafe(TO& to, const FROM& from)
        {
            from.Convert2Int32(to).DoNotCheck();
        }
    };

#if !defined(SF_CC_GNU) && !defined(SF_CC_SNC) && !defined(SF_CC_ARM) && !defined(SF_CC_GHS) && !defined(SF_CC_CLANG)
    template <>
    class ConvertUnsafe<Value, unsigned int>
    {
    public:
        typedef unsigned int TO;
        typedef Value FROM;

    public:
        SF_INLINE
        ConvertUnsafe(TO& to, const FROM& from)
        {
            UInt32 v;
            if (from.Convert2UInt32(v))
                to = v;
        }
    };
#endif

    template <>
    class ConvertUnsafe<Value, UInt32>
    {
    public:
        typedef UInt32 TO;
        typedef Value FROM;

    public:
        SF_INLINE
        ConvertUnsafe(TO& to, const FROM& from)
        {
            from.Convert2UInt32(to).DoNotCheck();
        }
    };

    template <>
    class ConvertUnsafe<Value, Value::Number>
    {
    public:
        typedef Value::Number TO;
        typedef Value FROM;

    public:
        SF_INLINE
        ConvertUnsafe(TO& to, const FROM& from)
        {
            from.Convert2Number(to).DoNotCheck();
        }
    };

    template <class T>
    class ConvertUnsafe<Value, T>
    {
    public:
        typedef T TO;
        typedef Value FROM;

    public:
        SF_INLINE
        ConvertUnsafe(TO& to, const FROM& from)
        {
            // This *if* statement is required only in case when REPORT_NOT_IMPLEMENTED
            // is not defined.
            if (from.IsUndefined())
                to = T();
            else
                to = (TO)(from.GetObject());
        }
    };

    template <typename FROM>
    class ConvertUnsafe<FROM, Value>
    {
    public:
        typedef Value TO;

    public:
        SF_INLINE
        ConvertUnsafe(TO& to, FROM from)
        {
            to.AssignUnsafe(from);
        }
    };

    template <>
    class ConvertUnsafe<bool, Value>
    {
    public:
        typedef Value TO;
        typedef bool FROM;

    public:
        SF_INLINE
        ConvertUnsafe(TO& to, const FROM& from)
        {
            to.SetBoolUnsafe(from);
        }
    };

    template <>
    class ConvertUnsafe<SInt32, Value>
    {
    public:
        typedef Value TO;
        typedef SInt32 FROM;

    public:
        SF_INLINE
        ConvertUnsafe(TO& to, const FROM& from)
        {
            to.SetSInt32Unsafe(from);
        }
    };

    template <>
    class ConvertUnsafe<UInt32, Value>
    {
    public:
        typedef Value TO;
        typedef UInt32 FROM;

    public:
        SF_INLINE
        ConvertUnsafe(TO& to, const FROM& from)
        {
            to.SetUInt32Unsafe(from);
        }
    };

    template <>
    class ConvertUnsafe<Value::Number, Value>
    {
    public:
        typedef Value TO;
        typedef Value::Number FROM;

    public:
        SF_INLINE
        ConvertUnsafe(TO& to, const FROM& from)
        {
            to.SetNumberUnsafe(from);
        }
    };

    template <>
    class ConvertUnsafe<ASString, Value>
    {
    public:
        typedef Value TO;
        typedef ASString FROM;

    public:
        SF_INLINE
        ConvertUnsafe(TO& to, const FROM& from)
        {
            to.AssignUnsafe(from);
        }
    };

    ///////////////////////////////////////////////////////////////////////
    template <typename FROM, typename TO>
    class Coerce
    {
    public:
        SF_INLINE Coerce(VM& /*vm*/, TO& /*to*/, const FROM& /*from*/)
        {
            SF_ASSERT(false);
        }
    };

    template <>
    class Coerce<Value, Value>
    {
    public:
        typedef Value TO;
        typedef Value FROM;

    public:
        SF_INLINE Coerce(VM& /*vm*/, TO& to, const FROM& from)
        {
            to.Assign(from);
        }
    };

    template <typename TO>
    class Coerce<VObject, TO>
    {
    public:
        typedef VObject FROM;

    public:
        SF_INLINE Coerce(VM& vm, TO& to, const FROM& from)
        {
            Coerce(vm, to, static_cast<const Value&>(from));
        }
    };

    template <>
    class Coerce<Value, String>
    {
    public:
        typedef String TO;
        typedef Value FROM;

    public:
        SF_INLINE Coerce(VM& /*vm*/, TO& to, const FROM& from)
        {
            StringBuffer buf;
            if (from.Convert2String(buf))
                to.AssignString(buf.ToCStr(), buf.GetSize());
        }
    };

    template <>
    class Coerce<Value, ASString>
    {
    public:
        typedef ASString TO;
        typedef Value FROM;

    public:
        SF_INLINE Coerce(VM& /*vm*/, TO& to, const FROM& from)
        {
            if (from.IsNull())
                to.SetNull();
            else
                from.Convert2String(to).DoNotCheck();
        }
    };

    template <>
    class Coerce<Value, bool>
    {
    public:
        typedef bool TO;
        typedef Value FROM;

    public:
        SF_INLINE Coerce(VM& /*vm*/, TO& to, const FROM& from)
        {
            to = from.Convert2Boolean();
        }
    };

#if !defined(SF_CC_GNU) && !defined(SF_CC_SNC) && !defined(SF_CC_ARM) && !defined(SF_CC_GHS) && !defined(SF_CC_CLANG)
    template <>
    class Coerce<Value, int>
    {
    public:
        typedef int TO;
        typedef Value FROM;

    public:
        SF_INLINE Coerce(VM& /*vm*/, TO& to, const FROM& from)
        {
            SInt32 v;
            if (from.Convert2Int32(v))
                to = v;
        }
    };
#endif

    template <>
    class Coerce<Value, SInt32>
    {
    public:
        typedef SInt32 TO;
        typedef Value FROM;

    public:
        SF_INLINE Coerce(VM& /*vm*/, TO& to, const FROM& from)
        {
            from.Convert2Int32(to).DoNotCheck();
        }
    };

#if !defined(SF_CC_GNU) && !defined(SF_CC_SNC) && !defined(SF_CC_ARM) && !defined(SF_CC_GHS) && !defined(SF_CC_CLANG)
    template <>
    class Coerce<Value, unsigned int>
    {
    public:
        typedef unsigned int TO;
        typedef Value FROM;

    public:
        SF_INLINE Coerce(VM& /*vm*/, TO& to, const FROM& from)
        {
            UInt32 v;
            if (from.Convert2UInt32(v))
                to = v;
        }
    };
#endif

    template <>
    class Coerce<Value, UInt32>
    {
    public:
        typedef UInt32 TO;
        typedef Value FROM;

    public:
        SF_INLINE Coerce(VM& /*vm*/, TO& to, const FROM& from)
        {
            from.Convert2UInt32(to).DoNotCheck();
        }
    };

    template <>
    class Coerce<Value, Value::Number>
    {
    public:
        typedef Value::Number TO;
        typedef Value FROM;

    public:
        SF_INLINE Coerce(VM& /*vm*/, TO& to, const FROM& from)
        {
            from.Convert2Number(to).DoNotCheck();
        }
    };

    void CoerceInternal(VM& vm, const TypeInfo& ti, Value& to, const Value& from);

    template <>
    class Coerce<Value, Instances::fl::Object*>
    {
    public:
        typedef Instances::fl::Object* TO;
        typedef Value FROM;

    public:
        SF_INLINE Coerce(VM& vm, TO& to, const FROM& from)
        {
            Value r;
            if (vm.GetClassTraitsObject().Coerce(from, r))
                to = static_cast<TO>(r.GetObject());
        }
    };

    template <>
    class Coerce<Value, Instances::fl::Namespace*>
    {
    public:
        typedef Instances::fl::Namespace* TO;
        typedef Value FROM;

    public:
        SF_INLINE Coerce(VM& vm, TO& to, const FROM& from)
        {
            Value r;
            if (vm.GetClassTraitsNamespace().Coerce(from, r))
                to = static_cast<TO>(r.GetNamespace());
        }
    };

    template <>
    class Coerce<Value, Instances::Function*>
    {
    public:
        typedef Instances::Function* TO;
        typedef Value FROM;

    public:
        SF_INLINE Coerce(VM& vm, TO& to, const FROM& from)
        {
            Value r;
            if (vm.GetClassTraitsFunction().Coerce(from, r))
                to = static_cast<TO>(&r.AsFunction());
        }
    };

    template <>
    class Coerce<Value, VObject>
    {
    public:
        typedef VObject TO;
        typedef Value FROM;

    public:
        SF_INLINE Coerce(VM& vm, TO& to, const FROM& from)
        {
            vm.GetClassTraitsObject().Coerce(from, to);
        }
    };

    template <class T>
    class Coerce<Value, T>
    {
    public:
        typedef T TO;
        typedef Value FROM;

    public:
        SF_INLINE Coerce(VM& vm, TO& to, const FROM& from)
        {
#if 0
            SF_UNUSED1(vm);
            // This *if* statement is required only in case when REPORT_NOT_IMPLEMENTED
            // is not defined.
            if (from.IsUndefined())
                to = TO();
            else
                to = static_cast<TO>(from.GetObject());
#else
            Value r;
            CoerceInternal(vm, Clean2<T>::type::GetTypeInfo(), r, from);
            to = static_cast<TO>(r.GetObject());
#endif
        }
    };

    template <>
    class Coerce<Value, SPtr<Instances::fl::Object> >
    {
    public:
        typedef SPtr<Instances::fl::Object> TO;
        typedef Value FROM;

    public:
        SF_INLINE Coerce(VM& vm, TO& to, const FROM& from)
        {
            Value r;
            if (vm.GetClassTraitsObject().Coerce(from, r))
                to.SetPtr(static_cast<Instances::fl::Object*>(r.GetObject()));
        }
    };

    template <>
    class Coerce<Value, SPtr<Instances::fl::Namespace> >
    {
    public:
        typedef SPtr<Instances::fl::Namespace> TO;
        typedef Value FROM;

    public:
        SF_INLINE Coerce(VM& vm, TO& to, const FROM& from)
        {
            Value r;
            if (vm.GetClassTraitsNamespace().Coerce(from, r))
                to.SetPtr(static_cast<Instances::fl::Namespace*>(r.GetNamespace()));
        }
    };

    template <>
    class Coerce<Value, SPtr<Instances::Function> >
    {
    public:
        typedef SPtr<Instances::Function> TO;
        typedef Value FROM;

    public:
        SF_INLINE Coerce(VM& vm, TO& to, const FROM& from)
        {
            Value r;
            if (vm.GetClassTraitsFunction().Coerce(from, r))
                to.SetPtr(static_cast<Instances::Function*>(&r.AsFunction()));
        }
    };

    template <class T>
    class Coerce<Value, SPtr<T> >
    {
    public:
        typedef SPtr<T> TO;
        typedef Value FROM;

    public:
        SF_INLINE Coerce(VM& vm, TO& to, const FROM& from)
        {
#if 0
            SF_UNUSED1(vm);
            // This *if* statement is required only in case when REPORT_NOT_IMPLEMENTED
            // is not defined.
            if (from.IsUndefined())
                to = TO();
            else
                to.SetPtr(static_cast<T*>(from.GetObject()));
#else
            Value r;
            CoerceInternal(vm, Clean2<T>::type::GetTypeInfo(), r, from);
            to.SetPtr(static_cast<T*>(r.GetObject()));
#endif
        }
    };

    template <typename FROM>
    class Coerce<FROM, Value>
    {
    public:
        typedef Value TO;

    public:
        SF_INLINE Coerce(VM& /*vm*/, TO& to, FROM from)
        {
            to.Assign(from);
        }
    };

    template <typename FROM>
    class Coerce<FROM, VObject>
    {
    public:
        typedef VObject TO;

    public:
        SF_INLINE Coerce(VM& vm, TO& to, FROM from)
        {
            Coerce(vm, static_cast<Value&>(to), from);
        }
    };

    template <>
    class Coerce<bool, Value>
    {
    public:
        typedef Value TO;
        typedef bool FROM;

    public:
        SF_INLINE Coerce(VM& /*vm*/, TO& to, const FROM& from)
        {
            to.SetBool(from);
        }
    };

    template <>
    class Coerce<SInt32, Value>
    {
    public:
        typedef Value TO;
        typedef SInt32 FROM;

    public:
        SF_INLINE Coerce(VM& /*vm*/, TO& to, const FROM& from)
        {
            to.SetSInt32(from);
        }
    };

    template <>
    class Coerce<UInt32, Value>
    {
    public:
        typedef Value TO;
        typedef UInt32 FROM;

    public:
        SF_INLINE Coerce(VM& /*vm*/, TO& to, const FROM& from)
        {
            to.SetUInt32(from);
        }
    };

    template <>
    class Coerce<Value::Number, Value>
    {
    public:
        typedef Value TO;
        typedef Value::Number FROM;

    public:
        SF_INLINE Coerce(VM& /*vm*/, TO& to, const FROM& from)
        {
            to.SetNumber(from);
        }
    };

    template <>
    class Coerce<ASString, Value>
    {
    public:
        typedef Value TO;
        typedef ASString FROM;

    public:
        SF_INLINE Coerce(VM& /*vm*/, TO& to, const FROM& from)
        {
            to.Assign(from);
        }
    };

    bool ConvertTO(VM& vm, Value& to, const Value& from, const TypeInfo& ti);
}

///////////////////////////////////////////////////////////////////////////
template <typename TO, typename FROM>
SF_INLINE
void Convert(TO& to, const FROM& from)
{
    Impl::Convert<FROM, TO>(to, from);
}

template <typename TO, typename FROM>
SF_INLINE
void ConvertUnsafe(TO& to, const FROM& from)
{
    Impl::ConvertUnsafe<FROM, TO>(to, from);
}

///////////////////////////////////////////////////////////////////////////
template <typename TO, typename FROM>
SF_INLINE
void Convert(VM& vm, TO& to, const FROM& from)
{
    Impl::Coerce<FROM, TO>(vm, to, from);
}

// template <typename TO, typename FROM>
// SF_INLINE
// void ConvertUnsafe(VM& vm, TO& to, const FROM& from)
// {
//     Impl::Coerce<FROM, TO>(vm, to, from);
// }

///////////////////////////////////////////////////////////////////////////
#ifdef SF_AS3_AOTC2

class VMAppDomain;
struct TypeInfo;

namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const TypeInfo int_TI;
    extern const TypeInfo uintTI;
    extern const TypeInfo NumberTI;
    extern const TypeInfo StringTI;
} // namespace fl

template <const TypeInfo& ti>
SF_INLINE CheckResult ConvertTO(VM& vm, Value& to, const Value& from, VMAppDomain& appDomain)
{
    return Impl::ConvertTO(vm, to, from, appDomain, ti);
}

template <>
SF_INLINE CheckResult ConvertTO<fl::BooleanTI>(VM& vm, Value& to, const Value& from, VMAppDomain& appDomain)
{
    SF_UNUSED2(vm, appDomain);

    to = from;
    to.ToBooleanValue();
    return true;
}

template <>
SF_INLINE CheckResult ConvertTO<fl::int_TI>(VM& vm, Value& to, const Value& from, VMAppDomain& appDomain)
{
    SF_UNUSED2(vm, appDomain);

    to = from;
    return to.ToInt32Value();
}

template <>
SF_INLINE CheckResult ConvertTO<fl::uintTI>(VM& vm, Value& to, const Value& from, VMAppDomain& appDomain)
{
    SF_UNUSED2(vm, appDomain);

    to = from;
    return to.ToUInt32Value();
}

template <>
SF_INLINE CheckResult ConvertTO<fl::NumberTI>(VM& vm, Value& to, const Value& from, VMAppDomain& appDomain)
{
    SF_UNUSED2(vm, appDomain);

    to = from;
    return to.ToNumberValue();
}

template <>
SF_INLINE CheckResult ConvertTO<fl::StringTI>(VM& vm, Value& to, const Value& from, VMAppDomain& appDomain)
{
    SF_UNUSED1(appDomain);

    to = from;
    return to.ToStringValue(vm.GetStringManager());
}

#endif

///////////////////////////////////////////////////////////////////////////
#if 0
template <typename TO>
SF_INLINE
void ConvertValue(Value& /*to*/, StringManager& /*sm*/)
{
}

template <>
SF_INLINE
void ConvertValue<bool>(Value& to, StringManager& /*sm*/)
{
    to.ToBooleanValue();
}

#if !defined(SF_CC_GNU) && !defined(SF_CC_SNC) && !defined(SF_CC_ARM) && !defined(SF_CC_GHS) && !defined(SF_CC_CLANG)
template <>
SF_INLINE
void ConvertValue<int>(Value& to, StringManager& /*sm*/)
{
    to.ToInt32Value().DoNotCheck();
}

template <>
SF_INLINE
void ConvertValue<unsigned int>(Value& to, StringManager& /*sm*/)
{
    to.ToUInt32Value().DoNotCheck();
}
#endif

template <>
SF_INLINE
void ConvertValue<SInt32>(Value& to, StringManager& /*sm*/)
{
    to.ToInt32Value().DoNotCheck();
}

template <>
SF_INLINE
void ConvertValue<UInt32>(Value& to, StringManager& /*sm*/)
{
    to.ToUInt32Value().DoNotCheck();
}

template <>
SF_INLINE
void ConvertValue<Value::Number>(Value& to, StringManager& /*sm*/)
{
    to.ToNumberValue().DoNotCheck();
}

template <>
SF_INLINE
void ConvertValue<ASString>(Value& to, StringManager& sm)
{
    to.ToStringValue(sm).DoNotCheck();
}
#endif

///////////////////////////////////////////////////////////////////////////

// ThunkFunk0-6 are thunk function generator classes.
// The actual implementation of static Func() is provided in VM.h; its
// implementation is responsible for reading the arguments from argv and
// calling the pointer-to-member.

template <typename T, size_t N, typename R>
struct ThunkFunc0
{
    typedef void (T::*TMethod)(R&);

    static void Func(const ThunkInfo& ti, VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv);
    static const TMethod Method;
};

template <typename T, size_t N, typename R> 
const typename ThunkFunc0<T, N, R>::TMethod ThunkFunc0<T, N, R>::Method = NULL;

///////////////////////////////////////////////////////////////////////////
template <typename T, size_t N, typename R, typename A0>
struct ThunkFunc1
{
    typedef void (T::*TMethod)(R&, A0);

    static void Func(const ThunkInfo& ti, VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv);
    static const TMethod Method;
};

template <typename T, size_t N, typename R, typename A0> 
const typename ThunkFunc1<T, N, R, A0>::TMethod ThunkFunc1<T, N, R, A0>::Method = NULL;

///////////////////////////////////////////////////////////////////////////
template <typename T, size_t N, typename R, typename A0, typename A1>
struct ThunkFunc2
{
    typedef void (T::*TMethod)(R&, A0, A1);

    static void Func(const ThunkInfo& ti, VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv);
    static const TMethod Method;
};

template <typename T, size_t N, typename R, typename A0, typename A1> 
const typename ThunkFunc2<T, N, R, A0, A1>::TMethod ThunkFunc2<T, N, R, A0, A1>::Method = NULL;

// Specialization for variable-argument length function call.
template <typename T, size_t N>
struct ThunkFunc2<T, N, Value, unsigned, const Value*>
{
    typedef void (T::*TMethod)(Value&, unsigned, const Value*);

//     SF_INLINE
    static void Func(const ThunkInfo& /*ti*/, VM& /*vm*/, const Value& obj, Value& result, unsigned argc, const Value* argv)
    {
        T& _this = static_cast<T&>(*obj.GetObject());

        (_this.*Method)(result, argc, argv);
    }

    static const TMethod Method;
};

template <typename T, size_t N> 
const typename ThunkFunc2<T, N, Value, unsigned, const Value*>::TMethod ThunkFunc2<T, N, Value, unsigned, const Value*>::Method = NULL;

///////////////////////////////////////////////////////////////////////////
template <typename T, size_t N, typename R, typename A0, typename A1, typename A2>
struct ThunkFunc3
{
    typedef void (T::*TMethod)(R&, A0, A1, A2);

    static void Func(const ThunkInfo& ti, VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv);
    static const TMethod Method;
};

template <typename T, size_t N, typename R, typename A0, typename A1, typename A2> 
const typename ThunkFunc3<T, N, R, A0, A1, A2>::TMethod
ThunkFunc3<T, N, R, A0, A1, A2>::Method = NULL;

///////////////////////////////////////////////////////////////////////////
template <typename T, size_t N, typename R, typename A0, typename A1, typename A2, typename A3>
struct ThunkFunc4
{
    typedef void (T::*TMethod)(R&, A0, A1, A2, A3);

    static void Func(const ThunkInfo& ti, VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv);
    static const TMethod Method;
};

template <typename T, size_t N, typename R, typename A0, typename A1, typename A2, typename A3> 
const typename ThunkFunc4<T, N, R, A0, A1, A2, A3>::TMethod
ThunkFunc4<T, N, R, A0, A1, A2, A3>::Method = NULL;

///////////////////////////////////////////////////////////////////////////
template <typename T, size_t N, typename R, typename A0, typename A1, typename A2,
    typename A3, typename A4>
struct ThunkFunc5
{
    typedef void (T::*TMethod)(R&, A0, A1, A2, A3, A4);

    static void Func(const ThunkInfo& ti, VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv);
    static const TMethod Method;
};

template <typename T, size_t N, typename R, typename A0, typename A1, typename A2,
typename A3, typename A4> 
const typename ThunkFunc5<T, N, R, A0, A1, A2, A3, A4>::TMethod
ThunkFunc5<T, N, R, A0, A1, A2, A3, A4>::Method = NULL;

///////////////////////////////////////////////////////////////////////////
template <typename T, size_t N, typename R, typename A0, typename A1, typename A2,
    typename A3, typename A4, typename A5>
struct ThunkFunc6
{
    typedef void (T::*TMethod)(R&, A0, A1, A2, A3, A4, A5);

    static void Func(const ThunkInfo& ti, VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv);
    static const TMethod Method;
};

template <typename T, size_t N, typename R, typename A0, typename A1, typename A2,
typename A3, typename A4, typename A5> 
const typename ThunkFunc6<T, N, R, A0, A1, A2, A3, A4, A5>::TMethod
ThunkFunc6<T, N, R, A0, A1, A2, A3, A4, A5>::Method = NULL;

///////////////////////////////////////////////////////////////////////////
////

template <typename A0>
struct DefArgs1
{
    SF_INLINE
    DefArgs1(A0 a0 = A0())
        : _0(a0)
    {
    }
    DefArgs1& operator=(const DefArgs1&);

    typename Clean<A0>::type _0;
};

template <typename A0, typename A1>
struct DefArgs2 : public DefArgs1<A0>
{
    SF_INLINE
    DefArgs2(A0 a0 = A0(), A1 a1 = A1())
        : DefArgs1<A0>(a0)
        , _1(a1)
    {
    }
    DefArgs2& operator=(const DefArgs2&);

    typename Clean<A1>::type _1;
};

// A stub for variable number of arguments.
template <>
struct DefArgs2<unsigned, const Value*>
{
    SF_INLINE
    DefArgs2(unsigned, const Value*)
    {
    }
    DefArgs2& operator=(const DefArgs2&);
};

template <typename A0, typename A1, typename A2>
struct DefArgs3 : public DefArgs2<A0, A1>
{
    SF_INLINE
    DefArgs3(A0 a0 = A0(), A1 a1 = A1(), A2 a2 = A2())
        : DefArgs2<A0, A1>(a0, a1)
        , _2(a2)
    {
    }
    DefArgs3& operator=(const DefArgs3&);

    typename Clean<A2>::type _2;
};

template <typename A0, typename A1, typename A2, typename A3>
struct DefArgs4 : public DefArgs3<A0, A1, A2>
{
    SF_INLINE
    DefArgs4(A0 a0 = A0(), A1 a1 = A1(), A2 a2 = A2(), A3 a3 = A3())
        : DefArgs3<A0, A1, A2>(a0, a1, a2)
        , _3(a3)
    {
    }
    DefArgs4& operator=(const DefArgs4&);

    typename Clean<A3>::type _3;
};

template <typename A0, typename A1, typename A2, typename A3, typename A4>
struct DefArgs5 : public DefArgs4<A0, A1, A2, A3>
{
    SF_INLINE
    DefArgs5(A0 a0 = A0(), A1 a1 = A1(), A2 a2 = A2(), A3 a3 = A3(), A4 a4 = A4())
        : DefArgs4<A0, A1, A2, A3>(a0, a1, a2, a3)
        , _4(a4)
    {
    }
    DefArgs5& operator=(const DefArgs5&);

    typename Clean<A4>::type _4;
};

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
struct DefArgs6 : public DefArgs5<A0, A1, A2, A3, A4>
{
    SF_INLINE
    DefArgs6(A0 a0 = A0(), A1 a1 = A1(), A2 a2 = A2(), A3 a3 = A3(), A4 a4 = A4(), A5 a5 = A5())
        : DefArgs5<A0, A1, A2, A3, A4>(a0, a1, a2, a3, a4)
        , _5(a5)
    {
    }
    DefArgs6& operator=(const DefArgs6&);

    typename Clean<A5>::type _5;
};


///////////////////////////////////////////////////////////////////////////
// Forcing inline for the functions below generates a lot of code.

template <class C, int M, typename A0>
inline
DefArgs1<A0> MakeDefArgs(AS3::StringManager& sm)
{
    return DefArgs1<A0>(
        Impl::GetMethodDefArg<C, M, 0, A0>(sm)
        );
}

template <class C, int M, typename A0, typename A1>
inline
DefArgs2<A0, A1> MakeDefArgs(AS3::StringManager& sm)
{
    return DefArgs2<A0, A1>(
        Impl::GetMethodDefArg<C, M, 0, A0>(sm),
        Impl::GetMethodDefArg<C, M, 1, A1>(sm)
        );
}

template <class C, int M, typename A0, typename A1, typename A2>
inline
DefArgs3<A0, A1, A2> MakeDefArgs(AS3::StringManager& sm)
{
    return DefArgs3<A0, A1, A2>(
        Impl::GetMethodDefArg<C, M, 0, A0>(sm),
        Impl::GetMethodDefArg<C, M, 1, A1>(sm),
        Impl::GetMethodDefArg<C, M, 2, A2>(sm)
        );
}

template <class C, int M, typename A0, typename A1, typename A2, typename A3>
inline
DefArgs4<A0, A1, A2, A3> MakeDefArgs(AS3::StringManager& sm)
{
    return DefArgs4<A0, A1, A2, A3>(
        Impl::GetMethodDefArg<C, M, 0, A0>(sm),
        Impl::GetMethodDefArg<C, M, 1, A1>(sm),
        Impl::GetMethodDefArg<C, M, 2, A2>(sm),
        Impl::GetMethodDefArg<C, M, 3, A3>(sm)
        );
}

template <class C, int M, typename A0, typename A1, typename A2, typename A3, typename A4>
inline
DefArgs5<A0, A1, A2, A3, A4> MakeDefArgs(AS3::StringManager& sm)
{
    return DefArgs5<A0, A1, A2, A3, A4>(
        Impl::GetMethodDefArg<C, M, 0, A0>(sm),
        Impl::GetMethodDefArg<C, M, 1, A1>(sm),
        Impl::GetMethodDefArg<C, M, 2, A2>(sm),
        Impl::GetMethodDefArg<C, M, 3, A3>(sm),
        Impl::GetMethodDefArg<C, M, 4, A4>(sm)
        );
}

template <class C, int M, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
inline
DefArgs6<A0, A1, A2, A3, A4, A5> MakeDefArgs(AS3::StringManager& sm)
{
    return DefArgs6<A0, A1, A2, A3, A4, A5>(
        Impl::GetMethodDefArg<C, M, 0, A0>(sm),
        Impl::GetMethodDefArg<C, M, 1, A1>(sm),
        Impl::GetMethodDefArg<C, M, 2, A2>(sm),
        Impl::GetMethodDefArg<C, M, 3, A3>(sm),
        Impl::GetMethodDefArg<C, M, 4, A4>(sm),
        Impl::GetMethodDefArg<C, M, 5, A5>(sm)
        );
}

///////////////////////////////////////////////////////////////////////////
// Template class does not depend on T and N template arguments.

#ifdef SF_AS3_EMIT_DEF_ARGS

template <typename R>
class UnboxArgV0
{
public:
    SF_INLINE
    UnboxArgV0(VM& vm, Value& result)
    : Vm(vm)
    , Result(result)
    , r(DefaultValue<R>(GetStringManager()))
    {
    }
    SF_INLINE
    ~UnboxArgV0()
    {
        if (GetVM().IsException())
            return;

        AS3::ConvertUnsafe(Result, r);
    }

public:
    VM& GetVM() const
    {
        return Vm;
    }
    SF_INLINE
    StringManager& GetStringManager() const
    {
        return GetVM().GetStringManager();
    }

private:
    UnboxArgV0& operator =(const UnboxArgV0&);

private:
    VM& Vm;

public:
    Value& Result;
    R r;
};

template <>
class UnboxArgV0<Value>
{
public:
    SF_INLINE
    UnboxArgV0(VM& vm, Value& result)
    : Vm(vm)
    , r(result)
    {
    }
    SF_INLINE
    ~UnboxArgV0()
    {
        // No conversion is necessary.
    }

public:
    VM& GetVM() const
    {
        return Vm;
    }
    SF_INLINE
    StringManager& GetStringManager() const
    {
        return GetVM().GetStringManager();
    }

private:
    UnboxArgV0& operator =(const UnboxArgV0&);

private:
    VM& Vm;

public:
    Value& r;
};

template <>
class UnboxArgV0<const Value>
{
public:
    SF_INLINE
    UnboxArgV0(VM& vm, Value& result)
    : Vm(vm)
    , r(result)
    {
    }
    SF_INLINE
    ~UnboxArgV0()
    {
        // No conversion is necessary.
    }

public:
    VM& GetVM() const
    {
        return Vm;
    }
    SF_INLINE
    StringManager& GetStringManager() const
    {
        return GetVM().GetStringManager();
    }

private:
    UnboxArgV0& operator =(const UnboxArgV0&);

private:
    VM& Vm;

public:
    const Value& r;
};

////
template <typename R, typename A0>
class UnboxArgV1 : public UnboxArgV0<R>
{
public:
    SF_INLINE
    UnboxArgV1(VM& vm, Value& result, const Value* argv)
    : UnboxArgV0<R>(vm, result)
    , a0(DefaultValue<typename Clean<A0>::type>(vm.GetStringManager()))
    {
        AS3::Convert(vm, a0, argv[0]);
    }

private:
    UnboxArgV1& operator =(const UnboxArgV1&);

public:
    typename Clean<A0>::type a0;
};

template <typename R>
class UnboxArgV1<R, Value> : public UnboxArgV0<R>
{
public:
    SF_INLINE
    UnboxArgV1(VM& vm, Value& result, const Value* argv)
    : UnboxArgV0<R>(vm, result)
    , a0(argv[0])
    {
    }

private:
    UnboxArgV1& operator =(const UnboxArgV1&);

public:
    const Value& a0;
};

template <typename R>
class UnboxArgV1<R, const Value&> : public UnboxArgV0<R>
{
public:
    SF_INLINE
    UnboxArgV1(VM& vm, Value& result, const Value* argv)
    : UnboxArgV0<R>(vm, result)
    , a0(argv[0])
    {
    }

private:
    UnboxArgV1& operator =(const UnboxArgV1&);

public:
    const Value& a0;
};

////
template <typename R, typename A0, typename A1>
class UnboxArgV2 : public UnboxArgV1<R, A0>
{
public:
    UnboxArgV2(VM& vm, Value& result, const Value* argv)
    : UnboxArgV1<R, A0>(vm, result, argv)
    , a1(DefaultValue<typename Clean<A1>::type>(vm.GetStringManager()))
    {
        AS3::Convert(vm, a1, argv[1]);
    }

private:
    UnboxArgV2& operator =(const UnboxArgV2&);

public:
    typename Clean<A1>::type a1;
};

template <typename R, typename A0>
class UnboxArgV2<R, A0, Value> : public UnboxArgV1<R, A0>
{
public:
    UnboxArgV2(VM& vm, Value& result, const Value* argv)
    : UnboxArgV1<R, A0>(vm, result, argv)
    , a1(argv[1])
    {
    }

private:
    UnboxArgV2& operator =(const UnboxArgV2&);

public:
    const Value& a1;
};

template <typename R, typename A0>
class UnboxArgV2<R, A0, const Value&> : public UnboxArgV1<R, A0>
{
public:
    UnboxArgV2(VM& vm, Value& result, const Value* argv)
    : UnboxArgV1<R, A0>(vm, result, argv)
    , a1(argv[1])
    {
    }

private:
    UnboxArgV2& operator =(const UnboxArgV2&);

public:
    const Value& a1;
};

////
template <typename R, typename A0, typename A1, typename A2>
class UnboxArgV3 : public UnboxArgV2<R, A0, A1>
{
public:
    UnboxArgV3(VM& vm, Value& result, const Value* argv)
    : UnboxArgV2<R, A0, A1>(vm, result, argv)
    , a2(DefaultValue<typename Clean<A2>::type>(vm.GetStringManager()))
    {
        AS3::Convert(vm, a2, argv[2]);
    }

private:
    UnboxArgV3& operator =(const UnboxArgV3&);

public:
    typename Clean<A2>::type a2;
};

template <typename R, typename A0, typename A1>
class UnboxArgV3<R, A0, A1, Value> : public UnboxArgV2<R, A0, A1>
{
public:
    UnboxArgV3(VM& vm, Value& result, const Value* argv)
    : UnboxArgV2<R, A0, A1>(vm, result, argv)
    , a2(argv[2])
    {
    }

private:
    UnboxArgV3& operator =(const UnboxArgV3&);

public:
    const Value& a2;
};

template <typename R, typename A0, typename A1>
class UnboxArgV3<R, A0, A1, const Value&> : public UnboxArgV2<R, A0, A1>
{
public:
    UnboxArgV3(VM& vm, Value& result, const Value* argv)
    : UnboxArgV2<R, A0, A1>(vm, result, argv)
    , a2(argv[2])
    {
    }

private:
    UnboxArgV3& operator =(const UnboxArgV3&);

public:
    const Value& a2;
};

////
template <typename R, typename A0, typename A1, typename A2, typename A3>
class UnboxArgV4 : public UnboxArgV3<R, A0, A1, A2>
{
public:
    UnboxArgV4(VM& vm, Value& result, const Value* argv)
    : UnboxArgV3<R, A0, A1, A2>(vm, result, argv)
    , a3(DefaultValue<typename Clean<A3>::type>(vm.GetStringManager()))
    {
        AS3::Convert(vm, a3, argv[3]);
    }

private:
    UnboxArgV4& operator =(const UnboxArgV4&);

public:
    typename Clean<A3>::type a3;
};

////
template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4>
class UnboxArgV5 : public UnboxArgV4<R, A0, A1, A2, A3>
{
public:
    UnboxArgV5(VM& vm, Value& result, const Value* argv)
    : UnboxArgV4<R, A0, A1, A2, A3>(vm, result, argv)
    , a4(DefaultValue<typename Clean<A4>::type>(vm.GetStringManager()))
    {
        AS3::Convert(vm, a4, argv[4]);
    }

private:
    UnboxArgV5& operator =(const UnboxArgV5&);

public:
    typename Clean<A4>::type a4;
};

////
template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
class UnboxArgV6 : public UnboxArgV5<R, A0, A1, A2, A3, A4>
{
public:
    UnboxArgV6(VM& vm, Value& result, const Value* argv)
    : UnboxArgV5<R, A0, A1, A2, A3, A4>(vm, result, argv)
    , a5(DefaultValue<typename Clean<A5>::type>(vm.GetStringManager()))
    {
        AS3::Convert(vm, a5, argv[5]);
    }

private:
    UnboxArgV6& operator =(const UnboxArgV6&);

public:
    typename Clean<A5>::type a5;
};

///////////////////////////////////////////////////////////////////////////
// This method definition is intentionally not declared as *inline* ...
template <typename T, size_t N, typename R>
void ThunkFunc0<T, N, R>::Func(const ThunkInfo& /*ti*/, VM& vm, const Value& obj, Value& result, unsigned /*argc*/, const Value* /*argv*/)
{
    T& _this = static_cast<T&>(*obj.GetObject());
    UnboxArgV0<R> args(vm, result);

    (_this.*Method)(args.r);
}

// This method definition is intentionally not declared as *inline* ...
template <typename T, size_t N, typename R, typename A0>
void ThunkFunc1<T, N, R, A0>::Func(const ThunkInfo& /*ti*/, VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv)
{
    T& _this = static_cast<T&>(*obj.GetObject());
    SF_UNUSED1(argc);
    SF_ASSERT(argc > 0);
    UnboxArgV1<R, A0> args(vm, result, argv);

    if (vm.IsException())
        return;

    (_this.*Method)(args.r, args.a0);
}

// This method definition is intentionally not declared as *inline* ...
template <typename T, size_t N, typename R, typename A0, typename A1>
void ThunkFunc2<T, N, R, A0, A1>::Func(const ThunkInfo& /*ti*/, VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv)
{
    T& _this = static_cast<T&>(*obj.GetObject());
    SF_UNUSED1(argc);
    SF_ASSERT(argc > 1);
    UnboxArgV2<R, A0, A1> args(vm, result, argv);

    if (vm.IsException())
        return;

    (_this.*Method)(args.r, args.a0, args.a1);
}

// This method definition is intentionally not declared as *inline* ...
template <typename T, size_t N, typename R, typename A0, typename A1, typename A2>
void ThunkFunc3<T, N, R, A0, A1, A2>::Func(const ThunkInfo& /*ti*/, VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv)
{
    T& _this = static_cast<T&>(*obj.GetObject());
    SF_UNUSED1(argc);
    SF_ASSERT(argc > 2);
    UnboxArgV3<R, A0, A1, A2> args(vm, result, argv);

    if (vm.IsException())
        return;

    (_this.*Method)(args.r, args.a0, args.a1, args.a2);
}

// This method definition is intentionally not declared as *inline* ...
template <typename T, size_t N, typename R, typename A0, typename A1, typename A2, typename A3>
void ThunkFunc4<T, N, R, A0, A1, A2, A3>::Func(const ThunkInfo& /*ti*/, VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv)
{
    T& _this = static_cast<T&>(*obj.GetObject());
    SF_UNUSED1(argc);
    SF_ASSERT(argc > 3);
    UnboxArgV4<R, A0, A1, A2, A3> args(vm, result, argv);

    if (vm.IsException())
        return;

    (_this.*Method)(args.r, args.a0, args.a1, args.a2, args.a3);
}

// This method definition is intentionally not declared as *inline* ...
template <typename T, size_t N, typename R, typename A0, typename A1, typename A2, typename A3, typename A4>
void ThunkFunc5<T, N, R, A0, A1, A2, A3, A4>::Func(const ThunkInfo& /*ti*/, VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv)
{
    T& _this = static_cast<T&>(*obj.GetObject());
    SF_UNUSED1(argc);
    SF_ASSERT(argc > 4);
    UnboxArgV5<R, A0, A1, A2, A3, A4> args(vm, result, argv);

    if (vm.IsException())
        return;

    (_this.*Method)(args.r, args.a0, args.a1, args.a2, args.a3, args.a4);
}

// This method definition is intentionally not declared as *inline* ...
template <typename T, size_t N, typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
void ThunkFunc6<T, N, R, A0, A1, A2, A3, A4, A5>::Func(const ThunkInfo& /*ti*/, VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv)
{
    T& _this = static_cast<T&>(*obj.GetObject());
    SF_UNUSED1(argc);
    SF_ASSERT(argc > 5);
    UnboxArgV6<R, A0, A1, A2, A3, A4, A5> args(vm, result, argv);

    if (vm.IsException())
        return;

    (_this.*Method)(args.r, args.a0, args.a1, args.a2, args.a3, args.a4, args.a5);
}

///////////////////////////////////////////////////////////////////////////
template <typename A0, typename T>
void CallConstructor(T& obj, unsigned argc, const Value* argv)
{
    Value result;
    SF_UNUSED1(argc);
    SF_ASSERT(argc > 0);
    UnboxArgV1<Value, A0> args(obj.GetVM(), result, argc, argv, MakeDefArgs<T, 0, A0>(obj.GetStringManager()));
    obj.Constructor(args.a0);
}

template <typename A0, typename A1, typename T>
void CallConstructor(T& obj, unsigned argc, const Value* argv)
{
    Value result;
    SF_ASSERT(argc > 1);
    UnboxArgV2<Value, A0, A1> args(obj.GetVM(), result, argc, argv, MakeDefArgs<T, 0, A0, A1>(obj.GetStringManager()));
    obj.Constructor(args.a0, args.a1);
}

template <typename A0, typename A1, typename A2, typename T>
void CallConstructor(T& obj, unsigned argc, const Value* argv)
{
    Value result;
    SF_ASSERT(argc > 2);
    UnboxArgV3<Value, A0, A1, A2> args(obj.GetVM(), result, argc, argv, MakeDefArgs<T, 0, A0, A1, A2>(obj.GetStringManager()));
    obj.Constructor(args.a0, args.a1, args.a2);
}

template <typename A0, typename A1, typename A2, typename A3, typename T>
void CallConstructor(T& obj, unsigned argc, const Value* argv)
{
    Value result;
    SF_ASSERT(argc > 3);
    UnboxArgV4<Value, A0, A1, A2, A3> args(obj.GetVM(), result, argc, argv, MakeDefArgs<T, 0, A0, A1, A2, A3>(obj.GetStringManager()));
    obj.Constructor(args.a0, args.a1, args.a2, args.a3);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename T>
void CallConstructor(T& obj, unsigned argc, const Value* argv)
{
    Value result;
    SF_ASSERT(argc > 4);
    UnboxArgV5<Value, A0, A1, A2, A3, A4> args(obj.GetVM(), result, argc, argv, MakeDefArgs<T, 0, A0, A1, A2, A3, A4>(obj.GetStringManager()));
    obj.Constructor(args.a0, args.a1, args.a2, args.a3, args.a4);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename T>
void CallConstructor(T& obj, unsigned argc, const Value* argv)
{
    Value result;
    SF_ASSERT(argc > 5);
    UnboxArgV6<Value, A0, A1, A2, A3, A4, A5> args(obj.GetVM(), result, argc, argv, MakeDefArgs<T, 0, A0, A1, A2, A3, A4, A5>(obj.GetStringManager()));
    obj.Constructor(args.a0, args.a1, args.a2, args.a3, args.a4, args.a5);
}

#else // SF_AS3_EMIT_DEF_ARGS

template <typename R>
class UnboxArgV0
{
public:
    SF_INLINE
        UnboxArgV0(VM& vm, Value& result)
        : Vm(vm)
        , Result(result)
        , r(DefaultValue<R>(GetStringManager()))
    {
    }
    SF_INLINE
        ~UnboxArgV0()
    {
        if (GetVM().IsException())
            return;

        AS3::ConvertUnsafe(Result, r);
    }

public:
    VM& GetVM() const
    {
        return Vm;
    }
    SF_INLINE
        StringManager& GetStringManager() const
    {
        return GetVM().GetStringManager();
    }

private:
    UnboxArgV0& operator =(const UnboxArgV0&);

private:
    VM& Vm;

public:
    Value& Result;
    R r;
};

template <>
class UnboxArgV0<Value>
{
public:
    SF_INLINE
        UnboxArgV0(VM& vm, Value& result)
        : Vm(vm)
        , r(result)
    {
    }
    SF_INLINE
        ~UnboxArgV0()
    {
        // No conversion is necessary.
    }

public:
    VM& GetVM() const
    {
        return Vm;
    }
    SF_INLINE
        StringManager& GetStringManager() const
    {
        return GetVM().GetStringManager();
    }

private:
    UnboxArgV0& operator =(const UnboxArgV0&);

private:
    VM& Vm;

public:
    Value& r;
};

template <>
class UnboxArgV0<const Value>
{
public:
    SF_INLINE
        UnboxArgV0(VM& vm, Value& result)
        : Vm(vm)
        , r(result)
    {
    }
    SF_INLINE
        ~UnboxArgV0()
    {
        // No conversion is necessary.
    }

public:
    VM& GetVM() const
    {
        return Vm;
    }
    SF_INLINE
        StringManager& GetStringManager() const
    {
        return GetVM().GetStringManager();
    }

private:
    UnboxArgV0& operator =(const UnboxArgV0&);

private:
    VM& Vm;

public:
    const Value& r;
};

////
template <typename R, typename A0>
class UnboxArgV1 : public UnboxArgV0<R>
{
public:
    SF_INLINE
        UnboxArgV1(VM& vm, Value& result, unsigned argc, const Value* argv, const DefArgs1<A0>& da)
        : UnboxArgV0<R>(vm, result)
        , a0(da._0)
    {
        ReadArg(vm, 0, a0, argc, argv);
    }
    SF_INLINE
        UnboxArgV1(VM& vm, Value& result, unsigned argc, const Value* argv)
        : UnboxArgV0<R>(vm, result)
        , a0(DefaultValue<typename Clean<A0>::type>(vm.GetStringManager()))
    {
        ReadArg(vm, 0, a0, argc, argv);
    }

private:
    UnboxArgV1& operator =(const UnboxArgV1&);

public:
    typename Clean<A0>::type a0;
};

template <typename R>
class UnboxArgV1<R, Value> : public UnboxArgV0<R>
{
public:
    SF_INLINE
        UnboxArgV1(VM& vm, Value& result, unsigned argc, const Value* argv, const DefArgs1<Value>& da)
        : UnboxArgV0<R>(vm, result)
        , a0(argc > 0 ? argv[0] : da._0)
    {
    }
    SF_INLINE
        UnboxArgV1(VM& vm, Value& result, unsigned argc, const Value* argv)
        : UnboxArgV0<R>(vm, result)
        , a0(argc > 0 ? argv[0] : Value::GetUndefined())
    {
    }

private:
    UnboxArgV1& operator =(const UnboxArgV1&);

public:
    const Value& a0;
};

template <typename R>
class UnboxArgV1<R, const Value&> : public UnboxArgV0<R>
{
public:
    SF_INLINE
        UnboxArgV1(VM& vm, Value& result, unsigned argc, const Value* argv, const DefArgs1<const Value&>& da)
        : UnboxArgV0<R>(vm, result)
        , a0(argc > 0 ? argv[0] : da._0)
    {
    }
    SF_INLINE
        UnboxArgV1(VM& vm, Value& result, unsigned argc, const Value* argv)
        : UnboxArgV0<R>(vm, result)
        , a0(argc > 0 ? argv[0] : Value::GetUndefined())
    {
    }

private:
    UnboxArgV1& operator =(const UnboxArgV1&);

public:
    const Value& a0;
};

////
template <typename R, typename A0, typename A1>
class UnboxArgV2 : public UnboxArgV1<R, A0>
{
public:
    UnboxArgV2(VM& vm, Value& result, unsigned argc, const Value* argv, const DefArgs2<A0, A1>& da)
        : UnboxArgV1<R, A0>(vm, result, argc, argv, da)
        , a1(da._1)
    {
        if (vm.IsException())
            return;

        ReadArg(vm, 1, a1, argc, argv);
    }
    UnboxArgV2(VM& vm, Value& result, unsigned argc, const Value* argv)
        : UnboxArgV1<R, A0>(vm, result, argv)
        , a1(DefaultValue<typename Clean<A1>::type>(vm.GetStringManager()))
    {
        if (vm.IsException())
            return;

        ReadArg(vm, 1, a1, argc, argv);
    }

private:
    UnboxArgV2& operator =(const UnboxArgV2&);

public:
    typename Clean<A1>::type a1;
};

template <typename R>
class UnboxArgV2<R, unsigned, const Value*> : public UnboxArgV0<R>
{
public:
    UnboxArgV2(VM& vm, Value& result, unsigned argc, const Value* argv, const DefArgs2<unsigned, const Value*>& /*da*/)
        : UnboxArgV0<R>(vm, result)
        , a0(argc)
        , a1(argv)
    {
    }
    UnboxArgV2(VM& vm, Value& result, unsigned argc, const Value* argv)
        : UnboxArgV0<R>(vm, result)
        , a0(argc)
        , a1(argv)
    {
    }

private:
    UnboxArgV2& operator =(const UnboxArgV2&);

public:
    unsigned        a0;
    const Value*    a1;
};

template <typename R, typename A0>
class UnboxArgV2<R, A0, Value> : public UnboxArgV1<R, A0>
{
public:
    UnboxArgV2(VM& vm, Value& result, unsigned argc, const Value* argv, const DefArgs2<A0, Value>& da)
        : UnboxArgV1<R, A0>(vm, result, argc, argv, da)
        , a1(argc > 1 ? argv[1] : da._1)
    {
    }
    UnboxArgV2(VM& vm, Value& result, unsigned argc, const Value* argv)
        : UnboxArgV1<R, A0>(vm, result, argv)
        , a1(argc > 1 ? argv[1] : Value::GetUndefined())
    {
    }

private:
    UnboxArgV2& operator =(const UnboxArgV2&);

public:
    const Value& a1;
};

template <typename R, typename A0>
class UnboxArgV2<R, A0, const Value&> : public UnboxArgV1<R, A0>
{
public:
    UnboxArgV2(VM& vm, Value& result, unsigned argc, const Value* argv, const DefArgs2<A0, const Value&>& da)
        : UnboxArgV1<R, A0>(vm, result, argc, argv, da)
        , a1(argc > 1 ? argv[1] : da._1)
    {
    }
    UnboxArgV2(VM& vm, Value& result, unsigned argc, const Value* argv)
        : UnboxArgV1<R, A0>(vm, result, argv)
        , a1(argc > 1 ? argv[1] : Value::GetUndefined())
    {
    }

private:
    UnboxArgV2& operator =(const UnboxArgV2&);

public:
    const Value& a1;
};

////
template <typename R, typename A0, typename A1, typename A2>
class UnboxArgV3 : public UnboxArgV2<R, A0, A1>
{
public:
    UnboxArgV3(VM& vm, Value& result, unsigned argc, const Value* argv, const DefArgs3<A0, A1, A2>& da)
        : UnboxArgV2<R, A0, A1>(vm, result, argc, argv, da)
        , a2(da._2)
    {
        if (vm.IsException())
            return;

        ReadArg(vm, 2, a2, argc, argv);
    }
    UnboxArgV3(VM& vm, Value& result, unsigned argc, const Value* argv)
        : UnboxArgV2<R, A0, A1>(vm, result, argv)
        , a2(DefaultValue<typename Clean<A2>::type>(vm.GetStringManager()))
    {
        if (vm.IsException())
            return;

        ReadArg(vm, 2, a2, argc, argv);
    }

private:
    UnboxArgV3& operator =(const UnboxArgV3&);

public:
    typename Clean<A2>::type a2;
};

template <typename R, typename A0, typename A1>
class UnboxArgV3<R, A0, A1, Value> : public UnboxArgV2<R, A0, A1>
{
public:
    UnboxArgV3(VM& vm, Value& result, unsigned argc, const Value* argv, const DefArgs3<A0, A1, Value>& da)
        : UnboxArgV2<R, A0, A1>(vm, result, argc, argv, da)
        , a2(argc > 2 ? argv[2] : da._2)
    {
    }
    UnboxArgV3(VM& vm, Value& result, unsigned argc, const Value* argv)
        : UnboxArgV2<R, A0, A1>(vm, result, argv)
        , a2(argc > 2 ? argv[2] : Value::GetUndefined())
    {
    }

private:
    UnboxArgV3& operator =(const UnboxArgV3&);

public:
    const Value& a2;
};

template <typename R, typename A0, typename A1>
class UnboxArgV3<R, A0, A1, const Value&> : public UnboxArgV2<R, A0, A1>
{
public:
    UnboxArgV3(VM& vm, Value& result, unsigned argc, const Value* argv, const DefArgs3<A0, A1, const Value&>& da)
        : UnboxArgV2<R, A0, A1>(vm, result, argc, argv, da)
        , a2(argc > 2 ? argv[2] : da._2)
    {
    }
    UnboxArgV3(VM& vm, Value& result, unsigned argc, const Value* argv)
        : UnboxArgV2<R, A0, A1>(vm, result, argv)
        , a2(argc > 2 ? argv[2] : Value::GetUndefined())
    {
    }

private:
    UnboxArgV3& operator =(const UnboxArgV3&);

public:
    const Value& a2;
};

////
template <typename R, typename A0, typename A1, typename A2, typename A3>
class UnboxArgV4 : public UnboxArgV3<R, A0, A1, A2>
{
public:
    UnboxArgV4(VM& vm, Value& result, unsigned argc, const Value* argv, const DefArgs4<A0, A1, A2, A3>& da)
        : UnboxArgV3<R, A0, A1, A2>(vm, result, argc, argv, da)
        , a3(da._3)
    {
        if (vm.IsException())
            return;

        ReadArg(vm, 3, a3, argc, argv);
    }
    UnboxArgV4(VM& vm, Value& result, unsigned argc, const Value* argv)
        : UnboxArgV3<R, A0, A1, A2>(vm, result, argv)
        , a3(DefaultValue<typename Clean<A3>::type>(vm.GetStringManager()))
    {
        if (vm.IsException())
            return;

        ReadArg(vm, 3, a3, argc, argv);
    }

private:
    UnboxArgV4& operator =(const UnboxArgV4&);

public:
    typename Clean<A3>::type a3;
};

////
template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4>
class UnboxArgV5 : public UnboxArgV4<R, A0, A1, A2, A3>
{
public:
    UnboxArgV5(VM& vm, Value& result, unsigned argc, const Value* argv, const DefArgs5<A0, A1, A2, A3, A4>& da)
        : UnboxArgV4<R, A0, A1, A2, A3>(vm, result, argc, argv, da)
        , a4(da._4)
    {
        if (vm.IsException())
            return;

        ReadArg(vm, 4, a4, argc, argv);
    }
    UnboxArgV5(VM& vm, Value& result, unsigned argc, const Value* argv)
        : UnboxArgV4<R, A0, A1, A2, A3>(vm, result, argv)
        , a4(DefaultValue<typename Clean<A4>::type>(vm.GetStringManager()))
    {
        if (vm.IsException())
            return;

        ReadArg(vm, 4, a4, argc, argv);
    }

private:
    UnboxArgV5& operator =(const UnboxArgV5&);

public:
    typename Clean<A4>::type a4;
};

////
template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
class UnboxArgV6 : public UnboxArgV5<R, A0, A1, A2, A3, A4>
{
public:
    UnboxArgV6(VM& vm, Value& result, unsigned argc, const Value* argv, const DefArgs6<A0, A1, A2, A3, A4, A5>& da)
        : UnboxArgV5<R, A0, A1, A2, A3, A4>(vm, result, argc, argv, da)
        , a5(da._5)
    {
        if (vm.IsException())
            return;

        ReadArg(vm, 5, a5, argc, argv);
    }
    UnboxArgV6(VM& vm, Value& result, unsigned argc, const Value* argv)
        : UnboxArgV5<R, A0, A1, A2, A3, A4>(vm, result, argv)
        , a5(DefaultValue<typename Clean<A5>::type>(vm.GetStringManager()))
    {
        if (vm.IsException())
            return;

        ReadArg(vm, 5, a5, argc, argv);
    }

private:
    UnboxArgV6& operator =(const UnboxArgV6&);

public:
    typename Clean<A5>::type a5;
};

///////////////////////////////////////////////////////////////////////////
// RT stands for Result Type. It can be of type Value, which can hold multiple
// value types.
// If argument is not available, result won't be changed.
template <typename RT>
SF_INLINE
void ReadArg(unsigned n, RT& result, unsigned argc, const Value* argv)
{
    if (argc > n)
        AS3::Convert(result, argv[n]);
}

template <typename RT>
SF_INLINE
void ReadArg(VM& vm, unsigned n, RT& result, unsigned argc, const Value* argv)
{
    if (argc > n)
        AS3::Convert(vm, result, argv[n]);
}

///////////////////////////////////////////////////////////////////////////
// This method definition is intentionally not declared as *inline* ...
template <typename T, size_t N, typename R>
void ThunkFunc0<T, N, R>::Func(const ThunkInfo& /*ti*/, VM& vm, const Value& obj, Value& result, unsigned /*argc*/, const Value* /*argv*/)
{
    T& _this = static_cast<T&>(*obj.GetObject());
    UnboxArgV0<R> args(vm, result);

    (_this.*Method)(args.r);
}

// This method definition is intentionally not declared as *inline* ...
template <typename T, size_t N, typename R, typename A0>
void ThunkFunc1<T, N, R, A0>::Func(const ThunkInfo& /*ti*/, VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv)
{
    T& _this = static_cast<T&>(*obj.GetObject());
    StringManager& sm = vm.GetStringManager();
    DefArgs1<A0> def_ags(
        Impl::GetMethodDefArg<T, N, 0, A0>(sm)
        );
    UnboxArgV1<R, A0> args(vm, result, argc, argv, def_ags);

    if (vm.IsException())
        return;

    (_this.*Method)(args.r, args.a0);
}

// This method definition is intentionally not declared as *inline* ...
template <typename T, size_t N, typename R, typename A0, typename A1>
void ThunkFunc2<T, N, R, A0, A1>::Func(const ThunkInfo& /*ti*/, VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv)
{
    T& _this = static_cast<T&>(*obj.GetObject());
    StringManager& sm = vm.GetStringManager();
    DefArgs2<A0, A1> def_ags(
        Impl::GetMethodDefArg<T, N, 0, A0>(sm),
        Impl::GetMethodDefArg<T, N, 1, A1>(sm)
        );
    UnboxArgV2<R, A0, A1> args(vm, result, argc, argv, def_ags);

    if (vm.IsException())
        return;

    (_this.*Method)(args.r, args.a0, args.a1);
}

// This method definition is intentionally not declared as *inline* ...
template <typename T, size_t N, typename R, typename A0, typename A1, typename A2>
void ThunkFunc3<T, N, R, A0, A1, A2>::Func(const ThunkInfo& /*ti*/, VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv)
{
    T& _this = static_cast<T&>(*obj.GetObject());
    StringManager& sm = vm.GetStringManager();
    DefArgs3<A0, A1, A2> def_ags(
        Impl::GetMethodDefArg<T, N, 0, A0>(sm),
        Impl::GetMethodDefArg<T, N, 1, A1>(sm),
        Impl::GetMethodDefArg<T, N, 2, A2>(sm)
        );
    UnboxArgV3<R, A0, A1, A2> args(vm, result, argc, argv, def_ags);

    if (vm.IsException())
        return;

    (_this.*Method)(args.r, args.a0, args.a1, args.a2);
}

// This method definition is intentionally not declared as *inline* ...
template <typename T, size_t N, typename R, typename A0, typename A1, typename A2, typename A3>
void ThunkFunc4<T, N, R, A0, A1, A2, A3>::Func(const ThunkInfo& /*ti*/, VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv)
{
    T& _this = static_cast<T&>(*obj.GetObject());
    StringManager& sm = vm.GetStringManager();
    DefArgs4<A0, A1, A2, A3> def_ags(
        Impl::GetMethodDefArg<T, N, 0, A0>(sm),
        Impl::GetMethodDefArg<T, N, 1, A1>(sm),
        Impl::GetMethodDefArg<T, N, 2, A2>(sm),
        Impl::GetMethodDefArg<T, N, 3, A3>(sm)
        );
    UnboxArgV4<R, A0, A1, A2, A3> args(vm, result, argc, argv, def_ags);

    if (vm.IsException())
        return;

    (_this.*Method)(args.r, args.a0, args.a1, args.a2, args.a3);
}

// This method definition is intentionally not declared as *inline* ...
template <typename T, size_t N, typename R, typename A0, typename A1, typename A2, typename A3, typename A4>
void ThunkFunc5<T, N, R, A0, A1, A2, A3, A4>::Func(const ThunkInfo& /*ti*/, VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv)
{
    T& _this = static_cast<T&>(*obj.GetObject());
    StringManager& sm = vm.GetStringManager();
    DefArgs5<A0, A1, A2, A3, A4> def_ags(
        Impl::GetMethodDefArg<T, N, 0, A0>(sm),
        Impl::GetMethodDefArg<T, N, 1, A1>(sm),
        Impl::GetMethodDefArg<T, N, 2, A2>(sm),
        Impl::GetMethodDefArg<T, N, 3, A3>(sm),
        Impl::GetMethodDefArg<T, N, 4, A4>(sm)
        );
    UnboxArgV5<R, A0, A1, A2, A3, A4> args(vm, result, argc, argv, def_ags);

    if (vm.IsException())
        return;

    (_this.*Method)(args.r, args.a0, args.a1, args.a2, args.a3, args.a4);
}

// This method definition is intentionally not declared as *inline* ...
template <typename T, size_t N, typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
void ThunkFunc6<T, N, R, A0, A1, A2, A3, A4, A5>::Func(const ThunkInfo& /*ti*/, VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv)
{
    T& _this = static_cast<T&>(*obj.GetObject());
    StringManager& sm = vm.GetStringManager();
    DefArgs6<A0, A1, A2, A3, A4, A5> def_ags(
        Impl::GetMethodDefArg<T, N, 0, A0>(sm),
        Impl::GetMethodDefArg<T, N, 1, A1>(sm),
        Impl::GetMethodDefArg<T, N, 2, A2>(sm),
        Impl::GetMethodDefArg<T, N, 3, A3>(sm),
        Impl::GetMethodDefArg<T, N, 4, A4>(sm),
        Impl::GetMethodDefArg<T, N, 5, A5>(sm)
        );
    UnboxArgV6<R, A0, A1, A2, A3, A4, A5> args(vm, result, argc, argv, def_ags);

    if (vm.IsException())
        return;

    (_this.*Method)(args.r, args.a0, args.a1, args.a2, args.a3, args.a4, args.a5);
}

///////////////////////////////////////////////////////////////////////////
template <typename A0, typename T>
void CallConstructor(T& obj, unsigned argc, const Value* argv)
{
    Value result;
    UnboxArgV1<Value, A0> args(obj.GetVM(), result, argc, argv, MakeDefArgs<T, 0, A0>(obj.GetStringManager()));
    obj.Constructor(args.a0);
}

template <typename A0, typename A1, typename T>
void CallConstructor(T& obj, unsigned argc, const Value* argv)
{
    Value result;
    UnboxArgV2<Value, A0, A1> args(obj.GetVM(), result, argc, argv, MakeDefArgs<T, 0, A0, A1>(obj.GetStringManager()));
    obj.Constructor(args.a0, args.a1);
}

template <typename A0, typename A1, typename A2, typename T>
void CallConstructor(T& obj, unsigned argc, const Value* argv)
{
    Value result;
    UnboxArgV3<Value, A0, A1, A2> args(obj.GetVM(), result, argc, argv, MakeDefArgs<T, 0, A0, A1, A2>(obj.GetStringManager()));
    obj.Constructor(args.a0, args.a1, args.a2);
}

template <typename A0, typename A1, typename A2, typename A3, typename T>
void CallConstructor(T& obj, unsigned argc, const Value* argv)
{
    Value result;
    UnboxArgV4<Value, A0, A1, A2, A3> args(obj.GetVM(), result, argc, argv, MakeDefArgs<T, 0, A0, A1, A2, A3>(obj.GetStringManager()));
    obj.Constructor(args.a0, args.a1, args.a2, args.a3);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename T>
void CallConstructor(T& obj, unsigned argc, const Value* argv)
{
    Value result;
    UnboxArgV5<Value, A0, A1, A2, A3, A4> args(obj.GetVM(), result, argc, argv, MakeDefArgs<T, 0, A0, A1, A2, A3, A4>(obj.GetStringManager()));
    obj.Constructor(args.a0, args.a1, args.a2, args.a3, args.a4);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename T>
void CallConstructor(T& obj, unsigned argc, const Value* argv)
{
    Value result;
    UnboxArgV6<Value, A0, A1, A2, A3, A4, A5> args(obj.GetVM(), result, argc, argv, MakeDefArgs<T, 0, A0, A1, A2, A3, A4, A5>(obj.GetStringManager()));
    obj.Constructor(args.a0, args.a1, args.a2, args.a3, args.a4, args.a5);
}

#endif // SF_AS3_EMIT_DEF_ARGS

///////////////////////////////////////////////////////////////////////////
namespace Impl
{
    template <typename TR>
    inline SPtr<typename TR::InstanceType> Construct(TR& tr)
    {
        Pickable<typename TR::InstanceType> result = InstanceTraits::MakeInstance(tr);
        result->Constructor();
        return result;
    }

    template <typename TR, typename T1>
    inline SPtr<typename TR::InstanceType> Construct(TR& tr, const T1& a1)
    {
        Pickable<typename TR::InstanceType> result = InstanceTraits::MakeInstance(tr);
        result->Constructor(a1);
        return result;
    }

    template <typename TR, typename T1, typename T2>
    inline SPtr<typename TR::InstanceType> Construct(TR& tr, const T1& a1, const T2& a2)
    {
        Pickable<typename TR::InstanceType> result = InstanceTraits::MakeInstance(tr);
        result->Constructor(a1, a2);
        return result;
    }

    template <typename TR, typename T1, typename T2, typename T3>
    inline SPtr<typename TR::InstanceType> Construct(TR& tr, const T1& a1, const T2& a2, const T3& a3)
    {
        Pickable<typename TR::InstanceType> result = InstanceTraits::MakeInstance(tr);
        result->Constructor(a1, a2, a3);
        return result;
    }

    template <typename TR, typename T1, typename T2, typename T3, typename T4>
    inline SPtr<typename TR::InstanceType> Construct(TR& tr, const T1& a1, const T2& a2, const T3& a3, const T3& a4)
    {
        Pickable<typename TR::InstanceType> result = InstanceTraits::MakeInstance(tr);
        result->Constructor(a1, a2, a3, a4);
        return result;
    }

    template <typename TR, typename T1, typename T2, typename T3, typename T4, typename T5>
    inline SPtr<typename TR::InstanceType> Construct(TR& tr, const T1& a1, const T2& a2, const T3& a3, const T3& a4, const T3& a5)
    {
        Pickable<typename TR::InstanceType> result = InstanceTraits::MakeInstance(tr);
        result->Constructor(a1, a2, a3, a4, a5);
        return result;
    }

    template <typename TR, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    inline SPtr<typename TR::InstanceType> Construct(TR& tr, const T1& a1, const T2& a2, const T3& a3, const T3& a4, const T3& a5, const T3& a6)
    {
        Pickable<typename TR::InstanceType> result = InstanceTraits::MakeInstance(tr);
        result->Constructor(a1, a2, a3, a4, a5, a6);
        return result;
    }
}

///////////////////////////////////////////////////////////////////////////
template <typename R, typename T1>
inline
bool Class::Call(R& r, const T1& a1)
{
    Value result;
    Value argv[] = {Value(a1)};
    this->Call(Value::GetUndefined(), result, (unsigned)1, argv);

    if (GetVM().IsException())
        return false;

    Convert<R, Value>(r, result);

    return true;
}

///////////////////////////////////////////////////////////////////////////
// "NEC" stands for "No Exception Check".
 
inline Value CallPropertyNEC(VM& vm, const Value& _this, const char* n)
{
    Value result;
    const ASString name = vm.GetStringManager().CreateString(n);
    ExecutePropertyUnsafe(vm, Multiname(vm.GetPublicNamespace(), name), _this, result, 0, NULL).DoNotCheck();
    return result;
}

template <typename T1>
inline Value CallPropertyNEC(VM& vm, const Value& _this, const char* n, const T1& a1)
{
    Value result;
    StringManager& sm = vm.GetStringManager();
    const ASString name = sm.CreateString(n);
    BoxArgV1<T1> argv(a1, sm);
    ExecutePropertyUnsafe(vm, Multiname(vm.GetPublicNamespace(), name), _this, result, argv.AN, &argv.V).DoNotCheck();
    return result;
}

template <typename T1, typename T2>
inline Value CallPropertyNEC(VM& vm, const Value& _this, const char* n, const T1& a1, const T2& a2)
{
    Value result;
    StringManager& sm = vm.GetStringManager();
    const ASString name = sm.CreateString(n);
    BoxArgV2<T1, T2> argv(a1, a2, sm);
    ExecutePropertyUnsafe(vm, Multiname(vm.GetPublicNamespace(), name), _this, result, argv.AN, &argv.V).DoNotCheck();
    return result;
}

template <typename T1, typename T2, typename T3>
inline Value CallPropertyNEC(VM& vm, const Value& _this, const char* n, const T1& a1, const T2& a2, const T3& a3)
{
    Value result;
    StringManager& sm = vm.GetStringManager();
    const ASString name = sm.CreateString(n);
    BoxArgV3<T1, T2, T3> argv(a1, a2, a3, sm);
    ExecutePropertyUnsafe(vm, Multiname(vm.GetPublicNamespace(), name), _this, result, argv.AN, &argv.V).DoNotCheck();
    return result;
}

template <typename T1, typename T2, typename T3, typename T4>
inline Value CallPropertyNEC(VM& vm, const Value& _this, const char* n, const T1& a1, const T2& a2, const T3& a3, const T4& a4)
{
    Value result;
    StringManager& sm = vm.GetStringManager();
    const ASString name = sm.CreateString(n);
    BoxArgV4<T1, T2, T3, T4> argv(a1, a2, a3, a4, sm);
    ExecutePropertyUnsafe(vm, Multiname(vm.GetPublicNamespace(), name), _this, result, argv.AN, &argv.V).DoNotCheck();
    return result;
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
inline Value CallPropertyNEC(VM& vm, const Value& _this, const char* n, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5)
{
    Value result;
    StringManager& sm = vm.GetStringManager();
    const ASString name = sm.CreateString(n);
    BoxArgV5<T1, T2, T3, T4, T5> argv(a1, a2, a3, a4, a5, sm);
    ExecutePropertyUnsafe(vm, Multiname(vm.GetPublicNamespace(), name), _this, result, argv.AN, &argv.V).DoNotCheck();
    return result;
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
inline Value CallPropertyNEC(VM& vm, const Value& _this, const char* n, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T5& a6)
{
    Value result;
    StringManager& sm = vm.GetStringManager();
    const ASString name = sm.CreateString(n);
    BoxArgV6<T1, T2, T3, T4, T5, T6> argv(a1, a2, a3, a4, a5, a6, sm);
    ExecutePropertyUnsafe(vm, Multiname(vm.GetPublicNamespace(), name), _this, result, argv.AN, &argv.V).DoNotCheck();
    return result;
}

///////////////////////////////////////////////////////////////////////////
template <typename R>
inline CheckResult CallProperty(VM& vm, const Value& _this, const char* n, R& r)
{
    Value result;
    const ASString name = vm.GetStringManager().CreateString(n);
    const Multiname mn(vm.GetPublicNamespace(), name);
    return ExecutePropertyUnsafe(vm, mn, _this, result, 0, NULL) && (Convert(r, result), !vm.IsException());
}

template <typename R, typename T1>
inline CheckResult CallProperty(VM& vm, const Value& _this, const char* n, R& r, const T1& a1)
{
    Value result;
    StringManager& sm = vm.GetStringManager();
    const ASString name = sm.CreateString(n);
    const Multiname mn(vm.GetPublicNamespace(), name);
    BoxArgV1<T1> argv(a1, sm);
    return ExecutePropertyUnsafe(vm, mn, _this, result, argv.AN, &argv.V) && (Convert(r, result), !vm.IsException());
}

template <typename R, typename T1, typename T2>
inline CheckResult CallProperty(VM& vm, const Value& _this, const char* n, R& r, const T1& a1, const T2& a2)
{
    Value result;
    StringManager& sm = vm.GetStringManager();
    const ASString name = sm.CreateString(n);
    const Multiname mn(vm.GetPublicNamespace(), name);
    BoxArgV2<T1, T2> argv(a1, a2, sm);
    return ExecutePropertyUnsafe(vm, mn, _this, result, argv.AN, &argv.V) && (Convert(r, result), !vm.IsException());
}

template <typename R, typename T1, typename T2, typename T3>
inline CheckResult CallProperty(VM& vm, const Value& _this, const char* n, R& r, const T1& a1, const T2& a2, const T3& a3)
{
    Value result;
    StringManager& sm = vm.GetStringManager();
    const ASString name = sm.CreateString(n);
    const Multiname mn(vm.GetPublicNamespace(), name);
    BoxArgV3<T1, T2, T3> argv(a1, a2, a3, sm);
    return ExecutePropertyUnsafe(vm, mn, _this, result, argv.AN, &argv.V) && (Convert(r, result), !vm.IsException());
}

template <typename R, typename T1, typename T2, typename T3, typename T4>
inline CheckResult CallProperty(VM& vm, const Value& _this, const char* n, R& r, const T1& a1, const T2& a2, const T3& a3, const T4& a4)
{
    Value result;
    StringManager& sm = vm.GetStringManager();
    const ASString name = sm.CreateString(n);
    const Multiname mn(vm.GetPublicNamespace(), name);
    BoxArgV4<T1, T2, T3, T4> argv(a1, a2, a3, a4, sm);
    return ExecutePropertyUnsafe(vm, mn, _this, result, argv.AN, &argv.V) && (Convert(r, result), !vm.IsException());
}

template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
inline CheckResult CallProperty(VM& vm, const Value& _this, const char* n, R& r, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5)
{
    Value result;
    StringManager& sm = vm.GetStringManager();
    const ASString name = sm.CreateString(n);
    const Multiname mn(vm.GetPublicNamespace(), name);
    BoxArgV5<T1, T2, T3, T4, T5> argv(a1, a2, a3, a4, a5, sm);
    return ExecutePropertyUnsafe(vm, mn, _this, result, argv.AN, &argv.V) && (Convert(r, result), !vm.IsException());
}

template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
inline CheckResult CallProperty(VM& vm, const Value& _this, const char* n, R& r, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T5& a6)
{
    Value result;
    StringManager& sm = vm.GetStringManager();
    const ASString name = sm.CreateString(n);
    const Multiname mn(vm.GetPublicNamespace(), name);
    BoxArgV6<T1, T2, T3, T4, T5, T6> argv(a1, a2, a3, a4, a5, a6, sm);
    return ExecutePropertyUnsafe(vm, mn, _this, result, argv.AN, &argv.V) && (Convert(r, result), !vm.IsException());
}

///////////////////////////////////////////////////////////////////////////
inline 
Value Value::operator()(VM& vm) const
{
    Value result;

    vm.ExecuteUnsafe(*this, Value::GetUndefined(), result, 0, NULL);

    return result;
}

template <typename T1>
inline 
Value Value::operator()(VM& vm, const T1& a1) const
{
    Value result;

    BoxArgV1<T1> argv(a1, vm.GetStringManager());
    vm.ExecuteUnsafe(*this, Value::GetUndefined(), result, argv.AN, &argv.V);

    return result;
}

template <typename T1, typename T2>
inline 
Value Value::operator()(VM& vm, const T1& a1, const T2& a2) const
{
    Value result;

    BoxArgV2<T1, T2> argv(a1, a2, vm.GetStringManager());
    vm.ExecuteUnsafe(*this, Value::GetUndefined(), result, argv.AN, &argv.V);

    return result;
}

template <typename T1, typename T2, typename T3>
inline 
Value Value::operator()(VM& vm, const T1& a1, const T2& a2, const T3& a3) const
{
    Value result;

    BoxArgV3<T1, T2, T3> argv(a1, a2, a3, vm.GetStringManager());
    vm.ExecuteUnsafe(*this, Value::GetUndefined(), result, argv.AN, &argv.V);

    return result;
}

///////////////////////////////////////////////////////////////////////////
#ifdef SF_AS3_EMIT_DEF_ARGS

template <typename T, typename R>
struct UnboxCall0
{
    UnboxCall0(T* _this, void (T::*Method)(R&), Value& result, unsigned argc, const Value* argv)
    {
        SF_UNUSED2(argc, argv);
        VM& vm = _this->GetVM();
        UnboxArgV0<R> args(vm, result);

        if (vm.IsException())
            return;

        (_this->*Method)(args.r);
    }
};

template <typename T, typename R, typename A0>
struct UnboxCall1
{
    UnboxCall1(T* _this, void (T::*Method)(R&, A0), Value& result, unsigned argc, const Value* argv)
    {
        VM& vm = _this->GetVM();
        SF_ASSERT(argc > 0);
        UnboxArgV1<R, A0> args(vm, result, argc, argv);

        if (vm.IsException())
            return;

        (_this->*Method)(args.r, args.a0);
    }
};

template <typename T, typename R, typename A0, typename A1>
struct UnboxCall2
{
    UnboxCall2(T* _this, void (T::*Method)(R&, A0, A1), Value& result, unsigned argc, const Value* argv)
    {
        VM& vm = _this->GetVM();
        SF_ASSERT(argc > 1);
        UnboxArgV2<R, A0, A1> args(vm, result, argc, argv);

        if (vm.IsException())
            return;

        (_this->*Method)(args.r, args.a0, args.a1);
    }
};

template <typename T, typename R>
struct UnboxCall2<T, R, unsigned, const Value*>
{
    UnboxCall2(T* _this, void (T::*Method)(R&, unsigned, const Value*), Value& result, unsigned argc, const Value* argv)
    {
        VM& vm = _this->GetVM();
        UnboxArgV0<R> args(vm, result);

        // No exceptions at this point.

        (_this->*Method)(args.r, argc, argv);
    }
};

template <typename T, typename R, typename A0, typename A1, typename A2>
struct UnboxCall3
{
    UnboxCall3(T* _this, void (T::*Method)(R&, A0, A1, A2), Value& result, unsigned argc, const Value* argv)
    {
        VM& vm = _this->GetVM();
        SF_ASSERT(argc > 2);
        UnboxArgV3<R, A0, A1, A2> args(vm, result, argc, argv);

        if (vm.IsException())
            return;

        (_this->*Method)(args.r, args.a0, args.a1, args.a2);
    }
};

template <typename T, typename R, typename A0, typename A1, typename A2, typename A3>
struct UnboxCall4
{
    UnboxCall4(T* _this, void (T::*Method)(R&, A0, A1, A2, A3), Value& result, unsigned argc, const Value* argv)
    {
        VM& vm = _this->GetVM();
        SF_ASSERT(argc > 3);
        UnboxArgV4<R, A0, A1, A2, A3> args(vm, result, argc, argv);

        if (vm.IsException())
            return;

        (_this->*Method)(args.r, args.a0, args.a1, args.a2, args.a3);
    }
};

template <typename T, typename R, typename A0, typename A1, typename A2, typename A3, typename A4>
struct UnboxCall5
{
    UnboxCall5(T* _this, void (T::*Method)(R&, A0, A1, A2, A3, A4), Value& result, unsigned argc, const Value* argv)
    {
        VM& vm = _this->GetVM();
        SF_ASSERT(argc > 4);
        UnboxArgV5<R, A0, A1, A2, A3, A4> args(vm, result, argc, argv);

        if (vm.IsException())
            return;

        (_this->*Method)(args.r, args.a0, args.a1, args.a2, args.a3, args.a4);
    }
};

template <typename T, typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
struct UnboxCall6
{
    UnboxCall6(T* _this, void (T::*Method)(R&, A0, A1, A2, A3, A4, A5), Value& result, unsigned argc, const Value* argv)
    {
        VM& vm = _this->GetVM();
        SF_ASSERT(argc > 5);
        UnboxArgV6<R, A0, A1, A2, A3, A4, A5> args(vm, result, argc, argv);

        if (vm.IsException())
            return;

        (_this->*Method)(args.r, args.a0, args.a1, args.a2, args.a3, args.a4, args.a5);
    }
};

#else // SF_AS3_EMIT_DEF_ARGS

template <typename T, typename R>
struct UnboxCall0
{
    UnboxCall0(T* _this, void (T::*Method)(R&), Value& result, unsigned argc, const Value* argv)
    {
        SF_UNUSED2(argc, argv);
        VM& vm = _this->GetVM();
        UnboxArgV0<R> args(vm, result);

        if (vm.IsException())
            return;

        (_this->*Method)(args.r);
    }
};

template <typename T, typename R, typename A0>
struct UnboxCall1
{
    UnboxCall1(T* _this, void (T::*Method)(R&, A0), Value& result, unsigned argc, const Value* argv)
    {
        VM& vm = _this->GetVM();
        UnboxArgV1<R, A0> args(vm, result, argc, argv);

        if (vm.IsException())
            return;

        (_this->*Method)(args.r, args.a0);
    }
};

template <typename T, typename R, typename A0, typename A1>
struct UnboxCall2
{
    UnboxCall2(T* _this, void (T::*Method)(R&, A0, A1), Value& result, unsigned argc, const Value* argv)
    {
        VM& vm = _this->GetVM();
        UnboxArgV2<R, A0, A1> args(vm, result, argc, argv);

        if (vm.IsException())
            return;

        (_this->*Method)(args.r, args.a0, args.a1);
    }
};

template <typename T, typename R>
struct UnboxCall2<T, R, unsigned, const Value*>
{
    UnboxCall2(T* _this, void (T::*Method)(R&, unsigned, const Value*), Value& result, unsigned argc, const Value* argv)
    {
        VM& vm = _this->GetVM();
        UnboxArgV0<R> args(vm, result);

        // No exceptions at this point.

        (_this->*Method)(args.r, argc, argv);
    }
};

template <typename T, typename R, typename A0, typename A1, typename A2>
struct UnboxCall3
{
    UnboxCall3(T* _this, void (T::*Method)(R&, A0, A1, A2), Value& result, unsigned argc, const Value* argv)
    {
        VM& vm = _this->GetVM();
        UnboxArgV3<R, A0, A1, A2> args(vm, result, argc, argv);

        if (vm.IsException())
            return;

        (_this->*Method)(args.r, args.a0, args.a1, args.a2);
    }
};

template <typename T, typename R, typename A0, typename A1, typename A2, typename A3>
struct UnboxCall4
{
    UnboxCall4(T* _this, void (T::*Method)(R&, A0, A1, A2, A3), Value& result, unsigned argc, const Value* argv)
    {
        VM& vm = _this->GetVM();
        UnboxArgV4<R, A0, A1, A2, A3> args(vm, result, argc, argv);

        if (vm.IsException())
            return;

        (_this->*Method)(args.r, args.a0, args.a1, args.a2, args.a3);
    }
};

template <typename T, typename R, typename A0, typename A1, typename A2, typename A3, typename A4>
struct UnboxCall5
{
    UnboxCall5(T* _this, void (T::*Method)(R&, A0, A1, A2, A3, A4), Value& result, unsigned argc, const Value* argv)
    {
        VM& vm = _this->GetVM();
        UnboxArgV5<R, A0, A1, A2, A3, A4> args(vm, result, argc, argv);

        if (vm.IsException())
            return;

        (_this->*Method)(args.r, args.a0, args.a1, args.a2, args.a3, args.a4);
    }
};

template <typename T, typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
struct UnboxCall6
{
    UnboxCall6(T* _this, void (T::*Method)(R&, A0, A1, A2, A3, A4, A5), Value& result, unsigned argc, const Value* argv)
    {
        VM& vm = _this->GetVM();
        UnboxArgV6<R, A0, A1, A2, A3, A4, A5> args(vm, result, argc, argv);

        if (vm.IsException())
            return;

        (_this->*Method)(args.r, args.a0, args.a1, args.a2, args.a3, args.a4, args.a5);
    }
};

#endif // SF_AS3_EMIT_DEF_ARGS

///////////////////////////////////////////////////////////////////////////
template <typename T, typename R>
SF_INLINE
void MakeCall0(T* _this, void (T::*Method)(R&), Value& result, unsigned argc, const Value* argv)
{
    UnboxCall0<T, R>(_this, Method, result, argc, argv);
}

template <typename T, typename R, typename A0>
SF_INLINE
void MakeCall1(T* _this, void (T::*Method)(R&, A0), Value& result, unsigned argc, const Value* argv)
{
    UnboxCall1<T, R, A0>(_this, Method, result, argc, argv);
}

template <typename T, typename R, typename A0, typename A1>
SF_INLINE
void MakeCall2(T* _this, void (T::*Method)(R&, A0, A1), Value& result, unsigned argc, const Value* argv)
{
    UnboxCall2<T, R, A0, A1>(_this, Method, result, argc, argv);
}

template <typename T, typename R, typename A0, typename A1, typename A2>
SF_INLINE
void MakeCall3(T* _this, void (T::*Method)(R&, A0, A1, A2), Value& result, unsigned argc, const Value* argv)
{
    UnboxCall3<T, R, A0, A1, A2>(_this, Method, result, argc, argv);
}

template <typename T, typename R, typename A0, typename A1, typename A2, typename A3>
SF_INLINE
void MakeCall4(T* _this, void (T::*Method)(R&, A0, A1, A2, A3), Value& result, unsigned argc, const Value* argv)
{
    UnboxCall4<T, R, A0, A1, A2, A3>(_this, Method, result, argc, argv);
}

template <typename T, typename R, typename A0, typename A1, typename A2, typename A3, typename A4>
SF_INLINE
void MakeCall5(T* _this, void (T::*Method)(R&, A0, A1, A2, A3, A4), Value& result, unsigned argc, const Value* argv)
{
    UnboxCall5<T, R, A0, A1, A2, A3, A4>(_this, Method, result, argc, argv);
}

template <typename T, typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
SF_INLINE
void MakeCall6(T* _this, void (T::*Method)(R&, A0, A1, A2, A3, A4, A5), Value& result, unsigned argc, const Value* argv)
{
    UnboxCall6<T, R, A0, A1, A2, A3, A4, A5>(_this, Method, result, argc, argv);
}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

#endif // INC_AS3_MARSHALLING_H

