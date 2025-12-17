//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_URLVariables.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Net_URLVariables.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "GFx/GFx_ASUtils.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc1<Instances::fl_net::URLVariables, Instances::fl_net::URLVariables::mid_decode, const Value, const ASString&> TFunc_Instances_URLVariables_decode;
typedef ThunkFunc0<Instances::fl_net::URLVariables, Instances::fl_net::URLVariables::mid_toString, ASString> TFunc_Instances_URLVariables_toString;

template <> const TFunc_Instances_URLVariables_decode::TMethod TFunc_Instances_URLVariables_decode::Method = &Instances::fl_net::URLVariables::decode;
template <> const TFunc_Instances_URLVariables_toString::TMethod TFunc_Instances_URLVariables_toString::Method = &Instances::fl_net::URLVariables::toString;

namespace Instances { namespace fl_net
{
    URLVariables::URLVariables(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
//##protect##"instance::URLVariables::URLVariables()$data"
//##protect##"instance::URLVariables::URLVariables()$data"
    {
//##protect##"instance::URLVariables::URLVariables()$code"
//##protect##"instance::URLVariables::URLVariables()$code"
    }

    void URLVariables::decode(const Value& result, const ASString& source)
    {
//##protect##"instance::URLVariables::decode()"
        SF_UNUSED2(result, source);
        // parser state machine
        bool parseName = true;

        StringBuffer name, value;
        const char* pstr;

        if (source.IsEmpty())
            return;

        UInt32 c = source.GetFirstCharAt(0, &pstr);
        while (c)
        {
            if ((UInt32)'%' == c)
            {
                // only 2 hexadecimal digits are allowed after the '%';
                // if one of them is invalid the whole chunk of data (starting with '%' and 
                // ending with the first invalid char) is ignored.
                c = source.GetNextChar(&pstr);
                UInt32 cc = 0;
                for (int i = 0; c && i < 2; ++i)
                {
                    if (!SFiswxdigit((wchar_t)c))
                    {
                        cc = 0;
                        break;
                    }
                    c = SFtolower(c);
                    cc <<= i*4;
                    cc |= ((c <= '9') ? c - '0' : c - 'a' + 0xa);

                    c = source.GetNextChar(&pstr);
                }
                if (cc > 0)
                {
                    StringBuffer& sb = (parseName) ? name : value;
                    UPInt sz = sb.GetSize();
                    sb.Resize(sz + 1);
                    sb[sz] = (char)cc;
                }
                else
                    c = source.GetNextChar(&pstr);
                continue;
            }

            if ((UInt32)'\r' == c)
            {
                // flash converts LF to CR, but console printing
                // required LFs
                c = '\n';
            }
            if ((UInt32)'&' == c)
            {
                StringManager& sm = GetVM().GetStringManager();

                if (!SetProperty(Multiname(
                    GetVM().GetPublicNamespace(), sm.CreateString(name.ToCStr())), 
                    Value(sm.CreateString(value.ToCStr())))
                    )
                    // Exception.
                    return;
                name.Clear();
                value.Clear();
                parseName = true;
            }
            else if (parseName)
            {
                if ((UInt32)'=' == c)
                    parseName = false;
                else
                    name.AppendChar(c);
            }
            else
                value.AppendChar(c);

            c = source.GetNextChar(&pstr);
        }

        // clean up stragglers
        if (name.GetLength() > 0)
        {
            StringManager& sm = GetVM().GetStringManager();

            SetProperty(Multiname(
                GetVM().GetPublicNamespace(), sm.CreateString(name.ToCStr())), 
                Value(sm.CreateString(value.ToCStr()))).DoNotCheck();
        }    
//##protect##"instance::URLVariables::decode()"
    }
    void URLVariables::toString(ASString& result)
    {
//##protect##"instance::URLVariables::toString()"
        StringBuffer sbuf;
        const AS3::Object::DynAttrsType* dynAttrs = GetDynamicAttrs();
        if (dynAttrs)
        {
            AS3::Object::DynAttrsType::ConstIterator it = dynAttrs->Begin();
            for (; !it.IsEnd(); ++it)
            {
                // Include "DontEnum" properties.
                const ASString& name = it->First.GetName();
                const AS3::Value& asval = it->Second;

                if (sbuf.GetLength() > 0)
                    sbuf.AppendChar('&');
                ASUtils::AS3::EncodeURIComponent(name.ToCStr(), name.GetSize(), sbuf, true);
                sbuf.AppendChar('=');

                ASString valueStr(GetVM().GetStringManager().CreateEmptyString());
                if (asval.Convert2String(valueStr))
                {
                    ASUtils::AS3::EncodeVar(valueStr.ToCStr(), valueStr.GetSize(), sbuf, true);
                    //printf("%s = %s\n", name.ToCStr(), res.ToCStr());
                }
            }
        }
        result = GetVM().GetStringManager().CreateString(sbuf.ToCStr());
//##protect##"instance::URLVariables::toString()"
    }

//##protect##"instance$methods"
    void URLVariables::AS3Constructor(unsigned argc, const Value* argv)
    {
        if (argc >= 1)
        {
            const Value& v = argv[0];

            ASString data(GetVM().GetStringManager().CreateEmptyString());
            if (v.Convert2String(data))
            {
                Value r;
                decode(r, data);
            }
        }
    }

//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_net
{
    // const UInt16 URLVariables::tito[URLVariables::ThunkInfoNum] = {
    //    0, 2, 
    // };
    const TypeInfo* URLVariables::tit[3] = {
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo URLVariables::ti[URLVariables::ThunkInfoNum] = {
        {TFunc_Instances_URLVariables_decode::Func, &URLVariables::tit[0], "decode", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_URLVariables_toString::Func, &URLVariables::tit[2], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    URLVariables::URLVariables(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::URLVariables::URLVariables()"
//##protect##"InstanceTraits::URLVariables::URLVariables()"

    }

    void URLVariables::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<URLVariables&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_net
{

    URLVariables::URLVariables(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::URLVariables::URLVariables()"
//##protect##"ClassTraits::URLVariables::URLVariables()"

    }

    Pickable<Traits> URLVariables::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) URLVariables(vm, AS3::fl_net::URLVariablesCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_net::URLVariablesCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_net
{
    const TypeInfo URLVariablesTI = {
        TypeInfo::CompileTime | TypeInfo::DynamicObject,
        sizeof(ClassTraits::fl_net::URLVariables::InstanceType),
        0,
        0,
        InstanceTraits::fl_net::URLVariables::ThunkInfoNum,
        0,
        "URLVariables", "flash.net", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo URLVariablesCI = {
        &URLVariablesTI,
        ClassTraits::fl_net::URLVariables::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_net::URLVariables::ti,
        NULL,
    };
}; // namespace fl_net


}}} // namespace Scaleform { namespace GFx { namespace AS3

