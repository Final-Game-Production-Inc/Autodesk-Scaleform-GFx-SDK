/**************************************************************************

Filename    :   CLIKDataBinding.h
Content     :   Data binding interface for CLIK components in MMO Kit.
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "CLIKDataBinding.h"

#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_MsgFormat.h"

//
// CLIK AS3 DataBinding class method map
//

typedef void (*CLIKDataBindingCallback)(CLIKDataBindingAdapter* pthis, CLIKDataBindingAdapter::ParamsType& params);

struct CLIKDataBindingNamedFunction
{
    const char*                 Name;
    CLIKDataBindingCallback     Function;
};

const CLIKDataBindingNamedFunction CLIKDataBindingStaticFunctionTable[] = 
{
    { "createBindingImpl",      &CLIKDataBindingAdapter::OnCreateBinding        },
    { "deleteBindingImpl",      &CLIKDataBindingAdapter::OnDeleteBinding        },

    { 0, 0 }
};


//
// Registrar and delegator for GameDataModel AS3 class callbacks
//

class CLIKDataBindingCallbackHandler : public GFx::FunctionHandler
{
public:
    CLIKDataBindingCallbackHandler(CLIKDataBindingAdapter* pthis) : pThis(pthis) {}

    virtual void    Call(const GFx::FunctionHandler::Params& params)
    {
        ((CLIKDataBindingCallback)params.pUserData)(pThis, params);
    }

protected:
    CLIKDataBindingAdapter*  pThis;
};


class CLIKDataBindingDelegate : public GFx::ExternalInterface
{
public:
    CLIKDataBindingDelegate(CLIKDataBindingAdapter* pthis, GFx::ExternalInterface* pxiChain) 
        : pThis(pthis), pExtIfcChain(pxiChain) {}

    virtual void Callback(GFx::Movie* pmovie, const char* methodName, const GFx::Value* args, unsigned argCount)
    {
        // Register the adapter class methods in the VM
        if (!SFstrcmp("dataBindingInit", methodName))
        {
            SF_DEBUG_MESSAGE(1, "Registering CLIK AS3 Data Binding handlers\n");

            // First param is expected to be the class object
            SF_UNUSED(argCount);
            SF_ASSERT(argCount == 1 && args[0].IsObject());
            GFx::Value classObj = args[0];

            Ptr<CLIKDataBindingCallbackHandler>   handler = *SF_NEW CLIKDataBindingCallbackHandler(pThis);
            for(unsigned i = 0; CLIKDataBindingStaticFunctionTable[i].Name; i++)
            {
                GFx::Value func;
                pmovie->CreateFunction(&func, handler, (void *) CLIKDataBindingStaticFunctionTable[i].Function);
                classObj.SetMember(CLIKDataBindingStaticFunctionTable[i].Name, func);

                GFx::Value test;
                classObj.GetMember(CLIKDataBindingStaticFunctionTable[i].Name, &test);
                SF_ASSERT(test.IsObject());
            }
        }
        else if (pExtIfcChain)
        {
            pExtIfcChain->Callback(pmovie, methodName, args, argCount);
        }
        else
        {
            SF_DEBUG_WARNING1(1, "Unknown method '%s' called on CLIKDataBindingDelegate!\n", methodName);
        }
    }

protected:
    CLIKDataBindingAdapter*         pThis;
    Ptr<GFx::ExternalInterface>     pExtIfcChain;
};


//////////////////////////////////////////////////////////////////////////

void CLIKDataBindingAdapter::Binding::AddHandlerDesc( const char* binding, const CLIKDataBindingHandlerDesc& desc )
{
    HandlerDescriptors.PushBack(desc);
    Data->OnAddHandler(binding, desc);
}

void CLIKDataBindingAdapter::Binding::RemoveHandlerDesc( const char* binding, const CLIKDataBindingHandlerDesc& desc )
{
    int delIdx = -1;
    for (unsigned i = 0; i < HandlerDescriptors.GetSize(); i++)
    {
        if (HandlerDescriptors[i] == desc)
        {
            delIdx = i;
            break;
        }
    }
    if (delIdx >= 0)
    {
        Data->OnRemoveHandler(binding, desc);
        HandlerDescriptors.RemoveAt(delIdx);
    }
}

bool    CLIKDataBindingAdapter::Binding::GetValue(GFx::Movie* pmovie, GFx::Value* value)
{
    if (Data)
    {
        Data->GetValue(pmovie, value);
        return true;
    }
    return false;
}

bool    CLIKDataBindingAdapter::Binding::SetValue(const GFx::Value& value)
{
    if (Data)
    {
        return Data->SetValue(value);
    }
    return false;
}

void    CLIKDataBindingAdapter::Binding::NotifyDataChange(GFx::Movie* pmovie)
{
    GFx::Value arg;
    if (!Data) return;
    Data->GetValue(pmovie, &arg);
    for (unsigned i = 0; i < HandlerDescriptors.GetSize(); i++)
    {
        HandlerDescriptors[i].HandlerClosure.InvokeSelf(NULL, &arg, 1);
    }
}

CLIKDataWrapper*    CLIKDataBindingAdapter::Binding::GetDataWrapper() const
{
    return Data;
}

//////////////////////////////////////////////////////////////////////////

CLIKDataBindingAdapter::CLIKDataBindingAdapter(GFx::Movie* pmovie, GFx::ExternalInterface* pxiChain)
: pMovie(pmovie)
{
    Ptr<CLIKDataBindingDelegate> dg = *SF_NEW CLIKDataBindingDelegate(this, pxiChain);
    pmovie->SetExternalInterface(dg);
}

void    CLIKDataBindingAdapter::RegisterBoundData(const char* key, Ptr<CLIKDataWrapper> boundData)
{
    SF_DEBUG_MESSAGE1(1, "# Created BINDING register: %s\n", key);
    Bindings.Add(key, Binding(boundData));
    Binding* b = Bindings.Get(key);
    SF_UNUSED(b);
    SF_ASSERT(b);
}

void    CLIKDataBindingAdapter::UnregisterBoundData(const char* key)
{
    SF_DEBUG_MESSAGE1(1, "# Destroyed BINDING register: %s\n", key);
    Bindings.Remove(key);
}

void    CLIKDataBindingAdapter::NotifyDataChange(const char* key)
{
    Binding* b = Bindings.Get(key);
    if (b)
        b->NotifyDataChange(pMovie);
}

CLIKDataWrapper* CLIKDataBindingAdapter::GetDataWrapper(const char* key) const
{
    if (!key) return NULL;
    const Binding* b = Bindings.Get(key);
    if (b)
        return b->GetDataWrapper();
    return NULL;
}

void    CLIKDataBindingAdapter::OnCreateBinding(CLIKDataBindingAdapter* pthis, ParamsType& params)
{
    // Must be three params
    SF_ASSERT(params.ArgCount == 3);
    // First is key (string), second is handler ref (closure), third is bound obj (optional)
    SF_ASSERT(params.pArgs[0].IsString());
    SF_ASSERT(params.pArgs[1].IsClosure());
    SF_ASSERT(params.pArgs[2].IsObject() || params.pArgs[2].IsNull());
    const char*     key = params.pArgs[0].GetString();
    GFx::Value&     handlerClosure = params.pArgs[1];
    GFx::Value&     boundObj = params.pArgs[2];

    // Register the new handler
    Binding* b = pthis->Bindings.Get(key);
    if (b)
    {
        CLIKDataBindingHandlerDesc desc(handlerClosure, boundObj);
        b->AddHandlerDesc(key, desc);

        // Notify new handler of current value
        String value;
        GFx::Value arg;
        if (b->GetValue(params.pMovie, &arg))
            desc.HandlerClosure.InvokeSelf(NULL, &arg, 1);
        else
        {
            // A bound value should exist! Did you forget to register one?
            String s;
            Format(s, "Binding register {0} does not exist!\n", key);
            SF_DEBUG_MESSAGE(1, s.ToCStr());
            printf(s.ToCStr());
        }
    }
}

void    CLIKDataBindingAdapter::OnDeleteBinding(CLIKDataBindingAdapter* pthis, ParamsType& params)
{
    // Must be three params
    SF_ASSERT(params.ArgCount == 3);
    // First is key (string), second is handler ref (closure), third is bound obj (optional)
    SF_ASSERT(params.pArgs[0].IsString());
    SF_ASSERT(params.pArgs[1].IsClosure());
    const char*     key = params.pArgs[0].GetString();
    GFx::Value&     handlerClosure = params.pArgs[1];
    GFx::Value&     boundObj = params.pArgs[2];

    Binding* b = pthis->Bindings.Get(key);
    if (b)
    {
        CLIKDataBindingHandlerDesc desc(handlerClosure, boundObj);
        b->RemoveHandlerDesc(key, desc);
    }
}

/*
void     CLIKDataBindingAdapter::OnGetValue(CLIKDataBindingAdapter* pthis, ParamsType& params)
{
    // Must be two params
    SF_ASSERT(params.ArgCount == 1);
    // First is key (string), second is any value
    SF_ASSERT(params.pArgs[0].IsString());
    const char*     key = params.pArgs[0].GetString();
    
    Binding* b = pthis->Bindings.Get(key);
    if (b)
    {
        b->GetValue(params.pRetVal);
    }
    else
    {
        SF_ASSERT(0);   // No binding!
    }
}

void     CLIKDataBindingAdapter::OnSetValue(CLIKDataBindingAdapter* pthis, ParamsType& params)
{
    // Must be two params
    SF_ASSERT(params.ArgCount == 2);
    // First is key (string), second is any value
    SF_ASSERT(params.pArgs[0].IsString());
    const char*     key = params.pArgs[0].GetString();
    GFx::Value&     value = params.pArgs[1];

    Binding* b = pthis->Bindings.Get(key);
    if (b)
    {
        if (b->SetValue(value))
        {
            params.pRetVal->SetBoolean(true);
            return;
        }
    }
    else
    {
        SF_ASSERT(0);   // No binding!
    }
    params.pRetVal->SetBoolean(false);
}
*/
